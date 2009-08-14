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

#include "ipkgdownload.h"
#include "ipkg.h"

eHTTPDownload::eHTTPDownload(eHTTPConnection *c, const char *filename): eHTTPDataSource(c), filename(filename)
{
	if (c->remote_header.count("Content-Length")) total=atoi(c->remote_header["Content-Length"].c_str());
	else total=-1;

	received=0;
	fd=::creat(filename, 0777);
	progress(received, total);
}

eHTTPDownload::~eHTTPDownload()
{
	if (fd >= 0) ::close(fd);
	if ((total != -1) && (total != received)) ::unlink(filename.c_str());
}

void eHTTPDownload::haveData(void *data, int len)
{
	if (len)
	{
		if (fd >= 0) ::write(fd, data, len);
	}
	received+=len;
	progress(received, total);
}

eHTTPDataSource *eDownload::create_datasink(eHTTPConnection *conn)
{
	file_load = new eHTTPDownload(conn, tar.c_str());
	lasttime=0;
	CONNECT(file_load->progress, eDownload::downloadProgress);
	return file_load;
}


eDownload::eDownload(eString url, eString target, bool autostart)
:http(0)
{
	int fd=eSkin::getActive()->queryValue("fontsize", 20);
	gFont fontsmall = eSkin::getActive()->queryFont("epg.title");
	cmove(ePoint(70, 180)); cresize(eSize(580, 210)); setText("Download");

	urlfull = url;
	tar = target;
	auto_start = autostart;

	write_ipkg_log_file("Download %s\nto %s\n", urlfull.c_str(),  tar.c_str());

	eLabel *la = new eLabel(this);
	la->move(ePoint(10, 50));
	la->resize(eSize(clientrect.width()-20,fd+10));
	la->setFont(fontsmall);
	la->setText("url: " + urlfull);

	start_bt=new eButton(this);
	start_bt->move(ePoint(10, 10));
	start_bt->resize(eSize(200, fd+10));
	start_bt->setText(_("Start"));
	start_bt->setHelpText(_("press ok to start download"));
	start_bt->setShortcut("green");
	start_bt->setShortcutPixmap("green");
	start_bt->loadDeco();
	CONNECT(start_bt->selected, eDownload::download_start);

	abort_bt=new eButton(this);
	abort_bt->move(ePoint(10, 10));
	abort_bt->resize(eSize(200, fd+10));
	abort_bt->setShortcut("red");
	abort_bt->setShortcutPixmap("red");
	abort_bt->loadDeco();
	abort_bt->setText(_("abort"));
	abort_bt->hide();
	CONNECT(abort_bt->selected, eDownload::download_abort);

	progresstext=new eLabel(this);
	progresstext->move(ePoint(10, clientrect.height()-115));
	progresstext->resize(eSize(clientrect.width()-20, fd+10));
	progresstext->setText(" ");
	progresstext->hide();

	progress=new eProgress(this);
	progress->move(ePoint(10, clientrect.height()-80));
	progress->resize(eSize( clientrect.width()-20, 20));
	progress->setName(_("Progress"));
	progress->hide();

	status = new eStatusBar(this);
	status->move( ePoint(5, clientrect.height()-50) ); status->resize( eSize( clientrect.width()-10, 50) );
	status->loadDeco();

	timer = new eTimer(eApp);
	CONNECT( timer->timeout,eDownload::download_start);
	if(auto_start)
		timer->start(1000, true);

}

void eDownload::download_start()
{
	int error;
	if (http) delete http;

	start_bt->hide();
	progress->show();
	progresstext->show();
	abort_bt->show();
	status->setText("downloading File...");
	setFocus(abort_bt);

	http=eHTTPConnection::doRequest(urlfull.c_str(), eApp, &error);
	if (!http) transfer_done(error);
	else
	{
		CONNECT(http->transferDone, eDownload::transfer_done);
		CONNECT(http->createDataSource, eDownload::create_datasink);
		http->start();
	}
}

void eDownload::transfer_done(int err)
{
	progress->hide();
	progresstext->hide();
	abort_bt->hide();

	if (err || !http || http->code != 200)
	{
		eString errmsg;
		switch (err)
		{
			case 0:
			if (http && http->code != 200) errmsg="error: server replied "+eString().setNum(http->code)+" "+http->code_descr;
				break;
			case -2:
				errmsg="Can't resolve hostname!";
				break;
			case -3:
				errmsg="Can't connect! (check network settings)";
				break;
			default:
				errmsg.sprintf("unknown error %d", err);
				break;
		}
		status->setText(errmsg);
		write_ipkg_log_file("ERROR %s\n",errmsg.c_str());
		http=0;
		start_bt->show();

		if(auto_start)	close(0);
	}
	else
	{
		http=0;
		status->setText(_("One moment please..."));
		close(1);
	}
}

void eDownload::downloadProgress(int received, int total)
{
	if ((time(0) == lasttime) && (received != total)) return;
	lasttime=time(0);
	eString pt;
	if (total > 0)
	{
		int perc=received*100/total;
		pt.sprintf("%d/%d kb (%d%%)", received/1024, total/1024, perc);
		progress->setPerc(perc);
		progresstext->setText(pt);
	}
	else
	{
		pt.sprintf("%d kb", received/1024);
		progress->setPerc(0);
		progresstext->setText(pt);
	}
}

void eDownload::download_abort()
{
	eString errmsg = _("Download aborted.");
	if (http)
	{
		delete http; http=0;
	}

	status->setText(errmsg);
	progress->hide();
	progresstext->hide();
	abort_bt->hide();
	start_bt->show();
	write_ipkg_log_file("%s\n",errmsg.c_str());
	if(auto_start)
		close(0);
}

eDownload::~eDownload()
{
	if (http) delete http;
	http=0;
}
