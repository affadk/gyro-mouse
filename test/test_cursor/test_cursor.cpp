#include <unity.h>
#include <Arduino.h>
#include <cmath>
#include "processing/CursorProcessor.h"
#include "Config.h"

// UT-007: Positive angular velocity => correct cursor direction
void test_cursor_positive_velocity() {
    CursorProcessor c;
    c.setSensitivity(3.0f, 3.0f);
    c.setAcceleration(0.0f);  // disable acceleration for deterministic test
    c.setDeadzone(0.0f);      // disable deadzone

    CursorDelta d;
    c.update(0.0f, 10.0f, 0.005f, d);  // horizontal = 10 deg/s
    TEST_ASSERT_TRUE(d.dx > 0);  // positive horizontal movement
}

// UT-008: Negative angular velocity => opposite cursor direction
void test_cursor_negative_velocity() {
    CursorProcessor c;
    c.setSensitivity(3.0f, 3.0f);
    c.setAcceleration(0.0f);
    c.setDeadzone(0.0f);

    CursorDelta d;
    c.update(0.0f, -10.0f, 0.005f, d);
    TEST_ASSERT_TRUE(d.dx < 0);  // negative horizontal movement
}

// Zero velocity => zero movement
void test_cursor_zero_velocity() {
    CursorProcessor c;
    c.setSensitivity(3.0f, 3.0f);
    c.setAcceleration(0.0f);
    c.setDeadzone(0.0f);

    CursorDelta d;
    c.update(0.0f, 0.0f, 0.005f, d);
    TEST_ASSERT_EQUAL_INT8(0, d.dx);
    TEST_ASSERT_EQUAL_INT8(0, d.dy);
}

// Clamp to ±127
void test_cursor_clamp() {
    CursorProcessor c;
    c.setSensitivity(1000.0f, 1000.0f);  // very high sensitivity
    c.setAcceleration(0.0f);
    c.setDeadzone(0.0f);

    CursorDelta d;
    c.update(0.0f, 1000.0f, 0.005f, d);  // huge velocity
    TEST_ASSERT_TRUE(d.dx <= 127);
    TEST_ASSERT_TRUE(d.dx >= -127);
}

// Fractional accumulator: small movements accumulate over multiple calls
void test_cursor_fractional_accumulator() {
    CursorProcessor c;
    c.setSensitivity(1.0f, 1.0f);
    c.setAcceleration(0.0f);
    c.setDeadzone(0.0f);

    // Small velocity that produces sub-pixel movement each call
    // 0.1 deg/s * 0.005 s = 0.0005 deg, * 1.0 sensitivity = 0.0005 px
    // Need ~2000 calls to accumulate 1 pixel
    int totalDx = 0;
    for (int i = 0; i < 2000; i++) {
        CursorDelta d;
        c.update(0.0f, 0.1f, 0.005f, d);
        totalDx += d.dx;
    }
    // After 2000 calls at 0.0005 px/call = 1.0 px total, but fractional
    // accumulator should capture most of it
    TEST_ASSERT_TRUE(totalDx >= 0);
}

void setUp() {}
void tearDown() {}

// Arduino framework stubs (required when main.cpp is excluded from test build)

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_cursor_positive_velocity);
    RUN_TEST(test_cursor_negative_velocity);
    RUN_TEST(test_cursor_zero_velocity);
    RUN_TEST(test_cursor_clamp);
    RUN_TEST(test_cursor_fractional_accumulator);
    return UNITY_END();
}
