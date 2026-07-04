#include "battery_manager.h"
#include "bq76940_driver.h" 
#include "config.h"
#include "pinout.h"
#include "esp_log.h"
#include <cstring>

static const char *TAG = "AetherLink_BMS";

static BatteryState current_pack_state;

esp_err_t battery_manager_init() {
    std::memset(&current_pack_state, 0, sizeof(BatteryState));
    ESP_LOGI(TAG, "Battery Application Manager initialized. Configured for %d cells.", BATTERY_DEFAULT_CELLS);
    return ESP_OK;
}

esp_err_t battery_manager_update() {
    float temp_cell_mv[12] = {0};
    bool bq_ok = bq_read_all_cell_mv(temp_cell_mv, BATTERY_DEFAULT_CELLS);
    if (!bq_ok) {
        ESP_LOGE(TAG, "Telemetry update aborted: Failed to communicate with AFE driver.");
        return ESP_FAIL;
    }

    uint32_t total_pack_mv = 0;
    uint16_t min_cell_mv = 0xFFFF;
    uint16_t max_cell_mv = 0x0000;

    for (int i = 0; i < BATTERY_DEFAULT_CELLS; i++) {
        current_pack_state.cell_voltages_mv[i] = (uint16_t)temp_cell_mv[i];
        uint16_t cell_mv = current_pack_state.cell_voltages_mv[i];
        total_pack_mv += cell_mv;
        
        if (cell_mv < min_cell_mv) min_cell_mv = cell_mv;
        if (cell_mv > max_cell_mv) max_cell_mv = cell_mv;
    }
    
    current_pack_state.pack_voltage_mv = total_pack_mv;
    
    current_pack_state.max_cell_delta_mv = max_cell_mv - min_cell_mv;

    if (current_pack_state.max_cell_delta_mv > CRITICAL_DELTA_MV) {
        current_pack_state.soh_percentage = 60; // Critical degradation
    } else if (current_pack_state.max_cell_delta_mv > MAX_ALLOWED_DELTA_MV) {
        current_pack_state.soh_percentage = 85; // Unbalanced Warning
    } else {
        current_pack_state.soh_percentage = 100; // Good Health
    }

    current_pack_state.pack_status = 0; 
    
    current_pack_state.charge_cycles = 14;    
    current_pack_state.discharge_cycles = 32; 

    if (min_cell_mv >= CELL_MAX_VOLTAGE_MV) {
        current_pack_state.soc_percentage = 100;
    } else if (min_cell_mv <= CELL_MIN_VOLTAGE_MV) {
        current_pack_state.soc_percentage = 0;
    } else {
        current_pack_state.soc_percentage = (uint8_t)(((min_cell_mv - CELL_MIN_VOLTAGE_MV) * 100) / 
                                            (CELL_MAX_VOLTAGE_MV - CELL_MIN_VOLTAGE_MV));
    }

    ESP_LOGD(TAG, "Pack: %lu mV | Min: %d mV | Delta: %d mV | SoC: %d%% | SoH: %d%%", 
             current_pack_state.pack_voltage_mv, min_cell_mv, current_pack_state.max_cell_delta_mv, 
             current_pack_state.soc_percentage, current_pack_state.soh_percentage);

    return ESP_OK;
}

const BatteryState* battery_manager_get_state() {
    return &current_pack_state;
}