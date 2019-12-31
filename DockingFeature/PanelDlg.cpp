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
#include "resource.h"

extern NppData nppData;
extern HWND hDialog;

extern bool g_enabled;
extern bool g_GotoIncSave;
extern int  g_Width;
extern long g_ChangeColor;
extern long g_SaveColor;
extern int  g_ChangeMarkStyle;
extern int  g_SaveMarkStyle;

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

void refreshDialog()
{
    SendMessage( GetDlgItem( hDialog, IDC_CHK1 ), BM_SETCHECK, ( WPARAM )( g_enabled ? 1 : 0 ), 0 );
    SendMessage( GetDlgItem( hDialog, IDC_CHK2 ), BM_SETCHECK, ( WPARAM )( g_GotoIncSave ? 1 : 0 ), 0 );

    TCHAR strHint[500] = {0};
    wsprintf( strHint, TEXT( "%d" ), g_Width );
    SendMessage( GetDlgItem( hDialog, IDC_EDT1 ), WM_SETTEXT, 0, ( LPARAM )strHint );

    SendMessage( GetDlgItem( hDialog, IDC_CBO1 ), CB_SETCURSEL, getMarkerType( g_ChangeMarkStyle ), 0 );
    SendMessage( GetDlgItem( hDialog, IDC_CBO2 ), CB_SETCURSEL, getMarkerType( g_SaveMarkStyle ), 0 );
}

void initDialog()
{
    SendMessage( GetDlgItem( hDialog, IDC_CBO1 ), CB_ADDSTRING, 0, ( LPARAM )TEXT( "Default" ) );
    SendMessage( GetDlgItem( hDialog, IDC_CBO1 ), CB_ADDSTRING, 0, ( LPARAM )TEXT( "Arrow" ) );
    SendMessage( GetDlgItem( hDialog, IDC_CBO1 ), CB_ADDSTRING, 0, ( LPARAM )TEXT( "Highlight" ) );

    SendMessage( GetDlgItem( hDialog, IDC_CBO2 ), CB_ADDSTRING, 0, ( LPARAM )TEXT( "Default" ) );
    SendMessage( GetDlgItem( hDialog, IDC_CBO2 ), CB_ADDSTRING, 0, ( LPARAM )TEXT( "Arrow" ) );
    SendMessage( GetDlgItem( hDialog, IDC_CBO2 ), CB_ADDSTRING, 0, ( LPARAM )TEXT( "Highlight" ) );

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
                case IDC_CHK1 :
                {
                    doEnable();
                    return TRUE;
                }
                case IDC_BTN1 :
                {
                    gotoNextChange();
                    return TRUE;
                }
                case IDC_BTN2 :
                {
                    gotoPrevChange();
                    return TRUE;
                }
                case IDC_CHK2 :
                {
                    int check = ( int )::SendMessage( GetDlgItem( hDialog, IDC_CHK2 ), BM_GETCHECK, 0, 0 );
                    
                    if( check & BST_CHECKED )
                        g_GotoIncSave = true;
                    else
                        g_GotoIncSave = false;

                    return TRUE;
                }
                case IDC_BTN3 :
                {
                    clearAllCF();
                    return TRUE;
                }

                case MAKELONG( IDC_EDT1, EN_CHANGE ) :
                {
                    BOOL isSuccessful;
                    int val = ( int )::GetDlgItemInt( _hSelf, IDC_EDT1, &isSuccessful, FALSE );

                    if ( val >= 1 && val <= 100 )
                    {
                        g_Width = val;
                        updateWidth();
                    }
                    return TRUE;
                }
                case IDC_BTN4 :
                {
                    COLORREF rgbCustom[16] = {0};
                    CHOOSECOLOR cc = {sizeof( CHOOSECOLOR )};
                
                    cc.Flags        = CC_RGBINIT | CC_FULLOPEN /* | CC_ANYCOLOR */;
                    cc.hwndOwner    = hDialog;
                    cc.rgbResult    = g_ChangeColor;
                    cc.lpCustColors = rgbCustom;

                    if ( ChooseColor( &cc ) )
                    {
                        g_ChangeColor = cc.rgbResult;
                        updateChangeColor();
                    }

                    return TRUE;
                }
                case IDC_BTN6 :
                {
                    g_ChangeColor = DefaultChangeColor;
                    updateChangeColor();
                    return TRUE;
                }
                case MAKELONG( IDC_CBO1, CBN_SELCHANGE ):
                {
                    int markType = ( int )::SendMessage( GetDlgItem( hDialog, IDC_CBO1 ), CB_GETCURSEL, 0, 0 );
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
                        refreshDialog();
                    }
                        
                    return TRUE;
                }

                case IDC_BTN5 :
                {
                    COLORREF rgbCustom[16] = {0};
                    CHOOSECOLOR cc = {sizeof( CHOOSECOLOR )};
                
                    cc.Flags        = CC_RGBINIT | CC_FULLOPEN /* | CC_ANYCOLOR */;
                    cc.hwndOwner    = hDialog;
                    cc.rgbResult    = g_SaveColor;
                    cc.lpCustColors = rgbCustom;

                    if ( ChooseColor( &cc ) )
                    {
                        g_SaveColor = cc.rgbResult;
                        updateSaveColor();
                    }

                    return TRUE;
                }
                case IDC_BTN7 :
                {
                    g_SaveColor = DefaultSaveColor;
                    updateSaveColor();
                    return TRUE;
                }
                case MAKELONG( IDC_CBO2, CBN_SELCHANGE ):
                {
                    int markType = ( int )::SendMessage( GetDlgItem( hDialog, IDC_CBO2 ), CB_GETCURSEL, 0, 0 );
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
                        refreshDialog();
                    }
                        
                    return TRUE;
                }

            }
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
