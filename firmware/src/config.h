/**
 * config.h
 *
 * Central configuration for the August Smart Lock ESP32-C6 firmware.
 * All pin definitions, timing constants, and default values are here.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// Firmware identity
// ============================================================================
#define FW_NAME             "August-HomeKey-ESP32C6"
#define FW_VERSION          "1.0.0"
#define FW_AUTHOR           "leonseg"

// ============================================================================
// Pin assignments (ESP32-C6 Super Mini)
// ============================================================================

// --- DRV8833 Motor Driver Module ---
#define PIN_MOTOR_AIN1      5   // PWM — forward
#define PIN_MOTOR_AIN2      6   // PWM — reverse
#define PIN_MOTOR_BIN1      7   // PWM — channel B (unused)
#define PIN_MOTOR_BIN2      8   // PWM — channel B (unused)
// DRV8833 nSLEEP is tied to 3.3V on the proto PCB

// --- LIS3DH Accelerometer Module (I2C) ---
#define PIN_I2C_SDA         19
#define PIN_I2C_SCL         20
#define PIN_ACCEL_INT1      9   // Optional motion interrupt
#define LIS3DH_ADDR         0x18  // SDO=GND; 0x19 if SDO=VDD

// --- PN532 NFC Module (SPI) ---
#define PIN_NFC_SCK         4
#define PIN_NFC_MISO        2
#define PIN_NFC_MOSI        3
#define PIN_NFC_SS          10
#define PIN_NFC_IRQ         1   // Optional — wake on NFC tap

// --- WS2812B LED Ring ---
#define PIN_LED_DATA        0
#define NUM_LEDS            12
#define LED_DEFAULT_BRIGHTNESS  128

// --- Battery monitoring ---
#define PIN_BATTERY_ADC     21  // Optional — via voltage divider

// ============================================================================
// Motor control
// ============================================================================
#define MOTOR_PWM_FREQ_HZ   20000   // 20kHz — above audible range
#define MOTOR_PWM_RES_BITS  8       // 0-255 duty
#define MOTOR_PWM_MAX_DUTY  255

#define MOTOR_LEDC_CH_AIN1  0
#define MOTOR_LEDC_CH_AIN2  1
#define MOTOR_LEDC_CH_BIN1  2
#define MOTOR_LEDC_CH_BIN2  3

// Duration (ms) the motor runs to lock/unlock
// These are safety timeouts — the accelerometer ends motion earlier if position reached
#define MOTOR_LOCK_TIMEOUT_MS       3000
#define MOTOR_UNLOCK_TIMEOUT_MS     3000
#define MOTOR_SETTLE_MS             300   // Wait after stopping before reading position

// Motor duty cycle (0-255)
#define MOTOR_DUTY_LOCK     255
#define MOTOR_DUTY_UNLOCK   255

// ============================================================================
// Accelerometer
// ============================================================================
#define ACCEL_SAMPLE_RATE_HZ        10      // Hz
#define ACCEL_RANGE_G               2       // ±2g
#define ACCEL_POLL_INTERVAL_MS      250     // How often to poll position

// Angle thresholds (degrees) — calibrate for your specific lock orientation
// These are default placeholders; runtime calibration overrides them.
#define ACCEL_ANGLE_LOCKED_DEG      -10.0f  // Angle when locked
#define ACCEL_ANGLE_UNLOCKED_DEG    70.0f   // Angle when unlocked
#define ACCEL_ANGLE_TOLERANCE_DEG   15.0f   // ± tolerance for position match

// ============================================================================
// WiFi
// ============================================================================
#define WIFI_AP_SSID_PREFIX "AugustHomeKey-"
#define WIFI_AP_PASSWORD    "configure123"    // AP mode password (8+ chars)
#define WIFI_CONNECT_TIMEOUT_MS 20000
#define WIFI_RETRY_INTERVAL_MS  60000

// ============================================================================
// MQTT
// ============================================================================
#define MQTT_DEFAULT_PORT       1883
#define MQTT_KEEPALIVE_SEC      60
#define MQTT_RECONNECT_INTERVAL_MS  5000
#define MQTT_PUBLISH_INTERVAL_MS    10000   // Heartbeat / state refresh

#define MQTT_TOPIC_STATE            "homekey/lock/state"
#define MQTT_TOPIC_CMD              "homekey/lock/set"
#define MQTT_TOPIC_AVAILABILITY     "homekey/lock/availability"
#define MQTT_TOPIC_BATTERY          "homekey/lock/battery"
#define MQTT_TOPIC_RSSI             "homekey/lock/wifi_rssi"
#define MQTT_TOPIC_LED_SET          "homekey/lock/led/set"
#define MQTT_TOPIC_LED_STATE        "homekey/lock/led/state"
#define MQTT_TOPIC_LED_BRIGHTNESS   "homekey/lock/led/brightness"
#define MQTT_TOPIC_LED_BRIGHTNESS_SET "homekey/lock/led/brightness/set"
#define MQTT_TOPIC_LED_RGB          "homekey/lock/led/rgb"
#define MQTT_TOPIC_LED_RGB_SET      "homekey/lock/led/rgb/set"

#define MQTT_PAYLOAD_LOCK           "LOCK"
#define MQTT_PAYLOAD_UNLOCK         "UNLOCK"
#define MQTT_PAYLOAD_LOCKED         "LOCKED"
#define MQTT_PAYLOAD_UNLOCKED       "UNLOCKED"
#define MQTT_PAYLOAD_ONLINE         "online"
#define MQTT_PAYLOAD_OFFLINE        "offline"

// ============================================================================
// NFC / HomeKey
// ============================================================================
#define NFC_POLL_INTERVAL_MS        500
#define NFC_MAX_AUTHORIZED_UIDS     16
#define NFC_UID_MAX_LEN             7

// ============================================================================
// Storage (NVS) keys
// ============================================================================
#define NVS_NAMESPACE       "august"
#define NVS_KEY_WIFI_SSID   "wifi_ssid"
#define NVS_KEY_WIFI_PASS   "wifi_pass"
#define NVS_KEY_MQTT_HOST   "mqtt_host"
#define NVS_KEY_MQTT_PORT   "mqtt_port"
#define NVS_KEY_MQTT_USER   "mqtt_user"
#define NVS_KEY_MQTT_PASS   "mqtt_pass"
#define NVS_KEY_ACCEL_CAL_L "ac_locked"    // Calibrated locked angle
#define NVS_KEY_ACCEL_CAL_U "ac_unlocked"  // Calibrated unlocked angle
#define NVS_KEY_NFC_UIDS    "nfc_uids"     // Authorized NFC UIDs

// ============================================================================
// Debug
// ============================================================================
#define DEBUG_SERIAL        1
#define DEBUG_BAUD          115200

#if DEBUG_SERIAL
  #define LOG(fmt, ...)     Serial.printf("[%lu] " fmt "\n", millis(), ##__VA_ARGS__)
  #define LOG_TAG(tag, fmt, ...) Serial.printf("[%lu][%s] " fmt "\n", millis(), tag, ##__VA_ARGS__)
#else
  #define LOG(fmt, ...)
  #define LOG_TAG(tag, fmt, ...)
#endif

#endif // CONFIG_H
