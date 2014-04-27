#include "stdafx.h"
#include <iostream>
#include <string>
#include <exception>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "PatcherOptions.h"
#include "Patcher.h"


using namespace std;
using namespace boost;


namespace oggpatcher
{
bool UserWantsToContinue(const PatcherOptions& options);
}

using namespace oggpatcher;

int main(int argc, char* argv[])
{
	int exitCode = 0;
	bool userChickenedOut = false;
	bool interactive = false;
	try
	{
		PatcherOptions options(argc, argv);
		interactive = options.Interactive();

		if(options.DisplayHelp())
		{
			options.PrintVersion(cout);
			options.PrintHelp(cout, argv[0]);
			return 0;
		}
		if(options.DisplayVersion())
		{
			options.PrintVersion(cout);
			return 0;
		}

		userChickenedOut = !UserWantsToContinue(options);

		if(!userChickenedOut)
		{
			Patcher patcher(options);
			patcher.Patch();

			if(interactive)
			{
				cout << "Done. Press enter to exit." << endl;
				string line;
				getline(cin, line);
			}
		}
	}
	catch(std::exception& ex)
	{
		cout << ex.what() << endl;
		if(interactive)
		{
			cout << "Press enter to exit." << endl;
			string line;
			getline(cin, line);
		}
		exitCode = 2;
	}

	return exitCode;
}


namespace oggpatcher
{

bool UserWantsToContinue(const PatcherOptions& options)
{
	if(!options.Interactive())
	{
		return true;
	}
	
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

} // end namespace oggpatcher

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
