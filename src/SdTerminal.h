#pragma once
#include "Arduino.h"
#include "SdFat.h"
#include "FireTimer.h"




const char H_RULE[] PROGMEM = "--------------------------------------------------";
const uint8_t MAX_INPUT_LEN = 255;
const uint8_t MAX_NAME_LEN = 40;
const uint8_t MAX_PATH_LEN = 255;




class Terminal
{
public:
	void begin(SdFs& sd, Stream& stream=Serial, const uint16_t& timeout=100);
	void handleCmds();




private:
	Stream* _serial;
	SdFs* _sd;
	FsFile root;

	FireTimer msTimer;

	char* pwd;
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
	void handle_CREATE(char input[]);
	void handle_AP(char input[]);
	void handle_PWD();
	void handle_CD(char input[]);

	void readInput(char input[], const uint8_t& inputSize);
	bool startsWith(const char scan[], const char target[]);
	char* findOccur(char input[], char target[], uint16_t ithOccur);
	char* findSubStr(char input[], uint8_t place=1, const char* delim=" ");
	char* findBasename(char path[]);
	char* findDirname(char path[]);
	char* join(char path[], char add[]);
	void copyFile(char fullSrcPath[], char fullDestPath[]);
	void copyDir(char fullSrcPath[], char fullDestPath[]);
	void deleteDirectory(FsFile dir, char* path);
	uint8_t numOccur(char* input, const char& target=',');
};
