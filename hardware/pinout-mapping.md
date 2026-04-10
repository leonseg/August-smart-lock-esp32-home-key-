# Pin Mapping — STM32 → ESP32-C6 → Components

This document maps the original STM32L152 pin functions to their ESP32-C6 equivalents.

> [!WARNING]
> The STM32 column must be filled in during PCB reverse-engineering. The ESP32-C6 assignments are preliminary and may need adjustment based on actual PCB traces.

## Motor Control (DRV8833)

| Function | STM32L152 Pin | DRV8833 Pin | ESP32-C6 Pin | Type | Notes |
|----------|--------------|-------------|-------------|------|-------|
| Motor Forward (AIN1) | _TODO: trace_ | Pin 2 (AIN1) | GPIO5 | PWM output | LEDC ch0, 20kHz |
| Motor Reverse (AIN2) | _TODO: trace_ | Pin 7 (AIN2) | GPIO6 | PWM output | LEDC ch1, 20kHz |
| Channel B Fwd (BIN1) | _TODO: trace_ | Pin 10 (BIN1) | GPIO7 | PWM output | May be unused |
| Channel B Rev (BIN2) | _TODO: trace_ | Pin 15 (BIN2) | GPIO8 | PWM output | May be unused |
| Sleep (nSLEEP) | _TODO: trace_ | Pin 1 (nSLEEP) | 3.3V (tied) | Power | Always active |

### DRV8833 Pinout (HTSSOP-16 or WSON-10)

```
HTSSOP-16:
                 ┌──────┐
    nSLEEP  1 ──┤      ├── 16  VCC
      AIN1  2 ──┤      ├── 15  BIN2
      AIN2  3 ──┤      ├── 14  BIN1
     AOUT1  4 ──┤      ├── 13  BOUT1
     AOUT2  5 ──┤      ├── 12  BOUT2
       GND  6 ──┤      ├── 11  GND
       GND  7 ──┤      ├── 10  GND
        VM  8 ──┤      ├──  9  nFAULT
                 └──────┘
```

> [!NOTE]
> Verify the DRV8833 package type on your PCB. WSON-10 has a different pinout arrangement.

## Position Sensor (LIS3DHTR)

| Function | STM32L152 Pin | LIS3DHTR Pin | ESP32-C6 Pin | Type | Notes |
|----------|--------------|-------------|-------------|------|-------|
| I2C Data (SDA) | _TODO: trace_ | Pin 4 (SDA) | GPIO19 | I2C | 4.7kΩ pull-up needed |
| I2C Clock (SCL) | _TODO: trace_ | Pin 6 (SCL) | GPIO20 | I2C | 4.7kΩ pull-up needed |
| Interrupt 1 (INT1) | _TODO: trace_ | Pin 11 (INT1) | GPIO3 (opt) | Input | Motion detect |
| Interrupt 2 (INT2) | _TODO: trace_ | Pin 9 (INT2) | - | - | Not used |
| Address Select (SDO) | _TODO: trace_ | Pin 5 (SDO) | - | - | Determines I2C addr |

### LIS3DHTR I2C Address

| SDO Pin State | I2C Address |
|--------------|-------------|
| LOW (GND) | 0x18 |
| HIGH (VDD) | 0x19 |

## NFC Reader (PN532) — New Component

| Function | PN532 Pin | ESP32-C6 Pin | Type | Notes |
|----------|-----------|-------------|------|-------|
| SPI Clock | SCK | GPIO4 | SPI | |
| SPI Data In | MISO | GPIO2 | SPI | |
| SPI Data Out | MOSI | GPIO3 | SPI | |
| Chip Select | SS/CS | GPIO10 | SPI | Active low |
| Interrupt | IRQ | GPIO1 | Input | Optional, for wake |
| Power | VCC | 3.3V | Power | |
| Ground | GND | GND | Power | |

## LED Driver (74AC541)

| Function | STM32L152 Pin | 74AC541 Pin | ESP32-C6 Pin | Type | Notes |
|----------|--------------|-------------|-------------|------|-------|
| Output Enable | _TODO: trace_ | OE (pin 1, 19) | _TODO_ | GPIO | Active low |
| Data bit 0 | _TODO: trace_ | A1 (pin 2) | _TODO_ | GPIO | |
| Data bit 1 | _TODO: trace_ | A2 (pin 3) | _TODO_ | GPIO | |
| ... | _TODO: trace_ | ... | _TODO_ | GPIO | |

> [!NOTE]
> The 74AC541 is an octal buffer/line driver. The STM32 likely used 4-8 GPIO pins to drive the LED ring. These need to be mapped and reassigned to available ESP32-C6 GPIOs. The LED ring is non-critical for basic functionality — prioritize motor, NFC, and accelerometer first.

## Power

| Rail | Source | Voltage | Consumers |
|------|--------|---------|-----------|
| VBAT | 4xAA batteries | ~6V (fresh) to ~4V (depleted) | DRV8833 VM |
| VCC_3V3 | Onboard LDO regulator | 3.3V | ESP32-C6, PN532, LIS3DHTR, 74AC541 |
| GND | Common | 0V | All components |

## Summary Pin Usage

```
ESP32-C6 Super Mini
┌──────────────────────────────────────┐
│  GPIO0  ── [free / LED driver]       │
│  GPIO1  ── PN532 IRQ (optional)      │
│  GPIO2  ── PN532 MISO (SPI)         │
│  GPIO3  ── PN532 MOSI (SPI)         │
│  GPIO4  ── PN532 SCK (SPI)          │
│  GPIO5  ── DRV8833 AIN1 (PWM)       │
│  GPIO6  ── DRV8833 AIN2 (PWM)       │
│  GPIO7  ── DRV8833 BIN1 (PWM)       │
│  GPIO8  ── DRV8833 BIN2 (PWM)       │
│  GPIO9  ── [free / button]           │
│  GPIO10 ── PN532 SS/CS (SPI)        │
│  GPIO18 ── [free]                    │
│  GPIO19 ── LIS3DHTR SDA (I2C)       │
│  GPIO20 ── LIS3DHTR SCL (I2C)       │
│  GPIO21 ── [free / battery ADC]      │
│  GPIO22 ── [free]                    │
│  GPIO23 ── [free]                    │
│  3V3    ── Power rail + DRV8833 nSLEEP│
│  GND    ── Common ground              │
└──────────────────────────────────────┘
```
