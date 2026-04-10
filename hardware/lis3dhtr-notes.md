# LIS3DHTR Accelerometer — Technical Notes

## Overview

The LIS3DHTR is a 3-axis MEMS accelerometer by STMicroelectronics. In the August Smart Lock, it detects the rotational position of the deadbolt by measuring the gravitational acceleration vector at different angles.

## Key Specifications

| Parameter | Value |
|-----------|-------|
| Interface | I2C / SPI |
| I2C Speed | Up to 400kHz (Fast Mode) |
| Supply Voltage | 1.71V – 3.6V |
| Current (normal mode) | ~11μA at 1Hz ODR |
| Current (low-power) | ~2μA at 1Hz ODR |
| Current (power-down) | ~0.5μA |
| Resolution | 10-bit (normal), 8-bit (low-power), 12-bit (high-res) |
| Range | ±2g / ±4g / ±8g / ±16g (selectable) |
| Package | LGA-16 (3×3×1mm) |

## I2C Configuration

### Address Selection

The I2C address depends on the SDO/SA0 pin:

| SDO Pin | I2C Address (7-bit) | I2C Address (8-bit R/W) |
|---------|-------------------|------------------------|
| LOW (GND) | 0x18 | 0x30 (W) / 0x31 (R) |
| HIGH (VDD) | 0x19 | 0x32 (W) / 0x33 (R) |

> [!NOTE]
> Check the SDO pin connection on the August PCB with a multimeter to determine the correct address. Most designs tie SDO to GND (address 0x18).

### I2C Wiring

```
ESP32-C6 GPIO19 (SDA) ──── LIS3DHTR Pin 4 (SDA)
ESP32-C6 GPIO20 (SCL) ──── LIS3DHTR Pin 6 (SCL)
```

Both SDA and SCL need 4.7kΩ pull-up resistors to 3.3V. The August PCB likely already has these.

## Important Registers

| Register | Address | Description | Default |
|----------|---------|-------------|---------|
| WHO_AM_I | 0x0F | Device identification | 0x33 |
| CTRL_REG1 | 0x20 | Data rate, power mode, axis enable | 0x07 |
| CTRL_REG4 | 0x23 | Full-scale range, high-res mode | 0x00 |
| STATUS_REG | 0x27 | Data available flags | 0x00 |
| OUT_X_L | 0x28 | X-axis output (low byte) | - |
| OUT_X_H | 0x29 | X-axis output (high byte) | - |
| OUT_Y_L | 0x2A | Y-axis output (low byte) | - |
| OUT_Y_H | 0x2B | Y-axis output (high byte) | - |
| OUT_Z_L | 0x2C | Z-axis output (low byte) | - |
| OUT_Z_H | 0x2D | Z-axis output (high byte) | - |
| INT1_CFG | 0x30 | Interrupt 1 configuration | 0x00 |
| INT1_SRC | 0x31 | Interrupt 1 source | 0x00 |
| INT1_THS | 0x32 | Interrupt 1 threshold | 0x00 |
| INT1_DURATION | 0x33 | Interrupt 1 duration | 0x00 |

### Initialization Sequence

```
1. Read WHO_AM_I (0x0F) → expect 0x33 (confirms communication)
2. Write CTRL_REG1 (0x20) = 0x27 → 10Hz ODR, normal mode, X/Y/Z enabled
3. Write CTRL_REG4 (0x23) = 0x00 → ±2g range, normal mode
```

For low-power operation:
```
Write CTRL_REG1 (0x20) = 0x2F → 10Hz ODR, low-power mode, X/Y/Z enabled
```

## Lock Position Detection

The LIS3DHTR measures gravitational acceleration (1g = 9.81 m/s²). When the lock's thumb-turn rotates, the accelerometer (mounted on the rotating mechanism) changes its orientation relative to gravity.

### How It Works

```
                  Gravity (1g downward)
                         │
                         ▼

    LOCKED position          UNLOCKED position
    ┌─────────┐              ┌─────────┐
    │         │              │         │
    │  ┌───┐  │              │  ┌───┐  │
    │  │LIS│  │ ◄── accel    │  │LIS│  │ ◄── accel rotated
    │  └───┘  │    reads     │  └───┘  │    reads different
    │         │    Z = -1g   │         │    Z = +0.5g
    └─────────┘              └─────────┘    X = +0.7g
    (thumb-turn              (thumb-turn
     vertical)                horizontal)
```

