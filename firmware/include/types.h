#pragma once
#include <cstdint>

struct __attribute__((packed)) BatteryState {
    uint16_t cell_voltages_mv[16];
    uint32_t pack_voltage_mv;
    uint16_t max_cell_delta_mv;
    uint8_t soc_percentage;

    uint8_t soh_percentage;
    uint8_t pack_status;
    uint16_t charge_cycles;
    uint16_t discharge_cycles;
};