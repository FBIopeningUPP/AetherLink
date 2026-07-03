#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

bool battery_manager_init(void);
void battery_manager_deinit(void);

// Keep API compatible
int32_t battery_read_cell_adc(uint8_t cell_number);
bool battery_read_all_cells(CellArray *cell_array);

float battery_read_current(void);
float battery_read_pack_voltage(void);

float battery_adc_to_voltage(uint16_t adc_value);
bool battery_calibrate_cell(uint8_t cell_number, float refrence_voltage);
bool battery_calibrate_current(void);

float battery_calculate_soc_from_voltage(float pack_voltage);
float battery_calculate_soc_coulomb_counting(int16_t current_ma, uint32_t time_ms);
void battery_set_soc(float soc_percentage);
float battery_get_soc(void);

uint8_t battery_calculate_health(void);
BatteryHealth battery_get_health_state(void);

uint32_t battery_get_discharge_cycles(void);
uint32_t battery_get_charge_cycles(void);
void battery_increment_charge_cycle(void);
void battery_increment_discharge_cycle(void);

bool battery_check_cell_imbalance(const CellArray *cell_array);
float battery_get_cell_voltage_spread(const CellArray *cell_array);
uint8_t battery_get_min_voltage_cell(const CellArray *cell_array);
uint8_t battery_get_max_voltage_cell(const CellArray *cell_array);

bool battery_check_overvoltage(float cell_voltage);
bool battery_check_undervoltage(float cell_voltage);
bool battery_check_overcurrent(float current_ma);
ErrorCode battery_safety_check(const BatteryStatus *status);
ErrorCode battery_get_last_error(void);
void battery_clear_errors(void);

bool battery_get_status(BatteryStatus *status);
bool battery_get_cell_voltage(CellArray *cell_array);

uint32_t battery_get_total_energy_in(void);
uint32_t battery_get_total_energy_out(void);
uint8_t battery_get_efficiency(void);

BatteryState battery_get_state(void);
BatteryState battery_update_state(float pack_voltage, float pack_current);

bool battery_is_charging(void);
bool battery_is_discharging(void);
bool battery_is_faulted(void);

void battery_print_status(void);
void battery_print_cell_voltages(void);
void battery_print_health(void);

bool battery_run_selftest(void);

#endif // BATTERY_MANAGER_H