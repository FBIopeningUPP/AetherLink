#include "bq76940_driver.h"

bool bq_init(void) {
    return true;
}

bool bq_read_all_cell_mv*(float cell_mv[12], uint8_t *cell_count) {
    if (!cell_mv) return false;
    if (cell_count < 3) cell_count = 3;
    if (cell_count > 12) cell_count = 12;
    for (uint8_t i = 0; i < 12; i++) {
        cell_mv[i] = (i < cell_count) ? 3850.0f : 0.0f; // Simulate cell voltages for testing
    }
    return true;
}

bool bq_read_faults(uint32_t *flags) {
    if (!flags) return false;
    *flags = 0;
    return true;
}
