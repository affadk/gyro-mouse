#ifndef BLE_MOUSE_H
#define BLE_MOUSE_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include "FirmwareTypes.h"

// BLE HID Mouse module (SRS 13, FR-008, FR-009, FR-016, FR-020).
// ESP32 operates as BLE peripheral + HID mouse. Host is BLE central.
// Manages advertising, connection, HID report delivery, and disconnect cleanup.
class BleMouse {
public:
    BleMouse();

    // Initialize BLE stack + HID service and start advertising.
    bool begin();

    // Call repeatedly from loop(): re-advertise when disconnected.
    void loop();

    bool isConnected() const { return m_connected; }

    // Send a HID mouse input report: dx/dy in [-127,127], left/right button state.
    // Only sends when connected.
    void report(int8_t dx, int8_t dy, bool leftPressed, bool rightPressed);

    // Called by BLE server callbacks on connect.
    void onConnect();

    // Called by BLE server callbacks on disconnect.
    void onDisconnect();

private:
    BLEServer* m_server;
    BLEHIDDevice* m_hid;
    BLECharacteristic* m_inputReport;
    bool m_connected;
};

#endif
