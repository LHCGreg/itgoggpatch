#ifndef __UTILITIES_TEMPLATES_H__
#define __UTILITIES_TEMPLATES_H__

#include "utilities.h"

// Template implementation

namespace lhcutilities
{

template<typename T>
T Read(FILE* file, bool& eofOut)
{
	std::vector<unsigned char> bytes = ReadBytes(file, sizeof(T));
	if(bytes.size() == 0)
	{
		eofOut = true;
		return T();
	}
	else if(bytes.size() < sizeof(T))
	{
		eofOut = true;
		throw IoError("Unexpected end of file.");
	}
	else
	{
		eofOut = false;
		return GetFromBytes<T>(bytes);
	}
}

template<typename T>
T ReadOrDie(FILE* file)
{
	bool eof;
	T ret = Read<T>(file, eof);
	if(eof)
	{
		throw IoError("Unexpected end of file.");
	}
	else
	{
		return ret;
	}
}

template<typename T>
void WriteOrDie(FILE* file, T data)
{
	size_t elementsWritten = fwrite(&data, sizeof(T), 1, file);
	if(elementsWritten < 1)
	{
		throw IoError("Error while writing.");
	}
}

template<typename T>
T GetFromBytes(const std::vector<unsigned char>& bytes, size_t offset /* = 0 */)
{
	T ret = *(reinterpret_cast<const T*>(&(bytes[offset])));
	return ret;
}

template<typename T>
void AppendBytes(std::vector<unsigned char>& vec, T data)
{
	unsigned char* dataBegin = reinterpret_cast<unsigned char*>(&data);
	for(unsigned int byteIndex = 0; byteIndex < sizeof(T); byteIndex++)
	{
		vec.push_back(dataBegin[byteIndex]);
	}
}

template<typename T>
bool CheckBit(T number, unsigned int bitIndex)
{
	return (number & (1 << bitIndex)) != 0;
}


} // end namespace lhcutilites

#endif // end include guard

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
