#include "stdafx.h"
#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <cctype>
#include "ogglength.h"
#include "utilities.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/system/system_error.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>


using namespace std;
using namespace boost;
using namespace ogglength;
using namespace lhcutilities;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

enum PatcherLengthCondition
{
	condition_none,
	condition_equal,
	condition_greater
};

class PatcherOptions
{
private:
	bool m_displayHelp;
	bool m_patchToRealLength;
	PatcherLengthCondition m_lengthConditionType;
	double m_lengthCondition;
	double m_timeInSeconds;
	vector<string> m_startingPaths;

	po::options_description GetCmdOptions() const;

public:
	PatcherOptions() : m_displayHelp(false), m_patchToRealLength(false), m_lengthConditionType(condition_none),
		m_lengthCondition(120), m_timeInSeconds(105), m_startingPaths()
	{
	}

	PatcherOptions(int argc, char* argv[]);
	
	void PrintHelp(ostream& output) const;
	void DisplayHelp(bool displayHelp) { m_displayHelp = displayHelp; }
	bool DisplayHelp() const { return m_displayHelp; }
	void PatchToRealLength() { m_patchToRealLength = true; }
	bool PatchingToRealLength() const { return m_patchToRealLength; }
	void TimeInSeconds(double timeInSeconds) { m_patchToRealLength = false; m_timeInSeconds = timeInSeconds; }
	double TimeInSeconds() const { return !m_patchToRealLength ? m_timeInSeconds : -1; }
	void SetStartingPaths(const vector<string>& startingPaths) { m_startingPaths = startingPaths; }
	const vector<string>& StartingPaths() const { return m_startingPaths; }
	vector<string>& StartingPaths() { return m_startingPaths; }
	
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

	bool LengthMeetsConditions(double reportedSongLength) const;

	bool FileMeetsConditions(const string& file) const;
};

bool PatcherOptions::LengthMeetsConditions(double reportedSongLength) const
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

bool PatcherOptions::FileMeetsConditions(const string& file) const
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

void PatcherOptions::PrintHelp(ostream& output) const
{
	po::options_description desc = GetCmdOptions();
	output << desc << endl;
}

po::options_description PatcherOptions::GetCmdOptions() const
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "Show program usage information.")
		("unpatch", "Reverse the length patching process by setting the length of .ogg files to their true length. Files that do not have a reported length of 1:45 are skipped. The unpatching process is significantly slower than the patching process (3-5 seconds).")
		("patchall", "Patches all .ogg files found. If patching, this means even files shorter than 2:00 will be patched. If unpatching, even files that do not have a reported length of 1:45 will be processed.")
		("patchpaths", po::value<vector<string> >(),"Paths to the files or directories containing .ogg files to patch. Directories will be recursively searched for all .ogg files. If this option is not specified, this program's starting current working directory will be used (usually the directory this program is in).")
	;

	return desc;
}

PatcherOptions::PatcherOptions(int argc, char* argv[])
{
	po::options_description desc = GetCmdOptions();

	po::positional_options_description positionalOptions;
	positionalOptions.add("patchpaths", -1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions).run(), vm);
	po::notify(vm);

	if(vm.count("help"))
	{
		DisplayHelp(true);
	}
	else
	{
		DisplayHelp(false);
	}

	bool unpatch = vm.count("unpatch") > 0;
	bool patchall = vm.count("patchall") > 0;

	if(vm.count("patchpaths"))
	{
		SetStartingPaths(vm["patchpaths"].as<vector<string> >());
	}

	if(StartingPaths().size() == 0)
	{
		StartingPaths().push_back(fs::initial_path().file_string());
	}

	if(!unpatch)
	{
		TimeInSeconds(105);
		UseLengthGreaterThanCondition(120);
	}
	else
	{
		PatchToRealLength();
		UseLengthEqualCondition(105);
	}

	if(patchall)
	{
		DontUseLengthCondition();
	}
}

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

bool UserWantsToContinue(const PatcherOptions& options)
{
	if(options.PatchingToRealLength())
	{
		cout << "You have chosen to patch the following files and directories to the song's true length:";
	}
	else
	{
		cout << "You have chosen to patch the following files and directories to " << options.TimeInSeconds() << " seconds:";
	}

	for(vector<string>::size_type pathIndex = 0; pathIndex < options.StartingPaths().size(); pathIndex++)
	{
		cout << " " << options.StartingPaths()[pathIndex];
	}
	cout << endl << endl;

	string line;
	while(!boost::istarts_with(line, "y") && !boost::istarts_with(line, "n"))
	{
		cout << "Proceed? (y/n)" << endl;
		getline(cin, line);
	}

	if(boost::istarts_with(line, "y"))
	{
		return true;
	}
	else
	{
		return false;
	}
}

int main(int argc, char* argv[])
{
	int exitCode = 0;
	bool userChickenedOut = false;
	try
	{
		PatcherOptions options(argc, argv);
		if(options.DisplayHelp())
		{
			options.PrintHelp(cout);
			return 0;
		}

		userChickenedOut = !UserWantsToContinue(options);

		if(!userChickenedOut)
		{
			Patcher patcher(options);
			patcher.Patch();

			cout << "Done. Press enter to exit." << endl;
			string line;
			getline(cin, line);
		}
	}
	catch(std::exception& ex)
	{
		cout << ex.what() << endl;
		exitCode = 2;
	}

	return exitCode;
}
