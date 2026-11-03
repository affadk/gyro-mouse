#include <Arduino.h>
#include <Wire.h>

#include "Config.h"
#include "PinConfig.h"
#include "FirmwareTypes.h"
#include "system/RuntimeConfig.h"
#include "system/SystemState.h"
#include "hardware/MPU6050Driver.h"
#include "processing/GyroCalibration.h"
#include "processing/SensorFilter.h"
#include "processing/AxisMapping.h"
#include "processing/CursorProcessor.h"
#include "connectivity/BleMouse.h"
#include "input/ButtonInput.h"
#include "diagnostics/SerialDebug.h"

// Module instances (wired together in setup / loop).
MPU6050Driver mpu;
GyroCalibration calibration;
SensorFilter filter;
AxisMapping mapping;
CursorProcessor cursor;
BleMouse bleMouse;
ButtonInput buttonInput(PIN_BUTTON_LEFT, PIN_BUTTON_RIGHT, BUTTON_DEBOUNCE_MS, BUTTON_ACTIVE_LOW);
SystemState systemState;
SerialDebug serialDebug;

// Timing (SRS 16.2: use micros() for interval measurement, no delay() in loop).
unsigned long g_lastLoopMicros = 0;

// Last reported button state (for change detection in HID report).
bool g_lastLeftPressed = false;
bool g_lastRightPressed = false;

#ifndef UNIT_TEST
void setup() {
    // SRS 6.1 step 1: Initialize Serial interface.
    serialDebug.begin(GyroConfig::DEBUG_BAUD);
    Serial.println(F("=== Gyro Mouse Firmware ==="));

    // SRS 6.1 step 2: Initialize I2C.
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(I2C_FREQUENCY);

    // SRS 6.1 steps 3-5: MPU6050 init + configuration.
    Serial.println(F("Initializing MPU6050..."));
    if (!mpu.begin(Wire)) {
        Serial.println(F("ERROR: MPU6050 not detected on I2C"));
        Serial.println(F("HALT: check wiring (SDA=21, SCL=22, addr=0x68)"));
        systemState.transitionTo(SystemState_t::ERROR);
        while (1) {
            // NFR-007: error detection, stop on init failure
            delay(1000);
        }
    }
    mpu.configure();
    Serial.println(F("MPU6050 initialized and configured"));

    // SRS 6.1 step 6: Initialize button GPIO.
    buttonInput.begin();

    // SRS 6.1 steps 7-8: Execute gyroscope calibration (automatic at startup, NFR-008).
    // Reuse saved bias from flash if available to avoid 2.5 s calibration on every boot.
    Serial.println(F("Loading gyro calibration..."));
    systemState.transitionTo(SystemState_t::CALIBRATING);
    bool usedSavedBias = calibration.loadBias();
    if (!usedSavedBias) {
        Serial.println(F("Calibrating gyroscope... keep device still"));
        if (!calibration.calibrate(mpu, GyroConfig::CALIBRATION_SAMPLES)) {
            Serial.println(F("WARN: calibration failed, using zero bias"));
        } else {
            calibration.saveBias();
        }
    } else {
        Serial.println(F("Using saved gyro calibration from flash"));
    }
    float bx, by, bz;
    calibration.getBias(bx, by, bz);
    Serial.print(F("Bias: "));
    Serial.print(bx, 3);
    Serial.print(F(" "));
    Serial.print(by, 3);
    Serial.print(F(" "));
    Serial.println(bz, 3);

    // SRS 6.1 step 9: Initialize BLE HID service.
    bleMouse.begin();
    systemState.transitionTo(SystemState_t::READY);
    systemState.transitionTo(SystemState_t::ADVERTISING);
    Serial.println(F("BLE HID Mouse service started (advertising as GyroMouse)"));

    // SRS 6.1 steps 10: Initialize cursor processing.
    cursor.setSensitivity(cfg.sensitivityX, cfg.sensitivityY);
    cursor.setAcceleration(cfg.acceleration);
    cursor.setDeadzone(cfg.deadzone);

    // Configure axis mapping from runtime config.
    mapping.setMapping(Function_t::VERTICAL, cfg.axisFor(Function_t::VERTICAL), cfg.signFor(Function_t::VERTICAL));
    mapping.setMapping(Function_t::HORIZONTAL, cfg.axisFor(Function_t::HORIZONTAL), cfg.signFor(Function_t::HORIZONTAL));

    // Configure filter.
    filter.setDeadzone(cfg.deadzone);

    Serial.println(F("System ready"));
    g_lastLoopMicros = micros();
}
#else
void setup() {}
#endif

