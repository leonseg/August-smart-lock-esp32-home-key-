# August Smart Lock Gen 2 → Apple HomeKey + Home Assistant

![ESP32-C6](https://img.shields.io/badge/ESP32--C6-Super_Mini-blue)
![HomeKey](https://img.shields.io/badge/Apple-HomeKey-black)
![Home Assistant](https://img.shields.io/badge/Home_Assistant-MQTT-41BDF5)
![License](https://img.shields.io/badge/License-MIT-green)

## Overview

A complete brain transplant of the **August Smart Lock Gen 2 (ASL-02)**. The original PCB (STM32L152 + CC2541 BLE + all onboard ICs) is removed entirely and replaced with a new custom control board built on a **prototype PCB** using off-the-shelf breakout modules, centered around an **ESP32-C6 Super Mini**.

This adds **Apple HomeKey** (NFC tap-to-unlock), **native HomeKit**, **MQTT integration** with Home Assistant, and **Zigbee** mesh routing. The original LED ring is replaced with **WS2812B addressable LEDs**.

Only the mechanical components are reused from the original lock: DC motor with gears, lock housing, and 4xAA battery holder.

This is the first public documentation of this type of modification.

| | |
|---|---|
| ![August PCB removed](docs/images/august-pcb-removed.jpg) | ![Lock internals with gears](docs/images/lock-internals-gears.jpg) |
| *Original August PCB (removed)* | *Lock body — motor and gear train (reused)* |

## Features

- **Apple HomeKey** — Tap your iPhone or Apple Watch on the door to unlock via NFC (PN532 + HomeKey-ESP32 firmware)
- **Native HomeKit** — Full HomeKit integration over WiFi, appears as a lock accessory in Apple Home
- **MQTT → Home Assistant** — Real-time lock state, remote control, and automations via MQTT
- **Zigbee Mesh Router** — ESP32-C6's IEEE 802.15.4 radio acts as a Zigbee router for your mesh network
- **WiFi 6 + BLE 5** — Modern wireless stack natively supported by the ESP32-C6
- **WS2812B Status Ring** — Addressable RGB LEDs replace the original LED ring, fully customizable colors/animations
- **Lock Position Feedback** — LIS3DH accelerometer module detects locked/unlocked position via gravitational angle
- **Silent Operation** — 20kHz PWM motor drive (above human hearing threshold)
- **Proto PCB Build** — All breakout modules on a single prototype board, no original PCB reverse-engineering needed

## Hardware Required

| Component | Description | Price (CAD) |
|-----------|-------------|-------------|
| ESP32-C6 Super Mini | Main MCU — WiFi 6, BLE 5, Zigbee | ~$4 |
| Elechouse PN532 NFC MINI | NFC reader for HomeKey (SPI mode) | ~$47 |
| NFC Antenna IPX 13.56MHz | External antenna for through-door reading | ~$3 |
| DRV8833 Motor Driver Module | Breakout board — dual H-bridge, 1.5A/ch | ~$6 |
| LIS3DH Accelerometer Module | Breakout board — 3-axis, I2C | ~$3 |
| DC-DC Step-Down Module | Buck converter — 6V battery → 3.3V logic | ~$2 |
| WS2812B LED Ring/Strip | Addressable RGB LEDs for status indication | ~$3 |
| Prototype PCB Board | Mounting board for all modules | ~$2 |
| 30AWG Silicone Wire | Point-to-point wiring | ~$3 |
| Flux (no-clean) | For soldering work | ~$4 |
| **Total** | | **~$77** |

> [!NOTE]
> You also need an August Smart Lock Gen 2 (ASL-02, FCC ID: 2AB6UASL2), a soldering iron, a multimeter, and a USB-C cable for flashing.

## How It Works

```
┌──────────────────────────────────────────────────────────────┐
│                     DOOR (exterior)                           │
│                                                               │
│    ┌──────────────┐    IPEX cable    ┌───────────────────┐    │
│    │  NFC Antenna  │◄───────────────►│  PN532 NFC Mini   │    │
│    │  (13.56 MHz)  │                 │  (SPI mode)       │    │
│    │  in door edge  │                 └────────┬──────────┘    │
│    └──────────────┘                           │ SPI           │
│                                                │               │
│         ┌── PROTO PCB ─────────────────────────┤              │
│         │                                      │              │
│         │  ┌──────────┐       ┌────────────────▼───────────┐  │
│         │  │ LIS3DH   │ I2C  │     ESP32-C6 Super Mini    │  │
│         │  │ (module)  │◄────►│                            │  │
│         │  └──────────┘      │  WiFi ──► HomeKit + MQTT   │  │
│         │                     │  802.15.4 ──► Zigbee       │  │
│         │  ┌──────────┐ PWM  │                            │  │
│         │  │ DRV8833  │◄────►│  GPIO0 ──► WS2812B LEDs   │  │
│         │  │ (module)  │      └────────────────────────────┘  │
│         │  └────┬─────┘                                       │
│         │       │          ┌──────────┐                       │
│         │  ┌────▼─────┐   │ DC-DC    │                       │
│         │  │ DC Motor │   │ Step-Down│  6V → 3.3V            │
│         │  │ (reused)  │   └─────┬────┘                       │
│         │  └──────────┘         │                             │
│         └───────────────────────┼─────────────────────────────│
│                                 │                             │
│  ┌────────────┐    ┌────────────▼─┐    ┌───────────────┐     │
│  │ WS2812B    │    │ 4xAA Battery │    │ Lock Housing  │     │
│  │ LED Ring   │    │ Pack (6V)    │    │ + Gears       │     │
│  └────────────┘    └──────────────┘    └───────────────┘     │
│                     AUGUST LOCK BODY (interior)               │
└──────────────────────────────────────────────────────────────┘

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
3. **Open the lock & identify motor/battery** → [`docs/pcb-mapping.md`](docs/pcb-mapping.md)
4. **Wire everything up** → [`docs/wiring-guide.md`](docs/wiring-guide.md)
5. **Install the NFC antenna** → [`docs/nfc-antenna-install.md`](docs/nfc-antenna-install.md)
6. **Flash the firmware** → [`docs/flashing-guide.md`](docs/flashing-guide.md)
7. **Configure Home Assistant** → [`home-assistant/`](home-assistant/)
8. **Troubleshoot issues** → [`docs/troubleshooting.md`](docs/troubleshooting.md)

## Project Status

- [x] Open lock and photograph internals
- [ ] Order all components (ESP32-C6, PN532, DRV8833 module, LIS3DH module, DC-DC step-down, WS2812B, proto PCB)
- [ ] Remove original August PCB
- [ ] Identify motor wires and battery connector
- [ ] Assemble new control board on proto PCB
- [ ] Wire DC-DC step-down converter (6V → 3.3V)
- [ ] Wire DRV8833 module to motor
- [ ] Wire LIS3DH module (I2C)
- [ ] Wire PN532 NFC module (SPI)
- [ ] Wire WS2812B LED ring
- [ ] Flash HomeKey-ESP32 firmware
- [ ] Configure MQTT broker and Home Assistant
- [ ] Calibrate motor timing (lock/unlock duration)
- [ ] Calibrate LIS3DH position thresholds
- [ ] Test Apple HomeKey tap-to-unlock
- [ ] Install NFC antenna in door edge
- [ ] Final assembly and long-term testing

## Important Notes

> [!NOTE]
> The original August PCB is fully removed and replaced. You only need to identify the motor wires (2 wires) and battery connector — no complex PCB reverse-engineering required.

> [!WARNING]
> Apple HomeKey support is based on reverse-engineered protocols (HomeKey-ESP32 by rednblkx). This is not officially supported by Apple and may break with future iOS updates.

- **SPI is mandatory** for the PN532 when used with HomeKey-ESP32 (I2C mode is not supported for HomeKey)
- **Zigbee concurrent operation** with WiFi + HomeKey has not been validated yet — this is a [TODO: validate] item
- **Battery life** will differ from the original lock due to WiFi power consumption; expect shorter battery life
- The original PCB, Bluetooth, LEDs, and onboard ICs are all removed — everything is replaced

## Hardware Reference

- [ESP32-C6 Super Mini Pinout](hardware/pinout-esp32c6-supermini.md)
- [Pin Mapping (ESP32-C6 → Modules)](hardware/pinout-mapping.md)
- [DRV8833 Motor Driver Notes](hardware/drv8833-notes.md)
- [LIS3DH Accelerometer Notes](hardware/lis3dhtr-notes.md)

## Credits

- **[HomeKey-ESP32](https://github.com/rednblkx/HomeKey-ESP32)** by rednblkx — The firmware that makes Apple HomeKey possible on ESP32
- **[Espressif ESP32-C6](https://www.espressif.com/en/products/socs/esp32-c6)** — The SoC that makes WiFi + BLE + Zigbee possible on a single chip
- **August / Assa Abloy** — For making a lock with a clean mechanical design worth modifying

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
