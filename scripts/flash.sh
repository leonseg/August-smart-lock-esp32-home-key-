#!/bin/bash
#
# Flash HomeKey-ESP32 firmware to ESP32-C6 Super Mini
#
# Usage: ./scripts/flash.sh [PORT]
#   PORT defaults to /dev/ttyACM0 (typical for ESP32-C6 native USB)
#
# Prerequisites:
#   pip install esptool

set -e

PORT="${1:-/dev/ttyACM0}"
FIRMWARE="firmware/homekey-esp32c6.firmware.factory.bin"

echo "=== August HomeKey ESP32-C6 Flasher ==="
echo ""

# Check if esptool is installed
if ! command -v esptool.py &> /dev/null; then
    echo "ERROR: esptool.py not found"
    echo "Install with: pip install esptool"
    exit 1
fi

# Check if firmware file exists
if [ ! -f "$FIRMWARE" ]; then
    echo "ERROR: Firmware file not found: $FIRMWARE"
    echo ""
    echo "Download the latest .factory.bin from:"
    echo "  https://github.com/rednblkx/HomeKey-ESP32/releases"
    echo ""
    echo "Place it at: $FIRMWARE"
    exit 1
fi

# Check if port exists
if [ ! -e "$PORT" ]; then
    echo "ERROR: Serial port not found: $PORT"
    echo ""
    echo "Available ports:"
    ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null || echo "  No serial ports detected"
    echo ""
    echo "Make sure the ESP32-C6 is connected via USB-C"
    exit 1
fi

echo "Port:     $PORT"
echo "Firmware: $FIRMWARE"
echo ""
echo "If the flash fails, put the ESP32-C6 in download mode:"
echo "  1. Hold BOOT button"
echo "  2. Press and release RESET"
echo "  3. Release BOOT"
echo ""
read -p "Press Enter to start flashing..."

esptool.py \
    --chip esp32c6 \
    --port "$PORT" \
    --baud 460800 \
    write_flash 0x0 "$FIRMWARE"

echo ""
echo "=== Flash complete ==="
echo "Press RESET on the ESP32-C6 to boot the firmware."
echo "Then connect to the HomeKey-XXXX WiFi AP to configure."
