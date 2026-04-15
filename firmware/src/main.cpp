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
