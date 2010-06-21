#include "stdafx.h"
#include "Patcher.h"
#include <vector>
#include <string>
#include <iostream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/system/system_error.hpp>
#include <boost/algorithm/string.hpp>
#include "utilities.h"
#include "ogglength.h"

using namespace std;
using namespace lhcutilities;
using namespace ogglength;
namespace fs = boost::filesystem;


namespace oggpatcher
{

void Patcher::Patch()
{	
	// For each path that we were told to patch
	for(vector<string>::size_type pathIndex = 0; pathIndex < m_options.StartingPaths().size(); pathIndex++)
	{
		const string& path = m_options.StartingPaths()[pathIndex];
		try
		{
			if(!fs::exists(path))
			{
				throw IoError("No file or directory with this path exists.");
			}
			
			if(fs::is_directory(path))
			{
				LengthPatchDirectory(path);
			}
			else if(fs::is_regular_file(path))
			{
				LengthPatchFile(path);
			}
			else
			{
				throw IoError("This path indicates something that is not a file or a directory.");
			}
		}
		catch(IoError& ex)
		{
			// ITG Ogg Patch code can throw IoError. It doesn't throw boost::system::system_error because an error code
			// must be provided. Although I think I could just use any error code I like...oh well, what's done is done.
			PrintError(path, ex);
		}
		catch(OggVorbisError& ex)
		{
			PrintError(path, ex);
		}
		catch(boost::system::system_error& ex)
		{
			PrintError(path, ex);
		}
	}
}

// Can throw boost::system::system_error if something goes wrong with the directory specified.
// Errors with files contained in the directory are handled locally by printing an error message.
void Patcher::LengthPatchDirectory(const string& directory)
{
	fs::directory_iterator endIt;
	for(fs::directory_iterator dirIt(directory); dirIt != endIt; ++dirIt)
	{
		try
		{
			// Don't recursively search a directory if it is a symlink to avoid infinite recursion.
			if(fs::is_directory(dirIt->status()) && !fs::is_symlink(dirIt->status()))
			{
				LengthPatchDirectory(dirIt->path().file_string());
			}
			else if(fs::is_regular_file(dirIt->status()) && boost::iends_with(dirIt->path().filename(), ".ogg"))
			{
				LengthPatchFile(dirIt->path().file_string());
			}
		}
		catch(IoError& ex)
		{
			PrintError(dirIt->path().file_string(), ex);
		}
		catch(OggVorbisError& ex)
		{
			PrintError(dirIt->path().file_string(), ex);
		}
		catch(boost::system::system_error& ex)
		{
			PrintError(dirIt->path().file_string(), ex);
		}
	}
}

// Can throw ogglength::OggVorbisError if there was an error patching the file.
void Patcher::LengthPatchFile(const string& file)
{
	// Skip the file if it does not meet the conditions for processing it.
	if(m_options.FileMeetsConditions(file))
	{
		double lengthToPatchTo;
		if(m_options.PatchingToRealLength())
		{
			cout << file << "   - " << "getting actual song length..." << endl;
			lengthToPatchTo = GetRealTime(file.c_str());
		}
		else
		{
			lengthToPatchTo = m_options.TimeInSeconds();
		}

		cout << file << "   - " << "patching to " << lengthToPatchTo << " seconds." << endl; // TODO: minutes:second formatting?
		ChangeSongLength(file.c_str(), lengthToPatchTo);
		cout << file << "   - " << "patched." << endl;
	}
	else
	{
		// Perhaps we should be more clear to the user about why we are skipping the file.
		cout << file << "   - " << "skipping." << endl;
	}
}

void Patcher::PrintError(const string& path, const std::exception& error)
{
	cout << path << "   - " << error.what() << endl;
}

} // end namespace oggpatcher
