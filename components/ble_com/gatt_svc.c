/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "gatt_svc.h"
#include "common.h"
#include "freertos/queue.h"

static const char *TAG = "NimBLE_GATT";

/* Private function declarations */
static int custom_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt, void *arg);
/* Private variables */
static const ble_uuid128_t custom_svc_uuid =
    BLE_UUID128_INIT(0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0xfe, 0xdc,
                     0xba, 0x09, 0x87, 0x65, 0x43, 0x21); // Service UUID

static const ble_uuid128_t custom_chr_uuid =
    BLE_UUID128_INIT(0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe, 0xbe, 0xba,
                     0xfe, 0xed, 0xfa, 0xce, 0x12, 0x34); // Characteristic UUID

static uint8_t custom_chr_val[BLE_QUEUE_ITEM_MAX_LEN] = {
    0}; // buffer for read/write data
static uint16_t custom_chr_val_handle;

static uint16_t custom_chr_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static bool custom_chr_notify_enabled = false;

/* GATT services table */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &custom_svc_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = &custom_chr_uuid.u,
                    .access_cb = custom_chr_access,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE
                             | BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &custom_chr_val_handle,
                },
                {0} // End
            },
    },
    {0} // No more services
};

void send_ble_notification(const uint8_t *data, uint16_t len)
{
    if (!custom_chr_notify_enabled
        || custom_chr_conn_handle == BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGW("BLE_NOTIFY",
                 "Cannot notify: not subscribed or invalid connection");
        return;
    }

    // Create an mbuf for the notification payload
    struct os_mbuf *om = ble_hs_mbuf_from_flat(data, len);
    if (!om) {
        ESP_LOGE("BLE_NOTIFY", "Failed to allocate mbuf for notification");
        return;
    }

    // Send the notification
    int rc = ble_gatts_notify_custom(custom_chr_conn_handle,
                                     custom_chr_val_handle, om);
    if (rc != 0) {
        ESP_LOGE("BLE_NOTIFY", "Notification failed, rc=%d", rc);
    } else {
        ESP_LOGI("BLE_NOTIFY", "Notification sent: %.*s", len, data);
    }
}

/* characteristic access callback */
static int custom_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR :
        rc = os_mbuf_append(ctxt->om, custom_chr_val, sizeof(custom_chr_val));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    case BLE_GATT_ACCESS_OP_WRITE_CHR :
        if (ctxt->om->om_len > sizeof(custom_chr_val)) {
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        rc = os_mbuf_copydata(ctxt->om, 0, ctxt->om->om_len, custom_chr_val);
        if (rc != 0) {
            return BLE_ATT_ERR_UNLIKELY;
        }

        // Push to queue
        ble_queue_item_t item = {0};
        item.len = ctxt->om->om_len;
        memcpy(item.data, custom_chr_val, item.len);
        if (ble_rx_queue != NULL) {
            xQueueSend(ble_rx_queue, &item, 0); // non-blocking send
        }
        return 0;
    default :
        return BLE_ATT_ERR_UNLIKELY;
    }
}

/* Public functions */

/*
 *  Handle GATT attribute register events
 *      - Service register event
 *      - Characteristic register event
 *      - Descriptor register event
 */
void gatt_service_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    /* Local variables */
    char buf[BLE_UUID_STR_LEN];

    /* Handle GATT attributes register events */
    switch (ctxt->op) {
    /* Service register event */
    case BLE_GATT_REGISTER_OP_SVC :
        ESP_LOGD(TAG, "registered service %s with handle=%d",
                 ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                 ctxt->svc.handle);
        break;

    /* Characteristic register event */
    case BLE_GATT_REGISTER_OP_CHR :
        ESP_LOGD(TAG,
                 "registering characteristic %s with "
                 "def_handle=%d val_handle=%d",
                 ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                 ctxt->chr.def_handle, ctxt->chr.val_handle);
        break;

    /* Descriptor register event */
    case BLE_GATT_REGISTER_OP_DSC :
        ESP_LOGD(TAG, "registering descriptor %s with handle=%d",
                 ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                 ctxt->dsc.handle);
        break;

    /* Unknown event */
    default :
        assert(0);
        break;
    }
}

/*
 *  GATT server subscribe event callback
 *      1. Update custom subscription status
 */

void gatt_svr_subscribe_cb(struct ble_gap_event *event)
{
    /* Check connection handle */
    if (event->subscribe.conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGI(TAG, "subscribe event; conn_handle=%d attr_handle=%d",
                 event->subscribe.conn_handle, event->subscribe.attr_handle);
    } else {
        ESP_LOGI(TAG, "subscribe by nimble stack; attr_handle=%d",
                 event->subscribe.attr_handle);
    }

    /* Check attribute handle */
    if (event->subscribe.attr_handle == custom_chr_val_handle) {
        custom_chr_conn_handle = event->subscribe.conn_handle;
        custom_chr_notify_enabled =
            event->subscribe.cur_notify; // true if enabled
        ESP_LOGI(TAG, "Notify %s by conn_handle=%d",
                 custom_chr_notify_enabled ? "enabled" : "disabled",
                 custom_chr_conn_handle);
    }
}

/*
 *  GATT server initialization
 *      1. Initialize GATT service
 *      2. Update NimBLE host GATT services counter
 *      3. Add GATT services to server
 */
int gatt_svc_init(void)
{
    /* Local variables */
    int rc;

    /* 1. GATT service initialization */
    ble_svc_gatt_init();

    /* 2. Update GATT services counter */
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    /* 3. Add GATT services */
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
