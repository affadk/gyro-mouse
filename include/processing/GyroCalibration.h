#ifndef GYRO_CALIBRATION_H
#define GYRO_CALIBRATION_H

#include "hardware/MPU6050Driver.h"
#include "SensorConfig.h"
#include "FirmwareTypes.h"

// Gyroscope calibration module (SRS 8).
// Computes per-axis bias by averaging N raw samples while the device is
// stationary. Bias is subtracted from subsequent readings.
// SRS 8.5: 500 samples at 5 ms => ~2.5 s calibration duration.
class GyroCalibration {
public:
    GyroCalibration();

    // Run startup calibration: average N raw samples to estimate per-axis bias.
    // Call with device held still (NFR-008 automatic startup calibration).
    // Returns true on success, false if calibration rejected (e.g. device moved).
    bool calibrate(MPU6050Driver& sensor, int samples = 500);

    // Get measured bias per axis (in deg/s).
    void getBias(float& bx, float& by, float& bz) const;

    bool isCalibrated() const { return m_calibrated; }

private:
    float m_bias[3];
    bool m_calibrated;
};

#endif
