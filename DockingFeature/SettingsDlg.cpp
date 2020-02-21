#include <windows.h>
#include <shlobj.h>

#include "..\PluginDefinition.h"
#include "..\PluginInterface.h"
#include "PanelDlg.h"
#include "SettingsDlg.h"
#include "resource.h"

extern HINSTANCE g_hInst;
extern NppData   nppData;

extern int  g_Width;
extern long g_ChangeColor;
extern long g_SaveColor;
extern int  g_ChangeMarkStyle;
extern int  g_SaveMarkStyle;
extern bool g_useNppColors;

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
    TCHAR strHint[500] = {0};
    wsprintf( strHint, TEXT( "%d" ), g_Width );
    SendMessage( GetDlgItem( hWndDlg, IDC_EDT_WIDTH ), WM_SETTEXT, 0, ( LPARAM )strHint );

    SendMessage( GetDlgItem( hWndDlg, IDC_CBO_MARKCHANGE ), CB_SETCURSEL, getMarkerType( g_ChangeMarkStyle ), 0 );
    SendMessage( GetDlgItem( hWndDlg, IDC_CBO_MARKSAVE ), CB_SETCURSEL, getMarkerType( g_SaveMarkStyle ), 0 );
}

INT_PTR CALLBACK SettingsDlg(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ::SendMessage( GetDlgItem( hWndDlg, IDC_CHK_NPPCOLOR ), BM_SETCHECK,
                   ( LPARAM )( g_useNppColors ? 1 : 0 ), 0 );

    switch(msg)
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

            refreshSettings( hWndDlg );

            return TRUE;
        }

        case WM_CLOSE:
        {
            PostMessage(hWndDlg, WM_DESTROY, 0, 0);
            return TRUE;
        }

        case WM_DESTROY:
        {
            EndDialog(hWndDlg, 0);
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (wParam)
            {
                case IDB_OK:
                    PostMessage(hWndDlg, WM_CLOSE, 0, 0);
                    return TRUE;

                case IDC_CHK_NPPCOLOR :
                {
                    if ( SendMessage( GetDlgItem( hWndDlg, IDC_CHK_NPPCOLOR ), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
                    {
                      SetSysColors();
                      g_useNppColors = false;
                    }
                    else
                    {
                      SetNppColors();
                      g_useNppColors = true;
                    }
                    ChangeColors();
                    return TRUE;
                }

                case MAKELONG( IDC_EDT_WIDTH, EN_CHANGE ) :
                {
                    BOOL isSuccessful;
                    int val = ( int )::GetDlgItemInt( hWndDlg, IDC_EDT_WIDTH, &isSuccessful, FALSE );

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
                    }

                    return TRUE;
                }
                case IDC_BTN_COLORCHANGEDEF :
                {
                    g_ChangeColor = DefaultChangeColor;
                    updateChangeColor();
                    return TRUE;
                }
                case MAKELONG( IDC_CBO_MARKCHANGE, CBN_SELCHANGE ):
                {
                    int markType = ( int )::SendMessage( GetDlgItem( hWndDlg, IDC_CBO_MARKCHANGE ), CB_GETCURSEL, 0, 0 );
                    if ( markType >= 0 && markType <= N_ELEMS(MarkTypeArr) )
                    {
                        g_ChangeMarkStyle = MarkTypeArr[markType];
                        if ( markType == Arrow )
                        {
                            if ( g_Width < DefaultArrowWidth ){
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
                    }

                    return TRUE;
                }
                case IDC_BTN_COLORSAVEDEF :
                {
                    g_SaveColor = DefaultSaveColor;
                    updateSaveColor();
                    return TRUE;
                }
                case MAKELONG( IDC_CBO_MARKSAVE, CBN_SELCHANGE ):
                {
                    int markType = ( int )::SendMessage( GetDlgItem( hWndDlg, IDC_CBO_MARKSAVE ), CB_GETCURSEL, 0, 0 );
                    if ( markType >= 0 && markType <= N_ELEMS(MarkTypeArr) )
                    {
                        g_SaveMarkStyle = MarkTypeArr[markType];
                        if ( markType == Arrow )
                        {
                            if ( g_Width < DefaultArrowWidth ){
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
    DialogBoxParam( g_hInst, MAKEINTRESOURCE( IDD_SETTINGS ), nppData._nppHandle, SettingsDlg, 0 );
}
