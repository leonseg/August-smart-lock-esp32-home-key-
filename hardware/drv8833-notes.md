# DRV8833 Motor Driver — Technical Notes

## Overview

The DRV8833 is a dual H-bridge motor driver by Texas Instruments, used in the August Smart Lock to drive the DC motor that rotates the deadbolt.

## Key Specifications

| Parameter | Value |
|-----------|-------|
| Supply Voltage (VM) | 2.7V – 10.8V |
| Logic Voltage (VCC) | 2.7V – 6.8V |
| Output Current (per channel) | 1.5A continuous, 2A peak |
| Channels | 2 (A and B) |
| Control | PWM per H-bridge leg |
| Protection | Overcurrent, thermal shutdown, UVLO |
| Sleep Current | ~1mA (nSLEEP HIGH), <1μA (nSLEEP LOW) |
| Package | HTSSOP-16 or WSON-10 |

## H-Bridge Control Modes

### Channel A (Main Motor)

| Mode | AIN1 | AIN2 | AOUT1 | AOUT2 | Description |
|------|------|------|-------|-------|-------------|
| Forward | PWM | LOW | PWM | LOW | Motor turns clockwise (lock direction) |
| Reverse | LOW | PWM | LOW | PWM | Motor turns counter-clockwise (unlock) |
| Brake | HIGH | HIGH | LOW | LOW | Active braking — motor stops quickly |
| Coast | LOW | LOW | Hi-Z | Hi-Z | Motor free-spins to a stop |

### Channel B (May Be Unused)

Same control logic as Channel A but on BIN1/BIN2 and BOUT1/BOUT2.

> [!NOTE]
> The August lock likely uses only Channel A for the motor. Channel B may drive a piezo buzzer/speaker for feedback sounds, or it may be entirely unused. **Verify during PCB reverse-engineering.**

## PWM Control with ESP32-C6

The ESP32-C6 uses the LEDC (LED Control) peripheral for PWM generation. This is the recommended approach for motor control:

```
LEDC Channel 0 → GPIO5 → AIN1 (forward PWM)
LEDC Channel 1 → GPIO6 → AIN2 (reverse PWM)
LEDC Channel 2 → GPIO7 → BIN1 (if needed)
LEDC Channel 3 → GPIO8 → BIN2 (if needed)
```

### PWM Frequency Selection

| Frequency | Audible? | Motor Efficiency | Notes |
|-----------|----------|-----------------|-------|
| 1 kHz | Yes (loud whine) | Lower | Not recommended |
| 5 kHz | Yes (noticeable) | Medium | Acceptable for testing |
| 20 kHz | No (above hearing) | Good | **Recommended for production** |
| 40 kHz | No | Slightly less efficient | Unnecessary |

**Recommendation: 20kHz** — inaudible, efficient, well within the DRV8833's switching capability.

### PWM Resolution

Using 8-bit resolution (0-255) provides sufficient control granularity:

- 255 = 100% duty = full speed
- 200 = ~78% duty = reduced speed (quieter operation)
- 128 = 50% duty = half speed
- 0 = 0% duty = stopped

## nSLEEP Pin

The nSLEEP pin controls the DRV8833's low-power sleep mode:

| nSLEEP | State | Current Draw |
|--------|-------|-------------|
| HIGH (3.3V) | Active — ready to drive | ~1mA standby |
| LOW (GND) | Sleep — outputs disabled | <1μA |

### Our Design Choice: Tied to 3.3V

We tie nSLEEP directly to the 3.3V rail, keeping the DRV8833 always active. This simplifies the design (no GPIO needed for sleep control) at the cost of ~1mA continuous draw.

**Trade-off:**
- **Pro:** Simpler wiring, instant motor response
- **Con:** ~1mA continuous draw from batteries

For a battery-powered lock, this is acceptable. The ESP32-C6 WiFi radio draws 80-120mA, making the 1mA DRV8833 standby negligible.

If battery optimization becomes critical, you can:
1. Cut the 3.3V tie to nSLEEP
2. Wire nSLEEP to a GPIO
3. Pull nSLEEP LOW when not actuating the motor

## nFAULT Pin

The nFAULT output goes LOW when the DRV8833 detects an overcurrent or thermal fault. This pin is open-drain and requires a pull-up resistor (usually present on the August PCB).

Optionally connect to an ESP32-C6 GPIO for fault detection:

```
DRV8833 nFAULT → GPIO (input, with pull-up) → monitor for LOW = fault
```

This is not critical for initial operation but useful for debugging motor issues.

## Motor Direction in the August Lock

The deadbolt rotation direction depends on the lock's orientation:

| Action | Motor Direction | DRV8833 Mode |
|--------|----------------|-------------|
| Lock (extend deadbolt) | Forward | AIN1=PWM, AIN2=LOW |
| Unlock (retract deadbolt) | Reverse | AIN1=LOW, AIN2=PWM |
| Stop (hold position) | Brake | AIN1=HIGH, AIN2=HIGH |
| Stop (release) | Coast | AIN1=LOW, AIN2=LOW |

> [!WARNING]
> The "forward" and "reverse" directions above are assumptions. The actual direction depends on the motor wiring polarity on the August PCB. If the lock turns the wrong way, swap AIN1 and AIN2 assignments in the firmware.

## Timing Considerations

| Parameter | Initial Value | Notes |
|-----------|--------------|-------|
| Lock duration | 1500ms | Time motor runs to fully extend deadbolt |
| Unlock duration | 1500ms | Time motor runs to fully retract deadbolt |
| Settle delay | 200ms | Wait after motor stops before reading accelerometer |
| Max duration | 3000ms | Safety timeout — stop motor if position not reached |

> [!TIP]
> Use the LIS3DHTR accelerometer for closed-loop position feedback instead of fixed timing. Read the accelerometer during motor operation and stop when the target angle is reached. This is more reliable than fixed-time operation.

## Current Consumption

| State | Estimated Current |
|-------|------------------|
| Motor stall (worst case) | ~500mA at 6V |
| Motor running (no load) | ~100-200mA |
| Motor running (loaded, normal) | ~300-500mA |
| DRV8833 standby (nSLEEP HIGH) | ~1mA |
| DRV8833 sleep (nSLEEP LOW) | <1μA |

> [!WARNING]
> Motor stall current can spike to 1A+. The DRV8833's overcurrent protection should handle this, but avoid prolonged stall conditions (e.g., if the deadbolt is jammed). Implement a timeout in firmware.
