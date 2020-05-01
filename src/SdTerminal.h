#pragma once
#include "Arduino.h"
#include "SdFat.h"
#include "FireTimer.h"




const uint8_t MAX_FILENAME_LEN = 40;
const uint8_t MAX_HEADER_LEN = 150;
const uint8_t MAX_INPUT_LEN = 255;

const char H_RULE[] PROGMEM = "--------------------------------------------------";




struct Log_Meta {
	char nameTemplate[MAX_FILENAME_LEN];
	char headerRow[MAX_HEADER_LEN];
};




class Logger
{
public:
	bool initialized;
	uint8_t numColumns;




	bool begin(Log_Meta& logMeta, Stream& stream=Serial, const uint16_t& timeout=100);
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
	bool echo = true;




	void handle_HELP(char input[]);
	void handle_LS(char input[]);
	void handle_RM(char input[]);
	void handle_MV(char input[]);
	void handle_MKDIR(char input[]);
	void handle_CP(char input[]);
	void handle_ECHO(char input[]);
	void handle_PRINT(char input[]);

	void readInput(char input[], const uint8_t& inputSize);
	bool startsWith(const char scan[], const char target[]);
	char* findArg(char input[], uint8_t argNum=1);
	void printDirectory(File dir, const int& numTabs);
	void deleteDirectory(File dir, const char* path="");
	uint8_t numOccur(char* input, const uint16_t& len, const char& target=',');
};
