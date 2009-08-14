/* 
Recordings IMDB info
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
#include <lib/gui/guiactions.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <lib/base/thread.h>
#include <lib/base/estring.h>
#include <lib/dvb/serviceplaylist.h>

#define MOVIEDIR "/media/hdd/movie"

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

class imdbMain: public eWindow
{
private:
	eListBox<eListBoxEntryText> * theList;
	void imdbMain::selectedItem(eListBoxEntryText *item);
	Fetcher theFetcher;
	int inDownloadFlag;
	void imdbMain::downloadDone(int err);
public:
	imdbMain();
	~imdbMain();

};

class eHTTPDownload: public eHTTPDataSource
{
	int received;
	int total;
	int fd;
	eString filename;
public:
	eHTTPDownload(eHTTPConnection *c, const char *filename);
	Signal2<void,int,int> progress; // received, total (-1 for unknown)
	~eHTTPDownload();
	void haveData(void *data, int len);
};

