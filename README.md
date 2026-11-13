# ESP32 Coordinate Domofon

Firmware for ESP32-PICO-V3 based coordinate intercom switcher (Vizit / Eltis style).

## Hardware

| Chip              | Role                          | Driver source                  |
|-------------------|-------------------------------|--------------------------------|
| ESP32-PICO-V3     | MCU                           | —                              |
| TCA9535PWR        | I²C GPIO expander             | **esp-idf-lib/tca95x5** (Registry) |
| ADG1608BR         | 8:1 analog mux                | Local driver                   |
| MC33879APEKR2     | 8-ch power switch (SPI)       | Local driver                   |
| 74LVC2G17         | Schmitt buffer                | Hardware only                  |

**Door open:** GPIO27 → RS-ON → 1W-CTRL → OpenD  
**Line detect:** GPIO36 / GPIO37  
**1-Wire:** GPIO12 (RX) / GPIO15 (TX)

## Build

```bash
idf.py set-target esp32
idf.py build
idf.py -p PORT flash monitor
```

Requires ESP-IDF ≥ 5.3 (tested with 5.5 / 6.1).

## Component Registry

TCA9535 is taken from the official registry:

```yaml
dependencies:
  esp-idf-lib/tca95x5: "^1.0.8"
```

## Status

- [x] Project structure
- [x] TCA9535 via Component Registry
- [x] ADG1608 driver
- [x] MC33879 driver (opaque handle)
- [x] Door open + basic 1-Wire
- [x] GitHub Actions CI
- [ ] Exact SPI pin mapping (board specific)
- [ ] Call detection logic
