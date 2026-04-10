# Troubleshooting

Common problems and solutions for the August HomeKey mod.

## NFC / HomeKey Issues

### HomeKey doesn't respond to iPhone tap

| Check | Fix |
|-------|-----|
| PN532 in SPI mode? | Both DIP switches must be DOWN |
| SPI wiring correct? | Verify SCK, MISO, MOSI, SS continuity |
| SPI pins configured in firmware? | Check web config: SCK=4, MISO=2, MOSI=3, SS=10 |
| External antenna connected? | Ensure IPEX connector is firmly seated |
| iPhone NFC enabled? | Settings → NFC must be on (iPhone XS+) |
| HomeKey enrolled? | Re-enroll in Home app → Lock settings → Express Mode |

### NFC reads are intermittent

- Check the IPEX connector — it can work loose. Add a small dot of hot glue to secure it.
- Ensure no metal is within 10mm behind the antenna.
- Reduce the distance between antenna and door surface.
- Try repositioning the antenna for better alignment.

### HomeKey works on iPhone but not Apple Watch

- Apple Watch has a weaker NFC antenna. The watch must be within ~2cm.
- Ensure the antenna is flush-mounted and as close to the surface as possible.
- Try tapping with the watch face (not the band side) directly on the antenna location.

## WS2812B LED Issues

### LEDs don't light up

| Check | Fix |
|-------|-----|
| Data pin correct? | Verify GPIO0 → 330Ω resistor → WS2812B DIN |
| Power connected? | Check 3.3V to WS2812B VCC |
| GND connected? | Verify common ground |
| LED count in config? | Must match actual number of LEDs |

### Wrong colors or flickering

- Add a **330Ω resistor** on the data line between GPIO0 and DIN
- Add a **100μF capacitor** across VCC/GND near the first LED
- Ensure solid solder joints — NeoPixel protocol is timing-sensitive

### Only the first LED works

- Check the data chain: DOUT of first LED → DIN of second LED
- A dead LED in the chain breaks all downstream LEDs

## Motor Issues

### Motor doesn't turn

| Check | Fix |
|-------|-----|
| Battery voltage? | Measure across battery holder — should be 5-6V (4xAA) |
| DRV8833 module VM? | Measure VM pin — should show 6V battery voltage |
| nSLEEP high? | Should be tied to 3.3V (module may have pull-up) |
| PWM signals? | Use oscilloscope or LED on GPIO5/6 to verify output |
| Motor wires connected? | Check AOUT1/AOUT2 → motor wire continuity |
| Correct GPIOs? | Verify AIN1=GPIO5, AIN2=GPIO6 in firmware config |

### Motor turns but doesn't lock/unlock fully

- **Timing too short:** Increase lock/unlock duration in config (default 1500ms, try 2000-3000ms)
- **PWM duty too low:** Increase duty cycle (try 100% / 255 for initial testing)
- **Gear binding:** Check for mechanical obstruction in the gear train
- **Low batteries:** Fresh alkaline or lithium AA batteries deliver more consistent voltage

### Motor runs continuously / won't stop

- **GPIO conflict:** Check that motor pins aren't shared with SPI or I2C
- **DRV8833 stuck:** Verify both AIN1 and AIN2 are controlled — setting both LOW = coast (stop)
- **Firmware crash:** Check serial log for crash/reboot loop. May need to reflash.

## Accelerometer (LIS3DH Module) Issues

### Can't communicate with LIS3DH

| Check | Fix |
|-------|-----|
| I2C address correct? | Try both 0x18 and 0x19. SDO pin LOW=0x18, HIGH=0x19 |
| I2C wiring? | Verify SDA=GPIO19, SCL=GPIO20 continuity |
| WHO_AM_I register? | Read register 0x0F — should return 0x33 |
| Module powered? | Verify 3.3V on module VCC pin |
| Bus conflict? | Ensure no SPI device is sharing GPIO19/20 |

> [!TIP]
> The LIS3DH breakout module includes onboard I2C pull-up resistors. No external pull-ups needed.

### Position detection unreliable

