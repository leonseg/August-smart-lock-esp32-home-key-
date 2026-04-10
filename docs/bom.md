# Bill of Materials (BOM)

All components needed for the August Smart Lock Gen 2 → HomeKey modification.

## Core Components

| # | Component | Specification | Qty | Price (CAD) | Source | Notes |
|---|-----------|--------------|-----|-------------|--------|-------|
| 1 | ESP32-C6 Super Mini | WiFi 6 + BLE 5 + 802.15.4 | 1 | ~$4 | AliExpress | Main MCU |
| 2 | Elechouse PN532 NFC MINI | V3, SPI/I2C/UART | 1 | ~$47 | Amazon / AliExpress | HomeKey NFC reader |
| 3 | NFC Antenna 13.56MHz | IPEX/U.FL connector, patch type | 1 | ~$3 | AliExpress | External door antenna |
| 4 | LIS3DH Breakout Module | 3-axis accelerometer, I2C/SPI | 1 | ~$3 | AliExpress | Lock position sensing |
| 5 | DRV8833 Motor Driver Module | Dual H-bridge breakout, 1.5A/ch | 1 | ~$6 | AliExpress | DC motor control |
| 6 | DC-DC Step-Down Module | 5V→3.3V buck converter | 1 | ~$2 | AliExpress | 6V battery → 3.3V logic |
| 7 | WS2812B LED Ring/Strip | Addressable RGB, various sizes | 1 | ~$3 | AliExpress | Status LED ring |
| 8 | Prototype PCB Board | Various sizes, pack | 1 | ~$2 | AliExpress | Mounting board for modules |
| 9 | 30AWG Silicone Wire | Multi-color assortment | 1 set | ~$3 | AliExpress | Point-to-point wiring |
| 10 | No-clean Flux | Paste or liquid | 1 | ~$4 | AliExpress / Amazon | Soldering |

**Subtotal: ~$77 CAD**

## Tools Required (if not already owned)

| Tool | Purpose | Approx Price (CAD) |
|------|---------|-------------------|
| Soldering iron (temp controlled) | Header pins and wiring | $30-80 |
| Solder (leaded or lead-free) | Joining connections | $5-10 |
| Multimeter with continuity | Verifying connections | $20-40 |
| Tweezers (ESD-safe) | Handling small wires | $5-10 |
| Wire strippers (30AWG capable) | Stripping fine wire | $10 |
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

Only the mechanical components are reused — the entire original PCB is removed:

| Component | Function |
|-----------|----------|
| DC Motor + Gear train | Physical lock actuation |
| 4xAA battery holder | Power supply (6V) |
| Lock housing + mechanism | Physical lock body |
| Thumb-turn + tailpiece | Manual operation + deadbolt adapter |

### DRV8833 Motor Driver Module

- **IC:** TI DRV8833
- **Channels:** 2 (A and B), 1.5A continuous per channel
- **Input voltage:** 2.7V – 10.8V (VM pin)
- **Control:** PWM on AIN1/AIN2 and BIN1/BIN2 header pins
- **Features:** nSLEEP pin (pull-up on most modules), nFAULT output

### LIS3DH Accelerometer Module

- **IC:** ST LIS3DH
- **Interface:** I2C (default) or SPI
- **Range:** ±2g / ±4g / ±8g / ±16g
- **I2C pull-ups:** Included on module
- **I2C Address:** 0x18 (SDO→GND) or 0x19 (SDO→VDD)

### DC-DC Step-Down Module

- **Input:** 4.5V – 7V (from 4xAA battery pack)
- **Output:** 3.3V fixed
- **Current:** Typically 500mA–1A (varies by module)
- **Efficiency:** ~90% (much better than a linear LDO)

### WS2812B LEDs

- **Protocol:** Single-wire NeoPixel (800kHz)
- **Voltage:** 5V nominal (works at 3.3V for short runs)
- **Current:** ~20mA per LED at moderate brightness, 60mA at full white
- **Data pin:** Single GPIO (GPIO0)

## Sourcing Tips

> [!TIP]
> - Order the ESP32-C6 Super Mini from AliExpress for the best price (~$4 CAD). Amazon prices are typically 3-4x higher.
> - The Elechouse PN532 MINI is the most expensive component. Clones exist on AliExpress for ~$10-15 but quality varies and the IPEX connector may be unreliable.
> - Order spare ESP32-C6 boards — they're cheap and you might damage one during soldering.
> - Get a multi-color 30AWG wire set so you can color-code your connections (e.g., red=3V3, black=GND, blue=SPI, green=I2C).
