#pragma once

#include "esp_err.h"
#include "types.h"

esp_err_t battery_manager_init();
esp_err_t battery_manager_update();
const BatteryState* battery_manager_get_state();