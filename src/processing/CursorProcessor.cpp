#include "processing/CursorProcessor.h"
#include <cmath>

CursorProcessor::CursorProcessor()
    : m_sensitivityX(GyroConfig::SENSITIVITY_X),
      m_sensitivityY(GyroConfig::SENSITIVITY_Y),
      m_acceleration(GyroConfig::ACCELERATION_COEFF),
      m_deadzone(GyroConfig::DEADZONE),
      m_accumX(0.0f), m_accumY(0.0f) {}

void CursorProcessor::update(float verticalDegPerSec, float horizontalDegPerSec,
                             float dtSeconds, CursorDelta& out) {
    if (dtSeconds <= 0.0f) dtSeconds = 0.005f;

    // Values already deadzoned by SensorFilter (SRS 10.3). Do not re-apply.
    float v = verticalDegPerSec;
    float h = horizontalDegPerSec;

    // Angular integration: delta_theta = omega * dt (SRS 11.2)
    float dthetaV = v * dtSeconds;
    float dthetaH = h * dtSeconds;

    // Nonlinear acceleration: G(w) = 1 + beta * |w| (SRS 11.3)
    float gainV = accelerationGain(v);
    float gainH = accelerationGain(h);

    // Cursor delta with sensitivity (SRS 11.2)
    float dxFloat = h * m_sensitivityX * dthetaH * gainH;
    float dyFloat = v * m_sensitivityY * dthetaV * gainV;

    // Fractional accumulator (SRS 11.4): carry sub-pixel remainders forward
    m_accumX += dxFloat;
    m_accumY += dyFloat;

    int8_t outX = (int8_t)m_accumX;
    int8_t outY = (int8_t)m_accumY;

    // Clamp to int8 range [-127, 127] (SRS 11.6)
    if (outX > GyroConfig::MAX_HID_MOVEMENT) outX = GyroConfig::MAX_HID_MOVEMENT;
    if (outX < -GyroConfig::MAX_HID_MOVEMENT) outX = -GyroConfig::MAX_HID_MOVEMENT;
    if (outY > GyroConfig::MAX_HID_MOVEMENT) outY = GyroConfig::MAX_HID_MOVEMENT;
    if (outY < -GyroConfig::MAX_HID_MOVEMENT) outY = -GyroConfig::MAX_HID_MOVEMENT;

    m_accumX -= outX;
    m_accumY -= outY;

    out.dx = outX;
    out.dy = outY;
}

void CursorProcessor::setSensitivity(float sx, float sy) {
    m_sensitivityX = sx;
    m_sensitivityY = sy;
}

void CursorProcessor::setAcceleration(float beta) { m_acceleration = beta; }

void CursorProcessor::resetAccumulators() {
    m_accumX = 0.0f;
    m_accumY = 0.0f;
}

void CursorProcessor::setDeadzone(float dz) { m_deadzone = dz; }

float CursorProcessor::applyDeadzone(float v) const {
    if (v > m_deadzone) return v - m_deadzone;
    if (v < -m_deadzone) return v + m_deadzone;
    return 0.0f;
}

float CursorProcessor::accelerationGain(float omega) const {
    return 1.0f + m_acceleration * fabsf(omega);
}
