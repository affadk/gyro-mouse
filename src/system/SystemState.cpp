#include "system/SystemState.h"

SystemState::SystemState() : m_state(SystemState_t::INITIALIZING) {}

bool SystemState::transitionTo(SystemState_t newState) {
    // Validate allowed transitions per SRS 14.2
    switch (m_state) {
        case SystemState_t::INITIALIZING:
            if (newState != SystemState_t::CALIBRATING &&
                newState != SystemState_t::ERROR) return false;
            break;
        case SystemState_t::CALIBRATING:
            if (newState != SystemState_t::READY &&
                newState != SystemState_t::ERROR) return false;
            break;
        case SystemState_t::READY:
            if (newState != SystemState_t::ADVERTISING &&
                newState != SystemState_t::ERROR) return false;
            break;
        case SystemState_t::ADVERTISING:
            if (newState != SystemState_t::CONNECTED &&
                newState != SystemState_t::DISCONNECTED &&
                newState != SystemState_t::ERROR) return false;
            break;
        case SystemState_t::CONNECTED:
            if (newState != SystemState_t::PAUSED &&
                newState != SystemState_t::DISCONNECTED &&
                newState != SystemState_t::ERROR) return false;
            break;
        case SystemState_t::PAUSED:
            if (newState != SystemState_t::CONNECTED &&
                newState != SystemState_t::DISCONNECTED &&
                newState != SystemState_t::ERROR) return false;
            break;
        case SystemState_t::DISCONNECTED:
            if (newState != SystemState_t::ADVERTISING &&
                newState != SystemState_t::ERROR) return false;
            break;
        case SystemState_t::ERROR:
            // ERROR is terminal; no transitions out
            return false;
    }
    m_state = newState;
    return true;
}

bool SystemState::isCursorProcessingEnabled() const {
    return m_state == SystemState_t::CONNECTED;
}

bool SystemState::isButtonProcessingActive() const {
    return m_state != SystemState_t::ERROR;
}
