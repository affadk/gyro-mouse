#include "connectivity/BleMouse.h"

// HID report descriptor for a 3-button mouse with relative X/Y/Wheel.
// Report ID = 1; input report layout: [reportID, buttons, X, Y, wheel].
static const uint8_t REPORT_MAP[] = {
    0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,        // USAGE (Mouse)
    0xA1, 0x01,        // COLLECTION (Application)
    0x85, 0x01,        //   REPORT_ID (1)
    0x09, 0x01,        //   USAGE (Pointer)
    0xA1, 0x00,        //   COLLECTION (Physical)
    0x05, 0x09,        //     USAGE_PAGE (Button)
    0x19, 0x01,        //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,        //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x01,        //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,        //     REPORT_COUNT (3)
    0x75, 0x01,        //     REPORT_SIZE (1)
    0x81, 0x02,        //     INPUT (Data,Var,Abs)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x75, 0x05,        //     REPORT_SIZE (5)
    0x81, 0x01,        //     INPUT (Cnst,Ary,Abs)
    0x05, 0x01,        //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,        //     USAGE (X)
    0x09, 0x31,        //     USAGE (Y)
    0x09, 0x38,        //     USAGE (Wheel)
    0x15, 0x81,        //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,        //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x03,        //     REPORT_COUNT (3)
    0x81, 0x06,        //     INPUT (Data,Var,Rel)
    0xC0,              //   END_COLLECTION
    0xC0               // END_COLLECTION
};

class ServerCallbacks : public BLEServerCallbacks {
public:
    explicit ServerCallbacks(BleMouse* mouse) : m_mouse(mouse) {}
    void onConnect(BLEServer* pServer) override { m_mouse->onConnect(); }
    void onDisconnect(BLEServer* pServer) override { m_mouse->onDisconnect(); }
private:
    BleMouse* m_mouse;
};

BleMouse::BleMouse()
    : m_server(nullptr), m_hid(nullptr), m_inputReport(nullptr), m_connected(false) {}

bool BleMouse::begin() {
    BLEDevice::init("GyroMouse");

    m_server = BLEDevice::createServer();
    m_server->setCallbacks(new ServerCallbacks(this));

    m_hid = new BLEHIDDevice(m_server);

    m_hid->reportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
    m_hid->manufacturer("GyroMouse");
    m_hid->pnp(0x02, 0x1E0F, 0x0001, 0x0100);
    m_hid->hidInfo(0x00, 0x02);

    m_inputReport = m_hid->inputReport(1);  // report ID 1

    m_hid->startServices();

    // Advertise as a HID mouse so hosts discover it during enumeration.
    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(m_hid->hidService()->getUUID());
    adv->setAppearance(0x03C2);  // HID_MOUSE
    adv->start();

    m_connected = false;
    return true;
}

void BleMouse::loop() {
    // FR-016 / NFR-006: if the host disconnects, keep advertising so it can
    // reconnect without blocking the control loop.
    if (!m_connected && m_server) {
        if (m_server->getConnectedCount() == 0) {
            BLEAdvertising* adv = BLEDevice::getAdvertising();
            if (!m_connected) {
                adv->start();
            }
        }
    }
}

void BleMouse::report(int8_t dx, int8_t dy, bool leftPressed, bool rightPressed) {
    if (!m_connected || !m_inputReport) return;

    uint8_t buttons = (leftPressed ? 0x01 : 0x00) | (rightPressed ? 0x02 : 0x00);
    uint8_t report[5] = {0x01, buttons, (uint8_t)dx, (uint8_t)dy, 0x00};
    m_inputReport->setValue(report, sizeof(report));
    m_inputReport->notify();
}

void BleMouse::onConnect() {
    m_connected = true;
}

void BleMouse::onDisconnect() {
    m_connected = false;
}
