#ifndef HANDLE_ALERT_STATE_H
#define HANDLE_ALERT_STATE_H

#include <Arduino.h>
#include "system_state.h"

void handleClosing();
void handleClosed();
void handleOpening();
void handleOpen();
void handleAlert();
void handleAutomations();

#endif
