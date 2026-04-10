# PCB Mapping — August Smart Lock Gen 2 (ASL-02)

This document is a template for reverse-engineering the August Smart Lock Gen 2 PCB. You must fill in the tables below by tracing each pin/pad with a multimeter in continuity mode.

> [!WARNING]
> **This is the most critical step of the project.** Incorrect pin mapping can destroy components. Take your time, verify every connection twice, and photograph everything.

## FCC Reference

- **Model:** ASL-02
- **FCC ID:** 2AB6UASL2
- FCC filings may contain internal photos — check [FCC ID search](https://www.fcc.gov/oet/ea/fccid) for reference images

## Equipment Needed

- Multimeter with continuity/beep mode
- Magnifying glass or USB microscope
- Good lighting (LED ring light recommended)
- Camera for HD photos
- Tweezers and probes
- Isopropyl alcohol + brush (to clean flux residue for clearer trace visibility)

## Step 1: Photography

Take the following photos before desoldering anything:

- [ ] PCB front (component side) — full board, in focus
- [ ] PCB back (solder side) — full board, in focus
- [ ] Close-up of STM32L152 and surrounding passives
- [ ] Close-up of CC2541 BLE chip and antenna trace
- [ ] Close-up of DRV8833 motor driver
- [ ] Close-up of LIS3DHTR accelerometer
- [ ] Close-up of 74AC541 LED driver
- [ ] Close-up of each connector (flex cables, battery, motor)
- [ ] Close-up of voltage regulator(s)

Place photos in the [`docs/images/`](images/) directory.

## Step 2: Identify ICs

| IC | Package | Marking | Datasheet | Location on PCB |
|----|---------|---------|-----------|----------------|
| STM32L152 | LQFP64 (?) | _to verify_ | [ST](https://www.st.com/en/microcontrollers-microprocessors/stm32l152.html) | _main MCU area_ |
| CC2541 | QFN-40 (?) | _to verify_ | [TI](https://www.ti.com/product/CC2541) | _near antenna_ |
| DRV8833 | WSON-10 or HTSSOP-16 | _to verify_ | [TI](https://www.ti.com/product/DRV8833) | _near motor connector_ |
| LIS3DHTR | LGA-16 | _to verify_ | [ST](https://www.st.com/en/mems-and-sensors/lis3dhtr.html) | _center of PCB?_ |
| 74AC541 | TSSOP-20 (?) | _to verify_ | Various | _near LED connector_ |
| Voltage Reg | SOT-23-5 (?) | _to verify_ | _TBD_ | _near battery connector_ |

## Step 3: Trace STM32L152 Pins

For each pin of the STM32, use the multimeter continuity mode to trace where it connects. Mark the function based on the STM32L152 datasheet and the destination component.

| STM32 Pin # | STM32 Function | Trace Goes To | Component | Signal | Notes |
|-------------|---------------|---------------|-----------|--------|-------|
| 1 | | | | | |
| 2 | | | | | |
| 3 | | | | | |
| 4 | | | | | |
| 5 | | | | | |
| ... | | | | | |

> [!TIP]
> Focus on pins that connect to the DRV8833, LIS3DHTR, and 74AC541 first. Those are the ones you need to rewire to the ESP32-C6.

### Key STM32L152 Pins to Look For

| Function | Expected STM32 Pin Type | Destination |
|----------|------------------------|-------------|
| Motor PWM (AIN1) | Timer output (TIMx_CHx) | DRV8833 pin 2 |
| Motor PWM (AIN2) | Timer output (TIMx_CHx) | DRV8833 pin 7 |
| Motor PWM (BIN1) | Timer output (TIMx_CHx) | DRV8833 pin 10 |
| Motor PWM (BIN2) | Timer output (TIMx_CHx) | DRV8833 pin 15 |
| I2C SDA | I2Cx_SDA | LIS3DHTR pin 4 |
| I2C SCL | I2Cx_SCL | LIS3DHTR pin 6 |
| SPI/I2C to CC2541 | SPI or I2C | CC2541 |
| LED data/enable | GPIO | 74AC541 inputs |
| Button input | GPIO (input, pull-up) | Button connector |
| Battery ADC | ADC input | Voltage divider |

## Step 4: Trace CC2541 Pins

| CC2541 Pin # | CC2541 Function | Trace Goes To | Component | Signal | Notes |
|-------------|----------------|---------------|-----------|--------|-------|
| 1 | | | | | |
| 2 | | | | | |
| ... | | | | | |

> [!NOTE]
> The CC2541 pins are less critical since we're removing BLE entirely. However, documenting them helps identify shared traces that might be affected when desoldering.

## Step 5: Power Rails

### 6V Motor Rail

| Test Point | Voltage | Connects To | Notes |
|-----------|---------|------------|-------|
| Battery + | 6V | | Main battery positive |
| DRV8833 VM | 6V | | Motor supply |
| | | | |

### 3.3V Logic Rail

| Test Point | Voltage | Connects To | Notes |
|-----------|---------|------------|-------|
| Regulator OUT | 3.3V | | |
| STM32 VDD | 3.3V | | |
| CC2541 VDD | 3.3V | | |
| LIS3DHTR VDD | 3.3V | | |
| DRV8833 VCC | 3.3V (?) | | Logic supply, verify |
| 74AC541 VCC | 3.3V | | |

### Ground

Verify that all GND pins are connected to a common ground plane:

- [ ] STM32 GND pins
- [ ] CC2541 GND pins/pad
- [ ] DRV8833 GND pins
- [ ] LIS3DHTR GND
- [ ] 74AC541 GND
- [ ] Battery negative
- [ ] Motor connector GND

## Step 6: Connectors

### Main Flex Connector

| Pin # | Signal | Voltage | Connects To | Notes |
|-------|--------|---------|------------|-------|
| 1 | | | | |
| 2 | | | | |
| 3 | | | | |
| ... | | | | |

### LED Flex Connector

| Pin # | Signal | Voltage | Connects To | Notes |
|-------|--------|---------|------------|-------|
| 1 | | | | |
| 2 | | | | |
| ... | | | | |

### Motor Connector

| Pin # | Signal | Voltage | Connects To | Notes |
|-------|--------|---------|------------|-------|
| 1 | | | | |
| 2 | | | | |
| ... | | | | |

## Step 7: Decoupling and Passives

Note any important passives near the MCU:

| Component | Value | Connected Between | Purpose |
|-----------|-------|-------------------|---------|
| C? | _pF/nF_ | STM32 VDD-GND | Decoupling |
| R? | _kΩ_ | | Pull-up/down |
| ... | | | |

## Post-Mapping Checklist

After completing the trace mapping:

- [ ] All DRV8833 control pins identified
- [ ] LIS3DHTR I2C address confirmed (SDO pin high=0x19 or low=0x18)
- [ ] 74AC541 enable and data pins identified
- [ ] Power regulator input/output identified
- [ ] No unexpected connections between components
- [ ] Pin mapping table in [`hardware/pinout-mapping.md`](../hardware/pinout-mapping.md) updated
- [ ] Wiring guide in [`wiring-guide.md`](wiring-guide.md) updated with actual pin assignments
- [ ] Photos placed in [`docs/images/`](images/)
