# Firmware

This project uses the **[HomeKey-ESP32](https://github.com/rednblkx/HomeKey-ESP32)** firmware by rednblkx. We do not reinvent the wheel — we configure and flash the existing firmware for our specific hardware setup.

## Firmware Source

- **Repository:** [rednblkx/HomeKey-ESP32](https://github.com/rednblkx/HomeKey-ESP32)
- **Target:** ESP32-C6
- **Framework:** ESP-IDF / Arduino

## What HomeKey-ESP32 Provides

- Apple HomeKey authentication (NFC via PN532 in SPI mode)
- HomeKit accessory server (lock type)
- MQTT client for Home Assistant integration
- Web-based configuration portal
- OTA firmware updates

## What We Customize

- **SPI pin assignments** for the PN532 (to avoid conflicts with motor/I2C pins)
- **GPIO assignments** for motor control (DRV8833) and position sensing (LIS3DHTR)
- **MQTT topics** and broker configuration
- **Lock timing** parameters (motor duration for lock/unlock)

## Files in This Directory

| File | Purpose |
|------|---------|
| `config/homekey-config.json` | Template configuration for HomeKey-ESP32 |
| `config/ha-mqtt-config.yaml` | MQTT auto-discovery payload for Home Assistant |
| `platformio.ini` | PlatformIO build config for ESP32-C6 (if building from source) |
| `src/motor_control.h` | Pin definitions and motor control constants |

## Quick Start

1. Download the latest HomeKey-ESP32 `.factory.bin` for ESP32-C6 from [releases](https://github.com/rednblkx/HomeKey-ESP32/releases)
2. Flash using the provided script: `./scripts/flash.sh /dev/ttyACM0`
3. Connect to the `HomeKey-XXXX` WiFi AP
4. Configure via web portal at `http://192.168.4.1`
5. Use the pin assignments from `config/homekey-config.json` as reference

See the [flashing guide](../docs/flashing-guide.md) for detailed instructions.
