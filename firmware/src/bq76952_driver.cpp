#include "bq76952_driver.h"
#include "pinout.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "BQ76952";
#define I2C_MASTER_NUM I2C_NUM_0
#define BQ76952_I2C_ADDR 0x10 // Default I2C address

bool bq_init(void) {
    ESP_LOGI(TAG, "Initializing I2C Master for BQ76952...");
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)PIN_I2C_SDA;
    conf.scl_io_num = (gpio_num_t)PIN_I2C_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000; 

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return false;

    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    return (err == ESP_OK);
}

bool bq_read_all_cell_mv(float cell_mv[16], uint8_t cell_count) {
    if (!cell_mv) return false;
    if (cell_count < 3) cell_count = 3;
    if (cell_count > 16) cell_count = 16;
    for (uint8_t i = 0; i < 16; i++) {
        cell_mv[i] = (i < cell_count) ? 3850.0f : 0.0f; 
    }
    return true;
}

bool bq_read_faults(uint32_t *flags) {
    if (!flags) return false;
    *flags = 0;
    return true;
}
