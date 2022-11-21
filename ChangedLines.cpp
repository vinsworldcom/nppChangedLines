///----------------------------------------------------------------------------
/// Copyright (c) 2008-2010
/// Brandon Cannaday
/// Paranoid Ferret Productions (support@paranoidferret.com)
///
/// This program is free software; you can redistribute it and/or
/// modify it under the terms of the GNU General Public License
/// as published by the Free Software Foundation; either
/// version 2 of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
///----------------------------------------------------------------------------

#include "PluginDefinition.h"
#include "resource.h"
#include "Scintilla.h"

extern FuncItem funcItem[nbFunc];
extern HINSTANCE g_hInst;
extern NppData   nppData;
extern bool      g_NppReady;
extern bool      g_enabled;
extern int       g_Margin;
extern toolbarIcons g_TBCL;

static Sci_Position preModifyPos = -1;
static Sci_Position preModifyLineAdd = -1;
static bool isAutoModify = false;

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  reasonForCall, LPVOID /*lpReserved*/ )
{
    switch ( reasonForCall )
    {
        case DLL_PROCESS_ATTACH:
            g_hInst = ( HINSTANCE )hModule;
            pluginInit( hModule );
            break;

        case DLL_PROCESS_DETACH:
            pluginCleanUp();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

extern "C" __declspec( dllexport ) void setInfo( NppData notpadPlusData )
{
    nppData = notpadPlusData;
    commandMenuInit();
}

extern "C" __declspec( dllexport ) const TCHAR *getName()
{
    return NPP_PLUGIN_NAME;
}

extern "C" __declspec( dllexport ) FuncItem *getFuncsArray( int *nbF )
{
    *nbF = nbFunc;
    return funcItem;
}

extern "C" __declspec( dllexport ) void beNotified( SCNotification *notifyCode )
{
    int ModifyType = notifyCode->modificationType;

    switch (notifyCode->nmhdr.code)
    {
        case NPPN_TBMODIFICATION:
        {
            g_TBCL.hToolbarBmp = (HBITMAP)::LoadImage((HINSTANCE)g_hInst, MAKEINTRESOURCE(IDB_TB_CL), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
            ::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON_DEPRECATED, (WPARAM)funcItem[DOCKABLE_INDEX]._cmdID, (LPARAM)&g_TBCL);
        }
        break;

        case NPPN_READY:
        {
            g_NppReady = true;
            if ( g_enabled )
            {
                bool success = InitPlugin();
                if ( ! success)
                {
                    g_enabled = false;
                    DestroyPlugin();
                }
            }
        }
        break;

        case NPPN_FILESAVED:
        case NPPN_BUFFERACTIVATED:
        {
            if ( !g_enabled )
                break;

            if ( g_NppReady )
            {
                updateWidth();
                updateIndicators();
                updatePanel();
            }
        }
        break;

        case SCN_MARGINCLICK:
        {
            if ( !g_enabled )
                break;

            if ( notifyCode->margin != g_Margin )
                break;

            if ( notifyCode->modifiers == SCMOD_NORM )
                gotoNextChangeAll();
            else if ( notifyCode->modifiers == SCMOD_SHIFT )
                gotoPrevChangeAll();
            else if ( notifyCode->modifiers == SCMOD_CTRL )
                gotoNextChangeCOnly();
            else if ( notifyCode->modifiers == (SCMOD_SHIFT | SCMOD_CTRL) )
                gotoPrevChangeCOnly();
            else if ( notifyCode->modifiers == SCMOD_ALT )
                gotoNextChangeSOnly();
            else if ( notifyCode->modifiers == (SCMOD_SHIFT | SCMOD_ALT) )
                gotoPrevChangeSOnly();
        }
        break;

        case SCN_MARGINRIGHTCLICK:
        {
            if ( !g_enabled )
                break;

            if ( notifyCode->margin != g_Margin )
                break;

            HMENU pm = CreatePopupMenu();
            AppendMenu( pm, MF_STRING,    100000, TEXT("Previous Change") );
            AppendMenu( pm, MF_STRING,    100001, TEXT("Previous Change (only)") );
            AppendMenu( pm, MF_STRING,    100002, TEXT("Previous Save (only)") );
            AppendMenu( pm, MF_SEPARATOR, 0,      TEXT("") );
            AppendMenu( pm, MF_STRING,    100003, TEXT("Next Change") );
            AppendMenu( pm, MF_STRING,    100004, TEXT("Next Change (only)") );
            AppendMenu( pm, MF_STRING,    100005, TEXT("Next Save (only)") );
            AppendMenu( pm, MF_SEPARATOR, 0,      TEXT("") );
            AppendMenu( pm, MF_STRING,    100006, TEXT("Clear Change History (current file)") );

            POINT pos;
            GetCursorPos( &pos );
            SetForegroundWindow( nppData._nppHandle );
            int ret = TrackPopupMenu(
                pm, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD,
                pos.x, pos.y, 0, nppData._nppHandle, NULL
            );
            PostMessage( nppData._nppHandle, WM_NULL, 0, 0 );

            if ( ret == 0 )
                break;

            else if ( ret == 100000 )
                gotoPrevChangeAll();
            else if ( ret == 100001 )
                gotoPrevChangeCOnly();
            else if ( ret == 100002 )
                gotoPrevChangeSOnly();
            else if ( ret == 100003 )
                gotoNextChangeAll();
            else if ( ret == 100004 )
                gotoNextChangeCOnly();
            else if ( ret == 100005 )
                gotoNextChangeSOnly();
            else if ( ret == 100006 )
                clearAllCF();

            updatePanel();
        }
        break;

        case SCN_MODIFIED:
        {
            if ( !g_enabled )
                break;

            if ( notifyCode->nmhdr.hwndFrom != nppData._scintillaMainHandle &&
                 notifyCode->nmhdr.hwndFrom != nppData._scintillaSecondHandle )
                break;

            Sci_Position pos = 0;
            Sci_Position len = 0;
            bool flag = false;

            if ( ModifyType & SC_MOD_INSERTTEXT )
            {
                pos = notifyCode->position;
                len = notifyCode->length;
                flag = true;
            }
            else if ( ModifyType & SC_MOD_DELETETEXT )
            {
                pos = notifyCode->position;
                len = -notifyCode->length;
                flag = true;
            }

            if ( flag )
            {
                if ( notifyCode->text == NULL && ( ModifyType == 0x12 ||
                                                   ModifyType == 0x11 ) )
                    break;

                if ( 0 == pos )
                {
                    // maybe search in files, checked it
                    // Get the current scintilla
                    HWND tmpcurScintilla = ( HWND ) - 1;
                    int which = -1;
                    SendMessage( nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, ( LPARAM )&which );

                    if ( which != -1 )
                        tmpcurScintilla = ( which == 0 ) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

                    // search in files
                    if ( -1 == ( long long )tmpcurScintilla )
                        break;

                    //  SC_PERFORMED_USER| SC_MOD_DELETETEXT
                    // Open New file // ??????? || ModifyType == 0x2011
                    if ( notifyCode->text == NULL && 1024 == notifyCode->length )
                        break;
                }
            }

            updatePosition();
            updatePanel();
        }
        break;

        case NPPN_SHUTDOWN:
        {
            commandMenuCleanUp();
        }
        break;

        default:
            return;
    }
}

// Here you can process the Npp Messages
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return TRUE;
}

#ifdef UNICODE
extern "C" __declspec( dllexport ) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE
