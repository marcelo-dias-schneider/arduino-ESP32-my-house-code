#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include "system_state.h"
#include <BuzzerESP32.h>

extern BuzzerESP32 buzzer;

void buzzerWelcome();
void buzzerGrantedAccess();
void buzzerClosing();
void buzzerAlarmGas();
void buzzerWarningError();
void buzzerOff();

// Non-blocking sequencer
void buzzerPlaySequence(const void* seq, int len, bool repeat = false, unsigned long repeatPauseMs = 0);
void buzzerUpdate();

#endif
