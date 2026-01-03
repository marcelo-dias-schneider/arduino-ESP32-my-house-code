#ifndef FAN_H
#define FAN_H

#include <Arduino.h>
#include "system_state.h"

enum class FanDirectionActuatorState;

void setFan(FanDirectionActuatorState direction, uint8_t speed);

#endif // FAN_H