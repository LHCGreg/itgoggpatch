#ifndef __PATCHER_OPTIONS_H__
#define __PATCHER_OPTIONS_H__

#include <vector>
#include <string>
#include <iostream>

namespace oggpatcher
{

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
	std::vector<std::string> m_startingPaths;

	boost::program_options::options_description GetCmdOptions() const;

public:
	PatcherOptions() : m_displayHelp(false), m_patchToRealLength(false), m_lengthConditionType(condition_none),
		m_lengthCondition(120), m_timeInSeconds(105), m_startingPaths()
	{
	}

	PatcherOptions(int argc, char* argv[]);
	
	void PrintHelp(std::ostream& output) const;
	void DisplayHelp(bool displayHelp) { m_displayHelp = displayHelp; }
	bool DisplayHelp() const { return m_displayHelp; }
	void PatchToRealLength() { m_patchToRealLength = true; }
	bool PatchingToRealLength() const { return m_patchToRealLength; }
	void TimeInSeconds(double timeInSeconds) { m_patchToRealLength = false; m_timeInSeconds = timeInSeconds; }
	double TimeInSeconds() const { return !m_patchToRealLength ? m_timeInSeconds : -1; }
	void SetStartingPaths(const std::vector<std::string>& startingPaths) { m_startingPaths = startingPaths; }
	const std::vector<std::string>& StartingPaths() const { return m_startingPaths; }
	std::vector<std::string>& StartingPaths() { return m_startingPaths; }
	
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

	bool FileMeetsConditions(const std::string& file) const;
};

} // end namespace oggpatcher

#endif // end include guard