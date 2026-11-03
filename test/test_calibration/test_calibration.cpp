#include <unity.h>
#include <Arduino.h>
#include "processing/GyroCalibration.h"
#include "system/SystemState.h"

class MockMPU6050 : public MPU6050Driver {
public:
    MockMPU6050() : m_ready(true), m_readCount(0) {}
    bool begin(TwoWire& wire) { m_ready = true; return true; }
    void configure() {}
    bool readGyroRaw(int16_t& gx, int16_t& gy, int16_t& gz) {
        m_readCount++;
        gx = m_gx; gy = m_gy; gz = m_gz;
        return m_ready;
    }
    void setValues(int16_t x, int16_t y, int16_t z) { m_gx = x; m_gy = y; m_gz = z; }
    void setReady(bool ready) { m_ready = ready; }

private:
    bool m_ready;
    int16_t m_gx = 0, m_gy = 0, m_gz = 0;
    int m_readCount = 0;
};

void test_calibration_still_sensor() {
    MockMPU6050 mpu;
    mpu.setValues(100, 200, 300);
    GyroCalibration cal;
    TEST_ASSERT_TRUE(cal.calibrate(mpu, 10));
    TEST_ASSERT_TRUE(cal.isCalibrated());
    float bx, by, bz;
    cal.getBias(bx, by, bz);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f / 65.5f, bx);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 200.0f / 65.5f, by);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 300.0f / 65.5f, bz);
}

void test_calibration_sensor_not_ready() {
    MockMPU6050 mpu;
    mpu.setReady(false);
    GyroCalibration cal;
    TEST_ASSERT_FALSE(cal.calibrate(mpu, 10));
    TEST_ASSERT_FALSE(cal.isCalibrated());
}

void test_state_initializing_to_calibrating() {
    SystemState s;
    TEST_ASSERT_TRUE(s.transitionTo(SystemState_t::CALIBRATING));
    TEST_ASSERT_EQUAL_INT8((uint8_t)SystemState_t::CALIBRATING, (uint8_t)s.currentState());
}

void test_state_calibrating_to_ready() {
    SystemState s;
    s.transitionTo(SystemState_t::CALIBRATING);
    TEST_ASSERT_TRUE(s.transitionTo(SystemState_t::READY));
    TEST_ASSERT_EQUAL_INT8((uint8_t)SystemState_t::READY, (uint8_t)s.currentState());
}

void test_state_ready_to_advertising() {
    SystemState s;
    s.transitionTo(SystemState_t::READY);
    TEST_ASSERT_TRUE(s.transitionTo(SystemState_t::ADVERTISING));
    TEST_ASSERT_EQUAL_INT8((uint8_t)SystemState_t::ADVERTISING, (uint8_t)s.currentState());
}

void test_state_advertising_to_connected() {
    SystemState s;
    s.transitionTo(SystemState_t::ADVERTISING);
    TEST_ASSERT_TRUE(s.transitionTo(SystemState_t::CONNECTED));
    TEST_ASSERT_EQUAL_INT8((uint8_t)SystemState_t::CONNECTED, (uint8_t)s.currentState());
}

void test_state_connected_to_paused() {
    SystemState s;
    s.transitionTo(SystemState_t::CONNECTED);
    TEST_ASSERT_TRUE(s.transitionTo(SystemState_t::PAUSED));
    TEST_ASSERT_EQUAL_INT8((uint8_t)SystemState_t::PAUSED, (uint8_t)s.currentState());
}

void test_state_paused_to_connected() {
    SystemState s;
    s.transitionTo(SystemState_t::PAUSED);
    TEST_ASSERT_TRUE(s.transitionTo(SystemState_t::CONNECTED));
    TEST_ASSERT_EQUAL_INT8((uint8_t)SystemState_t::CONNECTED, (uint8_t)s.currentState());
}

void test_state_connected_to_disconnected() {
    SystemState s;
    s.transitionTo(SystemState_t::CONNECTED);
    TEST_ASSERT_TRUE(s.transitionTo(SystemState_t::DISCONNECTED));
    TEST_ASSERT_EQUAL_INT8((uint8_t)SystemState_t::DISCONNECTED, (uint8_t)s.currentState());
}

void test_state_disconnected_to_advertising() {
    SystemState s;
    s.transitionTo(SystemState_t::DISCONNECTED);
    TEST_ASSERT_TRUE(s.transitionTo(SystemState_t::ADVERTISING));
    TEST_ASSERT_EQUAL_INT8((uint8_t)SystemState_t::ADVERTISING, (uint8_t)s.currentState());
}

void test_state_invalid_transition() {
    SystemState s;
    TEST_ASSERT_FALSE(s.transitionTo(SystemState_t::READY));
    TEST_ASSERT_EQUAL_INT8((uint8_t)SystemState_t::INITIALIZING, (uint8_t)s.currentState());
}

void test_state_error_terminal() {
    SystemState s;
    s.transitionTo(SystemState_t::ERROR);
    TEST_ASSERT_FALSE(s.transitionTo(SystemState_t::INITIALIZING));
    TEST_ASSERT_EQUAL_INT8((uint8_t)SystemState_t::ERROR, (uint8_t)s.currentState());
}

void test_cursor_processing_enabled() {
    SystemState s;
    TEST_ASSERT_FALSE(s.isCursorProcessingEnabled());
    s.transitionTo(SystemState_t::CONNECTED);
    TEST_ASSERT_TRUE(s.isCursorProcessingEnabled());
    s.transitionTo(SystemState_t::PAUSED);
    TEST_ASSERT_FALSE(s.isCursorProcessingEnabled());
}

void test_button_processing_active() {
    SystemState s;
    TEST_ASSERT_TRUE(s.isButtonProcessingActive());
    s.transitionTo(SystemState_t::ERROR);
    TEST_ASSERT_FALSE(s.isButtonProcessingActive());
}

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_calibration_still_sensor);
    RUN_TEST(test_calibration_sensor_not_ready);
    RUN_TEST(test_state_initializing_to_calibrating);
    RUN_TEST(test_state_calibrating_to_ready);
    RUN_TEST(test_state_ready_to_advertising);
    RUN_TEST(test_state_advertising_to_connected);
    RUN_TEST(test_state_connected_to_paused);
    RUN_TEST(test_state_paused_to_connected);
    RUN_TEST(test_state_connected_to_disconnected);
    RUN_TEST(test_state_disconnected_to_advertising);
    RUN_TEST(test_state_invalid_transition);
    RUN_TEST(test_state_error_terminal);
    RUN_TEST(test_cursor_processing_enabled);
    RUN_TEST(test_button_processing_active);
    return UNITY_END();
}
