/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "i2c.h"
#include <stdio.h>
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define BME_ADDR 0x44
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
int bme_read(double *temp, double *humidity, double *pressure, double *voc)
{
    uint8_t read_buffer[6];
    // citamo 6 bytova
    I2C_read(BME_ADDR, read_buffer, sizeof(read_buffer));

    // printf("%x:%x:%x::%x:%x:%x\n", read_buffer[0], read_buffer[1],
    // read_buffer[2], read_buffer[3], read_buffer[4], read_buffer[5]);
    uint16_t temp_r =
        (uint16_t)((((uint16_t)read_buffer[0]) << 8) | read_buffer[1]);
    uint16_t humi_r =
        (uint16_t)((((uint16_t)read_buffer[3]) << 8) | read_buffer[4]);

    *temp = (double)(temp_r) / 65535.0f * 175.0f - 45.0f;
    *humidity = (double)(humi_r) / 65535.0f * 100.0f;

    return 0;
}

int bme_start_measurement()
{
    uint8_t write_buf[2] = {0x2C, 0x10};
    I2C_write(BME_ADDR, write_buf, sizeof(write_buf));
    return 0;
}
