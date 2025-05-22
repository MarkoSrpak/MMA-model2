/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "data_queues.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
QueueHandle_t gps_queue;
QueueHandle_t bme_queue;
QueueHandle_t mic_queue;
QueueHandle_t sweat_queue;
QueueHandle_t accel_queue;
QueueHandle_t sdcard_queue;
QueueHandle_t ble_rx_queue;
QueueHandle_t ble_tx_queue;
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void init_queues(void)
{
    // Create queues with depth 1 for sensor sampling logic (overwrite pattern)
    gps_queue = xQueueCreate(1, sizeof(gps_data_t));
    bme_queue = xQueueCreate(1, sizeof(bme_data_t));
    mic_queue = xQueueCreate(1, sizeof(mic_data_t));
    sweat_queue = xQueueCreate(1, sizeof(sweat_data_t));
    accel_queue = xQueueCreate(1, sizeof(accel_queue_data_t));

    // Larger depth for BLE queues
    sdcard_queue = xQueueCreate(10, sizeof(uint8_t));
    ble_rx_queue = xQueueCreate(10, sizeof(ble_queue_item_t));
    ble_tx_queue = xQueueCreate(10, sizeof(ble_queue_item_t));
}

uint32_t get_current_timestamp_ms(void)
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}
