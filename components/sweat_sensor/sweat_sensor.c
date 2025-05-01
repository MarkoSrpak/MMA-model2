/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "sweat_sensor.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define POT_ADC_UNIT    (2)
#define POT_ADC_CHANNEL ADC_CHANNEL_5
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/

int sweat_init(void)
{
    // Potentiometer is on adc unit 2, channels 6, aka GPIO16
    return adc_init_channel(POT_ADC_UNIT, POT_ADC_CHANNEL);
}

int sweat_read(void)
{
    return adc_read(POT_ADC_UNIT, POT_ADC_CHANNEL);
}
