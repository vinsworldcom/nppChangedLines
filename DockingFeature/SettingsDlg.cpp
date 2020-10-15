#include <windows.h>
#include <shlobj.h>
#include <string>

#include "..\PluginDefinition.h"
#include "..\PluginInterface.h"
#include "SettingsDlg.h"
#include "resource.h"
#include "..\resource.h"

extern HINSTANCE g_hInst;
extern NppData   nppData;

extern int  g_Width;
extern long g_ChangeColor;
extern long g_SaveColor;
extern int  g_ChangeMarkStyle;
extern int  g_SaveMarkStyle;
extern bool g_GotoIncSave;

HBRUSH ghButtonColor;

int getMarkerType( int marker )
{
    switch ( marker )
    {
        case SC_MARK_FULLRECT:
            return Default;

        case SC_MARK_ARROW:
            return Arrow;

        case SC_MARK_BACKGROUND:
            return Highlight;

        default:
            return -1;
    }
}

void refreshSettings( HWND hWndDlg )
{
    SendMessage( GetDlgItem( hWndDlg, IDC_CHK_INCSAVES ), BM_SETCHECK,
                 ( WPARAM )( g_GotoIncSave ? 1 : 0 ), 0 );

    TCHAR strHint[500] = {0};
    wsprintf( strHint, TEXT( "%d" ), g_Width );
    SendMessage( GetDlgItem( hWndDlg, IDC_EDT_WIDTH ), WM_SETTEXT, 0,
                 ( LPARAM )strHint );

    SendMessage( GetDlgItem( hWndDlg, IDC_CBO_MARKCHANGE ), CB_SETCURSEL,
                 getMarkerType( g_ChangeMarkStyle ), 0 );
    SendMessage( GetDlgItem( hWndDlg, IDC_CBO_MARKSAVE ), CB_SETCURSEL,
                 getMarkerType( g_SaveMarkStyle ), 0 );
}

