# Wiring Guide

Complete wiring reference for assembling the new control board on a prototype PCB using breakout modules.

## Overview

All components are breakout modules soldered/wired on a single proto PCB:

1. **ESP32-C6 Super Mini** — main controller
2. **DRV8833 module** — motor driver (PWM)
3. **LIS3DH module** — accelerometer (I2C)
4. **PN532 NFC Mini** — HomeKey reader (SPI)
5. **DC-DC step-down module** — 6V → 3.3V power
6. **WS2812B LED ring** — status LEDs (single data pin)

## 1. Motor Control — DRV8833 Module

| DRV8833 Module Pin | ESP32-C6 Pin | Notes |
|-------------------|-------------|-------|
| AIN1 | GPIO5 (PWM) | LEDC channel 0, 20kHz — motor forward |
| AIN2 | GPIO6 (PWM) | LEDC channel 1, 20kHz — motor reverse |
| BIN1 | GPIO7 (PWM) | Channel B — possibly unused |
| BIN2 | GPIO8 (PWM) | Channel B — possibly unused |
| SLP (nSLEEP) | 3.3V (tied high) | Always active. Module may have onboard pull-up |
| VM | 6V battery direct | Motor power — bypass the DC-DC step-down |
| GND | GND | Common ground |
| AOUT1 | Motor wire + | To DC motor |
| AOUT2 | Motor wire - | To DC motor |

> [!NOTE]
> The DRV8833 module likely uses only Channel A for the lock motor. Channel B is available for a buzzer or left unused.

## 2. Position Sensor — LIS3DH Module

| LIS3DH Module Pin | ESP32-C6 Pin | Notes |
|-------------------|-------------|-------|
| SDA | GPIO19 | I2C data — pull-ups on module |
| SCL | GPIO20 | I2C clock — pull-ups on module |
| VCC | 3.3V | From DC-DC step-down |
| GND | GND | Common |
| INT1 | GPIO9 (optional) | Motion interrupt for wake-on-tap |

> [!TIP]
> The LIS3DH module includes onboard I2C pull-up resistors. No external pull-ups needed.

## 3. NFC Reader — PN532 (SPI — Required for HomeKey)

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

## 4. Status LEDs — WS2812B

| WS2812B Pin | ESP32-C6 Pin | Notes |
|-------------|-------------|-------|
| DIN (Data In) | GPIO0 | NeoPixel protocol, add 330Ω series resistor |
| VCC | 3.3V | Works at 3.3V for short LED strips |
| GND | GND | Common |

> [!TIP]
> Add a **330Ω resistor** between GPIO0 and the WS2812B DIN pin to protect the data line. Add a **100μF capacitor** across VCC/GND near the first LED to smooth power.

## 5. Power — DC-DC Step-Down Module

| Connection | From | To | Notes |
|-----------|------|-----|-------|
| VIN | 4xAA battery + (6V) | Module VIN | Input voltage |
| VOUT | Module 3.3V output | 3.3V rail | Powers ESP32-C6, PN532, LIS3DH, WS2812B |
| GND | Battery GND | Module GND + common GND | |

```
4xAA Battery Pack (6V)
│
├──► DRV8833 module VM ──► Motor power (6V direct)
│
├──► DC-DC Step-Down module VIN
│    │
│    └──► 3.3V VOUT ──► 3.3V rail
│         │
│         ├──► ESP32-C6 3V3 pin
│         ├──► PN532 VCC
│         ├──► LIS3DH VCC
│         └──► WS2812B VCC
│
└──► GND (common to all modules)
```

> [!WARNING]
> **Do NOT power the ESP32-C6 from USB and batteries simultaneously.** Disconnect the battery pack when flashing via USB-C.

## Pin Conflict Resolution

With breakout modules on a proto PCB, most pin conflicts are avoided by choosing non-overlapping GPIOs:

