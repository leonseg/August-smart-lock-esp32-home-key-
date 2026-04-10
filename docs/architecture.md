# Architecture

## System Overview

This project replaces the entire original August Smart Lock Gen 2 (ASL-02) PCB with a new custom control board built on a prototype PCB using breakout modules. Only the mechanical components are reused (motor, gears, housing, battery holder).

### Original vs Modified Architecture

```
ORIGINAL (ASL-02)                     MODIFIED
┌──────────────────────┐              ┌──────────────────────────┐
│  STM32L152 (main MCU)│              │  NEW PROTO PCB BUILD:    │
│  CC2541 (BLE radio)  │              │                          │
│  DRV8833 (onboard)   │   ────►      │  ESP32-C6 Super Mini     │
│  LIS3DHTR (onboard)  │  (removed)   │  + PN532 NFC Mini (SPI)  │
│  74AC541 LED driver  │              │  + DRV8833 module         │
│  Voltage regulator   │              │  + LIS3DH module          │
│  Proprietary BLE     │              │  + DC-DC step-down        │
└──────────────────────┘              │  + WS2812B LED ring       │
                                      └──────────────────────────┘
        │                                      │
        │ (reused)                             │ (reused)
        ▼                                      ▼
┌──────────────────────┐              ┌──────────────────────┐
│  DC motor + gears    │              │  DC motor + gears    │
│  Lock housing        │              │  Lock housing        │
│  4xAA batteries      │              │  4xAA batteries      │
└──────────────────────┘              └──────────────────────┘
```

## Hardware Architecture

```
                    ┌───────────────────────────────────────┐
                    │           4xAA Battery Pack            │
                    │           (6V nominal)                 │
                    └────────┬──────────────┬───────────────┘
                             │              │
                        6V direct      DC-DC Step-Down
                        (motor)        6V → 3.3V module
                             │              │
                    ┌────────▼──────┐   ┌───▼───────────────┐
                    │   DRV8833     │   │  ESP32-C6         │
                    │   Module      │   │  Super Mini       │
                    │               │   │                   │
                    │  AIN1◄────────┼───┤  GPIO5 (PWM)     │
                    │  AIN2◄────────┼───┤  GPIO6 (PWM)     │
                    │  BIN1◄────────┼───┤  GPIO7 (PWM)     │
                    │  BIN2◄────────┼───┤  GPIO8 (PWM)     │
                    │               │   │                   │
                    │  AOUT1/2──────┼───┤► DC Motor         │
                    └───────────────┘   │                   │
                                        │  GPIO19 (SDA)─────┼──► LIS3DH module
                    ┌───────────────┐   │  GPIO20 (SCL)─────┼──► (I2C)
                    │   PN532       │   │                   │
                    │   NFC Mini    │   │  GPIO0 ───────────┼──► WS2812B LEDs
                    │               │   │                   │
                    │  SCK──────────┼───┤  GPIO4            │
                    │  MISO─────────┼───┤  GPIO2            │
                    │  MOSI─────────┼───┤  GPIO3            │
                    │  SS/CS────────┼───┤  GPIO10           │
                    │               │   │                   │
                    │  ┌─IPEX──┐   │   │  WiFi  ──► HomeKit│
                    │  │NFC Ant│   │   │  802.15.4─► Zigbee│
                    │  └───────┘   │   │                   │
                    └───────────────┘   └───────────────────┘

              ── All modules mounted on a prototype PCB ──
```

> [!NOTE]
> All pin assignments are final and conflict-free. See [wiring-guide.md](wiring-guide.md) for the complete wiring reference.