- **Recalibrate:** Read accelerometer values in both locked and unlocked positions. Update thresholds.
- **Mounting angle:** The LIS3DH relies on gravity direction. If the lock is mounted at an unusual angle, the axis thresholds change.
- **Module placement:** The LIS3DH module must be mounted on the rotating part of the lock (or in a fixed position that correlates with bolt position).
- **Vibration:** Door vibration can cause false readings. Add a moving average filter (sample 10 readings, average them).

## WiFi / Network Issues

### ESP32 won't connect to WiFi

| Check | Fix |
|-------|-----|
| SSID correct? | Case-sensitive, no trailing spaces |
| Password correct? | Special characters may need escaping in config |
| 2.4GHz network? | ESP32-C6 does not support 5GHz |
| Signal strength? | Lock may be too far from AP — check RSSI in serial log |
| AP mode still active? | If config failed, ESP32 falls back to AP mode |

### WiFi disconnects frequently

- Check signal strength (RSSI). Below -75dBm may cause drops.
- Consider a WiFi repeater near the door.
- Reduce other 2.4GHz interference (Zigbee, Bluetooth, microwaves).

## MQTT Issues

### Home Assistant doesn't see the lock

| Check | Fix |
|-------|-----|
| MQTT broker running? | `sudo systemctl status mosquitto` |
| Broker IP correct? | Verify in ESP32 config |
| Port correct? | Default 1883, check firewall |
| Credentials correct? | Test with `mosquitto_sub -h IP -u USER -P PASS -t '#'` |
| Topics matching? | HA expects `homekey/lock/state` — verify in ESP32 log |

### Lock state doesn't update in Home Assistant

- Check MQTT messages: `mosquitto_sub -h BROKER_IP -t 'homekey/#' -v`
- Verify the lock entity exists in HA: Settings → Devices & Services → MQTT
- Check `configuration.yaml` for correct topic names
- Restart Home Assistant after config changes

## Power Issues

### Lock eats through batteries quickly

- **WiFi is the main drain.** ESP32-C6 with WiFi active draws ~80-120mA.
- With 4xAA alkaline (~2500mAh effective), expect ~1-3 weeks battery life.
- **Improvements:**
  - Use lithium AA batteries (longer shelf life, more consistent voltage)
  - Implement WiFi sleep between operations (requires firmware modification)
  - Use PN532 IRQ pin to wake ESP32 only on NFC tap
  - Consider a USB power supply if an outlet is nearby

### ESP32 resets randomly

- **Check DC-DC step-down output:** Measure 3.3V rail with multimeter during motor operation. It should stay stable.
- Battery voltage may be dropping below the step-down's minimum input under load (motor start).
- Add a 470μF capacitor across the 3.3V output rail to handle transients.
- Check for loose wires on the proto PCB that may disconnect during motor vibration.

## Firmware Issues

### Boot loop (continuous restart)

1. Connect USB and monitor serial output
2. Look for crash backtrace or panic message
3. Common causes:
   - Wrong firmware binary (not ESP32-C6 build)
   - Flash corruption — erase and reflash: `esptool.py --chip esp32c6 erase_flash`
   - GPIO conflict causing watchdog timeout

### Can't access web config portal

- Connect to the `HomeKey-XXXX` AP first
- Try `http://192.168.4.1` in a browser
- If no AP appears, the ESP32 may have connected to a previously configured WiFi
- Factory reset: hold BOOT button for 10 seconds during power-on (check HomeKey-ESP32 docs for exact procedure)

### OTA update fails

- Ensure stable WiFi connection during update
- Check available flash space in serial log
- Try USB flash instead as a fallback

## Mechanical Issues

### Lock doesn't fully engage/disengage

- Motor timing may need adjustment (increase duration by 500ms increments)
- Check the gear train for debris or stripped gears
- Verify the tailpiece adapter is properly engaged with the deadbolt
- Test the deadbolt manually (with the lock body removed) to ensure smooth operation

### Lock is noisy

- 20kHz PWM should be inaudible. If you hear whining, verify PWM frequency is set to 20000Hz.
- Gear noise is mechanical — check for worn gears or insufficient lubrication.
- Apply a small amount of silicone grease to the gear train if needed.

## Getting Help

If none of the above solutions work:

1. **Check serial output** — most issues reveal themselves in the serial log
2. **Take photos** of your wiring and share them when asking for help
3. **Include your config** (remove WiFi password and MQTT credentials)
4. **Open an issue** on this repository with the above information
