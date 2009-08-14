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

#include <imdb.h>

extern "C" int plugin_exec( PluginParam *par );

int plugin_exec( PluginParam *par )
{	imdbMain dlg;

	dlg.show();
	int result=dlg.exec();
	dlg.hide();

	return result;
}

eString stringReplace( const eString &toReplace, char from, char to )
{	char buffer[8192];
	strncpy(buffer, (char *)toReplace.c_str(), sizeof(buffer));
	buffer[sizeof(buffer) - 1] = 0;
	char *p = buffer;
	while ( *p != '\0' ) 
	{	if ( *p == from )
			*p = to;
		p++;
	}
	return eString( buffer );
}

imdbMain::~imdbMain()
{
}

void imdbMain::downloadDone(int err)
{	if(err == 0 && inDownloadFlag)
	{	inDownloadFlag = 0;
		FILE *in = fopen("/var/tmp/imdb.tmp", "r");
		if(in)
		{	char buf[8192];
			unsigned int len = fread(buf, 1, sizeof(buf) - 1, in);
			buf[len] = '\0';
			eMessageBox msg(buf, _("Details"), eMessageBox::btOK || eMessageBox::btMax);
			msg.show();     msg.exec();     msg.hide();
			fclose(in);
		}
	}
}

void imdbMain::selectedItem(eListBoxEntryText *item)
{	if(item && !inDownloadFlag)
	{	inDownloadFlag = 1;
		eString title = item->getText();
		title = stringReplace(title, ' ', '+');
		theFetcher.fetch("http://imdb-dreambox.hijmans.nl/scripts/lookup.php?query=" + title);
	}
}

imdbMain::imdbMain(): eWindow(1)
{	inDownloadFlag = 0;

	cmove(ePoint(140, 140));
	cresize(eSize(440, 296));
	setText("Recordings IMDB Info");

	theList = new eListBox<eListBoxEntryText>(this);
	theList->move(ePoint(10, 10));
	theList->resize(eSize(clientrect.width() - 20, clientrect.height() - 20));
	theList->loadDeco();
	theList->setColumns(1);
	setFocus(theList);

	ePlaylist * recordings = new ePlaylist;
	recordings->load(MOVIEDIR "/recordings.epl");

	if(recordings)	
	{	for (std::list<ePlaylistEntry>::iterator it(recordings->getList().begin()); it != recordings->getList().end(); ++it)
		{	new eListBoxEntryText(theList, it->service.descr, (void *) 0);

		}
	}
	else
	{	eMessageBox msg("recordings = NULL", _("Details"), eMessageBox::btOK);
                msg.show();     msg.exec();     msg.hide();
	}

	CONNECT(theList->selected, imdbMain::selectedItem);
	CONNECT(theFetcher.downloadDone, imdbMain::downloadDone);	
}

void Fetcher::fetch(eString url)
{	tempPath = "/var/tmp/imdb.tmp";
	int error = 0;
	connectionP = eHTTPConnection::doRequest(url.c_str(), eApp, &error);
	
	if(!connectionP || error)
	{	eMessageBox msg("Error downloading " + url + "(" + eString().sprintf("%d", error) + ")", _("Details"), eMessageBox::btOK);
		msg.show();     msg.exec();     msg.hide();
	}
	else
	{	CONNECT(connectionP->transferDone, Fetcher::transferDone);
		CONNECT(connectionP->createDataSource, Fetcher::createDownloadSink);
		connectionP->local_header["User-Agent"] = "RSS";
		connectionP->start();
	}
}

void Fetcher::transferDone(int err)
{	
	if(!err)
	{	connectionP = NULL;

		// Tell caller download is ready
		/*emit*/ downloadDone(err);
	}
	else
	{	eString sMsg = "Error " + eString().sprintf("%d", err);
		eMessageBox msg(sMsg, _("User Abort"), eMessageBox::iconWarning|eMessageBox::btOK);
		msg.show();     msg.exec();     msg.hide();
	}
}

eHTTPDataSource * Fetcher::createDownloadSink(eHTTPConnection *conn)
{	dataSinkP = new eHTTPDownload(connectionP, (char *)tempPath.c_str());

	return(dataSinkP);
}

eHTTPDownload::eHTTPDownload(eHTTPConnection *c, const char *filename): eHTTPDataSource(c), filename(filename)
{
	if (c->remote_header.count("Content-Length"))
		total=atoi(c->remote_header["Content-Length"].c_str());
	else
		total=-1;
	received=0;
	fd=::creat(filename, 0777);
	progress(received, total);
}

eHTTPDownload::~eHTTPDownload()
{
	if (fd >= 0)
		::close(fd);
	if ((total != -1) && (total != received))
		::unlink(filename.c_str());
}

void eHTTPDownload::haveData(void *data, int len)
{
	if (len)
	{
		if (fd >= 0)
			::write(fd, data, len);
	}
	received+=len;
	progress(received, total);
}
