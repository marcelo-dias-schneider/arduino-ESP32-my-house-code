#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

// HANDLE FUNCTIONS
#include "handleState.h"

// INPUT and SENSOR
#define btn_1 16
#define btn_2 27
#define person_sensor 14
#define gas_sensor 23
#define water_sensor 34

// OUTPUT and ACTUATOR
#define yellow_led 12
#include "led.h"

#define LCD_MAX_MESSAGES 6
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
#include "lcd.h"

#include "neo-pixel.h"
#define LED_PIN 26
#define LED_COUNT 4

#define WINDOW_SERVO 5
#define WINDOW_PERIOD_HERTZ 50
#define WINDOW_MIN_PULSE_WIDTH 1000
#define WINDOW_MAX_PULSE_WIDTH 2000
#define WINDOW_OPEN_ANGLE 180
#define WINDOW_CLOSED_ANGLE 0
#include "window.h"

#define FAN_FORWARD_PIN 18
#define FAN_REVERSE_PIN 19
#include "fan.h"

#define BUZZER_GPIO25 25
#include "buzzer.h"

#include "nfc.h"
// NFC GRANTED ACCESS
const String NFC_GRANTED_UIDS[] = {
    "EB80B732",
    "D91A009D"};

// DHT SENSOR
#define DHT11PIN 17
#include "dht.h"

// System states and variables
enum class SystemStates
{
  STARTUP,
  CLOSING,
  CLOSED,
  OPENING,
  OPEN,
  ALERT
};
struct SystemState
{
  SystemStates current;
  SystemStates previous;
  SystemStates lastHandled;
  unsigned long enteredAt;
};
extern SystemState systemState;

// Inputs structure and variable
struct InputsState
{
  bool btn1Pressed;
  bool btn2Pressed;
  bool presenceDetected;
  bool gasDetected;
  int waterLevel;
  String nfcUid;
  int temperature;
  int humidity;
};
extern InputsState inputs;

// OUTPUT and ACTUATOR STATES
// ENUMS for Outputs
enum class YellowLedActuatorState
{
  OFF,
  ON,
  BLINK
};
enum class LedNeoPixelActuatorState
{
  OFF,
  RED,
  GREEN,
  BLUE,
  BLINKRED,
  BLINKGREEN,
  BLINKBLUE,
  BREATHERED,
  BREATHGREEN,
  BREATHBLUE
};
enum class WindowActuatorState
{
  CLOSED,
  OPEN,
};
enum class FanDirectionActuatorState
{
  OFF,
  FORWARD,
  REVERSE,
};
enum class BuzzerActuatorState
{
  OFF,
  WELCOME,
  GRANTED_ACCESS,
  CLOSING,
  ALARM_GAS,
  WARNING_ERROR,
};

// Outputs structure
struct YellowLed
{
  YellowLedActuatorState action;
  YellowLedActuatorState state;
};
struct LedNeoPixel
{
  LedNeoPixelActuatorState action;
  LedNeoPixelActuatorState state;
};
struct Window
{
  WindowActuatorState action;
  WindowActuatorState state;
  unsigned long delayMs;
};
struct Fan
{
  FanDirectionActuatorState directionState;
  FanDirectionActuatorState directionAction;
  uint8_t speedState;  // 0-255
  uint8_t speedAction; // 0-255
};
struct LcdDisplay
{
  String messages[LCD_MAX_MESSAGES];
  uint8_t count;
  unsigned long delayTime;
  String currentHash;
  String lastHash;
};
struct Buzzer
{
  BuzzerActuatorState action;
  BuzzerActuatorState state;
};

struct OutputsState
{
  YellowLed yellowLed;
  LedNeoPixel ledNeoPixel;
  Window window;
  Fan fan;
  LcdDisplay lcd;
  Buzzer buzzer;
};

// Shared variables
extern OutputsState outputs;

// Previous environmental readings used by automations (to detect changes)
#define DELAY_UPDATE_INTERVAL 500UL
#define WATER_LEVEL_THRESHOLD 1500
#define NO_PERSON_CLOSE_MS 60000UL

struct PrevEnvReadings
{
  int temperature;
  int humidity;
  int waterLevel;
  unsigned long lastWeatherUpdate;
  unsigned long lastNoPersonMillis;
};
extern PrevEnvReadings prevEnvReadings;

// Shared helper function prototype
void setPins();
void setSystemState(SystemStates newState);
void setupSystemState();

// Shared state machine functions
void readInputs();
void updateSystemState();
void buildOutputsAction();
void applyOutputs();
void logOutputsState();

#endif // SYSTEM_STATE_H
