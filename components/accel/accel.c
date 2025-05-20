/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "accel.h"
#include "esp_log.h"
#include "i2c.h"
#include "string.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define ACCEL_ADDR 0x68
#define DATA_LEN   12
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
static const char *TAG = "accel";
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void accel_init()
{
    uint8_t buffer[1] = {0x00};
    uint8_t read_buffer[1] = {0x00};
    I2C_write(ACCEL_ADDR, buffer, 1);     // write reg 0
    I2C_read(ACCEL_ADDR, read_buffer, 1); // read reg 0

    if (read_buffer[0] != 0xEA) {
        ESP_LOGW(TAG, "WHO_AM_I not correct: %02X", read_buffer[0]);
        return;
    }
    ESP_LOGI(TAG, "WHO_AM_I correct: %02X", read_buffer[0]);

    uint8_t config[2] = {0x06, 0x01};
    I2C_write(ACCEL_ADDR, config, 2);

    uint8_t configMag[2] = {0x31, 0x01};
    I2C_write(ACCEL_ADDR, configMag, 2);
    return;
}

void accel_read(accel_data_t *data)
{
    uint8_t raw[DATA_LEN];

    // I2C_read_register(ACCEL_ADDR, 0x2D, raw, DATA_LEN);

    uint8_t buffer[1] = {0x2D}; // Write that we want to read from 2D
    I2C_write(ACCEL_ADDR, buffer, 1);

    // Read 12 bytes from ACCEL_XOUT_H
    I2C_read(ACCEL_ADDR, raw, DATA_LEN);

    buffer[0] = 0x11; // Write that we want to read from 2D
    I2C_write(ACCEL_ADDR, buffer, 1);
    uint8_t rawMag[6];
    // Read 12 bytes from ACCEL_XOUT_H
    I2C_read(ACCEL_ADDR, rawMag, DATA_LEN);

    // Combine high and low bytes (raw)
    int16_t raw_ax = (int16_t)(raw[0] << 8 | raw[1]);
    int16_t raw_ay = (int16_t)(raw[2] << 8 | raw[3]);
    int16_t raw_az = (int16_t)(raw[4] << 8 | raw[5]);
    int16_t raw_gx = (int16_t)(raw[6] << 8 | raw[7]);
    int16_t raw_gy = (int16_t)(raw[8] << 8 | raw[9]);
    int16_t raw_gz = (int16_t)(raw[10] << 8 | raw[11]);
    int16_t rawMag_x = (rawMag[0] << 8 | rawMag[1]);
    int16_t rawMag_y = (rawMag[2] << 8 | rawMag[3]);
    int16_t rawMag_z = (rawMag[4] << 8 | rawMag[5]);

    // Apply scaling
    // Accelerometer: 16384 LSB/g → 1g = 1000 mg → 1 LSB = (1000/16384) mg ≈
    // 0.061 mg To keep integer math: (raw * 1000) / 16384
    data->accel_x = (raw_ax * 1000) / 16384;
    data->accel_y = (raw_ay * 1000) / 16384;
    data->accel_z = (raw_az * 1000) / 16384;

    // Gyroscope: 131 LSB/°/s → 1°/s = 1 deg/s → (raw) / 131
    data->gyro_x = (raw_gx * 10) / 131;
    data->gyro_y = (raw_gy * 10) / 131;
    data->gyro_z = (raw_gz * 10) / 131;

    data->mag_x = rawMag_x;
    data->mag_y = rawMag_y;
    data->mag_z = rawMag_z;

    return;
}
