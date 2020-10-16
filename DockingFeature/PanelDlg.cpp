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
#include "../CircularStackLinkList.h"
#include "SettingsDlg.h"
#include "resource.h"

#include <codecvt>
#include <commctrl.h>
#include <string>
#include <vector>
#include <windowsx.h>

extern NppData nppData;
extern HWND hDialog;

extern bool g_NppReady;
extern bool g_GotoIncSave;
extern bool g_useNppColors;

extern circular_buffer<tDocPos> prevPos;
extern circular_buffer<tDocPos> nextPos;

LVITEM   LvItem;
LVCOLUMN LvCol;
COLORREF colorBg;
COLORREF colorFg;

// #define COL_CHK 0
#define COL_LINE 0
#define COL_TEXT 1
#define TIMER_CHANGE       1
#define TIMER_CHANGE_DELAY 500

const int WS_TOOLBARSTYLE = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN |
                            WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | CCS_TOP | BTNS_AUTOSIZE
                            | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;

TBBUTTON tbButtonsAdd1[] =
{
    {MAKELONG( 0, 0 ), IDC_BTN_PREV,     TBSTATE_ENABLED, TBSTYLE_DROPDOWN, {0}, 0, 0},
    {MAKELONG( 1, 0 ), IDC_BTN_NEXT,     TBSTATE_ENABLED, TBSTYLE_DROPDOWN, {0}, 0, 0},
    {0,                0,                0,               BTNS_SEP,       {0}, 0, 0},
    {MAKELONG( 2, 0 ), IDC_BTN_CLEARALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {0,                0,                0,               BTNS_SEP,       {0}, 0, 0},
    {MAKELONG( 3, 0 ), IDC_BTN_SEARCH,   TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {0,                0,                0,               BTNS_SEP,       {0}, 0, 0},
    {MAKELONG( 4, 0 ), IDC_BTN_SETTINGS, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0}
};
const int sizeButtonArray1 = sizeof( tbButtonsAdd1 ) / sizeof( TBBUTTON );
const int numButtons1      = sizeButtonArray1 - 3 /* separators */;

static LPCTSTR szToolTip[30] =
{
    TEXT( "Previous Position" ),
    TEXT( "Next Position" ),
    TEXT( "Clear All in Current File" ),
    TEXT( "Find All in Current File" ),
    TEXT( "Settings" ),
};

LPCTSTR GetNameStrFromCmd( UINT resID )
{
    if ( ( IDC_BTN_PREV <= resID ) && ( resID <= IDC_BTN_SETTINGS ) )
        return szToolTip[resID - IDC_BTN_PREV];

    return NULL;
}

void imageToolbar( HINSTANCE hInst, HWND hWndToolbar, UINT ToolbarID,
                   const int numButtons )
{
    HBITMAP hbm = LoadBitmap( hInst, MAKEINTRESOURCE( ToolbarID ) );
    BITMAP bm = {0};
    GetObject( hbm, sizeof( BITMAP ), &bm );
    int iImageWidth  = bm.bmWidth / numButtons;
    int iImageHeight = bm.bmHeight;
    HIMAGELIST himlToolBar1 = ( HIMAGELIST )SendMessage( hWndToolbar,
                              TB_GETIMAGELIST, 0, 0 );
    ImageList_Destroy( himlToolBar1 );
    himlToolBar1 = ImageList_Create( iImageWidth, iImageHeight,
                                     ILC_COLOR32 | ILC_MASK, numButtons, 0 );
    ImageList_AddMasked( himlToolBar1, hbm, RGB( 192, 192, 192 ) );
    SendMessage( hWndToolbar, TB_SETIMAGELIST, 0, ( LPARAM )himlToolBar1 );
}

void clearList()
{
    SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_DELETEALLITEMS, 0, 0 );
}

void setListColumns( unsigned int uItem, std::wstring strLine,
                     std::wstring strText )
{
    // https://www.codeproject.com/Articles/2890/Using-ListView-control-under-Win32-API
    memset( &LvItem, 0, sizeof( LvItem ) ); // Zero struct's Members
    LvItem.mask       = LVIF_TEXT;    // Text Style
    LvItem.cchTextMax = MAX_PATH;     // Max size of text
    LvItem.iItem      = uItem;        // choose item

    // LvItem.iSubItem   = COL_CHK;      // Put in first coluom
    // LvItem.pszText    = TEXT( "" );
    // SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_INSERTITEM, 0, ( LPARAM )&LvItem );

    LvItem.iSubItem   = COL_LINE;        // Put in second coluom
    LvItem.pszText    = const_cast<LPWSTR>( strLine.c_str() );
    SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_INSERTITEM, 0,
                 ( LPARAM )&LvItem );

    LvItem.iSubItem   = COL_TEXT;        // Put in third coluom
    LvItem.pszText    = const_cast<LPWSTR>( strText.c_str() );
    SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_SETITEM, 0,
                 ( LPARAM )&LvItem );
}


