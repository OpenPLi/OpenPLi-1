/*
 * $Id: chttpdconfig.h,v 1.2 2005/10/19 16:41:30 digi_casi Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __chttpdconfig__
#define __chttpdconfig__

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <config.h>
#include <string>
#include <configfile.h>

using namespace std;

#define CHTTPD_CONFIGFILE CONFIGDIR "/enigma/chttpd.conf"
#define PRIVATEDOCUMENTROOT "/var/tuxbox/config/enigma/httpd"
#define PUBLICDOCUMENTROOT "/var/tmp/httpd"

class chttpdConfig
{
public:
	bool AutoStart;
	int Port;
	bool THREADS;
	bool MustAuthenticate;
	bool Verbose;
	bool Log;
	string PrivateDocumentRoot;
	string PublicDocumentRoot;
	string AuthUser;
	string AuthPassword;
	
	chttpdConfig()
	{
		load();
		if (AutoStart && system("pidof chttpd"))
			system("/bin/chttpd&");
	}
	
	~chttpdConfig() {}

	void load()
	{
		CConfigFile *Config = new CConfigFile(',');
		if (Config->loadConfig(CHTTPD_CONFIGFILE))
		{
			AutoStart = Config->getBool("AutoStart");
			Port = Config->getInt32("Port");
			THREADS = Config->getBool("THREADS");
			Verbose = Config->getBool("VERBOSE");
			Log = Config->getBool("LOG");
			MustAuthenticate = Config->getBool("Authenticate");
			PrivateDocumentRoot = Config->getString("PrivateDocRoot");
			PublicDocumentRoot = Config->getString("PublicDocRoot");
			AuthUser = Config->getString("AuthUser");
			AuthPassword = Config->getString("AuthPassword");
		}
		else
		{
			AutoStart = false;
			Port = 8085;
			THREADS = true;
			Verbose = false;
			Log = false;
			MustAuthenticate = false;
			PrivateDocumentRoot = PRIVATEDOCUMENTROOT;
			PublicDocumentRoot = PUBLICDOCUMENTROOT;
			AuthUser = "root";
			AuthPassword = "dreambox";
			save();
		}
		delete Config;
	}
	
	void save()
	{
		CConfigFile *Config = new CConfigFile(',');
		Config->setBool("AutoStart", AutoStart);
		Config->setInt32("Port", Port);
		Config->setBool("THREADS", THREADS);
		Config->setBool("VERBOSE", Verbose);
		Config->setBool("LOG", Log);
		Config->setBool("Authenticate", MustAuthenticate);
		Config->setString("AuthUser", AuthUser);
		Config->setString("AuthPassword", AuthPassword);
		Config->setString("PublicDocRoot", PublicDocumentRoot);
		Config->setString("PrivateDocRoot", PrivateDocumentRoot);
		Config->saveConfig(CHTTPD_CONFIGFILE);
		delete Config;
	}
};
#endif
