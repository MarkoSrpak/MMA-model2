/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

#ifndef ACCEL_H
#define ACCEL_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_err.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;
} accel_data_t;
/*--------------------------- EXTERN -----------------------------------------*/
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/
void accel_read(accel_data_t *data);
void accel_init();

#endif /*ACCEL_H*/