std::wstring stringToWstring( const std::string &t_str )
{
    //setup converter
    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    return converter.from_bytes( t_str );
}

void updateListTimer()
{
    KillTimer( hDialog, TIMER_CHANGE );
    SetTimer( hDialog, TIMER_CHANGE, TIMER_CHANGE_DELAY, NULL );
}

void updateList()
{
    if ( ! g_NppReady )
        return;

    clearList();

    HWND hCurScintilla = getCurScintilla();

    int mask = CHANGE_MASK;

    if ( g_GotoIncSave )
        mask |= SAVE_MASK;

    int line = 0;
    int i    = 0;

    while ( true )
    {
        line = findNextMark( hCurScintilla, line, mask );

        if ( line == -1 )
            break;

// TODO:2020-01-19:MVINCENT:  ListView, SCI_GETLINE, https://stackoverflow.com/questions/18536125/dynamic-memory-allocation-to-char-array
        int lineLen = ( int )::SendMessage( getCurScintilla(), SCI_GETLINE, line,
                                            ( LPARAM ) 0 );
        char *array = new char[ lineLen + 1 ];
        SendMessage( getCurScintilla(), SCI_GETLINE, line, ( LPARAM ) array );
        array[lineLen] = '\0';
        std::wstring buffer = stringToWstring( array );
        setListColumns( i, std::to_wstring( line + 1 ), buffer );
        delete[] array;

        line++;
        i++;
    }
}

void refreshDialog()
{
    SendMessage( GetDlgItem( hDialog, IDC_CHK_NPPCOLOR ), BM_SETCHECK,
                 ( WPARAM )( g_useNppColors ? 1 : 0 ), 0 );
}

void SetNppColors()
{
    colorBg = ( COLORREF )::SendMessage( getCurScintilla(), SCI_STYLEGETBACK, 0,
                                         0 );
    colorFg = ( COLORREF )::SendMessage( getCurScintilla(), SCI_STYLEGETFORE, 0,
                                         0 );
}

void SetSysColors()
{
    colorBg = GetSysColor( COLOR_WINDOW );
    colorFg = GetSysColor( COLOR_WINDOWTEXT );
}

