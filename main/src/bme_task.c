/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "bme68x.h"
#include "bme68x_defs.h"
#include "bme_task.h"
#include "data_queues.h"
#include "i2c.h"
#include "pwm.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define BME68X_I2C_ADDR       BME68X_I2C_ADDR_LOW // Use the lower I2C address
#define BME68X_READ_PERIOD_MS 1000
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
static BME68X_INTF_RET_TYPE bme68x_read_fptr(uint8_t reg_addr,
                                             uint8_t *reg_data, uint32_t length,
                                             void *intf_ptr);
static BME68X_INTF_RET_TYPE bme68x_write_fptr(uint8_t reg_addr,
                                              const uint8_t *reg_data,
                                              uint32_t length, void *intf_ptr);
static void bme68x_delay_us_fptr(uint32_t period, void *intf_ptr);
/*--------------------------- VARIABLES --------------------------------------*/
static uint8_t dev_addr = BME68X_I2C_ADDR;
static bool is_warned = false;
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
static BME68X_INTF_RET_TYPE bme68x_read_fptr(uint8_t reg_addr,
                                             uint8_t *reg_data, uint32_t length,
                                             void *intf_ptr)
{
    I2C_read_register(BME68X_I2C_ADDR, reg_addr, reg_data, length);
    // I2C_write(BME68X_I2C_ADDR, &reg_addr, 1);
    // I2C_read(BME68X_I2C_ADDR, reg_data, length);
    return 0;
}
static BME68X_INTF_RET_TYPE bme68x_write_fptr(uint8_t reg_addr,
                                              const uint8_t *reg_data,
                                              uint32_t length, void *intf_ptr)
{
    uint8_t msg[length + 1];
    msg[0] = reg_addr;
    for (uint32_t i = 0; i < length; i++) {
        msg[i + 1] = reg_data[i];
    }
    I2C_write(BME68X_I2C_ADDR, (uint8_t *)msg, length + 1);
    return 0;
}
static void bme68x_delay_us_fptr(uint32_t period, void *intf_ptr)
{
    esp_rom_delay_us(period);
    return;
}
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void bme68x_task(void *pvParameters)
{
    // Initialize I2C
    if (I2C_init() != ESP_OK) {
        printf("Failed to initialize I2C\n");
        vTaskDelete(NULL);
    }

    // Initialize BME68x
    struct bme68x_dev bme68x;
    bme68x.intf = BME68X_I2C_INTF;
    bme68x.read = (bme68x_read_fptr_t)bme68x_read_fptr;
    bme68x.write = (bme68x_write_fptr_t)bme68x_write_fptr;
    bme68x.delay_us = (bme68x_delay_us_fptr_t)bme68x_delay_us_fptr;
    bme68x.intf_ptr = &dev_addr; // not used
    bme68x.amb_temp = 25;

    int8_t result = bme68x_init(&bme68x);
    if (result == BME68X_OK) {
        printf("BME68x initialized successfully\n");
    } else {
        printf("BME68x initialization failed with error code: %d\n", result);
        vTaskDelete(NULL);
    }

    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;

    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_2X;
    result = bme68x_set_conf(&conf, &bme68x);
    if (result != BME68X_OK) {
        printf("Failed to configure BME68x with error code: %d\n", result);
        vTaskDelete(NULL);
    }

    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;
    result = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme68x);
    if (result != BME68X_OK) {
        printf("Failed to configure heater BME68x with error code: %d\n",
               result);
        vTaskDelete(NULL);
    }

    //  printf("Sample, TimeStamp(ms), Temperature(deg C), Pressure(Pa), "
    //         "Humidity(%%), Gas resistance(ohm), Status\n");

    struct bme68x_data data;
    uint32_t del_period;
    uint8_t n_fields;

    bme_data_t bme_data;

    while (true) {
        result = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme68x);
        if (result != BME68X_OK) {
            printf("Failed to setup op mode with result: %d\n", result);
            continue;
        }

        /* Calculate delay period in microseconds */
        del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme68x)
                     + (heatr_conf.heatr_dur * 1000);

        // printf("Del_period: %lu\n", del_period);

        vTaskDelay(pdMS_TO_TICKS(del_period / 1000 + 100));
        // blocking delay
        // bme68x.delay_us(del_period, bme68x.intf_ptr);

        /* Check if rslt == BME68X_OK, report or handle if otherwise */
        result = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme68x);
        if (result != BME68X_OK) {
            printf("Failed to get data with result: %d\n", result);
            continue;
        }

        if (n_fields) {
            // This works cos we have integer values, should remain like that
            bme_data.timestamp_ms = get_current_timestamp_ms();
            bme_data.temperature_c = data.temperature;
            bme_data.pressure_pa = data.pressure;
            bme_data.humidity_pct = data.humidity;
            bme_data.voc_ohm = data.gas_resistance;
            xQueueOverwrite(bme_queue, &bme_data);
            /*printf("%d(deg C), %lu(Pa), %lu(%%), %lu(ohm), 0x%x\n",
                   (data.temperature), (long unsigned int)data.pressure,
                   (long unsigned int)(data.humidity / 1000),
                   (long unsigned int)data.gas_resistance, data.status);*/
            if (data.temperature > 3500 && data.humidity > 50 && !is_warned) {
                printf("WARNING: Warm and humid weather. Avoid dehydration.\n");
                is_warned = true;
                pwm_on_perc(ID_BUZZER, 50);
                vTaskDelay(pdMS_TO_TICKS(100));
                pwm_off(ID_BUZZER);
                vTaskDelay(pdMS_TO_TICKS(100));
                pwm_on_perc(ID_BUZZER, 50);
                vTaskDelay(pdMS_TO_TICKS(100));
                pwm_off(ID_BUZZER);
                vTaskDelay(pdMS_TO_TICKS(100));
                pwm_on_perc(ID_BUZZER, 50);
                vTaskDelay(pdMS_TO_TICKS(100));
                pwm_off(ID_BUZZER);
                ble_queue_item_t warn_data = {
                    .data = "WARN:Warm and humid",
                    .len = 19,
                };
                xQueueSend(ble_tx_queue, &warn_data, 0);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(BME68X_READ_PERIOD_MS));
    }
}
