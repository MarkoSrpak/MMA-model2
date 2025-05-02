/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

#ifndef I2C_H
#define I2C_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_err.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- EXTERN -----------------------------------------*/
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/
esp_err_t I2C_init(void);
void I2C_write(uint8_t device_addr, uint8_t *data, uint32_t size);
void I2C_read(uint8_t device_addr, uint8_t *data, uint32_t size);
esp_err_t I2C_read_register(uint8_t device_addr, uint8_t reg_addr,
                            uint8_t *data, uint32_t size);

#endif /*I2C_H*/
