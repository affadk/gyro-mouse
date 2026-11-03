#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include "FirmwareTypes.h"

// System operating state machine (SRS 14).
// States: INITIALIZING -> CALIBRATING -> READY -> ADVERTISING -> CONNECTED
//         -> PAUSED -> DISCONNECTED -> ADVERTISING
class SystemState {
public:
    SystemState();

    // Transition to a new state. Returns true if transition accepted.
    bool transitionTo(SystemState_t newState);

    SystemState_t currentState() const { return m_state; }

    // Whether cursor processing should be enabled (only in CONNECTED, not PAUSED).
    bool isCursorProcessingEnabled() const;

    // Whether button processing should remain active (always except ERROR).
    bool isButtonProcessingActive() const;

private:
    SystemState_t m_state;
};

#endif
