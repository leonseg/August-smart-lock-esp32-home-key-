/**
 * lock_controller.h
 *
 * High-level lock state machine. This is the API that the rest of the
 * firmware (and, in Option B overlay, HomeKey-ESP32's auth callback)
 * calls to request lock/unlock.
 *
 *   lock_controller_request_lock()      — start locking sequence
 *   lock_controller_request_unlock()    — start unlocking sequence
 *   lock_controller_state()             — current state
 *
 * The controller orchestrates motor + accelerometer:
 *   1. Set state to LOCKING/UNLOCKING
 *   2. Drive motor in target direction
 *   3. Poll accelerometer for target angle
 *   4. Stop + brake when target reached OR timeout expires
 *   5. Measure settled position → LOCKED / UNLOCKED / JAMMED
 */

#ifndef LOCK_CONTROLLER_H
#define LOCK_CONTROLLER_H

#include "config.h"
#include "lock_state.h"

typedef void (*lock_state_cb_t)(lock_state_t old_state, lock_state_t new_state);

void lock_controller_init();
void lock_controller_loop();

lock_state_t lock_controller_state();
const char*  lock_controller_state_str();

bool lock_controller_request_lock();
bool lock_controller_request_unlock();
bool lock_controller_request_toggle();

/**
 * Register a callback fired every time the state changes.
 * Used by MQTT and LED modules to publish/render updates.
 */
void lock_controller_on_state_change(lock_state_cb_t cb);

#endif // LOCK_CONTROLLER_H
