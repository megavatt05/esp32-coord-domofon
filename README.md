# ESP32 Coordinate Domofon

Firmware for ESP32-PICO-V3 based coordinate intercom switcher (compatible with Vizit / Eltis style systems).

## Hardware

- **ESP32-PICO-V3**
- **TCA9535PWR** — I²C GPIO expander (controls ADG1608)
- **ADG1608BR** — 8:1 analog multiplexer
- **MC33879APEKR2** — 8-channel high-side/low-side switch (SPI)
- Door open via **GPIO27 → RS-ON → 1W-CTRL → OpenD**
- Line detection on **GPIO36 / GPIO37** (after 74LVC2G17)
- 1-Wire on **GPIO12 (RX) / GPIO15 (TX)**

## Project structure

```
main/
  main.c
  tca9535.c / .h
  adg1608.c / .h
  mc33879.c / .h
  door.c / .h
```

## Build (ESP-IDF 6.1)

```bash
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Status

Working skeleton. Drivers are implemented and compile-ready.  
Fine-tuning of SPI mode, exact MC33879 command format and channel mapping still needed after hardware bring-up.
