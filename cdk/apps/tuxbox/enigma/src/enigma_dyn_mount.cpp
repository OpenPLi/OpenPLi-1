/*
 * $Id: enigma_dyn_mount.cpp,v 1.30 2005/10/12 20:46:27 digi_casi Exp $
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

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <lib/system/http_dyn.h>
#include <lib/system/econfig.h>
#include <enigma_dyn.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn_mount.h>
#include <enigma_main.h>
#include <enigma_mount.h>
#include <media_mapping.h>

using namespace std;

eString getConfigMountMgr(void)
{
	eString result = readFile(TEMPLATE_DIR + "mountPoints.tmp");
	result.strReplace("#ADDMOUNTPOINTBUTTON#", button(100, "Add", GREEN, "javascript:addMountPoint()", "#FFFFFF"));
	eString skelleton = readFile(TEMPLATE_DIR + "mountPoint.tmp");
	result.strReplace("#BODY#", eNetworkMountMgr::getInstance()->listMountPoints(skelleton));
	return result;
}

static eString addChangeMountPoint(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eMountPoint mp;

	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	eString action = opt["action"];

	mp.id = atoi(opt["id"].c_str());
	mp.localDir = opt["localdir"];
	mp.fstype = (enum eMountPoint::mountType)atoi(opt["fstype"].c_str());
	mp.password = opt["password"];
	mp.userName = opt["username"];
	mp.mountDir = opt["mountdir"];
	mp.remoteHost = opt["remotehost"];
	mp.setIP();
	mp.automount = (opt["automount"] == "on") ? 1 : 0;
	eString options = opt["options"];
	mp.setMounted(false);
	eString async = opt["async"];
	eString sync = opt["sync"];
	eString atime = opt["atime"];
	eString autom = opt["autom"];
	eString execm = opt["execm"];
	eString noexec = opt["noexec"];
	eString ro = opt["ro"];
	eString rw = opt["rw"];
	eString users = opt["users"];
	eString nolock = opt["nolock"];
	eString intr = opt["intr"];
	eString soft = opt["soft"];
	eString tcp = opt["tcp"];
	mp.description = opt["description"];
	mp.linuxExtensions = opt["linuxextensions"];

	if (async == "on")
		mp.options += "async,";
	if (sync == "on")
		mp.options += "sync,";
	if (atime == "on")
		mp.options += "atime,";
	if (autom == "on")
		mp.options += "autom,";
	if (execm == "on")
		mp.options += "execm,";
	if (noexec == "on")
		mp.options += "noexec,";
	if (ro == "on")
		mp.options += "ro,";
	if (rw == "on")
		mp.options += "rw,";
	if (users == "on")
		mp.options += "users,";
	if (nolock == "on")
		mp.options += "nolock,";
	if (intr == "on")
		mp.options += "intr,";
	if (soft == "on")
		mp.options += "soft,";
	if (tcp == "on")
		mp.options += "tcp,";
	if (mp.options.length() > 0)
		mp.options = mp.options.substr(0, mp.options.length() - 1); //remove last comma
	mp.options += (options) ? ("," + options) : "";

	if (action == "change")
	{
		eNetworkMountMgr::getInstance()->changeMountPoint(mp.id, mp);
	}
	else
	{
		eNetworkMountMgr::getInstance()->addMountPoint(mp);
	}

	content->local_header["Content-Type"]="text/html; charset=utf-8";
	return "<html><body onUnload=\"parent.window.opener.location.reload(true)\"><script>window.close();</script></body></html>";
}

static eString removeMountPoint(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	eString id = opt["id"];

	eNetworkMountMgr::getInstance()->removeMountPoint(atoi(id.c_str()));

	content->local_header["Content-Type"]="text/html; charset=utf-8";
	return "<html><body onUnload=\"parent.window.opener.location.reload(true)\">Mount point deleted successfully.</body></html>";
}

static eString mountPointWindow(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString async, sync, atime, autom, execm, noexec, ro, rw, users, nolock, intr, soft, tcp;
	eMountPoint mp;

	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	eString action = opt["action"];
	eString id = opt["id"];

	eString result = readFile(TEMPLATE_DIR + "mountPointWindow.tmp");
	if (action == "change")
	{
		result.strReplace("#TITLE#", "Change Mount Point");
		mp = eNetworkMountMgr::getInstance()->getMountPointData(atoi(id.c_str()));
	}
	else
	{
		result.strReplace("#TITLE#", "Add Mount Point");
		mp.options = "nolock,intr,soft,rsize=8192,wsize=8192";
		mp.fstype = eMountPoint::cifsMount;
	}

	result.strReplace("#ACTION#", "/control/addChangeMountPoint");
	result.strReplace("#CMD#", action);

	unsigned int pos = 0;
	eString options, option;
	while (mp.options.length() > 0)
	{
		if ((pos = mp.options.find(",")) != eString::npos)
		{
			option = mp.options.substr(0, pos);
			mp.options = mp.options.substr(pos + 1);
		}
		else
		{
			option = mp.options;
			mp.options = "";
		}

		if (option == "async")
			async = "checked";
		else
		if (option == "sync")
			sync = "checked";
		else
		if (option == "atime")
			atime = "checked";
		else
		if (option == "autom")
			autom = "checked";
		else
		if (option == "execm")
			execm = "checked";
		else
		if (option == "noexec")
			noexec = "checked";
		else
		if (option == "ro")
			ro = "checked";
		else
		if (option == "rw")
			rw = "checked";
		else
		if (option == "users")
			users = "checked";
		else
		if (option == "nolock")
			nolock = "checked";
		else
		if (option == "intr")
			intr = "checked";
		else
		if (option == "soft")
			soft = "checked";
		else
		if (option == "tcp")
			tcp = "checked";
		else
			options += (options) ? ("," + option) : option;
	}

	result.strReplace("#ID#", eString().sprintf("%d", mp.id));
	result.strReplace("#LDIR#", mp.localDir);
	result.strReplace("#FSTYPE#", eString().sprintf("%d", mp.fstype));
	result.strReplace("#PW#", mp.password);
	result.strReplace("#USER#", mp.userName);
	result.strReplace("#MDIR#", mp.mountDir);
	result.strReplace("#MHOST#", mp.remoteHost);
	result.strReplace("#AUTO#", (mp.automount == 1) ? "checked" : "");
	result.strReplace("#OPTIONS#", options);
	result.strReplace("#ASYNC#", async);
	result.strReplace("#SYNC#", sync);
	result.strReplace("#ATIME#", atime);
	result.strReplace("#AUTOM#", autom);
	result.strReplace("#EXECM#", execm);
	result.strReplace("#NOEXEC#", noexec);
	result.strReplace("#RO#", ro);
	result.strReplace("#RW#", rw);
	result.strReplace("#USERS#", users);
	result.strReplace("#NOLOCK#", nolock);
	result.strReplace("#INTR#", intr);
	result.strReplace("#SOFT#", soft);
	result.strReplace("#TCP#", tcp);
	result.strReplace("#DESCRIPTION#", mp.description);
	result.strReplace("#LINUXEXTENSIONS#", (mp.linuxExtensions == 1) ? "checked" : "");

	content->local_header["Content-Type"]="text/html; charset=utf-8";

	return result;
}

static eString mountMountPoint(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	eString id = opt["id"];

	content->local_header["Content-Type"]="text/html; charset=utf-8";
	eString result = eNetworkMountMgr::getInstance()->mountMountPoint(atoi(id.c_str()));

	eDebug("[ENIGMA_DYN_MOUNT] mount: rc = '%s'", result.c_str());

	return closeWindow(content, "", 500);
}

static eString unmountMountPoint(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	eString id = opt["id"];

	content->local_header["Content-Type"]="text/html; charset=utf-8";
	eString result = eNetworkMountMgr::getInstance()->unmountMountPoint(atoi(id.c_str()));

	eDebug("[ENIGMA_DYN_MOUNT] unmount: rc = '%s'", result.c_str());

	return closeWindow(content, "", 500);
}

extern bool rec_movies();

static eString selectMovieSource(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString result;

	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	eString id = opt["id"] + "/movie";

	content->local_header["Content-Type"]="text/html; charset=utf-8";
	
	eDebug("selectMovieSource: new movie location '%s'", id.c_str());
	
	// Update movie storage location
	eMediaMapping::getInstance()->setStorageLocation(eMediaMapping::mediaMovies, id);

	if (access((id + "/recordings.epl").c_str(), R_OK) == 0)
	{
		eDebug("[ENIGMA_DYN_MOUNT] recordings.epl available");
		eZapMain::getInstance()->getRecordings()->lockPlaylist();
		eZapMain::getInstance()->loadRecordings();
		eZapMain::getInstance()->getRecordings()->unlockPlaylist();
	}
	else
	{
		eDebug("[ENIGMA_DYN_MOUNT] recordings.epl not available, recovering...");
		rec_movies();
	}
	
	eDebug("[ENIGMA_DYN_MOUNT] selectMovieSource.");

	return closeWindow(content, "", 500);
}

void ezapMountInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb)
{
	dyn_resolver->addDyn("GET", "/control/addChangeMountPoint", addChangeMountPoint, lockWeb);
	dyn_resolver->addDyn("GET", "/control/removeMountPoint", removeMountPoint, lockWeb);
	dyn_resolver->addDyn("GET", "/control/mountPointWindow", mountPointWindow, lockWeb);
	dyn_resolver->addDyn("GET", "/control/mountMountPoint", mountMountPoint, lockWeb);
	dyn_resolver->addDyn("GET", "/control/unmountMountPoint", unmountMountPoint, lockWeb);
	dyn_resolver->addDyn("GET", "/control/selectMovieSource", selectMovieSource, lockWeb);
}

#endif
