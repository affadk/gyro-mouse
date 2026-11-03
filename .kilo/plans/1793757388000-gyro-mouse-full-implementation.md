# Gyro Mouse — Full Firmware Implementation Plan

**Source spec:** `ref/srs.md` (System & Firmware Specification v1.1)
**Target:** Replace the total current codebase with a complete SRS-compliant implementation
**Platform:** PlatformIO + Arduino Framework (ESP32), LOLIN32 Lite board

---

## 1. Gap Analysis: Current Code vs SRS

The existing codebase is a partial, flat-layout implementation with these critical gaps:

| # | Issue | SRS Reference | Impact |
|---|-------|--------------|--------|
| 1 | Button pins wrong: config.h uses GPIO0/GPIO4; SRS requires GPIO32 (left) / GPIO33 (right) | §2.2, §2.4, §13.4 | Buttons won't work on target hardware |
| 2 | No `ButtonInput` module; debounce logic inline in `main.cpp` | §12, §15.3 | No independent per-button debounce, no press/release state machine, no simultaneous-button safety |
| 3 | No `SystemState` state machine | §14 | No INITIALIZING/CALIBRATING/READY/ADVERTISING/CONNECTED/PAUSED/ERROR flow |
| 4 | Cursor algorithm missing fractional accumulator and nonlinear acceleration `G(ω)=1+β\|ω\|` | §11.2–11.4 | Sub-pixel movement lost, no smooth acceleration curve |
| 5 | `SensorProcessing` uses LPF_ALPHA=0.15f (SRS: 0.25f), CALIBRATION_SAMPLES=1000 (SRS: 500) | §10.2, §8.5 | Wrong filter behavior, wrong calibration duration (2.5s target) |
| 6 | BLE disconnect does not reset cursor accumulator or button HID state | §13.4, §13.5, §18 | Stuck button state after disconnect |
| 7 | `main.cpp` uses `millis()` and `delay()`; SRS requires `micros()` and no blocking | §16.2 | Timing inaccuracies, blocked loop |
| 8 | `platformio.ini` missing `lib_deps`, `monitor_speed`, `upload_speed`, `build_flags` | §15.1, §15.2 | Build may fail; missing BLE/sensor dependencies |
| 9 | Directory structure is flat `include/` + `src/`; SRS §15.3 recommends module subdirectories | §15.3 | Poor modularity, doesn't match spec |
| 10 | No unit tests in `test/` | §19.1 | No verification path |

---

## 2. Target Directory Structure (per SRS §15.3)

```
gyro-mouse/
├── platformio.ini
├── include/
│   ├── Config.h          (central tuning parameters)
│   ├── PinConfig.h       (GPIO/I2C pin assignments)
│   ├── SensorConfig.h    (MPU6050 register map + sensor ranges)
│   └── FirmwareTypes.h   (shared enums/structs: Axis_t, Function_t, SystemState_t, ButtonState_t)
├── src/
│   ├── main.cpp          (composition root, setup/loop, module wiring)
│   ├── hardware/
│   │   ├── MPU6050Driver.cpp / .h   (I2C acquisition, init, detect, configure, readGyroRaw)
│   ├── input/
│   │   ├── ButtonInput.cpp / .h     (GPIO read, independent debounce, press/release events, simultaneous state)
│   ├── processing/
│   │   ├── GyroCalibration.cpp / .h (bias calculation, 500 samples, 2.5s)
│   │   ├── SensorFilter.cpp / .h    (deadzone + EMA low-pass, alpha=0.25)
│   │   ├── AxisMapping.cpp / .h     (configurable source axis + sign per function)
│   │   └── CursorProcessor.cpp / .h (angular integration, acceleration, fractional accumulator, int8 clamp ±127)
│   ├── connectivity/
│   │   ├── BleMouse.cpp / .h        (BLE HID peripheral, report descriptor, advertising, connect/disconnect, reset-on-disconnect)
│   ├── system/
│   │   ├── SystemState.cpp / .h     (state machine: INITIALIZING→CALIBRATING→READY→ADVERTISING→CONNECTED→PAUSED→DISCONNECTED)
│   │   └── RuntimeConfig.cpp / .h   (runtime config: sensitivity, deadzone, mapping, pause, recalibrate, connected)
│   └── diagnostics/
│       └── SerialDebug.cpp / .h     (periodic debug output, non-blocking)
├── test/
│   ├── test_calibration.cpp
│   ├── test_filter.cpp
│   ├── test_axis_mapping.cpp
│   ├── test_cursor.cpp
│   └── test_button.cpp
└── lib/   (empty, reserved for private libraries)
```

---

## 3. Implementation Tasks (ordered)

### Task A — platformio.ini (§15.1, §15.2)

Set up the build environment matching the SRS baseline:

