#ifndef __PATCHER_H__
#define __PATCHER_H__

#include <string>
#include <exception>
#include "PatcherOptions.h"

// namespace oggpatcher is stuff specific to ITG Ogg Patcher and is not intended to be reusable.
namespace oggpatcher
{

// A Patcher takes options including a list of files/directories to patch, and can be run.
class Patcher
{
private:
	PatcherOptions m_options;

public:
	// Creates a new patcher with the given options.
	explicit Patcher(const PatcherOptions& options) : m_options(options)
	{
	}

	// Runs the patcher. No exceptions are thrown other than bad_alloc and such.
	// As such, there's no way to know how many or what types of errors occurred.
	// Errors are printed to stdout.
	void Patch();

private:
	void LengthPatchDirectory(const std::string& directory);
	void LengthPatchFile(const std::string& file);
	void PrintError(const std::string& path, const std::exception& error);
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
