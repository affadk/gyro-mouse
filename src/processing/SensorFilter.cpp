#include "processing/SensorFilter.h"

SensorFilter::SensorFilter()
    : m_deadzone(GyroConfig::DEADZONE), m_alpha(GyroConfig::FILTER_ALPHA),
      m_lpf{0.0f, 0.0f, 0.0f} {}

void SensorFilter::process(const int16_t raw[3], const float bias[3],
                           float& gx, float& gy, float& gz) {
    // Bias compensation: G_corrected = G_raw - B (SRS 8.4)
    float vx = (float)raw[0] / GYRO_SENSITIVITY_LSB_PER_DPS - bias[0];
    float vy = (float)raw[1] / GYRO_SENSITIVITY_LSB_PER_DPS - bias[1];
    float vz = (float)raw[2] / GYRO_SENSITIVITY_LSB_PER_DPS - bias[2];

    // Deadzone (SRS 10.1)
    vx = applyDeadzone(vx);
    vy = applyDeadzone(vy);
    vz = applyDeadzone(vz);

    // Low-pass EMA filter (SRS 10.2): F_t = alpha * X_t + (1 - alpha) * F_{t-1}
    m_lpf[0] = m_alpha * vx + (1.0f - m_alpha) * m_lpf[0];
    m_lpf[1] = m_alpha * vy + (1.0f - m_alpha) * m_lpf[1];
    m_lpf[2] = m_alpha * vz + (1.0f - m_alpha) * m_lpf[2];

    gx = m_lpf[0];
    gy = m_lpf[1];
    gz = m_lpf[2];
}

void SensorFilter::setDeadzone(float dz) { m_deadzone = dz; }
void SensorFilter::setAlpha(float a) { m_alpha = a; }

float SensorFilter::applyDeadzone(float v) const {
    if (v > m_deadzone) return v - m_deadzone;
    if (v < -m_deadzone) return v + m_deadzone;
    return 0.0f;
}

float SensorFilter::lowPass(float input, float prev) const {
    return m_alpha * input + (1.0f - m_alpha) * prev;
}
