/*      
        webserver  -   DBoxII-Project

        Copyright (C) 2001/2002 Dirk Szymanski 'Dirch'

        $Id: webserver.h,v 1.18 2002/10/18 12:35:52 thegoodguy Exp $

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
        Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


#ifndef __webserver__
#define __webserver__

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <config.h>
#include <string>

#include <configfile.h>


using namespace std;

#define SA struct sockaddr
#define SAI struct sockaddr_in
#define PRIVATEDOCUMENTROOT "/share/tuxbox/neutrino/httpd"
#define PUBLICDOCUMENTROOT "/var/tmp/httpd"

class CWebDbox;
class TWebserverRequest;

struct Tmconnect
{
	int sock_fd;
	SAI servaddr;
};

//----------------------------------------------------------------------
class CWebserver
{
	int			Port;
	int			ListenSocket;
	bool			THREADS;

public:
// config vars / switches
	bool			STOP;
	bool			VERBOSE;
	bool			MustAuthenticate;
	bool			NewGui;

	string			PrivateDocumentRoot;
	string			PublicDocumentRoot;
	string			Zapit_XML_Path;

	string			AuthUser;
	string			AuthPassword;



	CWebDbox		*WebDbox;

	CWebserver(bool debug);
	~CWebserver();

	bool Init(bool debug);
	bool Start();
	void DoLoop();
	void Stop();

	int SocketConnect(Tmconnect * con,int Port);

	void SaveConfig();
	void ReadConfig();

	friend class CWebserverRequest;
	friend class TWebDbox;

};


#endif
