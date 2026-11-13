#include "adg1608.h"
#include "esp_log.h"

static const char *TAG = "ADG1608";

esp_err_t adg1608_init(adg1608_t *mux, tca9535_t *tca)
{
    mux->tca = tca;
    ESP_ERROR_CHECK(tca9535_write_pin(tca, ADG_EN_PIN, false));
    ESP_LOGI(TAG, "ADG1608 init OK");
    return ESP_OK;
}

esp_err_t adg1608_select(adg1608_t *mux, uint8_t channel)
{
    if (channel > 7) return ESP_ERR_INVALID_ARG;

    bool a0 = channel & 0x01;
    bool a1 = (channel >> 1) & 0x01;
    bool a2 = (channel >> 2) & 0x01;

    ESP_ERROR_CHECK(tca9535_write_pin(mux->tca, ADG_A0_PIN, a0));
    ESP_ERROR_CHECK(tca9535_write_pin(mux->tca, ADG_A1_PIN, a1));
    ESP_ERROR_CHECK(tca9535_write_pin(mux->tca, ADG_A2_PIN, a2));
    ESP_ERROR_CHECK(tca9535_write_pin(mux->tca, ADG_EN_PIN, true));

    ESP_LOGI(TAG, "Selected channel %d", channel);
    return ESP_OK;
}

esp_err_t adg1608_disable(adg1608_t *mux)
{
    return tca9535_write_pin(mux->tca, ADG_EN_PIN, false);
}
