#ifndef LCD_H
#define LCD_H

#include <Arduino.h>
#include "system_state.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C mylcd;

// Initialize the LCD (call once in setup)
void lcdInit();

// Call regularly from loop() to update display without blocking
void lcdLoop();

// Non-blocking: set the messages to show. Returns immediately.
// `delayTime` is used for both scroll step interval and page display time
void setMessageLCD(String messages[], int count, unsigned long delayTime);

#endif
