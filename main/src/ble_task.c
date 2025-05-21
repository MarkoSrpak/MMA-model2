/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdio.h>

#include "nvs_flash.h"

#include "ble_task.h"
#include "common.h"
#include "data_queues.h"
#include "gap.h"
#include "gatt_svc.h"

/* NimBLE stack APIs */
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

/*--------------------------- MACROS AND DEFINES -----------------------------*/
static const char *TAG = "ble_task";
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
void ble_store_config_init(void);
static void on_stack_reset(int reason);
static void on_stack_sync(void);
static void nimble_host_config_init(void);
static void nimble_host_task(void *param);
static void ble_receive_task(void *param);
static void ble_transmit_task(void *param);
/*--------------------------- VARIABLES --------------------------------------*/
/*--------------------------- STATIC FUNCTIONS -------------------------------*/

static void ble_receive_task(void *param)
{
    ble_queue_item_t item;
    while (true) {
        if (xQueueReceive(ble_rx_queue, &item, portMAX_DELAY)) {
            // Handle received BLE data
            ESP_LOGI("BLE_RX", "Received %d bytes: %.*s", item.len, item.len,
                     item.data);

            if (item.len == 4 && memcmp(item.data, "help", 4) == 0) {
                ESP_LOGI(TAG, "Help command received");
                const char *commands[] = {"help-list commands",
                                          "start-begin workout",
                                          "stop-end workout"};
                for (int i = 0; i < sizeof(commands) / sizeof(commands[0]);
                     ++i) {
                    ble_queue_item_t response;
                    size_t len = strlen(commands[i]);
                    if (len > BLE_QUEUE_ITEM_MAX_LEN) {
                        len = BLE_QUEUE_ITEM_MAX_LEN;
                    }
                    memcpy(response.data, commands[i], len);
                    response.len = len;
                    if (ble_tx_queue != NULL) {
                        xQueueSend(ble_tx_queue, &response, 0);
                    }
                }
            } else if (item.len == 5 && memcmp(item.data, "start", 5) == 0) {
                ESP_LOGI(TAG, "Start command received");
                // TODO: Start logic
            } else if (item.len == 4 && memcmp(item.data, "stop", 4) == 0) {
                ESP_LOGI(TAG, "Stop command received");
                // TODO: Stop logic
            } else {
                ESP_LOGW(TAG, "Unknown command received: %.*s", item.len,
                         item.data);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

static void ble_transmit_task(void *param)
{
    ble_queue_item_t item;
    while (true) {
        if (xQueueReceive(ble_tx_queue, &item, portMAX_DELAY)) {
            // Handle received BLE data
            ESP_LOGI("BLE_TX", "Transmiting %d bytes: %.*s", item.len, item.len,
                     item.data);

            // Notify the data to the connected device
            send_ble_notification((const uint8_t *)item.data, item.len);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

static void nimble_host_task(void *param)
{
    /* Task entry log */
    ESP_LOGI(TAG, "nimble host task has been started!");

    /* This function won't return until nimble_port_stop() is executed */
    nimble_port_run();

    /* Clean up at exit */
    vTaskDelete(NULL);
}

static void nimble_host_config_init(void)
{
    /* Set host callbacks */
    ble_hs_cfg.reset_cb = on_stack_reset;
    ble_hs_cfg.sync_cb = on_stack_sync;
    ble_hs_cfg.gatts_register_cb = gatt_service_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Store host configuration */
    ble_store_config_init();
}

static void on_stack_reset(int reason)
{
    /* On reset, print reset reason to console */
    ESP_LOGI(TAG, "nimble stack reset, reset reason: %d", reason);
}

static void on_stack_sync(void)
{
    /* On stack sync, do advertising initialization */
    adv_init();
}

/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void ble_task(void *pvParameters)
{
    /* Local variables */
    int rc;
    esp_err_t ret;

    /*
     * NVS flash initialization
     * Dependency of BLE stack to store configurations
     */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES
        || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize nvs flash, error code: %d ", ret);
        vTaskDelete(NULL);
        return;
    }

    /* NimBLE stack initialization */
    ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize nimble stack, error code: %d ",
                 ret);
        vTaskDelete(NULL);
        return;
    }

    /* GAP service initialization */
    rc = gap_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to initialize GAP service, error code: %d", rc);
        vTaskDelete(NULL);
        return;
    }

    /* GATT server initialization */
    rc = gatt_svc_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to initialize GATT server, error code: %d", rc);
        vTaskDelete(NULL);
        return;
    }

    /* NimBLE host configuration initialization */
    nimble_host_config_init();

    /* Start NimBLE host task thread and return */
    xTaskCreate(nimble_host_task, "NimBLE Host", 4 * 1024, NULL, 5, NULL);
    xTaskCreate(ble_receive_task, "BLE Receive", 4 * 1024, NULL, 5, NULL);
    xTaskCreate(ble_transmit_task, "BLE Transmit", 4 * 1024, NULL, 5, NULL);

    vTaskDelete(NULL);
    return;
}
