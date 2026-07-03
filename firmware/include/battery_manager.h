#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

//initilaztion

bool battery_manager_init(void);
void battery_manager_deinit(void);

//data read
//read adc value 
int32_t battery_read_cell_adc(uint8_t cell_number);
//read all cell ovltage
bool battery_read_all_cells(CellArray *cell_array);
//read current
float battery_read_current(void);
//read whole volt
float battery_read_pack_voltage(void);


//voltage calib

//convert raw adc to volt
float battery_adc_to_voltage(uint16_t adc_value);

//calibrate adc for s single cell
bool battery_calibrate_cell(uint8_t cell_number, float refrence_voltage);

//calibrate current sensor
bool battery_calibrate_current(void);

// sate of charge calcl soc
//voltage to soc mapping
float battery_calculate_soc_from_voltage(float pack_voltage);
//track charge and discharge
float battery_calculate_soc_coulomb_counting(int16_t current_ma, uint32_t time_ms);
//Set Soc to known value percentage kinda thing
void battery_set_soc(float soc_percentage);
//get current staate
float battery_get_soc(void);

//battery hrealth
//calculate health perc cycle count an dcapacity fade
uint8_t battery_calculate_health(void);

//heatlth classiciation
BatteryHealth battery_get_health_state(void);

//discharge cycels
uint32_t battery_get_discharge_cycles(void);

//charge cycles
uint32_t battery_get_charge_cycles(void);

//increment charge cycle
void battery_increment_charge_cycle(void);

//increment discharge cycle
void battery_increment_discharge_cycle(void);


// check the cell balancing and monitingr
//detect 
bool battery_check_cell_imbalance(const CellArray *cell_array);
//get voltage dif high low
float battery_get_cell_voltage_spread(const CellArray *cell_array);

//find cell with min volt
uint8_t battery_get_min_voltage_cell(const CellArray *cell_array);

//max volt
uint8_t battery_get_max_voltage_cell(const CellArray *cell_array);

//error detecth and safe
//over volt
bool battery_check_overvoltage(float cell_voltage);
//under volt
bool battery_check_undervoltage(float cell_voltage);
//over current
bool battery_check_overcurrent(float current_ma);
//safe check
ErrorCode battery_safety_check(const BatteryStatus *status);
//get last error 
ErrorCode battery_get_last_error(void);
//clear errors
void battery_clear_errors(void);

//batterystatus 
//whole shit battery snapchat
bool battery_get_status(BatteryStatus *status);
// cell voltage array
bool battery_get_cell_voltage(CellArray *cell_array);
//charging input shit
uint32_t battery_get_total_energy_in(void);
//out shit
uint32_t battery_get_total_energy_out(void);
//energy efficenty
uint8_t battery_get_efficiency(void);

//battery state machine
//current state
BatteryState battery_get_state(void);

// determine battery state based on voltage and current 
BatteryState battery_update_state(float pack_voltage, float pack_current);

//check charge 
bool battery_is_charging(void);
//check dicharge
bool battery_is_discharging(void);
//check fault
bool battery_is_faulted(void);


///debug
void battery_print_status(void);

//print cell volt
void battery_print_cell_voltages(void);

//health
void battery_print_health(void);

//test

bool battery_run_selftest(void);

#endif // BATTERY_MANAGER_H