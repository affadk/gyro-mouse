#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// --- I2C Configuration (SRS 2.2) ---
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#define I2C_ADDR_MPU6050 0x68
#define I2C_FREQUENCY 400000UL

// --- Button Configuration (SRS 2.2, 2.4) ---
#define PIN_BUTTON_LEFT 32
#define PIN_BUTTON_RIGHT 33
#define BUTTON_DEBOUNCE_MS 25
#define BUTTON_ACTIVE_LOW true

// --- Status LED ---
#define PIN_STATUS_LED 2

#endif
