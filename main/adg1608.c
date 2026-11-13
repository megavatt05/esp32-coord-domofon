#include "adg1608.h"
#include "esp_log.h"

static const char *TAG = "ADG1608";

esp_err_t adg1608_init(adg1608_t *mux, i2c_dev_t *tca)
{
    if (!mux || !tca) return ESP_ERR_INVALID_ARG;
    mux->tca = tca;

    // EN = 0 (disabled)
    ESP_ERROR_CHECK(tca95x5_set_level(tca, ADG1608_EN_PIN, 0));
    ESP_LOGI(TAG, "ADG1608 init OK");
    return ESP_OK;
}

esp_err_t adg1608_select(adg1608_t *mux, uint8_t channel)
{
    if (!mux || channel > 7) return ESP_ERR_INVALID_ARG;

    bool a0 = channel & 0x01;
    bool a1 = (channel >> 1) & 0x01;
    bool a2 = (channel >> 2) & 0x01;

    ESP_ERROR_CHECK(tca95x5_set_level(mux->tca, ADG1608_A0_PIN, a0));
    ESP_ERROR_CHECK(tca95x5_set_level(mux->tca, ADG1608_A1_PIN, a1));
    ESP_ERROR_CHECK(tca95x5_set_level(mux->tca, ADG1608_A2_PIN, a2));
    ESP_ERROR_CHECK(tca95x5_set_level(mux->tca, ADG1608_EN_PIN, 1));

    ESP_LOGD(TAG, "Selected channel %u", channel);
    return ESP_OK;
}

esp_err_t adg1608_disable(adg1608_t *mux)
{
    if (!mux) return ESP_ERR_INVALID_ARG;
    return tca95x5_set_level(mux->tca, ADG1608_EN_PIN, 0);
}
