#pragma once
#include "Arduino.h"
#include "SdFat.h"
#include "FireTimer.h"




const uint8_t MAX_FILENAME_LEN = 40;
const uint8_t MAX_HEADER_LEN = 150;




struct Log_Meta {
	char nameTemplate[MAX_FILENAME_LEN];
	char headerRow[MAX_HEADER_LEN];
};




class Logger
{
public:
	bool initialized;
	uint8_t numColumns;




	bool begin(Log_Meta& logMeta, Stream& stream=Serial, const uint16_t& timeout=1000);
	bool init();
	void log(const float* data, const uint16_t& dataLen);
	void log(const char* data);
	void handleCmds();




private:
	Stream* _serial;
	FireTimer msTimer;
	Log_Meta* metaP;
	SdFatSdioEX sd;
	SdFile myFile;
	File root;

	char filename[MAX_FILENAME_LEN];

	uint16_t _timeout;




	void readInput(char input[], const uint8_t& inputSize);
	void printDirectory(File dir, const int& numTabs);
	uint8_t numOccur(char* input, const uint16_t& len, const char& target=',');
};
