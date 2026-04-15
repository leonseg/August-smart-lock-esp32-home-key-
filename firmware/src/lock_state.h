/**
 * lock_state.h
 *
 * Lock state machine types and helpers.
 */

#ifndef LOCK_STATE_H
#define LOCK_STATE_H

#include <Arduino.h>

typedef enum {
    LOCK_STATE_UNKNOWN,     // Initial state at boot
    LOCK_STATE_LOCKED,      // Deadbolt fully extended
    LOCK_STATE_UNLOCKED,    // Deadbolt fully retracted
    LOCK_STATE_LOCKING,     // Motor running in lock direction
    LOCK_STATE_UNLOCKING,   // Motor running in unlock direction
    LOCK_STATE_JAMMED,      // Motor timed out without reaching target
    LOCK_STATE_ERROR        // Hardware fault
} lock_state_t;

inline const char* lock_state_str(lock_state_t s) {
    switch (s) {
        case LOCK_STATE_UNKNOWN:    return "UNKNOWN";
        case LOCK_STATE_LOCKED:     return "LOCKED";
        case LOCK_STATE_UNLOCKED:   return "UNLOCKED";
        case LOCK_STATE_LOCKING:    return "LOCKING";
        case LOCK_STATE_UNLOCKING:  return "UNLOCKING";
        case LOCK_STATE_JAMMED:     return "JAMMED";
        case LOCK_STATE_ERROR:      return "ERROR";
        default:                    return "?";
    }
}

#endif // LOCK_STATE_H
