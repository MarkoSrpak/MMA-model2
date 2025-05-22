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
#include "mic_task.h"
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
    vTaskDelay(pdMS_TO_TICKS(100));
    // Initialize the buzzer
    pwm_init(ID_BUZZER);
    vTaskDelay(pdMS_TO_TICKS(100));
    // Create tasks
    xTaskCreatePinnedToCore(bme68x_task, "BME68x Task", 4096, NULL, 5, NULL, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    // xTaskCreatePinnedToCore(accelGY_task, "AccelGY Task", 4096, NULL, 5,
    // NULL, 1);
    xTaskCreatePinnedToCore(accel_task, "Accel Task", 4096, NULL, 5, NULL, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreatePinnedToCore(gps_task, "GPS Task", 4096, NULL, 5, NULL, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreatePinnedToCore(mic_task, "Mic Task", 4096, NULL, 5, NULL, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreatePinnedToCore(sweat_task, "Sweat Task", 4096, NULL, 5, NULL, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreatePinnedToCore(sdcard_task, "SD Card Task", 4096 * 2, NULL, 5,
                            NULL, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreatePinnedToCore(ble_task, "GPS Task", 4096, NULL, 5, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    vTaskDelay(pdMS_TO_TICKS(3000));
    pwm_on_perc(ID_BUZZER, 50); // Turn on the buzzer at 50% duty cycle
    vTaskDelay(pdMS_TO_TICKS(500));
    pwm_off(ID_BUZZER); // Turn off the buzzer
    vTaskDelete(NULL);
}
