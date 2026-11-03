#ifndef MPU6050_DRIVER_H
#define MPU6050_DRIVER_H

#include <Wire.h>
#include "PinConfig.h"
#include "SensorConfig.h"

// Sensor acquisition module (SRS FR-001, FR-002, NFR-007).
// Handles MPU6050 initialization, device detection, configuration and raw
// three-axis gyroscope reads over I2C.
class MPU6050Driver {
public:
    MPU6050Driver();

    // Initialize I2C device, wake up sensor, detect presence.
    // Returns true when the MPU6050 responds (WHO_AM_I == 0x68).
    bool begin(TwoWire& wire);

    // Configure accel range, gyro range, DLPF and sample rate per SRS 2.2.
    void configure();

    // Read raw gyroscope three-axis values (LSB units at ±500 dps).
    // Returns false on I2C read failure.
    bool readGyroRaw(int16_t& gx, int16_t& gy, int16_t& gz);

    // Gyroscope full-scale sensitivity in LSB per deg/s (±500 dps => 65.5).
    float gyroScale() const { return m_gyroScale; }

    uint8_t address() const { return m_addr; }
    bool isReady() const { return m_ready; }

private:
    TwoWire* m_wire;
    uint8_t m_addr;
    float m_gyroScale;
    bool m_ready;

    bool writeReg(uint8_t reg, uint8_t val);
    uint8_t readReg(uint8_t reg);
    void readRegs(uint8_t reg, uint8_t* buf, size_t len);
};

#endif
