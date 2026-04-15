/**
 * leds.h
 *
 * WS2812B LED ring — status indicator.
 */

#ifndef LEDS_H
#define LEDS_H

#include "config.h"
#include "lock_state.h"

typedef enum {
    LED_MODE_OFF,
    LED_MODE_BOOT,         // White breathing
    LED_MODE_AP_CONFIG,    // Blue blink — AP config mode
    LED_MODE_WIFI_CONNECT, // Blue spin — connecting
    LED_MODE_LOCKED,       // Solid red
    LED_MODE_UNLOCKED,     // Solid green
    LED_MODE_LOCKING,      // Red spin
    LED_MODE_UNLOCKING,    // Green spin
    LED_MODE_JAMMED,       // Orange blink
    LED_MODE_ERROR,        // Red blink
    LED_MODE_NFC_OK,       // Green flash — valid tap
    LED_MODE_NFC_DENY,     // Red flash — unauthorized tap
    LED_MODE_CUSTOM        // User-set RGB color (MQTT)
} led_mode_t;

void leds_init();
void leds_set_mode(led_mode_t mode);
void leds_set_brightness(uint8_t b);         // 0-255
void leds_set_custom_rgb(uint8_t r, uint8_t g, uint8_t b);
void leds_tick();                             // Call from loop()

led_mode_t leds_mode();
uint8_t leds_brightness();

/**
 * Derive an automatic LED mode from a lock state.
 */
led_mode_t leds_mode_for_state(lock_state_t s);

#endif // LEDS_H
