#ifndef CONFIG_H
#define CONFIG_H

#include "PinConfig.h"
#include "SensorConfig.h"
#include "FirmwareTypes.h"

// --- Central tuning configuration (SRS 17) ---

namespace GyroConfig {

    constexpr int SAMPLE_RATE_HZ = 200;

    constexpr float DEADZONE = 2.0f;          // deg/s, SRS 10.1

    constexpr float FILTER_ALPHA = 0.25f;     // EMA coefficient, SRS 10.2

    constexpr float SENSITIVITY_X = 3.0f;     // Horizontal sensitivity, SRS 11.6
    constexpr float SENSITIVITY_Y = 3.0f;     // Vertical sensitivity, SRS 11.6

    constexpr float ACCELERATION_COEFF = 0.015f;  // beta, SRS 11.3

    constexpr int CALIBRATION_SAMPLES = 500;  // SRS 8.5

    constexpr int8_t MAX_HID_MOVEMENT = 127;  // SRS 11.6

    constexpr int DEBUG_BAUD = 115200;
    constexpr int DEBUG_INTERVAL_MS = 200;

}

namespace ButtonConfig {

    constexpr int LEFT_BUTTON_PIN = 32;
    constexpr int RIGHT_BUTTON_PIN = 33;
    constexpr uint32_t DEBOUNCE_MS = 25;      // SRS 12.5, NFR-012
    constexpr bool ACTIVE_LOW = true;

}

#endif
