---
name: Bug Report
about: Report a problem with the August HomeKey mod
title: '[BUG] '
labels: bug
assignees: ''
---

## Description

A clear and concise description of what the bug is.

## Hardware

- **August Lock Model:** ASL-02 (Gen 2) / Other: ___
- **ESP32-C6 Board:** Super Mini / Other: ___
- **PN532 Module:** Elechouse Mini / Other: ___
- **NFC Antenna:** Yes (external IPEX) / No (onboard only)

## Firmware

- **HomeKey-ESP32 Version:** (e.g., v1.0.0)
- **Flash Method:** esptool / PlatformIO / Web flasher

## Steps to Reproduce

1. ...
2. ...
3. ...

## Expected Behavior

What you expected to happen.

## Actual Behavior

What actually happened.

## Serial Log

```
Paste relevant serial output here (from scripts/monitor.sh)
```

## Photos

If applicable, add photos of your wiring, PCB, or setup.

## Checklist

- [ ] I have read the [troubleshooting guide](../../docs/troubleshooting.md)
- [ ] I have verified my wiring with a multimeter
- [ ] I have checked the serial output for error messages
- [ ] I am using SPI mode for the PN532 (both DIP switches DOWN)