void ChangeColors()
{
    HWND hList = GetDlgItem( hDialog, IDC_LSV1 );

    ::SendMessage( hList, WM_SETREDRAW, FALSE, 0 );

    ListView_SetBkColor( hList, colorBg );
    ListView_SetTextBkColor( hList, colorBg );
    ListView_SetTextColor( hList, colorFg );

    ::SendMessage( hList, WM_SETREDRAW, TRUE, 0 );
    ::RedrawWindow( hList, NULL, NULL,
                    RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN );
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
    hWndPager1 = CreateWindow( WC_PAGESCROLLER, NULL,
                               WS_VISIBLE | WS_CHILD | PGS_HORZ,
                               0, 0, 200, 32, hDialog, ( HMENU ) IDB_PAGER1,
                               GetModuleHandle( TEXT( "ChangedLines.dll" ) ), NULL );
    // Create Toolbar.  The parent window is the Pager.
    hWndToolbar1 = CreateWindowEx( 0, TOOLBARCLASSNAME, NULL, WS_TOOLBARSTYLE,
                                   0, 0, 200, 32, hWndPager1, ( HMENU ) IDB_TOOLBAR1,
                                   GetModuleHandle( TEXT( "ChangedLines.dll" ) ), NULL );

    SendMessage( hWndToolbar1, TB_BUTTONSTRUCTSIZE, sizeof( TBBUTTON ), 0 );
    SendMessage( hWndToolbar1, TB_SETEXTENDEDSTYLE, 0,
                 ( LPARAM ) TBSTYLE_EX_HIDECLIPPEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS );
    SendMessage( hWndToolbar1, TB_ADDBUTTONS, sizeButtonArray1,
                 ( LPARAM )tbButtonsAdd1 );
    SendMessage( hWndToolbar1, TB_AUTOSIZE, 0, 0 );
    // Notify the pager that it contains the toolbar
    SendMessage( hWndPager1, PGM_SETCHILD, 0, ( LPARAM ) hWndToolbar1 );

    imageToolbar( GetModuleHandle( TEXT( "ChangedLines.dll" ) ), hWndToolbar1,
                  IDB_TOOLBAR1, numButtons1 );

    if ( g_useNppColors )
        SetNppColors();
    else
        SetSysColors();

    ChangeColors();

    refreshDialog();

    HWND hList = GetDlgItem( hDialog, IDC_LSV1 );

    // https://www.codeproject.com/Articles/2890/Using-ListView-control-under-Win32-API
    SendMessage( hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                 ( LVS_EX_FULLROWSELECT /*| LVS_EX_CHECKBOXES*/ ) );

    memset( &LvCol, 0, sizeof( LvCol ) );            // Zero Members
    LvCol.mask    = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM; // Type of mask

    // Column I and W are Index and Working from:
    // https://git-scm.com/docs/git-status
    // LvCol.cx      = 25;                                    // width between each coloum
    // LvCol.pszText = TEXT( "" );                            // First Header Text
    // SendMessage( hList, LVM_INSERTCOLUMN, COL_CHK, ( LPARAM )&LvCol );

    LvCol.cx      =
        50;                                    // width between each coloum
    LvCol.pszText =
        TEXT( "Line" );                           // Second Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, COL_LINE, ( LPARAM )&LvCol );

    LvCol.cx      = 50;                                   // width of column
    LvCol.pszText = TEXT( "Text" );                        // Fourth Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, COL_TEXT, ( LPARAM )&LvCol );

    SendMessage( hList, LVM_SETCOLUMNWIDTH, COL_TEXT,
                 LVSCW_AUTOSIZE_USEHEADER );
    clearList();
}

void getAndGotoLine( int idx )
{
    TCHAR lineno[MAX_PATH] = {0};

    if ( idx == -1 )
        idx = ListView_GetNextItem( GetDlgItem( hDialog, IDC_LSV1 ), -1,
                                    LVIS_FOCUSED );

    memset( &LvItem, 0, sizeof( LvItem ) );
    LvItem.mask       = LVIF_TEXT;
    LvItem.iSubItem   = COL_LINE;
    LvItem.pszText    = lineno;
    LvItem.cchTextMax = MAX_PATH;
    LvItem.iItem      = idx;

    SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_GETITEMTEXT, idx,
                 ( LPARAM )&LvItem );
    gotoLine( std::stoi( lineno ) - 1 );
    PostMessage( nppData._nppHandle, WM_COMMAND, SCEN_SETFOCUS << 16,
                 reinterpret_cast<LPARAM>( getCurScintilla() ) );
}

void toolbarDropdown(LPNMTOOLBAR lpnmtb)
{
    int	i = 0;
    size_t elements = 0;
    std::vector<tDocPos> files;

    if (lpnmtb->iItem == IDC_BTN_PREV) {
        elements = prevPos.size();
        prevPos.list(files);
    } else if (lpnmtb->iItem == IDC_BTN_NEXT) {
        elements = nextPos.size();
        nextPos.list(files);
    }

    POINT pt = {0};
    HMENU hMenu = ::CreatePopupMenu();

    std::wstring menuItem;
    for ( i = 0; i < elements; i++ )
    {
        menuItem = std::to_wstring(files[i].lineNo + 1);
        menuItem += TEXT(": ");
        menuItem += files[i].docName;
        ::AppendMenu(hMenu, MF_STRING, i+1, menuItem.c_str());
    }

    ::GetCursorPos(&pt);
    INT cmd = ::TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hDialog, NULL);
    ::DestroyMenu(hMenu);

    if ( cmd )
        gotoNewPos( files[cmd-1] );
}

