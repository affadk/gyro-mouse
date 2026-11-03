#include <unity.h>
#include <Arduino.h>
#include "processing/AxisMapping.h"
#include "FirmwareTypes.h"

// UT-005: Input gyro Z => horizontal output
void test_axis_mapping_z_to_horizontal() {
    AxisMapping m;
    float gyro[3] = {0.0f, 0.0f, 10.0f};  // Z = 10
    float vertical, horizontal;
    m.transform(gyro, vertical, horizontal);
    // Default: Z -> horizontal, Y -> vertical
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, vertical);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, horizontal);
}

// UT-006: Input gyro Y => vertical output
void test_axis_mapping_y_to_vertical() {
    AxisMapping m;
    float gyro[3] = {0.0f, 10.0f, 0.0f};  // Y = 10
    float vertical, horizontal;
    m.transform(gyro, vertical, horizontal);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, vertical);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, horizontal);
}

// Sign inversion reverses direction
void test_axis_mapping_sign_inversion() {
    AxisMapping m;
    m.setMapping(Function_t::HORIZONTAL, Axis_t::Z, -1);
    float gyro[3] = {0.0f, 0.0f, 10.0f};
    float vertical, horizontal;
    m.transform(gyro, vertical, horizontal);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -10.0f, horizontal);
}

void setUp() {}
void tearDown() {}

// Arduino framework stubs (required when main.cpp is excluded from test build)

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_axis_mapping_z_to_horizontal);
    RUN_TEST(test_axis_mapping_y_to_vertical);
    RUN_TEST(test_axis_mapping_sign_inversion);
    return UNITY_END();
}
