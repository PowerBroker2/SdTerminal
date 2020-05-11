#include "SdTerminal.h"




void Terminal::begin(FatFileSystem& sd, Stream& stream, const uint16_t& timeout)
{
	_serial = &stream;
	_sd = &sd;
	_timeout = timeout;

	msTimer.begin(_timeout);

	pwd = (char*)malloc(MAX_PATH_LEN);
	pwd[0] = '/';
	pwd[1] = '\0';
}





void Terminal::handleCmds()
{
	if (_serial->available())
	{
		char input[MAX_INPUT_LEN] = { '\0' };

		readInput(input, sizeof(input));

		if (echo)
			_serial->println(input);

		_serial->println(H_RULE);

		if (startsWith(input, "help"))
			handle_HELP(input);
		else if (startsWith(input, "ls ") || !strcmp(input, "ls"))
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
		else if (startsWith(input, "> "))
			handle_CREATE(input);
		else if (startsWith(input, "ap "))
			handle_AP(input);
		else if (!strcmp(input, "pwd"))
			handle_PWD();
		else if (startsWith(input, "cd "))
			handle_CD(input);
		else
			_serial->println('?');

		_serial->println(H_RULE);
		_serial->println();
	}
}




void Terminal::handle_HELP(char input[])
{
	char* cmd = findSubStr(input);

	if (cmd != NULL)
	{
		if (!strcmp(cmd, ">"))
			_serial->println(F("> file - Create file if it doesn't already exist."));
		else if (!strcmp(cmd, "ap"))
			_serial->println(F("ap file line - Append line to the contents of file."));
		else if (!strcmp(cmd, "print"))
			_serial->println(F("print file - Print the contents of file."));
		else if (!strcmp(cmd, "pwd"))
			_serial->println(F("pwd - Print the present working directory."));
		else if (!strcmp(cmd, "cd"))
			_serial->println(F("cd dest - Change present working directory to dest."));
		else if (!strcmp(cmd, "cp"))
			_serial->println(F("cp src dest - Copy file src to dest."));
		else if (!strcmp(cmd, "echo"))
			_serial->println(F("echo cmd - Turn on or off echoing user commands. Arg cmd can be either on or off."));
		else if (!strcmp(cmd, "help"))
			_serial->println(F("help (optional)cmd - Provide info on commands. Can specify a specific command for help."));
		else if (!strcmp(cmd, "ls"))
			_serial->println(F("ls (optional)dir - List the contents of the card. Can specify dir for listing."));
		else if (!strcmp(cmd, "mkdir"))
			_serial->println(F("mkdir dir - Make a new directory at location dir."));
		else if (!strcmp(cmd, "mv"))
			_serial->println(F("mv src dest - Move file src to location dest."));
		else if (!strcmp(cmd, "rm"))
			_serial->println(F("rm path - Remove a file or dir as specified by path"));
		else
		{
			_serial->print(F("Invalid command: "));
			_serial->println(cmd);
		}
	}
	else
	{
		_serial->println(F("For more information on a specific command, type help command-name"));
		_serial->println(F(">     - Create file if it doesn't already exist."));
		_serial->println(F("ap    - Append a line to an existing file."));
		_serial->println(F("cd    - Change present working directory."));
		_serial->println(F("cp    - Copy a file to another location."));
		_serial->println(F("echo  - Turn on or off echoing user commands."));
		_serial->println(F("help  - Provide info on commands. Can specify a specific command for help."));
		_serial->println(F("ls    - List the contents of the card. Can specify a specific dir for listing."));
		_serial->println(F("mkdir - Make a new directory at a specific location."));
		_serial->println(F("mv    - Move or rename a file."));
		_serial->println(F("print - Print the contents of the given file."));
		_serial->println(F("pwd   - Print the present working directory."));
		_serial->println(F("rm    - Remove a file or dir"));
	}
}




