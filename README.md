# Notepad++ Changed Lines

Author:  Michael J. Vincent

## Description

Taken from Location Navigate:  https://sourceforge.net/projects/locationnav/

This plugin basically replicates only the change markings in the margin and 
does not try to store the places in the document where navigated to.  This 
was causing crashes for me in 32-bit and I had to build my own 64-bit version 
which did not work well at all.  Hence, this plugin which takes the change 
marking logic and does just that.

## Compiling

I compiled with MS Visual Studio Community 2017 and this seems to work OK.

For 32-bit:
```
    [x86 Native Tools Command Prompt for VS 2017]
    C:\> set Configuration=Release
    C:\> set Platform=x86
    C:\> msbuild
```

For 64-bit:
```
    [x64 Native Tools Command Prompt for VS 2017]
    C:\> set Configuration=Release
    C:\> set Platform=x64
    C:\> msbuild
```

## Installation

Copy the:

+ 32-bit:  ./bin/ChangedLines.dll
+ 64-bit:  ./bin64/ChangedLines.dll

to the Notepad++ plugins folder:
  + In N++ <7.6, directly in the plugins/ folder
  + In N++ >=7.6, in a directory called ChangedLines in the plugins/ folder (plugins/ChangedLines/)

Make sure to enable from the menu Plugins->Changed Lines->Enable.
