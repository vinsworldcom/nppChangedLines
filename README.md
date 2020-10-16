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


## Usage

ChangedLines provides a margin marker to indicate what lines have been changed and what changed lines have been saved.  As changes are made to a document in Notepad++, ChangedLines will add the appropriate color marking to the line.  The colors and markers can be changed in the Settings dialog.

As you navigate through opened Notepad++ documents, ChangedLines will record the document name and line number and store that in a previously visited location stack.  This is done based on "lingering" in a document position long enough for [SCN_MODIFIED](https://www.scintilla.org/ScintillaDoc.html#SCN_MODIFIED) to fire and the linger timer (2 seconds) to elapse.  This is not perfect, but it suits my needs 90% of the time.


### Navigation

There are plugin menu options, shortcuts and a docking panel to help you navigate through your changes.


+ **Shortcut keys** - by default, `CTRL+ALT+Z` and `CTRL+ALT+Y` are mapped to the previous and next document positions respectively.  This can be changed with Notepad++ Settings => Shortcut Mapper...

+ **Margin** - Within the current document, double-clicking the ChangedLines margin will move to the next change.  Shift+double-click will move to the previous.


### Docking Panel

There is a Notepad++ docking panel that shows a list of the changes in the current document.  Double-clicking, `Enter` or `Spacebar` will go to the beginning of selected line.  Escape will switch focus to the current document at the current position without navigating.

The "Previous Position" and "Next Position" toolbar buttons will move to the previous and next saved document positions respectively.


### Limitations

+ Notepad++ File => Save All will not result in change to saved marker updates in files other than the currently active buffer

+ Position tracking is not perfect!

    + It will only track across saved files (i.e., not "new 1" unsaved documents).

    + It will not remove closed files - so navigating to a position of a previously opened but not closed file will reopen the file (providing it still exists).

    + As lines are inserted and deleted, previously saved positions are **not** updated so while you will return to the exact line number saved, it may no longer be the line you think it is.
