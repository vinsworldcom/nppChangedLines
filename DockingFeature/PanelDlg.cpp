//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
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

#include "PanelDlg.h"
#include "../PluginDefinition.h"
#include "SettingsDlg.h"
#include "resource.h"

#include <commctrl.h>

extern NppData nppData;
extern HWND hDialog;

extern bool g_enabled;
extern bool g_GotoIncSave;

const int WS_TOOLBARSTYLE = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS |TBSTYLE_FLAT | CCS_TOP | BTNS_AUTOSIZE | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;
                         /* WS_CHILD | WS_VISIBLE |                                                                                                                    CCS_NORESIZE |                CCS_ADJUSTABLE */

TBBUTTON tbButtonsAdd1[] =
{
    {MAKELONG( 0, 0 ), IDC_BTN_PREV,     TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {MAKELONG( 1, 0 ), IDC_BTN_NEXT,     TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {0,                0,                0,               BTNS_SEP,       {0}, 0, 0},
    {MAKELONG( 2, 0 ), IDC_BTN_CLEARALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {0,                0,                0,               BTNS_SEP,       {0}, 0, 0},
    {MAKELONG( 3, 0 ), IDC_BTN_SEARCH,   TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {0,                0,                0,               BTNS_SEP,       {0}, 0, 0},
    {MAKELONG( 4, 0 ), IDC_BTN_SETTINGS, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0}
};
const int sizeButtonArray1 = sizeof( tbButtonsAdd1 ) / sizeof( TBBUTTON );
const int numButtons1      = sizeButtonArray1 - 3 /* separators */;

static LPCTSTR szToolTip[30] = {
    TEXT("Previous Change"),
    TEXT("Next Change"),
    TEXT("Clear All in Current File"),
    TEXT("Find All in Current File"),
    TEXT("Settings"),
};

LPCTSTR GetNameStrFromCmd( UINT resID )
{
    if ((IDC_BTN_PREV <= resID) && (resID <= IDC_BTN_SETTINGS)) {
        return szToolTip[resID - IDC_BTN_PREV];
    }
    return NULL;
}

void imageToolbar( HINSTANCE hInst, HWND hWndToolbar, UINT ToolbarID, const int numButtons )
{
    HBITMAP hbm = LoadBitmap( hInst, MAKEINTRESOURCE( ToolbarID ) );
    BITMAP bm = {0};
    GetObject( hbm, sizeof( BITMAP ), &bm );
    int iImageWidth  = bm.bmWidth / numButtons;
    int iImageHeight = bm.bmHeight;
    HIMAGELIST himlToolBar1 = ( HIMAGELIST )SendMessage( hWndToolbar, TB_GETIMAGELIST, 0, 0 );
    ImageList_Destroy( himlToolBar1 );
    himlToolBar1 = ImageList_Create( iImageWidth, iImageHeight, ILC_COLOR32 | ILC_MASK, numButtons, 0 );
    ImageList_AddMasked( himlToolBar1, hbm, RGB( 192, 192, 192 ) );
    SendMessage( hWndToolbar, TB_SETIMAGELIST, 0, ( LPARAM )himlToolBar1 );
}

void refreshDialog()
{
    SendMessage( GetDlgItem( hDialog, IDC_CHK_ENABLED ), BM_SETCHECK, ( WPARAM )( g_enabled ? 1 : 0 ), 0 );
    SendMessage( GetDlgItem( hDialog, IDC_CHK_INCSAVES ), BM_SETCHECK, ( WPARAM )( g_GotoIncSave ? 1 : 0 ), 0 );
}

void initDialog()
{
    INITCOMMONCONTROLSEX ic;

    ic.dwSize = sizeof( INITCOMMONCONTROLSEX );
    ic.dwICC = ICC_BAR_CLASSES | ICC_PAGESCROLLER_CLASS;
    InitCommonControlsEx( &ic );

    HWND hWndToolbar1, hWndPager1;

    // TOOLBAR1
    // Create pager.  The parent window is the parent.
    hWndPager1 = CreateWindow( WC_PAGESCROLLER, NULL, WS_VISIBLE | WS_CHILD | PGS_HORZ,
                              0, 0, 200, 32, hDialog, (HMENU) IDB_PAGER1, GetModuleHandle( TEXT("ChangedLines.dll" ) ), NULL );
    // Create Toolbar.  The parent window is the Pager.
    hWndToolbar1 = CreateWindowEx( 0, TOOLBARCLASSNAME, NULL, WS_TOOLBARSTYLE,
                                  0, 0, 200, 32, hWndPager1, ( HMENU ) IDB_TOOLBAR1, GetModuleHandle( TEXT("ChangedLines.dll" ) ), NULL );

    SendMessage( hWndToolbar1, TB_BUTTONSTRUCTSIZE, sizeof( TBBUTTON ), 0 );
    SendMessage( hWndToolbar1, TB_SETEXTENDEDSTYLE, 0, ( LPARAM ) TBSTYLE_EX_HIDECLIPPEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS );
    SendMessage( hWndToolbar1, TB_ADDBUTTONS, sizeButtonArray1, ( LPARAM )tbButtonsAdd1 );
    SendMessage( hWndToolbar1, TB_AUTOSIZE, 0, 0 );
    // Notify the pager that it contains the toolbar
    SendMessage(hWndPager1, PGM_SETCHILD, 0, (LPARAM) hWndToolbar1);

    imageToolbar( GetModuleHandle( TEXT("ChangedLines.dll" ) ), hWndToolbar1, IDB_TOOLBAR1, numButtons1 );

    refreshDialog();
}

INT_PTR CALLBACK DemoDlg::run_dlgProc( UINT message, WPARAM wParam,
                                       LPARAM lParam )
{
    switch ( message )
    {
        case WM_COMMAND :
        {
            switch ( wParam )
            {
                case IDC_CHK_ENABLED :
                {
                    doEnable();
                    return TRUE;
                }
                case IDC_BTN_NEXT :
                {
                    gotoNextChange();
                    return TRUE;
                }
                case IDC_BTN_PREV :
                {
                    gotoPrevChange();
                    return TRUE;
                }
                case IDC_CHK_INCSAVES :
                {
                    int check = ( int )::SendMessage( GetDlgItem( hDialog, IDC_CHK_INCSAVES ), BM_GETCHECK, 0, 0 );

                    if( check & BST_CHECKED )
                        g_GotoIncSave = true;
                    else
                        g_GotoIncSave = false;

                    return TRUE;
                }
                case IDC_BTN_CLEARALL :
                {
                    clearAllCF();
                    return TRUE;
                }
                case IDC_BTN_SETTINGS :
                {
                    doSettings();
                    return TRUE;
                }
            }
            return FALSE;
        }

        case WM_NOTIFY:
        {
            LPNMHDR nmhdr = (LPNMHDR)lParam;

            if ( nmhdr->code == TTN_GETDISPINFO ) /* TTN_NEEDTEXT */
            {
                UINT idButton;
                LPTOOLTIPTEXT lpttt;

                lpttt           = (LPTOOLTIPTEXT) lParam;
                lpttt->hinst    = NULL;
                idButton        = lpttt->hdr.idFrom;
                lpttt->lpszText = const_cast<LPTSTR>( GetNameStrFromCmd( idButton ) );
                return TRUE;
            }

			DockingDlgInterface::run_dlgProc( message, wParam, lParam );

			return FALSE;
        }

        case WM_INITDIALOG:
        {
            initDialog();
        }

        default :
            return DockingDlgInterface::run_dlgProc( message, wParam, lParam );
    }
}
