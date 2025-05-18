/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "accel.h"
#include "accel_task.h"
#include "i2c.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
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

    // accelGY_data_t data;
    accel_init();

    while (1) {
        // accelGY_read(&data);

        // printf("Accel: X=%d Y=%d Z=%d\n", data.accel_x, data.accel_y,
        // data.accel_z);
        // printf("Temp: %d\n", data.temp);
        // printf("Gyro: X=%d Y=%d Z=%d\n", data.gyro_x, data.gyro_y,
        // data.gyro_z);
        // printf("%d %d %d\n", data.gyro_x, data.gyro_y, data.gyro_z);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
