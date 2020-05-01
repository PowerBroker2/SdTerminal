#include "SdTerminal.h"


Terminal myTerminal;


void setup()
{
  Serial.begin(115200);
  while (!myTerminal.begin());
}


void loop()
{
  myTerminal.handleCmds();
}

