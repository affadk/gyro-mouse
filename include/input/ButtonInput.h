#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

#include <Arduino.h>
#include "PinConfig.h"
#include "FirmwareTypes.h"

// Physical button input module (SRS 12, NFR-011, NFR-012, NFR-013).
// Reads two physical push buttons (left = mouse left, right = mouse right)
// with independent non-blocking debouncing per button.
// Each button tracked independently; simultaneous press supported.
class ButtonInput {
public:
    ButtonInput(int leftPin, int rightPin, uint32_t debounceMs, bool activeLow);

    // Initialize GPIO pins as INPUT_PULLUP.
    void begin();

    // Non-blocking update: read GPIOs, run debounce state machines,
    // detect edges. Call every loop iteration.
    void update();

    // Latest debounced event since last update() call.
    const ButtonEvent& event() const { return m_event; }

    // Current logical state of both buttons.
    ButtonState_t leftState() const { return m_leftState; }
    ButtonState_t rightState() const { return m_rightState; }

    // Force both buttons to RELEASED (called on BLE disconnect).
    void reset();

private:
    int m_leftPin;
    int m_rightPin;
    uint32_t m_debounceMs;
    bool m_activeLow;

    ButtonState_t m_leftState;
    ButtonState_t m_rightState;

    int m_leftLastRaw;
    int m_rightLastRaw;
    unsigned long m_leftChangeTime;
    unsigned long m_rightChangeTime;

    ButtonEvent m_event;

    bool rawPressed(int pin) const;
};

#endif
