#ifndef __PATCHER_H__
#define __PATCHER_H__

#include <string>
#include <exception>
#include "PatcherOptions.h"

namespace oggpatcher
{

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
	void LengthPatchDirectory(const std::string& directory);
	void LengthPatchFile(const std::string& file);
	void PrintError(const std::string& path, const std::exception& error);
};

} // end namespace oggpatcher

#endif // end include guard