INT_PTR CALLBACK DemoDlg::run_dlgProc( UINT message, WPARAM wParam,
                                       LPARAM lParam )
{
    switch ( message )
    {
        case WM_TIMER:
        {
            KillTimer( hDialog, TIMER_CHANGE );
            updateList();
            return FALSE;
        }

        case WM_COMMAND:
        {
            switch ( wParam )
            {
                case IDC_BTN_NEXT:
                {
                    gotoNextPos();
                    return TRUE;
                }

                case IDC_BTN_PREV:
                {
                    gotoPrevPos();
                    return TRUE;
                }

                case IDC_CHK_NPPCOLOR:
                {
                    int check = ( int )::SendMessage( GetDlgItem( hDialog, IDC_CHK_NPPCOLOR ),
                                                      BM_GETCHECK, 0, 0 );

                    if ( check & BST_CHECKED )
                    {
                        SetNppColors();
                        g_useNppColors = true;
                    }
                    else
                    {
                        SetSysColors();
                        g_useNppColors = false;
                    }

                    ChangeColors();
                    refreshDialog();
                    return TRUE;
                }

                case IDC_BTN_CLEARALL:
                {
                    clearAllCF();
                    return TRUE;
                }

                case IDC_BTN_SEARCH:
                {
                    updateList();
                    return TRUE;
                }

                case IDC_BTN_SETTINGS:
                {
                    doSettings();
                    return TRUE;
                }

                // Trap VK_ENTER in the LISTVIEW
                case IDOK :
                {
                    HWND hWndCtrl = GetFocus();

                    if ( hWndCtrl == GetDlgItem( hDialog, IDC_LSV1 ) )
                        getAndGotoLine( -1 );

                    return TRUE;
                }
                // Trap VK_ESCAPE
                case IDCANCEL :
                { 
                    ::SetFocus( getCurScintilla() );
                    return TRUE;
                }
            }

            return FALSE;
        }

        case WM_NOTIFY:
        {
            LPNMHDR nmhdr = ( LPNMHDR )lParam;

            switch ( nmhdr->code )
            {
                case NM_DBLCLK:
                {
                    if ( nmhdr->hwndFrom == GetDlgItem( hDialog, IDC_LSV1 ) )
                    {
                        POINT         pt    = {0};
                        LVHITTESTINFO ht    = {0};
                        DWORD         dwpos = ::GetMessagePos();

                        pt.x = GET_X_LPARAM( dwpos );
                        pt.y = GET_Y_LPARAM( dwpos );

                        ht.pt = pt;
                        ::ScreenToClient( GetDlgItem( hDialog, IDC_LSV1 ), &ht.pt );

                        ListView_SubItemHitTest( GetDlgItem( hDialog, IDC_LSV1 ), &ht );

                        if ( ht.iItem == -1 )
                            break;

                        getAndGotoLine( ht.iItem );
                    }

                    return TRUE;
                }

                case TTN_GETDISPINFO: /* TTN_NEEDTEXT */
                {
                    UINT idButton;
                    LPTOOLTIPTEXT lpttt;

                    lpttt           = ( LPTOOLTIPTEXT ) lParam;
                    lpttt->hinst    = NULL;
                    idButton        = lpttt->hdr.idFrom;
                    lpttt->lpszText = const_cast<LPTSTR>( GetNameStrFromCmd( idButton ) );
                    return TRUE;
                }

                case LVN_KEYDOWN:
                {
                    LPNMLVKEYDOWN pnkd = ( LPNMLVKEYDOWN ) lParam;

                    if ( ( nmhdr->hwndFrom == GetDlgItem( hDialog, IDC_LSV1 ) ) &&
                            ( ( pnkd->wVKey == VK_RETURN )
                              || ( pnkd->wVKey == VK_SPACE )
                            ) )
                        getAndGotoLine( -1 );

                    return FALSE;
                }

                case TBN_DROPDOWN:
                {
                    toolbarDropdown((LPNMTOOLBAR)lParam);
                    return TBDDRET_DEFAULT;
                }
            }

            return FALSE;
        }

        case WM_SIZE:
        case WM_MOVE:
        {
            RECT rc = {0};
            getClientRect( rc );

            ::SetWindowPos( GetDlgItem( hDialog, IDC_LSV1 ), NULL,
                            rc.left + 15, rc.top + 100, rc.right - 25, rc.bottom - 110,
                            SWP_NOZORDER | SWP_SHOWWINDOW );

            SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_SETCOLUMNWIDTH, COL_TEXT,
                         LVSCW_AUTOSIZE_USEHEADER );

            // redraw();
            return FALSE;
        }

        case WM_PAINT:
        {
            ::RedrawWindow( hDialog, NULL, NULL, TRUE );
            return FALSE;
        }

        case WM_INITDIALOG:
        {
            initDialog();
            return TRUE;
        }

        default :
            return DockingDlgInterface::run_dlgProc( message, wParam, lParam );
    }
}
