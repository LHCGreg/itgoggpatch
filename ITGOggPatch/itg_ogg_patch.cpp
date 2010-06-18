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


namespace oggpatcher
{

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

} // end namespace oggpatcher
