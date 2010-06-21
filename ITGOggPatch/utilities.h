#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <vector>
#include <cstdio>
#include <stdexcept>

// Namespace lhcutilities contains various utility functions.
// The code is not tied to ITG Ogg Patcher and is reusable.
namespace lhcutilities
{

// Represents an error that happens while opening or reading a file
class IoError : public std::runtime_error
{
public:
	IoError(const std::string& message) : std::runtime_error(message)
	{
	}
};

// Reads numBytes from file. Throws lhcutilities::IoError if there was an error reading.
// If fewer bytes than asked for are returned because the end of the file was reached
// the returned vector will simply be of that size.
std::vector<unsigned char> ReadBytes(FILE* file, size_t numBytes);

// Reads numBytes from file. Throws IoError if there was an error reading.
// If fewer bytes than asked for are returned because the end of the file was reached,
// an lhcutilities::IoError is thrown
std::vector<unsigned char> ReadBytesOrDie(FILE* file, size_t numBytes);

// Type-safe enum for file seek origin
enum SeekOrigin
{
	Seek_Set = SEEK_SET,
	Seek_Cur = SEEK_CUR,
	Seek_End = SEEK_END
};

// Like fseek but throws lhcutilities::IoError if there is an error.
void SeekOrDie(FILE* file, long offset, SeekOrigin origin);

// Like ftell but throws lhcutilities::IoError if there is an error.
long TellOrDie(FILE* file);

// Like fopen but throws lhcutilities::IoError if there is an error.
FILE* OpenOrDie(const char* filename, const char* mode);

// Reads one T from the file. eofOut is set to true if eof is reached.
// If end of file was reached while trying to read (the number of bytes read was greater than 0 but less than sizeof(T)),
// lhcutilities::IoError is thrown.
// If eof was reached, the default value of T is returned. T should be a built-in type.
template<typename T>
T Read(FILE* file, bool& eofOut);

// Reads one T from the file. lhcutilities::IoError is thrown if end of file is reached while trying to read.
template<typename T>
T ReadOrDie(FILE* file);

// The bytes contained in data are written to file. T should be a built-in type.
// Throws lhcutilities::IoError if there is an error.
template<typename T>
void WriteOrDie(FILE* file, T data);

// Converts the bytes in the given vector at the given offset (default 0) to a T.
// T should be a built-in type.
template<typename T>
T GetFromBytes(const std::vector<unsigned char>& bytes, size_t offset = 0);

// Appends the bytes of data to the given vector. T should be a built-in type
template<typename T>
void AppendBytes(std::vector<unsigned char>& vec, T data);

// Returns the status of the bit at the given bit index in number (true for set, false for unset).
// T should be a built-in type.
template<typename T>
bool CheckBit(T number, unsigned int bitIndex);


} // end namespace lhcutilities


#include "utilities_templates.h" // template implementation

#endif // end include guard
