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

#ifdef _MSC_VER
#pragma warning(disable:4996) // 'fopen': This function or variable may be unsafe. Consider using fopen_s instead.
#endif
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

	#ifdef _MSC_VER
	#pragma warning(default:4996)
	#endif
}

void SeekOrDie(FILE* file, long offset, SeekOrigin origin)
{
	int seekSuccess = fseek(file, offset, static_cast<int>(origin));
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

/*
 Copyright 2010 Greg Najda

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
