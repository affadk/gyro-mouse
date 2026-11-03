#include "system/RuntimeConfig.h"

RuntimeConfig cfg;

RuntimeConfig::RuntimeConfig() {
    reset();
}

void RuntimeConfig::reset() {
    sensitivityX = GyroConfig::SENSITIVITY_X;
    sensitivityY = GyroConfig::SENSITIVITY_Y;
    deadzone = GyroConfig::DEADZONE;
    acceleration = GyroConfig::ACCELERATION_COEFF;
    paused = false;
    recalibrate = false;
    connected = false;
    // Default mapping per SRS 2.3: Y -> vertical, Z -> horizontal, X -> unused
    funcAxis[(uint8_t)Function_t::UNUSED] = Axis_t::X;
    funcAxis[(uint8_t)Function_t::VERTICAL] = Axis_t::Y;
    funcAxis[(uint8_t)Function_t::HORIZONTAL] = Axis_t::Z;
    sign[(uint8_t)Function_t::UNUSED] = 1;
    sign[(uint8_t)Function_t::VERTICAL] = 1;
    sign[(uint8_t)Function_t::HORIZONTAL] = 1;
}

void RuntimeConfig::setMapping(Function_t func, Axis_t axis, int8_t sign) {
    uint8_t idx = (uint8_t)func;
    if (idx < 3) {
        funcAxis[idx] = axis;
        this->sign[idx] = (sign == 0) ? 1 : sign;
    }
}

Axis_t RuntimeConfig::axisFor(Function_t func) const {
    uint8_t idx = (uint8_t)func;
    if (idx < 3) return funcAxis[idx];
    return Axis_t::X;
}

int8_t RuntimeConfig::signFor(Function_t func) const {
    uint8_t idx = (uint8_t)func;
    if (idx < 3) return sign[idx];
    return 1;
}
