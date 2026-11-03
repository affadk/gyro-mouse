#ifndef CURSOR_PROCESSOR_H
#define CURSOR_PROCESSOR_H

#include <cstdint>
#include "FirmwareTypes.h"
#include "Config.h"

// Cursor processing module (SRS 11, FR-007, FR-012).
// Converts vertical/horizontal angular velocity (deg/s) into signed mouse
// deltas with deadzone, sensitivity scaling, nonlinear acceleration, and
// fractional accumulator for sub-pixel movement.
class CursorProcessor {
public:
    CursorProcessor();

    // Update cursor deltas from angular velocity over dt seconds.
    // Output dx/dy clamped to [-127, 127].
    void update(float verticalDegPerSec, float horizontalDegPerSec,
                float dtSeconds, CursorDelta& out);

    // Set per-axis sensitivity.
    void setSensitivity(float sx, float sy);

    // Set acceleration coefficient beta.
    void setAcceleration(float beta);

    // Reset fractional accumulators (called on BLE disconnect).
    void resetAccumulators();

    // Set deadzone threshold applied before integration.
    void setDeadzone(float dz);

private:
    float m_sensitivityX;
    float m_sensitivityY;
    float m_acceleration;   // beta
    float m_deadzone;

    float m_accumX;         // fractional accumulator X
    float m_accumY;         // fractional accumulator Y

    float applyDeadzone(float v) const;
    float accelerationGain(float omega) const;  // G(w) = 1 + beta * |w|
};

#endif
