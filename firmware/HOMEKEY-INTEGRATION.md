# HomeKey-ESP32 Integration Guide (Option B)

This guide explains how to combine our custom firmware (motor PWM, LIS3DH
position detection, WS2812B status ring) with **rednblkx/HomeKey-ESP32**
to get full Apple HomeKey support.

> **Why an overlay?** HomeKey-ESP32 is a complete firmware (HomeSpan
> server + reverse-engineered Apple ECC crypto + NFC stack), not a library.
> An ESP32 only runs one firmware at a time, so we can't run both side by
> side — we have to merge them at build time.

## Overview

```
┌────────────────────────────────────────────────┐
│            Single ESP32-C6 firmware            │
├──────────────────────────┬─────────────────────┤
│  rednblkx/HomeKey-ESP32  │   This repo (ours)  │
│  ───────────────────────  │   ───────────────── │
│  • HomeSpan (HomeKit)    │  • motor.cpp (PWM)  │
│  • PN532 SPI driver       │  • accelerometer   │
│  • Apple HomeKey crypto  │  • leds (WS2812B)   │
│  • WiFi / MQTT / portal  │  • lock_controller  │
│  • main.cpp               │  • config.h         │
└──────────────────────────┴─────────────────────┘
                       │
                       ▼
              On HomeKey auth success
              call lock_controller_request_unlock()
```

## Step-by-step

### 1. Clone HomeKey-ESP32 as the base

```bash
git clone https://github.com/rednblkx/HomeKey-ESP32.git
cd HomeKey-ESP32
```

### 2. Add this repo as a remote / submodule

Pick one:

**Option B.1 — Submodule (cleanest):**
```bash
git submodule add https://github.com/leonseg/august-smart-lock-esp32-home-key- august-mod
```

**Option B.2 — Just copy the files in:**
```bash
cp /path/to/August-smart-lock-esp32-home-key-/firmware/src/{motor,accelerometer,leds,lock_controller,lock_state,config}.* src/
```

### 3. Resolve config.h conflicts

HomeKey-ESP32 has its own pin defines. Open their `src/main.cpp` (or
`src/HomeKey.cpp` depending on the version) and either:

- **Replace** their pin defines with `#include "config.h"` (ours takes over)
- **OR** edit `config.h` to match the names HomeKey-ESP32 expects

Pin assignments must match what's documented in `docs/wiring-guide.md`:

| Function   | GPIO | Used by                            |
|------------|------|------------------------------------|
| PN532 SCK  | 4    | HomeKey-ESP32 SPI                  |
| PN532 MISO | 2    | HomeKey-ESP32 SPI                  |
| PN532 MOSI | 3    | HomeKey-ESP32 SPI                  |
| PN532 SS   | 10   | HomeKey-ESP32 SPI                  |
| DRV8833 AIN1 | 5  | our motor.cpp                      |
| DRV8833 AIN2 | 6  | our motor.cpp                      |
| LIS3DH SDA | 19   | our accelerometer.cpp              |
| LIS3DH SCL | 20   | our accelerometer.cpp              |
| WS2812B    | 0    | our leds.cpp                       |

> ⚠️ HomeKey-ESP32 may default the PN532 to I²C. Force SPI mode by setting
> the appropriate `#define USE_SPI` (check their README — the symbol name
> varies between versions).

### 4. Patch HomeKey-ESP32's main to call our controller

Find where HomeKey-ESP32 handles a successful authentication. In recent
versions this is in `src/HomeKey.cpp` around the `auth0_keys` /
`auth1_keys` flow, or in the `Service::Lock` characteristic update.

Add **two lines** at boot and **one block** in the auth-success path:

**At the top of `src/main.cpp`:**
```cpp
#include "lock_controller.h"
#include "leds.h"
```

**In `setup()` after HomeSpan begins:**
```cpp
  leds_init();
  lock_controller_init();
  lock_controller_on_state_change([](lock_state_t, lock_state_t ns){
      leds_set_mode(leds_mode_for_state(ns));
  });
```

**In `loop()`:**
```cpp
  lock_controller_loop();
  leds_tick();
```

**On HomeKey auth success** (find the function — usually called something
like `nfcAuthFlow()` or hooked via the `LockMechanism::update()` override
that HomeKey-ESP32 sets up). Replace their direct GPIO toggle with:

```cpp
// was: digitalWrite(LOCK_PIN, HIGH); delay(500); digitalWrite(LOCK_PIN, LOW);
if (newTargetState == LOCK_TARGET_LOCKED)
    lock_controller_request_lock();
else
    lock_controller_request_unlock();
```

And feed our state back to HomeKit so the Home app stays in sync:

