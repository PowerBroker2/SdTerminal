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
	myFile.open(filename, FILE_WRITE);

	for (uint16_t i=0; i<dataLen; i++)
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




void Logger::log(const char* data)
{
	myFile.open(filename, FILE_WRITE);
	myFile.println(data);
	myFile.close();
}





void Logger::handleCmds()
{
	if (_serial->available())
	{
		char input[50] = { '\0' };

		readInput(input, sizeof(input));

		_serial->println(input);

		if (!strcmp(input, "ls"))
		{
			_serial->println(F("--------------------------------------------------"));

			root = sd.open("/");
			printDirectory(root, 0);

			_serial->println(F("--------------------------------------------------"));
			_serial->println();
		}
		else if (strstr(input, "rm "))
		{
			char* p = strstr(input, "rm ") + 3;

			char fileName[40] = { '\0' };

			for (byte i = 0; i < sizeof(fileName); i++)
			{
				fileName[i] = *p;

				if (*p == '\0')
					break;

				p++;
			}

			if (sd.exists(fileName))
			{
				_serial->print("Deleting File: ");
				_serial->println(fileName);

				myFile.open(fileName, O_WRITE);
				myFile.remove();
			}
			else if (!strcmp(fileName, "all"))
			{
				char fileName_[20];

				while (true)
				{
					File entry = root.openNextFile();

					if (!entry)
						break;

					entry.getName(fileName_, sizeof(fileName_));

					if (!entry.isDirectory())
					{
						_serial->print("Deleting: ");
						_serial->println(fileName_);

						myFile.open(fileName_, O_WRITE);
						myFile.remove();
					}

					entry.close();
				}
			}
			else
			{
				_serial->print(fileName);
				_serial->println(" not found");
			}
		}
		else if (sd.exists(input))
		{
			_serial->println(F("--------------------------------------------------"));
			_serial->print(input); _serial->println(" found:");

			myFile.open(input, FILE_READ);

			int data;
			while ((data = myFile.read()) >= 0)
				_serial->write(data);

			myFile.close();

			_serial->println(F("--------------------------------------------------"));
			_serial->println();
		}
		else
			_serial->println('?');
	}
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




uint8_t Logger::numOccur(char* input, const uint16_t& len, const char& target)
{
	char* p = input;
	uint8_t count = 0;

	while (*p != '\0')
		if (*p++ == target)
			count++;

	return count;
}