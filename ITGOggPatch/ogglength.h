#ifndef __OGGLENGTH_H__
#define __OGGLENGTH_H__

#include <vorbis/vorbisfile.h>
#include <boost/shared_ptr.hpp>
#include <stdexcept>

namespace ogglength
{

double GetReportedTime(const char* filePath);
double GetRealTime(const char* filePath); // TODO: make const
void ChangeSongLength(const char* filePath, double numSeconds); // TODO: make const

class OggVorbisError : public std::runtime_error
{
public:
	OggVorbisError(const std::string& message) : std::runtime_error(message)
	{
	}
};

struct _OggVorbisFile
{
	OggVorbis_File file;
	bool opened;

	_OggVorbisFile()
	{
		opened = false;
	}

	~_OggVorbisFile()
	{
		if(opened)
		{
			ov_clear(&file);
		}
	}
};

class OggVorbisFile
{
private:
	boost::shared_ptr<_OggVorbisFile> m_handle;

public:
	explicit OggVorbisFile(const char* filePath);

	// Compiler-supplied copy constructor, copy assignment, and destructor are ok

	OggVorbis_File* get();
};


} // end namespace ogglength

#endif // end include guard