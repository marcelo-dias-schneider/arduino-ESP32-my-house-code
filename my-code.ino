// CONTROL VARIABLES (shared types & declarations)
#include "system_state.h"

void setup()
{
  setPins();
  setupSystemState();
  applyOutputs();
  //////////////
  // myservo.write(WINDOW_CLOSED_ANGLE);
  //////////////
}

void loop()
{
  readInputs();
  updateSystemState();
  buildOutputsAction();
  applyOutputs();
}