/* 
Dreambox weather plugin
Copyright (C) 2004 Bjorn Hijmans (bjorn@hijmans.nl)

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

#include <stdio.h>
#include <stdlib.h>
#include <plugin.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/textinput.h>
#include <lib/gui/combobox.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/eskin.h>
#include <lib/system/httpd.h>
#include <lib/gdi/gfbdc.h>
#include <upgrade.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <lib/base/thread.h>

struct WeatherItem {
	eString date;
	int min_temp;
	int max_temp;
	eString description;
};

struct LocationItem {
	int id;
	eString name;
	eString url;
};

struct ConfigItem {
	eString origdescription;
	eString description;
	eString icon;
};

class Fetcher: public Object
{	eString tempPath;

	eHTTPConnection * connectionP;
	eHTTPDataSource * dataSinkP;

	void transferDone(int err);
	eHTTPDataSource * createDownloadSink(eHTTPConnection *conn);	
	void connectionTimeouted();
	
	int timerExpired;
public:
	Signal1<void,int> downloadDone;
	Signal1<void,int> downloadTimeout;
	void fetch();
	Fetcher(eString url);
	eString url;
	eTimer timeout;
};

class ConfigParser: public Object
{	
public:
	std::list<ConfigItem> configItems;
	std::list<LocationItem> locationItems;
	ConfigParser();
	void LookUp(eString orig, eString &desc, eString &icon);
	eString url;
	eString name;
};

class locationSelect: public eWindow
{
private:
	ConfigParser * theConfigParser;
        eListBox<eListBoxEntryText> * theList;
        
public:
	locationSelect(ConfigParser * t);
	//~locationSelect();
	void selectedItem(eListBoxEntryText *item);
	eString selectedURL;
	eString selectedName;
	void setLBFocus();
};

class weatherMain: public eWindow
{
private:
	ConfigParser * theConfigParser;
	Fetcher * theFetcher;
	void parse(eString url);
	std::list<WeatherItem> weatherItems;
	int getTemp(eString description, eString key);
	void getDescription(eString description, eString * out);
	eLabel *lb1, *lb2, *lb3, *lb4, *lb5, *lb6, *lb7;
	eButton *btn1;
	void dispData();
	locationSelect * theLocationSelect;
                                	                                
public:
	weatherMain();
	~weatherMain();
	void downloadDone(int err);
	void downloadTimeout(int err);
	void selectLocation();
};
