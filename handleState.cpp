#include "handleState.h"

// HELPER
String readWeather()
{
  return String(inputs.temperature) + "C " + String(inputs.humidity) + "%RH " + String(inputs.waterLevel) + "RD";
}

// HLANDLERS
void handleClosing()
{
  Serial.println("Handling CLOSING state");
  outputs.yellowLed.action = YellowLedActuatorState::BLINK;
  outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BREATHERED;

  outputs.window.action = WindowActuatorState::CLOSED;
  outputs.window.delayMs = 2000UL;

  outputs.fan.directionAction = FanDirectionActuatorState::OFF;
  outputs.fan.speedAction = 0;

  outputs.lcd.count = 1;
  outputs.lcd.messages[0] = "House Closing";
  outputs.lcd.delayTime = 2000;
  outputs.lcd.currentHash = "34f5g6h7j8k9a1s2d";

  outputs.buzzer.action = BuzzerActuatorState::CLOSING;
  Serial.println("Closed state handled.");
}

void handleClosed()
{
  Serial.println("Handling CLOSED state");
  outputs.yellowLed.action = YellowLedActuatorState::OFF;
  outputs.ledNeoPixel.action = LedNeoPixelActuatorState::OFF;

  outputs.window.action = WindowActuatorState::CLOSED;
  outputs.window.delayMs = 2000UL;

  outputs.fan.directionAction = FanDirectionActuatorState::OFF;
  outputs.fan.speedAction = 0;

  outputs.lcd.count = 1;
  outputs.lcd.messages[0] = "House Closed";
  outputs.lcd.delayTime = 2000;
  outputs.lcd.currentHash = "d4f5g6h7j8k9a1s23";

  outputs.buzzer.action = BuzzerActuatorState::OFF;
  Serial.println("Closed state handled.");
}

void handleOpening()
{
  Serial.println("Handling OPENING state");
  outputs.yellowLed.action = YellowLedActuatorState::BLINK;
  outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BREATHGREEN;

  outputs.window.action = WindowActuatorState::OPEN;
  outputs.window.delayMs = 2000UL;

  outputs.fan.directionAction = FanDirectionActuatorState::OFF;
  outputs.fan.speedAction = 0;

  outputs.lcd.count = 1;
  outputs.lcd.messages[0] = "House Openning";
  outputs.lcd.delayTime = 2000;
  outputs.lcd.currentHash = "89k9a1s23d4f5g6h7j";

  outputs.buzzer.action = BuzzerActuatorState::GRANTED_ACCESS;
  Serial.println("Opened state handled.");
}

void handleOpen()
{
  Serial.println("Handling OPEN state");
  outputs.yellowLed.action = YellowLedActuatorState::ON;
  outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BLUE;

  outputs.window.action = WindowActuatorState::OPEN;
  outputs.window.delayMs = 2000UL;

  outputs.fan.directionAction = FanDirectionActuatorState::OFF;
  outputs.fan.speedAction = 0;

  outputs.lcd.count = 2;
  outputs.lcd.messages[0] = "House Open";
  outputs.lcd.messages[1] = readWeather();

  outputs.lcd.delayTime = 2000;
  outputs.lcd.currentHash = "g6h7j8k9a1s23d4f5";

  outputs.buzzer.action = BuzzerActuatorState::OFF;
  Serial.println("Opened state handled.");
}

void handleAlert()
{
  Serial.println("Handling ALERT state");
  outputs.yellowLed.action = YellowLedActuatorState::BLINK;
  outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BREATHERED;

  outputs.window.action = WindowActuatorState::OPEN;
  outputs.window.delayMs = 1000UL;

  outputs.fan.directionAction = FanDirectionActuatorState::REVERSE;
  outputs.fan.speedAction = 200;

  outputs.lcd.count = 1;
  outputs.lcd.messages[0] = "Gas Leak!";
  outputs.lcd.delayTime = 2000;
  outputs.lcd.currentHash = "j8k9a1s23d4f5g6h7";

  outputs.buzzer.action = BuzzerActuatorState::ALARM_GAS;
  Serial.println("Alert state handled.");
}

void handleAutomations()
{
  // if water level is too high
  // set the window to close immediately
  if (systemState.current == SystemStates::OPEN)
  {
    if (inputs.waterLevel >= 1500 || inputs.temperature < 18)
    {
      outputs.window.action = WindowActuatorState::CLOSED;
      outputs.window.delayMs = 0;

      outputs.lcd.messages[1] = readWeather();
      outputs.lcd.count = 2;
      outputs.lcd.delayTime = 3000;
      outputs.lcd.currentHash = "jmh67j8k9a1s23d4f5g";
    }
    else
    {
      outputs.window.action = WindowActuatorState::OPEN;
      outputs.window.delayMs = 0;

      outputs.lcd.messages[1] = readWeather();
      outputs.lcd.count = 2;
      outputs.lcd.delayTime = 3000;
      outputs.lcd.currentHash = "po6d4f5g6h7j8k9a1s2";
    }
  }
}