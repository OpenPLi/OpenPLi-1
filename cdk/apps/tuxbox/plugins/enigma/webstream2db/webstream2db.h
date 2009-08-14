/* 
Dreambox webstream2db reader
Copyright (C) 2006 Aux

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
#include <lib/gdi/font.h>
#include <lib/system/httpd.h>
#include <lib/gui/eprogress.h>
#include <upgrade.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <lib/base/thread.h>
#include <lib/base/estring.h>
#include <enigma_dyn_utils.h> //included for getIP function
#include <lib/movieplayer/movieplayer.h> //the movieplayer plugin is needed and contains the IP of the server, so no need for using our own config here...
#include <lib/movieplayer/mpconfig.h>
#include <enigma_dyn_movieplayer.h>

#define KEY_STATE_DOWN          0
#define KEY_STATE_REPEAT        eRCKey::flagRepeat
#define KEY_STATE_UP            eRCKey::flagBreak
#define RC_UP           33
#define RC_DN           34


struct NewsItem {
	int id;
	eString title;
	eString description;
};

struct ConfigItem {
	int id;
	eString name;
	eString url;
};

class Fetcher : public Object
{	eString tempPath;
	eHTTPConnection * connectionP;
	eHTTPDataSource * dataSinkP;
	
	eString url;

	void transferDone(int err);
	eHTTPDataSource * createDownloadSink(eHTTPConnection *conn);	

public:
	Signal1<void,int> downloadDone;
	void fetch(eString url);
};

class webstream2dbParser : public Object
{	void getEmbeddedHtml(XMLTreeNode *i, eString &desc);

public:
	std::list<NewsItem> newsItems;
	void parse(eString file);
	void save(NewsItem i);
};

class ConfigParser : public Object
{	

public:
	std::list<ConfigItem> configItems;
	void parse(eString file);
	void save(ConfigItem i);
};

class webstream2dbFeed: public eWindow
{

private:
	eListBox<eListBoxEntryText> *theList;
	webstream2dbParser thewebstream2dbParser;
	public:
	// the constructor.
	webstream2dbFeed();
	void webstream2dbFeed::selectedItem(eListBoxEntryText *item);
	void downloadDone(int err);	
	void printwebstream2dbFeed(eString title);
};

class webstream2dbDetail: public eWindow
{
	int eventHandler(const eWidgetEvent &event);
	eLabel *descrLabel;
	eWidget *descrWidget;
	int total;
	
	eProgress *scrollbar;
	void updateScrollbar( void );
	void handleDown( void );
	void handleUp( void );

	void redrawWidget(gPainter *d, const eRect &area);
public: 
	webstream2dbDetail(const char *title, const char *desc);
};

// MAIN
class webstream2dbMain: public eWindow
{

private:
	eListBox<eListBoxEntryText> *theList;
	ConfigParser theConfigParser;
	eLabel *thelabel;
	Fetcher theFetcher;	
	void printFeeds();
	webstream2dbFeed * thewebstream2dbFeed;
	int downloadDoneFlag;
	int inDownloadFlag;
	eString currentName;
public:
	// the constructor.
	webstream2dbMain();
	// the destructor.
	~webstream2dbMain();
	void selectedItem(eListBoxEntryText *item);
	void downloadDone(int err);	
};