void Terminal::handle_LS(char input[])
{
	char* path = findSubStr(input);

	if (path != NULL)
	{
		if (_sd->exists(path))
		{
			File dir = _sd->open(path);

			if (dir.isDirectory())
				dir.ls(_serial, LS_R | LS_SIZE);
			else
			{
				_serial->print(path);
				_serial->println(F(" is not a directory"));
			}
		}
		else
		{
			_serial->print(path);
			_serial->println(F(" does not exist"));
		}
	}
	else
	{
		root = _sd->open("/");
		root.ls(_serial, LS_R | LS_SIZE);
	}
}




void Terminal::handle_RM(char input[])
{
	char* nameArg = findSubStr(input);

	if (_sd->exists(nameArg))
	{
		File file_ = _sd->open(nameArg, O_READ);

		if (file_.isDirectory())
			deleteDirectory(file_, nameArg);
		else
			_sd->remove(nameArg);

		if (!_sd->exists(nameArg))
		{
			_serial->print(F("Deleted: "));
			_serial->println(nameArg);
		}
		else
		{
			if (!strcmp(nameArg, "/ "))
			{
				_serial->print(F("Failed to delete: "));
				_serial->println(nameArg);
			}
		}
	}
	else if (!strcmp(nameArg, "all"))
	{
		char rootStr[] = { '/', '\0' };
		deleteDirectory(root, rootStr);
	}
	else
	{
		_serial->print(nameArg);
		_serial->println(F(" not found"));
	}
}




void Terminal::handle_MV(char input[])
{
	char* srcPath = findSubStr(input, 1);
	char* destPath = findSubStr(input, 2);

	if (_sd->exists(srcPath))
	{
		File file = _sd->open(srcPath);

		if (file.isDirectory())
		{
			if (!strstr(destPath, "."))
			{
				if (!_sd->exists(destPath))
					_sd->mkdir(destPath);

				char* newName = join(destPath, findBasename(srcPath));
				file.rename(newName);

				if (_sd->exists(newName))
				{
					_serial->print(F("Moved\n\t"));
					_serial->println(srcPath);
					_serial->print(F("to\n\t"));
					_serial->println(newName);
				}
				else
				{
					_serial->print(F("Error moving\n\t"));
					_serial->println(srcPath);
					_serial->print(F("to\n\t"));
					_serial->println(newName);
				}
			}
			else
				_serial->println(F("Can not move a directory to a file"));
		}
		else
		{
			if (_sd->exists(destPath))
			{
				_serial->print(destPath);
				_serial->println(F(" already exists"));
			}
			else
			{
				file.rename(destPath);

				if (_sd->exists(destPath))
				{
					_serial->print(F("Moved\n\t"));
					_serial->println(srcPath);
					_serial->print(F("to\n\t"));
					_serial->println(destPath);
				}
				else
				{
					_serial->print(F("Error moving\n\t"));
					_serial->println(srcPath);
					_serial->print(F("to\n\t"));
					_serial->println(destPath);
				}
			}
		}
	}
	else
	{
		_serial->print(srcPath);
		_serial->println(F("Does not exist"));
	}
}




void Terminal::handle_MKDIR(char input[])
{
	char* path = findSubStr(input);

	if (!_sd->exists(path))
	{
		_sd->mkdir(path);

		if (_sd->exists(path))
		{
			_serial->print(path);
			_serial->println(F(" created"));
		}
		else
		{
			_serial->print(F("Failed to create: "));
			_serial->println(path);
		}
	}
	else
	{
		_serial->print(path);
		_serial->println(F(" already exists"));
	}
}




void Terminal::handle_CP(char input[])
{
	char* fullSrcPath = findSubStr(input, 1);
	char* destPath = findSubStr(input, 2);
	char* basename = findBasename(fullSrcPath);
	char* fullDestPath = join(destPath, basename);

	if (_sd->exists(fullSrcPath))
	{
		File srcFile = _sd->open(fullSrcPath);

		// make sure we only copy to a dir and not a file
		if (!strstr(destPath, "."))
		{
			if (!_sd->exists(destPath))
				_sd->mkdir(destPath);

			if (strcmp(pwd, "/"))
				fullSrcPath = join(pwd, fullSrcPath);

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
		}
		else
		{
			_serial->print(destPath);
			_serial->println(F(" is not a directory"));
		}
	}
	else
	{
		if (!_sd->exists(fullSrcPath))
		{
			_serial->print(fullSrcPath);
			_serial->println(F(" does not exists"));
		}
	}
}




