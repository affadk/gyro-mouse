#include <unity.h>
#include <Arduino.h>
#include "input/ButtonInput.h"

static int g_leftPinValue = HIGH;
static int g_rightPinValue = HIGH;

static int mockDigitalRead(int pin) {
    if (pin == 32) return g_leftPinValue;
    if (pin == 33) return g_rightPinValue;
    return HIGH;
}

int digitalRead(int pin) {
    return mockDigitalRead(pin);
}

void test_button_left_pressed() {
    ButtonInput btn(32, 33, 25, true);
    btn.begin();
    g_leftPinValue = LOW;
    btn.update();
    TEST_ASSERT_EQUAL_INT8((uint8_t)ButtonState_t::RELEASED, (uint8_t)btn.leftState());
}

void test_button_left_released() {
    ButtonInput btn(32, 33, 25, true);
    btn.begin();
    g_leftPinValue = LOW;
    btn.update();
    g_leftPinValue = HIGH;
    btn.update();
    TEST_ASSERT_EQUAL_INT8((uint8_t)ButtonState_t::RELEASED, (uint8_t)btn.leftState());
}

void test_button_right_pressed() {
    ButtonInput btn(32, 33, 25, true);
    btn.begin();
    g_rightPinValue = LOW;
    btn.update();
    TEST_ASSERT_EQUAL_INT8((uint8_t)ButtonState_t::RELEASED, (uint8_t)btn.rightState());
}

void test_button_right_released() {
    ButtonInput btn(32, 33, 25, true);
    btn.begin();
    g_rightPinValue = LOW;
    btn.update();
    g_rightPinValue = HIGH;
    btn.update();
    TEST_ASSERT_EQUAL_INT8((uint8_t)ButtonState_t::RELEASED, (uint8_t)btn.rightState());
}

void test_button_no_bounce_duplicate() {
    ButtonInput btn(32, 33, 25, true);
    btn.begin();
    g_leftPinValue = LOW;
    btn.update();
    g_leftPinValue = HIGH;
    btn.update();
    g_leftPinValue = LOW;
    btn.update();
    TEST_ASSERT_EQUAL_INT8((uint8_t)ButtonState_t::RELEASED, (uint8_t)btn.leftState());
}

void test_button_both_pressed() {
    ButtonInput btn(32, 33, 25, true);
    btn.begin();
    g_leftPinValue = LOW;
    g_rightPinValue = LOW;
    btn.update();
    TEST_ASSERT_EQUAL_INT8((uint8_t)ButtonState_t::RELEASED, (uint8_t)btn.leftState());
    TEST_ASSERT_EQUAL_INT8((uint8_t)ButtonState_t::RELEASED, (uint8_t)btn.rightState());
}

void test_button_hold_no_repeat() {
    ButtonInput btn(32, 33, 25, true);
    btn.begin();
    g_leftPinValue = LOW;
    btn.update();
    btn.update();
    btn.update();
    ButtonEvent ev = btn.event();
    TEST_ASSERT_FALSE(ev.leftChanged);
}

void test_button_release_after_hold() {
    ButtonInput btn(32, 33, 25, true);
    btn.begin();
    g_leftPinValue = LOW;
    btn.update();
    g_leftPinValue = HIGH;
    btn.update();
    TEST_ASSERT_EQUAL_INT8((uint8_t)ButtonState_t::RELEASED, (uint8_t)btn.leftState());
}

void test_button_reset() {
    ButtonInput btn(32, 33, 25, true);
    btn.begin();
    btn.reset();
    TEST_ASSERT_EQUAL_INT8((uint8_t)ButtonState_t::RELEASED, (uint8_t)btn.leftState());
    TEST_ASSERT_EQUAL_INT8((uint8_t)ButtonState_t::RELEASED, (uint8_t)btn.rightState());
    ButtonEvent ev = btn.event();
    TEST_ASSERT_TRUE(ev.leftChanged);
    TEST_ASSERT_TRUE(ev.rightChanged);
    TEST_ASSERT_FALSE(ev.leftPressed);
    TEST_ASSERT_FALSE(ev.rightPressed);
}

void setUp() {
    g_leftPinValue = HIGH;
    g_rightPinValue = HIGH;
}

void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_button_left_pressed);
    RUN_TEST(test_button_left_released);
    RUN_TEST(test_button_right_pressed);
    RUN_TEST(test_button_right_released);
    RUN_TEST(test_button_no_bounce_duplicate);
    RUN_TEST(test_button_both_pressed);
    RUN_TEST(test_button_hold_no_repeat);
    RUN_TEST(test_button_release_after_hold);
    RUN_TEST(test_button_reset);
    return UNITY_END();
}
