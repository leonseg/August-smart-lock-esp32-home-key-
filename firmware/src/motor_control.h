/**
 * motor_control.h
 *
 * Pin definitions and constants for controlling the August Smart Lock
 * DC motor via a DRV8833 breakout module on the proto PCB.
 *
 * The DRV8833 operates in H-bridge mode:
 *   - Forward:  AIN1 = PWM, AIN2 = LOW  (motor turns clockwise)
 *   - Reverse:  AIN1 = LOW, AIN2 = PWM  (motor turns counter-clockwise)
 *   - Brake:    AIN1 = HIGH, AIN2 = HIGH (active braking)
 *   - Coast:    AIN1 = LOW, AIN2 = LOW   (motor free-spins)
 *
 * Channel A drives the lock motor. Channel B is available for a buzzer or unused.
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

/* --- DRV8833 Channel A (Main Motor) --- */
#define MOTOR_AIN1_PIN      5   // GPIO5 — PWM output for forward rotation
#define MOTOR_AIN2_PIN      6   // GPIO6 — PWM output for reverse rotation

/* --- DRV8833 Channel B (available for buzzer or unused) --- */
#define MOTOR_BIN1_PIN      7   // GPIO7 — PWM output
#define MOTOR_BIN2_PIN      8   // GPIO8 — PWM output

/* --- WS2812B Status LEDs --- */
#define LED_DATA_PIN        0   // GPIO0 — WS2812B data (NeoPixel protocol)
#define LED_NUM_PIXELS     12   // Number of WS2812B LEDs in the ring
#define LED_BRIGHTNESS    128   // Default brightness (0-255)

/* --- DRV8833 Control --- */
// nSLEEP on the module is tied to 3.3V (always active).
// Most DRV8833 modules have an onboard pull-up on nSLEEP.
// If battery life is critical, use a GPIO to control sleep:
// #define MOTOR_NSLEEP_PIN  9

/* --- PWM Configuration --- */
// 20kHz PWM is above human hearing threshold — eliminates motor whine
#define MOTOR_PWM_FREQ      20000   // Hz
#define MOTOR_PWM_RESOLUTION    8   // 8-bit = 0-255 duty cycle
#define MOTOR_PWM_MAX_DUTY    255   // Full duty cycle

// LEDC channels for ESP32-C6 PWM
#define MOTOR_LEDC_CHANNEL_A1   0   // LEDC channel for AIN1
#define MOTOR_LEDC_CHANNEL_A2   1   // LEDC channel for AIN2
#define MOTOR_LEDC_CHANNEL_B1   2   // LEDC channel for BIN1
#define MOTOR_LEDC_CHANNEL_B2   3   // LEDC channel for BIN2

/* --- Timing --- */
// Duration (ms) the motor runs to fully lock or unlock.
// TODO: Calibrate these values with the actual lock mechanism.
//       Start with 1500ms and adjust based on LIS3DH position feedback.
#define MOTOR_LOCK_DURATION_MS      1500
#define MOTOR_UNLOCK_DURATION_MS    1500

// Delay after motor stops before reading position (let vibration settle)
#define MOTOR_SETTLE_DELAY_MS       200

/* --- Motor Duty Cycle --- */
// Start with full duty for reliable operation.
// Reduce if the motor is too fast or draws too much current.
#define MOTOR_LOCK_DUTY     255     // Duty cycle for locking (0-255)
#define MOTOR_UNLOCK_DUTY   255     // Duty cycle for unlocking (0-255)

/* --- Lock State --- */
typedef enum {
    LOCK_STATE_LOCKED,      // Deadbolt fully extended
    LOCK_STATE_UNLOCKED,    // Deadbolt fully retracted
    LOCK_STATE_MOVING,      // Motor is running
    LOCK_STATE_JAMMED,      // Motor timed out without reaching target position
    LOCK_STATE_UNKNOWN      // Position not yet determined (boot state)
} lock_state_t;

/* --- Motor Direction --- */
typedef enum {
    MOTOR_DIR_LOCK,         // AIN1=PWM, AIN2=LOW (forward)
    MOTOR_DIR_UNLOCK,       // AIN1=LOW, AIN2=PWM (reverse)
    MOTOR_DIR_BRAKE,        // AIN1=HIGH, AIN2=HIGH
    MOTOR_DIR_COAST         // AIN1=LOW, AIN2=LOW
} motor_direction_t;

/*
 * Usage example (Arduino-style):
 *
 *   #include "motor_control.h"
 *
 *   void setup() {
 *       ledcSetup(MOTOR_LEDC_CHANNEL_A1, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
 *       ledcSetup(MOTOR_LEDC_CHANNEL_A2, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
 *       ledcAttachPin(MOTOR_AIN1_PIN, MOTOR_LEDC_CHANNEL_A1);
 *       ledcAttachPin(MOTOR_AIN2_PIN, MOTOR_LEDC_CHANNEL_A2);
 *   }
 *
 *   void motor_lock() {
 *       ledcWrite(MOTOR_LEDC_CHANNEL_A1, MOTOR_LOCK_DUTY);   // Forward PWM
 *       ledcWrite(MOTOR_LEDC_CHANNEL_A2, 0);                  // Low
 *       delay(MOTOR_LOCK_DURATION_MS);
 *       ledcWrite(MOTOR_LEDC_CHANNEL_A1, 0);                  // Coast
 *       ledcWrite(MOTOR_LEDC_CHANNEL_A2, 0);
 *   }
 *
 *   void motor_unlock() {
 *       ledcWrite(MOTOR_LEDC_CHANNEL_A1, 0);                  // Low
 *       ledcWrite(MOTOR_LEDC_CHANNEL_A2, MOTOR_UNLOCK_DUTY); // Reverse PWM
 *       delay(MOTOR_UNLOCK_DURATION_MS);
 *       ledcWrite(MOTOR_LEDC_CHANNEL_A1, 0);                  // Coast
 *       ledcWrite(MOTOR_LEDC_CHANNEL_A2, 0);
 *   }
 *
 *   void motor_brake() {
 *       ledcWrite(MOTOR_LEDC_CHANNEL_A1, MOTOR_PWM_MAX_DUTY); // High
 *       ledcWrite(MOTOR_LEDC_CHANNEL_A2, MOTOR_PWM_MAX_DUTY); // High
 *   }
 */

#endif // MOTOR_CONTROL_H
