#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    spi_host_device_t spi_host;
    gpio_num_t pin_cs;
    gpio_num_t pin_sclk;
    gpio_num_t pin_mosi;
    gpio_num_t pin_miso;
    gpio_num_t pin_en;       // -1 if not used
    gpio_num_t pin_in5;      // -1 if not used (direct control OUT5)
    gpio_num_t pin_in6;      // -1 if not used (direct control OUT6)
    uint32_t spi_clock_hz;
} mc33879_config_t;

typedef struct mc33879_dev_t* mc33879_handle_t;

esp_err_t mc33879_init(mc33879_handle_t *handle, const mc33879_config_t *config);
esp_err_t mc33879_deinit(mc33879_handle_t handle);
esp_err_t mc33879_enable(mc33879_handle_t handle, bool enable);
esp_err_t mc33879_set_outputs(mc33879_handle_t handle, uint8_t outputs);
esp_err_t mc33879_set_channel(mc33879_handle_t handle, uint8_t channel, bool on); // 1..8
esp_err_t mc33879_read_status(mc33879_handle_t handle, uint16_t *status);
esp_err_t mc33879_set_in5(mc33879_handle_t handle, bool level);
esp_err_t mc33879_set_in6(mc33879_handle_t handle, bool level);

#ifdef __cplusplus
}
#endif
