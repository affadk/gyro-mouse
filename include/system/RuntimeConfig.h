#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include "FirmwareTypes.h"
#include "Config.h"

// Runtime configuration shared across modules (SRS 17).
// FR-012 sensitivity, FR-005 deadzone, FR-014 recalibrate, FR-015 pause/resume.
class RuntimeConfig {
public:
    float sensitivityX;       // horizontal cursor sensitivity (default 3.0)
    float sensitivityY;       // vertical cursor sensitivity (default 3.0)
    float deadzone;           // deadzone threshold in deg/s (default 2.0)
    float acceleration;       // acceleration coefficient beta (default 0.015)
    bool paused;              // pause/resume cursor processing
    bool recalibrate;         // set to true to trigger gyro recalibration
    bool connected;           // BLE HID host connected

    // Axis mapping: index by Function_t, value = Axis_t (FR-004 configurable mapping)
    Axis_t funcAxis[3];
    int8_t sign[3];           // direction sign per function

    RuntimeConfig();
    void reset();

    void setMapping(Function_t func, Axis_t axis, int8_t sign = 1);
    Axis_t axisFor(Function_t func) const;
    int8_t signFor(Function_t func) const;
};

extern RuntimeConfig cfg;

#endif
