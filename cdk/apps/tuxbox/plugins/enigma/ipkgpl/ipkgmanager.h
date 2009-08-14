/*	Ipkgpl - Ipkg Enigma Plugin

	Copyright (C) 2005 'mechatron' (mechatron@gmx.net)

	Homepage: http://mechatron.6x.to/

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#include <lib/gui/ewindow.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/emessage.h>
#include <lib/gui/listbox.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/eskin.h>
#include <lib/system/econfig.h>
#include <lib/gdi/font.h>
#include <lib/base/console.h>

#include "ipkg.h"
#include "ipkgsetup.h"

class eListBoxEntryIpkg: public eListBoxEntryText
{
	//friend class eListBoxExt<eListBoxEntryIpkg>;
	friend struct moveFirstChar;
	eTextPara *namePara;
public:
	eString name;
	int menu, check1, check2, count;
	eListBoxEntryIpkg(eListBox<eListBoxEntryIpkg> *listbox, eString name, int menu, int check1, int check2, int count)
		: eListBoxEntryText((eListBox<eListBoxEntryText> *)listbox, name), name(name), menu(menu), check1(check1), check2(check2), namePara(0), count(count)
	{
	}
	~eListBoxEntryIpkg();
	void invalidate();
protected:
	const eString &redraw(gPainter *rc, const eRect &rect, gColor, gColor, gColor, gColor, int state);
};

class IPKG: public eWindow
{
private:
	eLabel *l_a, *l_b, *l_c, *l_d;
	eListBox<eListBoxEntryIpkg> *t_liste;
	eStatusBar *status;
	CIPKG *m_ipkg;
	eConsoleAppContainer *commCont;
	eMessageBox *ipkg_msg;
	int menu;
	char BrowseChar;

	void liste_fill();
	void T_Listeselected(eListBoxEntryIpkg *item);
	void T_Listeselchanged(eListBoxEntryIpkg *item);
	void setStatus(int val);
	void setLabel();
	int eventHandler( const eWidgetEvent &e );
	void comm_exe();
	void stop_cont(int);
	void get_data( eString str );
	void gotoChar(char c);
	void quickupgrade();
public:
	IPKG();
	~IPKG();
};

