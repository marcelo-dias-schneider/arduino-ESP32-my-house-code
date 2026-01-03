#include "window.h"

#include <ESP32Servo.h>

Servo myservo;

// Window scheduler state (file-local)
static bool scheduledOpen = false;
static bool scheduledClose = false;
static unsigned long scheduledTime = 0;

void openWindow(unsigned long delayMs)
{
  if (delayMs == 0)
  {
    myservo.write(WINDOW_OPEN_ANGLE);
    scheduledOpen = scheduledClose = false;
  }
  else
  {
    scheduledOpen = true;
    scheduledClose = false;
    scheduledTime = millis() + delayMs;
  }
}

void closeWindow(unsigned long delayMs)
{
  if (delayMs == 0)
  {
    myservo.write(WINDOW_CLOSED_ANGLE);
    scheduledOpen = scheduledClose = false;
  }
  else
  {
    scheduledClose = true;
    scheduledOpen = false;
    scheduledTime = millis() + delayMs;
  }
}

void windowUpdate()
{
  if (!(scheduledOpen || scheduledClose))
    return;
  unsigned long now = millis();
  if (now < scheduledTime)
    return;

  if (scheduledOpen)
  {
    myservo.write(WINDOW_OPEN_ANGLE);
  }
  else if (scheduledClose)
  {
    myservo.write(WINDOW_CLOSED_ANGLE);
  }
  scheduledOpen = scheduledClose = false;
}