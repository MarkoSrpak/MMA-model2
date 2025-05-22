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
#include "mic_task.h"
#include "microphone.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define SAMPLE_INTERVAL_MS 2 // ~500 Hz
#define SAMPLE_DURATION_MS 100
#define MAX_SAMPLES        (SAMPLE_DURATION_MS / SAMPLE_INTERVAL_MS)
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
static mic_data_t mic_data = {0};
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/

void mic_task(void *pvParameters)
{
    mic_init();
    uint16_t samples[MAX_SAMPLES];
    const int sample_count = MAX_SAMPLES;

    while (true) {
        uint32_t sum = 0;

        // Step 1: Collect samples
        for (int i = 0; i < sample_count; ++i) {
            samples[i] = mic_read();
            sum += samples[i];
            vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
        }

        // Step 2: Calculate average (DC component)
        uint16_t mean = sum / sample_count;

        // Step 3: Compute energy = sum((sample - mean)^2)
        uint32_t energy = 0;
        for (int i = 0; i < sample_count; ++i) {
            int16_t centered = (int16_t)samples[i] - (int16_t)mean;
            energy += (uint32_t)((uint32_t)centered * (uint32_t)centered);
        }
        // printf("%ld\n", energy);
        //  Step 4: Package and queue
        mic_data.timestamp_ms = get_current_timestamp_ms(),
        mic_data.noise_energy = energy;

        xQueueOverwrite(mic_queue, &mic_data);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
