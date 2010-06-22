==========
=Feedback=
==========

Send bug reports, comments, patches, etc to gregnajda@gmail.com


============================================
=Requirements for running prebuilt binaries=
============================================
Windows:
(none)

Linux:
libc, libc++ (what versions?)


=====================================================
=To patch .ogg files to 105 seconds (1:45) (Windows)=
=====================================================

Drag a file or folder onto ITGOggPatch.exe. If you drag a folder, the program will search the folder and all subfolders for files ending in .ogg and will patch all of them.

OR

Copy ITGOggPatch.exe to the folder you want to patch then double-click ITGOggPatch.exe.

The program will skip any files shorter than 2:00.



==========================================================
=To unpatch .ogg files to their original length (Windows)=
==========================================================

Follow the same procedure as above, but use ITGOggUnpatch.bat instead of ITGOggPatch.exe. ITGOggUnpatch.bat must be in the same folder as ITGOggPatch.exe.

The program will skip any files that are not 1:45.

This works on files patched using the original OggLengthPatch.exe.

Unpatching takes SIGNIFICANTLY longer than patching and depends on how long the song is. Determining how long the song actually is takes about 1 second for every 30 seconds of song time.


=======
=Linux=
=======

Follow the command-line use below. You will have to use itgoggpatch instead of ITGOggPatch.exe of course.



==================
=Command-line use=
==================

You can also use ITGOggPatch from the command-line. Run ITGOggPatch.exe --help to see the available options. For your convenience, here is the output of ITGOggPatch.exe --help

ITG Ogg Patcher 1.0.0.12
Copyright 2010 Greg Najda. The source code for this program is available under t
he Apache 2.0 license at http://code.google.com/p/itgoggpatch/.
Allowed options:
  --help                Show program usage information.
  --version             Show version number.
  --unpatch             Reverse the length patching process by setting the
                        length of .ogg files to their true length. Files that
                        do not have a reported length of 1:45 are skipped. The
                        unpatching process is significantly slower than the
                        patching process (3-5 seconds).
  --patchall            Patches all .ogg files found. If patching, this means
                        even files shorter than 2:00 will be patched. If
                        unpatching, even files that do not have a reported
                        length of 1:45 will be processed.
  --not-interactive     Suppresses the requests for user input when starting
                        and finishing.