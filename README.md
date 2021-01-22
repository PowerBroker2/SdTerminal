# SdTerminal
[![GitHub version](https://badge.fury.io/gh/PowerBroker2%2FSdTerminal.svg)](https://badge.fury.io/gh/PowerBroker2%2FSdTerminal) [![arduino-library-badge](https://www.ardu-badge.com/badge/SdTerminal.svg?)](https://www.ardu-badge.com/SdTerminal)<br/><br/>
Basic terminal interface for Arduino SD card logging/file management. Allows basic file management of an SD card with an Arduino through serial commands.

## Commands:

For more information on a specific command, type help command-name<br/>
`>`     - Create file if it doesn't already exist<br/>
`ap`    - Append a line to an existing file<br/>
`cd`    - Change present working directory<br/>
`cp`    - Copy a file to another location<br/>
`echo`  - Turn on or off echoing user commands<br/>
`help`  - Provide info on commands. Can specify a specific command for help<br/>
`ls`    - List the contents of the card. Can specify a specific dir for listing<br/>
`mkdir` - Make a new directory at a specific location<br/>
`mv`    - Move or rename a file<br/>
`print` - Print the contents of the given file<br/>
`pwd`   - Print the present working directory<br/>
`rm`    - Remove a file or dir

## Notes:

The returned message `?` signifies the last command was invalid

## Example Sketch

```C++

#include "SdFat.h"
#include "SdTerminal.h"


// Uncomment 'USE_EXTERNAL_SD' define to use an external SD card adapter or leave
// it commented to use the built in sd card.
//#define USE_EXTERNAL_SD 

#ifdef USE_EXTERNAL_SD
const uint8_t SD_CS_PIN = SS;
#define SPI_CLOCK SD_SCK_MHZ(10)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#else // Use built in SD card

#ifdef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif // SDCARD_SS_PIN
#define SPI_CLOCK SD_SCK_MHZ(50)
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#endif // USE_EXTERNAL_SD


SdFs sd;
Terminal myTerminal;


void setup()
{
  Serial.begin(115200);
  
  while (!sd.begin(SD_CONFIG))
  {
    Serial.println("SD initialization failed\n");
    delay(1000);
  }
  Serial.println("SD initialization succeeded\n");
  
  myTerminal.begin(sd);
}


void loop()
{
  myTerminal.handleCmds();
}

```
