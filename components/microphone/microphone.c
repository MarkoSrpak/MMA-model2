/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "microphone.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define MIC_ADC_UNIT    (2)
#define MIC_ADC_CHANNEL ADC_CHANNEL_4
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/

int mic_init(void)
{
    // Microphone is on adc unit 2, channels 5, aka GPIO15
    return adc_init_channel(MIC_ADC_UNIT, MIC_ADC_CHANNEL);
}

int mic_read(void)
{
    return adc_read(MIC_ADC_UNIT, MIC_ADC_CHANNEL);
}
