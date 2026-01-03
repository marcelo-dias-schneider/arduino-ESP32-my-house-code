#include "system_state.h"
#include "neo-pixel.h"
// Define the NeoPixel strip instance used across the sketch
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
/*
  Non-blocking NeoPixel effect state (file-local, persistent between calls)

  These `static` variables are initialized once and retain their values
  between calls to `startNeoPixel()` so the single function can both
  initialize and advance effects each time it's called from `applyOutputs()`.

  Tuning notes:
  - Blink speed: change `blinkInterval` (milliseconds). Smaller = faster.
  - Blink length: change `blinkCyclesRemaining` when starting the effect.
  - Breath speed: change `breathStepDelay` (ms between brightness steps).
    Smaller = faster. Also adjust `breathStepSize`.
  - `lastMillis` is used with `millis()` for non-blocking timing.

  Key variables:
  - `activeState` : current running effect.
  - `activeColor` : color used for the current effect.
  - `blinkInterval`, `blinkCyclesRemaining`, `blinkOn` : control blinking.
  - `breathStepDelay`, `breathBrightness`, `breathDir`, `breathCyclesRemaining` : breathing control.

  Example adjustments (no code change shown here):
  - Faster blink: `blinkInterval = 100`.
  - Slower breath: `breathStepDelay = 60` and reduce step delta to 3.
*/
// Non-blocking NeoPixel effect state
static LedNeoPixelActuatorState activeState = LedNeoPixelActuatorState::OFF;
static uint32_t activeColor = 0;
static uint16_t wipeIndex = 0;
static unsigned long lastMillis = 0;
static unsigned int stepInterval = 50; // default interval
static int blinkCyclesRemaining = 0;
static bool blinkOn = false;
static int blinkInterval = 200;
static int breathCycleCount = 0;
static int breathCyclesRemaining = 0;
static int breathStepDelay = 3;
static int breathBrightness = 0;
static int breathStepSize = 20;
static int breathDir = 1;

// helper: set all pixels to a color and show
static void setAllPixels(uint32_t color)
{
  for (uint16_t i = 0; i < strip.numPixels(); ++i)
    strip.setPixelColor(i, color);
  strip.show();
}

void startNeoPixel(LedNeoPixelActuatorState requested)
{
  unsigned long now = millis();

  // If a new request arrives, initialize the effect
  if (requested != activeState)
  {
    activeState = requested;
    lastMillis = now;
    wipeIndex = 0;
    blinkOn = false;

    switch (requested)
    {
    case LedNeoPixelActuatorState::OFF:
      activeColor = strip.Color(0, 0, 0);
      setAllPixels(activeColor);
      break;
    case LedNeoPixelActuatorState::RED:
      activeColor = strip.Color(255, 0, 0);
      setAllPixels(activeColor);
      break;
    case LedNeoPixelActuatorState::GREEN:
      activeColor = strip.Color(0, 255, 0);
      setAllPixels(activeColor);
      break;
    case LedNeoPixelActuatorState::BLUE:
      activeColor = strip.Color(0, 0, 255);
      setAllPixels(activeColor);
      break;
    case LedNeoPixelActuatorState::BLINKRED:
      activeColor = strip.Color(255, 0, 0);
      blinkCyclesRemaining = 5;
      blinkInterval = 200;
      break;
    case LedNeoPixelActuatorState::BLINKGREEN:
      activeColor = strip.Color(0, 255, 0);
      blinkCyclesRemaining = 5;
      blinkInterval = 200;
      break;
    case LedNeoPixelActuatorState::BLINKBLUE:
      activeColor = strip.Color(0, 0, 255);
      blinkCyclesRemaining = 5;
      blinkInterval = 200;
      break;
    case LedNeoPixelActuatorState::BREATHERED:
      activeColor = strip.Color(255, 0, 0);
      breathCyclesRemaining = 3;
      breathStepDelay = 30;
      breathBrightness = 0;
      breathDir = 1;
      break;
    case LedNeoPixelActuatorState::BREATHGREEN:
      activeColor = strip.Color(0, 255, 0);
      breathCyclesRemaining = 3;
      breathStepDelay = 30;
      breathBrightness = 0;
      breathDir = 1;
      break;
    case LedNeoPixelActuatorState::BREATHBLUE:
      activeColor = strip.Color(0, 0, 255);
      breathCyclesRemaining = 3;
      breathStepDelay = 30;
      breathBrightness = 0;
      breathDir = 1;
      break;
    }
  }

  // Advance the currently active effect (non-blocking)
  switch (activeState)
  {
  case LedNeoPixelActuatorState::OFF:
  case LedNeoPixelActuatorState::RED:
  case LedNeoPixelActuatorState::GREEN:
  case LedNeoPixelActuatorState::BLUE:
    // static colors — nothing to update
    return;

  case LedNeoPixelActuatorState::BLINKRED:
  case LedNeoPixelActuatorState::BLINKGREEN:
  case LedNeoPixelActuatorState::BLINKBLUE:
    if (now - lastMillis >= (unsigned long)blinkInterval)
    {
      lastMillis = now;
      blinkOn = !blinkOn;
      if (blinkOn)
      {
        setAllPixels(activeColor);
      }
      else
      {
        setAllPixels(strip.Color(0, 0, 0));
        --blinkCyclesRemaining;
      }
      if (blinkCyclesRemaining <= 0)
      {
        // finish and set to OFF
        activeState = LedNeoPixelActuatorState::OFF;
        setAllPixels(strip.Color(0, 0, 0));
      }
    }
    break;

  case LedNeoPixelActuatorState::BREATHERED:
  case LedNeoPixelActuatorState::BREATHGREEN:
  case LedNeoPixelActuatorState::BREATHBLUE:
    if (now - lastMillis >= (unsigned long)breathStepDelay)
    {
      lastMillis = now;
      // simple breathing by adjusting brightness
      breathBrightness += breathDir * breathStepSize;
      if (breathBrightness >= 255)
      {
        breathBrightness = 255;
        breathDir = -1;
      }
      else if (breathBrightness <= 0)
      {
        breathBrightness = 0;
        breathDir = 1;
        // completed a full in/out cycle
        if (--breathCyclesRemaining <= 0)
        {
          activeState = LedNeoPixelActuatorState::OFF;
          setAllPixels(strip.Color(0, 0, 0));
          return;
        }
      }
      strip.setBrightness(breathBrightness);
      setAllPixels(activeColor);
      // restore brightness to a reasonable value when finished (handled on OFF)
    }
    break;
  }
}