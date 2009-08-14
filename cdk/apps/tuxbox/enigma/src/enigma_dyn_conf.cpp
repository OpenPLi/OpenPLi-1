/*
 * $Id: enigma_dyn_conf.cpp,v 1.24 2008/09/24 19:20:16 dbluelle Exp $
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

#include <enigma.h>
#include <enigma_main.h>
#include <enigma_standby.h>
#include <timer.h>
#include <lib/driver/eavswitch.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/servicestructure.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/service.h>
#include <lib/dvb/record.h>
#include <lib/dvb/serviceplaylist.h>

#include <lib/system/info.h>
#include <lib/system/http_dyn.h>
#include <lib/system/econfig.h>
#include <enigma_dyn.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn_conf.h>
#include <configfile.h>

using namespace std;


void initHDDparms(void)
{
#ifndef DISABLE_FILE
	eString cmd;
	int ti = 60, ac = 128;

	eConfig::getInstance()->getKey("/extras/hdparm-s", ti);
	eConfig::getInstance()->getKey("/extras/hdparm-m", ac);

	cmd.sprintf("/sbin/hdparm -S %d -M %d /dev/ide/host0/bus0/target0/lun0/disc", ti, ac);
	system(cmd.c_str());
	// if second channel exist there is a second HDD active, also set sleep time and accoustics
	if (access("/proc/ide/hdb", F_OK) == 0)
	{
		cmd.sprintf("/sbin/hdparm -S %d -M %d /dev/ide/host0/bus0/target1/lun0/disc", ti, ac);
		system(cmd.c_str());
	}
#endif
}

eString getConfigSettings(void)
{
	eString result = readFile(TEMPLATE_DIR + "configSettings.tmp");
	int fastshutdown = 0;
	eConfig::getInstance()->getKey("/extras/fastshutdown", fastshutdown);
	int showSatPos = 1;
	eConfig::getInstance()->getKey("/extras/showSatPos", showSatPos);
	result.strReplace("#SHOWSATPOS#", (showSatPos == 1) ? "checked" : "");
	int timeroffsetstart = 0;
	eConfig::getInstance()->getKey("/enigma/timeroffsetstart", timeroffsetstart);
	result.strReplace("#TIMEROFFSETSTART#", eString().sprintf("%d", timeroffsetstart));
	int timeroffsetstop = 0;
	eConfig::getInstance()->getKey("/enigma/timeroffsetstop", timeroffsetstop);
	result.strReplace("#TIMEROFFSETSTOP#", eString().sprintf("%d", timeroffsetstop));
	int defaultaction = 0;
	eConfig::getInstance()->getKey("/enigma/timerenddefaultaction", defaultaction);
	switch(defaultaction)
	{
		default:
		case 0:
			result.strReplace("#TIMERENDDEFAULTACTION#", eString().sprintf("<option selected value=\"0\">Nothing</option><option value=\"%d\">Standby</option><option value=\"%d\">Shutdown</option>",ePlaylistEntry::doGoSleep,ePlaylistEntry::doShutdown));
			break;
		case ePlaylistEntry::doGoSleep:
			result.strReplace("#TIMERENDDEFAULTACTION#", eString().sprintf("<option value=\"0\">Nothing</option><option selected value=\"%d\">Standby</option><option value=\"%d\">Shutdown</option>",ePlaylistEntry::doGoSleep,ePlaylistEntry::doShutdown));
			break;
		case ePlaylistEntry::doShutdown:
			result.strReplace("#TIMERENDDEFAULTACTION#", eString().sprintf("<option value=\"0\">Nothing</option><option value=\"%d\">Standby</option><option selected value=\"%d\">Shutdown</option>",ePlaylistEntry::doGoSleep,ePlaylistEntry::doShutdown));
			break;
	}
	int maxmtu = 1500;
	eConfig::getInstance()->getKey("/elitedvb/network/maxmtu", maxmtu);
	result.strReplace("#MAXMTU#", eString().sprintf("%d", maxmtu));
	int samba = 1;
	eConfig::getInstance()->getKey("/elitedvb/network/samba", samba);
	result.strReplace("#SAMBA#", (samba == 1) ? "checked" : "");
	int webLock = 1;
	eConfig::getInstance()->getKey("/ezap/webif/lockWebIf", webLock);
	result.strReplace("#WEBIFLOCK#", (webLock == 1) ? "checked" : "");
	int hddti = 24;
	eConfig::getInstance()->getKey("/extras/hdparm-s", hddti);
	result.strReplace("#HDDSTANDBY#", eString().sprintf("%d", hddti / 12));
	int hddac = 160;
	eConfig::getInstance()->getKey("/extras/hdparm-m", hddac);
	result.strReplace("#HDDACOUSTICS#", eString().sprintf("%d", hddac));
	char *audiochannelspriority=0;
	eConfig::getInstance()->getKey("/extras/audiochannelspriority", audiochannelspriority);
	eString rpl="";
	if ( audiochannelspriority )
	{
		rpl = audiochannelspriority;
		free(audiochannelspriority);
	}
	result.strReplace("#AUDIOCHANNELSPRIORITY#", rpl);
	char *trustedhosts=NULL;
	eConfig::getInstance()->getKey("/ezap/webif/trustedhosts", trustedhosts);
	rpl = "";
	if ( trustedhosts )
	{
		rpl = trustedhosts;
		free(trustedhosts);
	}
	result.strReplace("#TRUSTEDHOSTS#", rpl);
	
	char *epgcachepath=NULL;
	eConfig::getInstance()->getKey("/enigma/epgMemStoreDir", epgcachepath);
	rpl = "";
	if ( epgcachepath )
	{
		rpl = epgcachepath;
		free(epgcachepath);
	}
	result.strReplace("#EPGCACHEPATH#", rpl);

	char *epgsqlpath=NULL;
	eConfig::getInstance()->getKey("/enigma/epgSQLiteDir", epgsqlpath);
	rpl = "";
	if ( epgsqlpath )
	{
		rpl = epgsqlpath;
		free(epgsqlpath);
	}

	result.strReplace("#EPGSQLPATH#", rpl);
	return result;
}

eString setConfigSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString maxmtu = opt["maxmtu"];
	eString samba = opt["samba"];
	eString hddti = opt["hddstandby"];
	eString hddac = opt["hddacoustics"];
	eString timeroffsetstart = opt["timeroffsetstart"];
	eString timeroffsetstop = opt["timeroffsetstop"];
	eString timerenddefaultaction = opt["timerenddefaultaction"];
	eString showsatpos = opt["showsatpos"];
	eString webiflock = opt["webiflock"];
	eString audiochannelspriority = opt["audiochannelspriority"];
	eString trustedhosts = opt["trustedhosts"];
	eString epgcachepath = opt["epgcachepath"];
	eString epgsqlpath = opt["epgsqlpath"];
	eConfig::getInstance()->setKey("/ezap/webif/trustedhosts", trustedhosts.c_str());
	eConfig::getInstance()->setKey("/enigma/epgMemStoreDir", epgcachepath.c_str());
	eConfig::getInstance()->setKey("/enigma/epgSQLiteDir", epgsqlpath.c_str());

	int oldti = 0;
	eConfig::getInstance()->getKey("/extras/hdparm-s", oldti);
	int oldac = 0;
	eConfig::getInstance()->getKey("/extras/hdparm-m", oldac);

	eConfig::getInstance()->setKey("/elitedvb/network/samba", (samba == "on" ? 1 : 0));

	int webLock1 = 0;
	eConfig::getInstance()->getKey("/ezap/webif/lockWebIf", webLock1);
	eConfig::getInstance()->setKey("/ezap/webif/lockWebIf", (webiflock == "on" ? 1 : 0));
	int webLock2 = 0;
	eConfig::getInstance()->getKey("/ezap/webif/lockWebIf", webLock2);
	if (webLock1 != webLock2)
		eZap::getInstance()->reconfigureHTTPServer();
	eConfig::getInstance()->setKey("/extras/showSatPos", (showsatpos == "on" ? 1 : 0));
	eConfig::getInstance()->setKey("/enigma/timeroffsetstart", atoi(timeroffsetstart.c_str()));
	eConfig::getInstance()->setKey("/enigma/timeroffsetstop", atoi(timeroffsetstop.c_str()));
	eConfig::getInstance()->setKey("/enigma/timerenddefaultaction", atoi(timerenddefaultaction.c_str()));
	eConfig::getInstance()->setKey("/elitedvb/network/maxmtu", atoi(maxmtu.c_str()));
	system(eString("/sbin/ifconfig eth0 mtu " + maxmtu).c_str());
	if ((atoi(hddti.c_str()) * 12) != oldti)
		eConfig::getInstance()->setKey("/extras/hdparm-s", atoi(hddti.c_str()) * 12);
	if (atoi(hddac.c_str()) != oldac)
		eConfig::getInstance()->setKey("/extras/hdparm-m", atoi(hddac.c_str()));
	initHDDparms();
	eConfig::getInstance()->setKey("/extras/audiochannelspriority", audiochannelspriority.c_str());

	return closeWindow(content, "", 500);
}

void ezapConfInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb)
{
	dyn_resolver->addDyn("GET", "/cgi-bin/setConfigSettings", setConfigSettings, lockWeb);
}
#endif
