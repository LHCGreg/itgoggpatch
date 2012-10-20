#ifndef __PATCHER_OPTIONS_H__
#define __PATCHER_OPTIONS_H__

#include <vector>
#include <string>
#include <iostream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

// namespace oggpatcher is stuff specific to ITG Ogg Patcher and is not intended to be reusable.
namespace oggpatcher
{

// Represents a kind of condition an Ogg Vorbis file's length must meet in order to be processed
enum PatcherLengthCondition
{
	condition_none, // Always process the file
	condition_equal, // Process the file if its length is equal to some length
	condition_greater // Process the file if its length is greater than some length
};

class PatcherOptions
{
private:
	bool m_displayHelp;
	bool m_displayVersion;
	bool m_interactive;
	bool m_patchToRealLength;
	double m_timeInSeconds;
	PatcherLengthCondition m_lengthConditionType; // The condition type to use when deciding whether to process a file
	double m_lengthCondition; // The number of seconds corresponding to the condition
	std::vector<std::string> m_startingPaths;

	// Get the command-line options object to use for processing command-line args
	boost::program_options::options_description GetCmdOptions() const;
	
	// Get the command-line options object to use for displaying program usage
	boost::program_options::options_description GetCmdOptionsForHelp() const;

public:
	// Constructs default patcher options - patch to 105 seconds
	// Might throw boost::system::system_error if the starting CWD couldn't be determined
	PatcherOptions() : m_displayHelp(false), m_displayVersion(false), m_interactive(true),
		m_patchToRealLength(false), m_timeInSeconds(105), m_lengthConditionType(condition_none),
		m_lengthCondition(120), m_startingPaths(1, boost::filesystem::initial_path().string())
	{
	}

	// Constructs patcher options from command-line arguments
	PatcherOptions(int argc, char* argv[]);
	
	// Prints program usage information to the given output stream, and given what name to use for this program.
	void PrintHelp(std::ostream& output, const std::string& programName) const;
	// Gets or sets the DisplayHelp property - whether the program should display usage information.
	void DisplayHelp(bool displayHelp) { m_displayHelp = displayHelp; }
	bool DisplayHelp() const { return m_displayHelp; }
	// Prints program version information to the given output stream
	void PrintVersion(std::ostream& output) const;
	// Gets or sets the DisplayVersion property - whether the program should display version information
	void DisplayVersion(bool displayVersion) { m_displayVersion = displayVersion; }
	bool DisplayVersion() const { return m_displayVersion; }
	// Gets or sets the Interactive property - if false, the program should not ask for user input
	void Interactive(bool interactive) { m_interactive = interactive; }
	bool Interactive() const { return m_interactive; }
	// Set the option to patch to the song's real length
	void PatchToRealLength() { m_patchToRealLength = true; }
	// Is the option set to patch to the song's real length?
	bool PatchingToRealLength() const { return m_patchToRealLength; }
	// Set the time in seconds to patch files to
	void TimeInSeconds(double timeInSeconds) { m_patchToRealLength = false; m_timeInSeconds = timeInSeconds; }
	// Gets the time in seconds to patch files to or -1 if patching to real length
	double TimeInSeconds() const { return !m_patchToRealLength ? m_timeInSeconds : -1; }
	
	// These methods force clients to use a std::vector<std::string>
	// and exposes that this class uses a std::vector<std::string>, which kinda breaks encapsulation.
	// C++ doesn't have interface classes for its standard library containers like C# does. :(

	// Sets the paths to patch
	void SetStartingPaths(const std::vector<std::string>& startingPaths) { m_startingPaths = startingPaths; }
	// Gets the paths to patch
	const std::vector<std::string>& StartingPaths() const { return m_startingPaths; }
	// Gets the paths to patch, allowing modification
	std::vector<std::string>& StartingPaths() { return m_startingPaths; }
	
	// Only process files with length equal to the given number of seconds. Only one condition may be used.
	void UseLengthEqualCondition(double lengthCondition)
	{
		m_lengthConditionType = condition_equal;
		m_lengthCondition = lengthCondition;
	}
	// Only process files with length greater than the given number of seconds. Only one condition may be used.
	void UseLengthGreaterThanCondition(double lengthCondition)
	{
		m_lengthConditionType = condition_greater;
		m_lengthCondition = lengthCondition;
	}
	// Process all files.
	void DontUseLengthCondition()
	{
		m_lengthConditionType = condition_none;
	}

	// Gets the length condition type in use
	PatcherLengthCondition LengthConditionType() const { return m_lengthConditionType; }
	// Gets the number of seconds for the length condition. Only has meaning for conditions other than condition_none
	double LengthCondition() const { return m_lengthCondition; }

	// Returns true if a song with the given reported song length meets the conditions of this options object
	bool LengthMeetsConditions(double reportedSongLength) const;

	// Returns true if the given Ogg Vorbis file meets the conditions of this options object.
	// Can throw ogglength::OggVorbisError if there is an error opening or reading the file.
	bool FileMeetsConditions(const std::string& file) const;
};

} // end namespace oggpatcher

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
