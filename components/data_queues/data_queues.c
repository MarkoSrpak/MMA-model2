/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "data_queues.h"
#include "esp_log.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
QueueHandle_t gps_queue;
QueueHandle_t bme_queue;
QueueHandle_t mic_queue;
QueueHandle_t sweat_queue;
QueueHandle_t accel_queue;
QueueHandle_t led_cmd_queue;
QueueHandle_t sdcard_queue;
QueueHandle_t ble_rx_queue;
QueueHandle_t ble_tx_queue;
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void init_queues(void)
{
    // Create queues with depth 1 for sensor sampling logic (overwrite pattern)
    gps_queue = xQueueCreate(1, sizeof(gps_data_t));
    if (gps_queue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create gps_queue");
    }

    bme_queue = xQueueCreate(1, sizeof(bme_data_t));
    if (bme_queue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create bme_queue");
    }

    mic_queue = xQueueCreate(1, sizeof(mic_data_t));
    if (mic_queue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create mic_queue");
    }

    sweat_queue = xQueueCreate(1, sizeof(sweat_data_t));
    if (sweat_queue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create sweat_queue");
    }

    accel_queue = xQueueCreate(1, sizeof(accel_queue_data_t));
    if (accel_queue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create accel_queue");
    }

    led_cmd_queue = xQueueCreate(10, sizeof(int));
    if (led_cmd_queue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create led_cmd_queue");
    }

    // Larger depth for BLE queues
    sdcard_queue = xQueueCreate(10, sizeof(uint8_t));
    if (sdcard_queue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create sdcard_queue");
    }

    ble_rx_queue = xQueueCreate(10, sizeof(ble_queue_item_t));
    if (ble_rx_queue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create ble_rx_queue");
    }

    ble_tx_queue = xQueueCreate(10, sizeof(ble_queue_item_t));
    if (ble_tx_queue == NULL) {
        ESP_LOGE("QUEUE", "Failed to create ble_tx_queue");
    }
}

uint32_t get_current_timestamp_ms(void)
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}
