# Architecture

## System Overview

This project replaces the original August Smart Lock Gen 2 (ASL-02) brain with an ESP32-C6 Super Mini, adding Apple HomeKey, HomeKit, MQTT, and Zigbee capabilities while reusing all original mechanical and power components.

### Original vs Modified Architecture

```
ORIGINAL (ASL-02)                     MODIFIED
┌──────────────────────┐              ┌──────────────────────┐
│  STM32L152 (main MCU)│              │  ESP32-C6 Super Mini │
│  CC2541 (BLE radio)  │   ────►      │  + PN532 NFC (SPI)   │
│  Proprietary BLE     │              │  + NFC Antenna (IPEX) │
└──────────────────────┘              └──────────────────────┘
        │                                      │
        │ (unchanged)                          │ (unchanged)
        ▼                                      ▼
┌──────────────────────┐              ┌──────────────────────┐
│  DRV8833 motor driver│              │  DRV8833 motor driver│
│  DC motor + gears    │              │  DC motor + gears    │
│  LIS3DHTR accel      │              │  LIS3DHTR accel      │
│  74AC541 LED driver  │              │  74AC541 LED driver  │
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
                        6V rail         3.3V rail
                        (motor)    (via onboard regulator)
                             │              │
                    ┌────────▼──────┐   ┌───▼───────────────┐
                    │   DRV8833     │   │  ESP32-C6         │
                    │   H-Bridge    │   │  Super Mini       │
                    │               │   │                   │
                    │  AIN1◄────────┼───┤  GPIO5 (PWM)     │
                    │  AIN2◄────────┼───┤  GPIO6 (PWM)     │
                    │  BIN1◄────────┼───┤  GPIO7 (PWM)     │
                    │  BIN2◄────────┼───┤  GPIO8 (PWM)     │
                    │  nSLEEP──3V3  │   │                   │
                    │               │   │                   │
                    │  AOUT1/2──────┼───┤► DC Motor         │
                    └───────────────┘   │                   │
                                        │  GPIO19 (SDA)─────┼──► LIS3DHTR
                    ┌───────────────┐   │  GPIO20 (SCL)─────┼──► (I2C)
                    │   PN532       │   │                   │
                    │   NFC Mini    │   │                   │
                    │               │   │                   │
                    │  SCK──────────┼───┤  GPIO18           │
                    │  MISO─────────┼───┤  GPIO2            │
                    │  MOSI─────────┼───┤  GPIO7 *conflict  │
                    │  SS/CS────────┼───┤  GPIO10           │
                    │               │   │                   │
                    │  ┌─IPEX──┐   │   │  WiFi  ──► HomeKit│
                    │  │NFC Ant│   │   │  802.15.4─► Zigbee│
                    │  └───────┘   │   │                   │
                    └───────────────┘   └───────────────────┘
```

> [!WARNING]
> The pin assignments above are preliminary. Some pins will conflict and must be reassigned after the PCB reverse-engineering phase. See [wiring-guide.md](wiring-guide.md) for conflict resolution.

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
│  │  - Position sensing (LIS3DHTR via I2C)                 │  │
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
├──► 6V Rail ──► DRV8833 VM ──► DC Motor (only during lock/unlock)
│
└──► 3.3V Regulator (on August PCB or ESP32-C6 onboard)
     │
     ├──► ESP32-C6 (main power consumer in idle)
     ├──► PN532 (idle current ~50mA — consider power gating)
     ├──► LIS3DHTR (~10μA in low-power mode)
     └──► 74AC541 LED driver
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
