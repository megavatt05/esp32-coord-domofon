#pragma once

#include "esp_err.h"
#include "tca95x5.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ADG1608 8:1 Analog Multiplexer controlled via TCA9535
 *
 * Pin mapping on TCA9535:
 *   A0 = P0.6
 *   A1 = P0.7
 *   A2 = P1.0
 *   EN = P1.1
 */

#define ADG1608_A0_PIN   6
#define ADG1608_A1_PIN   7
#define ADG1608_A2_PIN   8
#define ADG1608_EN_PIN   9

typedef struct {
    i2c_dev_t *tca;          // pointer to tca95x5 device
} adg1608_t;

esp_err_t adg1608_init(adg1608_t *mux, i2c_dev_t *tca);
esp_err_t adg1608_select(adg1608_t *mux, uint8_t channel);  // 0..7
esp_err_t adg1608_disable(adg1608_t *mux);

#ifdef __cplusplus
}
#endif
