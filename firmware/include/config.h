#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <driver/gpio.h>
#include "driver/i2c.h"

#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_VERSION "1.0.0"
#define DEVICE_NAME "AetherLink"

//battery config

#define CONFIG_SERIES_CELLS 12

#define CELL_MAX_VOLTAGE_MV 4200
#define CELL_MIN_VOLTAGE_MV 3000
#define CELL_NOMINAL_VOLTAGE_MV 3600

#define MAX_ALLOWED_DELTA_MV 100
#define CRITICAL_DELTA_MV 250

// i2c config

#define AFE_I2C_SDA_PIN GPIO_NUM_4
#define AFE_I2C_SCL_PIN GPIO_NUM_5
#define AFE_I2C_PORT I2C_NUM_0
#define AFE_I2C_ADDR 0x08

//STATUS LED
#define STATS_LED_PIN GPIO_NUM_14

//ble config
#define BLE_DEVICE_NAME "AetherLink"
#define BLE_TX_POWER 7 

#define BLE_SERVICE_UUID_PRIMARY \
    0x12, 0x34, 0x56, 0x78, \
    0x9A, 0xBC, 0xDE, 0xF0, \
    0x12, 0x34, 0x56, 0x78, \
    0x9A, 0xBC, 0xDE, 0xF1

#define BLE_CHAR_UUID_BATTERY_STATE \
    0x12, 0x34, 0x56, 0x78, \
    0x9A, 0xBC, 0xDE, 0xF0, \
    0x12, 0x34, 0x56, 0x78, \
    0x9A, 0xBC, 0xDE, 0xF2

#define BATTERY_TASK_STACK_SIZE 4096
#define BLE_TASK_STACK_SIZE 4096
#define STORAGE_TASK_STACK_SIZE 2048

#define BATTERY_TASK_PRIORITY 3
#define BLE_TASK_PRIORITY 2
#define STORAGE_TASK_PRIORITY 1

#define BATTERY_READ_INTERVAL 1000
#define BLE_UPDATE_INTERVAL 1000

//nvs
#define NVS_NAMESPACE_BATTERY "battery"
#define NVS_NAMEPSACE_STATS "stats"
#define NVS_KEY_BATTERY_UID "uid"

#endif
