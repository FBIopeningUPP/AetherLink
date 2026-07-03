#pragma once 
#include "esp_err.h"
#include "types.h"

esp_err_t ble_manager_init();

void ble_manager_update_telemetry(const BatteryState* state);