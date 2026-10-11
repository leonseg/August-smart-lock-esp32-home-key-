/**
 * motor.cpp
 *
 * DRV8833 motor driver — PWM control via ESP32 LEDC.
 *
 * DRV8833 truth table (per H-bridge):
 *   AIN1   AIN2   Function
 *   PWM    0      Forward  (LOCK)
 *   0      PWM    Reverse  (UNLOCK)
 *   1      1      Brake (active short to GND)
 *   0      0      Coast (outputs high-Z)
 */

#include "motor.h"
#include <driver/gpio.h>

// arduino-esp32 core 3.x replaced the channel-based LEDC API with a pin-based one
#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void pwm_attach(uint8_t pin, uint8_t /*ch*/) {
    if (!ledcAttach(pin, MOTOR_PWM_FREQ_HZ, MOTOR_PWM_RES_BITS)) {
        LOG_TAG("MOTOR", "ledcAttach FAILED on GPIO%u", pin);
    }
}
static void pwm_detach(uint8_t pin) { ledcDetach(pin); }
static void pwm_write(uint8_t pin, uint8_t /*ch*/, uint32_t duty) { ledcWrite(pin, duty); }
#else
static void pwm_attach(uint8_t pin, uint8_t ch) {
    ledcSetup(ch, MOTOR_PWM_FREQ_HZ, MOTOR_PWM_RES_BITS);
    ledcAttachPin(pin, ch);
}
static void pwm_write(uint8_t /*pin*/, uint8_t ch, uint32_t duty) { ledcWrite(ch, duty); }
static void pwm_detach(uint8_t pin) { ledcDetachPin(pin); }
#endif

static bool s_running = false;
static motor_dir_t s_dir = MOTOR_DIR_COAST;

void motor_init() {
    // Configure LEDC PWM channels for AIN1 / AIN2
    pwm_attach(PIN_MOTOR_AIN1, MOTOR_LEDC_CH_AIN1);
    pwm_attach(PIN_MOTOR_AIN2, MOTOR_LEDC_CH_AIN2);

    // BIN1/BIN2 unused but ensure they stay LOW so the B bridge coasts
    pinMode(PIN_MOTOR_BIN1, OUTPUT);
    pinMode(PIN_MOTOR_BIN2, OUTPUT);
    digitalWrite(PIN_MOTOR_BIN1, LOW);
    digitalWrite(PIN_MOTOR_BIN2, LOW);

    motor_stop();
    LOG_TAG("MOTOR", "init — AIN1=%d AIN2=%d @ %dHz", PIN_MOTOR_AIN1, PIN_MOTOR_AIN2, MOTOR_PWM_FREQ_HZ);
}

void motor_drive(motor_dir_t dir, uint8_t duty) {
    s_dir = dir;
    s_running = (dir == MOTOR_DIR_LOCK || dir == MOTOR_DIR_UNLOCK);

    switch (dir) {
        case MOTOR_DIR_LOCK:
            pwm_write(PIN_MOTOR_AIN1, MOTOR_LEDC_CH_AIN1, duty);
            pwm_write(PIN_MOTOR_AIN2, MOTOR_LEDC_CH_AIN2, 0);
            break;
        case MOTOR_DIR_UNLOCK:
            pwm_write(PIN_MOTOR_AIN1, MOTOR_LEDC_CH_AIN1, 0);
            pwm_write(PIN_MOTOR_AIN2, MOTOR_LEDC_CH_AIN2, duty);
            break;
        case MOTOR_DIR_BRAKE:
            pwm_write(PIN_MOTOR_AIN1, MOTOR_LEDC_CH_AIN1, MOTOR_PWM_MAX_DUTY);
            pwm_write(PIN_MOTOR_AIN2, MOTOR_LEDC_CH_AIN2, MOTOR_PWM_MAX_DUTY);
            break;
        case MOTOR_DIR_COAST:
        default:
            pwm_write(PIN_MOTOR_AIN1, MOTOR_LEDC_CH_AIN1, 0);
            pwm_write(PIN_MOTOR_AIN2, MOTOR_LEDC_CH_AIN2, 0);
            break;
    }
    LOG_TAG("MOTOR", "drive dir=%d duty=%u", dir, duty);
}

void motor_stop() {
    motor_drive(MOTOR_DIR_COAST, 0);
    s_running = false;
}

void motor_brake() {
    motor_drive(MOTOR_DIR_BRAKE, 0);
    s_running = false;
}

bool motor_is_running() {
    return s_running;
}

motor_dir_t motor_current_direction() {
    return s_dir;
}

// Take both bridge inputs off LEDC and make them readable outputs.
static void raw_begin() {
    pwm_detach(PIN_MOTOR_AIN1);
    pwm_detach(PIN_MOTOR_AIN2);
    const uint8_t pins[] = {PIN_MOTOR_AIN1, PIN_MOTOR_AIN2};
    for (uint8_t pin : pins) {
        gpio_reset_pin((gpio_num_t)pin);
        gpio_set_direction((gpio_num_t)pin, GPIO_MODE_INPUT_OUTPUT);
        gpio_set_level((gpio_num_t)pin, 0);
    }
}

static void raw_end() {
    gpio_set_level((gpio_num_t)PIN_MOTOR_AIN1, 0);
    gpio_set_level((gpio_num_t)PIN_MOTOR_AIN2, 0);
    pwm_attach(PIN_MOTOR_AIN1, MOTOR_LEDC_CH_AIN1);
    pwm_attach(PIN_MOTOR_AIN2, MOTOR_LEDC_CH_AIN2);
    motor_stop();
}

void motor_diag() {
    raw_begin();
    const uint8_t pins[] = {PIN_MOTOR_AIN1, PIN_MOTOR_AIN2};
    for (uint8_t pin : pins) {
        gpio_set_level((gpio_num_t)pin, 1);
        delay(20);
        int hi = gpio_get_level((gpio_num_t)pin);
        gpio_set_level((gpio_num_t)pin, 0);
        delay(20);
        int lo = gpio_get_level((gpio_num_t)pin);
        LOG_TAG("MOTOR", "diag GPIO%u: driven HIGH reads %d, driven LOW reads %d -> %s",
                pin, hi, lo, (hi == 1 && lo == 0) ? "pin OK" : "PIN FAULT (short or dead pad)");
    }
    raw_end();
}

void motor_raw(motor_dir_t dir, uint32_t ms) {
    raw_begin();
    gpio_set_level((gpio_num_t)PIN_MOTOR_AIN1, dir == MOTOR_DIR_LOCK ? 1 : 0);
    gpio_set_level((gpio_num_t)PIN_MOTOR_AIN2, dir == MOTOR_DIR_UNLOCK ? 1 : 0);
    LOG_TAG("MOTOR", "raw DC dir=%d for %lums (AIN1=%d AIN2=%d read back)", dir, (unsigned long)ms,
            gpio_get_level((gpio_num_t)PIN_MOTOR_AIN1), gpio_get_level((gpio_num_t)PIN_MOTOR_AIN2));
    delay(ms);
    raw_end();
}
