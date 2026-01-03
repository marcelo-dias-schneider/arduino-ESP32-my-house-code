#include "system_state.h"

// define shared variables (storage)
SystemState systemState;
InputsState inputs;
OutputsState outputs;
PrevEnvReadings prevEnvReadings;

// Delay for showing animations (milliseconds)
const unsigned long CLOSING_DELAY_MS = 2000UL;
const unsigned long OPENING_DELAY_MS = 2000UL;
const unsigned long ALERT_DELAY_MS = 4000UL;

void setPins()
{
  Serial.begin(115200);
  // INPUT and SENSOR
  pinMode(btn_1, INPUT);
  pinMode(btn_2, INPUT);
  pinMode(person_sensor, INPUT);
  pinMode(gas_sensor, INPUT);
  pinMode(water_sensor, INPUT);

  // OUTPUT and ACTUATOR
  pinMode(yellow_led, OUTPUT);
  // Initialize I2C for LCD and NFC

  // Window servo
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(WINDOW_PERIOD_HERTZ); // standard 50 hz servo
  // attaches the servo on pin 18 to the servo object
  // using default min/max of 1000us and 2000us
  // different servos may require different min/max settings
  // for an accurate 0 to 180 sweep
  myservo.attach(WINDOW_SERVO, WINDOW_MIN_PULSE_WIDTH, WINDOW_MAX_PULSE_WIDTH);
  delay(200); // wait for servo to stabilize
  lcdInit();

  // Fan control pins
  pinMode(FAN_FORWARD_PIN, OUTPUT);
  pinMode(FAN_REVERSE_PIN, OUTPUT);

  Wire.begin(); // I2C initialization for both LCD and NFC
  // Initialize LCD
  mylcd.init();
  mylcd.backlight();
  // Initialize NFC
  mfrc522.PCD_Init();

// NeoPixel
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1); // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
#endif
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  // Buzzer
  buzzer.setTimbre(30); // Set timbre (0-127)
}

void setSystemState(SystemStates newState)
{
  systemState.previous = systemState.current;
  systemState.current = newState;
  systemState.enteredAt = millis();
  systemState.lastHandled = systemState.previous; // ensure entry handler will run
}

void setupSystemState()
{
  systemState.previous = SystemStates::STARTUP;
  systemState.current = SystemStates::CLOSING;
  systemState.lastHandled = SystemStates::STARTUP;
  systemState.enteredAt = millis();

  outputs.yellowLed.action = YellowLedActuatorState::OFF;
  outputs.ledNeoPixel.action = LedNeoPixelActuatorState::OFF;

  outputs.window.action = WindowActuatorState::CLOSED;
  outputs.window.state = WindowActuatorState::OPEN;
  outputs.window.delayMs = 0;

  outputs.fan.directionAction = FanDirectionActuatorState::OFF;
  outputs.fan.speedAction = 0;

  outputs.lcd.messages[0] = "Initializing...";
  outputs.lcd.messages[1] = "";
  outputs.lcd.messages[2] = "";
  outputs.lcd.messages[3] = "";
  outputs.lcd.messages[4] = "";
  outputs.lcd.messages[5] = "";
  outputs.lcd.messages[6] = "";
  outputs.lcd.count = 1;
  outputs.lcd.delayTime = 2000;
  outputs.lcd.currentHash = "a1s23d4f5g6h7j8k9";
  outputs.lcd.lastHash = "";

  outputs.buzzer.action = BuzzerActuatorState::WELCOME;

  // initialize previous environmental readings to sentinel values
  prevEnvReadings.temperature = -9999;
  prevEnvReadings.humidity = -9999;
  prevEnvReadings.waterLevel = -999999;
  prevEnvReadings.lastWeatherUpdate = 0;
  prevEnvReadings.lastNoPersonMillis = 0;
}

void readInputs()
{
  inputs.btn1Pressed = digitalRead(btn_1) == LOW;
  inputs.btn2Pressed = digitalRead(btn_2) == LOW;
  inputs.presenceDetected = digitalRead(person_sensor);
  inputs.gasDetected = !digitalRead(gas_sensor);
  inputs.waterLevel = analogRead(water_sensor);
  inputs.nfcUid = nfcRead();
  inputs.temperature = readTemperature();
  inputs.humidity = readHumidity();
}

void updateSystemState()
{
  if (inputs.gasDetected && systemState.current != SystemStates::ALERT)
  {
    // wait a short, non-blocking time for animation can run
    if (millis() - systemState.enteredAt >= ALERT_DELAY_MS)
      setSystemState(SystemStates::ALERT);
    return;
  }

  if (!inputs.gasDetected && systemState.current == SystemStates::ALERT)
  {
    // wait a short, non-blocking time for animation can run
    if (millis() - systemState.enteredAt >= ALERT_DELAY_MS)
      setSystemState(systemState.previous);
    return;
  }

  switch (systemState.current)
  {
  case SystemStates::CLOSING:
  {
    // wait a short, non-blocking time for animation can run
    if (millis() - systemState.enteredAt >= CLOSING_DELAY_MS)
      setSystemState(SystemStates::CLOSED);
    break;
  }
  case SystemStates::CLOSED:
    if (isNfcUidGranted(inputs.nfcUid) || inputs.btn1Pressed)
      setSystemState(SystemStates::OPENING);
    break;

  case SystemStates::OPENING:
  {
    // wait a short, non-blocking time for animation can run
    if (millis() - systemState.enteredAt >= OPENING_DELAY_MS)
      setSystemState(SystemStates::OPEN);
    break;
  }
  case SystemStates::OPEN:
    if (inputs.btn2Pressed)
      setSystemState(SystemStates::CLOSING);
    break;
  }
}

