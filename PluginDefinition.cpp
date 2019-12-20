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

#include "PluginDefinition.h"
#include "menuCmdID.h"

#include "stdafx.h"
#include <string>
#include <vector>
#include <shlwapi.h>
#include "DockingFeature/PanelDlg.h"

const TCHAR sectionName[] = TEXT( "Settings" );
const TCHAR keyName[] = TEXT( "Enabled" );
const TCHAR configFileName[] = TEXT( "ChangedLines.ini" );

DemoDlg _Panel;
long ChangeColor = DefaultChangeColor;
long SaveColor   = DefaultSaveColor;

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

TCHAR iniFilePath[MAX_PATH];
bool g_enabled = false;

#define ENABLE_INDEX 0
#define DOCKABLE_INDEX 1

//
// Initialize your plugin data here
// It will be called while plugin loading
void pluginInit( HANDLE hModule )
{
    // Initialize dockable dialog
    _Panel.init( ( HINSTANCE )hModule, NULL );
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
    ::WritePrivateProfileString( sectionName, keyName,
                                 g_enabled ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
    //
    // Firstly we get the parameters from your plugin config file (if any)
    //

    // get path of plugin configuration
    ::SendMessage( nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH,
                   ( LPARAM )iniFilePath );

    // if config path doesn't exist, we create it
    if ( PathFileExists( iniFilePath ) == FALSE )
        ::CreateDirectory( iniFilePath, NULL );

    // make your plugin config file full file path name
    PathAppend( iniFilePath, configFileName );

    // get the parameter value from plugin config
    g_enabled = ::GetPrivateProfileInt( sectionName, keyName, 0,
                                        iniFilePath );

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );

    setCommand( ENABLE_INDEX, TEXT( "&Enable" ), doEnable, NULL,
                g_enabled ? true : false );
    setCommand( DOCKABLE_INDEX, TEXT( "Changed Lines Panel" ), DockableDlg,
                NULL, false );
    setCommand( 2, TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( 3, TEXT( "Goto &Next Change" ), gotoNextChange, NULL,
                false );
    setCommand( 4, TEXT( "Goto &Previous Change" ), gotoPrevChange, NULL,
                false );
    setCommand( 5, TEXT( "&Clear all in Current File" ), clearAllCF, NULL,
                false );
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
    // Don't forget to deallocate your shortcut here
}

//
// This function help you to initialize your plugin commands
//
bool setCommand( size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc,
                 ShortcutKey *sk, bool check0nInit )
{
    if ( index >= nbFunc )
        return false;

    if ( !pFunc )
        return false;

    lstrcpy( funcItem[index]._itemName, cmdName );
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
HWND getCurScintilla()
{
    int which = -1;
    ::SendMessage( nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0,
                   ( LPARAM )&which );
    return ( which == 0 ) ? nppData._scintillaMainHandle :
           nppData._scintillaSecondHandle;
}

int findNextMark( HWND hCurScintilla, int searchStart, int mask )
{
    return( ( int )::SendMessage( hCurScintilla, SCI_MARKERNEXT, searchStart, mask ) );
}

int findPrevMark( HWND hCurScintilla, int searchStart, int mask )
{
    return( ( int )::SendMessage( hCurScintilla, SCI_MARKERPREVIOUS, searchStart, mask ) );
}

void DeleteAllMarks( HWND hCurScintilla, int mask, int marker )
{
    int pos = 0;

    while ( true )
    {
        int line = findNextMark( hCurScintilla, pos, mask );
        if ( line == -1 )
            break;

        SendMessage( hCurScintilla, SCI_MARKERDELETE, line, marker );
        pos = line + 1;
    }
}

void InitPlugin()
{
    HWND ScintillaArr[] = { nppData._scintillaMainHandle, nppData._scintillaSecondHandle };

    for ( int i = 0; i < 2; i++ )
    {
        HWND hCurScintilla = ScintillaArr[i];

        SendMessage( hCurScintilla, SCI_SETMARGINTYPEN, DEFAULT_MARGIN,
                     SC_MARGIN_SYMBOL );
        SendMessage( hCurScintilla, SCI_SETMARGINWIDTHN, DEFAULT_MARGIN,
                     DEFAULT_WIDTH );

        int OriMask = ( int )::SendMessage( hCurScintilla, SCI_GETMARGINMASKN,
                                            DEFAULT_MARGIN, 0 );
        int tmpMask = 0;
        tmpMask = OriMask | CHANGE_MASK | SAVE_MASK;
        SendMessage( hCurScintilla, SCI_SETMARGINMASKN, DEFAULT_MARGIN, tmpMask );

        SendMessage( hCurScintilla, SCI_MARKERSETFORE, CHANGE_MARKER, ChangeColor );
        SendMessage( hCurScintilla, SCI_MARKERSETFORE, SAVE_MARKER, SaveColor );
        SendMessage( hCurScintilla, SCI_MARKERSETBACK, CHANGE_MARKER, ChangeColor );
        SendMessage( hCurScintilla, SCI_MARKERSETBACK, SAVE_MARKER, SaveColor );

        SendMessage( hCurScintilla, SCI_MARKERDEFINE, CHANGE_MARKER,
                     SC_MARK_FULLRECT );
        SendMessage( hCurScintilla, SCI_MARKERDEFINE, SAVE_MARKER,
                     SC_MARK_FULLRECT );
    }
}

void DestroyPlugin()
{

    HWND ScintillaArr[] = { nppData._scintillaMainHandle, nppData._scintillaSecondHandle };

    for ( int i = 0; i < 2; i++ )
    {
        HWND hCurScintilla = ScintillaArr[i];

        DeleteAllMarks( hCurScintilla, CHANGE_MASK, CHANGE_MARKER );
        DeleteAllMarks( hCurScintilla, SAVE_MASK, SAVE_MARKER );

        SendMessage( hCurScintilla, SCI_SETMARGINTYPEN, DEFAULT_MARGIN, 0 );
        SendMessage( hCurScintilla, SCI_SETMARGINWIDTHN, DEFAULT_MARGIN, 0 );
    }
}

void clearAllCF()
{
    HWND hCurScintilla = getCurScintilla();

    DeleteAllMarks( hCurScintilla, CHANGE_MASK, CHANGE_MARKER );
    DeleteAllMarks( hCurScintilla, SAVE_MASK, SAVE_MARKER );
}

void doEnable()
{
    UINT state = ::GetMenuState( ::GetMenu( nppData._nppHandle ),
                                 funcItem[ENABLE_INDEX]._cmdID,
                                 MF_BYCOMMAND );

    if ( state & MF_CHECKED )
    {
        g_enabled = 0;
        DestroyPlugin();
    }
    else
    {
        g_enabled = 1;
        InitPlugin();
    }

    ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                   funcItem[ENABLE_INDEX]._cmdID, !( state & MF_CHECKED ) );
}

void gotoNextChange()
{
    HWND hCurScintilla = getCurScintilla();

    int pos = ( int )::SendMessage( hCurScintilla, SCI_GETCURRENTPOS, 0, 0 );
    int searchStart = ( int )::SendMessage( hCurScintilla, SCI_LINEFROMPOSITION, pos, 0 );

    int line = findNextMark( hCurScintilla, searchStart+1, CHANGE_MASK );
    if ( line == -1 )
        line = findNextMark( hCurScintilla, 0, CHANGE_MASK );

    if ( line != -1 )
        SendMessage( hCurScintilla, SCI_GOTOLINE, line, 0 );
}

void gotoPrevChange()
{
    HWND hCurScintilla = getCurScintilla();

    int pos = ( int )::SendMessage( hCurScintilla, SCI_GETCURRENTPOS, 0, 0 );
    int searchStart = ( int )::SendMessage( hCurScintilla, SCI_LINEFROMPOSITION, pos, 0 );

    int line = findPrevMark( hCurScintilla, searchStart-1, CHANGE_MASK );
    if ( line == -1 ){
        int end = ( int )::SendMessage( hCurScintilla, SCI_GETLINECOUNT , 0, 0 );
        line = findPrevMark( hCurScintilla, end, CHANGE_MASK );
    }

    if ( line != -1 )
        SendMessage( hCurScintilla, SCI_GOTOLINE, line, 0 );
}

////////////////////////////////////////////////////////////////////////////
///
/// Dockable dialog:
///
void DockableDlg()
{
    _Panel.setParent( nppData._nppHandle );
    tTbData data = {0};

    if ( !_Panel.isCreated() )
    {
        _Panel.create( &data );

        // define the default docking behaviour
        data.uMask = DWS_DF_CONT_LEFT | DWS_ICONTAB;

        data.hIconTab = ( HICON )::LoadImage( _Panel.getHinst(),
                                              MAKEINTRESOURCE( IDI_PLUGINPANEL ), IMAGE_ICON, 0, 0,
                                              LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT );
        data.pszModuleName = _Panel.getPluginFileName();

        // the dlgDlg should be the index of funcItem where the current function pointer is
        // in this case is DOCKABLE_INDEX
        data.dlgID = DOCKABLE_INDEX;
        ::SendMessage( nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0,
                       ( LPARAM )&data );
    }

    UINT state = ::GetMenuState( ::GetMenu( nppData._nppHandle ),
                                 funcItem[DOCKABLE_INDEX]._cmdID, MF_BYCOMMAND );

    if ( state & MF_CHECKED )
        _Panel.display( false );
    else
        _Panel.display();

    ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                   funcItem[DOCKABLE_INDEX]._cmdID, !( state & MF_CHECKED ) );
}
