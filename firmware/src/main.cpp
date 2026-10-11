/**
 * main.cpp
 *
 * Entry point for the August HomeKey ESP32-C6 firmware (standalone mode).
 *
 * Responsibilities:
 *   - Bring up all hardware modules
 *   - Run the super-loop calling each module's loop()
 *   - Wire together state changes → LEDs + MQTT
 *   - NFC auth callback → unlock request
 *
 * For the Option B HomeKey overlay build, this file is NOT compiled — the
 * HomeKey-ESP32 main takes over and calls lock_controller_* from its own
 * auth callback. See HOMEKEY-INTEGRATION.md.
 */

#include <Arduino.h>

#include "config.h"
#include "lock_state.h"
#include "lock_controller.h"
#include "leds.h"
#include "nfc.h"
#include "wifi_mgr.h"
#include "mqtt.h"
#if SERIAL_TEST_CONSOLE
#include <Preferences.h>
#include "motor.h"
#endif

static void on_lock_state_change(lock_state_t /*old_s*/, lock_state_t new_s) {
    leds_set_mode(leds_mode_for_state(new_s));
    mqtt_publish_state(new_s);
}

static void on_nfc_tap(bool authorized, const uint8_t* uid, uint8_t uid_len) {
    (void)uid; (void)uid_len;
    if (authorized) {
        leds_set_mode(LED_MODE_NFC_OK);
        // Toggle: tap-to-unlock if locked, tap-to-lock if unlocked
        lock_controller_request_toggle();
    } else {
        leds_set_mode(LED_MODE_NFC_DENY);
    }
}

static void update_led_for_wifi() {
    switch (wifi_mgr_state()) {
        case WIFI_STATE_AP_CONFIG:    leds_set_mode(LED_MODE_AP_CONFIG); break;
        case WIFI_STATE_CONNECTING:   leds_set_mode(LED_MODE_WIFI_CONNECT); break;
        case WIFI_STATE_CONNECTED:    leds_set_mode(leds_mode_for_state(lock_controller_state())); break;
        default: break;
    }
}

#if SERIAL_TEST_CONSOLE
// Bench-test console over USB serial (esp32c6-bench env only). Lets the
// hardware be exercised without WiFi/MQTT:
//   status | lock | unlock | pulse <lock|unlock> <ms> | stop
//   led <mode 0-12> | rgb <r> <g> <b> | bri <0-255>
//   wifi <ssid>|<pass>   mqtt <host>|<port>|<user>|<pass>   (saved to NVS)
//   reboot
static void console_exec(String line) {
    line.trim();
    if (line.isEmpty()) return;
    char cmd[12] = {0}, arg[12] = {0};
    int a = 0, b = 0, c = 0;

    if (line == "status") {
        LOG_TAG("TEST", "lock=%s wifi=%d ip=%s led_mode=%d bri=%u motor_running=%d mqtt=%d",
                lock_controller_state_str(), wifi_mgr_state(), wifi_mgr_ip().c_str(),
                leds_mode(), leds_brightness(), motor_is_running(), mqtt_is_connected());
    } else if (line == "lock") {
        LOG_TAG("TEST", "request_lock -> %d", lock_controller_request_lock());
    } else if (line == "unlock") {
        LOG_TAG("TEST", "request_unlock -> %d", lock_controller_request_unlock());
    } else if (line == "stop") {
        motor_stop();
    } else if (line == "reboot") {
        ESP.restart();
    } else if (line.startsWith("wifi ")) {
        int sep = line.indexOf('|');
        String ssid = (sep < 0) ? line.substring(5) : line.substring(5, sep);
        String pass = (sep < 0) ? String("") : line.substring(sep + 1);
        LOG_TAG("TEST", "wifi saved ssid='%s' -> %d (reboot to apply)", ssid.c_str(),
                wifi_mgr_save_credentials(ssid, pass));
    } else if (line.startsWith("mqtt ")) {
        // host|port|user|pass — trailing fields optional
        String f[4]; int n = 0, from = 5;
        while (n < 4) {
            int sep = line.indexOf('|', from);
            f[n++] = (sep < 0) ? line.substring(from) : line.substring(from, sep);
            if (sep < 0) break;
            from = sep + 1;
        }
        Preferences p;
        p.begin(NVS_NAMESPACE, false);
        p.putString(NVS_KEY_MQTT_HOST, f[0]);
        p.putUInt(NVS_KEY_MQTT_PORT, f[1].isEmpty() ? MQTT_DEFAULT_PORT : f[1].toInt());
        p.putString(NVS_KEY_MQTT_USER, f[2]);
        p.putString(NVS_KEY_MQTT_PASS, f[3]);
        p.end();
        LOG_TAG("TEST", "mqtt saved host='%s' user='%s' (reboot to apply)", f[0].c_str(), f[2].c_str());
    } else if (sscanf(line.c_str(), "%11s %11s %d", cmd, arg, &a) == 3 && !strcmp(cmd, "pulse")) {
        a = constrain(a, 10, 1000);
        motor_drive(!strcmp(arg, "unlock") ? MOTOR_DIR_UNLOCK : MOTOR_DIR_LOCK, MOTOR_PWM_MAX_DUTY);
        delay(a);
        motor_stop();
        LOG_TAG("TEST", "pulse %s %dms done", arg, a);
    } else if (sscanf(line.c_str(), "led %d", &a) == 1) {
        leds_set_mode((led_mode_t)constrain(a, 0, (int)LED_MODE_CUSTOM));
        LOG_TAG("TEST", "led_mode=%d", leds_mode());
    } else if (sscanf(line.c_str(), "rgb %d %d %d", &a, &b, &c) == 3) {
        leds_set_custom_rgb((uint8_t)a, (uint8_t)b, (uint8_t)c);
        LOG_TAG("TEST", "rgb=%d,%d,%d led_mode=%d", a, b, c, leds_mode());
    } else if (sscanf(line.c_str(), "bri %d", &a) == 1) {
        leds_set_brightness((uint8_t)constrain(a, 0, 255));
        LOG_TAG("TEST", "bri=%u", leds_brightness());
    } else {
        LOG_TAG("TEST", "unknown command '%s'", line.c_str());
    }
}

static void console_poll() {
    static String buf;
    while (Serial.available()) {
        char ch = (char)Serial.read();
        if (ch == '\n' || ch == '\r') { console_exec(buf); buf = ""; }
        else if (buf.length() < 160) buf += ch;
    }
}
#endif

void setup() {
    Serial.begin(DEBUG_BAUD);
    delay(200);
    LOG("boot — %s v%s", FW_NAME, FW_VERSION);

    leds_init();
    leds_set_mode(LED_MODE_BOOT);

    lock_controller_init();
    lock_controller_on_state_change(on_lock_state_change);

    nfc_init();
    nfc_set_auth_callback(on_nfc_tap);

    wifi_mgr_init();
    mqtt_init();

    update_led_for_wifi();
    LOG("setup complete");
}

void loop() {
    lock_controller_loop();
    leds_tick();
    nfc_poll();
    wifi_mgr_loop();
    mqtt_loop();
#if SERIAL_TEST_CONSOLE
    console_poll();
#endif

    // If wifi transitioned, refresh LED mode (unless a transient animation is active)
    static wifi_state_t last_wifi = (wifi_state_t)-1;
    if (wifi_mgr_state() != last_wifi) {
        last_wifi = wifi_mgr_state();
        if (leds_mode() == LED_MODE_BOOT || leds_mode() == LED_MODE_AP_CONFIG ||
            leds_mode() == LED_MODE_WIFI_CONNECT) {
            update_led_for_wifi();
        }
    }
}