void Terminal::handle_ECHO(char input[])
{
	char* flag = findSubStr(input);

	if (strstr(flag, "on"))
	{
		echo = true;
		_serial->println(F("Echo turned on"));
	}
	else if (strstr(flag, "off"))
	{
		echo = false;
		_serial->println(F("Echo turned off"));
	}
	else
		_serial->println('?');
}




void Terminal::handle_PRINT(char input[])
{
	char* filepath = findSubStr(input);

	if (_sd->exists(filepath))
	{
		_serial->print(filepath); _serial->println(F(" found:"));

		File file = _sd->open(filepath, FILE_READ);

		size_t n;
		uint8_t buf[64];
		while ((n = file.read(buf, sizeof(buf))) > 0)
			_serial->write(buf, n);

		file.close();
	}
	else
	{
		_serial->print(filepath);
		_serial->println(F(" doesn't exist"));
	}
}




void Terminal::handle_CREATE(char input[])
{
	char* filepath = findSubStr(input);

	if (!_sd->exists(filepath))
	{
		File newFile = _sd->open(filepath, FILE_WRITE);
		newFile.close();

		if (_sd->exists(filepath))
		{
			_serial->print(F("Created "));
			_serial->println(filepath);
		}
		else
		{
			_serial->print(F("Failed to create "));
			_serial->println(filepath);
		}
	}
	else
	{
		_serial->print(filepath);
		_serial->println(F(" already exists"));
	}
}




void Terminal::handle_AP(char input[])
{
	char* filepath = findSubStr(input, 1);
	char* content = findSubStr(input, 2);

	char* line = strstr(input, content);

	if (_sd->exists(filepath))
	{
		File newFile = _sd->open(filepath, FILE_WRITE);
		newFile.println(line);
		newFile.close();

		_serial->print(F("Wrote\n\t"));
		_serial->println(line);
		_serial->print(F("to\n\t"));
		_serial->println(filepath);
	}
	else
	{
		_serial->print(filepath);
		_serial->println(F(" does not exist"));
	}
}




void Terminal::handle_PWD()
{
	_serial->print(F("PWD: "));
	_serial->println(pwd);
}




