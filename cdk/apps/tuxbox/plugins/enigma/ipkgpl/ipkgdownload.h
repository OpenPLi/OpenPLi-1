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

#ifndef __down__load__
#define __down__load__

#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/eskin.h>
#include <lib/system/httpd.h>

class eHTTPDownload: public eHTTPDataSource
{
	int received,total,fd;
	eString filename;
public:
	eHTTPDownload(eHTTPConnection *c, const char *filename);
	Signal2<void,int,int> progress;
	~eHTTPDownload();
	void haveData(void *data, int len);
};

class eDownload: public eWindow
{
	eHTTPConnection *http;
	eHTTPDownload *file_load;

	eString urlfull, tar;
	bool auto_start;
	int lasttime;
	eStatusBar *status;
	eProgress *progress;
	eLabel *progresstext;
	eButton *abort_bt, *start_bt;
	eTimer *timer;

	eHTTPDataSource *create_datasink(eHTTPConnection *conn);
	void transfer_done(int err);
	void download_start();
	void download_abort();
	void downloadProgress(int received, int total);
public:
	eDownload(eString url, eString target, bool autostart);
	~eDownload();
};

#endif


