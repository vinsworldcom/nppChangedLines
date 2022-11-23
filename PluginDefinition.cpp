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

const TCHAR configFileName[]      = TEXT( "ChangedLines.ini" );
const TCHAR sectionName[]         = TEXT( "Settings" );
const TCHAR iniKeyEnabled[]       = TEXT( "Enabled" );
const TCHAR iniKeyRaisePanel[]    = TEXT( "PanelRaiseorToggle" );
const TCHAR iniKeyPanelIncSave[]  = TEXT( "PanelIncludeSave" );
const TCHAR iniKeyWidth[]         = TEXT( "MarginWidth" );
const TCHAR iniKeyColorChange[]   = TEXT( "ColorChange" );
const TCHAR iniKeyColorSave[]     = TEXT( "ColorSave" );
const TCHAR iniKeyColorRevMod[]   = TEXT( "ColorRevMod" );
const TCHAR iniKeyColorRevOri[]   = TEXT( "ColorRevOri" );
const TCHAR iniKeyUseNppColors[]  = TEXT( "UseNppColors" );
const TCHAR iniKeyUseIndicators[] = TEXT( "UseIndicators" );

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
bool g_PanelIncSave    = DEFAULTGOTOINCSAVE;
int  g_Margin          = DEFAULTMARGIN;
int  g_Width           = DEFAULTWIDTH;
long g_ColorChange     = DEFAULTCOLOR_MODIFIED;
long g_ColorSave       = DEFAULTCOLOR_SAVED;
long g_ColorRevMod     = DEFAULTCOLOR_REVERTED_TO_MODIFIED;
long g_ColorRevOri     = DEFAULTCOLOR_REVERTED_TO_ORIGIN;
bool g_useNppColors    = false;
bool g_useIndicators   = false;
int  g_MaskChange      = ( 1 << SC_MARKNUM_HISTORY_MODIFIED );
int  g_MaskSave        = ( 1 << SC_MARKNUM_HISTORY_SAVED );
int  g_MaskRevMod      = ( 1 << SC_MARKNUM_HISTORY_REVERTED_TO_MODIFIED );
int  g_MaskRevOri      = ( 1 << SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN );

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
    ::WritePrivateProfileString( sectionName, iniKeyPanelIncSave,
                                 g_PanelIncSave ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );
    _itot_s( g_Width, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyWidth, buf,
                                 iniFilePath );
    _itot_s( g_ColorChange, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyColorChange, buf,
                                 iniFilePath );
    _itot_s( g_ColorSave, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyColorSave, buf,
                                 iniFilePath );
    _itot_s( g_ColorRevMod, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyColorRevMod, buf,
                                 iniFilePath );
    _itot_s( g_ColorRevOri, buf, NUMDIGIT, 10 );
    ::WritePrivateProfileString( sectionName, iniKeyColorRevOri, buf,
                                 iniFilePath );
    ::WritePrivateProfileString( sectionName, iniKeyUseNppColors,
                                 g_useNppColors ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );
    ::WritePrivateProfileString( sectionName, iniKeyRaisePanel,
                                 g_RaisePanel ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );
    ::WritePrivateProfileString( sectionName, iniKeyUseIndicators,
                                 g_useIndicators ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );

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
    g_PanelIncSave     = ::GetPrivateProfileInt( sectionName, iniKeyPanelIncSave, 0,
                        iniFilePath );
    g_Width           = ::GetPrivateProfileInt( sectionName, iniKeyWidth,
                        DEFAULTWIDTH, iniFilePath );
    g_ColorChange     = ::GetPrivateProfileInt( sectionName, iniKeyColorChange,
                        DEFAULTCOLOR_MODIFIED, iniFilePath );
    g_ColorSave       = ::GetPrivateProfileInt( sectionName, iniKeyColorSave,
                        DEFAULTCOLOR_SAVED, iniFilePath );
    g_ColorRevMod     = ::GetPrivateProfileInt( sectionName, iniKeyColorRevMod,
                        DEFAULTCOLOR_REVERTED_TO_MODIFIED, iniFilePath );
    g_ColorRevOri     = ::GetPrivateProfileInt( sectionName, iniKeyColorRevOri,
                        DEFAULTCOLOR_REVERTED_TO_ORIGIN, iniFilePath );
    g_useNppColors    = ::GetPrivateProfileInt( sectionName, iniKeyUseNppColors,
                                                0, iniFilePath );
    g_useIndicators   = ::GetPrivateProfileInt( sectionName, iniKeyUseIndicators,
                                                0, iniFilePath );
    g_RaisePanel      = ::GetPrivateProfileInt( sectionName, iniKeyRaisePanel,
                                                0, iniFilePath );

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

    setCommand( ENABLE_INDEX,   TEXT( "&Enable Plugin Features" ), doEnable, NULL, g_enabled ? true : false );
    setCommand( DOCKABLE_INDEX, TEXT( "Changed &Lines Panel" ), DockableDlg, NULL, _Panel.isVisible() ? true : false );
    setCommand( 2,  TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( 3,  TEXT( "Goto &Previous Change" ), gotoPrevChangeAll, NULL, false );
    setCommand( 4,  TEXT( "Goto Previous Change (only)" ), gotoPrevChangeCOnly, NULL, false );
    setCommand( 5,  TEXT( "Goto Previous Save (only)" ), gotoPrevChangeSOnly, NULL, false );
    setCommand( 6,  TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( 7,  TEXT( "Goto &Next Change" ), gotoNextChangeAll, NULL, false );
    setCommand( 8,  TEXT( "Goto Next Change (only)" ), gotoNextChangeCOnly, NULL, false );
    setCommand( 9,  TEXT( "Goto Next Save (only)" ), gotoNextChangeSOnly, NULL, false );
    setCommand( 10, TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( 11, TEXT( "&Clear Change History (current file)" ), clearAllCF, NULL, false );
    setCommand( 12, TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( 13, TEXT( "Goto Pre&vious Position" ), gotoPrevPos, PreChgKey, false );
    setCommand( 14, TEXT( "Goto Ne&xt Position" ), gotoNextPos, NextChgKey, false );
    setCommand( 15, TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( 16, TEXT( "&Settings" ), doSettings, NULL, false );
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
    return ( ( Sci_Position )::SendMessage( hCurScintilla, SCI_MARKERNEXT, 
                                   searchStart, mask ) );
}

Sci_Position findPrevMark( HWND hCurScintilla, Sci_Position searchStart, int mask )
{
    return ( ( Sci_Position )::SendMessage( hCurScintilla, SCI_MARKERPREVIOUS,
                                   searchStart, mask ) );
}

void UpdatePlugin( UINT Msg, WPARAM wParam, LPARAM lParam )
{
    if ( ! g_enabled )
        return;

    HWND ScintillaArr[] = { nppData._scintillaMainHandle, nppData._scintillaSecondHandle };

    for ( int i = 0; i < 2; i++ )
    {
        HWND hCurScintilla = ScintillaArr[i];
        SendMessage( hCurScintilla, Msg, wParam, lParam );
    }
}

void updateIndicators()
{
    int on = SC_CHANGE_HISTORY_DISABLED;
    on = ( int )::SendMessage( getCurScintilla(), SCI_GETCHANGEHISTORY, 0, 0 );
    if ( on > SC_CHANGE_HISTORY_DISABLED )
    {
        if ( g_useIndicators )
            UpdatePlugin( SCI_SETCHANGEHISTORY, on | SC_CHANGE_HISTORY_INDICATORS, 0 );
        else
            UpdatePlugin( SCI_SETCHANGEHISTORY, on &~ SC_CHANGE_HISTORY_INDICATORS, 0 );
    }
}

void updateWidth()
{
    UpdatePlugin( SCI_SETMARGINWIDTHN, g_Margin, g_Width );
}

void updateChangeColor()
{
    UpdatePlugin( SCI_MARKERSETFORE, SC_MARKNUM_HISTORY_MODIFIED, g_ColorChange );
    UpdatePlugin( SCI_MARKERSETBACK, SC_MARKNUM_HISTORY_MODIFIED, g_ColorChange );
}

void updateSaveColor()
{
    UpdatePlugin( SCI_MARKERSETFORE, SC_MARKNUM_HISTORY_SAVED, g_ColorSave );
    UpdatePlugin( SCI_MARKERSETBACK, SC_MARKNUM_HISTORY_SAVED, g_ColorSave );
}

void updateRevertModColor()
{
    UpdatePlugin( SCI_MARKERSETFORE, SC_MARKNUM_HISTORY_REVERTED_TO_MODIFIED, g_ColorRevMod );
    UpdatePlugin( SCI_MARKERSETBACK, SC_MARKNUM_HISTORY_REVERTED_TO_MODIFIED, g_ColorRevMod );
}

void updateRevertOriginColor()
{
    UpdatePlugin( SCI_MARKERSETFORE, SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN, g_ColorRevOri );
    UpdatePlugin( SCI_MARKERSETBACK, SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN, g_ColorRevOri );
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

bool InitPlugin()
{
    long version = ( long )::SendMessage( nppData._nppHandle, NPPM_GETNPPVERSION, true, 0 );
    if (( HIWORD( version ) < 8 ) ||
       (( HIWORD( version ) == 8 ) && ( LOWORD( version ) < 460 )))
    {
        MessageBox( nppData._nppHandle, 
            TEXT("This version of Change Lines only works with Notepad++ >= 8.4.6."),
            TEXT("Version Issue"), 
            MB_OK | MB_ICONERROR
        );
        return false;
    }

    int on = SC_CHANGE_HISTORY_DISABLED;
    on = ( int )::SendMessage( getCurScintilla(), SCI_GETCHANGEHISTORY, 0, 0 );
    if ( on == SC_CHANGE_HISTORY_DISABLED )
    {
        MessageBox( nppData._nppHandle, 
            TEXT("Change History is disabled.  Please enable it in Notepad++ Settings."), 
            TEXT("Change History Disabled"), 
            MB_OK | MB_ICONINFORMATION
        );
        return false;
    }

    HWND ScintillaArr[] = { nppData._scintillaMainHandle, nppData._scintillaSecondHandle };

    for ( int i = 0; i < 2; i++ )
    {
        HWND hCurScintilla = ScintillaArr[i];

        SendMessage( hCurScintilla, SCI_SETMARGINSENSITIVEN, g_Margin, true );
    }

    updateWidth();
    updateIndicators();
    updateChangeColor();
    updateSaveColor();
    updateRevertModColor();
    updateRevertOriginColor();

    ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                   funcItem[ENABLE_INDEX]._cmdID, MF_CHECKED );

    return true;
}

void DestroyPlugin()
{
    ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                   funcItem[ENABLE_INDEX]._cmdID, MF_UNCHECKED );
}

void clearAllCF()
{
    HWND hCurScintilla = getCurScintilla();

    int chFlags = ( int )::SendMessage( hCurScintilla, SCI_GETCHANGEHISTORY, 0, 0 );
    Sci_Position pos = ( Sci_Position )::SendMessage( hCurScintilla, SCI_GETCURRENTPOS, 0, 0 );
    
    SendMessage( hCurScintilla, SCI_EMPTYUNDOBUFFER, 0, 0 );
    SendMessage( hCurScintilla, SCI_SETCHANGEHISTORY, SC_CHANGE_HISTORY_DISABLED, 0 );

    SendMessage( hCurScintilla, SCI_SETCHANGEHISTORY, chFlags, 0 );

    SendMessage( hCurScintilla, SCI_GOTOPOS, pos, 0 );
}

void doEnable()
{
    // UINT state = ::GetMenuState( ::GetMenu( nppData._nppHandle ),
                                 // funcItem[ENABLE_INDEX]._cmdID,
                                 // MF_BYCOMMAND );

    if ( g_enabled )
    {
        g_enabled = false;
        DestroyPlugin();
    }
    else
    {
        g_enabled = true;
        bool success = InitPlugin();
        if ( ! success)
        {
            g_enabled = false;
            DestroyPlugin();
        }
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

void gotoNextChangeAll()
{
    gotoNextChange(false, false);
}

void gotoNextChangeCOnly()
{
    gotoNextChange(true, false);
}

void gotoNextChangeSOnly()
{
    gotoNextChange(false, true);
}

void gotoNextChange(bool changed, bool saved)
{
    HWND hCurScintilla = getCurScintilla();

    Sci_Position line = -1;
    Sci_Position pos = ( Sci_Position )::SendMessage( hCurScintilla, SCI_GETCURRENTPOS, 0, 0 );
    Sci_Position searchStart = ( Sci_Position )::SendMessage( hCurScintilla, SCI_LINEFROMPOSITION,
                                            pos, 0 );

    int mask = g_MaskChange | g_MaskSave | g_MaskRevMod | g_MaskRevOri;
    if ( changed )
        mask = g_MaskChange | g_MaskRevMod;
    if ( saved )
        mask = g_MaskSave | g_MaskRevOri;

/*
    `markerNext` doesn't work on ChangeHistory:
       https://sourceforge.net/p/scintilla/bugs/2353/
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
    `markerNext` doesn't work on ChangeHistory:
*/

    Sci_Position textLength = ( Sci_Position)::SendMessage( hCurScintilla, SCI_GETTEXTLENGTH, 0, 0 );
    Sci_Position lastLine = ( Sci_Position)::SendMessage( hCurScintilla, SCI_LINEFROMPOSITION, textLength, 0 );
    Sci_Position consec = searchStart;

    for ( Sci_Position i = searchStart+1; i < lastLine+1; i++ )
    {
        int mark = ( int )::SendMessage( hCurScintilla, SCI_MARKERGET, i, 0 );
        if ( mark & mask )
        {
            if ( i == consec + 1 )
                consec += 1;
            else
            {
                line = i;
                break;
            }
        }
    }

    if ( line == -1 )
    {
        for ( Sci_Position i = 0; i < searchStart; i++ )
        {
            int mark = ( int )::SendMessage( hCurScintilla, SCI_MARKERGET, i, 0 );
            if ( mark & mask )
            {
                if ( i == consec + 1 )
                    consec += 1;
                else
                {
                    line = i;
                    break;
                }
            }
        }
    }

    if ( line != -1 )
        gotoLine(line);
}

void gotoPrevChangeAll()
{
    gotoPrevChange(false, false);
}

void gotoPrevChangeCOnly()
{
    gotoPrevChange(true, false);
}

void gotoPrevChangeSOnly()
{
    gotoPrevChange(false, true);
}

void gotoPrevChange(bool changed, bool saved)
{
    HWND hCurScintilla = getCurScintilla();

    Sci_Position line = -1;
    Sci_Position pos = ( Sci_Position )::SendMessage( hCurScintilla, SCI_GETCURRENTPOS, 0, 0 );
    Sci_Position searchStart = ( Sci_Position )::SendMessage( hCurScintilla, SCI_LINEFROMPOSITION,
                                            pos, 0 );

    int mask = g_MaskChange | g_MaskSave | g_MaskRevMod | g_MaskRevOri;
    if ( changed )
        mask = g_MaskChange | g_MaskRevMod;
    if ( saved )
        mask = g_MaskSave | g_MaskRevOri;

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
