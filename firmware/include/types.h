#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <cstring>
 // battery data strcutre
 /** 
  *     single voltage reading
  */
typedef struct {
    uint8_t cell_number; // 1 to 12
    float voltage; // voltage reading in volts v
    uint32_t timestamp; // reading since the boot of micro controller in ms
} CellVoltage;

// complete battery status thing snapshot
typedef struct {
    float pack_voltage; // total voltage of the battery pack in volts v
    float pack_current; // current of the battery pack in amps a
    float state_of_charge; // state of charge of the battery pack in percentage %
    uint8_t battery_health; // battery health in percentage %
    uint32_t charge_cycles; // number of charge cycles the battery has gone through
    uint32_t discharge_cycles; // number of discharge cycles the battery has gone through
    uint32_t timestamp; // reading since the boot of micro controller in ms
} BatteryStatus;

// cell voltage ka array


typedef struct {
    CellVoltage cells[12]; // array of cell voltage readings
    uint8_t cell_count; // number of cells in the battery pack
    uint32_t timestamp; // reading since the boot of micro controller in ms
} CellArray;

//battery identity
typedef struct {
    char battery_uid[32]; // unique identifier for the battery pack
    char battery_name[64]; // name of the battery pack
    uint16_t nominal_capacity; // nominal capacity of the battery pack in mAh
    uint8_t cell_count; // number of cells in the battery pack
    uint32_t manufacturing_date; // manufacture date of the battery pack in YYYYMMDD format
    uint32_t total_energy_in; // total energy input to the battery pack in mWh
    uint32_t total_energy_out; // total energy output from the battery pack in mWh
} BatteryMetadata;

//ble messages to the website for bettter identificationof battery pack and everything
typedef enum {
    BLE_MSG_BATTERY_STATUS = 0x01, // battery status message
    BLE_MSG_CELL_VOLTAGES = 0x02, // cell voltage message
    BLE_MSG_HEALTH = 0x03, // battery health message
    BLE_MSG_SETTINGS = 0x04, // battery metadata message
    BLE_MSG_ACK = 0x05, // acknowledgment message
    BLE_MSG_ERROR = 0x06, // error message
} BLE_MessageType;

// wrapping up the message chyat

typedef struct {
    BLE_MessageType msg_type; // type of the message
    uint8_t payload[128]; // payload of the message
    uint16_t payload_length; // length of the payload
    uint32_t timestamp; // reading since the boot of micro controller in ms
} BLE_Message;

//battery stat
typedef struct {
    uint16_t pack_voltage_mv; // total voltage of the battery pack in millivolts mV
    int16_t current_ma; // current of the battery pack in milliamps mA
    uint8_t soc_percent; // state of charge of the battery pack in percentage %
    uint8_t health_percent; // battery health in percentage %
} BLE_BatteryStatusMsg;

//cell volt

typedef struct {
    uint8_t cell_count; // 1 to 12
    uint16_t voltage_mv[12]; // voltage reading in millivolts mV
} BLE_CellVoltagesMsg;

// health of messages
typedef struct {
    uint8_t health_percent; // battery health in percentage %
    uint32_t charge_cycles; // number of charge cycles the battery has gone through
    uint32_t discharge_cycles; // number of discharge cycles the battery has gone through
    uint32_t total_energy_in; // total energy input to the battery pack in mWh
    uint32_t total_energy_out; // total energy output from the battery pack in mWh
} BLE_HealthMsg;

//settings msg
typedef struct {
    uint8_t setting_id; // setting identifier
    uint8_t operation; // operation to be performed (e.g., read, write)
    uint32_t value; // value associated with the setting
    char string_value[64]; // string value associated with the setting (if applicable)
} BLE_SettingsMsg;


// battery state

//operational state
typedef enum {
    BATTERY_STATE_UNKOWN = 0, // unknown state
    BATTERY_STATE_IDLE = 1, // battery is idle
    BATTERY_STATE_CHARGING = 2, // battery is charging
    BATTERY_STATE_DISCHARGING = 3, // battery is discharging
    BATTERY_STATE_FAULT = 4, // battery is in fault state
    BATTERY_STATE_BALANCING = 5, // battery is balancing
} BatteryState;

//health bif
typedef enum {
    BATTERY_HEALTH_UNKOWN = 0, // unknown health
    BATTERY_HEALTH_EXCELLENT = 1, // battery health is excellent
    BATTERY_HEALTH_GOOD = 2, // battery health is good
    BATTERY_HEALTH_FAIR = 3, // battery health is fair
    BATTERY_HEALTH_WARNING = 4, // battery health is warning
    BATTERY_HEALTH_CRITICAL = 5, // battery health is critical
} BatteryHealth;

// error codes
typedef enum {
    ERROR_NONE = 0x00, // no error
    ERROR_OVER_VOLTAGE = 0x01, // overvoltage error
    ERROR_UNDER_VOLTAGE = 0x02, // undervoltage error
    ERROR_OVER_CURRENT = 0x03, // overcurrent error
    ERROR_OVER_TEMP = 0x04, // overtemperature error
    ERROR_ADC_FAILURE = 0x05, // ADC failure error
    ERROR_I2C_FAILURE = 0x06, // I2C failure error
    ERROR_BLE_FAILURE = 0x07, // BLE failure error
    ERROR_STORAGE_FAILURE = 0x08, // storage failure error
    ERROR_CALIBRATION_NEEDED = 0x09, // calibration needed error
    ERROR_UNKNOWN = 0xFF // unknown error
} ErrorCode;

//task and messagte thins
// messaged passed between esp32 
typedef struct {
    uint8_t message_type; // type of the message
    uint8_t priority; // priority of the message
    uint32_t data; // data associated with the message
    void* ptr_data; // pointer to additional data (if needed)
    uint32_t timestamp; // reading since the boot of micro controller in ms
} TaskMessage;

// circle buf
typedef struct {
    uint32_t timestamp; // reading since the boot of micro controller in ms
    uint8_t log_level; // log level (e.g., info, warning, error)
    char message[256]; // log message
} LogEntry;

//calibration and config stuct

//adc calib param
typedef struct {
    float factor; // calibration factor
    float offset; // calibration offset
    uint32_t last_calibrated; // timestamp of last calibration in ms
} ADCCalibration;

// cur sens calib param

typedef struct {
    float sensitivity; // calibration sensitivity
    float offset; // calibration offset
    uint32_t last_calibrated; // timestamp of last calibration in ms
} CurrentSensorCalibration;

//syys sconfig
typedef struct {
    BatteryMetadata battery_meta; // battery metadata
    ADCCalibration adc_calib; // ADC calibration parameters
    CurrentSensorCalibration current_calib; // current sensor calibration parameters
    uint8_t cell_count; // number of cells in the battery pack
    uint16_t soc_capacity_mah; // state of charge capacity in mAh
    uint32_t last_updated; // timestamp of last configuration update in ms
} SystemConfig;

//macros

#define BATTERY_STATUS_INIT { \
    .pack_voltage = 0.0f, \
    .pack_current = 0.0f, \
    .state_of_charge = 0.0f, \
    .battery_health = 0, \
    .charge_cycles = 0, \
    .discharge_cycles = 0, \
    .timestamp = 0 \
}

#define CELL_VOLTAGE_INIT { \
    .cell_number = 0, \
    .voltage = 0.0f, \
    .timestamp = 0 \
}

#define BLE_MESSAGE_INIT { \
    .msg_type = BLE_MSG_BATTERY_STATUS, \
    .payload = {0}, \
    .timestamp = 0 \
}

#endif // TYPES_H