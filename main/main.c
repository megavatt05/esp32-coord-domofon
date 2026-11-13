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

// MC33879 pins (TODO: уточнить по плате)
#define MC_CS_GPIO       19
#define MC_EN_GPIO       20
#define MC_IN5_GPIO      27
#define MC_IN6_GPIO      21
#define MC_MOSI_GPIO     23
#define MC_MISO_GPIO     19
#define MC_SCLK_GPIO     18

static i2c_dev_t tca;
static adg1608_t mux;
static mc33879_handle_t hs_switch;

void app_main(void)
{
    ESP_LOGI(TAG, "Coordinate Domofon start (ESP-IDF + Component Registry)");

    // --- I2C bus (esp-idf-lib/i2cdev) ---
    ESP_ERROR_CHECK(i2cdev_init());

    // --- TCA9535 ---
    memset(&tca, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(tca95x5_init_desc(&tca, TCA95X5_I2C_ADDR_BASE, I2C_NUM_0,
                                      I2C_SDA_GPIO, I2C_SCL_GPIO));

    // All 16 pins as outputs (0 = output, 1 = input)
    ESP_ERROR_CHECK(tca95x5_port_set_mode(&tca, 0x0000));

    // --- ADG1608 ---
    ESP_ERROR_CHECK(adg1608_init(&mux, &tca));

    // --- MC33879 ---
    mc33879_config_t mc_cfg = {
        .spi_host     = SPI2_HOST,
        .pin_cs       = MC_CS_GPIO,
        .pin_sclk     = MC_SCLK_GPIO,
        .pin_mosi     = MC_MOSI_GPIO,
        .pin_miso     = MC_MISO_GPIO,
        .pin_en       = MC_EN_GPIO,
        .pin_in5      = MC_IN5_GPIO,
        .pin_in6      = MC_IN6_GPIO,
        .spi_clock_hz = 1000000,
    };
    ESP_ERROR_CHECK(mc33879_init(&hs_switch, &mc_cfg));

    // --- Door ---
    ESP_ERROR_CHECK(door_init());

    // --- Line inputs ---
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

        // door_open(2000);  // test

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
