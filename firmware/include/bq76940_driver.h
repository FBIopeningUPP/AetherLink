#pragma once

#include "esp_err.h"
#include <cstdint>

esp_err_t afe_init();
esp_err_t afe_get_cell_voltages(uint16_t* cell_voltages_mv, uint8_t max_cells);