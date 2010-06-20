sources = itg_ogg_patch.cpp ogglength.cpp Patcher.cpp PatcherOptions.cpp \
          utilities.cpp version.cpp

headers = ogglength.h Patcher.h PatcherOptions.h stdafx.h utilities.h \
          utilities_templates.h version.h

CFLAGS = -Wctor-dtor-privacy -Wnon-virtual-dtor -Weffc++ -Wold-style-cast \
-Woverloaded-virtual -Wall -Wextra -Wdisabled-optimization -pedantic \
-O3 \
-o itgoggpatch

ogglinkage = dynamic

ogg_libs = -lvorbisfile -lvorbis -logg
ifeq ($(ogglinkage), dynamic)
logg = $(ogg_libs)
else
logg = -Wl,-Bstatic $(ogg_libs) -Wl,-Bdynamic
endif

boostlinkage = dynamic

boost_libs = -lboost_system-mt -lboost_filesystem-mt -lboost_program_options-mt
ifeq ($(boostlinkage), dynamic)
lboost = $(boost_libs)
else
lboost = -Wl,-Bstatic $(boost_libs) -Wl,-Bdynamic
endif

includedirs = -I /usr/include/

# omitting -Wunreachable-code because g++ reports warnings for system headers too...


itgoggpatch : $(sources) $(headers)
	g++ $(CFLAGS) $(includedirs) $(sources) $(logg) $(lboost)

.PHONY : version.h
version.h : 
	python ../Tools/UpdateSvnInfo.py . version.template.h version.h




