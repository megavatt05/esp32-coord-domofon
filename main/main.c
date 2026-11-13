#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "tca9535.h"
#include "adg1608.h"
#include "mc33879.h"
#include "door.h"

static const char *TAG = "DOMOFON";

#define I2C_SCL_GPIO     32
#define I2C_SDA_GPIO     33
#define TCA_INT_GPIO     35
#define LINE_DO1_GPIO    36
#define LINE_DO2_GPIO    37

static tca9535_t tca;
static adg1608_t mux;
static mc33879_t hs_switch;

void app_main(void)
{
    ESP_LOGI(TAG, "Coordinate Domofon firmware start (ESP-IDF 6.1)");

    // I2C bus
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    // TCA9535 + ADG1608
    ESP_ERROR_CHECK(tca9535_init(&tca, bus));
    ESP_ERROR_CHECK(adg1608_init(&mux, &tca));

    // MC33879
    ESP_ERROR_CHECK(mc33879_init(&hs_switch));

    // Door + 1-Wire
    ESP_ERROR_CHECK(door_init());

    // Line inputs
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LINE_DO1_GPIO) | (1ULL << LINE_DO2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "=== All hardware initialized ===");

    while (1) {
        int do1 = gpio_get_level(LINE_DO1_GPIO);
        int do2 = gpio_get_level(LINE_DO2_GPIO);
        ESP_LOGI(TAG, "Line: DO1=%d  DO2=%d", do1, do2);

        // Пример открытия двери (раскомментировать для теста)
        // door_open(2000);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
