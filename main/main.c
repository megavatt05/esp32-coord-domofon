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

/*
 * Pin map from the board schematic (net names), ESP32-PICO-V3.
 *
 * One external SPI bus on SPI2_HOST (HSPI). SPI0/SPI1 are left on the
 * in-package flash and are never initialized here.
 *
 *   MOSI  GPIO7   net IO7   -> MC33879 DI     free on PICO-V3
 *   MISO  GPIO9   net IO9   -> MC33879 DO     free on PICO-V3, PSRAM on V3-02
 *   SCLK  GPIO5   net IO5   -> MC33879 SCLK   free, strapping pin
 *   CS-D  GPIO8   net IO8   -> MC33879 CS     free on PICO-V3
 *   EN-DE GPIO20  net IO20  -> MC33879 EN
 *   CS-E  GPIO10  net IO10                    free on PICO-V3, PSRAM on V3-02
 *   CS-P  GPIO19  net IO19
 *   EN-P  GPIO22  net IO22
 *
 * Flash (datasheet Table 6, PICO-V3): GPIO6 = FLASH_CLK, GPIO11 = FLASH_CS.
 * Those two are not used. GPIO7/8/9/10 are free on PICO-V3 (Table 8).
 * On PICO-V3-02, GPIO9 and GPIO10 are PSRAM and must not be on this bus.
 *
 * MC33879 IN5 is the same net as RS-ON (GPIO27). Door open drives it.
 * SPI bit of OUT5 must stay 0, otherwise it ORs with the door pulse.
 * MC33879 IN6 is GPIO21 (A6).
 */

#define I2C_SCL_GPIO     32
#define I2C_SDA_GPIO     33
#define TCA_INT_GPIO     35

#define LINE_DO1_GPIO    36
#define LINE_DO2_GPIO    37

#define SPI_MOSI_GPIO    7
#define SPI_MISO_GPIO    9
#define SPI_SCLK_GPIO    5

#define MC_CS_GPIO       8     // CS-D
#define MC_EN_GPIO       20    // EN-DE
#define MC_IN5_GPIO      27    // RS-ON, also the door-open line
#define MC_IN6_GPIO      21

static i2c_dev_t tca;
static adg1608_t mux;
static mc33879_handle_t hs_switch;

void app_main(void)
{
    ESP_LOGI(TAG, "Coordinate Domofon start");

    ESP_ERROR_CHECK(i2cdev_init());

    memset(&tca, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(tca95x5_init_desc(&tca, TCA95X5_I2C_ADDR_BASE, I2C_NUM_0,
                                      I2C_SDA_GPIO, I2C_SCL_GPIO));
    ESP_ERROR_CHECK(tca95x5_port_set_mode(&tca, 0x0000));

    ESP_ERROR_CHECK(adg1608_init(&mux, &tca));

    // SPI2 only. SPI1_HOST stays with the internal flash.
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
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&io_conf);

    gpio_config_t int_conf = {
        .pin_bit_mask = (1ULL << TCA_INT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&int_conf);

    ESP_LOGI(TAG, "=== Hardware ready ===");

    while (1) {
        int do1 = gpio_get_level(LINE_DO1_GPIO);
        int do2 = gpio_get_level(LINE_DO2_GPIO);
        ESP_LOGI(TAG, "Line DO1=%d DO2=%d", do1, do2);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
