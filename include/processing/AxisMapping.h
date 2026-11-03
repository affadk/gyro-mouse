#ifndef AXIS_MAPPING_H
#define AXIS_MAPPING_H

#include "FirmwareTypes.h"

// Axis transformation module (SRS 9, FR-004).
// Maps processed gyroscope axis values to system functions (vertical cursor,
// horizontal cursor) according to configurable source axis and direction sign.
// Default mapping (SRS 2.3): Y -> vertical, Z -> horizontal, X -> unused.
class AxisMapping {
public:
    AxisMapping();

    // Configure which physical axis drives each function, with optional sign.
    // sign = +1 (normal) or -1 (inverted).
    void setMapping(Function_t func, Axis_t axis, int8_t sign = 1);

    // Given gyro values for [X, Y, Z], write mapped values for vertical
    // and horizontal cursor movement, applying direction signs.
    void transform(const float gyro[3], float& vertical, float& horizontal) const;

private:
    Axis_t m_funcAxis[3];   // index by Function_t (UNUSED, VERTICAL, HORIZONTAL)
    int8_t m_sign[3];       // direction sign per function
};

#endif
