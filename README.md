
# Arduino Home & Safety Controller

This project implements a simple state-driven home and safety controller for an ESP32-based board. The firmware reads sensors, maintains a system state machine, and drives actuators (window servo, fan, LEDs, buzzer, and LCD) and automations.

## Logic System

- The core is a state machine defined in `system_state.h` and handled in `handleState.cpp`.
- Each loop cycle: `readInputs()` → `updateSystemState()` → `buildOutputsAction()` → `applyOutputs()`.
- States determine the desired actuator *actions*; a separate apply step enforces those actions on hardware.

## Inputs

The firmware reads values from these inputs (names reflect the project's files and defines):

- `dht.h` (DHT11): temperature and humidity (`DHT11PIN`).
- `nfc.h`: NFC reader; supports a whitelist of UIDs (`NFC_GRANTED_UIDS`).
- Digital inputs (pins defined in `system_state.h`): buttons (`btn_1`, `btn_2`), `person_sensor` (presence), `gas_sensor`, and analog `water_sensor`.

These inputs are aggregated in the `InputsState` struct: `btn1Pressed`, `btn2Pressed`, `presenceDetected`, `gasDetected`, `waterLevel`, `nfcUid`, `temperature`, `humidity`.

## Outputs

Actuators and displays controlled by the firmware:

- `window.h`: window servo (open/close).
- `fan.h`: fan with forward/reverse and speed control.
- `buzzer.h`: buzzer with predefined tones (welcome, granted, closing, alarm, etc.).
- `led.h` and `neo-pixel.h`: status LEDs and NeoPixel animations.
- `lcd.h`: 16x2 LCD messages (up to `LCD_MAX_MESSAGES`).

Output states are stored in the `OutputsState` struct, with per-actuator enums for action/state.

## System States

Defined in `system_state.h` as `SystemStates`:

- `STARTUP` — initial boot state.
- `CLOSING` — system is closing the window/house (transitional).
- `CLOSED` — closed/secured state.
- `OPENING` — opening transition.
- `OPEN` — open/ventilating state.
- `ALERT` — alarm condition (e.g., gas leak).

Each state has a handler in `handleState.cpp` (e.g., `handleClosing()`, `handleClosed()`, `handleOpening()`, `handleOpen()`, `handleAlert()`) that sets outputs' actions, LCD messages, buzzer actions, and other behaviors.

## System Automations (rules)

- When in `OPEN` state:
	- If `waterLevel >= 1500` OR `temperature < 18°C`, the system will command the window to CLOSE immediately and update LCD with the latest weather/readings.
	- Otherwise the window remains OPEN and LCD shows current weather.
- If `gasDetected` is true, the system switches to `ALERT` state: opens window, sets fan to `REVERSE` at high speed, blinks LEDs, and sets buzzer to `ALARM_GAS`.
- NFC whitelist (`NFC_GRANTED_UIDS`) is used to grant access and trigger transitions (e.g., `GRANTED_ACCESS` buzzer and `OPENING` sequence) when a recognized UID is presented.

Other automations are implemented by combining input checks inside `handleAutomations()` and per-state handlers in `handleState.cpp`.

## Wiring / Notes

- Pin mappings and hardware settings are defined in `system_state.h` (pins for buttons, sensors, LED_PIN, servo, fan pins, buzzer pin, etc.).
- LCD: I2C address `LCD_ADDRESS` (default 0x27), `LCD_COLUMNS` and `LCD_ROWS` set for a 16x2 display.
- NeoPixel: `LED_PIN` and `LED_COUNT` are defined in `system_state.h`.
- Adjust `WINDOW_MIN_PULSE_WIDTH`, `WINDOW_MAX_PULSE_WIDTH`, and angle defines to fit your servo.

## Build / Upload

Build using the Arduino IDE or `arduino-cli`. Example compile command used in this workspace:

```
arduino-cli compile --fqbn esp32:esp32:esp32
```

## Files of interest

- `my-code.ino` — main sketch entrypoint.
- `system_state.h` / `handleState.cpp` — state machine and handlers.
- `dht.*`, `nfc.*`, `window.*`, `fan.*`, `buzzer.*`, `lcd.*`, `led.*`, `neo-pixel.*` — drivers for sensors and actuators.

If you'd like, I can add a wiring diagram, expand the automations section into a flow chart, or annotate the code with in-line comments. 
