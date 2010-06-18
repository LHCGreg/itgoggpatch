#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <vector>
#include <cstdio>
#include <stdexcept>

namespace lhcutilities
{

class IoError : public std::runtime_error
{
public:
	IoError(const std::string& message) : std::runtime_error(message)
	{
	}
};

std::vector<unsigned char> ReadBytes(FILE* file, size_t numBytes);
std::vector<unsigned char> ReadBytesOrDie(FILE* file, size_t numBytes);

enum SeekOrigin
{
	Seek_Set,
	Seek_Cur,
	Seek_End
};

void SeekOrDie(FILE* file, long offset, SeekOrigin origin);

long TellOrDie(FILE* file);

FILE* OpenOrDie(const char* filename, const char* mode);

template<typename T>
T Read(FILE* file, bool& eofOut);

template<typename T>
T ReadOrDie(FILE* file);

template<typename T>
void WriteOrDie(FILE* file, T data);

template<typename T>
T GetFromBytes(const std::vector<unsigned char>& bytes, size_t offset = 0);

template<typename T>
void AppendBytes(std::vector<unsigned char>& vec, T data);

template<typename T>
bool CheckBit(T number, unsigned int bitIndex);


} // end namespace lhcutilities


#include "utilities_templates.h" // template implementation

#endif // end include guard
