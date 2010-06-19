#include "stdafx.h"
#include "PatcherOptions.h"
#include <stdexcept>
#include <vector>
#include <string>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include "ogglength.h"
#include "version.h"

using namespace std;
using namespace ogglength;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace oggpatcher
{

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

void PatcherOptions::PrintVersion(ostream& output) const
{
	output << g_programName << " " << g_programVersionString << endl;
	output << g_copyrightMessage << endl;
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
		("version", "Show version number.")
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

	DisplayHelp(vm.count("help") > 0);
	DisplayVersion(vm.count("version") > 0);

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

}