```cpp
lock_controller_on_state_change([this](lock_state_t, lock_state_t ns){
    if (ns == LOCK_STATE_LOCKED)
        lockCurrentState->setVal(LOCK_CURRENT_LOCKED);
    else if (ns == LOCK_STATE_UNLOCKED)
        lockCurrentState->setVal(LOCK_CURRENT_UNLOCKED);
    else if (ns == LOCK_STATE_JAMMED)
        lockCurrentState->setVal(LOCK_CURRENT_JAMMED);
});
```

### 5. Merge the platformio.ini lib_deps

Add to HomeKey-ESP32's `platformio.ini`:

```ini
lib_deps =
    ; ... existing HomeKey-ESP32 deps ...
    sparkfun/SparkFun LIS3DH Arduino Library@^1.0.3
    fastled/FastLED@^3.7.0
```

(`PN532`, `ArduinoJson`, `PubSubClient`, `WiFi` are already in their list.)

### 6. Build and flash

```bash
pio run -e esp32c6 --target upload --target monitor
```

You should see boot logs from both stacks:

```
[123][LED] init — 12 WS2812B on GPIO0
[145][MOTOR] init — AIN1=5 AIN2=6 @ 20000Hz
[167][ACCEL] LIS3DH ok — cal locked=-10.0° unlocked=70.0°
[189][NFC] PN532 fw=0x01060707
[HomeSpan] HomeSpan v1.9.1 — Initializing...
[HomeSpan] Pairing code: ...
```

### 7. Pair with Home app

1. Open the iPhone Home app → **+** → **Add Accessory**
2. Scan the QR code printed in the serial monitor (or enter the 8-digit code)
3. Once paired, open the lock's tile → **Settings** → **Express Mode**
4. Select the iPhone(s) and Apple Watch(es) to enroll
5. **Tap the antenna** with the phone — it should unlock

### 8. Calibrate position thresholds

Send these MQTT commands (or trigger via your config tool of choice) once
the deadbolt is in each position:

```bash
mosquitto_pub -t homekey/lock/cal/locked   -m ""   # while deadbolt is fully locked
mosquitto_pub -t homekey/lock/cal/unlocked -m ""   # while deadbolt is fully retracted
```

(You'll need to wire those topics in the overlay — they're not published
by HomeKey-ESP32's default MQTT handler. See `lock_controller_request_*`
for the API.)

## Files in this overlay

When using Option B, these files from our `firmware/src/` are dropped into
HomeKey-ESP32's `src/`:

| File                  | Purpose in overlay                                |
|-----------------------|---------------------------------------------------|
| `config.h`            | Pin/timing constants — replaces theirs             |
| `lock_state.h`        | State enum + stringifier                           |
| `motor.h/.cpp`        | DRV8833 PWM (replaces their digitalWrite)          |
| `accelerometer.h/.cpp`| LIS3DH position sensing (new feature)              |
| `leds.h/.cpp`         | WS2812B animations (replaces their status LED)     |
| `lock_controller.h/.cpp` | State machine glue called from their auth cb    |

These files are **NOT** used in overlay mode (they conflict with
HomeKey-ESP32's own implementation):

- `wifi_mgr.h/.cpp` — HomeKey-ESP32 has its own captive portal
- `mqtt.h/.cpp` — HomeKey-ESP32 has its own MQTT client
- `main.cpp` — HomeKey-ESP32 has its own entry point
- `nfc.h/.cpp` — HomeKey-ESP32 owns the PN532

The `[env:esp32c6-homekey]` build env in `platformio.ini` excludes them
automatically.

## Troubleshooting the overlay

**Symbol clashes (`multiple definition of …`)**
HomeKey-ESP32 may define `MOTOR_PIN`, `LED_PIN`, etc. in their `config.h`.
Wrap our defines in `#ifndef … #endif` or rename theirs.

**LIS3DH and PN532 fight on the SPI bus?**
They shouldn't — LIS3DH is on I²C in our wiring (GPIO19/20). If you see
SPI errors, double-check that GPIO19/20 aren't being remapped to SPI by
HomeKey-ESP32.

**Motor runs continuously after HomeKey unlock**
Their original code may call `digitalWrite(LOCK_PIN, HIGH)` and never
reset. Make sure you replaced **both** the on AND the implicit off — our
`lock_controller_request_unlock()` handles both.

**HomeKit shows JAMMED**
Calibrate the LIS3DH thresholds — the angles in `config.h` are placeholders.

## Reference

- HomeKey-ESP32 repo: https://github.com/rednblkx/HomeKey-ESP32
- HomeSpan docs: https://github.com/HomeSpan/HomeSpan
- This repo: https://github.com/leonseg/august-smart-lock-esp32-home-key-
