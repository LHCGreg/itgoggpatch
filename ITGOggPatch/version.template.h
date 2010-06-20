#ifndef __ITGOP_VERSION_H__
#define __ITGOP_VERSION_H__

#ifdef _WIN32
#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif
#endif


#define ITGOP_VERSION_MAJOR 2
#define ITGOP_VERSION_MINOR 0
#define ITGOP_VERSION_FIX 0
#define ITGOP_VERSION_REVISION $revision$

#include <string>
namespace oggpatcher
{
extern const std::string g_programName;
extern const std::string g_programVersionString;
extern const std::string g_copyrightMessage;
}

#endif // end include guard
