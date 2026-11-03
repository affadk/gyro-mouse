#include "input/ButtonInput.h"

ButtonInput::ButtonInput(int leftPin, int rightPin, uint32_t debounceMs, bool activeLow)
    : m_leftPin(leftPin), m_rightPin(rightPin), m_debounceMs(debounceMs), m_activeLow(activeLow),
      m_leftState(ButtonState_t::RELEASED), m_rightState(ButtonState_t::RELEASED),
      m_leftLastRaw(HIGH), m_rightLastRaw(HIGH),
      m_leftChangeTime(0), m_rightChangeTime(0) {}

void ButtonInput::begin() {
    pinMode(m_leftPin, INPUT_PULLUP);
    pinMode(m_rightPin, INPUT_PULLUP);
    m_leftLastRaw = digitalRead(m_leftPin);
    m_rightLastRaw = digitalRead(m_rightPin);
    m_leftChangeTime = millis();
    m_rightChangeTime = millis();
    m_event = ButtonEvent();
}

void ButtonInput::update() {
    m_event = ButtonEvent();  // clear event

    int leftRaw = digitalRead(m_leftPin);
    int rightRaw = digitalRead(m_rightPin);

    // Left button debounce
    if (leftRaw != m_leftLastRaw) {
        m_leftChangeTime = millis();
    }
    if (millis() - m_leftChangeTime >= m_debounceMs) {
        bool pressed = rawPressed(leftRaw);
        ButtonState_t newState = pressed ? ButtonState_t::PRESSED : ButtonState_t::RELEASED;
        if (newState != m_leftState) {
            m_leftState = newState;
            m_event.leftChanged = true;
            m_event.leftPressed = (newState == ButtonState_t::PRESSED);
        }
    }
    m_leftLastRaw = leftRaw;

    // Right button debounce
    if (rightRaw != m_rightLastRaw) {
        m_rightChangeTime = millis();
    }
    if (millis() - m_rightChangeTime >= m_debounceMs) {
        bool pressed = rawPressed(rightRaw);
        ButtonState_t newState = pressed ? ButtonState_t::PRESSED : ButtonState_t::RELEASED;
        if (newState != m_rightState) {
            m_rightState = newState;
            m_event.rightChanged = true;
            m_event.rightPressed = (newState == ButtonState_t::PRESSED);
        }
    }
    m_rightLastRaw = rightRaw;
}

void ButtonInput::reset() {
    m_leftState = ButtonState_t::RELEASED;
    m_rightState = ButtonState_t::RELEASED;
    m_event.leftChanged = true;
    m_event.rightChanged = true;
    m_event.leftPressed = false;
    m_event.rightPressed = false;
    m_leftLastRaw = HIGH;
    m_rightLastRaw = HIGH;
    m_leftChangeTime = millis();
    m_rightChangeTime = millis();
}

bool ButtonInput::rawPressed(int pin) const {
    if (m_activeLow) {
        return pin == LOW;
    }
    return pin == HIGH;
}
