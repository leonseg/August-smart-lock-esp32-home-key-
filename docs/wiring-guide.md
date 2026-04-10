# Wiring Guide

Complete wiring reference for connecting the ESP32-C6 Super Mini to the August Smart Lock Gen 2 PCB and PN532 NFC module.

> [!WARNING]
> **Do not wire anything until you have completed the [PCB mapping](pcb-mapping.md).** The pin assignments below are based on the ESP32-C6 Super Mini pinout and known DRV8833/LIS3DHTR conventions. The actual PCB trace routing must be verified with a multimeter.

## Overview

Three subsystems need wiring:

1. **ESP32-C6 → August PCB** (motor driver + accelerometer + LEDs)
2. **ESP32-C6 → PN532 NFC** (SPI bus for HomeKey)
3. **Power rails** (6V motor, 3.3V logic)

## 1. ESP32-C6 → August PCB (DRV8833 + LIS3DHTR)

| Signal | Component | ESP32-C6 Pin | Notes |
|--------|-----------|-------------|-------|
| AIN1 (motor fwd) | DRV8833 | GPIO5 (PWM) | LEDC channel 0, 20kHz |
| AIN2 (motor rev) | DRV8833 | GPIO6 (PWM) | LEDC channel 1, 20kHz |
| BIN1 | DRV8833 | GPIO7 (PWM) | Channel B — may be unused, verify |
| BIN2 | DRV8833 | GPIO8 (PWM) | Channel B — may be unused, verify |
| nSLEEP | DRV8833 | 3.3V (tied high) | Always active, ~1mA standby |
| SDA | LIS3DHTR | GPIO19 | I2C data |
| SCL | LIS3DHTR | GPIO20 | I2C clock |
| INT1 | LIS3DHTR | GPIO3 (optional) | Motion interrupt, for wake-on-tap |
| 3V3 | Logic rail | 3V3 | From battery regulator |
| GND | Common | GND | Common ground |

> [!NOTE]
> The August lock likely uses only one DRV8833 channel (A) for the motor. Channel B may drive the speaker/buzzer or may be unused. Verify during PCB mapping.

## 2. PN532 → ESP32-C6 (SPI — Required for HomeKey)

| PN532 Pin | ESP32-C6 Pin | Function |
|-----------|-------------|----------|
| SCK | GPIO4 | SPI Clock |
| MISO | GPIO2 | SPI Master In |
| MOSI | GPIO3 | SPI Master Out |
| SS/CS | GPIO10 | SPI Chip Select |
| IRQ | GPIO1 (optional) | Interrupt — wake on NFC tap |
| VCC | 3.3V | Power |
| GND | GND | Ground |

> [!WARNING]
> **SPI mode is mandatory for HomeKey-ESP32.** I2C and UART modes will not work for HomeKey authentication. Set the PN532 DIP switches to SPI mode (both switches DOWN on the Elechouse Mini).

### PN532 DIP Switch Configuration

```
┌─────────┐
│  PN532  │
│  MINI   │
│         │
│  SW1: ▼ │  (DOWN = LOW)
│  SW2: ▼ │  (DOWN = LOW)
│         │
│  = SPI  │
└─────────┘
```

## Pin Conflict Resolution

The default HomeKey-ESP32 firmware expects specific SPI pins that may conflict with other peripherals. Here is how conflicts are resolved:

| GPIO | Default Use | Conflict | Resolution |
|------|-------------|----------|------------|
| GPIO5 | HomeKey SPI SS | Motor AIN1 | **Moved SPI SS → GPIO10** |
| GPIO19 | HomeKey SPI MISO | I2C SDA (LIS3DHTR) | **Moved SPI MISO → GPIO2** |
| GPIO23 | HomeKey SPI MOSI | Not available on C6 | **Moved SPI MOSI → GPIO3** |
| GPIO18 | HomeKey SPI SCK | May conflict | **Moved SPI SCK → GPIO4** |

