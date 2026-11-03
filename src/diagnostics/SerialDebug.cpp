#include "diagnostics/SerialDebug.h"
#include "Config.h"

SerialDebug::SerialDebug() : m_lastDebug(0) {}

void SerialDebug::begin(int baud) {
    Serial.begin(baud);
    while (!Serial && (millis() < 3000)) {
        // Wait for serial (optional, non-blocking after timeout).
    }
}

bool SerialDebug::printLoopInfo(float gx, float gy, float gz,
                                int8_t dx, int8_t dy,
                                bool leftPressed, bool rightPressed,
                                bool connected) {
    unsigned long now = millis();
    if (now - m_lastDebug < GyroConfig::DEBUG_INTERVAL_MS) return false;
    m_lastDebug = now;

    Serial.printf("gx=%7.2f gy=%7.2f gz=%7.2f | dx=%4d dy=%4d L=%d R=%d conn=%d\n",
                  gx, gy, gz, dx, dy, leftPressed ? 1 : 0, rightPressed ? 1 : 0, connected ? 1 : 0);
    return true;
}
