/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "accelGY.h"
#include "i2c.h"
#include "string.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define ACCELGY_ADDR 0x68 // MPU6050 default I2C address
#define PWR_MGMT_1   0x6B
#define ACCEL_XOUT_H 0x3B
#define DATA_LEN     14 // Number of bytes to read (Accel, Temp, Gyro)
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void accelGY_init()
{
    uint8_t buffer[2] = {PWR_MGMT_1, 0x00};

    I2C_write(ACCELGY_ADDR, buffer, 2);

    uint8_t config[] = {0x1A, 0x03}; // DLPF set to ~44 Hz
    I2C_write(ACCELGY_ADDR, config, 2);
    return;
}

void accelGY_read(accelGY_data_t *data)
{
    uint8_t raw[DATA_LEN];

    // Read 14 bytes from ACCEL_XOUT_H
    esp_err_t ret =
        I2C_read_register(ACCELGY_ADDR, ACCEL_XOUT_H, raw, DATA_LEN);
    if (ret != ESP_OK) {
        printf("I2C read error: %s\n", esp_err_to_name(ret));
        return;
    }

    // Combine high and low bytes (raw)
    int16_t raw_ax = (int16_t)(raw[0] << 8 | raw[1]);
    int16_t raw_ay = (int16_t)(raw[2] << 8 | raw[3]);
    int16_t raw_az = (int16_t)(raw[4] << 8 | raw[5]);
    int16_t raw_temp = (int16_t)(raw[6] << 8 | raw[7]);
    int16_t raw_gx = (int16_t)(raw[8] << 8 | raw[9]);
    int16_t raw_gy = (int16_t)(raw[10] << 8 | raw[11]);
    int16_t raw_gz = (int16_t)(raw[12] << 8 | raw[13]);

    // Apply scaling
    // Accelerometer: 16384 LSB/g → 1g = 1000 mg → 1 LSB = (1000/16384) mg ≈
    // 0.061 mg To keep integer math: (raw * 1000) / 16384
    data->accel_x = (raw_ax * 1000) / 16384;
    data->accel_y = (raw_ay * 1000) / 16384;
    data->accel_z = (raw_az * 1000) / 16384;

    // Temperature: tempC = raw / 340 + 36.53 → in millidegrees:
    data->temp = ((int32_t)raw_temp * 1000) / 340 + 36530;

    // Gyroscope: 131 LSB/°/s → 1°/s = 1000 milli-deg/s → (raw * 1000) / 131
    data->gyro_x = (raw_gx * 1) / 131;
    data->gyro_y = (raw_gy * 1) / 131;
    data->gyro_z = (raw_gz * 1) / 131;

    return;
}
