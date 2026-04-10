#!/bin/bash
#
# Monitor serial output from ESP32-C6 Super Mini
#
# Usage: ./scripts/monitor.sh [PORT] [BAUD]
#   PORT defaults to /dev/ttyACM0
#   BAUD defaults to 115200
#
# Prerequisites:
#   pip install pyserial (provides serial.tools.miniterm)
#
# Exit: Ctrl+] to quit miniterm

set -e

PORT="${1:-/dev/ttyACM0}"
BAUD="${2:-115200}"

echo "=== August HomeKey Serial Monitor ==="
echo "Port: $PORT"
echo "Baud: $BAUD"
echo "Exit: Ctrl+]"
echo ""

# Check if port exists
if [ ! -e "$PORT" ]; then
    echo "ERROR: Serial port not found: $PORT"
    echo ""
    echo "Available ports:"
    ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null || echo "  No serial ports detected"
    exit 1
fi

# Try pyserial miniterm first, fall back to screen
if python3 -c "import serial" 2>/dev/null; then
    python3 -m serial.tools.miniterm "$PORT" "$BAUD"
elif command -v screen &> /dev/null; then
    echo "pyserial not found, using screen instead"
    echo "Exit screen: Ctrl+A then K"
    screen "$PORT" "$BAUD"
else
    echo "ERROR: Neither pyserial nor screen found"
    echo "Install pyserial: pip install pyserial"
    exit 1
fi
