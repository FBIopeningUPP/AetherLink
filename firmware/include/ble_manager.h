#ifndef AETHERLINK_BLE_MANAGER_H
#define AETHERLINK_BLE_MANAGER_H

#include <esp_err.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Init NimBLE stack, configure GATT server, and begin advertising
esp_err_t ble_manager_init(void);

// Notify connected clients with updated pack telemetry
void ble_manager_update_telemetry(const BatteryState *state);

#ifdef __cplusplus
}
#endif

#endif // AETHERLINK_BLE_MANAGER_H