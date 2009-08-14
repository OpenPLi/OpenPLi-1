/*
 * $Id: enigma_dyn_pda.cpp,v 1.4 2006/02/15 18:57:41 digi_casi Exp $
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
#include <linux/input.h>
#include <iomanip>
#include <iostream>
#include <fstream>

#include <enigma.h>
#include <timer.h>
#include <enigma_main.h>
#include <math.h>

#include <lib/dvb/frontend.h>
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
#include <lib/gdi/fb.h>
#include <lib/gdi/glcddc.h>
#include <lib/gdi/gfbdc.h>
#include <lib/gdi/epng.h>
#include <lib/gui/emessage.h>
#include <lib/system/http_dyn.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>
#include <lib/system/dmfp.h>
#include <lib/system/file_eraser.h>
#include <enigma_dyn.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn_pda.h>
#include <parentallock.h>

using namespace std;

extern int pdaScreen;
extern int screenWidth;
extern eString getCurService(void);
extern eString getCurrentSubChannel(eString curServiceRef);
extern bool onSameTP(const eServiceReferenceDVB& ref1, const eServiceReferenceDVB &ref2); // implemented in timer.cpp
extern bool canPlayService(const eServiceReference & ref); // implemented in timer.cpp
extern eString getContent(eString mode, eString path, eString opts);
extern eString getLeftNavi(eString mode);
extern eString getTopNavi();
extern bool playService(const eServiceReference &ref);

static eString getChannelNavi(void)
{
	eString result;

	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();

	if (sapi && sapi->service)
	{
		result = button(100, "Audio", NOCOLOR, "javascript:selectAudio()");
		result += button(100, "Video", NOCOLOR, "javascript:selectSubChannel()");

		if (getCurService() || getCurrentSubChannel(ref2string(sapi->service)))
		{
			result += button(100, "EPG", NOCOLOR, "javascript:openEPG('')");
			if (pdaScreen == 0)
			{
				result += button(100, "Info", NOCOLOR, "javascript:openChannelInfo()");
				result += button(100, "Stream Info", NOCOLOR, "javascript:openSI()");
			}
#ifndef DISABLE_FILE
			if (eZapMain::getInstance()->isRecording())
				result += button(100, "Stop", NOCOLOR, "javascript:DVRrecord('stop')");
			else
				result += button(100, "Record", NOCOLOR, "javascript:DVRrecord('record')");
#endif
		}
	}

	return result;
}

static eString getVolBar()
{
	std::stringstream result;
	int volume = (eAVSwitch::getInstance()->getMute()) ? 0 : 63 - eAVSwitch::getInstance()->getVolume();

	for (int i = 9; i <= 63; i += 6)
	{
		result << "<td width=\"15\" height=\"8\">"
			"<a href=\"javascript:setVol(" << i << ")\">";
		if (i <= volume)
			result << "<img src=\"led_on.gif\" border=\"0\" width=\"15\" height=\"8\">";
		else
			result << "<img src=\"led_off.gif\" border=\"0\" width=\"15\" height=\"8\">";
		result << "</a>"
			"</td>";
	}

	return result.str();
}

static eString getMute()
{
	std::stringstream result;

	result << "<a href=\"javascript:toggleMute(" << eAVSwitch::getInstance()->getMute() << ")\">";
	if (eAVSwitch::getInstance()->getMute())
		result << "<img src=\"speak_off.gif\" border=0>";
	else
		result << "<img src=\"speak_on.gif\" border=0>";
	result << "</a>";

	return result.str();
}

struct countDVBServices: public Object
{
	int &count;
	countDVBServices(const eServiceReference &bouquetRef, int &count)
		:count(count)
	{
		Signal1<void, const eServiceReference&> cbSignal;
		CONNECT(cbSignal, countDVBServices::countFunction);
		eServiceInterface::getInstance()->enterDirectory(bouquetRef, cbSignal);
		eServiceInterface::getInstance()->leaveDirectory(bouquetRef);
	}
	void countFunction(const eServiceReference& ref)
	{
		if (ref.path
			|| ref.flags & eServiceReference::isDirectory
			|| ref.type != eServiceReference::idDVB)
			return;

		// sorry.. at moment we dont show any directory.. or locked service in webif
		if (ref.isLocked() && pinCheck::getInstance()->pLockActive())
			return;

		++count;
	}
};

class eWebNavigatorListDirectory: public Object
{
	eString &result;
	eString path;
	eServiceInterface &iface;
	int num;
public:
	eWebNavigatorListDirectory(eString &result, eString path, eServiceInterface &iface): result(result), path(path), iface(iface)
	{
//		eDebug("path: %s", path.c_str());
		num = 0;
	}
	void addEntry(const eServiceReference &e)
	{
		// sorry.. at moment we dont show any directory.. or locked service in webif
		if (e.isLocked() && pinCheck::getInstance()->pLockActive())
			return;
#ifndef DISABLE_FILE
		if (eDVB::getInstance()->recorder && !e.path && !e.flags)
		{
			if (!onSameTP(eDVB::getInstance()->recorder->recRef,(eServiceReferenceDVB&)e))
				 return;
		}
#endif
		result += "<tr bgcolor=\"";
		result += (num & 1) ? LIGHTGREY : DARKGREY;
		result += "\"><td width=30 align=center>";

		eString serviceRef = ref2string(e);
		if (!(e.flags & eServiceReference::isDirectory))
		{
			if (!e.path)
				result += button(50, "EPG", GREEN, "javascript:openEPG('" + serviceRef + "')");
			else
			if (serviceRef.find("%2fmedia%2fhdd%2fmovie%2f") != eString::npos)
			{
				result += "<a href=\"javascript:deleteMovie('";
				result += serviceRef;
				result += "')\"><img src=\"trash.gif\" height=12 border=0></a>";
			}
			else
				result += "&#160;";
		}
		else
		{
			int count = 0;
			countDVBServices bla(e, count);
			if (count)
				result += button(50, "EPG", GREEN, "javascript:openMultiEPG('" + serviceRef + "')");
			else
				result += "&#160;";
		}
		result += eString("</td><td><a href=\"/")+ "?mode=zap&path=" + serviceRef + "\">";

		eService *service = iface.addRef(e);
		if (!service)
			result += "N/A";
		else
		{
			result += filter_string(service->service_name);
			iface.removeRef(e);
		}

		result += "</a>";
		result += "</td></tr>\n";
		num++;
	}
};

eString getPDAZapContent(eString path)
{
	eString result;

	eServiceReference current_service = string2ref(path);
	eServiceInterface *iface = eServiceInterface::getInstance();

	if (!(current_service.flags&eServiceReference::isDirectory))	// is playable
	{
		playService(current_service);
		result = "";
	}
	else
	{
		eWebNavigatorListDirectory navlist(result, path, *iface);
		Signal1<void, const eServiceReference&> signal;
		signal.connect(slot(navlist, &eWebNavigatorListDirectory::addEntry));
		result += "<table width=\"100%\" cellspacing=\"2\" cellpadding=\"1\" border=\"0\">\n";
		iface->enterDirectory(current_service, signal);
		result += "</table>\n";
//		eDebug("entered");
		iface->leaveDirectory(current_service);
//		eDebug("exited");
	}

	return result;
}

eString getSatellites(void)
{
	eString result;
	int num = 0;

	for (std::list<eLNB>::iterator it2(eTransponderList::getInstance()->getLNBs().begin()); it2 != eTransponderList::getInstance()->getLNBs().end(); it2++)
	{
		// go thru all satellites...
		for (ePtrList<eSatellite>::iterator s (it2->getSatelliteList().begin()); s != it2->getSatelliteList().end(); s++)
		{
			result += "<tr bgcolor=\"";
			result += (num & 1) ? LIGHTGREY : DARKGREY;
			result += "\">";
			result += "<td><a href=\'?mode=controlSatFinder&display=transponders&sat=" + eString().sprintf("%d", s->getOrbitalPosition()) + "\'>" + s->getDescription() + "</a></td>";
			result += "</tr>\n";
			num++;
		}
	}
	return result;
}

eString getTransponders(int orbital_position)
{
	eString result;
	int num = 0;

	result += "<h2>Satellite Orbital Position: " + eString().sprintf("%d", orbital_position) + "</h2>\n";
	// go thru all transponders...
	for (std::list<tpPacket>::iterator it3(eTransponderList::getInstance()->getNetworks().begin()); it3 != eTransponderList::getInstance()->getNetworks().end(); it3++)
	{
		if (it3->orbital_position == orbital_position)
		{
			for (std::list<eTransponder>::iterator it(it3->possibleTransponders.begin()); it != it3->possibleTransponders.end(); it++)
			{
				eString transponder = eString().sprintf("%d / %d / %c", it->satellite.frequency / 1000, it->satellite.symbol_rate / 1000, it->satellite.polarisation ? 'V' : 'H');
				result += "<tr bgcolor=\"";
				result += (num & 1) ? LIGHTGREY : DARKGREY;
				result += "\">";
				result += "<td><a href=\'javascript:tuneTransponder(\"" + it->satellite.toString() + "\")\'>" + transponder + "</a></td>";
				result += "</tr>\n";
				num++;
			}
		}
	}
	return result;
}

eString getPDAContent(eString opts)
{
	eString result;
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString mode = opt["mode"];
	eString path = opt["path"];
	
	if (!path)
		path = eServiceStructureHandler::getRoot(eServiceStructureHandler::modeTV).toString();
	if (!mode)
		mode = "zap";

	result = readFile(TEMPLATE_DIR + "index_small.tmp");
	eString tmp = getContent(mode, path, opts);
	if (!tmp)
		result = "";
	result.strReplace("#CONTENT#", tmp);
	result.strReplace("#VOLBAR#", getVolBar());
	result.strReplace("#MUTE#", getMute());
	result.strReplace("#TOPNAVI#", getTopNavi());
	result.strReplace("#CHANNAVI#", getChannelNavi());
	result.strReplace("#LEFTNAVI#", getLeftNavi(mode));
	if (eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000
		|| eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7020)
		result.strReplace("#TOPBALK#", "topbalk_small.png");
	else
	if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::dbox2Nokia)
		result.strReplace("#TOPBALK#", "topbalk2_small.png");
	else
	if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::dbox2Sagem)
		result.strReplace("#TOPBALK#", "topbalk3_small.png");
	else
//	if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::dbox2Philips)
		result.strReplace("#TOPBALK#", "topbalk4_small.png");
	if (!result)
		result = "<html><body>Please wait...<script language=\"javascript\">window.close();</script></body></html>";
	return result;
}

static eString pda_root(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString result;

	pdaScreen = 1;
	screenWidth = 240;
	eConfig::getInstance()->setKey("/ezap/webif/screenWidth", screenWidth);

	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	content->local_header["Content-Type"] = "text/html; charset=utf-8";
	content->local_header["Cache-Control"] = "no-cache";

	result = getPDAContent(opts);

	return result;
}

void ezapPDAInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb)
{
	dyn_resolver->addDyn("GET", "/pda", pda_root, lockWeb);
}
