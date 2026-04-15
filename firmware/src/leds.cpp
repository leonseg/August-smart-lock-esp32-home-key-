/**
 * leds.cpp
 *
 * WS2812B ring animations driven by FastLED.
 */

#include "leds.h"
#include <FastLED.h>

static CRGB s_leds[NUM_LEDS];
static led_mode_t s_mode = LED_MODE_OFF;
static uint8_t s_brightness = LED_DEFAULT_BRIGHTNESS;
static CRGB s_custom = CRGB::White;
static uint32_t s_last_tick = 0;
static uint32_t s_anim_start = 0;
static uint8_t  s_anim_step = 0;

void leds_init() {
    FastLED.addLeds<WS2812B, PIN_LED_DATA, GRB>(s_leds, NUM_LEDS);
    FastLED.setBrightness(s_brightness);
    fill_solid(s_leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    LOG_TAG("LED", "init — %d WS2812B on GPIO%d", NUM_LEDS, PIN_LED_DATA);
}

void leds_set_mode(led_mode_t mode) {
    if (s_mode == mode) return;
    s_mode = mode;
    s_anim_start = millis();
    s_anim_step = 0;
}

void leds_set_brightness(uint8_t b) {
    s_brightness = b;
    FastLED.setBrightness(s_brightness);
}

void leds_set_custom_rgb(uint8_t r, uint8_t g, uint8_t b) {
    s_custom = CRGB(r, g, b);
    s_mode = LED_MODE_CUSTOM;
}

led_mode_t leds_mode() { return s_mode; }
uint8_t leds_brightness() { return s_brightness; }

led_mode_t leds_mode_for_state(lock_state_t s) {
    switch (s) {
        case LOCK_STATE_LOCKED:     return LED_MODE_LOCKED;
        case LOCK_STATE_UNLOCKED:   return LED_MODE_UNLOCKED;
        case LOCK_STATE_LOCKING:    return LED_MODE_LOCKING;
        case LOCK_STATE_UNLOCKING:  return LED_MODE_UNLOCKING;
        case LOCK_STATE_JAMMED:     return LED_MODE_JAMMED;
        case LOCK_STATE_ERROR:      return LED_MODE_ERROR;
        default:                    return LED_MODE_OFF;
    }
}

static void anim_solid(CRGB c) {
    fill_solid(s_leds, NUM_LEDS, c);
}

static void anim_breathe(CRGB c, uint32_t elapsed, uint16_t period_ms) {
    uint8_t lvl = (uint8_t)(((sinf((elapsed % period_ms) * 2.0f * (float)M_PI / period_ms) + 1.0f) / 2.0f) * 255.0f);
    CRGB col = c;
    col.nscale8(lvl);
    fill_solid(s_leds, NUM_LEDS, col);
}

static void anim_blink(CRGB c, uint32_t elapsed, uint16_t period_ms) {
    bool on = ((elapsed / (period_ms / 2)) & 1) == 0;
    fill_solid(s_leds, NUM_LEDS, on ? c : CRGB::Black);
}

static void anim_spin(CRGB c, uint32_t elapsed, uint16_t period_ms) {
    fill_solid(s_leds, NUM_LEDS, CRGB::Black);
    uint8_t head = (elapsed * NUM_LEDS / period_ms) % NUM_LEDS;
    for (int i = 0; i < 4; i++) {
        CRGB col = c;
        col.nscale8(255 - i * 60);
        s_leds[(head + NUM_LEDS - i) % NUM_LEDS] = col;
    }
}

static void anim_flash(CRGB c, uint32_t elapsed, uint16_t duration_ms) {
    if (elapsed < duration_ms) {
        fill_solid(s_leds, NUM_LEDS, c);
    } else {
        fill_solid(s_leds, NUM_LEDS, CRGB::Black);
    }
}

void leds_tick() {
    uint32_t now = millis();
    if (now - s_last_tick < 20) return;  // ~50 Hz refresh
    s_last_tick = now;
    uint32_t elapsed = now - s_anim_start;

    switch (s_mode) {
        case LED_MODE_OFF:            anim_solid(CRGB::Black); break;
        case LED_MODE_BOOT:           anim_breathe(CRGB::White, elapsed, 2000); break;
        case LED_MODE_AP_CONFIG:      anim_blink(CRGB::Blue, elapsed, 800); break;
        case LED_MODE_WIFI_CONNECT:   anim_spin(CRGB::Blue, elapsed, 1200); break;
        case LED_MODE_LOCKED:         anim_solid(CRGB::Red); break;
        case LED_MODE_UNLOCKED:       anim_solid(CRGB::Green); break;
        case LED_MODE_LOCKING:        anim_spin(CRGB::Red, elapsed, 800); break;
        case LED_MODE_UNLOCKING:      anim_spin(CRGB::Green, elapsed, 800); break;
        case LED_MODE_JAMMED:         anim_blink(CRGB::Orange, elapsed, 500); break;
        case LED_MODE_ERROR:          anim_blink(CRGB::Red, elapsed, 300); break;
        case LED_MODE_NFC_OK:         anim_flash(CRGB::Green, elapsed, 600); break;
        case LED_MODE_NFC_DENY:       anim_flash(CRGB::Red, elapsed, 600); break;
        case LED_MODE_CUSTOM:         anim_solid(s_custom); break;
    }
    FastLED.show();
}
