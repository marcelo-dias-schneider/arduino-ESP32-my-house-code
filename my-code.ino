// CONTROL VARIABLES (shared types & declarations)
#include "system_state.h"

void setup()
{
  setPins();
  setupSystemState();
  applyOutputs();
}

void loop()
{
  readInputs();
  updateSystemState();
  buildOutputsAction();
  applyOutputs();
}