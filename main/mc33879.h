#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include <stdbool.h>

// TODO: уточнить точные GPIO по плате
#define MC33879_CS_GPIO     19
#define MC33879_EN_GPIO     20
#define MC33879_IN5_GPIO    27      // RS-ON (также используется для двери)
#define MC33879_IN6_GPIO    21

#define MC33879_SPI_HOST    SPI2_HOST
#define MC33879_MOSI_GPIO   23
#define MC33879_MISO_GPIO   19
#define MC33879_SCLK_GPIO   18

typedef struct {
    spi_device_handle_t spi;
    gpio_num_t cs_gpio;
    gpio_num_t en_gpio;
    gpio_num_t in5_gpio;
    gpio_num_t in6_gpio;
    uint8_t output_shadow;
} mc33879_t;

esp_err_t mc33879_init(mc33879_t *dev);
esp_err_t mc33879_enable(mc33879_t *dev, bool enable);
esp_err_t mc33879_set_outputs(mc33879_t *dev, uint8_t outputs);
esp_err_t mc33879_set_channel(mc33879_t *dev, uint8_t channel, bool on);
esp_err_t mc33879_read_status(mc33879_t *dev, uint16_t *status);
void mc33879_set_in5(mc33879_t *dev, bool level);
void mc33879_set_in6(mc33879_t *dev, bool level);
