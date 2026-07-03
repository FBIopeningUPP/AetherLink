#include "ble_manager.h"
#include "config.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <cstdio>
#include <cstring>

static const char *TAG = "AetherLink_BLE";

static const ble_uuid128_t gatt_svr_svc_uuid =
    BLE_UUID128_INIT(BLE_SERVICE_UUID_PRIMARY);

static const ble_uuid128_t gatt_svr_chr_telemetry_uuid =
    BLE_UUID128_INIT(BLE_CHAR_UUID_BATTERY_STATUS);

static BatteryState current_telemetry;
static uint16_t telemetry_chr_val_handle;
static char device_name[32];

static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle, 
                               struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        int rc = os_mbuf_append(ctxt->om, &current_telemetry, sizeof(BatteryState));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
            .type = BLE_GATT_SVC_TYPE_PRIMARY,
            .uuid = &gatt_svr_svc_uuid.u,
            .characteristics = (struct ble_gatt_chr_def[]) {
                {
                    .uuid = &gatt_svr_chr_telemetry_uuid.u,
                    .access_cb = gatt_svr_chr_access,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &telemetry_chr_val_handle,
                },
                {
                    0, // No more characteristics in this service
                }
            },
    }
}

static void ble_app_advertise(void) {
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;

    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    fields.uuids128 = (ble_uuid_t *)&gatt_svr_svc_uuid;
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ble_gap_adv_start(nimble_port_own_addr_type(), NULL, BLE_HS_FOREVER,
                      &adv_params, NULL, NULL);
    ESP_LOGI(TAG, "Advertising started as %s", device_name);
}

static void ble_app_on_sync(void) {
    ble_hs_id_infer_auto(0, NULL);
    ble_app_advertise();
}

static void ble_host_task(void *param) {
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t ble_manager_init() {
    uint8_t mac[6];
    snprintf(device_name, sizeof(device_name), "%s-%02X%02X", BLE_DEVICE_NAME, mac[4], mac[5]);
    ESP_LOGI(TAG, "Initializing BLE Device: %s", device_name);

    esp_err_t err = nimble_port_init();
    if (err != ESP_OK) return err;

    ble_hs_cfg.sync_cb = ble_app_on_sync;

    ble_svc_gap_device_name_set(device_name);
    ble_svc_gap_init();
    ble_svc_gatt_init();

    err = ble_gatts_count_cfg(gatt_svr_svcs);
    if (err != 0) return ESP_FAIL;
    err = ble_gatts_add_svcs(gatt_svr_svcs);
    if (err != 0) return ESP_FAIL;

    nimble_port_freertos_init(ble_host_task);

    return ESP_OK;
}

void ble_manager_update_telemetry(const BatteryState* state) {
    if (state != nullptr) {

        memcpy(&current_telemetry, state, sizeof(BatteryState));

        if (telemetry_chr_val_handle != 0) {
            struct os_mbuf *om = ble_hs_mbuf_from_flat(&current_telemetry, sizeof(BatteryState));
            ble_gatts_notify_custom(0, telemetry_chr_val_handle, om);
        }
    }
}