```ini
[env:lolin32_lite]
platform = espressif32
board = lolin32_lite
framework = arduino
monitor_speed = 115200
upload_speed = 460800
lib_deps =
    h2zero/NimBLE-Arduino
build_flags =
    -DCORE_DEBUG_LEVEL=0
monitor_filters =
    time
    colorize
```

**Decision:** Keep the existing custom `MPU6050Driver` (Wire.h-based I2C) instead of adding `adafruit/Adafruit MPU6050`. The custom driver already implements WHO_AM_I detection, wake-up, register config, and raw gyro reads. Adding Adafruit MPU6050 would duplicate functionality and risk dependency conflicts. Keep BLE on the ESP32 Arduino BLE stack (`BLEDevice.h`, `BLEServer.h`, `BLEHIDDevice.h`) already used in the current code — do NOT switch to NimBLE-Arduino since `BLEHIDDevice` is part of the classic ESP32 BLE library, not NimBLE. Remove `adafruit/Adafruit Unified Sensor` and `h2zero/NimBLE-Arduino` from lib_deps; add only if linker errors appear.

### Task B — Configuration headers (include/)

**include/PinConfig.h** — GPIO and I2C pin assignments (§2.2):
- `PIN_I2C_SDA = 21`, `PIN_I2C_SCL = 22`
- `PIN_BUTTON_LEFT = 32`, `PIN_BUTTON_RIGHT = 33` (fix from current 0/4)
- `I2C_ADDR_MPU6050 = 0x68`, `I2C_FREQUENCY = 400000`
- `BUTTON_DEBOUNCE_MS = 25`

**include/SensorConfig.h** — MPU6050 register map + sensor ranges (§2.2):
- Register defines: PWR_MGMT_1=0x6B, SMPLRT_DIV=0x19, CONFIG=0x1A, GYRO_CONFIG=0x1B, ACCEL_CONFIG=0x1C, ACCEL_XOUT_H=0x3B, GYRO_XOUT_H=0x43
- `GYRO_FULL_SCALE_DPS = 500`, `ACCEL_FULL_SCALE_G = 8`, `DLPF_CUTOFF_HZ = 21`
- `GYRO_SENSITIVITY_LSB_PER_DPS = 65.5f` (±500 dps)
- `SAMPLE_RATE_HZ = 200`, `PROCESSING_INTERVAL_MS = 5`
- `CALIBRATION_SAMPLES = 500` (2.5s at 5ms)

**include/FirmwareTypes.h** — Shared types:
- `enum class Axis_t { X, Y, Z, COUNT }`
- `enum class Function_t { UNUSED, VERTICAL, HORIZONTAL, COUNT }`
- `enum class SystemState_t { INITIALIZING, CALIBRATING, READY, ADVERTISING, CONNECTED, PAUSED, ERROR }`
- `enum class ButtonState_t { RELEASED, PRESSED }`
- `struct ButtonEvent { bool leftChanged; bool rightChanged; bool leftPressed; bool rightPressed; }`
- `struct CursorDelta { int8_t dx; int8_t dy; }`

**include/Config.h** — Central tuning constants (§17):
- `DEADZONE = 2.0f`, `FILTER_ALPHA = 0.25f`
- `SENSITIVITY_X = 3.0f`, `SENSITIVITY_Y = 3.0f`
- `ACCELERATION_COEFF = 0.015f` (β)
- `MAX_HID_MOVEMENT = 127`
- `DEBUG_BAUD = 115200`, `DEBUG_INTERVAL_MS = 200`
- Button: `LEFT_BUTTON_PIN=32`, `RIGHT_BUTTON_PIN=33`, `DEBOUNCE_MS=25`, `ACTIVE_LOW=true`

### Task C — MPU6050Driver (src/hardware/)

Refactor existing `MPU6050` into `MPU6050Driver`:
- `begin(TwoWire&)`: wake sensor, WHO_AM_I detection (0x68), return bool
- `configure()`: SMPLRT_DIV=4 (200Hz), DLPF=0x04 (21Hz), GYRO_CONFIG=0x10 (±500dps), ACCEL_CONFIG=0x10 (±8G)
- `readGyroRaw(int16_t&, int16_t&, int16_t&)`: read 6 bytes from GYRO_XOUT_H
- `gyroScale()`: return 65.5f
- `isReady()`: device detection status
- NFR-007: error detection — `begin()` returns false if WHO_AM_I mismatches

### Task D — ButtonInput (src/input/)

New module (§12, NFR-011, NFR-012, NFR-013):
- Constructor takes leftPin, rightPin, debounceMs
- `begin()`: `pinMode(pin, INPUT_PULLUP)` for both
- `update()`: non-blocking, called every loop iteration
  - Reads both GPIOs
  - Independent per-button debounce using `millis()` timestamps
  - State machine: RELEASED ↔ PRESSED (no repeated events while held)
  - Detects edges: returns `ButtonEvent` with changed flags
