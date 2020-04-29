# SdTerminal
Basic terminal interface for Arduino SD card logging/file management. Allows basic file management of an SD card with an Arduino through serial commands.

## Commands:

`ls` -- List all files on SD card <br/>
`rm all` -- Delete all files on SD card except for the file currently being used for datalogging <br/>
`rm <filename>` -- Delete the specified file on the SD card except for the file currently being used for datalogging <br/>
`<filename>` -- Display the entire contents of the file

## Notes:

The returned message `?` signifies the last command was invalid

## Example Sketch

```C++
#include "SerialTransfer.h"
#include "Shared_Tools.h"
#include "SdTerminal.h"




const char nameTemplate[] = "flight_%d.txt";
const char headerRow[] = "epoch_ms,alt_cm,roll_deg,pitch_deg,velocity_m/s,lat_dd,lon_dd,year,month,day,hour,min,sec,sog,cog,throttle_command,pitch_command,yaw_command,roll_command";
char target[] = "%d,%s,%s,%s,%s,%s,%s,%d,%d,%d,%d,%d,%s,%s,%s,%d,%d,%d,%d";




telemetry_struct telemetry;
controlInputs_struct controlInputs;

SerialTransfer telemTransfer;

Log_Meta myLogMeta;
Logger myLog;




void setup()
{
  // turn on power led
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  
  Serial.begin(115200);
  Serial1.begin(2000000);

  telemTransfer.begin(Serial1);

  strcpy(myLogMeta.nameTemplate, nameTemplate);
  strcpy(myLogMeta.headerRow, headerRow);
  
  while(!myLog.begin(myLogMeta));
}




void loop()
{
  if(telemTransfer.available())
    logData();

  myLog.handleCmds();
}




void logData()
{
  uint16_t recLen;
  
  telemTransfer.rxObj(telemetry, sizeof(telemetry));
  recLen = sizeof(telemetry);

  telemTransfer.txObj(controlInputs, sizeof(controlInputs), recLen);
  recLen += sizeof(controlInputs);

  char str_temp[6];
  char buff[100];

  sprintf(buff, target, millis(),
    dtostrf(telemetry.altitude, 4, 2, str_temp),
    dtostrf(telemetry.rollAngle, 4, 2, str_temp),
    dtostrf(telemetry.pitchAngle, 4, 2, str_temp),
    dtostrf(telemetry.velocity, 4, 2, str_temp),
    dtostrf(telemetry.latitude, 4, 2, str_temp),
    dtostrf(telemetry.longitude, 4, 2, str_temp),
    telemetry.UTC_year,
    telemetry.UTC_month,
    telemetry.UTC_day,
    telemetry.UTC_hour,
    telemetry.UTC_minute,
    dtostrf(telemetry.UTC_second, 4, 2, str_temp),
    dtostrf(telemetry.speedOverGround, 4, 2, str_temp),
    dtostrf(telemetry.courseOverGround, 4, 2, str_temp),
    controlInputs.throttle_command,
    controlInputs.pitch_command,
    controlInputs.yaw_command,
    controlInputs.roll_command);
  
  myLog.log(buff);
}
```
