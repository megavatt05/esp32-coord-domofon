#include "mc33879.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MC33879";

static esp_err_t mc33879_spi_transfer(mc33879_t *dev, uint16_t tx_data, uint16_t *rx_data)
{
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = &tx_data,
        .rx_buffer = rx_data,
    };

    gpio_set_level(dev->cs_gpio, 0);
    esp_err_t ret = spi_device_transmit(dev->spi, &t);
    gpio_set_level(dev->cs_gpio, 1);
    return ret;
}

esp_err_t mc33879_init(mc33879_t *dev)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << MC33879_CS_GPIO) |
                        (1ULL << MC33879_EN_GPIO) |
                        (1ULL << MC33879_IN5_GPIO) |
                        (1ULL << MC33879_IN6_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));

    gpio_set_level(MC33879_CS_GPIO, 1);
    gpio_set_level(MC33879_EN_GPIO, 0);
    gpio_set_level(MC33879_IN5_GPIO, 0);
    gpio_set_level(MC33879_IN6_GPIO, 0);

    spi_bus_config_t buscfg = {
        .mosi_io_num = MC33879_MOSI_GPIO,
        .miso_io_num = MC33879_MISO_GPIO,
        .sclk_io_num = MC33879_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(MC33879_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1000000,
        .mode = 1,
        .spics_io_num = -1,
        .queue_size = 1,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(MC33879_SPI_HOST, &devcfg, &dev->spi));

    dev->cs_gpio = MC33879_CS_GPIO;
    dev->en_gpio = MC33879_EN_GPIO;
    dev->in5_gpio = MC33879_IN5_GPIO;
    dev->in6_gpio = MC33879_IN6_GPIO;
    dev->output_shadow = 0;

    ESP_ERROR_CHECK(mc33879_enable(dev, true));
    vTaskDelay(pdMS_TO_TICKS(5));
    ESP_ERROR_CHECK(mc33879_set_outputs(dev, 0x00));

    ESP_LOGI(TAG, "MC33879 init OK");
    return ESP_OK;
}

esp_err_t mc33879_enable(mc33879_t *dev, bool enable)
{
    gpio_set_level(dev->en_gpio, enable ? 1 : 0);
    return ESP_OK;
}

esp_err_t mc33879_set_outputs(mc33879_t *dev, uint8_t outputs)
{
    uint16_t cmd = outputs;
    uint16_t status = 0;
    esp_err_t ret = mc33879_spi_transfer(dev, cmd, &status);
    if (ret == ESP_OK) {
        dev->output_shadow = outputs;
    }
    return ret;
}

esp_err_t mc33879_set_channel(mc33879_t *dev, uint8_t channel, bool on)
{
    if (channel < 1 || channel > 8) return ESP_ERR_INVALID_ARG;

    uint8_t mask = 1 << (channel - 1);
    if (on) dev->output_shadow |= mask;
    else    dev->output_shadow &= ~mask;

    return mc33879_set_outputs(dev, dev->output_shadow);
}

esp_err_t mc33879_read_status(mc33879_t *dev, uint16_t *status)
{
    return mc33879_spi_transfer(dev, 0x0000, status);
}

void mc33879_set_in5(mc33879_t *dev, bool level)
{
    gpio_set_level(dev->in5_gpio, level);
}

void mc33879_set_in6(mc33879_t *dev, bool level)
{
    gpio_set_level(dev->in6_gpio, level);
}
