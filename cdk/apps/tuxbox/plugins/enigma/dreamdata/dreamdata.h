/*
	DreamData - Enigma Plugin

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

#include <lib/gui/emessage.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/statusbar.h>
#include <src/enigma_main.h>
#include <src/enigma_processutils.h>

struct dataservice {
	eString name;
	eString conf[5];
	int sid, tsid;
	int pid[5];
	bool pid_set[5];
};

class eListBoxEntryData: public eListBoxEntryText
{
public:
	eString name;
	dataservice m_dataservice;
	eListBoxEntryData(eListBox<eListBoxEntryData> *listbox,eString name,dataservice m_dataservice)
	: eListBoxEntryText((eListBox<eListBoxEntryText> *)listbox, name), name(name), m_dataservice(m_dataservice)
	{
	}
};

class dreamdata: public eWindow
{
	eString dvb_ip[5], dvb_mtu[5];
	bool con;
	dataservice m_dataservice, old_dataservice;

	eListBox<eListBoxEntryData> *p_liste;
	eStatusBar *status;
	eServiceReference s_zap;

	int add_net(int dev, int dpid);
	int del_net();
	void p_listeselected(eListBoxEntryData *item);
	void showPic();
	void read_xml(eString file);
	void show_error(eString mess);
	int eventHandler( const eWidgetEvent &e );
public:
	dreamdata();
};
