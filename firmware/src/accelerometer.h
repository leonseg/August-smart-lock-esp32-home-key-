/**
 * accelerometer.h
 *
 * LIS3DH position sensor interface — detects deadbolt position by
 * measuring gravity direction on the rotating part of the lock body.
 */

#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "config.h"
#include "lock_state.h"

/**
 * Initialize the LIS3DH on the shared I2C bus.
 * Returns true on success, false if the sensor did not respond.
 */
bool accel_init();

/**
 * Poll the accelerometer. Call from loop().
 * Returns true if a new reading was taken.
 */
bool accel_poll();

/**
 * Current angle in degrees, computed from X/Z axes.
 */
float accel_angle_deg();

/**
 * Best-guess lock state from the current angle compared against
 * the calibrated locked/unlocked angles.
 *
 * Returns LOCK_STATE_LOCKED, LOCK_STATE_UNLOCKED, or LOCK_STATE_UNKNOWN
 * if the angle falls outside both tolerance windows.
 */
lock_state_t accel_inferred_state();

/**
 * Store the current angle as the calibrated LOCKED or UNLOCKED reference
 * and persist it to NVS.
 */
void accel_calibrate_locked();
void accel_calibrate_unlocked();

/**
 * Accessors for the calibrated reference angles.
 */
float accel_angle_locked();
float accel_angle_unlocked();

#endif // ACCELEROMETER_H
