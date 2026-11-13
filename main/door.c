#include "door.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"

static const char *TAG = "DOOR";

#define DOOR_OPEN_GPIO   27      // RS-ON → 1W-CTRL → OpenD
#define DOOR_1WRX_GPIO   12
#define DOOR_1WTX_GPIO   15

esp_err_t door_init(void)
{
    gpio_config_t open_conf = {
        .pin_bit_mask = (1ULL << DOOR_OPEN_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&open_conf));
    gpio_set_level(DOOR_OPEN_GPIO, 0);

    gpio_config_t rx_conf = {
        .pin_bit_mask = (1ULL << DOOR_1WRX_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&rx_conf));

    gpio_config_t tx_conf = {
        .pin_bit_mask = (1ULL << DOOR_1WTX_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&tx_conf));
    gpio_set_level(DOOR_1WTX_GPIO, 1);

    ESP_LOGI(TAG, "Door init OK (Open=GPIO%d)", DOOR_OPEN_GPIO);
    return ESP_OK;
}

esp_err_t door_open(uint32_t duration_ms)
{
    ESP_LOGI(TAG, "Opening door for %lu ms", (unsigned long)duration_ms);
    gpio_set_level(DOOR_OPEN_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    gpio_set_level(DOOR_OPEN_GPIO, 0);
    return ESP_OK;
}

bool door_1wire_reset(void)
{
    gpio_set_level(DOOR_1WTX_GPIO, 0);
    ets_delay_us(480);
    gpio_set_level(DOOR_1WTX_GPIO, 1);
    ets_delay_us(70);
    bool presence = !gpio_get_level(DOOR_1WRX_GPIO);
    ets_delay_us(410);
    return presence;
}

void door_1wire_write_bit(bool bit)
{
    gpio_set_level(DOOR_1WTX_GPIO, 0);
    ets_delay_us(bit ? 6 : 60);
    gpio_set_level(DOOR_1WTX_GPIO, 1);
    ets_delay_us(bit ? 64 : 10);
}

bool door_1wire_read_bit(void)
{
    gpio_set_level(DOOR_1WTX_GPIO, 0);
    ets_delay_us(6);
    gpio_set_level(DOOR_1WTX_GPIO, 1);
    ets_delay_us(9);
    bool bit = gpio_get_level(DOOR_1WRX_GPIO);
    ets_delay_us(55);
    return bit;
}