## Software Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    HomeKey-ESP32 Firmware                     │
│                   (by rednblkx, pre-built)                   │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │  HomeKey      │  │  HomeKit     │  │  MQTT Client     │  │
│  │  NFC Handler  │  │  Accessory   │  │                  │  │
│  │              │  │  Server      │  │  Publishes:       │  │
│  │  PN532 SPI   │  │              │  │  - lock/state     │  │
│  │  driver      │  │  WiFi mgmt   │  │  - lock/avail     │  │
│  │              │  │              │  │                  │  │
│  │  Auth flow:  │  │  Pair setup  │  │  Subscribes:      │  │
│  │  ISO 18092   │  │  Pair verify │  │  - lock/set       │  │
│  │  ECP + NDEF  │  │              │  │                  │  │
│  └──────┬───────┘  └──────┬───────┘  └────────┬─────────┘  │
│         │                 │                    │             │
│  ┌──────▼─────────────────▼────────────────────▼─────────┐  │
│  │              Lock Control Logic                        │  │
│  │                                                        │  │
│  │  - Motor control (DRV8833 via PWM)                     │  │
│  │  - Position sensing (LIS3DH module via I2C)             │  │
│  │  - LED feedback (WS2812B via NeoPixel protocol)         │  │
│  │  - State machine: LOCKED ↔ UNLOCKED                    │  │
│  └────────────────────────────────────────────────────────┘  │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │              ESP-IDF / Arduino Framework                │  │
│  │  WiFi 6 · BLE 5 · IEEE 802.15.4 · SPI · I2C · PWM     │  │
│  └────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## Communication Flow

### HomeKey Unlock Sequence

```
iPhone/Watch          PN532             ESP32-C6          DRV8833         Motor
    │                   │                  │                │              │
    │  NFC tap          │                  │                │              │
    │──────────────────►│                  │                │              │
    │                   │  SPI data        │                │              │
    │                   │─────────────────►│                │              │
    │                   │                  │  Auth check    │              │
    │                   │                  │  (HomeKey)     │              │
    │                   │                  │                │              │
    │                   │                  │  PWM signals   │              │
    │                   │                  │───────────────►│              │
    │                   │                  │                │  Drive motor │
    │                   │                  │                │─────────────►│
    │                   │                  │                │              │
    │                   │                  │  Read accel    │              │
    │                   │                  │  (I2C)         │              │
    │                   │                  │  position OK   │              │
    │                   │                  │                │              │
    │                   │                  │  MQTT publish  │              │
    │                   │                  │  "UNLOCKED"    │              │
    │                   │                  │───────────────────────► HA    │
    │  HomeKit update   │                  │                │              │
    │◄──────────────────────────────────────│                │              │
```

### MQTT Topics

| Topic | Direction | Payload | Description |
|-------|-----------|---------|-------------|
| `homekey/lock/state` | ESP32 → HA | `LOCKED` / `UNLOCKED` | Current lock state |
| `homekey/lock/set` | HA → ESP32 | `LOCK` / `UNLOCK` | Command to change state |
| `homekey/lock/availability` | ESP32 → HA | `online` / `offline` | Device availability (LWT) |
| `homekey/lock/battery` | ESP32 → HA | `0-100` | Battery percentage (TODO) |
| `homekey/lock/position` | ESP32 → HA | JSON `{x,y,z}` | Raw accelerometer data (debug) |

## Power Architecture

```
4xAA Batteries (6V nominal, 4V cutoff)
│
├──► 6V direct ──► DRV8833 module VM ──► DC Motor (only during lock/unlock)
│
└──► DC-DC Step-Down Module (6V → 3.3V)
     │
     ├──► ESP32-C6 3V3 pin (main power consumer in idle)
     ├──► PN532 VCC (idle current ~50mA — consider power gating)
     ├──► LIS3DH module (~10μA in low-power mode)
     └──► WS2812B LEDs (~20mA per LED at moderate brightness)
```

> [!TIP]
> The biggest battery drain will be the ESP32-C6 WiFi radio. Consider implementing deep sleep between HomeKey polls if battery life is a concern. The PN532 can wake the ESP32 via an IRQ pin.

## Zigbee Integration

The ESP32-C6 includes a native IEEE 802.15.4 radio that can function as a Zigbee router, extending your Zigbee mesh network through the lock.

```
Zigbee Coordinator (HA)  ◄──►  ESP32-C6 (Router)  ◄──►  Other Zigbee Devices
                                  │
                                  └── Also running WiFi + HomeKey simultaneously
                                      [TODO: validate concurrent radio operation]
```

> [!WARNING]
> It is unclear whether the ESP32-C6 can run WiFi, HomeKey NFC polling, and Zigbee routing simultaneously without performance issues. This needs validation after the basic lock functionality is working.
