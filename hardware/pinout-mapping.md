# Pin Mapping — ESP32-C6 → Breakout Modules

Complete GPIO assignment from the ESP32-C6 Super Mini to all breakout modules on the proto PCB.

## Motor Control — DRV8833 Module

| Function | DRV8833 Module Pin | ESP32-C6 Pin | Type | Notes |
|----------|-------------------|-------------|------|-------|
| Motor Forward (AIN1) | AIN1 | GPIO5 | PWM output | LEDC ch0, 20kHz |
| Motor Reverse (AIN2) | AIN2 | GPIO6 | PWM output | LEDC ch1, 20kHz |
| Channel B Fwd (BIN1) | BIN1 | GPIO7 | PWM output | May be unused |
| Channel B Rev (BIN2) | BIN2 | GPIO8 | PWM output | May be unused |
| Sleep (nSLEEP) | SLP | 3.3V (tied) | Power | Module may have onboard pull-up |
| Motor Power | VM | 6V battery | Power | Direct from battery, not from step-down |
| Motor Output | AOUT1/AOUT2 | — | — | Connect to DC motor wires |

## Position Sensor — LIS3DH Module

| Function | LIS3DH Module Pin | ESP32-C6 Pin | Type | Notes |
|----------|-------------------|-------------|------|-------|
| I2C Data (SDA) | SDA | GPIO19 | I2C | Pull-ups on module |
| I2C Clock (SCL) | SCL | GPIO20 | I2C | Pull-ups on module |
| Interrupt 1 (INT1) | INT1 | GPIO9 (opt) | Input | Motion detect / wake |
| Power | VCC | 3.3V | Power | From DC-DC step-down |
| Ground | GND | GND | Power | Common |

### LIS3DH I2C Address

| SDO Pin State | I2C Address |
|--------------|-------------|
| LOW (GND) | 0x18 |
| HIGH (VDD) | 0x19 |

## NFC Reader — PN532

| Function | PN532 Pin | ESP32-C6 Pin | Type | Notes |
|----------|-----------|-------------|------|-------|
| SPI Clock | SCK | GPIO4 | SPI | |
| SPI Data In | MISO | GPIO2 | SPI | |
| SPI Data Out | MOSI | GPIO3 | SPI | |
| Chip Select | SS/CS | GPIO10 | SPI | Active low |
| Interrupt | IRQ | GPIO1 | Input | Optional, for wake |
| Power | VCC | 3.3V | Power | |
| Ground | GND | GND | Power | |

## Status LEDs — WS2812B

| Function | WS2812B Pin | ESP32-C6 Pin | Type | Notes |
|----------|------------|-------------|------|-------|
| Data | DIN | GPIO0 | Digital out | NeoPixel protocol, add 330Ω series resistor |
| Power | VCC | 3.3V | Power | 100μF cap recommended near first LED |
| Ground | GND | GND | Power | |

## Power — DC-DC Step-Down Module

| Function | Module Pin | Connection | Notes |
|----------|-----------|------------|-------|
| Input | VIN | 6V battery + | From 4xAA pack |
| Output | VOUT | 3.3V rail | Powers all logic components |
| Ground | GND | Common GND | Battery - and all module GNDs |

## Power Distribution

| Rail | Source | Voltage | Consumers |
|------|--------|---------|-----------|
| VBAT | 4xAA batteries | ~6V (fresh) to ~4V (depleted) | DRV8833 VM |
| VCC_3V3 | DC-DC step-down module | 3.3V | ESP32-C6, PN532, LIS3DH, WS2812B |
| GND | Common | 0V | All modules |

## Summary Pin Usage

```
ESP32-C6 Super Mini
┌──────────────────────────────────────┐
│  GPIO0  ── WS2812B DIN (NeoPixel)    │
│  GPIO1  ── PN532 IRQ (optional)      │
│  GPIO2  ── PN532 MISO (SPI)         │
│  GPIO3  ── PN532 MOSI (SPI)         │
│  GPIO4  ── PN532 SCK (SPI)          │
│  GPIO5  ── DRV8833 AIN1 (PWM)       │
│  GPIO6  ── DRV8833 AIN2 (PWM)       │
│  GPIO7  ── DRV8833 BIN1 (PWM)       │
│  GPIO8  ── DRV8833 BIN2 (PWM)       │
│  GPIO9  ── LIS3DH INT1 (optional)   │
│  GPIO10 ── PN532 SS/CS (SPI)        │
│  GPIO18 ── [free]                    │
│  GPIO19 ── LIS3DH SDA (I2C)         │
│  GPIO20 ── LIS3DH SCL (I2C)         │
│  GPIO21 ── [free / battery ADC]      │
│  GPIO22 ── [free]                    │
│  GPIO23 ── [free]                    │
│  3V3    ── Power rail (from DC-DC)   │
│  GND    ── Common ground              │
└──────────────────────────────────────┘
```
