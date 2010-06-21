#include "stdafx.h"
#include "version.h"
#include <string>

using namespace std;

namespace oggpatcher
{
const string g_programName = "ITG Ogg Patcher";

// These macros get the version macros into "" quotes so they can be treated as strings.
#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

const string g_programVersionString = string(QUOTE(ITGOP_VERSION_MAJOR))
                                     + "." + QUOTE(ITGOP_VERSION_MINOR)
                                     + "." + QUOTE(ITGOP_VERSION_FIX)
                                     + "." + QUOTE(ITGOP_VERSION_REVISION);

const string g_copyrightMessage = "Copyright 2010 Greg Najda. The source code for this program is available under the Apache 2.0 license at http://code.google.com/p/itgoggpatch/.";
}

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
