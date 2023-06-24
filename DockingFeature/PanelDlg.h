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

#ifndef PANEL_DLG_H
#define PANEL_DLG_H

#include "DockingDlgInterface.h"
#include "resource.h"

#include <commctrl.h>

class DemoDlg : public DockingDlgInterface
{
public :
	DemoDlg() : DockingDlgInterface(IDD_PLUGINPANEL){};

    virtual void display(bool toShow = true) const {
        DockingDlgInterface::display(toShow);
    };

	void setParent(HWND parent2set){
		_hParent = parent2set;
	};

	bool isFloating() const {
		return _isFloating;
	};

    void updateListTimer();

    TCHAR addInfo[32];

protected :
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private :
    void clearList();
    void setListColumns( unsigned int uItem, std::wstring strLine, std::wstring strText );
    std::wstring stringToWstring( const std::string &t_str );
    void updateList();
    void refreshDialog();
    void SetNppColors();
    void SetSysColors();
    void ChangeColors();
    void initDialog();
    void getAndGotoLine( int idx );
    void toolbarDropdown(LPNMTOOLBAR lpnmtb);
};

#endif //PANEL_DLG_H