void Terminal::handle_CD(char input[])
{
	char* newPWD = findSubStr(input);

	if (!strcmp(newPWD, ".."))
	{
		char* tempPWD = pwd;
		pwd = findDirname(pwd);

		if (!_sd->chdir(pwd, true))
		{
			_serial->print(F("Could not change PWD to "));
			_serial->println(pwd);

			pwd = tempPWD;
		}
	}
	else if (!strstr(newPWD, "."))
	{
		if (_sd->exists(newPWD))
		{
			if (!_sd->chdir(newPWD, true))
			{
				_serial->print(F("Could not change PWD to "));
				_serial->println(newPWD);
			}

			pwd = join(pwd, newPWD);
		}
		else
		{
			_serial->print(newPWD);
			_serial->println(F(" does not exist"));
		}
	}
	else
	{
		_serial->print(newPWD);
		_serial->println(F(" is not a directory"));
	}

	handle_PWD();
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




char* Terminal::findOccur(char input[], char target[], uint16_t ithOccur)
{
	char* argP = input;
	char* divP = NULL;

	for (uint8_t i=0; i<ithOccur; i++)
	{
		divP = strstr(argP, target);

		if (!divP)
			return NULL;

		argP = divP + 1;
	}

	return divP;
}




char* Terminal::findSubStr(char input[], uint8_t place, const char* delim)
{
	char* argP = input;
	char* divP = NULL;
	char* returnStr;

	for (uint8_t i=0; i<place; i++)
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




char* Terminal::findDirname(char path[])
{
	char* dirName = (char*)malloc(strlen(path) + 1);

	memcpy(dirName, path, strlen(path) + 1);
	uint8_t numLevels = numOccur(dirName, '/');

	if (dirName[strlen(dirName) - 1] == '/')
		numLevels--;

	if (numLevels > 1)
	{
		char rootStr[] = { '/', '\0' };
		dirName[(int)findOccur(dirName, rootStr, numLevels) - (int)dirName] = '\0';
	}
	else
	{
		dirName[0] = '/';
		dirName[1] = '\0';
	}

	return dirName;
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
	File srcFile = _sd->open(fullSrcPath, FILE_READ);
	File destFile = _sd->open(fullDestPath, FILE_WRITE);

	if (!srcFile)
	{
		_serial->print(F("Error opening source file at "));
		_serial->println(fullSrcPath);
		return;
	}

	if (!destFile)
	{
		_serial->print(F("Error opening destination file at "));
		_serial->println(fullDestPath);
		return;
	}

	size_t n;
	uint8_t buf[64];
	while ((n = srcFile.read(buf, sizeof(buf))) > 0)
		destFile.write(buf, n);

	if (srcFile.size() == destFile.size())
	{
		_serial->print(F("Copied "));
		_serial->print(fullSrcPath);
		_serial->print(F(" to "));
		_serial->println(fullDestPath);
	}
	else
	{
		_serial->print(F("Problem trying to copy "));
		_serial->print(fullSrcPath);
		_serial->print(F(" to "));
		_serial->println(fullDestPath);
	}

	srcFile.close();
	destFile.close();
}




void Terminal::copyDir(char fullSrcPath[], char fullDestPath[])
{
	File srcFile = _sd->open(fullSrcPath);

	if (srcFile.isDirectory())
	{
		if (!_sd->exists(fullDestPath))
			_sd->mkdir(fullDestPath);

		while (true)
		{
			File subFile = srcFile.openNextFile();

			if (!subFile)
				break;

			char fileName[MAX_NAME_LEN];

			subFile.getName(fileName, sizeof(fileName));
			char* srcFilePath = join(fullSrcPath, fileName);
			char* destFilePath = join(fullDestPath, fileName);

			if (subFile.isDirectory())
			{
				srcFile.close();
				copyDir(srcFilePath, destFilePath);
			}
			else
			{
				srcFile.close();
				copyFile(srcFilePath, destFilePath);
			}
		}

		if (_sd->exists(fullDestPath))
		{
			_serial->print(F("Copied "));
			_serial->print(fullSrcPath);
			_serial->print(F(" to "));
			_serial->println(fullDestPath);
		}
		else
		{
			_serial->print(F("Problem trying to copy "));
			_serial->print(fullSrcPath);
			_serial->print(F(" to "));
			_serial->println(fullDestPath);
		}
	}
	else
	{
		srcFile.close();
		copyFile(fullSrcPath, fullDestPath);
	}
}




void Terminal::deleteDirectory(File dir, char* path)
{
	char* fullPath;

	while (true)
	{
		File file = dir.openNextFile();

		if (!file)
			break;

		char fileName[MAX_NAME_LEN] = { '\0' };

		for (byte i = 0; i < sizeof(fileName); i++)
			fileName[i] = '\0';

		file.getName(fileName, sizeof(fileName));
		fullPath = join(path, fileName);

		// don't delete the volume info files
		if (!strstr(fullPath, "System Volume Infor"))
		{
			if (file.isDirectory())
				deleteDirectory(file, fullPath);
			else
			{
				_sd->remove(fullPath);

				if (!_sd->exists(fullPath))
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
	}

	_sd->rmdir(path);

	if (!_sd->exists(path))
	{
		_serial->print(F("Deleted: "));
		_serial->println(path);
	}
	else
	{
		if (!strcmp(path, "/ "))
		{
			_serial->print(F("Failed to delete: "));
			_serial->println(path);
		}
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
