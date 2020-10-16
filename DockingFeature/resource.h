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

#ifndef DOCKINGFEATURE_RESOURCE_H
#define DOCKINGFEATURE_RESOURCE_H


#ifndef IDC_STATIC
#define IDC_STATIC  -1
#endif

// Docking
#define IDD_PLUGINPANEL 2500

#define IDI_PLUGINPANEL 2501

#define IDC_CHK_ENABLED        (IDD_PLUGINPANEL + 201)
#define IDC_CHK_INCSAVES       (IDD_PLUGINPANEL + 203)
#define IDC_LSV1               (IDD_PLUGINPANEL + 204)

#define IDB_TOOLBAR1    1600
#define IDB_PAGER1      1701

#define IDC_BTN_PREV           (IDB_TOOLBAR1 + 1)
#define IDC_BTN_NEXT           (IDB_TOOLBAR1 + 2)
#define IDC_BTN_CLEARALL       (IDB_TOOLBAR1 + 3)
#define IDC_BTN_SEARCH         (IDB_TOOLBAR1 + 4)
#define IDC_BTN_SETTINGS       (IDB_TOOLBAR1 + 5)

// Settings
#define IDD_SETTINGS      2600

#define IDB_OK                 (IDD_SETTINGS + 1)

#define IDC_EDT_WIDTH          (IDD_SETTINGS + 2)
#define IDC_CHK_NPPCOLOR       (IDD_SETTINGS + 3)

#define IDC_GRP1               (IDD_SETTINGS + 4)
#define IDC_BTN_COLORCHANGE    (IDD_SETTINGS + 5)
#define IDC_GRP_BTNCC          (IDD_SETTINGS + 6)
#define IDC_BTN_COLORCHANGEDEF (IDD_SETTINGS + 7)
#define IDC_CBO_MARKCHANGE     (IDD_SETTINGS + 8)

#define IDC_GRP2               (IDD_SETTINGS + 9)
#define IDC_BTN_COLORSAVE      (IDD_SETTINGS + 10)
#define IDC_GRP_BTNCS          (IDD_SETTINGS + 11)
#define IDC_BTN_COLORSAVEDEF   (IDD_SETTINGS + 12)
#define IDC_CBO_MARKSAVE       (IDD_SETTINGS + 13)

#endif // DOCKINGFEATURE_RESOURCE_H
