#include "fan.h"

void setFan(FanDirectionActuatorState direction, uint8_t speed)
{
  // Set fan direction pins
  switch (direction)
  {
  case FanDirectionActuatorState::OFF:
  {
    Serial.println("Fan OFF");
    digitalWrite(FAN_FORWARD_PIN, LOW);
    analogWrite(FAN_FORWARD_PIN, 0);
    digitalWrite(FAN_REVERSE_PIN, LOW);
    analogWrite(FAN_REVERSE_PIN, 0);
    break;
  }
  case FanDirectionActuatorState::FORWARD:
  {
    Serial.println("Fan FORWARD at speed " + String(speed));
    digitalWrite(FAN_REVERSE_PIN, LOW);
    analogWrite(FAN_REVERSE_PIN, 0);
    digitalWrite(FAN_FORWARD_PIN, HIGH);
    analogWrite(FAN_FORWARD_PIN, speed);
    break;
  }
  case FanDirectionActuatorState::REVERSE:
  {
    Serial.println("Fan REVERSE at speed " + String(speed));
    digitalWrite(FAN_FORWARD_PIN, LOW);
    analogWrite(FAN_FORWARD_PIN, 0);
    digitalWrite(FAN_REVERSE_PIN, HIGH);
    analogWrite(FAN_REVERSE_PIN, speed);
    break;
  }
  }
}