#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

#include <stdint.h>

// --- MPU6050 Register Map ---
#define REG_PWR_MGMT_1    0x6B
#define REG_SMPLRT_DIV    0x19
#define REG_CONFIG        0x1A
#define REG_GYRO_CONFIG   0x1B
#define REG_ACCEL_CONFIG  0x1C
#define REG_ACCEL_XOUT_H  0x3B
#define REG_GYRO_XOUT_H   0x43
#define REG_INT_PIN_CFG   0x37
#define REG_USER_CTRL     0x6A

#define MPU6050_WHO_AM_I_REG 0x75
#define MPU6050_WHO_AM_I_VALUE 0x68

// --- Sensor Ranges (SRS 2.2 Sensor Configuration) ---
#define GYRO_FULL_SCALE_DPS 500    // +/- 500 deg/s
#define ACCEL_FULL_SCALE_G 8       // +/- 8G
#define DLPF_CUTOFF_HZ 21          // 21 Hz digital low-pass filter

// --- Gyroscope sensitivity in LSB per deg/s (±500 dps => 65.5) ---
#define GYRO_SENSITIVITY_LSB_PER_DPS 65.5f

// --- Sampling Configuration (SRS 7.2, NFR-001, NFR-002) ---
// SAMPLE_RATE_HZ and PROCESSING_INTERVAL_MS are defined as constexpr in Config.h
// to avoid macro/constexpr conflicts. These register values are hardware-only.

// --- Calibration Specification (SRS 8.5) ---
// CALIBRATION_SAMPLES and CALIBRATION_INTERVAL_MS are defined as constexpr in Config.h

#endif
