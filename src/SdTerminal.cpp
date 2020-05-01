#include "SdTerminal.h"




bool Terminal::begin(Stream& stream, uint8_t csPin, SPISettings spiSettings, const uint16_t& timeout)
{
	_serial = &stream;
	_timeout = timeout;

	msTimer.begin(_timeout);

	return init();
}




bool Terminal::init(uint8_t csPin, SPISettings spiSettings)
{
	if (!sd.begin())
	{
		initialized = false;
		_serial->println("SD Card Init Failed");
	}
	else
		initialized = true;

	return initialized;
}





void Terminal::handleCmds()
{
	if (_serial->available() && initialized)
	{
		char input[MAX_INPUT_LEN] = { '\0' };

		readInput(input, sizeof(input));

		if (echo)
			_serial->println(input);

		_serial->println(H_RULE);

		if (startsWith(input, "help"))
			handle_HELP(input);
		else if (startsWith(input, "ls"))
			handle_LS(input);
		else if (startsWith(input, "rm "))
			handle_RM(input);
		else if (startsWith(input, "mv "))
			handle_MV(input);
		else if (startsWith(input, "mkdir "))
			handle_MKDIR(input);
		else if (startsWith(input, "cp "))
			handle_CP(input);
		else if (startsWith(input, "echo "))
			handle_ECHO(input);
		else if (startsWith(input, "print "))
			handle_PRINT(input);
		else
			_serial->println('?');

		_serial->println(H_RULE);
		_serial->println();
	}
}




void Terminal::handle_HELP(char input[])
{
	char* arg1 = findSubStr(input);

	if (arg1 != NULL)
	{
		if (!strcmp(arg1, "print"))
			_serial->println(F("print file - Print the contents of file."));
		else if (!strcmp(arg1, "cp"))
			_serial->println(F("cp src dest - Copy file src to dest."));
		else if (!strcmp(arg1, "echo"))
			_serial->println(F("echo cmd - Turn on or off echoing user commands. Arg cmd can be either on or off."));
		else if (!strcmp(arg1, "help"))
			_serial->println(F("help (optional)cmd - Provide info on commands. Can specify a specific command for help."));
		else if (!strcmp(arg1, "ls"))
			_serial->println(F("ls (optional)dir - List the contents of the card. Can specify dir for listing."));
		else if (!strcmp(arg1, "mkdir"))
			_serial->println(F("mkdir dir - Make a new directory at location dir."));
		else if (!strcmp(arg1, "mv"))
			_serial->println(F("mv src dest - Move file src to location dest."));
		else if (!strcmp(arg1, "rm"))
			_serial->println(F("rm path - Remove a file or dir as specified by path"));
		else
		{
			_serial->print(F("Invalid command: "));
			_serial->println(arg1);
		}
	}
	else
	{
		_serial->println(F("For more information on a specific command, type help command-name"));
		_serial->println(F("cp    - Copy a file to another location."));
		_serial->println(F("echo  - Turn on or off echoing user commands."));
		_serial->println(F("help  - Provide info on commands. Can specify a specific command for help."));
		_serial->println(F("ls    - List the contents of the card. Can specify a specific dir for listing."));
		_serial->println(F("mkdir - Make a new directory at a specific location."));
		_serial->println(F("mv    - Move or rename a file."));
		_serial->println(F("print - Print the contents of the given file."));
		_serial->println(F("rm    - Remove a file or dir"));
	}

	// garbage collection
	free(arg1);
}




void Terminal::handle_LS(char input[])
{
	char* arg1 = findSubStr(input);

	if (arg1 != NULL)
	{
		if (sd.exists(arg1))
		{
			File dir = sd.open(arg1);

			if (dir.isDirectory())
				dir.ls(_serial, LS_R | LS_SIZE);
			else
			{
				_serial->print(arg1);
				_serial->println(F(" is not a directory"));
			}
		}
		else
		{
			_serial->print(arg1);
			_serial->println(F(" does not exist"));
		}
	}
	else
	{
		root = sd.open("/");
		root.ls(_serial, LS_R | LS_SIZE);
	}

	// garbage collection
	free(arg1);
}




