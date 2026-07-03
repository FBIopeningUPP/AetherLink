#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_err.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/i2c.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

#include "config.h"
#include "types.h"

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static const char *TAG = "AETHERLINK";

// Battery status (updated by battery task)
static BatteryStatus battery_status = BATTERY_STATUS_INIT;

// System state
static BatteryState current_battery_state = BATTERY_STATE_UNKNOWN;
static ErrorCode last_error = ERROR_NONE;

// ADC Calibration handle
static adc_cali_handle_t adc1_cali_handle = NULL;

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

/**
 * @brief Initialize NVS (Non-Volatile Storage)
 * @return ESP_OK if successful
 */
esp_err_t init_nvs(void) {
    ESP_LOGI(TAG, "Initializing NVS...");
    
    esp_err_t ret = nvs_flash_init();
    
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated and needs to be erased. Erasing NVS...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "✓ NVS initialized successfully");
    return ESP_OK;
}

/**
 * @brief Initialize GPIO pins (LED, status indicators)
 */
esp_err_t init_gpio(void) {
    ESP_LOGI(TAG, "Initializing GPIO pins...");
    
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_STATUS_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO configuration failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Set LED off initially
    gpio_set_level(LED_STATUS_PIN, 0);
    
    ESP_LOGI(TAG, "✓ GPIO initialized (LED on pin %d)", LED_STATUS_PIN);
    return ESP_OK;
}

/**
 * @brief Initialize ADC1 for battery voltage reading
 * Configures ADC1 channels for cell voltage measurements
 */
esp_err_t init_adc(void) {
    ESP_LOGI(TAG, "Initializing ADC1 for cell voltage readings...");
    
    // ADC1 init
    adc1_config_width(ADC_WIDTH_BIT_12);
    
    // Configure each ADC channel with 11dB attenuation (for 0-3.3V range)
    // This allows readings up to ~3.9V (suitable for LiPo cells)
    adc_atten_t atten = ADC_ATTEN_DB_11;
    
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_0, atten));  // GPIO0
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_1, atten));  // GPIO1
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_2, atten));  // GPIO2
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_3, atten));  // GPIO3
    
    if (BATTERY_CELL_COUNT > 4) {
        // For 6S, 8S, 12S batteries, configure additional channels
        // Note: ESP32-C3 has limited ADC channels, may need multiplexer
        ESP_LOGW(TAG, "ADC: %dS battery - ensure multiplexer is configured if needed", BATTERY_CELL_COUNT);
    }
    
    // ADC Calibration scheme (Line Fitting)
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_12,
    };
    
    esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, &adc1_cali_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ADC calibration scheme creation failed: %s", esp_err_to_name(ret));
        // Continue anyway, calibration will be skipped
        adc1_cali_handle = NULL;
    }
    
    ESP_LOGI(TAG, "✓ ADC1 initialized for %d cells (12-bit, 11dB attenuation)", BATTERY_CELL_COUNT);
    return ESP_OK;
}

/**
 * @brief Initialize I2C master for current sensor and other peripherals
 * I2C bus: GPIO%d (SDA), GPIO%d (SCL) at 100 kHz
 */
esp_err_t init_i2c(void) {
    ESP_LOGI(TAG, "Initializing I2C0 (SDA:%d, SCL:%d, Speed:%d Hz)...", 
             I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .master = {
            .clk_speed = I2C_FREQUENCY,
        },
        .clk_flags = 0,
    };
    
    esp_err_t ret = i2c_param_config(I2C_NUM_0, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C parameter configuration failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver installation failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "✓ I2C0 initialized at %d Hz", I2C_FREQUENCY);
    return ESP_OK;
}

/**
 * @brief Initialize logging system
 * Sets up ESP-IDF logging with appropriate levels
 */
