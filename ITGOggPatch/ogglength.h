#ifndef __OGGLENGTH_H__
#define __OGGLENGTH_H__

#include <vorbis/vorbisfile.h>
#include <boost/shared_ptr.hpp>
#include <stdexcept>

// ogglength is reusable code.
namespace ogglength
{

// Gets the length in seconds of an Ogg Vorbis file that will be reported by most media players and utilities.
// Can throw ogglength::OggVorbisError if there is a problem opening or reading the file.
double GetReportedTime(const char* filePath);

// Gets the real length in seconds of an Ogg Vorbis file. This can differ from the reported length if the file has
// been tampered with.
// Can throw ogglength::OggVorbisError if there is a problem opening or reading the file.
double GetRealTime(const char* filePath);

// Sets the length of an Ogg Vorbis file in seconds.
// This is done by changing the granule position field of the last Ogg page.
// The file must be a normal Ogg Vorbis file (1 logical bitstream).
// ogglength::OggVorbisError can be thrown for various error conditions, including being unable to open
// the file, the file not being an Ogg Vorbis file, or the file appearing to be corrupt.
// If the function returns without throwing an exception, it succeeded.
void ChangeSongLength(const char* filePath, double numSeconds);

// Represents an error while opening or reading an Ogg Vorbis file.
class OggVorbisError : public std::runtime_error
{
public:
	OggVorbisError(const std::string& message) : std::runtime_error(message)
	{
	}
};

// Helper struct for OggVorbisFile, not intended to be used by other code.
struct _OggVorbisFile
{
	OggVorbis_File file; // Vorbis file handle
	bool opened; // Whether the file has actually been opened yet and the handle is valid

	explicit _OggVorbisFile() : file(), opened(false)
	{
	}

	~_OggVorbisFile()
	{
		// This is what we need this struct for - only call ov_clear if we succeeded in opening the file.
		if(opened)
		{
			ov_clear(&file);
		}
	}
};

// Resource-managing class for an OggVorbis_File handle from libvorbisfile.
// The underlying handle is reference counted. Copying an object of this class
// increments the reference count and both objects will have the same handle.
// The file is only closed when the last reference is done with it.
class OggVorbisFile
{
private:
	boost::shared_ptr<_OggVorbisFile> m_handle; // reference counting made easy

public:
	// Opens the given file as an Ogg Vorbis file.
	// Throws ogglength::OggVorbisError if an error occurs.
	explicit OggVorbisFile(const char* filePath);

	// Compiler-supplied copy constructor, copy assignment, and destructor are ok

	// Gets the underlying OggVorbis_File handle
	OggVorbis_File* get();
};


} // end namespace ogglength

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
