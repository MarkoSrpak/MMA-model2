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
#include "sdkconfig.h"
#include <stdio.h>
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define I2C_MASTER_SCL_IO         41 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO         40 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM            0
#define I2C_MASTER_FREQ_HZ        50000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0     /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0     /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS     1000
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void I2C_write(uint8_t device_addr, uint8_t *data, uint32_t size)
{
    // printf("Calling i2c_write: %s", data);
    i2c_master_write_to_device(I2C_MASTER_NUM, device_addr, data, size,
                               I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return;
}

void I2C_read(uint8_t device_addr, uint8_t *data, uint32_t size)
{
    // printf("Calling i2c_read");
    i2c_master_read_from_device(I2C_MASTER_NUM, device_addr, data, size,
                                I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return;
}

esp_err_t I2C_read_register(uint8_t device_addr, uint8_t reg_addr,
                            uint8_t *data, uint32_t size)
{
    return i2c_master_write_read_device(
        I2C_MASTER_NUM, device_addr, &reg_addr, 1, data, size,
        I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t I2C_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}
