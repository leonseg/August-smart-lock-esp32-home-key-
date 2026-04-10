# Bill of Materials (BOM)

All components needed for the August Smart Lock Gen 2 → HomeKey modification.

## Core Components

| # | Component | Specification | Qty | Price (CAD) | Source | Notes |
|---|-----------|--------------|-----|-------------|--------|-------|
| 1 | ESP32-C6 Super Mini | WiFi 6 + BLE 5 + 802.15.4 | 1 | ~$4 | AliExpress | Main MCU, replaces STM32+CC2541 |
| 2 | Elechouse PN532 NFC MINI | V3, SPI/I2C/UART | 1 | ~$47 | Amazon / AliExpress | HomeKey NFC reader |
| 3 | NFC Antenna 13.56MHz | IPEX/U.FL connector, patch type | 1 | ~$3 | AliExpress | External antenna for door mounting |
| 4 | 30AWG Silicone Wire | Multi-color assortment | 1 set | ~$3 | AliExpress / Amazon | Point-to-point wiring |
| 5 | No-clean Flux | Paste or liquid | 1 | ~$4 | Amazon | For desoldering/soldering |

**Subtotal: ~$61 CAD**

## Tools Required (if not already owned)

| Tool | Purpose | Approx Price (CAD) |
|------|---------|-------------------|
| Soldering iron (temp controlled) | Fine pitch soldering | $30-80 |
| Hot air station | Desoldering QFP/QFN ICs | $50-100 |
| Solder wick / desoldering braid | Cleaning pads | $5 |
| Multimeter with continuity | PCB trace mapping | $20-40 |
| USB microscope or loupe | Inspecting fine-pitch solder joints | $20-40 |
| Tweezers (ESD-safe) | Handling small components | $5-10 |
| Chisel or router bit (15-20mm) | NFC antenna door pocket | $10-20 |
| Drill with 6mm bit | Cable routing hole in door | $5 |

## Optional Components

| Component | Purpose | Price (CAD) |
|-----------|---------|-------------|
| IPEX to SMA adapter | Testing NFC with external antenna | ~$3 |
| USB-C breakout board | Easier serial debug access | ~$2 |
| JST-SH connectors (1.0mm) | Modular connections instead of direct solder | ~$5 |
| Kapton tape | Insulating wire runs on PCB | ~$5 |
| Hot glue sticks | Securing wires and antenna | ~$3 |
| 4xAA batteries (lithium) | Longer battery life than alkaline | ~$10 |

## Component Specifications

### ESP32-C6 Super Mini

- **SoC:** ESP32-C6 (RISC-V single core, 160MHz)
- **Flash:** 4MB
- **PSRAM:** None
- **WiFi:** 802.11 b/g/n/ax (WiFi 6, 2.4GHz)
- **Bluetooth:** BLE 5.0
- **802.15.4:** Zigbee 3.0 / Thread
- **GPIO:** 23 (usable varies by board layout)
- **USB:** USB-C (native USB-Serial)
- **Size:** ~25 × 18mm
- **Operating voltage:** 3.3V (onboard regulator from 5V USB)

### Elechouse PN532 NFC MINI

- **Chip:** NXP PN532
- **Frequency:** 13.56 MHz
- **Protocols:** ISO 14443A/B, ISO 18092 (NFC), Mifare, NTAG
- **Interface:** SPI / I2C / UART (set via DIP switches)
- **Antenna:** Onboard + IPEX connector for external
- **Operating voltage:** 3.3V - 5V
- **Size:** ~28 × 40mm

> [!NOTE]
> The "MINI" version with the IPEX antenna connector is required. The standard PN532 breakout without IPEX will work for testing but cannot drive an external door-mounted antenna.

### NFC Antenna (13.56 MHz)

- **Frequency:** 13.56 MHz (ISO 14443)
- **Connector:** IPEX/U.FL
- **Size:** ~27 × 18mm (patch antenna)
- **Cable length:** 5-10cm typical
- **Mounting:** Adhesive backing

## Reused from August Lock

These components are already in the lock and are not purchased:

| Component | Function |
|-----------|----------|
| DRV8833 | Dual H-bridge motor driver |
| DC Motor + Gear train | Physical lock actuation |
| LIS3DHTR | 3-axis accelerometer (lock position) |
| 74AC541 | Octal buffer for LED ring |
| LED ring PCB | Status indication |
| 4xAA battery holder | Power supply (6V) |
| Voltage regulator | 6V → 3.3V conversion |
| Lock housing + mechanism | Physical lock body |

## Sourcing Tips

> [!TIP]
> - Order the ESP32-C6 Super Mini from AliExpress for the best price (~$4 CAD). Amazon prices are typically 3-4x higher.
> - The Elechouse PN532 MINI is the most expensive component. Clones exist on AliExpress for ~$10-15 but quality varies and the IPEX connector may be unreliable.
> - Order spare ESP32-C6 boards — they're cheap and you might damage one during soldering.
> - Get a multi-color 30AWG wire set so you can color-code your connections (e.g., red=3V3, black=GND, blue=SPI, green=I2C).
