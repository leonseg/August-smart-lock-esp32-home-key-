# ESP32-C6 Super Mini — Pinout Reference

## Board Overview

The ESP32-C6 Super Mini is a compact development board (~25×18mm) based on the Espressif ESP32-C6 SoC. It features native USB-C, WiFi 6, BLE 5, and IEEE 802.15.4 (Zigbee/Thread).

## Pinout Diagram

```
                    ┌───────────┐
                    │  USB-C    │
                    │           │
              5V  ──┤           ├──  5V
             GND  ──┤           ├──  GND
             3V3  ──┤           ├──  GPIO10
           GPIO4  ──┤           ├──  GPIO9
           GPIO5  ──┤           ├──  GPIO8
           GPIO6  ──┤           ├──  GPIO7
           GPIO7  ──┤           ├──  GPIO3 *
           GPIO0  ──┤           ├──  GPIO2
           GPIO1  ──┤           ├──  GPIO1 *
          GPIO18  ──┤           ├──  GPIO23
          GPIO19  ──┤           ├──  GPIO22
          GPIO20  ──┤           ├──  GPIO21
          GPIO21  ──┤           ├──  GPIO20 *
                    │           │
                    └───────────┘

* Some pins may appear on both sides depending on
  the specific board variant. Verify with your board.
```

> [!WARNING]
> ESP32-C6 Super Mini boards vary between manufacturers. Some pins may differ. Always verify with your specific board's silkscreen markings.

## Pin Capabilities

| GPIO | ADC | PWM | I2C | SPI | UART | Notes |
|------|-----|-----|-----|-----|------|-------|
| GPIO0 | ADC1_0 | Yes | Yes | Yes | Yes | BOOT button on some boards |
| GPIO1 | ADC1_1 | Yes | Yes | Yes | Yes | |
| GPIO2 | ADC1_2 | Yes | Yes | MISO* | Yes | |
| GPIO3 | ADC1_3 | Yes | Yes | MOSI* | Yes | |
| GPIO4 | ADC1_4 | Yes | Yes | SCK* | Yes | |
| GPIO5 | - | Yes | Yes | Yes | Yes | |
| GPIO6 | - | Yes | Yes | Yes | FSPI | |
| GPIO7 | - | Yes | Yes | Yes | FSPI | |
| GPIO8 | - | Yes | Yes | Yes | Yes | Status LED on some boards |
| GPIO9 | - | Yes | Yes | Yes | Yes | BOOT button on some boards |
| GPIO10 | - | Yes | Yes | SS* | Yes | |
| GPIO18 | - | Yes | Yes | Yes | U0RXD | USB D- (native USB) |
| GPIO19 | - | Yes | SDA* | Yes | U0TXD | USB D+ (native USB) |
| GPIO20 | - | Yes | SCL* | Yes | U1RXD | |
| GPIO21 | - | Yes | Yes | Yes | U1TXD | |
| GPIO22 | - | Yes | Yes | Yes | Yes | |
| GPIO23 | - | Yes | Yes | Yes | Yes | |

`*` = Default peripheral function (can be remapped)

## Power Pins

| Pin | Description |
|-----|-------------|
| 5V | USB 5V input/output (when powered via USB) |
| 3V3 | 3.3V regulated output (from onboard LDO) |
| GND | Ground |

> [!WARNING]
> The 3V3 pin is an **output** from the onboard regulator when powered via USB. When powering from batteries, you can supply 3.3V directly to this pin, but **do not connect USB simultaneously** — the voltage regulator may back-feed.

## Important Notes for This Project

### Used Pins (Our Assignment)

| GPIO | Assignment | Peripheral |
|------|-----------|------------|
| GPIO2 | PN532 MISO | SPI |
| GPIO3 | PN532 MOSI | SPI |
| GPIO4 | PN532 SCK | SPI |
| GPIO10 | PN532 SS/CS | SPI |
| GPIO1 | PN532 IRQ | Input (optional) |
| GPIO5 | DRV8833 AIN1 | PWM (LEDC) |
| GPIO6 | DRV8833 AIN2 | PWM (LEDC) |
| GPIO7 | DRV8833 BIN1 | PWM (LEDC) |
| GPIO8 | DRV8833 BIN2 | PWM (LEDC) |
| GPIO19 | LIS3DHTR SDA | I2C |
| GPIO20 | LIS3DHTR SCL | I2C |

### Remaining Free Pins

| GPIO | Available For |
|------|--------------|
| GPIO0 | LED driver (74AC541) or button input |
| GPIO9 | LED driver (74AC541) or button input |
| GPIO18 | Spare (also USB D-) |
| GPIO21 | Battery voltage ADC |
| GPIO22 | Spare |
| GPIO23 | Spare |

### Strapping Pins

| GPIO | Strapping Function | Safe State |
|------|-------------------|------------|
| GPIO8 | - | Any |
| GPIO9 | Boot mode | Pull-up = normal boot, pull-down = download mode |
| GPIO15 | - | Any |

> [!NOTE]
> Avoid using GPIO9 for critical functions that might interfere with boot mode selection. If GPIO9 is used for DRV8833 BIN2, ensure the pull-up resistor on the August PCB doesn't force download mode on boot.