- `getState()`: current logical state of both buttons
- Simultaneous press support: each button tracked independently (§12.10)
- On disconnect signal: `reset()` — forces both buttons to RELEASED (§13.5, §18)

### Task E — GyroCalibration (src/processing/)

New module (§8):
- `calibrate(MPU6050Driver&, int samples=500)`: averages N raw samples at 5ms intervals (~2.5s)
- Validates stability: if device moves significantly during calibration, reject/repeat (§8.5)
- Stores per-axis bias in deg/s
- `getBias()`: returns bias array
- `isCalibrated()`: status flag

### Task F — SensorFilter (src/processing/)

New module (§10):
- Filter flow: RAW → BIAS COMPENSATION → DEADZONE → LOW-PASS (EMA)
- `applyBias(raw[3], bias[3])`: subtract bias per axis
- `applyDeadzone(value, threshold=2.0f)`: zero if |value| < D, else value ± D
- `lowPass(input, prev, alpha=0.25f)`: EMA `F_t = α·X_t + (1−α)·F_{t-1}`
- `process(raw[3], bias[3], filtered[3])`: full pipeline

### Task G — AxisMapping (src/processing/)

Refactor existing `AxisMapper` (§9):
- `setMapping(Function_t, Axis_t, int8_t sign=+1)`: source axis + direction sign per function
- `transform(gyro[3], &vertical, &horizontal)`: applies mapping and sign
- Default: Y→vertical, Z→horizontal, X→unused (§2.3)
- Sign can be inverted to reverse movement direction

### Task H — CursorProcessor (src/processing/)

Refactor existing `Cursor` with corrected algorithm (§11):
- `update(verticalDegPerSec, horizontalDegPerSec, dtSeconds, CursorDelta& out)`
- Angular integration: `Δθ = ω × Δt`
- Cursor delta: `Δx = Kx × ωz × Δt`, `Δy = Ky × ωy × Δt` (§11.2)
- Nonlinear acceleration: `G(ω) = 1 + β|ω|`, capped (§11.3)
- Fractional accumulator: sub-pixel remainders carried forward (§11.4)
- Clamp output to `[-127, 127]` (§11.6)
- `setSensitivity(x, y)`, `setAcceleration(β)`, `resetAccumulator()` (called on disconnect)

### Task I — BleMouse (src/connectivity/)

Refactor existing `BLEHIDMouse` (§13):
- HID report descriptor: 3-button mouse, relative X/Y, report ID 1 (keep existing descriptor)
- `begin()`: `BLEDevice::init("GyroMouse")`, server, HID service, advertising with appearance 0x03C2
- `loop()`: non-blocking re-advertise when disconnected
- `report(dx, dy, left, right)`: build 5-byte report, notify only when connected
- `onConnect()`: set connected=true, reset button state
- `onDisconnect()`: set connected=false, signal reset to cursor accumulator + button state (§13.4, §13.5, §18)
- `isConnected()`: status

### Task J — SystemState (src/system/)

New module (§14):
- State enum: INITIALIZING → CALIBRATING → READY → ADVERTISING → CONNECTED → PAUSED → DISCONNECTED
- `transitionTo(SystemState_t)`: validate and change state
- `currentState()`: getter
- `isCursorProcessingEnabled()`: false in PAUSED/DISCONNECTED/ERROR
- Button processing continues independently of cursor pause state (§14.2)

### Task K — RuntimeConfig (src/system/)

Refactor existing `RuntimeConfig` (§17):
- Fields: sensitivity (3.0 default), deadzone (2.0), paused, recalibrate, connected
- Axis mapping storage: `Axis_t funcAxis[FUNC_COUNT]` + sign per function
- `reset()`: defaults per §17
- Singleton `extern RuntimeConfig cfg`

### Task L — SerialDebug (src/diagnostics/)

New module (§17, FR-017):
- `begin(baud)`: init Serial
- `printLoopInfo(...)`: periodic non-blocking output (gx, gy, gz, dx, dy, button states, connection)
- Guarded by `DEBUG_INTERVAL_MS` to avoid excessive printing (§16.2)

### Task M — main.cpp (src/)

Composition root (§6, §16):
- `setup()`:
  1. Serial init
  2. I2C init (`Wire.begin(21, 22)`, `Wire.setClock(400000)`)
  3. MPU6050Driver begin + configure; on failure → SystemState::ERROR, halt
  4. ButtonInput begin
  5. GyroCalibration (automatic at startup, NFR-008)
  6. SensorFilter + AxisMapping + CursorProcessor init
  7. BleMouse begin (advertising)
  8. SystemState: INITIALIZING → CALIBRATING → READY → ADVERTISING
