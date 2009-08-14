/* 
LAN-Control Plugin for Enigma 1
Copyright (C) 2007 Dre (dre@drecomx.net) [http://dreambox.funfiles.cc]

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef DISABLE_NETWORK

#ifndef __lancontrol_h
#define __lancontrol_h

#include <plugin.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <dirent.h> // needed for searching keys in /var/etc or /usr/etc
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/eskin.h>
#include <lib/gui/listbox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/textinput.h>
#include <lib/gui/elabel.h>
#include <lib/gui/combobox.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/epgcache.h>
#include <lib/socket/socket.h>
#include <lib/base/estring.h>
#include <lib/base/console.h>
#include <lib/system/info.h> // needed for model detection

#include <setupnetwork.h> // needed for calling network settings
#include <setupengrab.h> // needed for calling ngrab settings
#include <setup_mounts.h>

class eWake: public eWindow
{
	eButton *bt_STOP, *bt_MAC, *bt_OK, *bt_START, *bt_STATUS, *bt_PREV, *bt_NEXT, *bt_MOUNTS, *bt_NET, *bt_NGRAB, *bt_CONFIG;

	eLabel *IP, *MAC, *OUTPUT;
	eNumber *inet_address;
	eStatusBar *statusbar;
	eTextInputField *serverMAC;
	eString cmd, headline, path;
	int pageHeight;
	int total;
	int curEntry;
	void getConf();
	void prevPressed();
	void nextPressed();
	void saveSettings();
	void communication_setup();
	void communication_setup2();
	void ngrab_setup();
	void showSettingsMenu();

private:
	void detectMAC();
	void startServer();
	void stopServer();
	void status();

public:
	eWake();
	~eWake();
};

class eWakeConf: public eWindow
{
	eLabel *lb_SERVER_OS, *lb_PROFILE_NO, *lb_USER, *lb_RSA, *lb_dummy;
	eComboBox *cbb_SERVER_OS, *cbb_PROFILE_NO, *cbb_RSA;
	eTextInputField *tf_USER;
	eStatusBar *statusbar;
	eButton *bt_SCRIPT;

	eTimer *loadTimer;

	int profileno, keystate;
	eString cmd, ipaddress, shutdown, path, key, user, keys[30];

	void changeOS(eListBoxEntryText *os);
	void changeProfile(eListBoxEntryText *profile);
	void getServerIP();
	void writeScript();
	void getStoredKeys();

public:
	eWakeConf(int curEntry);
	~eWakeConf();
};

class eWakeHelper
{
public:
	eString path;
	const eString& getBoxType();
};


#endif

#endif // DISABLE_NETWORK

