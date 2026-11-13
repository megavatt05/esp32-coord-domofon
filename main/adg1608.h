#pragma once

#include "tca9535.h"
#include "esp_err.h"

#define ADG_A0_PIN   6   // P0.6
#define ADG_A1_PIN   7   // P0.7
#define ADG_A2_PIN   8   // P1.0
#define ADG_EN_PIN   9   // P1.1

typedef struct {
    tca9535_t *tca;
} adg1608_t;

esp_err_t adg1608_init(adg1608_t *mux, tca9535_t *tca);
esp_err_t adg1608_select(adg1608_t *mux, uint8_t channel);
esp_err_t adg1608_disable(adg1608_t *mux);
