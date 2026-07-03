#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "bq76940_driver.h"
#include "battery_manager.h"
#include "ble_manager.h"

static const char *TAG = "AetherLink_Main";

static void telemetry_task(void *pvParameters) {
    ESP_LOGI(TAG, "Telemetry loop started, Polling every 1000ms.");

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);

    while (true) {
        esp_err_t err = battery_manager_update();

        if (err == ESP_OK) {
            const BatteryState* latest_state = battery_manager_get_state();
            ble_manager_update_telemetry(latest_state);
        } else {
            ESP_LOGW(TAG, "Skipping BLE update due to AFE read error.");
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Booting AetherLink BMS...");

    esp_err_t err = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(afe_init());
    ESP_ERROR_CHECK(battery_manager_init());
    ESP_ERROR_CHECK(ble_manager_init());

    xTaskCreatePinnedToCore(
        telemetry_task,
        "telemetry_task",
        4096,
        NULL,
        5,
        NULL,
        0
    );
}