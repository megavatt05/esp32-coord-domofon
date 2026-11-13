#include "mc33879.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "MC33879";

struct mc33879_dev_t {
    spi_device_handle_t spi;
    gpio_num_t pin_cs;
    gpio_num_t pin_en;
    gpio_num_t pin_in5;
    gpio_num_t pin_in6;
    uint8_t output_shadow;
};

static esp_err_t mc33879_transfer(mc33879_handle_t dev, uint16_t tx, uint16_t *rx)
{
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = &tx,
        .rx_buffer = rx,
    };

    gpio_set_level(dev->pin_cs, 0);
    esp_err_t ret = spi_device_transmit(dev->spi, &t);
    gpio_set_level(dev->pin_cs, 1);
    return ret;
}

esp_err_t mc33879_init(mc33879_handle_t *out_handle, const mc33879_config_t *cfg)
{
    ESP_RETURN_ON_FALSE(out_handle && cfg, ESP_ERR_INVALID_ARG, TAG, "invalid arg");

    mc33879_handle_t dev = calloc(1, sizeof(struct mc33879_dev_t));
    ESP_RETURN_ON_FALSE(dev, ESP_ERR_NO_MEM, TAG, "no mem");

    dev->pin_cs  = cfg->pin_cs;
    dev->pin_en  = cfg->pin_en;
    dev->pin_in5 = cfg->pin_in5;
    dev->pin_in6 = cfg->pin_in6;
    dev->output_shadow = 0;

    // GPIO setup
    uint64_t mask = (1ULL << cfg->pin_cs);
    if (cfg->pin_en  >= 0) mask |= (1ULL << cfg->pin_en);
    if (cfg->pin_in5 >= 0) mask |= (1ULL << cfg->pin_in5);
    if (cfg->pin_in6 >= 0) mask |= (1ULL << cfg->pin_in6);

    gpio_config_t io = {
        .pin_bit_mask = mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));

    gpio_set_level(cfg->pin_cs, 1);
    if (cfg->pin_en  >= 0) gpio_set_level(cfg->pin_en, 0);
    if (cfg->pin_in5 >= 0) gpio_set_level(cfg->pin_in5, 0);
    if (cfg->pin_in6 >= 0) gpio_set_level(cfg->pin_in6, 0);

    // SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = cfg->pin_mosi,
        .miso_io_num = cfg->pin_miso,
        .sclk_io_num = cfg->pin_sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(cfg->spi_host, &buscfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = cfg->spi_clock_hz ? cfg->spi_clock_hz : 1000000,
        .mode = 1,
        .spics_io_num = -1,          // manual CS
        .queue_size = 1,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(cfg->spi_host, &devcfg, &dev->spi));

    // Enable chip
    if (cfg->pin_en >= 0) {
        gpio_set_level(cfg->pin_en, 1);
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    // All outputs off
    ESP_ERROR_CHECK(mc33879_set_outputs(dev, 0x00));

    *out_handle = dev;
    ESP_LOGI(TAG, "MC33879 init OK");
    return ESP_OK;
}

esp_err_t mc33879_deinit(mc33879_handle_t handle)
{
    if (!handle) return ESP_ERR_INVALID_ARG;
    mc33879_set_outputs(handle, 0x00);
    if (handle->pin_en >= 0) gpio_set_level(handle->pin_en, 0);
    spi_bus_remove_device(handle->spi);
    free(handle);
    return ESP_OK;
}

esp_err_t mc33879_enable(mc33879_handle_t handle, bool enable)
{
    if (!handle || handle->pin_en < 0) return ESP_ERR_INVALID_ARG;
    gpio_set_level(handle->pin_en, enable ? 1 : 0);
    return ESP_OK;
}

esp_err_t mc33879_set_outputs(mc33879_handle_t handle, uint8_t outputs)
{
    if (!handle) return ESP_ERR_INVALID_ARG;
    uint16_t status = 0;
    esp_err_t ret = mc33879_transfer(handle, outputs, &status);
    if (ret == ESP_OK) handle->output_shadow = outputs;
    return ret;
}

esp_err_t mc33879_set_channel(mc33879_handle_t handle, uint8_t channel, bool on)
{
    if (!handle || channel < 1 || channel > 8) return ESP_ERR_INVALID_ARG;

    uint8_t mask = 1 << (channel - 1);
    if (on) handle->output_shadow |= mask;
    else    handle->output_shadow &= ~mask;

    return mc33879_set_outputs(handle, handle->output_shadow);
}

esp_err_t mc33879_read_status(mc33879_handle_t handle, uint16_t *status)
{
    if (!handle || !status) return ESP_ERR_INVALID_ARG;
    return mc33879_transfer(handle, 0x0000, status);
}

esp_err_t mc33879_set_in5(mc33879_handle_t handle, bool level)
{
    if (!handle || handle->pin_in5 < 0) return ESP_ERR_INVALID_ARG;
    gpio_set_level(handle->pin_in5, level);
    return ESP_OK;
}

esp_err_t mc33879_set_in6(mc33879_handle_t handle, bool level)
{
    if (!handle || handle->pin_in6 < 0) return ESP_ERR_INVALID_ARG;
    gpio_set_level(handle->pin_in6, level);
    return ESP_OK;
}
