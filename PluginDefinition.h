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
const TCHAR NPP_PLUGIN_NAME[] = TEXT( "Changed Lines" );

//-----------------------------------------------//
//-- STEP 2. DEFINE YOUR PLUGIN COMMAND NUMBER --//
//-----------------------------------------------//
//
// Here define the number of your plugin commands
//
const int nbFunc = 17;

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
#define DEFAULTGOTOINCSAVE 1
#define DEFAULTMARGIN      2
#define DEFAULTWIDTH       9
#define DEFAULTCOLOR_REVERTED_TO_ORIGIN   0x40a0bf
#define DEFAULTCOLOR_SAVED                0x00a000
#define DEFAULTCOLOR_MODIFIED             0x0080ff
#define DEFAULTCOLOR_REVERTED_TO_MODIFIED 0xcad788
#define NUMDIGIT           64

#define N_ELEMS(x) (sizeof(x) / sizeof((x)[0]))

HWND getCurScintilla();
Sci_Position findNextMark( HWND, Sci_Position, int );
Sci_Position findPrevMark( HWND, Sci_Position, int );
void updateWidth();
void updateChangeColor();
void updateSaveColor();
void updateRevertModColor();
void updateRevertOriginColor();
void updatePanel();
void updatePosition();
void InitPlugin();
void DestroyPlugin();
void clearAllCF();
void doEnable();
void gotoLine( Sci_Position );
void gotoNextChangeAll();
void gotoNextChangeCOnly();
void gotoNextChangeSOnly();
void gotoNextChange(bool, bool);
void gotoPrevChangeAll();
void gotoPrevChangeCOnly();
void gotoPrevChangeSOnly();
void gotoPrevChange(bool, bool);
void DockableDlg();

#endif //PLUGINDEFINITION_H