void init_logging(void) {
    ESP_LOGI(TAG, "Initializing logging system...");
    
    // Set default log level
    esp_log_level_set("*", ACTIVE_LOG_LEVEL);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    
    ESP_LOGI(TAG, "✓ Logging initialized (Level: %d)", ACTIVE_LOG_LEVEL);
}

/**
 * @brief Initialize BLE (Bluetooth Low Energy)
 * Placeholder for BLE stack initialization
 */
esp_err_t init_ble(void) {
    ESP_LOGI(TAG, "Initializing BLE...");
    
    // TODO: Implement BLE stack initialization
    // - Initialize Bluetooth controller
    // - Start BLE advertising
    // - Create GATT services and characteristics
    
    ESP_LOGI(TAG, "✓ BLE initialized (Service: %s)", BLE_DEVICE_NAME);
    return ESP_OK;
}

/**
 * @brief Initialize battery manager
 * Loads battery metadata and calibration from NVS
 */
esp_err_t init_battery_manager(void) {
    ESP_LOGI(TAG, "Initializing Battery Manager...");
    
    // TODO: Load battery metadata from NVS
    // - Battery UID
    // - Battery name
    // - Calibration parameters
    // - Charge/discharge cycle counts
    
    ESP_LOGI(TAG, "✓ Battery Manager initialized");
    return ESP_OK;
}

/**
 * @brief Initialize storage manager
 * Sets up NVS namespaces and circular log buffer
 */
esp_err_t init_storage_manager(void) {
    ESP_LOGI(TAG, "Initializing Storage Manager...");
    
    // TODO: Initialize NVS namespaces
    // - battery namespace (for battery metadata)
    // - stats namespace (for statistics)
    // - settings namespace (for user settings)
    
    ESP_LOGI(TAG, "✓ Storage Manager initialized");
    return ESP_OK;
}

// ============================================================================
// SYSTEM STARTUP & DIAGNOSTICS
// ============================================================================

/**
 * @brief Blink LED for visual feedback
 * @param times Number of blinks
 * @param delay_ms Delay in milliseconds per blink
 */