INT_PTR CALLBACK SettingsDlg( HWND hWndDlg, UINT msg, WPARAM wParam,
                              LPARAM lParam )
{

    switch ( msg )
    {
        case WM_INITDIALOG:
        {
            HWND change = GetDlgItem( hWndDlg, IDC_CBO_MARKCHANGE );
            HWND save   = GetDlgItem( hWndDlg, IDC_CBO_MARKSAVE );

            SendMessage( change, CB_ADDSTRING, 0, ( LPARAM )TEXT( "Default" ) );
            SendMessage( change, CB_ADDSTRING, 0, ( LPARAM )TEXT( "Arrow" ) );
            SendMessage( change, CB_ADDSTRING, 0, ( LPARAM )TEXT( "Highlight" ) );

            SendMessage( save, CB_ADDSTRING, 0, ( LPARAM )TEXT( "Default" ) );
            SendMessage( save, CB_ADDSTRING, 0, ( LPARAM )TEXT( "Arrow" ) );
            SendMessage( save, CB_ADDSTRING, 0, ( LPARAM )TEXT( "Highlight" ) );

            std::string version;
            version = "<a>";
            version += VER_STRING;
            version += "</a>";
            SetDlgItemTextA(hWndDlg, IDC_STC_VER, version.c_str());

            refreshSettings( hWndDlg );

            return TRUE;
        }

        case WM_CLOSE:
        {
            PostMessage( hWndDlg, WM_DESTROY, 0, 0 );
            return TRUE;
        }

        case WM_DESTROY:
        {
            EndDialog( hWndDlg, 0 );
            DeleteObject( ghButtonColor );
            return TRUE;
        }

        case WM_CTLCOLORBTN:
        {
            if ( ( HWND )lParam == GetDlgItem( hWndDlg, IDC_BTN_COLORCHANGE ) )
            {
                ghButtonColor = CreateSolidBrush( g_ChangeColor );
                return ( INT_PTR )ghButtonColor;
            }
            else if ( ( HWND )lParam == GetDlgItem( hWndDlg, IDC_BTN_COLORSAVE ) )
            {
                ghButtonColor = CreateSolidBrush( g_SaveColor );
                return ( INT_PTR )ghButtonColor;
            }

            return FALSE;
        }

        case WM_NOTIFY:
        {
            switch (((LPNMHDR)lParam)->code)
            {
                case NM_CLICK:
                case NM_RETURN:
                {
                    PNMLINK pNMLink = (PNMLINK)lParam;
                    LITEM   item    = pNMLink->item;
                    HWND ver = GetDlgItem( hWndDlg, IDC_STC_VER );

                    if ((((LPNMHDR)lParam)->hwndFrom == ver) && (item.iLink == 0))
                        ShellExecute(hWndDlg, TEXT("open"), TEXT("https://github.com/VinsWorldcom/nppChangedLines"), NULL, NULL, SW_SHOWNORMAL);

                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            switch ( wParam )
            {
                case IDB_OK:
                    PostMessage( hWndDlg, WM_CLOSE, 0, 0 );
                    return TRUE;

                case IDC_CHK_INCSAVES:
                {
                    int check = ( int )::SendMessage( GetDlgItem( hWndDlg, IDC_CHK_INCSAVES ),
                                                      BM_GETCHECK, 0, 0 );

                    if ( check & BST_CHECKED )
                        g_GotoIncSave = true;
                    else
                        g_GotoIncSave = false;

                    updatePanel();

                    return TRUE;
                }

                case MAKELONG( IDC_EDT_WIDTH, EN_CHANGE ) :
                {
                    BOOL isSuccessful;
                    int val = ( int )::GetDlgItemInt( hWndDlg, IDC_EDT_WIDTH, &isSuccessful,
                                                      FALSE );

                    if ( val >= 1 && val <= 100 )
                    {
                        g_Width = val;
                        updateWidth();
                    }

                    return TRUE;
                }

                case IDC_BTN_COLORCHANGE :
                {
                    COLORREF rgbCustom[16] = {0};
                    CHOOSECOLOR cc = {sizeof( CHOOSECOLOR )};

                    cc.Flags        = CC_RGBINIT | CC_FULLOPEN /* | CC_ANYCOLOR */;
                    cc.hwndOwner    = hWndDlg;
                    cc.rgbResult    = g_ChangeColor;
                    cc.lpCustColors = rgbCustom;

                    if ( ChooseColor( &cc ) )
                    {
                        g_ChangeColor = cc.rgbResult;
                        updateChangeColor();
                        InvalidateRect( GetDlgItem( hWndDlg, IDC_BTN_COLORCHANGE ), NULL, TRUE );
                    }

                    return TRUE;
                }

                case IDC_BTN_COLORCHANGEDEF :
                {
                    g_ChangeColor = DefaultChangeColor;
                    updateChangeColor();
                    InvalidateRect( GetDlgItem( hWndDlg, IDC_BTN_COLORCHANGE ), NULL, TRUE );
                    return TRUE;
                }

                case MAKELONG( IDC_CBO_MARKCHANGE, CBN_SELCHANGE ):
                {
                    int markType = ( int )::SendMessage( GetDlgItem( hWndDlg,
                                                         IDC_CBO_MARKCHANGE ), CB_GETCURSEL, 0, 0 );

                    if ( markType >= 0 && markType <= N_ELEMS( MarkTypeArr ) )
                    {
                        g_ChangeMarkStyle = MarkTypeArr[markType];

                        if ( markType == Arrow )
                        {
                            if ( g_Width < DefaultArrowWidth )
                            {
                                g_Width = DefaultArrowWidth;
                                updateWidth();
                            }
                        }

                        updateChangeStyle();
                        refreshSettings( hWndDlg );
                    }

                    return TRUE;
                }

                case IDC_BTN_COLORSAVE :
                {
                    COLORREF rgbCustom[16] = {0};
                    CHOOSECOLOR cc = {sizeof( CHOOSECOLOR )};

                    cc.Flags        = CC_RGBINIT | CC_FULLOPEN /* | CC_ANYCOLOR */;
                    cc.hwndOwner    = hWndDlg;
                    cc.rgbResult    = g_SaveColor;
                    cc.lpCustColors = rgbCustom;

                    if ( ChooseColor( &cc ) )
                    {
                        g_SaveColor = cc.rgbResult;
                        updateSaveColor();
                        InvalidateRect( GetDlgItem( hWndDlg, IDC_BTN_COLORSAVE ), NULL, TRUE );
                    }

                    return TRUE;
                }

                case IDC_BTN_COLORSAVEDEF :
                {
                    g_SaveColor = DefaultSaveColor;
                    updateSaveColor();
                    InvalidateRect( GetDlgItem( hWndDlg, IDC_BTN_COLORSAVE ), NULL, TRUE );
                    return TRUE;
                }

                case MAKELONG( IDC_CBO_MARKSAVE, CBN_SELCHANGE ):
                {
                    int markType = ( int )::SendMessage( GetDlgItem( hWndDlg,
                                                         IDC_CBO_MARKSAVE ), CB_GETCURSEL, 0, 0 );

                    if ( markType >= 0 && markType <= N_ELEMS( MarkTypeArr ) )
                    {
                        g_SaveMarkStyle = MarkTypeArr[markType];

                        if ( markType == Arrow )
                        {
                            if ( g_Width < DefaultArrowWidth )
                            {
                                g_Width = DefaultArrowWidth;
                                updateWidth();
                            }
                        }

                        updateSaveStyle();
                        refreshSettings( hWndDlg );
                    }

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

void doSettings()
{
    DialogBoxParam( g_hInst, MAKEINTRESOURCE( IDD_SETTINGS ),
                    nppData._nppHandle, SettingsDlg, 0 );
}
