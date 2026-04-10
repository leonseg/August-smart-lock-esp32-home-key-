# August Smart Lock Gen 2 → Apple HomeKey + Home Assistant

![ESP32-C6](https://img.shields.io/badge/ESP32--C6-Super_Mini-blue)
![HomeKey](https://img.shields.io/badge/Apple-HomeKey-black)
![Home Assistant](https://img.shields.io/badge/Home_Assistant-MQTT-41BDF5)
![License](https://img.shields.io/badge/License-MIT-green)

## Overview

A surgical modification of the **August Smart Lock Gen 2 (ASL-02)** that replaces the original STM32L152 + CC2541 BLE combo with a single **ESP32-C6 Super Mini**, adding **Apple HomeKey** (NFC tap-to-unlock), **native HomeKit**, **MQTT integration** with Home Assistant, and **Zigbee** mesh routing capability.

The original lock's mechanical components are fully reused: DC motor with gears, DRV8833 motor driver, LIS3DHTR accelerometer for lock position sensing, 74AC541 LED driver, and the 4xAA battery holder (6V motor rail, 3V logic rail).

This is the first public documentation of this type of modification. No existing teardown or mod guide existed for this specific conversion path.

## Features

- **Apple HomeKey** — Tap your iPhone or Apple Watch on the door to unlock via NFC (PN532 + HomeKey-ESP32 firmware)
- **Native HomeKit** — Full HomeKit integration over WiFi, appears as a lock accessory in Apple Home
- **MQTT → Home Assistant** — Real-time lock state, remote control, and automations via MQTT
- **Zigbee Mesh Router** — ESP32-C6's IEEE 802.15.4 radio acts as a Zigbee router for your mesh network
- **WiFi 6 + BLE 5** — Modern wireless stack natively supported by the ESP32-C6
- **Lock Position Feedback** — LIS3DHTR accelerometer detects locked/unlocked position via gravitational angle
- **Silent Operation** — 20kHz PWM motor drive (above human hearing threshold)

## Hardware Required

| Component | Description | Price (CAD) |
|-----------|-------------|-------------|
| ESP32-C6 Super Mini | Main MCU — WiFi 6, BLE 5, Zigbee | ~$4 |
| Elechouse PN532 NFC MINI | NFC reader for HomeKey (SPI mode) | ~$47 |
| NFC Antenna IPX 13.56MHz | External antenna for through-door reading | ~$3 |
| 30AWG Silicone Wire | Point-to-point wiring on PCB | ~$3 |
| Flux (no-clean) | For desoldering/soldering work | ~$4 |
| **Total** | | **~$61** |

> [!NOTE]
> You also need an August Smart Lock Gen 2 (ASL-02, FCC ID: 2AB6UASL2), basic soldering equipment, a multimeter, and a USB-C cable for flashing.

## How It Works

```
┌─────────────────────────────────────────────────────────────┐
│                     DOOR (exterior)                          │
│                                                              │
│    ┌──────────────┐    IPEX cable    ┌──────────────────┐    │
│    │  NFC Antenna  │◄───────────────►│  PN532 NFC Mini  │    │
│    │  (13.56 MHz)  │                 │  (SPI mode)      │    │
│    │  in door edge  │                 └────────┬─────────┘    │
│    └──────────────┘                           │ SPI bus       │
│                                                │              │
│                              ┌─────────────────▼──────────┐  │
│  ┌──────────┐   I2C bus     │     ESP32-C6 Super Mini     │  │
│  │ LIS3DHTR │◄─────────────►│                             │  │
│  │ (accel)  │               │  WiFi ──► HomeKit + MQTT    │  │
│  └──────────┘               │  BLE  ──► (reserved)        │  │
│                              │  802.15.4 ──► Zigbee Router │  │
│  ┌──────────┐   PWM x2     │                             │  │
│  │ DRV8833  │◄─────────────►│  GPIO5/6 ──► AIN1/AIN2     │  │
│  │ (H-bridge)│               │  GPIO7/8 ──► BIN1/BIN2     │  │
│  └────┬─────┘               └─────────────────────────────┘  │
│       │                                                       │
│  ┌────▼─────┐    ┌────────────┐                              │
│  │ DC Motor │    │ 4xAA Batt  │                              │
│  │ + Gears  │    │ 6V + 3V3   │                              │
│  └──────────┘    └────────────┘                              │
│                                                              │
│                     AUGUST LOCK BODY (interior)              │
└─────────────────────────────────────────────────────────────┘

         ▼ WiFi / MQTT ▼

┌─────────────────────────────────┐
│        Home Assistant           │
│  ┌───────────┐  ┌────────────┐  │
│  │ MQTT Lock │  │ Automations│  │
│  └───────────┘  └────────────┘  │
└─────────────────────────────────┘
```

## Step-by-Step Guide

1. **Understand the architecture** → [`docs/architecture.md`](docs/architecture.md)
2. **Gather components** → [`docs/bom.md`](docs/bom.md)
3. **Open the lock & map the PCB** → [`docs/pcb-mapping.md`](docs/pcb-mapping.md)
4. **Wire everything up** → [`docs/wiring-guide.md`](docs/wiring-guide.md)
5. **Install the NFC antenna** → [`docs/nfc-antenna-install.md`](docs/nfc-antenna-install.md)
6. **Flash the firmware** → [`docs/flashing-guide.md`](docs/flashing-guide.md)
7. **Configure Home Assistant** → [`home-assistant/`](home-assistant/)
8. **Troubleshoot issues** → [`docs/troubleshooting.md`](docs/troubleshooting.md)

## Project Status

- [ ] Open lock and photograph PCB (recto/verso HD photos)
- [ ] Reverse-engineer PCB traces with multimeter continuity
- [ ] Complete STM32 → component pin mapping
- [ ] Order ESP32-C6 Super Mini + PN532 + NFC antenna
- [ ] Desolder STM32L152 and CC2541 BLE
- [ ] Solder ESP32-C6 Super Mini point-to-point
- [ ] Flash HomeKey-ESP32 firmware
- [ ] Configure MQTT broker and Home Assistant
- [ ] Calibrate motor timing (lock/unlock duration)
- [ ] Calibrate LIS3DHTR position thresholds
- [ ] Test Apple HomeKey tap-to-unlock
- [ ] Install NFC antenna in door edge
- [ ] Final assembly and long-term testing

## Important Notes

> [!WARNING]
> This project requires manual reverse-engineering of the August PCB. No public pinout documentation exists for the ASL-02. You must trace every connection with a multimeter before wiring.

> [!WARNING]
> Apple HomeKey support is based on reverse-engineered protocols (HomeKey-ESP32 by rednblkx). This is not officially supported by Apple and may break with future iOS updates.

- **SPI is mandatory** for the PN532 when used with HomeKey-ESP32 (I2C mode is not supported for HomeKey)
- **Pin conflicts** exist between the default SPI pins and I2C/motor pins — see the [wiring guide](docs/wiring-guide.md) for resolution
- **Zigbee concurrent operation** with WiFi + HomeKey has not been validated yet — this is a [TODO: validate] item
- **Battery life** will differ from the original lock due to WiFi power consumption; expect shorter battery life
- The original Bluetooth functionality is completely removed — this mod replaces it entirely

## Hardware Reference

- [ESP32-C6 Super Mini Pinout](hardware/pinout-esp32c6-supermini.md)
- [Pin Mapping (STM32 → ESP32-C6)](hardware/pinout-mapping.md)
- [DRV8833 Motor Driver Notes](hardware/drv8833-notes.md)
- [LIS3DHTR Accelerometer Notes](hardware/lis3dhtr-notes.md)

## Credits

- **[HomeKey-ESP32](https://github.com/rednblkx/HomeKey-ESP32)** by rednblkx — The firmware that makes Apple HomeKey possible on ESP32
- **[Espressif ESP32-C6](https://www.espressif.com/en/products/socs/esp32-c6)** — The SoC that makes WiFi + BLE + Zigbee possible on a single chip
- **August / Assa Abloy** — For making a lock with a clean mechanical design worth modifying

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
