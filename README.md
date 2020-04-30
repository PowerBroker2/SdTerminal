# SdTerminal
[![GitHub version](https://badge.fury.io/gh/PowerBroker2%2FSdTerminal.svg)](https://badge.fury.io/gh/PowerBroker2%2FSdTerminal) [![arduino-library-badge](https://www.ardu-badge.com/badge/SdTerminal.svg?)](https://www.ardu-badge.com/SdTerminal)<br/><br/>
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

  while (!myLog.begin(myLogMeta));
}




void loop()
{
  if (telemTransfer.available())
    logData();

  myLog.handleCmds();
}




void logData()
{
  uint16_t recLen;

  telemTransfer.rxObj(telemetry, sizeof(telemetry));
  recLen = sizeof(telemetry);

  telemTransfer.rxObj(controlInputs, sizeof(controlInputs), recLen);
  recLen += sizeof(controlInputs);

  char str_alt[15];
  char str_roll[15];
  char str_pitch[15];
  char str_vel[15];
  char str_lat[15];
  char str_lon[15];
  char str_sec[15];
  char str_sog[15];
  char str_cog[15];

  char buff[150];

  dtostrf(telemetry.altitude, 4, 2, str_alt);
  dtostrf(telemetry.rollAngle, 4, 6, str_roll);
  dtostrf(telemetry.pitchAngle, 4, 6, str_pitch);
  dtostrf(telemetry.velocity, 4, 6, str_vel);
  dtostrf(telemetry.latitude, 4, 7, str_lat);
  dtostrf(telemetry.longitude, 4, 7, str_lon);
  dtostrf(telemetry.UTC_second, 4, 2, str_sec);
  dtostrf(telemetry.speedOverGround, 4, 2, str_sog);
  dtostrf(telemetry.courseOverGround, 4, 2, str_cog);

  sprintf(buff,
          target,
          millis(),
          str_alt,
          str_roll,
          str_pitch,
          str_vel,
          str_lat,
          str_lon,
          telemetry.UTC_year,
          telemetry.UTC_month,
          telemetry.UTC_day,
          telemetry.UTC_hour,
          telemetry.UTC_minute,
          str_sec,
          str_sog,
          str_cog,
          controlInputs.throttle_command,
          controlInputs.pitch_command,
          controlInputs.yaw_command,
          controlInputs.roll_command);

  myLog.log(buff);
}
```
