#include "SdTerminal.h"




bool Logger::begin(Log_Meta& logMeta, Stream& stream, const uint16_t& timeout)
{
	metaP = &logMeta;
	_serial = &stream;
	_timeout = timeout;

	msTimer.begin(_timeout);

	numColumns = numOccur(metaP->headerRow, sizeof(metaP->headerRow)) + 1;

	return init();
}




bool Logger::init()
{
	unsigned int count = 1;

	if (sd.begin())
	{
		sprintf(filename, metaP->nameTemplate, count);

		while (sd.exists(filename))
		{
			count++;
			sprintf(filename, metaP->nameTemplate, count);
		}

		myFile.open(filename, FILE_WRITE);
		myFile.println(metaP->headerRow);
		myFile.close();

		initialized = true;
	}
	else
	{
		initialized = false;
		_serial->println("SD Card Init Failed");
	}

	return initialized;
}




void Logger::log(const float* data, const uint16_t& dataLen)
{
	if (initialized)
	{
		myFile.open(filename, FILE_WRITE);

		for (uint16_t i = 0; i < dataLen; i++)
		{
			myFile.print(data[i]);

			if ((i != (dataLen - 1)) && (i != (numColumns - 1)))
				myFile.print(',');
			else if (i == (numColumns - 1))
				break;
		}

		myFile.println();
		myFile.close();
	}
}




void Logger::log(const char* data)
{
	if (initialized)
	{
		myFile.open(filename, FILE_WRITE);
		myFile.println(data);
		myFile.close();
	}
}





void Logger::handleCmds()
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
		else if (sd.exists(input))
			handle_PRINT(input);
		else
			_serial->println('?');

		_serial->println(H_RULE);
		_serial->println();
	}
}




void Logger::handle_HELP(char input[])
{
	char* arg1 = findArg(input);

	if (arg1 != NULL)
	{
		if (strstr(arg1, "<filepath>"))
			_serial->println(F("<filepath> - Print the contents of the given file."));
		else if (strstr(arg1, "cp"))
			_serial->println(F("cp src dest - Copy file src to dest."));
		else if (strstr(arg1, "echo"))
			_serial->println(F("echo cmd - Turn on or off echoing user commands. Arg cmd can be either on or off."));
		else if (strstr(arg1, "help"))
			_serial->println(F("help (optional)cmd - Provide info on commands. Can specify a specific command for help."));
		else if (strstr(arg1, "ls"))
			_serial->println(F("ls (optional)dir - List the contents of the card. Can specify dir for listing."));
		else if (strstr(arg1, "mkdir"))
			_serial->println(F("mkdir dir - Make a new directory at location dir."));
		else if (strstr(arg1, "mv"))
			_serial->println(F("mv src dest - Move file src to location dest."));
		else if (strstr(arg1, "rm"))
			_serial->println(F("rm path - Remove a file or dir as specified by path"));
	}
	else
	{
		_serial->println(F("For more information on a specific command, type help command-name"));
		_serial->println(F("<filepath> - Print the contents of the given file."));
		_serial->println(F("cp         - Copy a file to another location."));
		_serial->println(F("echo       - Turn on or off echoing user commands."));
		_serial->println(F("help       - Provide info on commands. Can specify a specific command for help."));
		_serial->println(F("ls         - List the contents of the card. Can specify a specific dir for listing."));
		_serial->println(F("mkdir      - Make a new directory at a specific location."));
		_serial->println(F("mv         - Move or rename a file."));
		_serial->println(F("rm         - Remove a file or dir"));
	}
}




void Logger::handle_LS(char input[])
{
	char* arg1 = findArg(input);

	if (arg1 != NULL)
	{
		File DIR = sd.open(arg1);
		printDirectory(DIR, 0);
	}
	else
	{
		root = sd.open("/");
		printDirectory(root, 0);
	}
}




void Logger::handle_RM(char input[])
{
	char* nameArg = findArg(input);

	if (sd.exists(nameArg))
	{
		File entry = sd.open(nameArg, O_READ);

		if (entry.isDirectory())
			deleteDirectory(entry, nameArg);
		else if (strcmp(nameArg, filename))
		{
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
		else
			_serial->println(F("Can't delete current log"));
	}
	else if (!strcmp(nameArg, "all"))
		deleteDirectory(root);
	else
	{
		_serial->print(nameArg);
		_serial->println(F(" not found"));
	}
}




void Logger::handle_MV(char input[])
{
	//TODO
}




void Logger::handle_MKDIR(char input[])
{
	//TODO
}




void Logger::handle_CP(char input[])
{
	//TODO
}




void Logger::handle_ECHO(char input[])
{
	char* arg1 = findArg(input);

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
}




void Logger::handle_PRINT(char input[])
{
	_serial->print(input); _serial->println(F(" found:"));

	myFile.open(input, FILE_READ);

	int data;
	while ((data = myFile.read()) >= 0)
		_serial->write(data);

	myFile.close();
}




void Logger::readInput(char input[], const uint8_t& inputSize)
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




bool Logger::startsWith(const char scan[], const char target[])
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




char* Logger::findArg(char input[], uint8_t argNum)
{
	char* p = input;

	for (uint8_t i=0; i<argNum; i++)
	{
		char* tempP = strstr(p, " ");

		if (!tempP)
			return NULL;

		p = tempP + 1;
	}

	return p;
}




void Logger::printDirectory(File dir, const int& numTabs)
{
	char fileName[20];

	while (true)
	{
		File entry = dir.openNextFile();

		if (!entry)
			break;

		for (uint8_t i=0; i <numTabs; i++)
			_serial->print('\t');

		entry.getName(fileName, sizeof(fileName));
		_serial->print(fileName);

		if (entry.isDirectory())
		{
			_serial->println("/");
			printDirectory(entry, numTabs + 1);
		}
		else
		{
			_serial->print("\t\t");
			_serial->println(entry.size(), DEC);
		}

		entry.close();
	}
}




void Logger::deleteDirectory(File dir, const char* path)
{
	char fullPath[100];

	if (dir.isDirectory())
	{
		char fileName[40];

		while (true)
		{
			File entry = dir.openNextFile();

			if (!entry)
				break;

			entry.getName(fileName, sizeof(fileName));

			for (byte i=0; i<sizeof(fullPath); i++)
				fullPath[i] = '\0';

			memcpy(fullPath, path, strlen(path));
			strcat(fullPath, "/");
			strcat(fullPath, fileName);

			// don't delete the volume info files
			if (!strstr(fullPath, "System Volume Infor"))
			{
				if (entry.isDirectory())
					deleteDirectory(entry, fullPath);
				else if (strcmp(fileName, filename))
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

			entry.close();
		}
		sd.rmdir(path);
	}
}




uint8_t Logger::numOccur(char* input, const uint16_t& len, const char& target)
{
	char* p = input;
	uint8_t count = 0;

	while (*p != '\0')
		if (*p++ == target)
			count++;

	return count;
}