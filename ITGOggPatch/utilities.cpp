#include "stdafx.h"
#include "utilities.h"
#include <exception>
#include <string>

using namespace std;

namespace lhcutilities
{

vector<unsigned char> ReadBytes(FILE* file, size_t numBytes)
{
	if(file == NULL)
	{
		throw logic_error("Assertion failed: file is null.");
	}

	vector<unsigned char> buffer(numBytes, 0);

	if(numBytes == 0)
	{
		return buffer;
	}

	size_t bytesRead = fread(&buffer[0], 1, numBytes, file);
	if(ferror(file) != 0)
	{
		throw IoError("Error reading from file.");
	}

	if(bytesRead < numBytes)
	{
		buffer.resize(bytesRead);
	}

	return buffer;
}

vector<unsigned char> ReadBytesOrDie(FILE* file, size_t numBytes)
{
	vector<unsigned char> bytes = ReadBytes(file, numBytes);
	if(bytes.size() < numBytes)
	{
		throw IoError("Unexpected end of file.");
	}
	else
	{
		return bytes;
	}
}

FILE* OpenOrDie(const char* filename, const char* mode)
{
	FILE* file = fopen(filename, mode);
	if(file == NULL)
	{
		throw IoError(string("Could not open file ") + filename + ".");
	}
	else
	{
		return file;
	}
}

void SeekOrDie(FILE* file, long offset, SeekOrigin origin)
{
	int originInt;
	if(origin == Seek_Cur)
	{
		originInt = SEEK_CUR;
	}
	else if(origin == Seek_End)
	{
		originInt = SEEK_END;
	}
	else
	{
		originInt = SEEK_SET;
	}

	int seekSuccess = fseek(file, offset, originInt);
	if(seekSuccess != 0)
	{
		throw IoError("Error while seeking.");
	}
}

long TellOrDie(FILE* file)
{
	long seekPosition = ftell(file);
	if(seekPosition == -1)
	{
		throw IoError("Error while getting file position.");
	}

	return seekPosition;
}

} // end namespace lhcutilities
