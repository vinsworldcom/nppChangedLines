NAME

Notepad++ Changed Lines
Author:  Michael J. Vincent


DESCRIPTION

Taken from Location Navigate
https://sourceforge.net/projects/locationnav/


COMPILING

I compiled with MS Visual Studio Community 2017 and this seems to work 
OK.

For 32-bit:
  [x86 Native Tools Command Prompt for VS 2017]
  C:\> set Configuration=Release
  C:\> set Platform=x86
  C:\> msbuild

For 64-bit:
  [x64 Native Tools Command Prompt for VS 2017]
  C:\> set Configuration=Release
  C:\> set Platform=x64
  C:\> msbuild

INSTALLATION

Copy the:

32-bit:
    ./bin/ChangedLines.dll
   
64-bit:
    ./bin64/ChangedLines.dll

to the Notepad++ plugins folder:
  - In N++ <7.6, directly in the plugins/ folder
  - In N++ >=7.6, in a directory called ChangedLines in the plugins/ folder
    (plugins/ChangedLines/)
