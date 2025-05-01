/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */
#ifndef SWEAT_SENSOR_H
#define SWEAT_SENSOR_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include "adc_drv.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- EXTERN -----------------------------------------*/
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/
int sweat_init(void);
int sweat_read(void);

#endif /*SWEAT_SENSOR_H*/
