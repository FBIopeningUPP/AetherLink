#include "bq76940_driver.h"
#include "esp_log.h"

static const char* TAG = "AFE_DRIVER";

esp_err_t afe_init() {
    ESP_LOGI(TAG, "Mock AFE Driver Initialized successfully.");
    return ESP_OK;
}

esp_err_t afe_get_cell_voltages(uint16_t* cell_voltages_mv, uint8_t max_cells) {
    // Feed fake cell data so the Web Dashboard has something to display!
    for (int i = 0; i < max_cells; i++) {
        // Generates fake voltages: 3800mV, 3805mV, 3810mV, etc.
        cell_voltages_mv[i] = 3800 + (i * 5); 
    }
    return ESP_OK;
}