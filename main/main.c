/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_rgb.h"
#include <stdio.h>
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void app_main(void)
{
    // Initialize the LED RGB module
    led_rgb_init();

    uint32_t ledId = 0; // Assuming a single LED for simplicity

    while (true) {
        // Alternate between red, green, and blue
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
        printf("Hello, FreeRTOS!\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
