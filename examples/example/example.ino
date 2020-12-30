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
