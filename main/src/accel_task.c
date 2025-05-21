/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "accel.h"
#include "accel_task.h"
#include "i2c.h"
#include "led_rgb.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
static const char *TAG = "accel_task";
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
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

    while (1) {
        accel_read(&data);
        /*
                printf("Accel: X=%d Y=%d Z=%d\n", data.accel_x, data.accel_y,
                       data.accel_z);
                printf("Gyro: X=%d Y=%d Z=%d\n", data.gyro_x, data.gyro_y,
           data.gyro_z); printf("Mag: X=%d Y=%d Z=%d\n", data.mag_x, data.mag_y,
           data.mag_z);
        */
        if (data.accel_x > 800 && data.accel_x < 1200 && data.accel_y < 500
            && data.accel_z > 700 && data.accel_z < 1200) {
            // breaking detected
            ESP_LOGI(TAG, "breaking detected");
            led_rgb_set_color(3, 255, 0, 0); // Red
            led_rgb_set_color(4, 255, 0, 0); // Red
            led_rgb_set_color(5, 255, 0, 0); // Red
        } else {
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
