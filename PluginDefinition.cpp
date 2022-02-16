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
#include "resource.h"
#include "CircularStackLinkList.h"
#include "DockingFeature/PanelDlg.h"
#include "DockingFeature/SettingsDlg.h"
#include "menuCmdID.h"
#include "stdafx.h"

#include <string>
#include <vector>
#include <shlwapi.h>

#ifdef _DEBUG
#include <sstream>
std::wstringstream debugString;
#endif

const TCHAR configFileName[]     = TEXT( "ChangedLines.ini" );
const TCHAR sectionName[]        = TEXT( "Settings" );
const TCHAR iniKeyEnabled[]      = TEXT( "Enabled" );
const TCHAR iniKeyRaisePanel[]   = TEXT( "RaisePanelorToggle" );
const TCHAR iniKeyGotoIncSave[]  = TEXT( "GotoIncludeSave" );
const TCHAR iniKeyMargin[]       = TEXT( "Margin" );
const TCHAR iniKeyWidth[]        = TEXT( "Width" );
const TCHAR iniKeyColorChange[]  = TEXT( "ColorChange" );
const TCHAR iniKeyColorSave[]    = TEXT( "ColorSave" );
const TCHAR iniKeyStyleChange[]  = TEXT( "StyleChange" );
const TCHAR iniKeyStyleSave[]    = TEXT( "StyleSave" );
const TCHAR iniKeyMarkIdChange[] = TEXT( "MarkIdChange" );
const TCHAR iniKeyMarkIdSave[]   = TEXT( "MarkIdSave" );
const TCHAR iniKeyUseNppColors[] = TEXT( "UseNppCStyle" );

DemoDlg _Panel;
toolbarIcons g_TBCL;
//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;
HINSTANCE g_hInst;

TCHAR iniFilePath[MAX_PATH];
bool g_NppReady        = false;
bool g_RaisePanel      = false;
bool g_enabled         = true;
bool g_GotoIncSave     = DEFAULTGOTOINCSAVE;
int  g_Margin          = DEFAULTMARGIN;
int  g_Width           = DEFAULTWIDTH;
long g_ChangeColor     = DEFAULTCHANGECOLOR;
long g_SaveColor       = DEFAULTSAVECOLOR;
int  g_ChangeStyle     = DEFAULTCHANGESTYLE;
int  g_SaveStyle       = DEFAULTSAVESTYLE;
int  g_ChangeMarkId    = DEFAULTCHANGEMARKER;
int  g_SaveMarkId      = DEFAULTSAVEMARKER;
bool g_useNppColors    = false;
int  g_ChangeMask      = ( 1 << g_ChangeMarkId );
int  g_SaveMask        = ( 1 << g_SaveMarkId );

#define TIMER_POS       2
#define TIMER_POS_DELAY 2000

