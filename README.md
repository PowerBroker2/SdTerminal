# SdTerminal
[![GitHub version](https://badge.fury.io/gh/PowerBroker2%2FSdTerminal.svg)](https://badge.fury.io/gh/PowerBroker2%2FSdTerminal) [![arduino-library-badge](https://www.ardu-badge.com/badge/SdTerminal.svg?)](https://www.ardu-badge.com/SdTerminal)<br/><br/>
Basic terminal interface for Arduino SD card logging/file management. Allows basic file management of an SD card with an Arduino through serial commands.

## Commands:

For more information on a specific command, type help command-name<br/>
`cp`    - Copy a file to another location<br/>
`echo`  - Turn on or off echoing user commands<br/>
`help`  - Provide info on commands. Can specify a specific command for help<br/>
`ls`    - List the contents of the card. Can specify a specific dir for listing<br/>
`mkdir` - Make a new directory at a specific location<br/>
`mv`    - Move or rename a file<br/>
`print` - Print the contents of the given file<br/>
`rm`    - Remove a file or dir

## Notes:

The returned message `?` signifies the last command was invalid

## Example Sketch

```C++
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

```
