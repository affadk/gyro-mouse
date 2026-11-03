#ifndef SERIAL_DEBUG_H
#define SERIAL_DEBUG_H

#include <Arduino.h>

// Serial debugging module (SRS FR-017).
// Provides periodic non-blocking diagnostic output guarded by DEBUG_INTERVAL_MS
// to avoid excessive printing (SRS 16.2).
class SerialDebug {
public:
    SerialDebug();

    void begin(int baud);

    // Print loop diagnostics if interval elapsed. Returns true if printed.
    bool printLoopInfo(float gx, float gy, float gz,
                       int8_t dx, int8_t dy,
                       bool leftPressed, bool rightPressed,
                       bool connected);

private:
    unsigned long m_lastDebug;
};

#endif