The key insight: the accelerometer's axes rotate with the thumb-turn, so the projection of gravity onto each axis changes with the rotation angle.

### Calibration Procedure

1. **Read LOCKED position:**
   - Manually set the lock to the locked position
   - Read X, Y, Z acceleration values 10 times, average them
   - Record: `locked_x`, `locked_y`, `locked_z`

2. **Read UNLOCKED position:**
   - Manually set the lock to the unlocked position
   - Read X, Y, Z acceleration values 10 times, average them
   - Record: `unlocked_x`, `unlocked_y`, `unlocked_z`

3. **Determine discriminating axis:**
   - Find the axis with the largest difference between locked and unlocked
   - This axis is your primary position indicator

4. **Set thresholds:**
   - `locked_threshold` = midpoint - margin
   - `unlocked_threshold` = midpoint + margin
   - Use ~0.2g margin for noise rejection

### Example Calibration Values

These are **example values only** — your lock's orientation will differ:

| State | X (g) | Y (g) | Z (g) |
|-------|-------|-------|-------|
| Locked | -0.05 | 0.02 | -0.98 |
| Unlocked | 0.71 | 0.02 | -0.65 |

In this example, both X and Z change significantly. A combined threshold using atan2(X, Z) to compute the rotation angle would be the most robust approach.

### Angle Calculation

```c
// Compute rotation angle from X and Z axes
float angle = atan2(accel_x, accel_z) * 180.0 / M_PI;

// Example thresholds (adjust after calibration):
// Locked:   angle ≈ -3° (nearly vertical)
// Unlocked: angle ≈ 47° (rotated ~50°)
// Midpoint threshold: ~22°
```

## Recommended Arduino Library

**[SparkFun LIS3DH Arduino Library](https://github.com/sparkfun/SparkFun_LIS3DH_Arduino_Library)**

Compatible with ESP32 and supports I2C. Basic usage:

```cpp
#include "SparkFunLIS3DH.h"
#include "Wire.h"

LIS3DH accel(I2C_MODE, 0x18);  // or 0x19

void setup() {
    Wire.begin(19, 20);  // SDA=GPIO19, SCL=GPIO20
    accel.begin();
    accel.settings.accelRange = 2;      // ±2g
    accel.settings.accelSampleRate = 10; // 10 Hz
}

void loop() {
    float x = accel.readFloatAccelX();
    float y = accel.readFloatAccelY();
    float z = accel.readFloatAccelZ();

    float angle = atan2(x, z) * 180.0 / M_PI;

    // TODO: Compare angle against calibrated thresholds
}
```

## Power Optimization

For battery-powered operation, the LIS3DHTR can be configured for minimal power consumption:

| Mode | ODR | Current | Use Case |
|------|-----|---------|----------|
| Power-down | 0 Hz | 0.5μA | Between operations |
| Low-power | 1 Hz | 2μA | Periodic position check |
| Normal | 10 Hz | 11μA | During motor operation |
| High-resolution | 10 Hz | 11μA | Precise calibration |

**Strategy:** Run at 1Hz in low-power mode for periodic position monitoring. Switch to 10Hz normal mode during motor operation for responsive feedback. Power down when the lock state is confirmed stable.

## Interrupt Configuration

The LIS3DHTR can generate an interrupt when acceleration exceeds a threshold, useful for detecting:

- **Manual lock turn** — someone physically rotating the thumb-turn
- **Door vibration** — potential tamper detection
- **Free-fall** — lock removed from door (anti-theft)

Configure INT1 for wake-on-motion:

```
Write INT1_CFG (0x30) = 0x2A  → Enable X/Y/Z high events (OR combination)
Write INT1_THS (0x32) = 0x10  → Threshold (~250mg at ±2g range)
Write INT1_DURATION (0x33) = 0x00  → No minimum duration
Write CTRL_REG3 (0x22) = 0x40  → Route INT1 to INT1 pin
```

> [!TIP]
> The motion interrupt can wake the ESP32-C6 from deep sleep, enabling a very low-power mode where the lock only wakes when someone physically touches the thumb-turn or when an NFC tap is detected (via PN532 IRQ).
