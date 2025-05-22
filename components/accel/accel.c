/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "accel.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"
#include "string.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define ACCEL_ADDR 0x68
#define DATA_LEN   20
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
static void switch_bank(uint8_t bank);
/*--------------------------- VARIABLES --------------------------------------*/
static const char *TAG = "accel";
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
static void switch_bank(uint8_t bank)
{
    uint8_t reg_bank_sel[2] = {0x7F, bank << 4}; // 0x7F is REG_BANK_SEL
    I2C_write(ACCEL_ADDR, reg_bank_sel, 2);
}
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

    switch_bank(0);
    uint8_t config[2] = {0x06, 0x01};
    I2C_write(ACCEL_ADDR, config, 2);

    // Enable I2C master interface
    uint8_t user_ctrl[2] = {0x03, 0x20}; // USER_CTRL: I2C_MST_EN
    I2C_write(ACCEL_ADDR, user_ctrl, 2);

    switch_bank(3);
    uint8_t mst_ctrl[2] = {0x01, 0x07}; // 400kHz = 7 per datasheet
    I2C_write(ACCEL_ADDR, mst_ctrl, 2);

    // Setup SLV0 to write 0x08 to AK09916 CNTL2 register (0x31)
    uint8_t slv0_addr[2] = {0x03, 0x0C}; // write to slave 0x0C
    I2C_write(ACCEL_ADDR, slv0_addr, 2);

    uint8_t slv0_reg[2] = {0x04, 0x31}; // CNTL2 register
    I2C_write(ACCEL_ADDR, slv0_reg, 2);

    uint8_t slv0_do[2] = {0x05, 0x08}; // Continuous measurement mode 2
    I2C_write(ACCEL_ADDR, slv0_do, 2);

    uint8_t slv0_ctrl[2] = {0x06, 0x81}; // Enable SLV0, 1 byte
    I2C_write(ACCEL_ADDR, slv0_ctrl, 2);

    // Read from magnetometer's data registers automatically
    uint8_t slv0_addr_read[2] = {
        0x03, 0x19}; // Slave address with read bit set (0x0C + 0x80)
    I2C_write(ACCEL_ADDR, slv0_addr_read, 2);

    uint8_t slv0_reg_start[2] = {
        0x04, 0x11}; // Start reading at AK09916 register 0x11
    I2C_write(ACCEL_ADDR, slv0_reg_start, 2);

    uint8_t slv0_ctrl_read[2] = {
        0x06, 0x87}; // Enable SLV0, read 7 bytes (status + XYZ mag)
    I2C_write(ACCEL_ADDR, slv0_ctrl_read, 2);

    vTaskDelay(pdMS_TO_TICKS(10));

    // Switch back to Bank 0 before normal operation
    switch_bank(0);

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

    // buffer[0] = 0x11; // Write that we want to read from 0x11
    // I2C_write(ACCEL_ADDR, buffer, 1);
    // Read 12 bytes from ACCEL_XOUT_H
    // I2C_read(ACCEL_ADDR, rawMag, DATA_LEN);

    // Combine high and low bytes (raw)
    int16_t raw_ax = (int16_t)(raw[0] << 8 | raw[1]);
    int16_t raw_ay = (int16_t)(raw[2] << 8 | raw[3]);
    int16_t raw_az = (int16_t)(raw[4] << 8 | raw[5]);
    int16_t raw_gx = (int16_t)(raw[6] << 8 | raw[7]);
    int16_t raw_gy = (int16_t)(raw[8] << 8 | raw[9]);
    int16_t raw_gz = (int16_t)(raw[10] << 8 | raw[11]);
    int16_t rawMag_x = (int16_t)(raw[14] << 8 | raw[15]);
    int16_t rawMag_y = (int16_t)(raw[16] << 8 | raw[17]);
    int16_t rawMag_z = (int16_t)(raw[18] << 8 | raw[19]);

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

void accel_read_only_x(int16_t *accel_x)
{
    uint8_t raw[2];

    // I2C_read_register(ACCEL_ADDR, 0x2D, raw, DATA_LEN);

    uint8_t buffer[1] = {0x2D}; // Write that we want to read from 2D
    I2C_write(ACCEL_ADDR, buffer, 1);

    // Read 12 bytes from ACCEL_XOUT_H
    I2C_read(ACCEL_ADDR, raw, 2);

    // Combine high and low bytes (raw)
    int16_t raw_ax = (int16_t)(raw[0] << 8 | raw[1]);

    // Apply scaling
    // Accelerometer: 16384 LSB/g → 1g = 1000 mg → 1 LSB = (1000/16384) mg ≈
    // 0.061 mg To keep integer math: (raw * 1000) / 16384
    *accel_x = (raw_ax * 1000) / 16384;

    return;
}
