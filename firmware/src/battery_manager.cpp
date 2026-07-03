#include "battery_manager.h"
#include "bq76940_driver.h" 
#include "config.h"
#include "esp_log.h"
#include <cstring>

static const char *TAG = "AetherLink_BMS";

static BatteryState current_pack_state;

esp_err_t battery_manager_init() {
    std::memset(&current_pack_state, 0, sizeof(BatteryState));
    ESP_LOGI(TAG, "Battery Application Manager initialized. Configured for %d cells.", CONFIG_SERIES_CEL);
    return ESP_OK;
}

esp_err_t battery_manager_update() {
    esp_err_t err = afe_get_cell_voltages(current_pack_state.cell_voltages_mv, CONFIG_SERIES_CELL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Telemetry update aborted: Failed to communicate with AFE Driver.");
        return err;
    }
}

uint32_t total_pack_mv = 0;
uint16_t max_cell_mv = 0xFFFF;
uint16_t min_cell_mv = 0x0000;

for (int i = 0; i < CONFIG_SERIES_CELL; i++) {
    uint16_t cell_mv = current_pack_state.cell_voltages_mv[i];
    total_pack_mv += cell_mv;

    if (cell_mv < min_cell_mv) min_cell_mv = cell_mv;
    if (cell_mv > max_cell_mv) max_cell_mv = cell_mv;
}

current_pack_state.pack_voltage_mv = total_pack_mv;
current_pack_state.max_cell_delta_mv = max_cell_mv - min_cell_mv;




