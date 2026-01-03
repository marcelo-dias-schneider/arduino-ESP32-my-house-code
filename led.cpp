#include "led.h"

void yellowLedOn()
{
  digitalWrite(yellow_led, HIGH);
}

void yellowLedOff()
{
  digitalWrite(yellow_led, LOW);
}

// Minimal non-blocking blink using millis() for a single LED.
// Call `blinkYellowLed()` repeatedly from `loop()` while you want
// that pin to blink. When you want the LED off, write the pin LOW.
void blinkYellowLed()
{
  static unsigned long last = 0;
  static bool state = false;
  const unsigned long interval = 200;

  unsigned long now = millis();
  if (now - last >= interval)
  {
    last = now;
    state = !state;
    digitalWrite(yellow_led, state ? HIGH : LOW);
  }
}
