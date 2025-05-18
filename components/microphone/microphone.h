/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */
#ifndef MICROPHONE_H
#define MICROPHONE_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include "adc_drv.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- EXTERN -----------------------------------------*/
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/
int mic_init(void);
int mic_read(void);

#endif /*MICROPHONE_H*/
