============================================
=Requirements for running prebuilt binaries=
============================================
Windows:
(none)

Linux:
(nothing special other than libc, libc++)

======================================
=Additional requirements for building=
======================================
Python (for prebuild step)
Subversion tools. For Windows, download the Subversion installer (even if you have TortoiseSVN). For Linux, get the subversion-tools package. The svnversion program is used in a prebuild step.
A C++ compiler (MSVC 2008 is supported on Windows; g++ is supported on Linux; other compilers should work but no guarantees)
libogg, libvorbis, and libvorbisfile (On Windows, you'll have to compile them yourself; on Linux you can get the packages for them. You will need the -dev packages.)
Boost C++ libraries (http://www.boost.org/) (Windows: download from the Boost website and follow the build instructions. Linux: Get from your package manager. You will need the filesystem, system, and program options libraries, which are sometimes separated from the rest of Boost. Again, you will need the -dev packages, not just the regular packages.)

If you build your own ogg libraries, make sure you build them as optimized as possible. The MSVC project settings provided with the library source code could use some tweaking, especially for libvorbisfile. It makes a huge difference in the time taken to find the real length of a song. (~17 seconds vs. ~3 seconds).


On Windows with MSVC you can define the symbol OGG_STATIC or OGG_DYNAMIC to specify the linkage to the ogg libraries. The default is static linking. Make sure you have the ogg libraries you are linking on your library path. If linking statically, make sure the libraries and ITG Ogg Length Patch are using the same standard library versions (debug or non-debug).

On Windows, Boost is always statically linked.

On Linux, you can choose static or dynamic linking to the ogg libraries and boost by setting ogglinkage and boostlinkage in the make invocation. The default for both is dynamic linking (although packaged releases use static linking). Example:

$ make ogglinkage=static boostlinkage=dynamic


====================
=Known deficiencies=
====================
ITG Ogg Length Patch currently only works on little-endian architectures due to assuming a little-endian processor when reading various fields from an Ogg Vorbis file. This means it doesn't work on Macs. This is planned to be fixed.

Paths with non-ASCII characters in them are not supported.

Getting the real length of a song is relatively slow. Getting the real length of a song is currently implemented by actually decoding it. I am not sure if that is necessary. The Vorbis spec is scary. :(

Length-patched files seem to have about .02 seconds chopped off the end of a song for decoders using libvorbis (and maybe others?). Unpatching does not fix it. The data is still there in the file, it's just that doing an ov_read loop stops a little short of where the original file ended. I am not entirely sure why it happens, but I suspect it may have something to do with the decoder detecting that the granule position for the last vorbis page is not correct. Patching to 1:45 changes the granule position to something that is incorrect. Decoding then drops that ogg page. Since unpatching currently uses the method of decoding the file, unpatching gets a slightly incorrect length. So even after unpatching, the granule position of the last vorbis page is still incorrect.
