#include "stdafx.h"
#include "ogglength.h"
#include <vorbis/vorbisfile.h>
#include "utilities.h"
#include <vector>
#include <cstdio>
#include <string>
#include <exception>
#include <boost/lexical_cast.hpp>

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

// Automatically link to libogg, libvorbis, and libvorbis file if using MSVC.
// Use the dynamic versions if OGG_DYNAMIC is defined, use the static versions if OGG_STATIC is defined or neither is defined.
// If linking statically, the libraries must have been compiled with the same standard library settings (debug vs. release)
#ifdef _MSC_VER
#if defined(OGG_STATIC) && defined(OGG_DYNAMIC)
#error Ogg static or Ogg dynamic - pick one, not both
#endif
#ifdef OGG_DYNAMIC
#pragma comment(lib, "libogg.lib")
#pragma comment(lib, "libvorbis.lib")
#pragma comment(lib, "libvorbisfile.lib")
#else
#pragma comment(lib, "libogg_static.lib")
#pragma comment(lib, "libvorbis_static.lib")
#pragma comment(lib, "libvorbisfile_static.lib")
#endif
#endif

using namespace std;
using namespace boost;
using namespace lhcutilities;

namespace ogglength
{

OggVorbisFile::OggVorbisFile(const char* filePath) : m_handle(new _OggVorbisFile())
{
	// ov_fopen taking a char* instead of a const char* is an acknowledged bug in the API
	int openResult = ov_fopen(const_cast<char*>(filePath), &(m_handle->file));
	if(openResult != 0)
	{
		throw OggVorbisError("Error opening Ogg Vorbis file.");
	}
	
	m_handle->opened = true;
}

OggVorbis_File* OggVorbisFile::get()
{
	return &(m_handle->file);
}

double GetReportedTime(const char* filePath)
{
	OggVorbisFile oggFile(filePath);
	double reportedTime = ov_time_total(oggFile.get(), -1);
	if(reportedTime == OV_EINVAL) // I think this can happen if the file is marked as unseekable in the Vorbis headers?
	{
		throw OggVorbisError("Ogg Vorbis file is not seekable."); 
	}
	return reportedTime;
}

double GetRealTime(const char* filePath)
{
	OggVorbisFile oggFile(filePath);
	double totalTimeRead = 0;
	char buffer[4096];
	int logicalBitstreamRead = -555;
	int logicalBitstreamReading = -1;

	long totalBytesRead = 0;
	long totalSamplesRead = 0;

	long bytesRead;
	int pcmWordSize = 2; // 16-bit samples
	while((bytesRead = ov_read(oggFile.get(), buffer, 4096, 0, pcmWordSize, 1, &logicalBitstreamRead)) > 0)
	{
		if(logicalBitstreamReading == -1)
		{
			logicalBitstreamReading = logicalBitstreamRead;
		}
		else if(logicalBitstreamReading != logicalBitstreamRead)
		{
			throw OggVorbisError("More than one logical bitstream in the file. Can't handle that currently.");
		}
		long samplesRead = bytesRead / pcmWordSize;
		double timeRead = static_cast<double>(samplesRead) / oggFile.get()->vi[logicalBitstreamRead].rate;
		int numChannels = oggFile.get()->vi[logicalBitstreamRead].channels;
		if(numChannels <= 0)
		{
			throw OggVorbisError("Number of channels is not a positive number.");
		}
		timeRead /= numChannels;

		totalBytesRead += bytesRead;
		totalSamplesRead += samplesRead;
		totalTimeRead += timeRead;
	}

	if(bytesRead < 0)
	{
		throw OggVorbisError("Error while decoding. The file may be corrupt.");
	}

	return totalTimeRead;
}



// Sets the length of an Ogg Vorbis file in samples.
// This is done by changing the granule position field of the last Ogg page.
// The file is assumed to be a normal Ogg Vorbis file (1 logical bitstream).
// If the file is not an Ogg file, a logic_error will be thrown.
// If the file is an Ogg file but not a file containing only a Vorbis bitstream, behavior is undefined.
// ogglength::OggVorbisError will be thrown if the he file could not be opened or some other IO error occurs
// or the file does not appear to be an Ogg Vorbis file or seems corrupt.
// If the function returns without throwing an exception, it succeeded.
void ChangeSongLength(const char* filePath, double numSeconds)
{
	FILE* file = NULL;
	try
	{
		file = OpenOrDie(filePath, "r+b");

		unsigned char version; // Version field of an Ogg page
		unsigned char headerType; // Header type field of an Ogg page.
		ogg_uint32_t sampleRate = 0;
		ogg_int32_t savedBitstreamSerialNumber = 0; // assignment stops a gcc warning. I know that it will have been initialized when it's used.

		// Read Ogg pages until we get to the last page. Have the seek pointer at granule position then.
		while(true)
		{
			// All Ogg pages begin with the bytes "OggS"
			vector<unsigned char> buffer = ReadBytesOrDie(file, 4);
			if(buffer[0] != 'O' || buffer[1] != 'g' || buffer[2] != 'g' || buffer[3] != 'S')
			{
				throw OggVorbisError("File does not appear to be an Ogg file or is corrupted.");
			}

			// Ogg version field. Currently should always be 0.
			version = ReadOrDie<unsigned char>(file);
			if(version != 0)
			{
				throw OggVorbisError("Ogg version is " + lexical_cast<string>(static_cast<unsigned int>(version)) + ". Don't know how to handle versions other than 0.");
			}

			// Header type field indicates if this page is the beginning,
			// end, or middle of an Ogg logical bitstream.
			// It is permitted for a page to be both beginning and end - that means it's the only page.
			headerType = ReadOrDie<unsigned char>(file);
			UNUSED bool continuation = CheckBit(headerType, 0);
			UNUSED bool beginningOfStream = CheckBit(headerType, 1);
			bool endOfStream = CheckBit(headerType, 2);

			// End of stream bit was set in the header type field - this is the last page of the logical bitstream
			if(endOfStream)
			{
				break;
			}
			
			// In Vorbis logical bitstreams, the granule position is the number of the last sample
			// contained in this frame.
			UNUSED ogg_int64_t granulePosition = ReadOrDie<ogg_int64_t>(file);
			// Bitstream serial number might be of interest if we wanted to be able to handle Ogg files with
			// multiple logical bitstreams...but we don't care.
			ogg_int32_t bitstreamSerialNumber = ReadOrDie<ogg_int32_t>(file);
			if(sampleRate != 0 && bitstreamSerialNumber != savedBitstreamSerialNumber)
			{
				throw OggVorbisError("More than one logical bitstream in the file. Can't handle that currently.");
			}
			savedBitstreamSerialNumber = bitstreamSerialNumber;

			UNUSED ogg_int32_t pageSequenceNumber = ReadOrDie<ogg_int32_t>(file);
			UNUSED ogg_int32_t checksum = ReadOrDie<ogg_int32_t>(file);
			unsigned char numSegments = ReadOrDie<unsigned char>(file);

			if(sampleRate == 0)
			{
				vector<unsigned char> segmentSizes = ReadBytesOrDie(file, numSegments);
				ogg_uint32_t vorbisHeaderPacketSize = 0;
				for(int segIndex = 0; segIndex < numSegments; segIndex++)
				{
					vorbisHeaderPacketSize += segmentSizes[segIndex];
					if(segmentSizes[segIndex] < 255)
					{
						break;
					}
				}

				if(vorbisHeaderPacketSize < 16)
				{
					throw OggVorbisError("Couldn't read Vorbis header.");
				}

				unsigned char packetType = ReadOrDie<unsigned char>(file);
				if(packetType != 1)
				{
					throw OggVorbisError("Does not appear to be an Ogg Vorbis file.");
				}
				
				vector<unsigned char> vorbisString = ReadBytesOrDie(file, 6);
				if(vorbisString[0] != 'v' || vorbisString[1] != 'o' || vorbisString[2] != 'r' 
				|| vorbisString[3] != 'b' || vorbisString[4] != 'i' || vorbisString[5] != 's')
				{
					throw OggVorbisError("Does not appear to be an Ogg Vorbis file.");
				}

				ogg_uint32_t vorbisVersion = ReadOrDie<ogg_uint32_t>(file);
				if(vorbisVersion != 0)
				{
					throw OggVorbisError("Vorbis version is not 0, don't know how to handle other version.");
				}

				UNUSED unsigned char numChannels = ReadOrDie<unsigned char>(file);
				sampleRate = ReadOrDie<ogg_uint32_t>(file);
				if(sampleRate == 0)
				{
					throw OggVorbisError("File has a sample rate of 0. O_o");
				}

				ogg_int32_t pageDataSize = 0;
				for(unsigned char segmentIndex = 0; segmentIndex < numSegments; segmentIndex++)
				{
					unsigned char segmentSize = segmentSizes[segmentIndex];
					pageDataSize += segmentSize;
				}

				ogg_int32_t unreadDataBytes = pageDataSize - 16;
				SeekOrDie(file, unreadDataBytes, Seek_Cur);
			}
			else
			{
				// We're not reading the data portion of the page, so we don't need to know how
				// large each segment is, just the total size.
				ogg_int32_t pageDataSize = 0;
				for(unsigned char segmentIndex = 0; segmentIndex < numSegments; segmentIndex++)
				{
					unsigned char segmentSize = ReadOrDie<unsigned char>(file);
					pageDataSize += segmentSize;
				}

				// Skip the data of the page, we're not interested in it.
				SeekOrDie(file, pageDataSize, Seek_Cur);
			}
			
		}

		// Converting from seconds to samples might cause the result to be off be 1 if the number of seconds
		// came from GetRealTime().
		ogg_int64_t numSamples = static_cast<ogg_int64_t>(numSeconds * sampleRate);

		// Remember where the granule position field is, we're going to edit it later.
		// For now, we're reading the entire page so we can calculate what the checksum
		// should be after we change the granule position field.
		long granulePositionPosition = TellOrDie(file);

		ogg_int64_t granulePosition = ReadOrDie<ogg_int64_t>(file);
		granulePosition = numSamples; // Set to what it will be for checksum calculation

		ogg_int32_t bitstreamSerialNumber = ReadOrDie<ogg_int32_t>(file);
		ogg_int32_t pageSequenceNumber = ReadOrDie<ogg_int32_t>(file);

		ogg_int32_t checksum = ReadOrDie<ogg_int32_t>(file);
		checksum = 0; // Set to 0 for checksum calculation

		unsigned char numSegments = ReadOrDie<unsigned char>(file);

		vector<unsigned char> segmentSizes = ReadBytesOrDie(file, numSegments);

		// Reconstruct the header (with the new granule position) as a byte vector for checksumming
		vector<unsigned char> headerBytes;
		headerBytes.push_back('O');
		headerBytes.push_back('g');
		headerBytes.push_back('g');
		headerBytes.push_back('S');
		headerBytes.push_back(version);
		headerBytes.push_back(headerType);
		AppendBytes(headerBytes, granulePosition);
		AppendBytes(headerBytes, bitstreamSerialNumber);
		AppendBytes(headerBytes, pageSequenceNumber);
		AppendBytes(headerBytes, checksum);
		headerBytes.push_back(numSegments);
		headerBytes.insert(headerBytes.end(), segmentSizes.begin(), segmentSizes.end());

		
		// Calculate the size of the data part of the page
		ogg_int32_t pageDataSize = 0;
		for(unsigned char segmentIndex = 0; segmentIndex < numSegments; segmentIndex++)
		{
			unsigned char segmentSize = segmentSizes[segmentIndex];
			pageDataSize += segmentSize;
		}

		// Read the entire data part of the page into a byte vector for checksumming
		vector<unsigned char> dataBytes = ReadBytesOrDie(file, pageDataSize);


		ogg_page page;
		page.header_len = headerBytes.size();
		page.header = &(headerBytes[0]);
		page.body_len = dataBytes.size();
		if(dataBytes.size() > 0)
		{
			page.body = &(dataBytes[0]);
		}

		// Let libogg do the tricky CRC stuff
		ogg_page_checksum_set(&page);
		checksum = GetFromBytes<ogg_int32_t>(headerBytes, 22);

		// Finally, write the updated granule position and checksum. We're not changing the file
		// size or moving anything around, so we can just edit the file in place.
		SeekOrDie(file, granulePositionPosition, Seek_Set);
		WriteOrDie(file, granulePosition);
		SeekOrDie(file, 8, Seek_Cur);
		WriteOrDie(file, checksum);
	}
	catch(IoError& ex)
	{
		if(file != NULL)
		{
			fclose(file); // Clean up if something went wrong
			throw OggVorbisError(ex.what());
		}
	}
	catch(OggVorbisError&)
	{
		if(file != NULL)
		{
			fclose(file); // Clean up if something went wrong
			throw;
		}
	}

	fclose(file); // Clean up
}

} // end namespace ogglength
