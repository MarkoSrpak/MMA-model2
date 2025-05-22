/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdio.h>

#include "accel.h"
#include "accel_task.h"
#include "i2c.h"
#include "led_rgb.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
static const char *TAG = "accel_task";
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
typedef enum {
    INDICATOR_NONE = 0,
    INDICATOR_LEFT,
    INDICATOR_RIGHT
} indicator_cmd_t;
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
static void indicator_task(void *pvParameters);
/*--------------------------- VARIABLES --------------------------------------*/
QueueHandle_t indicator_queue = NULL;
static bool tilt_active = false; // Flag to indicate if tilt is active
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
    vTaskDelay(pdMS_TO_TICKS(300));
    xTaskCreatePinnedToCore(indicator_task, "Indicator Task", 4096, NULL, 5,
                            NULL, 1);
    bool is_breaking = false;

    while (1) {
        accel_read(&data);

        /* printf("Accel: X=%d Y=%d Z=%d\n", data.accel_x, data.accel_y,
                data.accel_z);
               printf("Gyro: X=%d Y=%d Z=%d\n", data.gyro_x, data.gyro_y,
          data.gyro_z); printf("Mag: X=%d Y=%d Z=%d\n", data.mag_x, data.mag_y,
          data.mag_z);
       */
        if (data.accel_x > 800 && data.accel_x < 1200 && data.accel_y < 500
            && data.accel_z > 400 && data.accel_z < 1200) {
            // breaking detected
            ESP_LOGI(TAG, "breaking detected");
            led_rgb_set_color(3, 255, 0, 0); // Red
            led_rgb_set_color(4, 255, 0, 0); // Red
            led_rgb_set_color(5, 255, 0, 0); // Red
            is_breaking = true;
        } else if (is_breaking) {
            led_rgb_off(3);
            led_rgb_off(4);
            led_rgb_off(5);
            is_breaking = false;
        }

        if (!tilt_active && abs(data.accel_y) > 700 && abs(data.accel_x) < 900
            && abs(data.accel_z) < 500) {
            tilt_active = true;
            indicator_cmd_t cmd =
                data.accel_y > 0 ? INDICATOR_RIGHT : INDICATOR_LEFT;
            xQueueSend(indicator_queue, &cmd, 0);
            ESP_LOGI(TAG, "Tilt detected: %s",
                     cmd == INDICATOR_LEFT ? "LEFT" : "RIGHT");
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