#ifndef UNIT_TEST
void loop() {
    // SRS FR-009: BLE connection management (non-blocking).
    bleMouse.loop();

    // Update connection flag in runtime config.
    cfg.connected = bleMouse.isConnected();

    // Handle disconnect: reset cursor accumulator and button state (SRS 13.4, 13.5, 18).
    static bool g_wasConnected = false;
    if (g_wasConnected && !cfg.connected) {
        cursor.resetAccumulators();
        buttonInput.reset();
        g_lastLeftPressed = false;
        g_lastRightPressed = false;
        systemState.transitionTo(SystemState_t::DISCONNECTED);
        systemState.transitionTo(SystemState_t::ADVERTISING);
    }
    g_wasConnected = cfg.connected;

    // SRS FR-015: pause / resume - button processing continues even when paused.
    bool cursorEnabled = systemState.isCursorProcessingEnabled();

    // SRS 12: Button processing (independent of cursor pause state).
    buttonInput.update();

    // Track current button state for HID report.
    bool leftPressed = (buttonInput.leftState() == ButtonState_t::PRESSED);
    bool rightPressed = (buttonInput.rightState() == ButtonState_t::PRESSED);

    // Cursor processing path (SRS Path A).
    int8_t dx = 0;
    int8_t dy = 0;
    float gx = 0.0f, gy = 0.0f, gz = 0.0f;

    if (cursorEnabled && cfg.connected) {
        // SRS 16.2: Use micros() for actual delta time.
        unsigned long nowMicros = micros();
        unsigned long dtUs = nowMicros - g_lastLoopMicros;
        g_lastLoopMicros = nowMicros;
        if (dtUs == 0) dtUs = 1;
        float dt = dtUs / 1000000.0f;

        // SRS FR-002: Read gyro three-axis.
        int16_t raw[3];
        if (!mpu.readGyroRaw(raw[0], raw[1], raw[2])) {
            // NFR-007: discard invalid sample (SRS 18).
            Serial.println(F("WARN: gyro read failed"));
        } else {
            float bias[3] = {0.0f, 0.0f, 0.0f};
            calibration.getBias(bias[0], bias[1], bias[2]);

            // Sensor processing: bias compensation + deadzone + low-pass filter (SRS 10.3).
            filter.process(raw, bias, gx, gy, gz);

            // Axis transformation (SRS FR-004 configurable mapping).
            float vertical, horizontal;
            float gyro[3] = {gx, gy, gz};
            mapping.transform(gyro, vertical, horizontal);

            // Cursor movement calculation (SRS FR-007) + sensitivity + acceleration + fractional accumulator.
            CursorDelta delta;
            cursor.update(vertical, horizontal, dt, delta);
            dx = delta.dx;
            dy = delta.dy;
        }
    } else {
        // Not connected or paused: keep dt tracking but no cursor output.
        g_lastLoopMicros = micros();
    }

    // SRS FR-008: Send BLE HID mouse report when there is movement or button change.
    bool reportNeeded = (dx != 0 || dy != 0 ||
                         leftPressed != g_lastLeftPressed ||
                         rightPressed != g_lastRightPressed);

    if (reportNeeded && cfg.connected) {
        bleMouse.report(dx, dy, leftPressed, rightPressed);
        g_lastLeftPressed = leftPressed;
        g_lastRightPressed = rightPressed;
    }

    // SRS FR-014: Runtime recalibration trigger.
    if (cfg.recalibrate) {
        Serial.println(F("Recalibrating gyroscope... keep still"));
        systemState.transitionTo(SystemState_t::CALIBRATING);
        if (calibration.calibrate(mpu, GyroConfig::CALIBRATION_SAMPLES)) {
            calibration.saveBias();
            float bx, by, bz;
            calibration.getBias(bx, by, bz);
            Serial.print(F("Recalibration complete. Bias: "));
            Serial.print(bx, 3);
            Serial.print(F(" "));
            Serial.print(by, 3);
            Serial.print(F(" "));
            Serial.println(bz, 3);
        } else {
            Serial.println(F("WARN: recalibration failed"));
        }
        cfg.recalibrate = false;
        systemState.transitionTo(SystemState_t::READY);
        if (cfg.connected) {
            systemState.transitionTo(SystemState_t::ADVERTISING);
            systemState.transitionTo(SystemState_t::CONNECTED);
        }
    }

    // SRS FR-017: Serial debugging (throttled).
    serialDebug.printLoopInfo(gx, gy, gz, dx, dy, leftPressed, rightPressed, cfg.connected);
}
#else
void loop() {}
#endif
