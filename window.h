#ifndef WINDOW_H
#define WINDOW_H

#include <Arduino.h>
#include "system_state.h"
#include <ESP32Servo.h>

extern Servo myservo;

// Non-blocking window control: schedule open/close with optional delay (ms).
void openWindow(unsigned long delayMs = 0);
void closeWindow(unsigned long delayMs = 0);
// Call regularly from applyOutputs() to process scheduled actions
void windowUpdate();

#endif
