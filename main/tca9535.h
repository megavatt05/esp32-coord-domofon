#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include <stdbool.h>

#define TCA9535_I2C_ADDR      0x20

#define TCA9535_REG_INPUT0    0x00
#define TCA9535_REG_INPUT1    0x01
#define TCA9535_REG_OUTPUT0   0x02
#define TCA9535_REG_OUTPUT1   0x03
#define TCA9535_REG_CONFIG0   0x06
#define TCA9535_REG_CONFIG1   0x07

typedef struct {
    i2c_master_dev_handle_t dev;
    uint8_t output0_shadow;
    uint8_t output1_shadow;
} tca9535_t;

esp_err_t tca9535_init(tca9535_t *dev, i2c_master_bus_handle_t bus);
esp_err_t tca9535_write_port(tca9535_t *dev, uint8_t port, uint8_t value);
esp_err_t tca9535_read_port(tca9535_t *dev, uint8_t port, uint8_t *value);
esp_err_t tca9535_write_pin(tca9535_t *dev, uint8_t pin, bool level);
