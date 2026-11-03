#ifndef SENSOR_FILTER_H
#define SENSOR_FILTER_H

#include "Config.h"

// Sensor filtering module (SRS 10).
// Filter flow: RAW -> BIAS COMPENSATION -> DEADZONE -> LOW-PASS (EMA).
class SensorFilter {
public:
    SensorFilter();

    // Apply full pipeline: bias compensation, deadzone, low-pass filter.
    // Output values are in deg/s.
    void process(const int16_t raw[3], const float bias[3], float& gx, float& gy, float& gz);

    // Set deadzone threshold (deg/s).
    void setDeadzone(float dz);

    // Set EMA alpha coefficient.
    void setAlpha(float alpha);

private:
    float m_deadzone;
    float m_alpha;
    float m_lpf[3];

    float applyDeadzone(float v) const;
    float lowPass(float input, float prev) const;
};

#endif
