#include <SPI.h>
#include "SdFat.h"
#include "SdTerminal.h"


#ifdef TEENSY_CORE
SdFatSdioEX sd;
#else
SdFat sd;
#endif

Terminal myTerminal;


void setup()
{
  Serial.begin(115200);
  sd.begin();
  myTerminal.begin(sd);
}


void loop()
{
  myTerminal.handleCmds();
}
