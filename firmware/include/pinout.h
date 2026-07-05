#pragma once

//ESP32
#define PIN_I2C_SDA     4
#define PIN_I2C_SCL     5

#define PIN_BQ_ALERT    2


#define PIN_SOC_LED1_25     9
#define PIN_SOC_LED1_50     10
#define PIN_SOC_LED1_75     18
#define PIN_SOC_LED1_100    19

#define PIN_STATUS_LED      3

#define LED_ACTIVE_HIGH     1
// active-low

#define BATTERY_MAX_CELLS 12
#define BATTERY_MIN_CELLS 3
#define BATTERY_DEFAULT_CELLS 4

// BQ7694003 Net Mapping
#define NET_BQ_VC0 "BATT_TAP_0_BNEG"
#define NET_BQ_VC1 "BATT_TAP_1"
#define NET_BQ_VC2 "BATT_TAP_2"
#define NET_BQ_VC3 "BATT_TAP_3"
#define NET_BQ_VC4 "BATT_TAP_4"
#define NET_BQ_VC5 "BATT_TAP_5"
#define NET_BQ_VC6 "BATT_TAP_6"
#define NET_BQ_VC7 "BATT_TAP_7"
#define NET_BQ_VC8 "BATT_TAP_8"
#define NET_BQ_VC9 "BATT_TAP_9"
#define NET_BQ_VC10 "BATT_TAP_10"
#define NET_BQ_VC11 "BATT_TAP_11"
#define NET_BQ_VC12 "BATT_TAP_12_BPOS"

#define NET_BQ_BAT "PACK_BAT_POS"
// sense/supply per datasheet
#define NET_BQ_VSS "PACK_BAT_NEG"
// refrence 

#define NET_BQ_REGOUT "BQ_REGOUT_3V3"

#define NET_BQ_SDA "I2C_SDA"
#define NET_BQ_SCL "I2C_SCL"
#define NET_BQ_ALERT "BQ_ALERT_N"
// active-low open-drain

#define NET_BQ_CHG "BQ_CHG_CTRL"
#define NET_BQ_DSG "BQ_DSG_CTRL"
