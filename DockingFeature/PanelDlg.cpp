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
extern bool g_enabled;
extern HWND hDialog;

INT_PTR CALLBACK DemoDlg::run_dlgProc( UINT message, WPARAM wParam,
                                       LPARAM lParam )
{

    ::SendMessage( GetDlgItem( hDialog, IDC_CHK1 ), BM_SETCHECK, ( LPARAM )( g_enabled ? 1 : 0 ), 0 );

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
                case IDC_BTN3 :
                {
                    clearAllCF();
                    return TRUE;
                }
            }
            return FALSE;
        }

        default :
            return DockingDlgInterface::run_dlgProc( message, wParam, lParam );
    }
}

