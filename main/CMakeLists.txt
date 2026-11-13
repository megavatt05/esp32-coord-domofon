idf_component_register(
    SRCS
        "main.c"
        "adg1608.c"
        "mc33879.c"
        "door.c"
    INCLUDE_DIRS "."
    REQUIRES
        driver
        esp_driver_gpio
        esp_driver_spi
        freertos
        log
        tca95x5
        i2cdev
)