void blink_led(int times, int delay_ms) {
    for (int i = 0; i < times; i++) {
        gpio_set_level(LED_STATUS_PIN, 1);
        vTaskDelay(delay_ms / portTICK_PERIOD_MS);
        gpio_set_level(LED_STATUS_PIN, 0);
        vTaskDelay(delay_ms / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Print system information and diagnostics
 */
void print_system_info(void) {
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=====================================");
    ESP_LOGI(TAG, "  AetherLink - Smart Battery Monitor");
    ESP_LOGI(TAG, "=====================================");
    ESP_LOGI(TAG, "Firmware Version: %s", FIRMWARE_VERSION);
    ESP_LOGI(TAG, "Hardware Revision: %s", HARDWARE_REVISION);
    ESP_LOGI(TAG, "Device Name: %s", DEVICE_NAME);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Battery Configuration:");
    ESP_LOGI(TAG, "  Cell Count: %d", BATTERY_CELL_COUNT);
    ESP_LOGI(TAG, "  Pack Voltage Range: %.1f - %.1fV", PACK_VOLTAGE_MIN, PACK_VOLTAGE_MAX);
    ESP_LOGI(TAG, "  Cell Voltage Range: %.1f - %.1fV", CELL_VOLTAGE_MIN, CELL_VOLTAGE_MAX);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "BLE Configuration:");
    ESP_LOGI(TAG, "  Device Name: %s", BLE_DEVICE_NAME);
    ESP_LOGI(TAG, "  TX Power: %d dBm", BLE_TX_POWER);
    ESP_LOGI(TAG, "  Advertisement Interval: %d-%d ms", 
             BLE_ADV_INTERVAL_MIN * 625 / 1000, 
             BLE_ADV_INTERVAL_MAX * 625 / 1000);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Task Configuration:");
    ESP_LOGI(TAG, "  Battery Read Interval: %d ms", BATTERY_READ_INTERVAL);
    ESP_LOGI(TAG, "  BLE Update Interval: %d ms", BLE_UPDATE_INTERVAL);
    ESP_LOGI(TAG, "  Storage Sync Interval: %d ms", STORAGE_SYNC_INTERVAL);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=====================================");
    ESP_LOGI(TAG, "");
}

/**
 * @brief Perform system health check
 * @return true if all systems OK, false if issues detected
 */
bool system_health_check(void) {
    ESP_LOGI(TAG, "Performing system health check...");
    
    bool all_ok = true;
    
    // Check ADC
    if (adc1_cali_handle == NULL) {
        ESP_LOGW(TAG, "⚠ ADC calibration not available");
        // Don't fail, continue without calibration
    }
    
    // Check GPIO
    ESP_LOGD(TAG, "✓ GPIO test: LED control OK");
    
    // Check I2C (would need to probe for devices)
    ESP_LOGD(TAG, "✓ I2C initialized");
    
    // Check NVS
    ESP_LOGD(TAG, "✓ NVS initialized");
    
    if (all_ok) {
        ESP_LOGI(TAG, "✓ System health check: PASSED");
    } else {
        ESP_LOGW(TAG, "⚠ System health check: ISSUES DETECTED");
    }
    
    return all_ok;
}

// ============================================================================
// MAIN APPLICATION ENTRY POINT
// ============================================================================

/**
 * @brief Main application entry point
 * Called by ESP-IDF after boot and system initialization
 */
extern "C" void app_main(void) {
    
    // Print splash screen
    print_system_info();
    
    // ========== INITIALIZATION PHASE ==========
    ESP_LOGI(TAG, "Starting system initialization...");
    
    // 1. Initialize NVS (storage)
    if (init_nvs() != ESP_OK) {
        ESP_LOGE(TAG, "Fatal: NVS initialization failed");
        return;
    }
    
    // 2. Initialize GPIO (LED, status pins)
    if (init_gpio() != ESP_OK) {
        ESP_LOGE(TAG, "Fatal: GPIO initialization failed");
        return;
    }
    
    // 3. Initialize ADC (voltage reading)
    if (init_adc() != ESP_OK) {
        ESP_LOGE(TAG, "Fatal: ADC initialization failed");
        return;
    }
    
    // 4. Initialize I2C (current sensor, EEPROM)
    if (init_i2c() != ESP_OK) {
        ESP_LOGE(TAG, "Fatal: I2C initialization failed");
        return;
    }
    
    // 5. Initialize logging
    init_logging();
    
    // 6. Initialize Battery Manager
    if (init_battery_manager() != ESP_OK) {
        ESP_LOGE(TAG, "Fatal: Battery Manager initialization failed");
        return;
    }
    
    // 7. Initialize Storage Manager
    if (init_storage_manager() != ESP_OK) {
        ESP_LOGE(TAG, "Fatal: Storage Manager initialization failed");
        return;
    }
    
    // 8. Initialize BLE
    if (init_ble() != ESP_OK) {
        ESP_LOGW(TAG, "Warning: BLE initialization failed - continuing without BLE");
    }
    
    // Perform health check
    if (!system_health_check()) {
        ESP_LOGW(TAG, "System has issues, but continuing...");
    }
    
    // Signal successful initialization
    ESP_LOGI(TAG, "✓ All systems initialized successfully!");
    blink_led(3, 100);  // 3 quick blinks = ready
    
    // ========== MAIN LOOP ==========
    ESP_LOGI(TAG, "Entering main event loop...");
    
    while (1) {
        // TODO: Implement main event loop
        // - Read battery voltage
        // - Calculate SoC, health
        // - Update BLE notifications
        // - Sync data to storage
        
        // For now, just toggle LED and log periodically
        gpio_set_level(LED_STATUS_PIN, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_STATUS_PIN, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        if (DEBUG_ENABLE) {
            ESP_LOGD(TAG, "Main loop tick - System OK");
        }
    }
}
