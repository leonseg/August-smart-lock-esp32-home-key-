# Flashing Guide

How to flash the HomeKey-ESP32 firmware onto the ESP32-C6 Super Mini.

## Prerequisites

- ESP32-C6 Super Mini connected via USB-C
- Python 3.x installed
- `esptool.py` installed (`pip install esptool`)
- Downloaded firmware binary from [HomeKey-ESP32 releases](https://github.com/rednblkx/HomeKey-ESP32/releases)

## Option 1: Flash Pre-built Binary (Recommended)

### Step 1: Download Firmware

1. Go to [HomeKey-ESP32 Releases](https://github.com/rednblkx/HomeKey-ESP32/releases)
2. Download the `.factory.bin` file for ESP32-C6
3. Place it in the `firmware/` directory

> [!NOTE]
> Look for a build specifically targeting the ESP32-C6. If only ESP32 or ESP32-S3 builds are available, you will need to build from source (Option 2).

### Step 2: Identify the Serial Port

**Linux:**
```bash
ls /dev/ttyUSB* /dev/ttyACM*
```

**macOS:**
```bash
ls /dev/cu.usbmodem*
```

**Windows:**
Check Device Manager → Ports (COM & LPT)

The ESP32-C6 Super Mini typically shows up as `/dev/ttyACM0` (Linux) or `/dev/cu.usbmodem*` (macOS) since it uses native USB-Serial.

### Step 3: Put ESP32-C6 in Download Mode

1. Hold the **BOOT** button on the ESP32-C6 Super Mini
2. While holding BOOT, press and release the **RESET** button
3. Release the BOOT button
4. The chip is now in download mode

> [!TIP]
> Some ESP32-C6 Super Mini boards enter download mode automatically when `esptool.py` connects. Try flashing without manual boot mode first.

### Step 4: Flash

Using the provided script:

```bash
./scripts/flash.sh /dev/ttyACM0
```

Or manually:

```bash
esptool.py --chip esp32c6 \
  --port /dev/ttyACM0 \
  --baud 460800 \
  write_flash 0x0 firmware/homekey-esp32c6.firmware.factory.bin
```

Expected output:
```
esptool.py v4.x
Serial port /dev/ttyACM0
Connecting...
Chip is ESP32-C6
...
Writing at 0x00000000... (1%)
...
Writing at 0x000xxxxx... (100%)
Wrote XXXXX bytes at 0x00000000
Hash of data verified.
Leaving...
Hard resetting via RTS pin...
```

### Step 5: Verify

1. Press the RESET button on the ESP32-C6
2. Monitor serial output:

```bash
./scripts/monitor.sh /dev/ttyACM0
```

3. You should see the HomeKey-ESP32 boot log with WiFi AP mode starting

## Option 2: Build from Source (PlatformIO)

Use this if no pre-built ESP32-C6 binary is available, or if you need to customize the firmware.

### Step 1: Clone HomeKey-ESP32

```bash
git clone https://github.com/rednblkx/HomeKey-ESP32.git
cd HomeKey-ESP32
```

### Step 2: Install PlatformIO

```bash
pip install platformio
```

Or use the PlatformIO IDE extension for VS Code.

### Step 3: Configure for ESP32-C6

Edit `platformio.ini` or use the provided config in this repo:

```bash
cp /path/to/this/repo/firmware/platformio.ini .
```

See [`firmware/platformio.ini`](../firmware/platformio.ini) for the ESP32-C6 configuration.

### Step 4: Build and Flash

```bash
pio run --target upload --upload-port /dev/ttyACM0
```

### Step 5: Monitor

```bash
pio device monitor --port /dev/ttyACM0 --baud 115200
```

## Initial Configuration

After flashing, the ESP32-C6 will start in AP (Access Point) mode:

1. Connect to the WiFi network: `HomeKey-XXXX` (XXXX = last 4 chars of MAC)
2. Open a browser and go to `http://192.168.4.1`
3. Configure:
   - **WiFi:** Your home network SSID and password
   - **MQTT:** Broker IP, port (1883), username, password
   - **HomeKit:** Setup code (8-digit, format XXX-XX-XXX)
   - **NFC Pins:** Set SPI pins to match your wiring (SCK=4, MISO=2, MOSI=3, SS=10)
   - **GPIO:** Configure motor and sensor pins
4. Save and reboot

> [!WARNING]
> Write down the HomeKit setup code — you'll need it to pair with Apple Home. If lost, you'll need to factory-reset the ESP32 and re-pair.

## Adding to Apple Home

1. Open the **Home** app on your iPhone
2. Tap **+** → **Add Accessory**
3. Select **More options...** or **I Don't Have a Code or Cannot Scan**
4. The lock should appear as a nearby accessory
5. Enter the HomeKit setup code
6. Assign to a room
7. The lock will appear as a Lock accessory in Apple Home

## Enabling HomeKey

After the lock is added to Apple Home:

1. In the Home app, long-press the lock accessory
2. Tap the gear icon (settings)
3. Look for the **Express Mode** or **HomeKey** option
4. Enable it and follow the prompts
5. Your iPhone/Watch will be enrolled for tap-to-unlock

> [!NOTE]
> HomeKey enrollment requires iOS 15+ and a device with NFC (iPhone XS or later). Apple Watch Series 4 or later supports HomeKey via wrist tap.

## Troubleshooting Flash Issues

| Problem | Solution |
|---------|----------|
| `Failed to connect` | Try manual boot mode (BOOT + RESET buttons) |
| `Wrong chip type` | Ensure `--chip esp32c6` is specified |
| `Permission denied on port` | Add user to dialout group: `sudo usermod -aG dialout $USER` |
| `Port not found` | Check USB cable (data cable, not charge-only) |
| `Write failed` | Try lower baud rate: `--baud 115200` |
| `Boot loop after flash` | Erase flash first: `esptool.py --chip esp32c6 erase_flash` |
