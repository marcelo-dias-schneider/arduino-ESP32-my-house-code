# Keyestudio IoT Smart Home Kit for ESP32

[Keyestudio Documentations](https://docs.keyestudio.com/projects/KS5009/en/latest/docs/)

# Arduino Home & Safety Controller

This project implements a simple state-driven home and safety controller for an ESP32-based board. The firmware reads sensors, maintains a system state machine, and drives actuators (window servo, fan, LEDs, buzzer, and LCD) and automations.

## Logic System

- The core is a state machine defined in `system_state.h` and handled in `handleState.cpp`.
- Each loop cycle: `readInputs()` → `updateSystemState()` → `buildOutputsAction()` → `applyOutputs()`.
- States determine the desired actuator _actions_; a separate apply step enforces those actions on hardware.

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
  - Window control is driven by temperature only:
    - If `temperature <= 18°C` the system commands the window to CLOSE.
    - If `temperature > 18°C` the system commands the window to OPEN.
  - LCD weather message (`outputs.lcd.messages[1]`) displays temperature, humidity and water-level readings and will update when any of those three readings change. Updates are throttled to at most once every 500 ms and the display hash is refreshed so the LCD will redraw the new message.
  - Fan automation:
    - If `temperature >= 27°C` the fan is turned ON in `FORWARD` direction at configured speed.
    - If `temperature < 27°C` the fan is turned OFF.
- If `gasDetected` is true the system switches to `ALERT` state: opens the window, sets the fan to `REVERSE` at high speed, blinks LEDs, and sets the buzzer to `ALARM_GAS`.
- NFC whitelist (`NFC_GRANTED_UIDS`) grants access and triggers the `GRANTED_ACCESS` buzzer and `OPENING` sequence when a recognized UID is presented.

Other automations are implemented by combining input checks inside `handleAutomations()` and per-state handlers in `handleState.cpp`. Previous behavior that used water-level alone to force a close has been replaced by the current temperature-driven window logic; water-level remains shown on the LCD and can influence manual decisions.

## Wiring / Notes

- Pin mappings and hardware settings are defined in `system_state.h` (pins for buttons, sensors, LED_PIN, servo, fan pins, buzzer pin, etc.).
- LCD: I2C address `LCD_ADDRESS` (default 0x27), `LCD_COLUMNS` and `LCD_ROWS` set for a 16x2 display.
- NeoPixel: `LED_PIN` and `LED_COUNT` are defined in `system_state.h`.
- Adjust `WINDOW_MIN_PULSE_WIDTH`, `WINDOW_MAX_PULSE_WIDTH`, and angle defines to fit your servo.

## Build / Upload

Build using the Arduino IDE or `arduino-cli`. Example compile command used in this workspace:

```
arduino-cli compile --fqbn esp32:esp32:esp32
arduino-cli upload -p {PORT USB CONNECTED} --fqbn esp32:esp32:esp32
```

PORT USB ESEMPLE: COM3 | COM5 

## Files of interest

- `my-code.ino` — main sketch entrypoint.
- `system_state.h` / `handleState.cpp` — state machine and handlers.
- `dht.*`, `nfc.*`, `window.*`, `fan.*`, `buzzer.*`, `lcd.*`, `led.*`, `neo-pixel.*` — drivers for sensors and actuators.
