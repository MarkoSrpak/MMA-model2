/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_dsp.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <math.h>
#include <stdio.h>

#include "accel.h"
#include "accel_task.h"
#include "data_queues.h"
#include "i2c.h"
#include "led_rgb.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define FFT_SIZE 2048
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
typedef enum {
    INDICATOR_NONE = 0,
    INDICATOR_LEFT,
    INDICATOR_RIGHT
} indicator_cmd_t;
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
static void indicator_task(void *pvParameters);
static void accel_fft_task(void *pvParameters);
/*--------------------------- VARIABLES --------------------------------------*/
static const char *TAG = "accel_task";
QueueHandle_t indicator_queue = NULL;
static bool tilt_active = false; // Flag to indicate if tilt is active
static float fft_input[FFT_SIZE * 2];
static float fft_magnitude[FFT_SIZE / 2];
static bool fft_initialized = false;
static SemaphoreHandle_t accel_mutex = NULL;
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
static void indicator_task(void *pvParameters)
{
    indicator_cmd_t cmd;
    uint32_t start_time = 0;
    bool blinking = false;
    const uint32_t duration_ms = 2000;
    const uint32_t interval_ms = 550;
    uint32_t last_toggle_time = 0;

    while (1) {
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (!blinking && xQueueReceive(indicator_queue, &cmd, 0)) {
            if (cmd == INDICATOR_LEFT || cmd == INDICATOR_RIGHT) {
                blinking = true;
                start_time = now;
                last_toggle_time = now;
            }
        }

        if (blinking) {
            if ((now - last_toggle_time) >= interval_ms) {
                if (cmd == INDICATOR_LEFT) {
                    led_rgb_set_color(2, 255, 125, 0); // Orange
                    vTaskDelay(pdMS_TO_TICKS(100));
                    led_rgb_off(2);
                    vTaskDelay(pdMS_TO_TICKS(10));
                    led_rgb_set_color(1, 255, 125, 0); // Orange
                    vTaskDelay(pdMS_TO_TICKS(100));
                    led_rgb_off(1);
                    vTaskDelay(pdMS_TO_TICKS(10));
                    led_rgb_set_color(0, 255, 125, 0); // Orange
                    vTaskDelay(pdMS_TO_TICKS(100));
                    led_rgb_off(0);
                } else if (cmd == INDICATOR_RIGHT) {
                    led_rgb_set_color(6, 255, 125, 0); // Orange
                    vTaskDelay(pdMS_TO_TICKS(100));
                    led_rgb_off(6);
                    vTaskDelay(pdMS_TO_TICKS(10));
                    led_rgb_set_color(7, 255, 125, 0); // Orange
                    vTaskDelay(pdMS_TO_TICKS(100));
                    led_rgb_off(7);
                    vTaskDelay(pdMS_TO_TICKS(10));
                    led_rgb_set_color(8, 255, 125, 0); // Orange
                    vTaskDelay(pdMS_TO_TICKS(100));
                    led_rgb_off(8);
                }
                last_toggle_time = now;
            }

            if ((now - start_time) >= duration_ms) {
                led_rgb_off(0);
                led_rgb_off(1);
                led_rgb_off(2);
                led_rgb_off(6);
                led_rgb_off(7);
                led_rgb_off(8);
                blinking = false;
                tilt_active = false;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void accel_fft_task(void *pvParameters)
{
    int16_t accel_x;
    while (true) {
        int32_t sum = 0;
        int32_t avg = 0;
        int64_t energy = 0;
        // Initialize RFFT if not already done
        if (!fft_initialized) {
            dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
            fft_initialized = true;
            ESP_LOGI(TAG, "FFT initialized");
        }
        // Sample X-axis every 2ms
        for (int i = 0; i < FFT_SIZE; i++) {
            if (xSemaphoreTake(accel_mutex, portMAX_DELAY) == pdTRUE) {
                accel_read_only_x(&accel_x);
                xSemaphoreGive(accel_mutex);
            }
            fft_input[i * 2 + 0] = (float)accel_x; // real
            fft_input[i * 2 + 1] = 0.0f;           // imag
            sum += accel_x / 100;
            vTaskDelay(pdMS_TO_TICKS(2));
        }
        avg = sum / FFT_SIZE;
        for (int i = 0; i < FFT_SIZE; i++) {
            energy += (sum - avg) * (sum - avg); // remove DC component
        }
        // Perform FFT
        dsps_fft2r_fc32(fft_input, FFT_SIZE);
        // Reverse bits order
        dsps_bit_rev_fc32(fft_input, FFT_SIZE);
        // dsps_cplx2real_fc32(fft_input, FFT_SIZE);

        // Calculate magnitude and find peak
        float max_magnitude = 0;
        int max_index = 0;
        for (int i = 1; i < FFT_SIZE / 2; i++) { // skip DC (i=0)
            float real = fft_input[2 * i];
            float imag = fft_input[2 * i + 1];
            fft_magnitude[i] = sqrtf(real * real + imag * imag);

            if (fft_magnitude[i] > max_magnitude) {
                max_magnitude = fft_magnitude[i];
                max_index = i;
            }
        }

        // Sampling rate is 1 sample every 2ms -> 500Hz
        float sampling_freq = 500.0f;
        float bin_resolution = sampling_freq / FFT_SIZE;
        float dominant_freq = max_index * bin_resolution;

        ESP_LOGI(TAG, "FFT peak: freq=%.2f Hz, magnitude=%.2f, energy=%lld",
                 dominant_freq, max_magnitude, energy);
        accel_queue_data_t accel_data = {
            .timestamp_ms = get_current_timestamp_ms(),
            .freq = dominant_freq,
            .ampl = max_magnitude,
            .accel_energy = (uint64_t)energy,
            .direction = 0,
        };
        xQueueOverwrite(accel_queue, &accel_data);
        /* for (int i = 0; i < FFT_SIZE / 2; i++) {
             printf("%d ", (int)fft_magnitude[i]);
         }*/
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void accel_task(void *pvParameters)
{
    // Initialize I2C
    if (I2C_init() != ESP_OK) {
        printf("Failed to initialize I2C\n");
        vTaskDelete(NULL);
    }

    accel_data_t data;
    accel_init();
    indicator_queue = xQueueCreate(4, sizeof(indicator_cmd_t));
    accel_mutex = xSemaphoreCreateMutex();
    if (accel_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create accel mutex");
        vTaskDelete(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(300));
    xTaskCreatePinnedToCore(indicator_task, "Indicator Task", 4096, NULL, 5,
                            NULL, 1);
    xTaskCreatePinnedToCore(accel_fft_task, "Acel FFT Task", 4096, NULL, 5,
                            NULL, 1);
    static bool is_breaking = false;
    static bool leds_enabled = true;
    int led_cmd = 0;
    int counteric = 0;
    while (1) {
        if (xQueueReceive(led_cmd_queue, &led_cmd, 0) == pdTRUE) {
            if (led_cmd == LOG_CMD_LEDT) {
                leds_enabled = !leds_enabled;
            }
            ESP_LOGI(TAG, "LEDs %s", leds_enabled ? "ENABLED" : "DISABLED");
        }

        if (xSemaphoreTake(accel_mutex, portMAX_DELAY) == pdTRUE) {
            accel_read(&data);
            xSemaphoreGive(accel_mutex);
        }

        /* printf("Accel: X=%d Y=%d Z=%d\n", data.accel_x, data.accel_y,
                data.accel_z);
               printf("Gyro: X=%d Y=%d Z=%d\n", data.gyro_x, data.gyro_y,
          data.gyro_z); printf("Mag: X=%d Y=%d Z=%d\n", data.mag_x, data.mag_y,
          data.mag_z);*/
        // printf("Magne: X=%d Y=%d Z=%d\n", data.mag_x, data.mag_y,
        // data.mag_z);

        if (data.accel_x > 800 && data.accel_x < 1200 && data.accel_y < 500
            && data.accel_z > 200 && data.accel_z < 1200
            && (data.accel_x * data.accel_x + data.accel_z * data.accel_z)
                   > 1035 * 1035
            && leds_enabled) {
            // breaking detected
            ESP_LOGI(TAG, "breaking detected");
            led_rgb_set_color(3, 255, 0, 0); // Red
            led_rgb_set_color(4, 255, 0, 0); // Red
            led_rgb_set_color(5, 255, 0, 0); // Red
            is_breaking = true;
            counteric = 0;
        } else if (is_breaking && counteric > 6) {
            led_rgb_off(3);
            led_rgb_off(4);
            led_rgb_off(5);
            is_breaking = false;
        }

        if (!tilt_active && abs(data.accel_y) > 700 && abs(data.accel_x) < 900
            && abs(data.accel_z) < 500 && leds_enabled) {
            tilt_active = true;
            indicator_cmd_t cmd =
                data.accel_y > 0 ? INDICATOR_RIGHT : INDICATOR_LEFT;
            xQueueSend(indicator_queue, &cmd, 0);
            ESP_LOGI(TAG, "Tilt detected: %s",
                     cmd == INDICATOR_LEFT ? "LEFT" : "RIGHT");
        }
        counteric++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