extern circular_buffer<tDocPos> prevPos;
extern circular_buffer<tDocPos> nextPos;
tDocPos lastPos = {};

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
    TCHAR buf[NUMDIGIT];

    ::WritePrivateProfileString( sectionName, iniKeyEnabled,
                                 g_enabled ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );
    ::WritePrivateProfileString( sectionName, iniKeyGotoIncSave,
                                 g_GotoIncSave ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );
    _itot_s( g_Margin, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyMargin, buf,
                                 iniFilePath );
    _itot_s( g_Width, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyWidth, buf,
                                 iniFilePath );
    _itot_s( g_ChangeColor, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyColorChange, buf,
                                 iniFilePath );
    _itot_s( g_SaveColor, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyColorSave, buf,
                                 iniFilePath );
    _itot_s( g_ChangeStyle, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyStyleChange, buf,
                                 iniFilePath );
    _itot_s( g_SaveStyle, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyStyleSave, buf,
                                 iniFilePath );
    _itot_s( g_ChangeMarkId, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyMarkIdChange, buf,
                                 iniFilePath );
    _itot_s( g_SaveMarkId, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyMarkIdSave, buf,
                                 iniFilePath );
    ::WritePrivateProfileString( sectionName, iniKeyUseNppColors,
                                 g_useNppColors ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );
    ::WritePrivateProfileString( sectionName, iniKeyRaisePanel,
                                 g_RaisePanel ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );

    if (g_TBCL.hToolbarBmp) {
        ::DeleteObject(g_TBCL.hToolbarBmp);
        g_TBCL.hToolbarBmp = nullptr;
    }
    if (g_TBCL.hToolbarIcon) {
        ::DestroyIcon(g_TBCL.hToolbarIcon);
        g_TBCL.hToolbarIcon = nullptr;
    }
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
    g_enabled         = ::GetPrivateProfileInt( sectionName, iniKeyEnabled, 1,
                        iniFilePath );
    g_GotoIncSave     = ::GetPrivateProfileInt( sectionName, iniKeyGotoIncSave, 0,
                        iniFilePath );
    g_Margin          = ::GetPrivateProfileInt( sectionName, iniKeyMargin,
                        DEFAULTMARGIN, iniFilePath );
    if ( g_Margin >= MAX_MARGINS )
        g_Margin = DEFAULTMARGIN;
    g_Width           = ::GetPrivateProfileInt( sectionName, iniKeyWidth,
                        DEFAULTWIDTH, iniFilePath );
    g_ChangeColor     = ::GetPrivateProfileInt( sectionName, iniKeyColorChange,
                        DEFAULTCHANGECOLOR, iniFilePath );
    g_SaveColor       = ::GetPrivateProfileInt( sectionName, iniKeyColorSave,
                        DEFAULTSAVECOLOR, iniFilePath );
    g_ChangeStyle     = ::GetPrivateProfileInt( sectionName, iniKeyStyleChange,
                        DEFAULTCHANGESTYLE, iniFilePath );
    g_SaveStyle       = ::GetPrivateProfileInt( sectionName, iniKeyStyleSave,
                        DEFAULTSAVESTYLE, iniFilePath );
    g_ChangeMarkId    = ::GetPrivateProfileInt( sectionName, iniKeyMarkIdChange,
                        DEFAULTCHANGEMARKER, iniFilePath );
    g_SaveMarkId      = ::GetPrivateProfileInt( sectionName, iniKeyMarkIdSave,
                        DEFAULTSAVEMARKER, iniFilePath );
    g_useNppColors    = ::GetPrivateProfileInt( sectionName, iniKeyUseNppColors,
                                                0, iniFilePath );
    g_RaisePanel      = ::GetPrivateProfileInt( sectionName, iniKeyRaisePanel,
                                                0, iniFilePath );

    g_ChangeMask    = ( 1 << g_ChangeMarkId );
    g_SaveMask      = ( 1 << g_SaveMarkId );

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
    ShortcutKey *PreChgKey  = new ShortcutKey;
    PreChgKey->_isAlt       = true;
    PreChgKey->_isCtrl      = true;
    PreChgKey->_isShift     = false;
    PreChgKey->_key         = 0x5A ; //VK_Z

    ShortcutKey *NextChgKey = new ShortcutKey;
    NextChgKey->_isAlt      = true;
    NextChgKey->_isCtrl     = true;
    NextChgKey->_isShift    = false;
    NextChgKey->_key        = 0x59;  //VK_Y

    setCommand( ENABLE_INDEX, TEXT( "&Enable" ), doEnable, NULL,
                g_enabled ? true : false );
    setCommand( DOCKABLE_INDEX, TEXT( "Changed &Lines Panel" ), DockableDlg,
                NULL, false );
    setCommand( 2, TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( 3, TEXT( "Goto &Next Change" ), gotoNextChange, NULL,
                false );
    setCommand( 4, TEXT( "Goto &Previous Change" ), gotoPrevChange, NULL,
                false );
    setCommand( 5, TEXT( "&Clear all in Current File" ), clearAllCF, NULL,
                false );
    setCommand( 6, TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( 7, TEXT( "Goto Next Position" ), gotoNextPos, NextChgKey,
                false );
    setCommand( 8, TEXT( "Goto Previous Position" ), gotoPrevPos, PreChgKey,
                false );
    setCommand( 9, TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( 10, TEXT( "&Settings" ), doSettings, NULL,
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

Sci_Position findNextMark( HWND hCurScintilla, Sci_Position searchStart, int mask )
{
    return ( ( int )::SendMessage( hCurScintilla, SCI_MARKERNEXT, searchStart,
                                   mask ) );
}

int findPrevMark( HWND hCurScintilla, int searchStart, int mask )
{
    return ( ( int )::SendMessage( hCurScintilla, SCI_MARKERPREVIOUS,
                                   searchStart, mask ) );
}

void UpdatePlugin( UINT Msg, WPARAM wParam, LPARAM lParam )
{
    HWND ScintillaArr[] = { nppData._scintillaMainHandle, nppData._scintillaSecondHandle };

    for ( int i = 0; i < 2; i++ )
    {
        HWND hCurScintilla = ScintillaArr[i];
        SendMessage( hCurScintilla, Msg, wParam, lParam );
    }
}

void updateWidth()
{
    UpdatePlugin( SCI_SETMARGINWIDTHN, g_Margin, g_Width );
}

void updateChangeColor()
{
    UpdatePlugin( SCI_MARKERSETFORE, g_ChangeMarkId, g_ChangeColor );
    UpdatePlugin( SCI_MARKERSETBACK, g_ChangeMarkId, g_ChangeColor );
}

void updateSaveColor()
{
    UpdatePlugin( SCI_MARKERSETFORE, g_SaveMarkId, g_SaveColor );
    UpdatePlugin( SCI_MARKERSETBACK, g_SaveMarkId, g_SaveColor );
}

void updateChangeStyle()
{
    UpdatePlugin( SCI_MARKERDEFINE, g_ChangeMarkId, g_ChangeStyle );

}

void updateSaveStyle()
{
    UpdatePlugin( SCI_MARKERDEFINE, g_SaveMarkId, g_SaveStyle );
}

void updatePanel()
{
    if ( _Panel.isVisible() )
        _Panel.updateListTimer();
}

void posTimerproc( HWND /*Arg1*/, UINT /*Arg2*/, UINT_PTR /*Arg3*/, DWORD /*Arg4*/)
{
    KillTimer( nppData._nppHandle, TIMER_POS );
    tDocPos x = getCurrentPos();

    if ( !_tcscmp( lastPos.docName, x.docName ) )
    {
        Sci_Position lines = ( Sci_Position )::SendMessage( getCurScintilla(), SCI_LINESONSCREEN, 0, 0 );
        Sci_Position plusMinus = Sci_Position( lines / 2 );
        if (( lastPos.lineNo > x.lineNo - plusMinus ) &&
            ( lastPos.lineNo < x.lineNo + plusMinus ))
            return;
    }

#ifdef _DEBUG
    debugString << "TIMER - put:" << x.docName << ":" << x.lineNo << std::endl;
    OutputDebugString( debugString.str().c_str() );
    debugString.str(TEXT("")); debugString.clear();
#endif
    prevPos.timerPut( x );
    lastPos = x;
}

void updatePosTimer()
{
    KillTimer( nppData._nppHandle, TIMER_POS );
    SetTimer( nppData._nppHandle, TIMER_POS, TIMER_POS_DELAY, ( TIMERPROC )posTimerproc );
}

void updatePosition()
{
    updatePosTimer();
}

void InitPlugin()
{
    HWND ScintillaArr[] = { nppData._scintillaMainHandle, nppData._scintillaSecondHandle };

    int margins = ( int )::SendMessage( getCurScintilla(), SCI_GETMARGINS, 0, 0 );
    if ( margins <= g_Margin )
        UpdatePlugin( SCI_SETMARGINS, ( g_Margin + 1 ), 0 );

    for ( int i = 0; i < 2; i++ )
    {
        HWND hCurScintilla = ScintillaArr[i];

        SendMessage( hCurScintilla, SCI_SETMARGINTYPEN, g_Margin,
                     SC_MARGIN_SYMBOL );
      
        // Mask
        int OriMask = ( int )::SendMessage( hCurScintilla, SCI_GETMARGINMASKN,
                                            g_Margin, 0 );
        int tmpMask = 0;
        tmpMask = OriMask | g_ChangeMask | g_SaveMask;
        SendMessage( hCurScintilla, SCI_SETMARGINMASKN, g_Margin, tmpMask );
        SendMessage( hCurScintilla, SCI_SETMARGINSENSITIVEN, g_Margin, true );
    }

    updateWidth();
    updateChangeColor();
    updateSaveColor();
    updateChangeStyle();
    updateSaveStyle();
}

void DestroyPlugin()
{
    HWND ScintillaArr[] = { nppData._scintillaMainHandle, nppData._scintillaSecondHandle };
    char currDoc[MAX_PATH];
    SendMessage( nppData._nppHandle, NPPM_GETFULLCURRENTPATH, ( WPARAM ) MAX_PATH, ( LPARAM) currDoc );

    for ( int i = 0; i < 2; i++ )
    {
        HWND hCurScintilla = ScintillaArr[i];

        SendMessage( hCurScintilla, SCI_MARKERDELETEALL, g_ChangeMarkId, 0 );
        SendMessage( hCurScintilla, SCI_MARKERDELETEALL, g_SaveMarkId, 0 );

        SendMessage( hCurScintilla, SCI_SETMARGINTYPEN, g_Margin, 0 );
        SendMessage( hCurScintilla, SCI_SETMARGINWIDTHN, g_Margin, 0 );
    }

    // Get open files
    TCHAR  **buffer;
    long filecount = ( long )::SendMessage( nppData._nppHandle, NPPM_GETNBOPENFILES, 0,
                                    ( LPARAM )ALL_OPEN_FILES );
    buffer = new TCHAR*[filecount];

    for ( int i = 0; i < filecount; i++ )
        buffer[i] = new TCHAR[MAX_PATH];

    SendMessage( nppData._nppHandle, NPPM_GETOPENFILENAMES, ( WPARAM )buffer,
                   ( LPARAM )filecount );

    for ( int i = 0; i < filecount; i++ )
    {
        SendMessage( nppData._nppHandle, NPPM_DOOPEN, 0, ( LPARAM )buffer[i] );
        SendMessage( ScintillaArr[0], SCI_MARKERDELETEALL, g_ChangeMarkId, 0 );
        SendMessage( ScintillaArr[0], SCI_MARKERDELETEALL, g_SaveMarkId, 0 );
    }

    // Cleanup
    for ( int i = 0; i < filecount; i++ )
        delete []buffer[i];
    delete []buffer;

    SendMessage( nppData._nppHandle, NPPM_DOOPEN, 0, ( LPARAM ) currDoc );
}

void clearAllCF()
{
    HWND hCurScintilla = getCurScintilla();

    SendMessage( hCurScintilla, SCI_MARKERDELETEALL, g_ChangeMarkId, 0 );
    SendMessage( hCurScintilla, SCI_MARKERDELETEALL, g_SaveMarkId, 0 );
}

void doEnable()
{
    // UINT state = ::GetMenuState( ::GetMenu( nppData._nppHandle ),
                                 // funcItem[ENABLE_INDEX]._cmdID,
                                 // MF_BYCOMMAND );

    if ( g_enabled )
    {
        g_enabled = 0;
        DestroyPlugin();
        ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                       funcItem[ENABLE_INDEX]._cmdID, MF_UNCHECKED );
    }
    else
    {
        g_enabled = 1;
        InitPlugin();
        ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                       funcItem[ENABLE_INDEX]._cmdID, MF_CHECKED );
    }

}

void gotoLine(Sci_Position line)
{
    HWND hCurScintilla = getCurScintilla();

    ::SendMessage( hCurScintilla, SCI_SETVISIBLEPOLICY, CARET_JUMPS | CARET_EVEN, 0 );
    ::SendMessage( hCurScintilla, SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0 );

    ::SendMessage( hCurScintilla, SCI_GOTOLINE, line, 0 );

    ::SendMessage( hCurScintilla, SCI_SETVISIBLEPOLICY, CARET_EVEN, 0 );
    ::SendMessage( hCurScintilla, SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0 );
}

void gotoNextChange()
{
    HWND hCurScintilla = getCurScintilla();

    Sci_Position line = 0;
    Sci_Position pos = ( Sci_Position )::SendMessage( hCurScintilla, SCI_GETCURRENTPOS, 0, 0 );
    Sci_Position searchStart = ( Sci_Position )::SendMessage( hCurScintilla, SCI_LINEFROMPOSITION,
                                            pos, 0 );

    int mask = g_ChangeMask;
    if ( g_GotoIncSave )
        mask |= g_SaveMask;
    while ( true )
    {
        line = findNextMark( hCurScintilla, searchStart + 1, mask );

        if ( line == -1 )
            break;

        if ( line == searchStart + 1 )
            searchStart++;
        else
            break;
    }

    if ( line == -1 )
        line = findNextMark( hCurScintilla, 0, mask );

    if ( line != -1 )
        gotoLine( line );
}

void gotoPrevChange()
{
    HWND hCurScintilla = getCurScintilla();

    int line = 0;
    int pos = ( int )::SendMessage( hCurScintilla, SCI_GETCURRENTPOS, 0, 0 );
    int searchStart = ( int )::SendMessage( hCurScintilla, SCI_LINEFROMPOSITION,
                                            pos, 0 );

    int mask = g_ChangeMask;
    if ( g_GotoIncSave )
        mask |= g_SaveMask;
    while ( true )
    {
        line = findPrevMark( hCurScintilla, searchStart - 1, mask );

        if ( line == -1 )
            break;

        if ( line == searchStart - 1 )
            searchStart--;
        else
            break;
    }

    if ( line == -1 )
    {
        int end = ( int )::SendMessage( hCurScintilla, SCI_GETLINECOUNT, 0, 0 );
        line = findPrevMark( hCurScintilla, end, mask );
    }

    if ( line != -1 )
       gotoLine( line );
}

int AddMarkFromLine( HWND hCurScintilla, Sci_Position line )
{
    int markHandle = -1;
    int state = ( int )::SendMessage( hCurScintilla, SCI_MARKERGET, line, 0 );

    if ( state == g_SaveMask )
        SendMessage( hCurScintilla, SCI_MARKERDELETE, line, g_SaveMarkId );
    else if ( state != 0 )
        return markHandle;

    markHandle = ( int )::SendMessage( hCurScintilla, SCI_MARKERADD, line,
                                       g_ChangeMarkId );

    return markHandle;
}

void SetBookmark( HWND hCurScintilla, Sci_Position lineNo, Sci_Position linesAdded )
{
    int handle = AddMarkFromLine( hCurScintilla, lineNo );

    if ( handle == -1 && linesAdded == 0 )
        return;

    if ( linesAdded > 0 )
    {
        for ( Sci_Position line = 1; line <= linesAdded; line++ )
            AddMarkFromLine( hCurScintilla, lineNo + line );
    }
}

bool RemoveMarkFromLine( HWND hCurScintilla, Sci_Position line )
{
    int state = ( int )::SendMessage( hCurScintilla, SCI_MARKERGET, line, 0 );

    if ( state == 0 )
        return false;

    if ( state == g_SaveMask )
        SendMessage( hCurScintilla, SCI_MARKERDELETE, line, g_SaveMarkId );
    else
        SendMessage( hCurScintilla, SCI_MARKERDELETE, line, g_ChangeMarkId );

    return true;
}

void DelBookmark( HWND hCurScintilla, Sci_Position lineNo, Sci_Position lineAdd )
{
    bool canUndoFlag = SendMessage( hCurScintilla, SCI_CANUNDO, 0, 0 );

    if ( !canUndoFlag )
    {
        SendMessage( hCurScintilla, SCI_MARKERDELETEALL, g_ChangeMarkId, 0 );
        SendMessage( hCurScintilla, SCI_MARKERDELETEALL, g_SaveMarkId, 0 );
        return;
    }

    Sci_Position lineB = lineNo;
    Sci_Position lineE = lineNo + lineAdd;

    if ( lineAdd < 0 )
        lineE = lineNo - lineAdd;

    for ( Sci_Position line = lineB; line <= lineE; line++ )
        RemoveMarkFromLine( hCurScintilla, line );
}

void convertChangeToSave()
{
    HWND hCurScintilla = getCurScintilla();

    Sci_Position pos = 0;

    while ( true )
    {
        Sci_Position line = findNextMark( hCurScintilla, pos, g_ChangeMask );

        if ( line == -1 )
            break;

        SendMessage( hCurScintilla, SCI_MARKERDELETE, line, g_ChangeMarkId );
        SendMessage( hCurScintilla, SCI_MARKERADD, line, g_SaveMarkId );
        pos = line;
    }
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
                                              LR_LOADTRANSPARENT );
        data.pszModuleName = _Panel.getPluginFileName();

        // the dlgDlg should be the index of funcItem where the current function pointer is
        // in this case is DOCKABLE_INDEX
        data.dlgID = DOCKABLE_INDEX;
        ::SendMessage( nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0,
                       ( LPARAM )&data );

        _Panel.setClosed(true);
    }

    if ( _Panel.isClosed() || g_RaisePanel )
    {
        _Panel.display();
        _Panel.setClosed(false);
        ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                       funcItem[DOCKABLE_INDEX]._cmdID, MF_CHECKED );
        _Panel.updateListTimer();
    }
    else
    {
        _Panel.display( false );
        _Panel.setClosed(true);
        ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                       funcItem[DOCKABLE_INDEX]._cmdID, MF_UNCHECKED );
    }
}