void Terminal::handle_RM(char input[])
{
	char* nameArg = findSubStr(input);

	if (sd.exists(nameArg))
	{
		File file = sd.open(nameArg, O_READ);

		if (file.isDirectory())
		{
			deleteDirectory(file, nameArg);

			if (!sd.exists(nameArg))
			{
				_serial->print(F("Deleted: "));
				_serial->println(nameArg);
			}
			else
			{
				_serial->print(F("Failed to delete: "));
				_serial->println(nameArg);
			}
		}

		myFile.open(nameArg, O_WRITE);
		myFile.remove();

		if (!sd.exists(nameArg))
		{
			_serial->print(F("Deleted: "));
			_serial->println(nameArg);
		}
		else
		{
			_serial->print(F("Failed to delete: "));
			_serial->println(nameArg);
		}
	}
	else if (!strcmp(nameArg, "all"))
		deleteDirectory(root);
	else
	{
		_serial->print(nameArg);
		_serial->println(F(" not found"));
	}

	// garbage collection
	free(nameArg);
}




void Terminal::handle_MV(char input[])
{
	char* arg1 = findSubStr(input, 1);
	char* arg2 = findSubStr(input, 2);

	if (sd.exists(arg1) && !sd.exists(arg2))
	{
		File file = sd.open(arg1);

		file.rename(arg2);

		if (sd.exists(arg2))
		{
			_serial->print(arg1);
			_serial->print(F(" moved to "));
			_serial->println(arg2);
		}
		else
		{
			_serial->print(F("Failed to move "));
			_serial->print(arg1);
			_serial->print(F(" to "));
			_serial->println(arg2);
		}
	}
	else
	{
		if (!sd.exists(arg1))
		{
			_serial->println(arg2);
			_serial->println(F(" does not exists"));
		}
		
		if (sd.exists(arg2))
		{
			_serial->println(arg2);
			_serial->println(F(" already exists"));
		}
	}

	// garbage collection
	free(arg1);
	free(arg2);
}




void Terminal::handle_MKDIR(char input[])
{
	char* arg1 = findSubStr(input);

	if (!sd.exists(arg1))
	{
		sd.mkdir(arg1);

		if (sd.exists(arg1))
		{
			_serial->println(arg1);
			_serial->println(F(" created"));
		}
		else
		{
			_serial->print(F("Failed to create: "));
			_serial->println(arg1);
		}
	}
	else
	{
		_serial->println(arg1);
		_serial->println(F(" already exists"));
	}

	// garbage collection
	free(arg1);
}




void Terminal::handle_CP(char input[])
{
	char* fullSrcPath = findSubStr(input, 1);
	char* destPath = findSubStr(input, 2);
	char* basename = findBasename(fullSrcPath);
	char* fullDestPath = join(destPath, basename);

	if (sd.exists(fullSrcPath))
	{
		File srcFile = sd.open(fullSrcPath);

		// make sure we only copy to a dir and not a file
		if (!strstr(destPath, "."))
		{
			if (!sd.exists(destPath))
				sd.mkdir(destPath);

			if (srcFile.isDirectory())
			{
				srcFile.close();
				copyDir(fullSrcPath, fullDestPath);
			}
			else
			{
				srcFile.close();
				copyFile(fullSrcPath, fullDestPath);
			}

			if (sd.exists(destPath))
			{
				_serial->print(fullSrcPath);
				_serial->print(F(" copied to "));
				_serial->println(destPath);
			}
			else
			{
				_serial->print(F("Failed to copy "));
				_serial->print(fullSrcPath);
				_serial->print(F(" to "));
				_serial->println(destPath);
			}
		}
		else
		{
			_serial->print(destPath);
			_serial->println(F(" is not a directory"));
		}
	}
	else
	{
		if (!sd.exists(fullSrcPath))
		{
			_serial->print(fullSrcPath);
			_serial->println(F(" does not exists"));
		}
	}

	// garbage collection
	free(fullSrcPath);
	free(destPath);
	free(basename);
	free(fullDestPath);
}




void Terminal::handle_ECHO(char input[])
{
	char* arg1 = findSubStr(input);

	if (strstr(arg1, "on"))
	{
		echo = true;
		_serial->println(F("Echo turned on"));
	}
	else if (strstr(arg1, "off"))
	{
		echo = false;
		_serial->println(F("Echo turned off"));
	}
	else
		_serial->println('?');

	// garbage collection
	free(arg1);
}




void Terminal::handle_PRINT(char input[])
{
	char* arg1 = findSubStr(input);

	if (sd.exists(arg1))
	{
		_serial->print(arg1); _serial->println(F(" found:"));

		myFile.open(arg1, FILE_READ);

		int data;
		while ((data = myFile.read()) >= 0)
			_serial->write(data);

		myFile.close();
	}
	else
	{
		_serial->println(arg1);
		_serial->println(F(" doesn't exist"));
	}

	// garbage collection
	free(arg1);
}




