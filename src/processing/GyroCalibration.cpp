#include "processing/GyroCalibration.h"
#include <Preferences.h>
#include <cstring>
#include <cmath>

GyroCalibration::GyroCalibration()
    : m_bias{0.0f, 0.0f, 0.0f}, m_calibrated(false) {}

bool GyroCalibration::calibrate(MPU6050Driver& sensor, int samples) {
    if (!sensor.isReady()) return false;

    float sum[3] = {0.0f, 0.0f, 0.0f};
    int valid = 0;

    for (int i = 0; i < samples; ++i) {
        int16_t gx, gy, gz;
        if (sensor.readGyroRaw(gx, gy, gz)) {
            float scale = sensor.gyroScale();
            sum[0] += gx / scale;
            sum[1] += gy / scale;
            sum[2] += gz / scale;
            valid++;
        }
        delay(5);  // 5 ms per sample (SRS 8.5)
    }

    if (valid <= 0) return false;

    m_bias[0] = sum[0] / valid;
    m_bias[1] = sum[1] / valid;
    m_bias[2] = sum[2] / valid;
    m_calibrated = true;
    return true;
}

void GyroCalibration::getBias(float& bx, float& by, float& bz) const {
    bx = m_bias[0];
    by = m_bias[1];
    bz = m_bias[2];
}

uint32_t GyroCalibration::crc32(const void* data, size_t len) {
    const uint8_t* p = static_cast<const uint8_t*>(data);
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= p[i];
        for (int b = 0; b < 8; ++b) {
            crc = (crc & 1u) ? (0xEDB88320u ^ (crc >> 1)) : (crc >> 1);
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

bool GyroCalibration::biasValid(float b) {
    return std::isfinite(b) && b >= BIAS_MIN_DEG_S && b <= BIAS_MAX_DEG_S;
}

bool GyroCalibration::loadBias() {
    Preferences prefs;
    if (!prefs.begin("gyro", true)) return false;  // read-only
    uint32_t ver = prefs.getUInt("ver", 0);
    if (ver != CALIBRATION_VERSION) {
        prefs.end();
        return false;
    }
    uint32_t storedCrc = prefs.getUInt("crc", 0);
    float bx = prefs.getFloat("bx", 0.0f);
    float by = prefs.getFloat("by", 0.0f);
    float bz = prefs.getFloat("bz", 0.0f);
    prefs.end();

    // Recompute CRC over version + 3 bias floats.
    uint8_t buf[4 + 3 * 4];
    std::memcpy(buf, &ver, 4);
    std::memcpy(buf + 4, &bx, 4);
    std::memcpy(buf + 8, &by, 4);
    std::memcpy(buf + 12, &bz, 4);
    uint32_t calcCrc = crc32(buf, sizeof(buf));
    if (calcCrc != storedCrc) return false;

    if (!biasValid(bx) || !biasValid(by) || !biasValid(bz)) return false;

    m_bias[0] = bx;
    m_bias[1] = by;
    m_bias[2] = bz;
    m_calibrated = true;
    return true;
}

bool GyroCalibration::saveBias() {
    Preferences prefs;
    if (!prefs.begin("gyro", false)) return false;
    prefs.putUInt("ver", CALIBRATION_VERSION);

    uint8_t buf[4 + 3 * 4];
    std::memcpy(buf, &CALIBRATION_VERSION, 4);
    std::memcpy(buf + 4, &m_bias[0], 4);
    std::memcpy(buf + 8, &m_bias[1], 4);
    std::memcpy(buf + 12, &m_bias[2], 4);
    prefs.putUInt("crc", crc32(buf, sizeof(buf)));

    prefs.putFloat("bx", m_bias[0]);
    prefs.putFloat("by", m_bias[1]);
    prefs.putFloat("bz", m_bias[2]);
    prefs.end();
    return true;
}

void GyroCalibration::clearBias() {
    Preferences prefs;
    if (!prefs.begin("gyro", false)) return;
    prefs.remove("ver");
    prefs.remove("crc");
    prefs.remove("bx");
    prefs.remove("by");
    prefs.remove("bz");
    prefs.end();
    m_bias[0] = m_bias[1] = m_bias[2] = 0.0f;
    m_calibrated = false;
}
