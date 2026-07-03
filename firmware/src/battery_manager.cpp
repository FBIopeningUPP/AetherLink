#include <string.h>
#include <math.h>
#include <esp_log.h>
#include <driver/adc.h>
#include <driver/i2c.h>
#include <esp_timer.h>
 
#include "config.h"
#include "types.h"
#include "battery_manager.h"

// const and local var

static const char *TAG = "BATTERY_MGR";

// manager state
typedef struct {
    bool initialized;
    BatteryStatus current_status;
    CellArray cell_voltages;
    BatteryState current_state;
    ErrorCode last_error;

    float adc_calibration_factor;
    float adc_calibration_offset;
    float adc_current_sensor_offset;

    float soc_percent;
    uint32_t last_soc_update;

    uint32_t charge_cycles;
    uint32_t discharge_cycles;

    uint32_t total_energy_in_mwh;
    uint32_t total_energy_out_mwh;

    bool was_charging;
    bool was_discharging;
} BatteryManagerState;

static BatteryManagerState battery_state = {
    .initialized = false,
    .current_status = BATTERY_STATUS_INIT,
    .current_state = BATTERY_STATE_UNKNOWN,
    .last_error = ERROR_NONE,
    .adc_calibration_factor = ADC_CALIBRATION_FACTOR,
    .adc_calibration_offset = ADC_OFFSET,
    .current_sensor_offset = CURRENT_OFFSET,
    .soc_percent = SOC_INIT_PERCENTAGE,
    .charge_cycles = 0,
    .discharge_cycles = 0,
    .total_energy_in_mwh = 0,
    .total_energy_out_mwh = 0,
    .was_charging = false,
    .was_discharging = false,
};

// ninit dinint

bool battery_manager_init(void) {
    ESP_LOGI(TAG, "Initializing Battery Manager...");

    if (battery_state.initialized) {
    ESP_LOGW(TAG, "Battery Manager already initialized.");
        return true;
    }

    memset(&battery_state.cell_voltages, 0, sizeof(CellArray));
    battery_state.cell_voltages.cell_count = BATTERY_CELL_COUNT;

    battery_state.current_status.state_of_charge = SOC_INIT_PERCENTAGE;
    battery_state.current_status.battery_health = 100;
    battery_state.current_status.charge_cycles = 0;
    battery_state.current_status.discharge_cycles = 0;

    battery_state.initialized = true;
    ESP_LOGI(TAG, "Battery Manager initialized successfully.");
    return true;
    
}

void battery_manager_deinit(void) {
    ESP_LOGI(TAG, "Deinitializing Battery Manager...");


    battery_state.initialized = false;
    ESP_LOGI(TAG, "Battery Manager deinitialized successfully.");
}

//adc raw data reading

int32_t battery_read_cell_adc(uint8_t cell_number) {
    if (cell_number < 1 || cell_number > BATTERY_CELL_COUNT) {
        ESP_LOGE(TAG, "Invalid cell number: %d", cell_number);
        battery_state.last_error = ERROR_ADC_FAILURE;
        return -1; // Invalid cell number
    }

    if (!battery_state.initialized) {
        ESP_LOGE(TAG, "Battery Manager not initialized.");
        return -1; // Not initialized
    }

    adc1_channel_t channel = ADC1_CHANNEL_0;
    switch (cell_number) {
        case 1: channel = ADC1_CHANNEL_0; break; // GPIO36
        case 2: channel = ADC1_CHANNEL_3; break; // GPIO39
        case 3: channel = ADC1_CHANNEL_6; break; // GPIO34
        case 4: channel = ADC1_CHANNEL_7; break; // GPIO35
        case 5: channel = ADC1_CHANNEL_4; break; // GPIO32
        case 6: channel = ADC1_CHANNEL_5; break; // GPIO33
        case 7: channel = ADC1_CHANNEL_2; break; // GPIO25
        case 8: channel = ADC1_CHANNEL_1; break; // GPIO26
        case 9: channel = ADC1_CHANNEL_8; break; // GPIO27
        case 10: channel = ADC1_CHANNEL_9; break; // GPIO14
        case 11: channel = ADC1_CHANNEL_10; break; // GPIO12
        case 12: channel = ADC1_CHANNEL_11; break; // GPIO13
        default:
            ESP_LOGE(TAG, "Cell %d not available on this board", cell_number);
            return -1;
    }

    int32_t adc_total = 0;
    int num_samples = 5;

    for (int i = 0; i < num_samples; i++) {
        int raw = adc1_get_raw(channel);
        if (raw < 0) {
            ESP_LOGE(TAG, "ADC read failed for cell %d", cell_number);
            battery_state.last_error = ERROR_ADC_FAILURE;
            return -1; // ADC read failure
        }
        adc_total += raw;
    }

    int32_t adc_value = adc_total / num_samples;

    if (DEBUG_BATTERY_LOGIC) {
        ESP_LOGD(TAG, "Cell %d ADC: %ld", cell_number, adc_value);
    }

    return adc_value;
}

bool battery_read_all_cells(CellArray *cell_array) {
    if (!battery_state.initialized || !cell_array) {
        return false;
    }

    cell_array->cell_count = BATTERY_CELL_COUNT;
    cell_array->timestamp = esp_timer_get_time() / 1000;

    for (uint8_t i = 1; i <= BATTERY_CELL_COUNT; i++) {
        int32_t adc_val = battery_read_cell_adc(i);

        if (adc_val < 0){
            ESP_LOGE(TAG, "Failed to read cell %d" 1);
            return false;
        }

        float voltage = battery_adc_to_voltages((uint16_t)adc_val);

        cell_array->cells[i - 1].cell_number = i;
        cell_array->cells[i - 1].voltage = voltage;
        cell_array->cells[i - 1].timestamp = cell_array->timestamp;
    }

    memcpy(&battery_state.cell_voltages, cell_array, sizeof(CellArray));

    return true;
}

float battery_read_current(void) {
    float current = 0.2f; // Placeholder for actual current reading logic
    
}


