/**
 * mqtt.h
 *
 * MQTT client + Home Assistant auto-discovery.
 *
 * Topics (defined in config.h):
 *   homekey/lock/state               — LOCKED / UNLOCKED / JAMMED / ...
 *   homekey/lock/set                 — LOCK / UNLOCK  (subscribed)
 *   homekey/lock/availability        — online / offline  (LWT)
 *   homekey/lock/battery             — battery % (optional)
 *   homekey/lock/wifi_rssi           — RSSI dBm
 *   homekey/lock/led/set             — ON / OFF
 *   homekey/lock/led/brightness/set  — 0..255
 *   homekey/lock/led/rgb/set         — "r,g,b"
 */

#ifndef MQTT_H
#define MQTT_H

#include "config.h"
#include "lock_state.h"

void mqtt_init();
void mqtt_loop();

bool mqtt_is_connected();

void mqtt_publish_state(lock_state_t s);
void mqtt_publish_availability(bool online);
void mqtt_publish_battery(uint8_t pct);
void mqtt_publish_rssi(int rssi);
void mqtt_publish_led_state();

/**
 * Fire-and-forget HA discovery. Called once after MQTT connects.
 */
void mqtt_publish_discovery();

#endif // MQTT_H
