#include "hardware/MPU6050Driver.h"

MPU6050Driver::MPU6050Driver()
    : m_wire(nullptr), m_addr(I2C_ADDR_MPU6050), m_gyroScale(GYRO_SENSITIVITY_LSB_PER_DPS), m_ready(false) {}

bool MPU6050Driver::begin(TwoWire& wire) {
    m_wire = &wire;
    m_ready = false;

    // Wake up: clear SLEEP bit (bit 6) in PWR_MGMT_1 and use internal 8 MHz oscillator.
    if (!writeReg(REG_PWR_MGMT_1, 0x00)) {
        return false;
    }
    delay(10);

    // Device detection: read WHO_AM_I.
    uint8_t who = readReg(MPU6050_WHO_AM_I_REG);
    if (who != MPU6050_WHO_AM_I_VALUE) {
        m_ready = false;
        return false;
    }

    m_ready = true;
    return true;
}

void MPU6050Driver::configure() {
    if (!m_ready) return;

    // Sample rate = 1 kHz / (1 + SMPLRT_DIV) => 200 Hz with DIV = 4.
    writeReg(REG_SMPLRT_DIV, 4);

    // Digital Low Pass Filter: 21 Hz (DLPF_CFG = 4).
    writeReg(REG_CONFIG, 0x04);

    // Gyroscope full-scale +/- 500 deg/s (FS_SEL = 1 => 0x10), 65.5 LSB/(deg/s).
    writeReg(REG_GYRO_CONFIG, 0x10);
    m_gyroScale = GYRO_SENSITIVITY_LSB_PER_DPS;

    // Accelerometer full-scale +/- 8G (AFS_SEL = 2 => 0x10).
    writeReg(REG_ACCEL_CONFIG, 0x10);
}

bool MPU6050Driver::readGyroRaw(int16_t& gx, int16_t& gy, int16_t& gz) {
    if (!m_ready) return false;

    uint8_t buf[6];
    readRegs(REG_GYRO_XOUT_H, buf, 6);

    gx = (int16_t)((uint16_t)buf[0] << 8 | buf[1]);
    gy = (int16_t)((uint16_t)buf[2] << 8 | buf[3]);
    gz = (int16_t)((uint16_t)buf[4] << 8 | buf[5]);
    return true;
}

bool MPU6050Driver::writeReg(uint8_t reg, uint8_t val) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg);
    m_wire->write(val);
    return m_wire->endTransmission() == 0;
}

uint8_t MPU6050Driver::readReg(uint8_t reg) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg);
    if (m_wire->endTransmission(false) != 0) {
        return 0;
    }
    m_wire->requestFrom(m_addr, (uint8_t)1);
    return m_wire->available() ? m_wire->read() : 0;
}

void MPU6050Driver::readRegs(uint8_t reg, uint8_t* buf, size_t len) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg);
    m_wire->endTransmission(false);
    m_wire->requestFrom(m_addr, (uint8_t)len);
    size_t i = 0;
    while (m_wire->available() && i < len) {
        buf[i++] = m_wire->read();
    }
    while (i < len) buf[i++] = 0;
}
