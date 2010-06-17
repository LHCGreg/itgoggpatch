#include "stdafx.h"
#include <iostream>
#include <exception>
#include <string>
#include "ogglength.h"
#include "utilities.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/system/system_error.hpp>
#include <boost/algorithm/string.hpp>


using namespace std;
using namespace boost;
using namespace ogglength;
using namespace lhcutilities;
namespace fs = boost::filesystem;

enum PatcherLengthCondition
{
	condition_none,
	condition_equal,
	condition_greater
};

class PatcherOptions
{
private:
	bool m_patchToRealLength;
	PatcherLengthCondition m_lengthConditionType;
	double m_lengthCondition;
	double m_timeInSeconds;
	string m_startingPath;

public:
	PatcherOptions() : m_patchToRealLength(false), m_lengthConditionType(condition_none),
		m_lengthCondition(120), m_timeInSeconds(105), m_startingPath()
	{
	}
	
	void PatchToRealLength() { m_patchToRealLength = true; }
	bool PatchingToRealLength() const { return m_patchToRealLength; }
	void TimeInSeconds(double timeInSeconds) { m_patchToRealLength = false; m_timeInSeconds = timeInSeconds; }
	double TimeInSeconds() const { return !m_patchToRealLength ? m_timeInSeconds : -1; }
	void StartingPath(const string& startingPath) { m_startingPath = startingPath; }
	const string& StartingPath() const { return m_startingPath; }
	
	void UseLengthEqualCondition(double lengthCondition)
	{
		m_lengthConditionType = condition_equal;
		m_lengthCondition = lengthCondition;
	}
	void UseLengthGreaterThanCondition(double lengthCondition)
	{
		m_lengthConditionType = condition_greater;
		m_lengthCondition = lengthCondition;
	}
	void DontUseLengthCondition()
	{
		m_lengthConditionType = condition_none;
	}

	PatcherLengthCondition LengthConditionType() const { return m_lengthConditionType; }
	double LengthCondition() const { return m_lengthCondition; }

	bool LengthMeetsConditions(double reportedSongLength)
	{
		if(m_lengthConditionType == condition_none)
		{
			return true;
		}
		else if(m_lengthConditionType == condition_equal)
		{
			return reportedSongLength < m_lengthCondition + .01 && reportedSongLength > m_lengthCondition - .01;
		}
		else if(m_lengthConditionType == condition_greater)
		{
			return reportedSongLength > m_lengthCondition;
		}
		else
		{
			throw std::runtime_error("Oops, missed a length condition type.");
		}
	}

	bool FileMeetsConditions(const string& file)
	{
		if(m_lengthConditionType != condition_none)
		{
			double reportedLength = GetReportedTime(file.c_str());
			return LengthMeetsConditions(reportedLength);
		}
		else
		{
			return true;
		}
	}
};

class Patcher
{
private:
	PatcherOptions m_options;

public:
	Patcher(const PatcherOptions& options)
	{
		m_options = options;
	}

	void Patch();

private:
	void LengthPatchDirectory(const string& directory);
	void LengthPatchFile(const string& file);
	void PrintError(const string& path, const std::exception& error);
};

void Patcher::Patch()
{
	if(m_options.StartingPath().length() == 0)
	{
		throw logic_error("No path specified");
	}

	try
	{
		if(!fs::exists(m_options.StartingPath()))
		{
			throw IoError("No file or directory with this path exists.");
		}
		
		if(fs::is_directory(m_options.StartingPath()))
		{
			LengthPatchDirectory(m_options.StartingPath());
		}
		else if(fs::is_regular_file(m_options.StartingPath()))
		{
			LengthPatchFile(m_options.StartingPath());
		}
		else
		{
			throw IoError("This path indicates something that is not a file or a directory.");
		}
	}
	catch(IoError& ex)
	{
		PrintError(m_options.StartingPath(), ex);
	}
	catch(OggVorbisError& ex)
	{
		PrintError(m_options.StartingPath(), ex);
	}
	catch(boost::system::system_error& ex)
	{
		PrintError(m_options.StartingPath(), ex);
	}
}

// Can throw lhcutilities::IoError, boost::system::system_error if something goes
// wrong with the directory specified. Errors with files contained in the
// directory are handled locally by printing an error message.
void Patcher::LengthPatchDirectory(const string& directory)
{
	cout << directory << "   - " << "traversing directory" << endl;
	fs::directory_iterator endIt;
	for(fs::directory_iterator dirIt(directory); dirIt != endIt; ++dirIt)
	{
		try
		{
			if(fs::is_directory(dirIt->status()))
			{
				LengthPatchDirectory(dirIt->path().file_string());
			}
			else if(fs::is_regular_file(dirIt->status()) && boost::iends_with(dirIt->path().filename(), ".ogg"))
			{
				LengthPatchFile(dirIt->path().file_string());
			}
		}
		catch(logic_error& ex)
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

void Patcher::LengthPatchFile(const string& file)
{
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

		cout << file << "   - " << "patching to " << lengthToPatchTo << " seconds." << endl;
		ChangeSongLength(file.c_str(), lengthToPatchTo);
		cout << file << "   - " << "patched." << endl; // TODO: minutes:second formatting?
	}
	else
	{
		cout << file << "   - " << "skipping." << endl;
	}
}

void Patcher::PrintError(const string& path, const std::exception& error)
{
	cout << path << "   - " << error.what() << endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int exitCode = 0;
	try
	{
		string oggPath = "C:\\OggTest\\Through The Fire And Flames.ogg";

		PatcherOptions options;
		//options.PatchToRealLength();
		//options.StartingPath(oggPath);
		//options.UseLengthEqualCondition(105);
		options.UseLengthGreaterThanCondition(120);
		options.TimeInSeconds(105);
		options.StartingPath(oggPath);

		Patcher patcher(options);
		patcher.Patch();
	}
	catch(std::exception& ex)
	{
		cout << ex.what() << endl;
		exitCode = 2;
	}

	cout << "Press enter to exit." << endl;
	string line;
	getline(cin, line);
}