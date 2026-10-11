/**
 * motor.h
 *
 * DRV8833 motor driver interface.
 */

#ifndef MOTOR_H
#define MOTOR_H

#include "config.h"
#include "lock_state.h"

typedef enum {
    MOTOR_DIR_LOCK,     // Forward: AIN1=PWM, AIN2=0
    MOTOR_DIR_UNLOCK,   // Reverse: AIN1=0, AIN2=PWM
    MOTOR_DIR_BRAKE,    // Active braking: AIN1=HIGH, AIN2=HIGH
    MOTOR_DIR_COAST     // Free-spin: AIN1=0, AIN2=0
} motor_dir_t;

/**
 * Initialize the DRV8833 PWM channels.
 */
void motor_init();

/**
 * Drive the motor in the given direction with a given duty cycle (0-255).
 * Does not block — motor runs until motor_stop() is called.
 */
void motor_drive(motor_dir_t dir, uint8_t duty);

/**
 * Stop the motor (coast mode).
 */
void motor_stop();

/**
 * Brake the motor (active short).
 */
void motor_brake();

/**
 * Returns true if the motor is currently driven.
 */
bool motor_is_running();

// Bench diagnostics: drive each bridge input as a plain GPIO and read the pad
// back, to tell an ESP-side fault from a driver/power/wiring fault.
void motor_diag();
// Drive the bridge with plain DC levels (no PWM) for ms, then coast.
void motor_raw(motor_dir_t dir, uint32_t ms);

/**
 * Returns the current direction (only valid if motor_is_running()).
 */
motor_dir_t motor_current_direction();

#endif // MOTOR_H
