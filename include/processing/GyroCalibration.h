#ifndef GYRO_CALIBRATION_H
#define GYRO_CALIBRATION_H

#include <cstdint>
#include "hardware/MPU6050Driver.h"
#include "SensorConfig.h"
#include "FirmwareTypes.h"

// Gyroscope calibration module (SRS 8).
// Computes per-axis bias by averaging N raw samples while the device is
// stationary. Bias is subtracted from subsequent readings.
// Supports persistent storage of bias in ESP32 flash (Preferences / NVS)
// so calibration does not need to be repeated on every boot.
// SRS 8.5: 500 samples at 5 ms => ~2.5 s calibration duration.
class GyroCalibration {
public:
    static constexpr uint32_t CALIBRATION_VERSION = 1;

    // Reasonable bias range in deg/s; values outside this are treated as
    // corrupt and rejected (SRS 22.1: bias drifts slowly with temperature).
    static constexpr float BIAS_MIN_DEG_S = -50.0f;
    static constexpr float BIAS_MAX_DEG_S = 50.0f;

    GyroCalibration();

    // Run startup calibration: average N raw samples to estimate per-axis bias.
    // Call with device held still (NFR-008 automatic startup calibration).
    // Returns true on success, false if calibration rejected (e.g. device moved).
    bool calibrate(MPU6050Driver& sensor, int samples = 500);

    // Get measured bias per axis (in deg/s).
    void getBias(float& bx, float& by, float& bz) const;

    bool isCalibrated() const { return m_calibrated; }

    // Persistent storage (ESP32 Preferences / NVS).
    // Load previously saved bias from flash. Returns true if valid data found
    // (version matches, CRC32 checksum valid, values within range).
    bool loadBias();
    // Save current bias to flash. Returns true on success.
    bool saveBias();
    // Clear saved bias from flash.
    void clearBias();

private:
    float m_bias[3];
    bool m_calibrated;

    static uint32_t crc32(const void* data, size_t len);
    static bool biasValid(float b);
};

#endif
