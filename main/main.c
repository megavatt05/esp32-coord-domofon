#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "i2cdev.h"
#include "tca95x5.h"
#include "adg1608.h"
#include "mc33879.h"
#include "door.h"

static const char *TAG = "DOMOFON";

// I2C
#define I2C_SCL_GPIO     32
#define I2C_SDA_GPIO     33

// Line detect
#define LINE_DO1_GPIO    36
#define LINE_DO2_GPIO    37

// Shared SPI bus (one MOSI/MISO/SCLK for every SPI chip on the board)
#define SPI_MOSI_GPIO    23
#define SPI_MISO_GPIO    19
#define SPI_SCLK_GPIO    18

// MC33879 device pins (TODO: confirm CS/EN against the board)
#define MC_CS_GPIO       5
#define MC_EN_GPIO       20
#define MC_IN5_GPIO      27
#define MC_IN6_GPIO      21

static i2c_dev_t tca;
static adg1608_t mux;
static mc33879_handle_t hs_switch;

void app_main(void)
{
    ESP_LOGI(TAG, "Coordinate Domofon start (ESP-IDF + Component Registry)");

    ESP_ERROR_CHECK(i2cdev_init());

    memset(&tca, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(tca95x5_init_desc(&tca, TCA95X5_I2C_ADDR_BASE, I2C_NUM_0,
                                      I2C_SDA_GPIO, I2C_SCL_GPIO));
    ESP_ERROR_CHECK(tca95x5_port_set_mode(&tca, 0x0000));

    ESP_ERROR_CHECK(adg1608_init(&mux, &tca));

    // One SPI bus. Additional chips (other CS) must call spi_bus_add_device on SPI2_HOST.
    ESP_ERROR_CHECK(mc33879_bus_init(SPI2_HOST, SPI_MOSI_GPIO, SPI_MISO_GPIO, SPI_SCLK_GPIO));

    mc33879_config_t mc_cfg = {
        .spi_host     = SPI2_HOST,
        .pin_cs       = MC_CS_GPIO,
        .pin_en       = MC_EN_GPIO,
        .pin_in5      = MC_IN5_GPIO,
        .pin_in6      = MC_IN6_GPIO,
        .spi_clock_hz = 1000000,
    };
    ESP_ERROR_CHECK(mc33879_init(&hs_switch, &mc_cfg));

    ESP_ERROR_CHECK(door_init());

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LINE_DO1_GPIO) | (1ULL << LINE_DO2_GPIO),
        .mode = GPIO_MODE_INPUT,
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "=== Hardware ready ===");

    while (1) {
        int do1 = gpio_get_level(LINE_DO1_GPIO);
        int do2 = gpio_get_level(LINE_DO2_GPIO);
        ESP_LOGI(TAG, "Line DO1=%d DO2=%d", do1, do2);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
