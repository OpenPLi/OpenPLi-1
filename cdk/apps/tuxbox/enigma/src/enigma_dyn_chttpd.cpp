/*
 * $Id: enigma_dyn_chttpd.cpp,v 1.4 2005/10/22 19:22:52 digi_casi Exp $
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
 
#ifdef ENABLE_EXPERT_WEBIF
#include <map>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>
#include <net/if.h>
#include <sys/mount.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <lib/system/info.h>
#include <lib/system/http_dyn.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn_chttpd.h>
#include <enigma_processutils.h>
#include <chttpd/chttpdconfig.h>

chttpdConfig chttpdconf;

using namespace std;

eString getConfigCHTTPD(void)
{
	eString chttpdStatus = "Stopped";
	long *pidList = NULL;
	pidList = eProcessUtils::getPID("chttpd");
	if (*pidList != -1)
		chttpdStatus = "Running";
	free(pidList);
	
	chttpdconf.load();
	
	eString result = readFile(TEMPLATE_DIR + "chttpdSettings.tmp");
	
	result.strReplace("#STATUS#", chttpdStatus);
	result.strReplace("#AUTOSTART#", (chttpdconf.AutoStart ? "checked" : ""));
	result.strReplace("#PORT#", eString().sprintf("%d", chttpdconf.Port));
	result.strReplace("#THREADS#", (chttpdconf.THREADS ? "checked" : ""));
	result.strReplace("#MUSTAUTHENTICATE#", (chttpdconf.MustAuthenticate ? "checked" : ""));
	result.strReplace("#VERBOSE#", (chttpdconf.Verbose ? "checked" : ""));
	result.strReplace("#LOG#", (chttpdconf.Log ? "checked" : ""));
	result.strReplace("#PRIVATEDOCUMENTROOT#", chttpdconf.PrivateDocumentRoot);
	result.strReplace("#PUBLICDOCUMENTROOT#", chttpdconf.PublicDocumentRoot);
	result.strReplace("#AUTHUSER#", chttpdconf.AuthUser);
	result.strReplace("#AUTHPASSWORD#", chttpdconf.AuthPassword);
	
	result.strReplace("#STARTBUTTON#", button(100, "Start", GREEN, "javascript:startCHTTPD('')", "#FFFFFF"));
	result.strReplace("#STOPBUTTON#", button(100, "Stop", RED, "javascript:stopCHTTPD('')", "#FFFFFF"));

	return result;
}

eString stopCHTTPD(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	
	eProcessUtils::killProcess("chttpd");
	
	return closeWindow(content, "", 10);
}

eString startCHTTPD(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	long *pidList = NULL;
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	
	pidList = eProcessUtils::getPID("chttpd");
	if (*pidList == -1)
		system("chttpd&");
	free(pidList);
	
	return closeWindow(content, "", 10);
}

eString setCHTTPDSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	
	chttpdconf.AutoStart = opt["AutoStart"] == "on";
	chttpdconf.Port = atoi(opt["Port"].c_str());
	chttpdconf.THREADS = opt["Threads"] == "on";
	chttpdconf.Verbose = opt["Verbose"] == "on";
	chttpdconf.MustAuthenticate = opt["MustAuthenticate"] == "on";
	chttpdconf.Log = opt["Log"] == "on";
	chttpdconf.PrivateDocumentRoot = opt["PrivateDocumentRoot"];
	chttpdconf.PublicDocumentRoot = opt["PublicDocumentRoot"];
	chttpdconf.AuthUser = opt["AuthUser"];
	chttpdconf.AuthPassword = opt["AuthPassword"];
	
	chttpdconf.save();
	
	return closeWindow(content, "", 10);
}

void ezapCHTTPDInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb)
{
	dyn_resolver->addDyn("GET", "/cgi-bin/startCHTTPD", startCHTTPD, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/stopCHTTPD", stopCHTTPD, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/setCHTTPDsettings", setCHTTPDSettings, lockWeb);
}
#endif
