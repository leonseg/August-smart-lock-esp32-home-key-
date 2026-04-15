/**
 * lock_controller.cpp
 *
 * Motor + accelerometer orchestration.
 */

#include "lock_controller.h"
#include "motor.h"
#include "accelerometer.h"

static lock_state_t    s_state = LOCK_STATE_UNKNOWN;
static uint32_t        s_motion_started = 0;
static uint32_t        s_settle_start = 0;
static bool            s_settling = false;
static lock_state_cb_t s_cb = nullptr;

static void set_state(lock_state_t ns) {
    if (ns == s_state) return;
    lock_state_t old = s_state;
    s_state = ns;
    LOG_TAG("LOCK", "%s -> %s", lock_state_str(old), lock_state_str(ns));
    if (s_cb) s_cb(old, ns);
}

void lock_controller_init() {
    motor_init();
    if (!accel_init()) {
        set_state(LOCK_STATE_ERROR);
        return;
    }
    // Sample a few readings to stabilise, then infer initial state
    for (int i = 0; i < 4; i++) { accel_poll(); delay(50); }
    lock_state_t inferred = accel_inferred_state();
    set_state(inferred);
}

lock_state_t lock_controller_state()     { return s_state; }
const char*  lock_controller_state_str() { return lock_state_str(s_state); }

static bool start_motion(motor_dir_t dir, lock_state_t transient) {
    if (s_state == LOCK_STATE_LOCKING || s_state == LOCK_STATE_UNLOCKING) {
        LOG_TAG("LOCK", "ignored request — already in motion");
        return false;
    }
    set_state(transient);
    motor_drive(dir, (dir == MOTOR_DIR_LOCK) ? MOTOR_DUTY_LOCK : MOTOR_DUTY_UNLOCK);
    s_motion_started = millis();
    s_settling = false;
    return true;
}

bool lock_controller_request_lock() {
    if (s_state == LOCK_STATE_LOCKED) return true;
    return start_motion(MOTOR_DIR_LOCK, LOCK_STATE_LOCKING);
}

bool lock_controller_request_unlock() {
    if (s_state == LOCK_STATE_UNLOCKED) return true;
    return start_motion(MOTOR_DIR_UNLOCK, LOCK_STATE_UNLOCKING);
}

bool lock_controller_request_toggle() {
    switch (s_state) {
        case LOCK_STATE_LOCKED:   return lock_controller_request_unlock();
        case LOCK_STATE_UNLOCKED: return lock_controller_request_lock();
        default: return false;
    }
}

void lock_controller_on_state_change(lock_state_cb_t cb) { s_cb = cb; }

void lock_controller_loop() {
    accel_poll();

    if (s_settling) {
        if (millis() - s_settle_start >= MOTOR_SETTLE_MS) {
            lock_state_t measured = accel_inferred_state();
            s_settling = false;
            if (measured == LOCK_STATE_LOCKED || measured == LOCK_STATE_UNLOCKED) {
                set_state(measured);
            } else {
                LOG_TAG("LOCK", "JAMMED — angle=%.1f° (locked=%.1f° unlocked=%.1f°)",
                        accel_angle_deg(), accel_angle_locked(), accel_angle_unlocked());
                set_state(LOCK_STATE_JAMMED);
            }
        }
        return;
    }

    if (s_state != LOCK_STATE_LOCKING && s_state != LOCK_STATE_UNLOCKING) return;

    uint32_t elapsed = millis() - s_motion_started;
    bool timeout = (s_state == LOCK_STATE_LOCKING)
                       ? (elapsed >= MOTOR_LOCK_TIMEOUT_MS)
                       : (elapsed >= MOTOR_UNLOCK_TIMEOUT_MS);

    // Early stop when the accelerometer confirms we've reached the target
    lock_state_t target = (s_state == LOCK_STATE_LOCKING) ? LOCK_STATE_LOCKED : LOCK_STATE_UNLOCKED;
    bool reached = (accel_inferred_state() == target) && (elapsed > 200);

    if (reached || timeout) {
        motor_brake();
        delay(50);
        motor_stop();
        s_settle_start = millis();
        s_settling = true;
        LOG_TAG("LOCK", "motion stopped — %s, elapsed=%lums", reached ? "reached" : "timeout", elapsed);
    }
}
