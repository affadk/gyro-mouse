#ifndef FIRMWARE_TYPES_H
#define FIRMWARE_TYPES_H

#include <stdint.h>

// --- Axis enumeration (SRS 2.3) ---
enum class Axis_t : uint8_t {
    X = 0,
    Y,
    Z,
    COUNT
};

// --- Function enumeration (SRS 2.3) ---
enum class Function_t : uint8_t {
    UNUSED = 0,   // Auxiliary / unused
    VERTICAL,     // Vertical cursor movement (Gyro Y)
    HORIZONTAL,   // Horizontal cursor movement (Gyro Z)
    COUNT
};

// --- System operating state (SRS 14.1) ---
enum class SystemState_t : uint8_t {
    INITIALIZING,   // Hardware initialization
    CALIBRATING,    // Gyroscope calibration
    READY,          // System initialized
    ADVERTISING,    // BLE waiting for host
    CONNECTED,      // Host connected
    PAUSED,         // Cursor processing disabled
    DISCONNECTED,   // Host disconnected
    ERROR           // Critical error
};

// --- Button logical state (SRS 12.6) ---
enum class ButtonState_t : uint8_t {
    RELEASED,
    PRESSED
};

// --- Button event structure ---
struct ButtonEvent {
    bool leftChanged = false;
    bool rightChanged = false;
    bool leftPressed = false;
    bool rightPressed = false;
};

// --- Cursor delta structure ---
struct CursorDelta {
    int8_t dx = 0;
    int8_t dy = 0;
};

#endif