| GPIO | Default HomeKey-ESP32 | Our Assignment | Conflict? |
|------|----------------------|----------------|-----------|
| GPIO5 | SPI SS | Motor AIN1 | Resolved → SPI SS moved to GPIO10 |
| GPIO19 | SPI MISO | I2C SDA | Resolved → SPI MISO moved to GPIO2 |
| GPIO23 | SPI MOSI | N/A on C6 | Resolved → SPI MOSI moved to GPIO3 |
| GPIO18 | SPI SCK | Free | Resolved → SPI SCK moved to GPIO4 |

> [!TIP]
> The HomeKey-ESP32 firmware allows configuring custom SPI pins via the web interface. Use the assignments listed above.

## Wiring Diagram

```
                         ┌──────────────────────────────────┐
                         │       ESP32-C6 Super Mini         │
                         │                                  │
  DRV8833 AIN1 ◄─────── ┤ GPIO5            GPIO4 ├────────► PN532 SCK
  DRV8833 AIN2 ◄─────── ┤ GPIO6            GPIO3 ├────────► PN532 MOSI
  DRV8833 BIN1 ◄─────── ┤ GPIO7            GPIO2 ├────────► PN532 MISO
  DRV8833 BIN2 ◄─────── ┤ GPIO8           GPIO10 ├────────► PN532 SS/CS
                         │                                  │
  LIS3DH SDA   ◄─────── ┤ GPIO19          GPIO1  ├────────► PN532 IRQ
  LIS3DH SCL   ◄─────── ┤ GPIO20          GPIO0  ├──[330Ω]─► WS2812B DIN
                         │                                  │
  DC-DC 3.3V   ─────────┤ 3V3              3V3   ├────────► PN532 VCC
  Common GND   ─────────┤ GND              GND   ├────────► PN532 GND
                         │                                  │
                         └──────────────────────────────────┘

  DC-DC Step-Down:  Battery 6V ──► VIN | VOUT ──► 3.3V rail
  DRV8833 Module:   Battery 6V ──► VM  | AOUT1/2 ──► Motor
```

## Assembly Tips

### Mounting on Proto PCB

1. Plan the layout before soldering — place all modules on the proto PCB to verify fit
2. The ESP32-C6 Super Mini can be soldered with header pins for easy removal
3. Mount the DRV8833 module near the edge closest to the motor wires
4. Mount the DC-DC step-down near the battery connector
5. Keep the PN532 accessible for IPEX antenna cable routing

### Wiring Best Practices

1. Use 30AWG silicone wire — flexible and heat resistant
2. Color-code your wires: red=3.3V, black=GND, blue=SPI, green=I2C, yellow=PWM
3. Keep wires short and route them flat against the proto PCB
4. Use the proto PCB copper traces for power rails (3.3V and GND) where possible
5. Secure wires with hot glue or kapton tape

### Fitting Inside the Lock

1. The proto PCB replaces the original August PCB in the same cavity
2. Measure the cavity before cutting your proto PCB to size
3. The PN532 can sit on top of or beside the proto PCB (it has its own PCB)
4. Route the IPEX antenna cable out through the original PCB opening area
5. Ensure no wires interfere with the gear train

## Validation Checklist

Before powering on, verify each connection with a multimeter:

- [ ] DC-DC step-down: 6V input, 3.3V output confirmed
- [ ] No shorts between 3.3V and GND
- [ ] No shorts between 6V motor rail and 3.3V logic rail
- [ ] Continuity: GPIO5 → DRV8833 AIN1
- [ ] Continuity: GPIO6 → DRV8833 AIN2
- [ ] Continuity: GPIO19 → LIS3DH SDA
- [ ] Continuity: GPIO20 → LIS3DH SCL
- [ ] Continuity: GPIO4 → PN532 SCK
- [ ] Continuity: GPIO2 → PN532 MISO
- [ ] Continuity: GPIO3 → PN532 MOSI
- [ ] Continuity: GPIO10 → PN532 SS/CS
- [ ] Continuity: GPIO0 → 330Ω → WS2812B DIN
- [ ] All GND connections are common
- [ ] DRV8833 VM connected to 6V battery (not 3.3V!)
