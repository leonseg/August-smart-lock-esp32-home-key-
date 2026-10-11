/**
 * accelerometer.cpp
 *
 * LIS3DH driver glue on top of SparkFun's library.
 * Computes the rotation angle from the X/Z components and compares
 * it against calibrated locked/unlocked references stored in NVS.
 */

#include "accelerometer.h"
#include <Wire.h>
#include <SparkFunLIS3DH.h>
#include <Preferences.h>
#include <math.h>

static LIS3DH s_lis(I2C_MODE, LIS3DH_ADDR);
static bool s_ok = false;
static float s_angle = 0.0f;
static uint32_t s_last_poll = 0;

static float s_cal_locked   = ACCEL_ANGLE_LOCKED_DEG;
static float s_cal_unlocked = ACCEL_ANGLE_UNLOCKED_DEG;

static Preferences s_prefs;

static float compute_angle(float x, float z) {
    // atan2 returns radians; convert to degrees in [-180, 180]
    return atan2f(x, z) * 180.0f / (float)M_PI;
}

bool accel_init() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);

    s_lis.settings.accelSampleRate = ACCEL_SAMPLE_RATE_HZ;
    s_lis.settings.accelRange      = ACCEL_RANGE_G;

    status_t rc = s_lis.begin();
    s_ok = (rc == IMU_SUCCESS);
    if (!s_ok) {
        LOG_TAG("ACCEL", "LIS3DH init FAILED (rc=%d) — check wiring/address", rc);
        return false;
    }

    // Load calibration from NVS
    s_prefs.begin(NVS_NAMESPACE, true);
    s_cal_locked   = s_prefs.getFloat(NVS_KEY_ACCEL_CAL_L, ACCEL_ANGLE_LOCKED_DEG);
    s_cal_unlocked = s_prefs.getFloat(NVS_KEY_ACCEL_CAL_U, ACCEL_ANGLE_UNLOCKED_DEG);
    s_prefs.end();

    LOG_TAG("ACCEL", "LIS3DH ok — cal locked=%.1f° unlocked=%.1f°", s_cal_locked, s_cal_unlocked);
    return true;
}

bool accel_available() { return s_ok; }

bool accel_poll() {
    if (!s_ok) return false;
    uint32_t now = millis();
    if (now - s_last_poll < ACCEL_POLL_INTERVAL_MS) return false;
    s_last_poll = now;

    float x = s_lis.readFloatAccelX();
    float z = s_lis.readFloatAccelZ();
    s_angle = compute_angle(x, z);
    return true;
}

float accel_angle_deg() { return s_angle; }

lock_state_t accel_inferred_state() {
    if (!s_ok) return LOCK_STATE_UNKNOWN;
    if (fabsf(s_angle - s_cal_locked)   <= ACCEL_ANGLE_TOLERANCE_DEG) return LOCK_STATE_LOCKED;
    if (fabsf(s_angle - s_cal_unlocked) <= ACCEL_ANGLE_TOLERANCE_DEG) return LOCK_STATE_UNLOCKED;
    return LOCK_STATE_UNKNOWN;
}

static void save_cal() {
    s_prefs.begin(NVS_NAMESPACE, false);
    s_prefs.putFloat(NVS_KEY_ACCEL_CAL_L, s_cal_locked);
    s_prefs.putFloat(NVS_KEY_ACCEL_CAL_U, s_cal_unlocked);
    s_prefs.end();
}

void accel_calibrate_locked() {
    accel_poll();
    s_cal_locked = s_angle;
    save_cal();
    LOG_TAG("ACCEL", "calibrated LOCKED = %.1f°", s_cal_locked);
}

void accel_calibrate_unlocked() {
    accel_poll();
    s_cal_unlocked = s_angle;
    save_cal();
    LOG_TAG("ACCEL", "calibrated UNLOCKED = %.1f°", s_cal_unlocked);
}

float accel_angle_locked()   { return s_cal_locked; }
float accel_angle_unlocked() { return s_cal_unlocked; }
