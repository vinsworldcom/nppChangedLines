//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H
#include <windows.h>
#include <tchar.h>

//
// All difinitions of plugin interface
//
#include "PluginInterface.h"

//-------------------------------------//
//-- STEP 1. DEFINE YOUR PLUGIN NAME --//
//-------------------------------------//
// Here define your plugin name
//
const TCHAR NPP_PLUGIN_NAME[] = TEXT( "Changed &Lines" );

//-----------------------------------------------//
//-- STEP 2. DEFINE YOUR PLUGIN COMMAND NUMBER --//
//-----------------------------------------------//
//
// Here define the number of your plugin commands
//
const int nbFunc = 11;

//
// Initialization of your plugin data
// It will be called while plugin loading
//
void pluginInit( HANDLE hModule );

//
// Cleaning of your plugin
// It will be called while plugin unloading
//
void pluginCleanUp();

//
//Initialization of your plugin commands
//
void commandMenuInit();

//
//Clean up your plugin commands allocation (if any)
//
void commandMenuCleanUp();

//
// Function which sets your command
//
bool setCommand( size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc,
                 ShortcutKey *sk = NULL, bool check0nInit = false );

//
// Your plugin command functions
//
#define DefaultChangeColor 0x86c2ff
#define DefaultSaveColor   0xb5ffb5
#define DefaultChangeStyle SC_MARK_FULLRECT
#define DefaultSaveStyle   SC_MARK_FULLRECT
#define DefaultGotoIncSave 0
#define DefaultMargin      4
#define DefaultWidth       4
#define DefaultArrowWidth  14
#define NUMDIGIT           64
const int MAX_MARGINS    = 10;
const int CHANGE_MARKER  = 10;
const int SAVE_MARKER    = 11;
const int CHANGE_MASK    = ( 1 << CHANGE_MARKER );
const int SAVE_MASK      = ( 1 << SAVE_MARKER );

#define N_ELEMS(x) (sizeof(x) / sizeof((x)[0]))
const int MarkTypeArr[] =
{
    SC_MARK_FULLRECT,
    SC_MARK_ARROW,
    SC_MARK_BACKGROUND
};
enum MarkType
{
    Default = 0,
    Arrow,
    Highlight
};

HWND getCurScintilla();
int findNextMark( HWND, int, int );
void updateWidth();
void updateChangeColor();
void updateSaveColor();
void updateChangeStyle();
void updateSaveStyle();
void updatePanel();
void updatePosition();
void InitPlugin();
void DestroyPlugin();
void clearAllCF();
void doEnable();
void gotoLine( int );
void gotoNextChange();
void gotoPrevChange();
void SetBookmark( HWND, int, Sci_Position );
void DelBookmark( HWND, int, Sci_Position );
void convertChangeToSave();
void DockableDlg();

#endif //PLUGINDEFINITION_H
