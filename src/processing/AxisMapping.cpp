#include "processing/AxisMapping.h"

AxisMapping::AxisMapping() {
    // Default mapping per SRS 2.3:
    //   UNUSED -> X, VERTICAL -> Y, HORIZONTAL -> Z
    m_funcAxis[(uint8_t)Function_t::UNUSED] = Axis_t::X;
    m_funcAxis[(uint8_t)Function_t::VERTICAL] = Axis_t::Y;
    m_funcAxis[(uint8_t)Function_t::HORIZONTAL] = Axis_t::Z;
    m_sign[(uint8_t)Function_t::UNUSED] = 1;
    m_sign[(uint8_t)Function_t::VERTICAL] = 1;
    m_sign[(uint8_t)Function_t::HORIZONTAL] = 1;
}

void AxisMapping::setMapping(Function_t func, Axis_t axis, int8_t sign) {
    uint8_t idx = (uint8_t)func;
    if (idx < 3) {
        m_funcAxis[idx] = axis;
        m_sign[idx] = (sign == 0) ? 1 : sign;
    }
}

void AxisMapping::transform(const float gyro[3], float& vertical, float& horizontal) const {
    // SRS 9.3: omega_horizontal = s_z * G_z, omega_vertical = s_y * G_y
    float gx = gyro[(uint8_t)Axis_t::X];
    float gy = gyro[(uint8_t)Axis_t::Y];
    float gz = gyro[(uint8_t)Axis_t::Z];

    float v = 0.0f;
    float h = 0.0f;

    // Vertical axis
    Axis_t vAxis = m_funcAxis[(uint8_t)Function_t::VERTICAL];
    int8_t vSign = m_sign[(uint8_t)Function_t::VERTICAL];
    if (vAxis == Axis_t::X) v = vSign * gx;
    else if (vAxis == Axis_t::Y) v = vSign * gy;
    else if (vAxis == Axis_t::Z) v = vSign * gz;

    // Horizontal axis
    Axis_t hAxis = m_funcAxis[(uint8_t)Function_t::HORIZONTAL];
    int8_t hSign = m_sign[(uint8_t)Function_t::HORIZONTAL];
    if (hAxis == Axis_t::X) h = hSign * gx;
    else if (hAxis == Axis_t::Y) h = hSign * gy;
    else if (hAxis == Axis_t::Z) h = hSign * gz;

    vertical = v;
    horizontal = h;
}
