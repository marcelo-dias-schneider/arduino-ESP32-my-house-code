#ifndef NEO_PIXEL_H
#define NEO_PIXEL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Forward-declare the enum to avoid circular include in some translation units
enum class LedNeoPixelActuatorState;

extern Adafruit_NeoPixel strip;

void startNeoPixel(LedNeoPixelActuatorState state); // start or advance effects; call each applyOutputs()

#endif
