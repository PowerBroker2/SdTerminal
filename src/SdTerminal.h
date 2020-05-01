#pragma once
#include "Arduino.h"
#include "SdFat.h"
#include "FireTimer.h"




const char H_RULE[] PROGMEM = "--------------------------------------------------";
const uint8_t MAX_INPUT_LEN = 255;




class Terminal
{
public:
	bool initialized;
	uint8_t numColumns;




	bool begin(Stream& stream=Serial, uint8_t csPin=SS, SPISettings spiSettings=SPI_FULL_SPEED, const uint16_t& timeout=100);
	bool init(uint8_t csPin=SS, SPISettings spiSettings=SPI_FULL_SPEED);
	void handleCmds();




private:
	Stream* _serial;
	FireTimer msTimer;
	SdFatSdioEX sd;
	SdFile myFile;
	File root;

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
	char* findSubStr(char input[], uint8_t place=1, const char* delim=" ");
	char* findBasename(char path[]);
	char* join(char path[], char add[]);
	void copyFile(char fullSrcPath[], char fullDestPath[]);
	void copyDir(char fullSrcPath[], char fullDestPath[]);
	void deleteDirectory(File dir, const char* path="");
	uint8_t numOccur(char* input, const char& target=',');
};
