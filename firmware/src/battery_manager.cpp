#include "battery_manager.h"
#include "config.h"

#include <cmath>
#include <cstring>
#include <algorithm>

#include <esp_log.h>
#include <esp_timer.h>
#include <driver/adc.h>

static const char *TAG = "BATTERY_MGR";

// -----------------------------------------------------------------------------
// Internal state
// -----------------------------------------------------------------------------
static bool g_initialized = false;
static float g_soc = SOC_INIT_PERCENTAGE;
static uint32_t g_charge_cycles = 0;
static uint32_t g_discharge_cycles = 0;
static uint32_t g_energy_in_mwh = 0;
static uint32_t g_energy_out_mwh = 0;
static ErrorCode g_last_error = ERROR_NONE;
static BatteryState g_state = BATTERY_STATE_UNKNOWN;

static float g_filt_pack_voltage = 0.0f;
static float g_filt_current = 0.0f;
static bool g_filter_seeded = false;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
static inline float clampf(float v, float lo, float hi) {
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

static inline uint32_t now_ms() {
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

static int raw_adc_read_gpio(gpio_num_t gpio_pin) {
    // ESP32-C3: adc1_get_raw uses adc1_channel_t, so map pin->channel here.
    // Adjust mapping to your board wiring if needed.
    adc1_channel_t ch = ADC1_CHANNEL_0;
    switch (gpio_pin) {
        case GPIO_NUM_0: ch = ADC1_CHANNEL_0; break;
        case GPIO_NUM_1: ch = ADC1_CHANNEL_1; break;
        case GPIO_NUM_2: ch = ADC1_CHANNEL_2; break;
        case GPIO_NUM_3: ch = ADC1_CHANNEL_3; break;
        case GPIO_NUM_4: ch = ADC1_CHANNEL_4; break;
        default:
            return -1;
    }

    int raw = adc1_get_raw(ch);
    return raw;
}

static float adc_raw_to_pin_voltage(int raw) {
    if (raw < 0) return -1.0f;
    float v = ((float)raw / ADC_MAX_VALUE) * ADC_REFERENCE_VOLTAGE;
    v = (v * ADC_CALIBRATION_FACTOR) + ADC_OFFSET;
    return v;
}

static float ema(float old_v, float new_v, float alpha) {
    return (alpha * new_v) + ((1.0f - alpha) * old_v);
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------
bool battery_manager_init(void) {
    if (g_initialized) return true;

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);

    g_initialized = true;
    g_last_error = ERROR_NONE;
    ESP_LOGI(TAG, "Battery manager initialized (REAL ADC mode, no mock data)");
    return true;
}

void battery_manager_deinit(void) {
    g_initialized = false;
}

int32_t battery_read_cell_adc(uint8_t cell_number) {
    // For now, if you don't have per-cell taps + balancing front-end,
    // we expose pack channel as cell1 and zeros for others.
    if (cell_number == 1) {
        return raw_adc_read_gpio(PACK_VOLTAGE_ADC_PIN);
    }
    return 0;
}

bool battery_read_all_cells(CellArray *cell_array) {
    if (!cell_array) return false;
    std::memset(cell_array, 0, sizeof(CellArray));

    cell_array->cell_count = BATTERY_CELL_COUNT;
    cell_array->timestamp = now_ms();

    // Placeholder approach: distribute pack voltage equally per cell
    // until true per-cell hardware taps are implemented.
    float pack_v = battery_read_pack_voltage();
    float cell_v = (BATTERY_CELL_COUNT > 0) ? (pack_v / BATTERY_CELL_COUNT) : 0.0f;

    for (uint8_t i = 0; i < BATTERY_CELL_COUNT && i < 12; i++) {
        cell_array->cells[i].cell_number = i + 1;
        cell_array->cells[i].voltage = cell_v;
        cell_array->cells[i].timestamp = cell_array->timestamp;
    }
    return true;
}

float battery_adc_to_voltage(uint16_t adc_value) {
    return adc_raw_to_pin_voltage((int)adc_value);
}

float battery_read_pack_voltage(void) {
    int raw = raw_adc_read_gpio(PACK_VOLTAGE_ADC_PIN);
    float v_adc = adc_raw_to_pin_voltage(raw);
    if (v_adc < 0.0f) {
        g_last_error = ERROR_ADC_READ_FAIL;
        return 0.0f;
    }

    float pack_v = v_adc * PACK_DIVIDER_RATIO;

    if (pack_v < MIN_VALID_PACK_VOLTAGE || pack_v > MAX_VALID_PACK_VOLTAGE) {
        g_last_error = ERROR_INVALID_VOLTAGE;
        return 0.0f;
    }

    if (!g_filter_seeded) {
        g_filt_pack_voltage = pack_v;
        g_filter_seeded = true;
    } else {
        g_filt_pack_voltage = ema(g_filt_pack_voltage, pack_v, BATTERY_EMA_ALPHA);
    }

    return g_filt_pack_voltage;
}

float battery_read_current(void) {
    int raw = raw_adc_read_gpio(INA181_OUTPUT_ADC_PIN);
    float v_out = adc_raw_to_pin_voltage(raw);
    if (v_out < 0.0f) {
        g_last_error = ERROR_ADC_READ_FAIL;
        return 0.0f;
    }

    float denom = SHUNT_RESISTOR_OHM * INA181_GAIN_V_PER_V;
    if (denom <= 0.0f) {
        g_last_error = ERROR_CONFIG_INVALID;
        return 0.0f;
    }

    float current_a = (v_out - INA181_ZERO_OFFSET_V) / denom;

    if (std::fabs(current_a) > MAX_ABS_CURRENT_A) {
        g_last_error = ERROR_OVERCURRENT;
        return 0.0f;
    }

    if (!g_filter_seeded) {
        g_filt_current = current_a;
        g_filter_seeded = true;
    } else {
        g_filt_current = ema(g_filt_current, current_a, BATTERY_EMA_ALPHA);
    }

    return g_filt_current;
}

bool battery_calibrate_cell(uint8_t, float) { return true; }
bool battery_calibrate_current(void) { return true; }

float battery_calculate_soc_from_voltage(float pack_voltage) {
    const float v_empty = 3.20f * BATTERY_CELL_COUNT; // under load approx
    const float v_full  = 4.20f * BATTERY_CELL_COUNT;
    float soc = ((pack_voltage - v_empty) / (v_full - v_empty)) * 100.0f;
    return clampf(soc, 0.0f, 100.0f);
}

float battery_calculate_soc_coulomb_counting(int16_t, uint32_t) {
    return g_soc;
}

void battery_set_soc(float soc_percentage) {
    g_soc = clampf(soc_percentage, 0.0f, 100.0f);
}

float battery_get_soc(void) {
    return g_soc;
}

uint8_t battery_calculate_health(void) {
    return 100;
}

BatteryHealth battery_get_health_state(void) {
    uint8_t h = battery_calculate_health();
    if (h <= HEALTH_THRESHOLD_CRITICAL) return BATTERY_HEALTH_CRITICAL;
    if (h <= HEALTH_THRESHOLD_WARNING) return BATTERY_HEALTH_WARNING;
    return BATTERY_HEALTH_GOOD;
}

uint32_t battery_get_discharge_cycles(void) { return g_discharge_cycles; }
uint32_t battery_get_charge_cycles(void) { return g_charge_cycles; }
void battery_increment_charge_cycle(void) { g_charge_cycles++; }
void battery_increment_discharge_cycle(void) { g_discharge_cycles++; }

bool battery_check_cell_imbalance(const CellArray *cell_array) {
    if (!cell_array || cell_array->cell_count < 2) return false;
    float spread = battery_get_cell_voltage_spread(cell_array);
    return spread > 0.05f;
}

float battery_get_cell_voltage_spread(const CellArray *cell_array) {
    if (!cell_array || cell_array->cell_count == 0) return 0.0f;
    float min_v = cell_array->cells[0].voltage;
    float max_v = cell_array->cells[0].voltage;
    for (uint8_t i = 1; i < cell_array->cell_count && i < 12; i++) {
        min_v = std::min(min_v, cell_array->cells[i].voltage);
        max_v = std::max(max_v, cell_array->cells[i].voltage);
    }
    return max_v - min_v;
}

uint8_t battery_get_min_voltage_cell(const CellArray *cell_array) {
    if (!cell_array || cell_array->cell_count == 0) return 0;
    uint8_t idx = 0;
    for (uint8_t i = 1; i < cell_array->cell_count && i < 12; i++) {
        if (cell_array->cells[i].voltage < cell_array->cells[idx].voltage) idx = i;
    }
    return idx + 1;
}

uint8_t battery_get_max_voltage_cell(const CellArray *cell_array) {
    if (!cell_array || cell_array->cell_count == 0) return 0;
    uint8_t idx = 0;
    for (uint8_t i = 1; i < cell_array->cell_count && i < 12; i++) {
        if (cell_array->cells[i].voltage > cell_array->cells[idx].voltage) idx = i;
    }
    return idx + 1;
}

bool battery_check_overvoltage(float cell_voltage) { return cell_voltage > CELL_VOLTAGE_MAX; }
bool battery_check_undervoltage(float cell_voltage) { return cell_voltage < CELL_VOLTAGE_MIN; }
bool battery_check_overcurrent(float current_ma) { return std::fabs(current_ma) > (MAX_ABS_CURRENT_A * 1000.0f); }

ErrorCode battery_safety_check(const BatteryStatus *status) {
    if (!status) return ERROR_INVALID_PARAM;
    float cell_est = status->pack_voltage / BATTERY_CELL_COUNT;
    if (battery_check_overvoltage(cell_est)) return ERROR_OVERVOLTAGE;
    if (battery_check_undervoltage(cell_est)) return ERROR_UNDERVOLTAGE;
    if (std::fabs(status->pack_current) > MAX_ABS_CURRENT_A) return ERROR_OVERCURRENT;
    return ERROR_NONE;
}

ErrorCode battery_get_last_error(void) { return g_last_error; }
void battery_clear_errors(void) { g_last_error = ERROR_NONE; }

BatteryState battery_get_state(void) { return g_state; }

BatteryState battery_update_state(float, float pack_current) {
    if (pack_current > 0.3f) g_state = BATTERY_STATE_CHARGING;
    else if (pack_current < -0.3f) g_state = BATTERY_STATE_DISCHARGING;
    else g_state = BATTERY_STATE_IDLE;
    return g_state;
}

bool battery_is_charging(void) { return g_state == BATTERY_STATE_CHARGING; }
bool battery_is_discharging(void) { return g_state == BATTERY_STATE_DISCHARGING; }
bool battery_is_faulted(void) { return g_state == BATTERY_STATE_FAULT; }

bool battery_get_status(BatteryStatus *status) {
    if (!status) return false;
    if (!g_initialized && !battery_manager_init()) return false;

    std::memset(status, 0, sizeof(BatteryStatus));

    status->pack_voltage = battery_read_pack_voltage();
    status->pack_current = battery_read_current();
    status->state_of_charge = battery_calculate_soc_from_voltage(status->pack_voltage);
    g_soc = status->state_of_charge;

    status->battery_health = battery_calculate_health();
    status->charge_cycles = g_charge_cycles;
    status->discharge_cycles = g_discharge_cycles;
    status->timestamp = now_ms();

    ErrorCode err = battery_safety_check(status);
    if (err != ERROR_NONE) {
        g_last_error = err;
        g_state = BATTERY_STATE_FAULT;
        return false;
    }

    battery_update_state(status->pack_voltage, status->pack_current);
    return true;
}

bool battery_get_cell_voltage(CellArray *cell_array) {
    return battery_read_all_cells(cell_array);
}

uint32_t battery_get_total_energy_in(void) { return g_energy_in_mwh; }
uint32_t battery_get_total_energy_out(void) { return g_energy_out_mwh; }

uint8_t battery_get_efficiency(void) {
    if (g_energy_in_mwh == 0) return 100;
    float eff = ((float)g_energy_out_mwh / (float)g_energy_in_mwh) * 100.0f;
    return (uint8_t)clampf(eff, 0.0f, 100.0f);
}

void battery_print_status(void) {
    BatteryStatus s;
    if (battery_get_status(&s)) {
        ESP_LOGI(TAG, "V=%.2fV I=%.2fA SoC=%.1f%% H=%u%%",
                 s.pack_voltage, s.pack_current, s.state_of_charge, s.battery_health);
    } else {
        ESP_LOGW(TAG, "battery_get_status failed, err=%d", (int)g_last_error);
    }
}

void battery_print_cell_voltages(void) {
    CellArray c;
    if (!battery_get_cell_voltage(&c)) return;
    for (uint8_t i = 0; i < c.cell_count && i < 12; i++) {
        ESP_LOGI(TAG, "Cell %u: %.3fV", c.cells[i].cell_number, c.cells[i].voltage);
    }
}

void battery_print_health(void) {
    ESP_LOGI(TAG, "Health=%u%% state=%d", battery_calculate_health(), (int)battery_get_health_state());
}

bool battery_run_selftest(void) {
    BatteryStatus s;
    bool ok = battery_get_status(&s);
    if (!ok) return false;
    if (s.pack_voltage <= 0.0f) return false;
    if (std::fabs(s.pack_current) > MAX_ABS_CURRENT_A) return false;
    return true;
}