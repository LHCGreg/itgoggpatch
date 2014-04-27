# Configurable make variables:
# CXX :         C++ compiler program. Default: g++

# CXXFLAGS :    Compiler flags. Default has many warnings on, -O3,
#               -o itgoggpatch
                
# includedirs : Directories to add to the include path in the form
#               -I /path/to/directory   Default: /usr/include/
                
# ogglinkage :  dynamic or static libogg, libvorbis, libvorbisfile linkage.
#               default: dynamic

# boostlinkage: dynamic or static linkage to boost libraries.
#               default: dynamic

sources = itg_ogg_patch.cpp ogglength.cpp Patcher.cpp PatcherOptions.cpp \
          utilities.cpp version.cpp

headers = ogglength.h Patcher.h PatcherOptions.h stdafx.h utilities.h \
          utilities_templates.h version.h

# Override CXXFLAGS with the make invocation if you wish
CXXFLAGS = -Wctor-dtor-privacy -Wnon-virtual-dtor -Weffc++ -Wold-style-cast \
-Woverloaded-virtual -Wall -Wextra -Wdisabled-optimization -pedantic \
-O3 \
-o itgoggpatch

# omitted -Wunreachable-code because g++ reports warnings for system headers -_-

# Default ogg linkage is dynamic. Set ogglinkage=static in the make
# invocation if you wish.
ogglinkage = dynamic

ogg_libs = -lvorbisfile -lvorbis -logg
ifeq ($(ogglinkage), dynamic)
logg = $(ogg_libs)
else
logg = -Wl,-Bstatic $(ogg_libs) -Wl,-Bdynamic
endif

# Default boost linkage is dynamic. Set boostlinkage=static in the make
# invocation if you wish.
boostlinkage = dynamic

boost_libs = -lboost_system -lboost_filesystem -lboost_program_options
ifeq ($(boostlinkage), dynamic)
lboost = $(boost_libs)
else
lboost = -Wl,-Bstatic $(boost_libs) -Wl,-Bdynamic
endif

# Set includedirs in the make invocation if your ogg/vorbis or boost headers
# are somewhere other than /usr/include.
includedirs = -I /usr/include



# This is intended to be used for release compiles, not development.
# All source files are specified at once to allow for more potential
# optimization. Because version.h is always regenerated, a full recompile
# occurs with every make invocation.
itgoggpatch : $(sources) $(headers)
	$(CXX) $(CXXFLAGS) $(includedirs) $(sources) $(logg) $(lboost)