- `loop()` (non-blocking, no `delay()`):
  - `bleMouse.loop()`
  - `buttonInput.update()` → ButtonEvent → build HID button bits
  - If connected and not paused:
    - Measure `dt` with `micros()`
    - `mpu.readGyroRaw()`
    - Filter pipeline (bias → deadzone → LPF)
    - Axis mapping
    - Cursor update → fractional accumulator → dx, dy
    - Build HID report if dx/dy/buttons changed
    - Serial debug output (throttled)
  - If disconnected: reset cursor accumulator, reset button state, continue advertising
  - Recalibration trigger: if `cfg.recalibrate`, re-run calibration, clear flag

### Task N — Unit tests (test/)

Per §19.1:
- `test_calibration.cpp`: still sensor → bias measured; moving sensor → calibration rejected
- `test_filter.cpp`: input below deadzone → zero output; input above → filtered output
- `test_axis_mapping.cpp`: gyro Z → horizontal output; gyro Y → vertical output
- `test_cursor.cpp`: positive angular velocity → correct direction; negative → opposite
- `test_button.cpp`: press → down; release → up; bounce → no duplicate; both pressed → both bits; hold → no repeat

Tests use the Arduino Unit Testing framework (PlatformIO Unit Testing). Each test instantiates the module class directly (no hardware dependency) with mocked inputs where needed.

---

## 4. Algorithm Specifications (verbatim from SRS)

### Cursor movement (§11)
```
Δx = Kx × ωz × Δt × G(ωz)
Δy = Ky × ωy × Δt × G(ωy)
G(ω) = 1 + β|ω|          (β = 0.015)
accumulatorX += Δx; outputX = int(accumulatorX); accumulatorX -= outputX
accumulatorY += Δy; outputY = int(accumulatorY); accumulatorY -= outputY
Clamp output to [-127, 127]
```

### Filtering flow (§10.3)
```
RAW GYRO → BIAS COMPENSATION → DEADZONE(D=2.0) → LOW-PASS(α=0.25) → FILTERED GYRO
```

### Button debounce (§12.5)
```
T_stable ≥ T_debounce (25 ms)
```
Independent per-button, non-blocking timestamp-based.

### State machine (§14.2)
```
INITIALIZING → CALIBRATING → READY → ADVERTISING → CONNECTED → PAUSED → DISCONNECTED → ADVERTISING
```

### Disconnect behavior (§13.4, §13.5, §18)
- Cursor report not sent
- Button HID state reset to released
- Cursor accumulator reset
- No stale button state retained

---

## 5. Validation & Acceptance (§21)

MVP acceptance criteria:
1. ESP32 initializes successfully
2. MPU6050 read consistently
3. Gyro calibration produces usable bias
4. Horizontal rotation → horizontal cursor movement
5. Vertical rotation → vertical cursor movement
6. Cursor stops when device stops rotating
7. Sensor noise doesn't cause significant cursor movement
8. BLE HID Mouse operation
9. Host pairing and report reception
10. Button 1 → left click
11. Button 2 → right click
12. Press/release → correct HID button state
13. Simultaneous button press
14. Button hold → drag operation
15. No bounce-induced double clicks
16. BLE disconnect clears button state
17. Stable operation during continuous testing

---

## 6. Risks & Mitigations

| Risk | Mitigation |
|------|-----------|
| `lolin32_lite` board ID may not exist in installed espressif32 platform | Fallback to `esp32dev` board ID (§15.2 note) |
| BLE HID compatibility varies by host OS | Test on Windows, Linux, macOS, Android, iOS (§19.3) |
| MPU6050 I2C address could be 0x69 on some units | Detect WHO_AM_I; report error if not 0x68 |
| Gyro drift over time | Startup calibration only; note stationary bias estimation as future (§22.1) |
| Blocking BLE calls stall sensor loop | Keep BLE operations non-blocking; `loop()` only re-advertises (§16.2) |

---

## 7. Open Questions (to resolve before implementation)

1. **MPU6050 library:** Keep custom Wire.h driver (already works) vs switch to Adafruit MPU6050 as SRS §15.2 suggests? Recommendation: keep custom driver.
2. **BLE stack:** Keep classic ESP32 Arduino BLE (`BLEDevice.h`/`BLEHIDDevice.h`) vs NimBLE-Arduino? Recommendation: keep classic — current code already uses it and `BLEHIDDevice` is not in NimBLE.
3. **Test hardware availability:** Are unit tests run on host (mocked) or on target ESP32? Recommendation: host-based unit tests with mocked inputs for CI-friendly validation.

---

**Plan status:** Draft — ready for review. Once approved, an implementation-capable agent can execute Tasks A–M in order, then Task N.