/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

#ifndef ACCELGY_H
#define ACCELGY_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_err.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} accelGY_data_t;
/*--------------------------- EXTERN -----------------------------------------*/
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/
void accelGY_read(accelGY_data_t *data);
void accelGY_init();

#endif /*ACCELGY_H*/
