# Notepad++ Changed Lines

Author:  Michael J. Vincent


## Description

Taken from Location Navigate:  https://sourceforge.net/projects/locationnav/

### Original

This plugin basically replicates the change markings in the margin and
a simple circular stack to track places in the document where navigated to. 
The original Location Navigate plugin was causing crashes for me in 32-bit 
and I had to build my own 64-bit version which did not work well at all. 
Hence, this plugin which takes the change marking logic and does just that.

### Current

As of [Notepad++ 8.4.6](https://notepad-plus-plus.org/news/v846-released/), 
native [Scintilla Change History](https://www.scintilla.org/ScintillaDoc.html#ChangeHistory) 
has been incorporated and rendered this plugin basically redundant at best. 
I have removed the plugin change tracking component and instead added ability 
to customize the native Notepad++ Change History.

The `Enable Plugin Features` only enables / disables the plugin features, it 
does **NOT** turn on / off the Notepad++ Change History feature.

## Compiling

I compiled with MS Visual Studio Community 2017 and this seems to work OK.

For 32-bit:
```
    [x86 Native Tools Command Prompt for VS 2017]
    C:\> set Configuration=Release
    C:\> set Platform=Win32
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

**NOTE:**  This plugin only works with Notepad++ 8.4.6 or newer.

Copy the:

+ 32-bit:  ./Release/Win32/ChangedLines.dll
+ 64-bit:  ./Release/x64/ChangedLines.dll

to the Notepad++ plugins folder:
  + In N++ >=7.6, in a directory called ChangedLines in the plugins/ folder (plugins/ChangedLines/)

Make sure to enable from the menu Plugins->Changed Lines->Enable Plugin Features.


## Usage

As of Notepad++ 8.4.6, Scintilla Change History provides margin markers to 
track changes in documents while editing.  This plugin allows some 
customization of the Change History feature and provides a panel to visually 
see the changed lines.

As you navigate through opened Notepad++ documents, ChangedLines will record 
the document name and line number and store that in a previously visited 
location stack.  This is done based on "lingering" in a document position long 
enough for [SCN_MODIFIED](https://www.scintilla.org/ScintillaDoc.html#SCN_MODIFIED) 
to fire and the linger timer (2 seconds) to elapse.  This is not perfect, but 
it suits my needs 90% of the time. 


### Navigation

There are plugin menu options, shortcuts and a docking panel to help you 
navigate through your changes.

+ **Shortcut keys** - by default, `CTRL+ALT+Z` and `CTRL+ALT+Y` are mapped to the previous and next document positions respectively.  This can be changed with Notepad++ Settings => Shortcut Mapper...
+ **Margin**
    + Double-clicking:
        + Next Change = double-clicking
        + Previous Change = SHIFT+double-click
        + Next Change (changes only) = CTRL+double-click
        + Previous Change (changes only) = SHIFT+CTRL+double-click
        + Next Change (saves only) = ALT+double-click
        + Previous Change (saves only) = SHIFT+ALT+double-click
    + Right-click:
        Popup menu for change navigation


### Docking Panel

There is a Notepad++ docking panel that shows a list of the changes in the 
current document.  Double-clicking, `Enter` or `Spacebar` will go to the 
beginning of selected line.  Escape will switch focus to the current document 
at the current position without navigating.

The "Previous Position" and "Next Position" toolbar buttons will move to the 
previous and next saved document positions respectively. 


### Limitations

+ Position tracking is not perfect!
    + It will only track across saved files (i.e., not "new 1" unsaved documents).
    + It will not remove closed files - so navigating to a position of a previously opened but not closed file will reopen the file (providing it still exists).
    + As lines are inserted and deleted, previously saved positions are **not** updated so while you will return to the exact line number saved, it may no longer be the line you think it is.
