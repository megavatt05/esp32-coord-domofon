#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t door_init(void);
esp_err_t door_open(uint32_t duration_ms);

void door_1wire_write_bit(bool bit);
bool door_1wire_read_bit(void);
bool door_1wire_reset(void);

#ifdef __cplusplus
}
#endif
