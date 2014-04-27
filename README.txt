The original OggLengthPatch.exe is a program used modify Ogg Vorbis files to trick In The Groove machines into thinking an Ogg Vorbis file is 1:45 in length when it actually may be longer than the 2 minute song length limit. No one seems to know who wrote the original OggLengthPatch.exe and it is not open-source.

itgoggpatch aims to be an open-source equivalent so that creators of other tools can see how to change the reported length of an Ogg Vorbis file, and to improve on the original program by making it easy to patch entire song packs at once or even your entire Stepmania song collection. itgoggpatch also allows the process to be reversed so that people who play Stepmania at home can see the correct song lengths and groove radar values for songs that are distributed in length-patched form.

Downloads are available at https://github.com/LHCGreg/itgoggpatch/releases

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
(nothing special other than libc, libc++)


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

Unpatching takes longer than patching and depends on how long the song is.


=======
=Linux=
=======

Follow the command-line use below. You will have to use itgoggpatch instead of ITGOggPatch.exe of course.



==================
=Command-line use=
==================

You can also use ITGOggPatch from the command-line. Run ITGOggPatch.exe --help to see the available options. For your convenience, here is the usage information ITGOggPatch.exe --help displays

Usage: ITGOggPatch.exe [OPTIONS] [Paths to the files or directories containing .
ogg files]
Allowed options:
  --help                Show program usage information.
  --version             Show version number.
  --unpatch             Reverse the length patching process by setting the
                        length of .ogg files to their true length. Files that
                        do not have a reported length of 1:45 are skipped. The
                        unpatching process is significantly slower than the
                        patching process and depends on how long the song is.
  --patchall            Patches all .ogg files found. If patching, this means
                        even files shorter than 2:00 will be patched. If
                        unpatching, even files that do not have a reported
                        length of 1:45 will be processed.
  --not-interactive     Suppresses the requests for user input when starting
                        and finishing.