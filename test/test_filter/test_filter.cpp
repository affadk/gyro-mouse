#include <unity.h>
#include <Arduino.h>
#include "processing/SensorFilter.h"
#include "Config.h"

// UT-003: Input below deadzone => output zero
void test_filter_below_deadzone() {
    SensorFilter f;
    float gx, gy, gz;
    int16_t raw[3] = {10, 10, 10};  // small raw values
    float bias[3] = {0.0f, 0.0f, 0.0f};
    f.process(raw, bias, gx, gy, gz);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, gx);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, gy);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, gz);
}

// UT-004: Input above deadzone => filtered output non-zero
void test_filter_above_deadzone() {
    SensorFilter f;
    float gx, gy, gz;
    int16_t raw[3] = {500, 500, 500};  // well above deadzone
    float bias[3] = {0.0f, 0.0f, 0.0f};
    f.process(raw, bias, gx, gy, gz);
    // After bias compensation (0), deadzone (2.0), and LPF
    // raw/scale = 500/65.5 = 7.63 deg/s, minus deadzone 2.0 = 5.63 deg/s
    TEST_ASSERT_TRUE(gx > 0.0f);
    TEST_ASSERT_TRUE(gy > 0.0f);
    TEST_ASSERT_TRUE(gz > 0.0f);
}

// Negative input below deadzone => zero
void test_filter_negative_below_deadzone() {
    SensorFilter f;
    float gx, gy, gz;
    int16_t raw[3] = {-10, -10, -10};
    float bias[3] = {0.0f, 0.0f, 0.0f};
    f.process(raw, bias, gx, gy, gz);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, gx);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, gy);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, gz);
}

void setUp() {}
void tearDown() {}

// Arduino framework stubs (required when main.cpp is excluded from test build)

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_filter_below_deadzone);
    RUN_TEST(test_filter_above_deadzone);
    RUN_TEST(test_filter_negative_below_deadzone);
    return UNITY_END();
}