> [!TIP]
> The HomeKey-ESP32 firmware allows configuring custom SPI pins via the web interface or config file. The pins listed above are the resolved assignments that avoid all conflicts.

## 3. Power Wiring

```
4xAA Battery Pack (6V)
│
├──► DRV8833 VM pin ──► Motor power (6V)
│
├──► August PCB 3.3V regulator ──► 3.3V rail
│    │
│    ├──► ESP32-C6 3V3 pin
│    ├──► PN532 VCC
│    ├──► LIS3DHTR VDD
│    └──► 74AC541 VCC
│
└──► GND (common to all components)
```

> [!WARNING]
> **Do NOT power the ESP32-C6 from USB and batteries simultaneously** during development. Disconnect the battery pack when flashing via USB-C to avoid voltage conflicts.

## Wiring Diagram (ASCII)

```
                         ┌──────────────────────────────┐
                         │      ESP32-C6 Super Mini      │
                         │                              │
  DRV8833 AIN1 ◄────────┤ GPIO5          GPIO4 ├───────► PN532 SCK
  DRV8833 AIN2 ◄────────┤ GPIO6          GPIO3 ├───────► PN532 MOSI
  DRV8833 BIN1 ◄────────┤ GPIO7          GPIO2 ├───────► PN532 MISO
  DRV8833 BIN2 ◄────────┤ GPIO8         GPIO10 ├───────► PN532 SS/CS
                         │                              │
  LIS3DHTR SDA ◄────────┤ GPIO19        GPIO1  ├───────► PN532 IRQ (opt)
  LIS3DHTR SCL ◄────────┤ GPIO20               │
                         │                              │
  Battery 3V3 ──────────┤ 3V3            3V3   ├───────► PN532 VCC
  Battery GND ──────────┤ GND            GND   ├───────► PN532 GND
                         │                              │
                         └──────────────────────────────┘
```

## Soldering Tips

### Desoldering the STM32L152 + CC2541

1. Apply flux generously to all pins
2. Use a hot air station at 350°C with a medium nozzle
3. Heat evenly — rotate around the chip until it lifts
4. Clean pads with solder wick and flux
5. Inspect for lifted pads or damaged traces under magnification

### Wiring the ESP32-C6

1. Use 30AWG silicone wire (flexible, heat resistant)
2. Tin both the wire ends and the PCB pads before joining
3. Keep wires short and route them flat against the PCB
4. Secure wires with a small dot of hot glue or kapton tape
5. Double-check every connection with multimeter continuity before powering on

> [!TIP]
> Take a photo of each connection as you solder it. This makes troubleshooting much easier and helps document the build for others.

## Connector Reference

The August PCB has several flex/ribbon connectors:

| Connector | Connects To | Notes |
|-----------|------------|-------|
| Main flex | Battery board + motor | Carries 6V, GND, motor signals |
| LED flex | LED ring PCB | Drives the status LEDs via 74AC541 |
| Button flex | Manual turn sensor | Detects physical button press |

> [!NOTE]
> These connectors remain in their original positions. Only the MCU and BLE chip are replaced.

## Validation Checklist

After wiring, verify each connection before powering on:

- [ ] Continuity: GPIO5 → DRV8833 AIN1
- [ ] Continuity: GPIO6 → DRV8833 AIN2
- [ ] Continuity: GPIO7 → DRV8833 BIN1
- [ ] Continuity: GPIO8 → DRV8833 BIN2
- [ ] Continuity: GPIO19 → LIS3DHTR SDA
- [ ] Continuity: GPIO20 → LIS3DHTR SCL
- [ ] Continuity: GPIO4 → PN532 SCK
- [ ] Continuity: GPIO2 → PN532 MISO
- [ ] Continuity: GPIO3 → PN532 MOSI
- [ ] Continuity: GPIO10 → PN532 SS/CS
- [ ] No shorts between 3V3 and GND
- [ ] No shorts between 6V motor rail and 3V3 logic rail
- [ ] All GND connections are common
