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

extern FuncItem funcItem[nbFunc];
extern HINSTANCE g_hInst;
extern NppData   nppData;
extern bool      g_NppReady;
extern bool      g_enabled;

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
        case NPPN_READY:
        {
            g_NppReady = true;
            if ( g_enabled )
                InitPlugin();
        }
        break;

        case NPPN_BUFFERACTIVATED:
        {
            if ( g_NppReady )
                updatePanel();
        }
        break;

        case SCN_MARGINCLICK:
        {
            if ( g_enabled )
                gotoNextChange();
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

                HWND hCurScintilla = getCurScintilla();
                isAutoModify = ( SendMessage( hCurScintilla, SCI_GETUNDOCOLLECTION, 0, 0 ) == 0 );

                if ( !isAutoModify )
                {
                    if ( ModifyType & SC_PERFORMED_UNDO )
                    {
                        int line = -1;

                        if ( notifyCode->linesAdded == 0 && preModifyPos != pos && preModifyPos != -1 )
                        {
                            line = ( int )::SendMessage( hCurScintilla, SCI_LINEFROMPOSITION, preModifyPos, 0 );
                            DelBookmark( hCurScintilla, line, preModifyLineAdd );
                        }
         
                        if ( notifyCode->linesAdded != 0 || ( ModifyType & SC_LASTSTEPINUNDOREDO ) )
                        {
                            line = ( int )::SendMessage( hCurScintilla, SCI_LINEFROMPOSITION, pos, 0 );
                            DelBookmark( hCurScintilla, line, notifyCode->linesAdded );
                            preModifyPos = -1;
                        }
                        else
                        {
                            preModifyPos = pos;
                            preModifyLineAdd = notifyCode->linesAdded;
                        }
                    }
                    else
                    {
                        // SC_PERFORMED_REDO
                        int line = ( int )::SendMessage( hCurScintilla, SCI_LINEFROMPOSITION, notifyCode->position, 0 );
                        SetBookmark( hCurScintilla, line, notifyCode->linesAdded );
                    }
                }
            }
        }
        break;

        case SCN_SAVEPOINTREACHED:
        {
            convertChangeToSave();
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
