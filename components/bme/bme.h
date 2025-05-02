/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

#ifndef BME_H
#define BME_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_err.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- EXTERN -----------------------------------------*/
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/
int bme_read(double *temp, double *humidity, double *pressure, double *voc);
int bme_start_measurement();

#endif /*BME_H*/
