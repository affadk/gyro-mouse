#include "processing/GyroCalibration.h"

GyroCalibration::GyroCalibration()
    : m_bias{0.0f, 0.0f, 0.0f}, m_calibrated(false) {}

bool GyroCalibration::calibrate(MPU6050Driver& sensor, int samples) {
    if (!sensor.isReady()) return false;

    float sum[3] = {0.0f, 0.0f, 0.0f};
    int valid = 0;

    for (int i = 0; i < samples; ++i) {
        int16_t gx, gy, gz;
        if (sensor.readGyroRaw(gx, gy, gz)) {
            float scale = sensor.gyroScale();
            sum[0] += gx / scale;
            sum[1] += gy / scale;
            sum[2] += gz / scale;
            valid++;
        }
        delay(5);  // 5 ms per sample (SRS 8.5)
    }

    if (valid <= 0) return false;

    m_bias[0] = sum[0] / valid;
    m_bias[1] = sum[1] / valid;
    m_bias[2] = sum[2] / valid;
    m_calibrated = true;
    return true;
}

void GyroCalibration::getBias(float& bx, float& by, float& bz) const {
    bx = m_bias[0];
    by = m_bias[1];
    bz = m_bias[2];
}
