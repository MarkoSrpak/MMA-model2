/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "accelGY_task.h"
#include "accel_task.h"
#include "ble_task.h"
#include "bme_task.h"
#include "data_queues.h"
#include "gps_task.h"
#include "i2c.h"
#include "led_rgb.h"
#include "microphone.h"
#include "pwm.h"
#include "sdcard_task.h"
#include "sweat_sensor.h"
#include "sweat_task.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void app_main(void)
{
    init_queues();
    // Initialize the LED RGB module
    led_rgb_init();
    // sweat_init();
    mic_init();
    pwm_init(ID_BUZZER); // Initialize the buzzer
    // Create tasks
    xTaskCreatePinnedToCore(bme68x_task, "BME68x Task", 4096, NULL, 5, NULL, 1);
    // xTaskCreatePinnedToCore(accelGY_task, "AccelGY Task", 4096, NULL, 5,
    // NULL, 1);
    xTaskCreatePinnedToCore(accel_task, "Accel Task", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(gps_task, "GPS Task", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(sweat_task, "Sweat Task", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(sdcard_task, "SD Card Task", 4096, NULL, 5, NULL,
                            1);
    xTaskCreatePinnedToCore(ble_task, "GPS Task", 4096, NULL, 5, NULL, 0);

    // uint32_t ledId = 2; // Assuming a single LED for simplicity

    while (true) {
        /*   // Alternate between red, green, and blue
           for (int ledId = 0; ledId < 9; ledId++) {
               led_rgb_set_color(ledId, 255, 0, 0); // Red
               vTaskDelay(pdMS_TO_TICKS(500));

               led_rgb_set_color(ledId, 0, 255, 0); // Green
               vTaskDelay(pdMS_TO_TICKS(500));

               led_rgb_set_color(ledId, 0, 0, 255); // Blue
               vTaskDelay(pdMS_TO_TICKS(500));

               led_rgb_on(ledId); // White
               vTaskDelay(pdMS_TO_TICKS(500));
               // Turn the LED off
               led_rgb_off(ledId);
               vTaskDelay(pdMS_TO_TICKS(500));
           }
           // printf("Sweat sensor value: %d\n", sweat_read());
           //  printf("Hello, FreeRTOS!\n");
           // pwm_on_perc(ID_BUZZER, 50);
           // vTaskDelay(pdMS_TO_TICKS(500));
           // pwm_off(ID_BUZZER);*/
        vTaskDelay(pdMS_TO_TICKS(2000));
        // printf("Mic sensor value: %d\n", mic_read());
        // printf("%d\n", mic_read());
    }
}
