/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "sdcard_task.h"
#include "sdspi.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void sdcard_task(void *pvParameters)
{
    // Initialize the SD card
    sdspi_init();

    // Example usage of writing and reading from the SD card
    char txBuffer[128] = "SD Card started";
    char rxBuffer[128];

    sdspi_write_line("logs/logstart.txt", txBuffer, sizeof(txBuffer));
    sdspi_read("logs/logstart.txt", rxBuffer, sizeof(rxBuffer));

    printf("Read from SD Card: %s\n", rxBuffer);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}
