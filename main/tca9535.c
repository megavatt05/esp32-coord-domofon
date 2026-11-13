#include "tca9535.h"
#include "esp_log.h"

static const char *TAG = "TCA9535";

static esp_err_t tca_write_reg(tca9535_t *dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(dev->dev, buf, 2, -1);
}

static esp_err_t tca_read_reg(tca9535_t *dev, uint8_t reg, uint8_t *val)
{
    return i2c_master_transmit_receive(dev->dev, &reg, 1, val, 1, -1);
}

esp_err_t tca9535_init(tca9535_t *dev, i2c_master_bus_handle_t bus)
{
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TCA9535_I2C_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &cfg, &dev->dev));

    // All pins as outputs
    ESP_ERROR_CHECK(tca_write_reg(dev, TCA9535_REG_CONFIG0, 0x00));
    ESP_ERROR_CHECK(tca_write_reg(dev, TCA9535_REG_CONFIG1, 0x00));

    dev->output0_shadow = 0x00;
    dev->output1_shadow = 0x00;
    ESP_ERROR_CHECK(tca_write_reg(dev, TCA9535_REG_OUTPUT0, 0x00));
    ESP_ERROR_CHECK(tca_write_reg(dev, TCA9535_REG_OUTPUT1, 0x00));

    ESP_LOGI(TAG, "TCA9535 init OK (addr 0x%02X)", TCA9535_I2C_ADDR);
    return ESP_OK;
}

esp_err_t tca9535_write_port(tca9535_t *dev, uint8_t port, uint8_t value)
{
    if (port == 0) {
        dev->output0_shadow = value;
        return tca_write_reg(dev, TCA9535_REG_OUTPUT0, value);
    } else {
        dev->output1_shadow = value;
        return tca_write_reg(dev, TCA9535_REG_OUTPUT1, value);
    }
}

esp_err_t tca9535_read_port(tca9535_t *dev, uint8_t port, uint8_t *value)
{
    uint8_t reg = (port == 0) ? TCA9535_REG_INPUT0 : TCA9535_REG_INPUT1;
    return tca_read_reg(dev, reg, value);
}

esp_err_t tca9535_write_pin(tca9535_t *dev, uint8_t pin, bool level)
{
    uint8_t port = pin / 8;
    uint8_t bit  = pin % 8;
    uint8_t *shadow = (port == 0) ? &dev->output0_shadow : &dev->output1_shadow;

    if (level) *shadow |= (1 << bit);
    else       *shadow &= ~(1 << bit);

    return tca9535_write_port(dev, port, *shadow);
}
