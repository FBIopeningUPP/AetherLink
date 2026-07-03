#pragma once 
#include <stdint.h>
#include <stdbool.h>

#ifndef BATTERY_MAX_CELLS
#define BATTERY_MAX_CELLS 12
#endif

typedef struct {
    float cell_voltages[BATTERY_MAX_CELLS];
    uint8_t cell_count;

    float pack_voltage;
    float min_cell_voltage;
    float max_cell_voltage;
    float cell_delta;
    float soc_percent;

    bool current_valid;
    float pack_current;

    uint32_t fault_flags;
} battery_snapshot_t;
