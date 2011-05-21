#include "stdafx.h"
#include "ogglength.h"
#include <vorbis/vorbisfile.h>
#include "utilities.h"
#include <vector>
#include <cstdio>
#include <string>
#include <exception>
#include <boost/lexical_cast.hpp>

// gcc can issue warnings for unused variables. It is common to read fields that are not otherwise needed
// when reading a file format. Those variables are marked with the gcc "unused" attribute to suppress
// warning for a specific variable.
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

// Automatically link to libogg, libvorbis, and libvorbisfile if using MSVC.
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
	// ov_fopen taking a char* instead of a const char* is an acknowledged bug in the API, so the cast is ok.
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
	// Get the real song length by decoding the vorbis stream and doing the math with the number of samples
	// each ov_read gives.
	
	OggVorbisFile oggFile(filePath);
	double totalTimeRead = 0; // in seconds
	char buffer[4096];
	int logicalBitstreamRead = -555; // Number of the logical bitstream of the current page
	
	// Number of the logical bitstream we've been reading the whole time, -1 if we're on the first page.
	// If different from logicalBistreamRead, there's more than one logical bitstream so we bail.
	int logicalBitstreamReading = -1;

	long bytesRead;
	int pcmWordSize = 2; // 16-bit samples
	while((bytesRead = ov_read(oggFile.get(), buffer, 4096, 0, pcmWordSize, 1, &logicalBitstreamRead)) > 0)
	{
		if(logicalBitstreamReading == -1)
		{
			// First Ogg page - this should be the logical bitstream all pages are in.
			logicalBitstreamReading = logicalBitstreamRead;
		}
		else if(logicalBitstreamReading != logicalBitstreamRead)
		{
			// A page in a logical bitstream different from the one we've been reading
			throw OggVorbisError("More than one logical bitstream in the file. Can't handle that.");
		}
		long samplesRead = bytesRead / pcmWordSize; // actually this is samples * channels
		int numChannels = oggFile.get()->vi[logicalBitstreamRead].channels;
		if(numChannels <= 0) // Don't crash with a divide by 0
		{
			throw OggVorbisError("Number of channels is not a positive number.");
		}
		long samplesPerChannel = samplesRead / numChannels;
		double timeRead = static_cast<double>(samplesPerChannel) / oggFile.get()->vi[logicalBitstreamRead].rate;

		totalTimeRead += timeRead;
	}

	if(bytesRead < 0)
	{
		throw OggVorbisError("Error while decoding. The file may be corrupt.");
	}

	return totalTimeRead;
}

void ChangeSongLength(const char* filePath, double numSeconds)
{
	// For details of the Ogg format, see http://xiph.org/ogg/doc/, http://xiph.org/ogg/doc/oggstream.html,
	// http://xiph.org/ogg/doc/framing.html, http://en.wikipedia.org/wiki/Ogg
	//
	// For details of the Vorbis format, see http://xiph.org/vorbis/doc/Vorbis_I_spec.html
	
	FILE* file = NULL;
	try
	{
		// Ehhhh, can't be bothered to make an RAII class for FILE*'s, so just catch exceptions and close then and at the end.
		file = OpenOrDie(filePath, "r+b");

		unsigned char version; // Version field of an Ogg page
		unsigned char headerType; // Header type field of an Ogg page.
		ogg_uint32_t sampleRate = 0;
		ogg_int32_t savedBitstreamSerialNumber = 0; // assignment stops a gcc warning. I know that it will have been initialized when it's used.

		// Read Ogg pages until we get to the last page (indicated by the "end of stream" bit set in the Ogg page header).
		// Have the seek pointer at granule position then.
		// Special care if given to the first page, because that is the primary Vorbis header and contains
		// the sample rate, which is needed to calculate what we should set the granule position of the last page to.
		while(true)
		{
			// All Ogg pages begin with the bytes "OggS"
			vector<unsigned char> buffer = ReadBytesOrDie(file, 4);
			if(buffer[0] != 'O' || buffer[1] != 'g' || buffer[2] != 'g' || buffer[3] != 'S')
			{
				throw OggVorbisError("File does not appear to be an Ogg file.");
			}

			// Ogg version field. Currently should always be 0.
			version = ReadOrDie<unsigned char>(file);
			if(version != 0)
			{
				throw OggVorbisError("The file is corrupt.");
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
				throw OggVorbisError("The file is not a simple Ogg Vorbis file.");
			}
			savedBitstreamSerialNumber = bitstreamSerialNumber;

			UNUSED ogg_int32_t pageSequenceNumber = ReadOrDie<ogg_int32_t>(file);
			UNUSED ogg_int32_t checksum = ReadOrDie<ogg_int32_t>(file);
			unsigned char numSegments = ReadOrDie<unsigned char>(file);

			if(sampleRate == 0)
			{
				// This is the first Ogg page, so it's the primary Vorbis header.
				vector<unsigned char> segmentSizes = ReadBytesOrDie(file, numSegments);
				ogg_uint32_t vorbisHeaderPacketSize = 0;
				for(int segIndex = 0; segIndex < numSegments; segIndex++)
				{
					vorbisHeaderPacketSize += segmentSizes[segIndex];
					if(segmentSizes[segIndex] < 255)
					{
						break; // a segment size of less than 255 indicates the end of a packet.
					}
				}

				if(vorbisHeaderPacketSize < 16)
				{
					throw OggVorbisError("Does not appear to be an Ogg Vorbis file.");
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
					throw OggVorbisError("The file is corrupt.");
				}

				UNUSED unsigned char numChannels = ReadOrDie<unsigned char>(file);
				sampleRate = ReadOrDie<ogg_uint32_t>(file);
				if(sampleRate == 0)
				{
					throw OggVorbisError("The file is corrupt.");
				}

				ogg_int32_t pageDataSize = 0;
				for(unsigned char segmentIndex = 0; segmentIndex < numSegments; segmentIndex++)
				{
					unsigned char segmentSize = segmentSizes[segmentIndex];
					pageDataSize += segmentSize;
				}

				// Skip the rest of the page, we're not interested in it.
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

		if(sampleRate == 0)
		{
			throw OggVorbisError("The file is corrupt.");
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


		// Create an ogg_page structure using the header and body byte vectors so that libogg can do the checksum
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
		}
		throw OggVorbisError(ex.what()); // Repackage as an OggVorbisError to keep the exception specification clean.
	}
	catch(OggVorbisError&)
	{
		if(file != NULL)
		{
			fclose(file); // Clean up if something went wrong
		}
		throw;
	}

	fclose(file); // Clean up
}

} // end namespace ogglength

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
