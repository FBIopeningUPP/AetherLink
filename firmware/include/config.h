#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <driver/gpio.h>

#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_VERSION "1.0.0"
#define DEVICE_NAME "AetherLink"

// ============================================================================
// BATTERY CONFIG (12S LiPo)
// ============================================================================
#define BATTERY_CELL_COUNT 12

#define CELL_VOLTAGE_MAX      4.2f
#define CELL_VOLTAGE_MIN      2.5f
#define CELL_VOLTAGE_NOMINAL  3.7f

#define PACK_VOLTAGE_MAX (CELL_VOLTAGE_MAX * BATTERY_CELL_COUNT)   // 50.4V
#define PACK_VOLTAGE_MIN (CELL_VOLTAGE_MIN * BATTERY_CELL_COUNT)   // 30.0V

// ============================================================================
// ADC CONFIG
// ============================================================================
#define ADC_RESOLUTION         12
#define ADC_REFERENCE_VOLTAGE  3.3f
#define ADC_MAX_VALUE          4095.0f

// ESP32-C3 ADC-safe pins (adjust to your exact board routing)
#define PACK_VOLTAGE_ADC_PIN   GPIO_NUM_0
#define INA181_OUTPUT_ADC_PIN  GPIO_NUM_1

// ADC calibration
#define ADC_CALIBRATION_FACTOR 1.0f
#define ADC_OFFSET             0.0f

// ============================================================================
// PACK VOLTAGE DIVIDER (for ~50.4V max -> <3.3V ADC)
// Vadc = Vpack * (R_BOTTOM / (R_TOP + R_BOTTOM))
// Vpack = Vadc * ((R_TOP + R_BOTTOM)/R_BOTTOM)
// ============================================================================
#define PACK_DIVIDER_R_TOP_OHM      330000.0f
#define PACK_DIVIDER_R_BOTTOM_OHM    22000.0f
#define PACK_DIVIDER_RATIO ((PACK_DIVIDER_R_TOP_OHM + PACK_DIVIDER_R_BOTTOM_OHM) / PACK_DIVIDER_R_BOTTOM_OHM)

// ============================================================================
// INA181 CURRENT SENSE CONFIG
// I = (Vout - Vzero) / (Rshunt * Gain)
// ============================================================================
#define SHUNT_RESISTOR_OHM      0.005f     // 5 mΩ
#define INA181_GAIN_V_PER_V     50.0f      // A2 variant = 50 V/V (change if different)
#define INA181_ZERO_OFFSET_V    1.650f     // for bidirectional centered output (0A at ~Vref/2)

// Sanity limits
#define MAX_ABS_CURRENT_A       200.0f
#define MAX_VALID_PACK_VOLTAGE  55.0f
#define MIN_VALID_PACK_VOLTAGE  0.0f

// Filtering
#define BATTERY_EMA_ALPHA       0.20f

// ============================================================================
// STATUS LED
// ============================================================================
#define STATS_LED_PIN GPIO_NUM_14

// ============================================================================
// BLE
// ============================================================================
#define BLE_DEVICE_NAME "AetherLink"
#define BLE_TX_POWER 7 // dBm

#define BLE_SERVICE_UUID_PRIMARY \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF0,  \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF1

#define BLE_CHAR_UUID_BATTERY_STATUS \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF0,  \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF2

#define BLE_CHAR_UUID_CELL_VOLTAGES \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF0,  \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF3

#define BLE_CHAR_UUID_CURRENT \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF0,  \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF4

#define BLE_CHAR_UUID_SOC \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF0,  \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF5

#define BLE_CHAR_UUID_HEALTH \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF0,  \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF6

#define BLE_CHAR_UUID_SETTINGS \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF0,  \
    0x12, 0x34, 0x56, 0x78,  \
    0x9A, 0xBC, 0xDE, 0xF7

#define BLE_ADV_INTERVAL_MIN 32
#define BLE_ADV_INTERVAL_MAX 64

// ============================================================================
// FreeRTOS
// ============================================================================
#define BATTERY_TASK_STACK_SIZE 4096
#define BLE_TASK_STACK_SIZE 4096
#define STORAGE_TASK_STACK_SIZE 2048
#define LOGGER_TASK_STACK_SIZE 2048

#define BATTERY_TASK_PRIORITY 3
#define BLE_TASK_PRIORITY 2
#define STORAGE_TASK_PRIORITY 1
#define LOGGER_TASK_PRIORITY 1

#define BATTERY_READ_INTERVAL 500
#define BLE_UPDATE_INTERVAL 1000
#define STORAGE_SYNC_INTERVAL 5000

#define BATTERY_QUEUE_SIZE 10
#define BLE_QUEUE_SIZE 20
#define STORAGE_QUEUE_SIZE 50
#define LOGGER_QUEUE_SIZE 100

// ============================================================================
// Storage / NVS
// ============================================================================
#define NVS_NAMESPACE_BATTERY "battery"
#define NVS_NAMESPACE_STATS "stats"
#define NVS_NAMESPACE_SETTINGS "settings"

#define NVS_KEY_BATTERY_UID "uid"
#define NVS_KEY_BATTERY_NAME "name"
#define NVS_KEY_BATTERY_CHARGE_CYCLE "charge_cycle"
#define NVS_KEY_BATTERY_DISCHARGE_CYCLE "discharge_cycle"
#define NVS_KEY_BATTERY_TOTAL_CAPACITY "total_capacity"
#define NVS_KEY_BATTERY_BATTERY_HEALTH "battery_health"

// SOC / health
#define SOC_MAX_CAPACITY 5000
#define SOC_INIT_PERCENTAGE 50

#define HEALTH_THRESHOLD_WARNING 80
#define HEALTH_THRESHOLD_CRITICAL 50

// Logger
#define LOG_BUFFER_SIZE 1024
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_VERBOSE 5
#define ACTIVE_LOG_LEVEL LOG_LEVEL_DEBUG

#define DEBUG_ENABLE 1
#define DEBUG_BATTERY_LOGIC 1
#define DEBUG_BLE_COMMS 1
#define DEBUG_STORAGE 1

#endif // CONFIG_H