/*
 * $Id: enigma_dyn_wap.cpp,v 1.11 2005/10/12 20:46:27 digi_casi Exp $
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
#include <enigma_dyn_epg.h>
#include <enigma_dyn_wap.h>

using namespace std;

extern eString zap[5][5];
extern eString getCurService();
extern bool onSameTP(const eServiceReferenceDVB& ref1, const eServiceReferenceDVB &ref2); // implemented in timer.cpp

struct countTimer
{
	int &count;
	bool repeating;
	countTimer(int &count,bool repeating)
		:count(count), repeating(repeating)
	{
	}

	void operator()(ePlaylistEntry *se)
	{
		if (se->type&ePlaylistEntry::isRepeating)
		{
			if (repeating)
				++count;
		}
		else
		{
			if (!repeating)
				++count;
		}
	}
};

static eString admin2(eString command)
{
	if (command == "shutdown")
	{
		if (eSystemInfo::getInstance()->canShutdown())
			eZap::getInstance()->quit();
	}
	else
	if (command == "reboot")
		eZap::getInstance()->quit(4);
	else
	if (command == "restart")
		eZap::getInstance()->quit(2);
	else
	if (command == "wakeup")
	{
		if (eZapStandby::getInstance())
			eZapStandby::getInstance()->wakeUp(0);
	}
	else
	if (command == "standby")
	{
		if (eZapStandby::getInstance())
			eZapMain::getInstance()->gotoStandby();
	}

	return "<?xml version=\"1.0\"?><!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\"><wml><card title=\"Info\"><p>Command " + command + " initiated.</p></card></wml>";
}

class eWapNavigatorListDirectory: public Object
{
	eString &result;
	eString origpath;
	eString path;
	eServiceInterface &iface;
public:
	eWapNavigatorListDirectory(eString &result, eString origpath, eString path, eServiceInterface &iface): result(result), origpath(origpath), path(path), iface(iface)
	{
		eDebug("path: %s", path.c_str());
	}
	void addEntry(const eServiceReference &e)
	{
#ifndef DISABLE_FILE
		if (eDVB::getInstance()->recorder && !e.path && !e.flags)
		{
			if (!onSameTP(eDVB::getInstance()->recorder->recRef,(eServiceReferenceDVB&)e))
				return;
		}
#endif
		eString serviceRef = ref2string(e);

		if (!(e.flags & eServiceReference::isDirectory))
			result += "<a href=\"/wap?mode=zapto,path=" + serviceRef + "\">";
		else
			result += "<a href=\"/wap?mode=zap,path=" + serviceRef + "\">";

		eService *service = iface.addRef(e);
		if (!service)
			result += "N/A";
		else
		{
			result += filter_string(service->service_name);
			iface.removeRef(e);
		}

		result += "</a>";
		result += "<br/>\n";
	}
};

static eString getWapZapContent(eString path)
{
	eString tpath, result;

	unsigned int pos = 0, lastpos = 0, temp = 0;

	if ((path.find(";", 0)) == eString::npos)
		path = ";" + path;

	while ((pos = path.find(";", lastpos)) != eString::npos)
	{
		lastpos = pos + 1;
		if ((temp = path.find(";", lastpos)) != eString::npos)
			tpath = path.mid(lastpos, temp - lastpos);
		else
			tpath = path.mid(lastpos, strlen(path.c_str()) - lastpos);

		eServiceReference current_service = string2ref(tpath);
		eServiceInterface *iface = eServiceInterface::getInstance();

		// first pass thru is to get all user bouquets
		eWapNavigatorListDirectory navlist(result, path, tpath, *iface);
		Signal1<void, const eServiceReference&> signal;
		signal.connect(slot(navlist, &eWapNavigatorListDirectory::addEntry));
		iface->enterDirectory(current_service, signal);
		eDebug("entered");
		iface->leaveDirectory(current_service);
		eDebug("exited");
	}

	return result;
}

struct getWapEntryString
{
	std::stringstream &result;
	bool repeating;

	getWapEntryString(std::stringstream &result, bool repeating)
		:result(result), repeating(repeating)
	{
	}

	void operator()(ePlaylistEntry* se)
	{
		if (!repeating && se->type & ePlaylistEntry::isRepeating)
			return;
		if (repeating && !(se->type & ePlaylistEntry::isRepeating))
			return;
		tm startTime = *localtime(&se->time_begin);
		time_t time_end = se->time_begin + se->duration;
		tm endTime = *localtime(&time_end);

		eString description = se->service.descr;
		eString channel = getLeft(description, '/');
		if (!channel)
		{
			eService *service = eDVB::getInstance()->settings->getTransponders()->searchService(se->service);
			if (service)
				channel = filter_string(service->service_name);
		}
		if (!channel)
			channel = "No channel available";

		description = getRight(description, '/');
		if (!description)
			description = "No description available";

		result 	<< std::setw(2) << startTime.tm_mday << '.'
			<< std::setw(2) << startTime.tm_mon+1 << ". - "
			<< getTimeStr(&startTime, 0)
			<< " / "
			<< std::setw(2) << endTime.tm_mday << '.'
			<< std::setw(2) << endTime.tm_mon+1 << ". - "
			<< getTimeStr(&endTime, 0)
			<< "<br/>"
			<< channel
			<< "<br/>"
			<< description
			<< "<br/>";
	}
};

static eString wapTimerList(void)
{
	std::stringstream result;
	eString tmp = readFile(TEMPLATE_DIR + "wapTimerList.tmp");

	int count = 0;
	eTimerManager::getInstance()->forEachEntry(countTimer(count, false));
	if (count)
	{
		result << std::setfill('0');
		if (!eTimerManager::getInstance()->getTimerCount())
			result << eString("No timer events available");
		else
			eTimerManager::getInstance()->forEachEntry(getWapEntryString(result, 0));
	}
	else
		result << eString("No timer events available");

	tmp.strReplace("#BODY#", result.str());

	return tmp;
}
static eString wapEPG(int page)
{
	std::stringstream result;
	eString description;
	result << std::setfill('0');

	eService* current;

	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (!sapi)
		return "No EPG available";

	eServiceReference ref = sapi->service;

	current = eDVB::getInstance()->settings->getTransponders()->searchService(ref);

	if (!current)
		return "No EPG available";

	eServiceReferenceDVB &rref = (eServiceReferenceDVB&)ref;
	timeMapPtr evt = eEPGCache::getInstance()->getTimeMapPtr(rref);

	if (!evt)
		return "No EPG available";
	else
	{
		timeMap::const_iterator It;
		int tsidonid = (rref.getTransportStreamID().get()<<16)|rref.getOriginalNetworkID().get();

		int i = 0;
		for(It=evt->begin(); It!= evt->end(); ++It)
		{
			if ((i >= page * 25) && (i < (page + 1) * 25))
			{
				EITEvent event(*It->second, tsidonid, It->second->type);
				LocalEventData led;
				led.getLocalData(&event, &description);
				tm* t = localtime(&event.start_time);

				result	<< std::setw(2) << t->tm_mday << '.'
					<< std::setw(2) << t->tm_mon+1 << ". - "
					<< getTimeStr(t, 0)
					<< "<br/>";

				result << "<a href=\"/wap?mode=epgDetails"
							<< ",path=" << ref2string(ref)
							<< ",ID=" << std::hex << event.event_id << std::dec
							<< "\">"
							<< filter_string(description)
							<< "</a><br/>\n";
			}
			i++;
		}
		if (i >= (page + 1) * 25)
		{
			page++;
			result << "<a href=\"wap?mode=epg,page=" << eString().sprintf("%d", page) << "\">Next Page</a><br/>";
		}
	}

	eString tmp = readFile(TEMPLATE_DIR + "wapepg.tmp");
	tmp.strReplace("#CHANNEL#", filter_string(current->service_name));
	tmp.strReplace("#BODY#", result.str());
	return tmp;
}

static eString wapAddTimerEvent(eString opts)
{
	eString result;

	std::map<eString, eString> opt = getRequestOptions(opts, ',');
	eString serviceRef = opt["path"];
	eString eventID = opt["ID"];
	eString eventStartTime = opt["start"];
	eString eventDuration = opt["duration"];
	eString channel = httpUnescape(opt["channel"]);
	eString description = httpUnescape(opt["descr"]);
	if (description == "")
		description = "No description available";

	int eventid;
	sscanf(eventID.c_str(), "%x", &eventid);

//	int timeroffset = 0;
//	if ((eConfig::getInstance()->getKey("/enigma/timeroffset", timeroffset)) != 0)
//		timeroffset = 0;

	int start = atoi(eventStartTime.c_str()); // - (timeroffset * 60);
	int duration = atoi(eventDuration.c_str()); // + (2 * timeroffset * 60);

	ePlaylistEntry entry(string2ref(serviceRef), start, duration, eventid, ePlaylistEntry::stateWaiting | ePlaylistEntry::RecTimerEntry | ePlaylistEntry::recDVR);
	entry.service.descr = channel + "/" + description;

	if (eTimerManager::getInstance()->addEventToTimerList(entry) == -1)
		result += "Timer event could not be added because time of the event overlaps with an already existing event.";
	else
		result += "Timer event was created successfully.";
	eTimerManager::getInstance()->saveTimerList(); //not needed, but in case enigma crashes ;-)
	return result;
}

static eString wapEPGDetails(eString serviceRef, eString eventID)
{
	eString result;
	eService *current = NULL;
	eString ext_description;
	std::stringstream record;
	int eventid;
	eString description = "No description available";

	sscanf(eventID.c_str(), "%x", &eventid);
	eDebug("[ENIGMA_DYN] getEPGDetails: serviceRef = %s, ID = %04x", serviceRef.c_str(), eventid);

	// search for the event... to get the description...
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (sapi)
	{
		eServiceReference ref(string2ref(serviceRef));
		current = eDVB::getInstance()->settings->getTransponders()->searchService((eServiceReferenceDVB&)ref);
		if (current)
		{
			EITEvent *event = eEPGCache::getInstance()->lookupEvent((eServiceReferenceDVB&)ref, eventid);
			if (event)
			{
				LocalEventData led;
				led.getLocalData(event, &description, &ext_description);
				ext_description.strReplace("\n", "<br/>");
				if (!ext_description)
					ext_description = "No detailed information available";
#ifndef DISABLE_FILE
				record << "<a href=\"/wap?mode=addTimerEvent"
					<< ",path=" << ref2string(ref)
					<< ",ID=" << std::hex << event->event_id << std::dec
					<< ",start=" << event->start_time
					<< ",duration=" << event->duration
					<< ",descr=" << filter_string(description)
					<< ",channel=" << filter_string(current->service_name)
					<< "\">Record</a>";
#endif
				delete event;
			}
		}
	}

	result = readFile(TEMPLATE_DIR + "wapEPGDetails.tmp");
	result.strReplace("#EVENT#", filter_string(description));
	result.strReplace("#RECORD#", record.str());
	result.strReplace("#BODY#", filter_string(ext_description));

	return result;
}

static eString wap_web_root(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString result;

	std::map<eString,eString> opt = getRequestOptions(opts, ',');
	eString mode = opt["mode"];
	eString spath = opt["path"];

	content->local_header["Content-Type"]="text/vnd.wap.wml";

	if (mode == "admin")
	{
		eString command = opt["command"];
		result = admin2(command);
	}
	else
	if (mode == "zap")
	{
		if (opts.find("path") == eString::npos)
			spath = zap[ZAPMODETV][ZAPSUBMODEBOUQUETS];
		result = readFile(TEMPLATE_DIR + "wapzap.tmp");
		result.strReplace("#BODY#", getWapZapContent(spath));
	}
	else
	if (mode == "zapto")
	{
		eServiceReference current_service = string2ref(spath);

		if (!(current_service.flags&eServiceReference::isDirectory))	// is playable
			eZapMain::getInstance()->playService(current_service, eZapMain::psSetMode|eZapMain::psDontAdd);

		result = "<?xml version=\"1.0\"?><!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\"><wml><card title=\"Info\"><p>Zap complete.</p></card></wml>";
	}
	else
	if (mode == "epg")
	{
		eString page = opt["page"];
		result = wapEPG(atoi(page.c_str()));
	}
	else
	if (mode == "epgDetails")
	{
		eString eventID = opt["ID"];
		result = wapEPGDetails(spath, eventID);
	}
	else
	if (mode == "addTimerEvent")
	{
		result = wapAddTimerEvent(opts);
		result = "<?xml version=\"1.0\"?><!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\"><wml><card title=\"Info\"><p>" + result + "</p></card></wml>";

	}
	else
	if (mode == "timer")
	{
		result = wapTimerList();
	}
	else
	if (mode == "cleanupTimerList")
	{
		eTimerManager::getInstance()->cleanupEvents();
		eTimerManager::getInstance()->saveTimerList(); //not needed, but in case enigma crashes ;-)
		result = "<?xml version=\"1.0\"?><!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\"><wml><card title=\"Info\"><p>Timers cleaned up.</p></card></wml>";
	}
	else
	if (mode == "clearTimerList")
	{
		eTimerManager::getInstance()->clearEvents();
		eTimerManager::getInstance()->saveTimerList(); //not needed, but in case enigma crashes ;-)
		result = "<?xml version=\"1.0\"?><!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\"><wml><card title=\"Info\"><p>Timer list cleared.</p></card></wml>";
	}
	else
	{
		result = readFile(TEMPLATE_DIR + "wap.tmp");
		result = getEITC(result, "HTML");
		result.strReplace("#SERVICE#", getCurService());
	}

	return result;
}

void ezapWapInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb)
{
	dyn_resolver->addDyn("GET", "/wap", wap_web_root, lockWeb);
}

