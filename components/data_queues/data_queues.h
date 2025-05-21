/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

#ifndef DATA_QUEUES_H
#define DATA_QUEUES_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define BLE_QUEUE_ITEM_MAX_LEN 20
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
typedef struct {
    uint8_t data[BLE_QUEUE_ITEM_MAX_LEN];
    size_t len;
} ble_queue_item_t;

typedef struct {
    uint32_t timestamp_ms;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    float latitude;
    float longitude;
    float speed_mps;
    float altitude_m;
} gps_data_t;

typedef struct {
    uint32_t timestamp_ms;
    int16_t temperature_c;
    uint32_t pressure_pa;
    uint32_t humidity_pct;
    uint32_t voc_ohm;
} bme_data_t;

typedef struct {
    uint32_t timestamp_ms;
    uint32_t noise_energy;
} mic_data_t;

typedef struct {
    uint32_t timestamp_ms;
    uint16_t sweat_level;
} sweat_data_t;

typedef struct {
    uint32_t timestamp_ms;
    uint16_t freq;
    uint16_t ampl;
    uint16_t direction;
} accel_data_t;
/*--------------------------- EXTERN -----------------------------------------*/
extern QueueHandle_t gps_queue;
extern QueueHandle_t bme_queue;
extern QueueHandle_t mic_queue;
extern QueueHandle_t sweat_queue;
extern QueueHandle_t accel_queue;
extern QueueHandle_t ble_rx_queue;
extern QueueHandle_t ble_tx_queue;
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/

void init_queues(void);
uint32_t get_current_timestamp_ms(void);

#endif /*DATA_QUEUES_H*/