void buildOutputsAction()
{
  handleAutomations();

  // Only run the handler once when entering a new state
  if (systemState.current == systemState.lastHandled)
    return;

  switch (systemState.current)
  {
  case SystemStates::CLOSING:
    handleClosing();
    systemState.lastHandled = SystemStates::CLOSING;
    logOutputsState();
    return;
  case SystemStates::CLOSED:
    handleClosed();
    systemState.lastHandled = SystemStates::CLOSED;
    logOutputsState();
    return;
  case SystemStates::OPENING:
    handleOpening();
    systemState.lastHandled = SystemStates::OPENING;
    logOutputsState();
    return;
  case SystemStates::OPEN:
    handleOpen();
    systemState.lastHandled = SystemStates::OPEN;
    logOutputsState();
    return;
  case SystemStates::ALERT:
    handleAlert();
    systemState.lastHandled = SystemStates::ALERT;
    logOutputsState();
    return;
  }
}

void applyOutputs()
{
  // YELLOW LED
  switch (outputs.yellowLed.action)
  {
  case YellowLedActuatorState::OFF:
  {
    if (outputs.yellowLed.state != YellowLedActuatorState::OFF)
    {
      yellowLedOff();
      outputs.yellowLed.state = YellowLedActuatorState::OFF;
    }
    break;
  }
  case YellowLedActuatorState::ON:
  {
    if (outputs.yellowLed.state != YellowLedActuatorState::ON)
    {
      yellowLedOn();
      outputs.yellowLed.state = YellowLedActuatorState::ON;
    }
    break;
  }
  case YellowLedActuatorState::BLINK:
  {
    blinkYellowLed();
    outputs.yellowLed.state = YellowLedActuatorState::BLINK;
    break;
  }
  }

  // NeoPixel
  startNeoPixel(outputs.ledNeoPixel.action);

  // WINDOW
  if (outputs.window.action != outputs.window.state)
  {
    switch (outputs.window.action)
    {
    case WindowActuatorState::CLOSED:
      closeWindow(outputs.window.delayMs);
      break;
    case WindowActuatorState::OPEN:
      openWindow(outputs.window.delayMs);
      break;
    }
    outputs.window.state = outputs.window.action;
  }
  windowUpdate(); // WINDOW: process scheduled open/close without blocking

  // FAN
  if (outputs.fan.directionAction != outputs.fan.directionState ||
      outputs.fan.speedAction != outputs.fan.speedState)
  {
    setFan(outputs.fan.directionAction, outputs.fan.speedAction);
    outputs.fan.directionState = outputs.fan.directionAction;
    outputs.fan.speedState = outputs.fan.speedAction;
  }

  // LCD DISPLAY
  if (outputs.lcd.count > 0 && outputs.lcd.currentHash != outputs.lcd.lastHash)
  {
    setMessageLCD(outputs.lcd.messages, outputs.lcd.count, outputs.lcd.delayTime);
  }
  outputs.lcd.lastHash = outputs.lcd.currentHash;
  lcdLoop();

  // BUZZER
  if (outputs.buzzer.action != outputs.buzzer.state)
  {
    switch (outputs.buzzer.action)
    {
    case BuzzerActuatorState::OFF:
      buzzerOff();
      break;
    case BuzzerActuatorState::WELCOME:
      buzzerWelcome();
      break;
    case BuzzerActuatorState::GRANTED_ACCESS:
      buzzerGrantedAccess();
      break;
    case BuzzerActuatorState::CLOSING:
      buzzerClosing();
      break;
    case BuzzerActuatorState::ALARM_GAS:
      buzzerAlarmGas();
      break;
    case BuzzerActuatorState::WARNING_ERROR:
      buzzerWarningError();
      break;
    }

    outputs.buzzer.state = outputs.buzzer.action;
  }

  // Advance non-blocking buzzer sequencer each time outputs are applied
  buzzerUpdate();
}

void logOutputsState()
{
  Serial.println("##### States #####");
  Serial.println("Current State: " + String(static_cast<int>(systemState.current)));
  Serial.println("Previous State: " + String(static_cast<int>(systemState.previous)));
  Serial.println("Last Handled State: " + String(static_cast<int>(systemState.lastHandled)));
  Serial.println("                         ");
  Serial.println("##### Inputs #####");
  Serial.println("Button 1 Pressed: " + String(inputs.btn1Pressed));
  Serial.println("Button 2 Pressed: " + String(inputs.btn2Pressed));
  Serial.println("Presence Detected: " + String(inputs.presenceDetected));
  Serial.println("Gas Detected: " + String(inputs.gasDetected));
  Serial.println("Water Level: " + String(inputs.waterLevel));
  Serial.println("Temperature: " + String(inputs.temperature));
  Serial.println("Humidity: " + String(inputs.humidity));  
  Serial.println("                         ");
  Serial.println("##### Outputs #####");
  Serial.println("Yellow LED Action: " + String(static_cast<int>(outputs.yellowLed.action)));
  Serial.println("LED NeoPixel Action: " + String(static_cast<int>(outputs.ledNeoPixel.action)));
  Serial.println("Window Action: " + String(static_cast<int>(outputs.window.action)));
  Serial.println("Fan Direction Action: " + String(static_cast<int>(outputs.fan.directionAction)));
  Serial.println("Fan Speed Action: " + String(outputs.fan.speedAction));
  Serial.println("Buzzer Action: " + String(static_cast<int>(outputs.buzzer.action)));
  Serial.println("-----");
}
