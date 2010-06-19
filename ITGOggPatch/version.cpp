#include "stdafx.h"
#include "version.h"
#include <string>

using namespace std;

namespace oggpatcher
{
const string g_programName = "ITG Ogg Patcher";

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

const string g_programVersionString = string(QUOTE(ITGOP_VERSION_MAJOR))
                                     + "." + QUOTE(ITGOP_VERSION_MINOR)
                                     + "." + QUOTE(ITGOP_VERSION_FIX)
                                     + "." + QUOTE(ITGOP_VERSION_REVISION);

const string g_copyrightMessage = "Copyright 2010 Greg Najda. The source code for this program is available under the Apache 2.0 license at http://code.google.com/p/itgoggpatch/.";
}
