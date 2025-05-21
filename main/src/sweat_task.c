/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "data_queues.h"
#include "sweat_sensor.h"
#include "sweat_task.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define MOVING_AVG_WINDOW 10
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/

void sweat_task(void *pvParameters)
{
    const int window_size = MOVING_AVG_WINDOW;
    uint16_t readings[MOVING_AVG_WINDOW] = {0};
    int index = 0;
    int count = 0;
    uint32_t sum = 0;

    while (true) {
        uint16_t raw = (uint16_t)sweat_read();

        // Update circular buffer and sum
        sum -= readings[index];
        readings[index] = raw;
        sum += raw;

        index = (index + 1) % window_size;
        if (count < window_size) {
            count++;
        }

        uint16_t average = (uint16_t)(sum / count);

        sweat_data_t data = {
            .timestamp_ms = get_current_timestamp_ms(),
            .sweat_level = average,
        };

        xQueueOverwrite(sweat_queue, &data); // overwrite with latest value

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