void Terminal::readInput(char input[], const uint8_t& inputSize)
{
	char c;
	uint8_t i = 0;
	
	msTimer.start();

	while (true)
	{
		if (_serial->available())
		{
			c = _serial->read();

			if (msTimer.fire())
				break;
			else if (c == '\n')
				break;
			else if (i >= inputSize)
				break;

			input[i] = c;
			i++;
		}

		if (msTimer.fire())
			break;
	}
}




bool Terminal::startsWith(const char scan[], const char target[])
{
	uint16_t scanLen = strlen(scan);
	uint16_t targetLen = strlen(target);

	if (scanLen >= targetLen)
	{
		for (uint16_t i=0; i<targetLen; i++)
			if (scan[i] != target[i])
				return false;
		return true;
	}
	
	return false;
}




char* Terminal::findSubStr(char input[], uint8_t place, const char* delim)
{
	char* argP = input;
	char* divP = NULL;
	char* returnStr;

	for (uint8_t i=0; i< place; i++)
	{
		divP = strstr(argP, delim);

		if (!divP)
			return NULL;

		argP = divP + 1;
	}

	divP = strstr(argP, delim);

	if (divP)
	{
		uint32_t argLen = divP - argP + 1;

		returnStr = (char*)malloc(argLen);
		memcpy(returnStr, argP, argLen);
		returnStr[argLen - 1] = '\0';
	}
	else
	{
		returnStr = (char *)malloc(strlen(argP) + 1);
		memcpy(returnStr, argP, strlen(argP) + 1);
	}

	return returnStr;
}




char* Terminal::findBasename(char path[])
{
	uint8_t numLevels = numOccur(path, '/');

	if (path[strlen(path) - 1] == '/')
		numLevels--;

	if (path[0] == '/')
		numLevels++;

	return findSubStr(path, numLevels, "/");
}




char* Terminal::join(char path[], char add[])
{
	uint16_t pathLen = strlen(path);
	uint16_t addLen = strlen(add);
	uint16_t joinedPathLen = pathLen + addLen + 1; // add one for joining slash

	char* joinedPath = (char*)malloc(joinedPathLen);

	if (path[pathLen-1] == '/')
		sprintf(joinedPath, "%s%s", path, add);
	else
		sprintf(joinedPath, "%s/%s", path, add);

	return joinedPath;
}




void Terminal::copyFile(char fullSrcPath[], char fullDestPath[])
{
	File srcFile = sd.open(fullSrcPath, FILE_READ);
	File destFile = sd.open(fullDestPath, FILE_WRITE);

	if (!srcFile)
	{
		_serial->print("Error opening source file at ");
		_serial->println(fullSrcPath);
	}

	if (!destFile)
	{
		_serial->println("Error opening destination file at ");
		_serial->println(fullDestPath);
	}

	size_t n;
	uint8_t buf[64];
	while ((n = srcFile.read(buf, sizeof(buf))) > 0)
		destFile.write(buf, n);

	srcFile.close();
	destFile.close();
}




void Terminal::copyDir(char fullSrcPath[], char fullDestPath[])
{
	//TODO
}




void Terminal::deleteDirectory(File dir, const char* path)
{
	char fullPath[100];

	if (dir.isDirectory())
	{
		char fileName[40];

		while (true)
		{
			File file = dir.openNextFile();

			if (!file)
				break;

			file.getName(fileName, sizeof(fileName));

			for (byte i=0; i<sizeof(fullPath); i++)
				fullPath[i] = '\0';

			memcpy(fullPath, path, strlen(path));
			strcat(fullPath, "/");
			strcat(fullPath, fileName);

			// don't delete the volume info files
			if (!strstr(fullPath, "System Volume Infor"))
			{
				if (file.isDirectory())
					deleteDirectory(file, fullPath);
				else
				{
					myFile.open(fullPath, O_WRITE);
					myFile.remove();

					if (!sd.exists(fullPath))
					{
						_serial->print(F("Deleted: "));
						_serial->println(fullPath);
					}
					else
					{
						_serial->print(F("Failed to delete: "));
						_serial->println(fullPath);
					}
				}
			}

			file.close();
		}
		sd.rmdir(path);
	}
}




uint8_t Terminal::numOccur(char* input, const char& target)
{
	char* p = input;
	uint8_t count = 0;

	while (*p != '\0')
		if (*p++ == target)
			count++;

	return count;
}
