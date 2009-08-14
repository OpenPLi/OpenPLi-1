/*
 * $Id: enigma_dyn_timer.cpp,v 1.23 2008/09/24 19:20:16 dbluelle Exp $
 *
 * (C) 2005,2007 by digi_casi <digi_casi@tuxbox.org>
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
#include <enigma_plugins.h>
#include <enigma_standby.h>
#include <sselect.h>
#include <upgrade.h>
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
#include <enigma_dyn.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn_timer.h>
#include <enigma_streamer.h>
#include <enigma_processutils.h>
#include <epgwindow.h>
#include <streaminfo.h>
#include <enigma_mount.h>
#include <parentallock.h>

using namespace std;

extern bool onSameTP(const eServiceReferenceDVB& ref1, const eServiceReferenceDVB& ref2); // implemented in timer.cpp
extern bool canPlayService(const eServiceReference& ref); // implemented in timer.cpp
extern int pdaScreen;
extern eString zap[5][5];
extern eString getZapContent(eString path, int depth, bool addEPG, bool sortList, bool forceAll);
extern bool playService(const eServiceReference& ref);

class myTimerEntry
{
public:
	int start;
	eString timerData;
	myTimerEntry(int pStart, eString pTimerData)
	{
		start = pStart;
		timerData = pTimerData;
	};
	~myTimerEntry() {};
	bool operator < (const myTimerEntry &a) const {return start < a.start;}
};

static eString getAfterEvent(int type)
{
	eString result;
	if (type & ePlaylistEntry::doGoSleep)
		result = "Standby";
	else
	if (eSystemInfo::getInstance()->canShutdown())
	{
		if (type & ePlaylistEntry::doShutdown)
			result = "Shutdown";
	}
	else
		result = "None";
	return result;
}

class eWebNavigatorSearchService: public Object
{
	eString &result;
	eString searched_service;
	eServiceInterface &iface;
public:
	eWebNavigatorSearchService(eString &result, eString searched_service, eServiceInterface &iface): result(result), searched_service(searched_service), iface(iface)
	{
		eDebug("[eWebNavigatorSearchService] searched_service: %s", searched_service.c_str());
	}

	void addEntry(const eServiceReference &e)
	{
		// sorry.. at moment we dont show any directory.. or locked service in webif
		if (e.isLocked() && pinCheck::getInstance()->pLockActive())
			return;

		eService *service = iface.addRef(e);
		if (service)
		{
			eDebug("[eWebNavigatorSearchService] searched_service: %s, service: %s\n", searched_service.c_str(), filter_string(service->service_name).c_str());
			if ((filter_string(service->service_name).upper() == searched_service.upper()) && !result)
			{
				result = ref2string(e);
				eDebug("[eWebNavigatorSearchService] service found: %s\n", searched_service.c_str());
			}
			iface.removeRef(e);
		}
	}
};

struct getEntryString
{
	std::list<myTimerEntry> &myList;
	bool repeating;
	eString format;

	getEntryString(std::list<myTimerEntry> &myList, bool repeating, eString format)
		:myList(myList), repeating(repeating), format(format)
	{
	}

	void operator()(ePlaylistEntry* se)
	{
		eString tmp = readFile(TEMPLATE_DIR + format + "TimerEntry.tmp");
			
		if (!repeating && se->type & ePlaylistEntry::isRepeating)
			return;
		if (repeating && !(se->type & ePlaylistEntry::isRepeating))
			return;
			
		tmp.strReplace("#REFERENCE#", ref2string(se->service));
		
		if (se->type & ePlaylistEntry::isRepeating)
			tmp.strReplace("#TYPE#", "REPEATING");
		else
			tmp.strReplace("#TYPE#", "SINGLE");
		
		tmp.strReplace("#TIMERTYPE#", eString().sprintf("%d", se->type));
		tmp.strReplace("#AFTEREVENT#", getAfterEvent(se->type));
			
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

		if (se->type & ePlaylistEntry::stateFinished)
			tmp.strReplace("#STATEPIC#", "on.gif");
		else
		if (se->type & ePlaylistEntry::stateError)
			tmp.strReplace("#STATEPIC#", "off.gif");
		else
			tmp.strReplace("#STATEPIC#", "trans.gif");
		
		if (se->type & ePlaylistEntry::stateFinished)
			tmp.strReplace("#STATUS#", "FINISHED");
		else
		if (se->type & ePlaylistEntry::stateError)
			tmp.strReplace("#STATUS#", "ERROR");
		else
			tmp.strReplace("#STATUS#", "ACTIVE");

		if (se->type & ePlaylistEntry::isRepeating)
		{
			eString days;
			if (se->type & ePlaylistEntry::Su)
				days += "Su ";
			if (se->type & ePlaylistEntry::Mo)
				days += "Mo ";
			if (se->type & ePlaylistEntry::Tue)
				days += "Tue ";
			if (se->type & ePlaylistEntry::Wed)
				days += "Wed ";
			if (se->type & ePlaylistEntry::Thu)
				days += "Thu ";
			if (se->type & ePlaylistEntry::Fr)
				days += "Fr ";
			if (se->type & ePlaylistEntry::Sa)
				days += "Sa";

			tmp.strReplace("#DAYS#", days);
			tmp.strReplace("#STARTTIME#", eString().sprintf("XX.XX. - %s", getTimeStr(&startTime, 0).c_str()));
			tmp.strReplace("#ENDTIME#", eString().sprintf("XX.XX. - %s", getTimeStr(&endTime, 0).c_str()));
		}
		else
		{
			if (format == "HTML")
				tmp.strReplace("#DAYS#", "&nbsp;");
			else
				tmp.strReplace("#DAYS#", "");
			tmp.strReplace("#STARTTIME#", eString().sprintf("%02d.%02d. - %s", startTime.tm_mday, startTime.tm_mon + 1, getTimeStr(&startTime, 0).c_str()));
			tmp.strReplace("#ENDTIME#", eString().sprintf("%02d.%02d. - %s", endTime.tm_mday, endTime.tm_mon + 1, getTimeStr(&endTime, 0).c_str()));
		}
		
		tmp.strReplace("#START#", eString().sprintf("%d", se->time_begin));
		tmp.strReplace("#DURATION#", eString().sprintf("%d", se->duration));
		tmp.strReplace("#DATE#", eString().sprintf("%02d.%02d.%04d", startTime.tm_mday, startTime.tm_mon + 1, startTime.tm_year + 1900));
		tmp.strReplace("#TIME#", getTimeStr(&startTime, 0));
		tmp.strReplace("#CHANNEL#", httpEscape(htmlChars(channel)));
		tmp.strReplace("#DESCRIPTION#", httpEscape(htmlChars(description)));
		if (se->type & ePlaylistEntry::SwitchTimerEntry)
			tmp.strReplace("#ACTION#", "ZAP");
		else if (se->type & ePlaylistEntry::recDVR)
			tmp.strReplace("#ACTION#", "DVR");
		else if (se->type & ePlaylistEntry::recNgrab)
			tmp.strReplace("#ACTION#", "NGRAB");
		else
			tmp.strReplace("#ACTION#", "&nbsp;");

		myList.push_back(myTimerEntry(se->time_begin, tmp));
	}
};

eString getTimerList(eString format)
{
	eString result = readFile(TEMPLATE_DIR + format + "TimerListBody.tmp");
		
	std::list<myTimerEntry> myList;
	std::list<myTimerEntry>::iterator myIt;
	eString tmp;
	
	// regular timers
	eTimerManager::getInstance()->forEachEntry(getEntryString(myList, 0, format));
	if (myList.size() > 0)
	{
		myList.sort();
		for (myIt = myList.begin(); myIt != myList.end(); ++myIt)
			tmp += myIt->timerData;
		result.strReplace("#TIMER_REGULAR#", tmp);
	}
	else
	{
		if (format == "HTML")
			result.strReplace("#TIMER_REGULAR#", "<tr><td colspan=\"8\">None</td></tr>");
		else
			result.strReplace("#TIMER_REGULAR#", "");
	}

	tmp = "";
	myList.clear();

	// repeating timers
	eTimerManager::getInstance()->forEachEntry(getEntryString(myList, 1, format));
	if (myList.size() > 0)
	{
		myList.sort();
		for (myIt = myList.begin(); myIt != myList.end(); ++myIt)
			tmp += myIt->timerData;
		result.strReplace("#TIMER_REPEATED#", tmp);
	}
	else
	{
		if (format == "HTML")
			result.strReplace("#TIMER_REPEATED#", "<tr><td colspan=\"8\">None</td></tr>");
		else
			result.strReplace("#TIMER_REPEATED#", "");
	}
	
	// buttons
	result.strReplace("#BUTTONCLEANUP#", button(100, "Cleanup", BLUE, "javascript:cleanupTimerList()", "#FFFFFF"));
	result.strReplace("#BUTTONCLEAR#", button(100, "Clear", RED, "javascript:clearTimerList()", "#FFFFFF"));
	result.strReplace("#BUTTONADD#", button(100, "Add", GREEN, "javascript:showAddTimerEventWindow()", "#FFFFFF"));

	return result;
}

static eString cleanupTimerList(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	eTimerManager::getInstance()->cleanupEvents();
	eTimerManager::getInstance()->saveTimerList();
	return closeWindow(content, "", 500);
}

static eString clearTimerList(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	eTimerManager::getInstance()->clearEvents();
	eTimerManager::getInstance()->saveTimerList();
	return closeWindow(content, "", 500);
}

#if 1
// cannot find where this is used.... MM
static eString addTVBrowserTimerEvent(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString result, result1;

	content->local_header["Content-Type"]="text/html; charset=utf-8";
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString command = opt["command"];
	eString sday = opt["sday"];
	eString smonth = opt["smonth"];
	eString syear = opt["syear"];
	eString shour = opt["shour"];
	eString smin = opt["smin"];
	eString eday = opt["eday"];
	eString emonth = opt["emonth"];
	eString eyear = opt["eyear"];
	eString ehour = opt["ehour"];
	eString emin = opt["emin"];
	eString channel = httpUnescape(opt["channel"]);
	eString description = httpUnescape(opt["descr"]);
	if (!description)
		description = "No description available";

	time_t now = time(0) + eDVB::getInstance()->time_difference;
	tm start = *localtime(&now);
	start.tm_mday = atoi(sday.c_str());
	start.tm_mon = atoi(smonth.c_str()) - 1;
	if (syear)
		start.tm_year = (atoi(syear.c_str()) % 100) + 100;
	start.tm_hour = atoi(shour.c_str());
	start.tm_min = atoi(smin.c_str());
	start.tm_sec = 0;
	start.tm_isdst = -1;
	tm end = *localtime(&now);
	end.tm_mday = atoi(eday.c_str());
	end.tm_mon = atoi(emonth.c_str()) - 1;
	if (eyear)
		end.tm_year = (atoi(eyear.c_str()) % 100) + 100;
	end.tm_hour = atoi(ehour.c_str());
	end.tm_min = atoi(emin.c_str());
	end.tm_sec = 0;
	end.tm_isdst = -1;

	time_t eventStartTime = mktime(&start);
	time_t eventEndTime = mktime(&end);
	int duration = eventEndTime - eventStartTime;

	if (channel.find("/") != eString::npos)
	{
		eString tmp = channel;
		channel = getLeft(tmp, '/');
		result1 = getRight(tmp, '/');
	}
	else
	{
		// determine service reference
		eServiceInterface *iface = eServiceInterface::getInstance();
		eServiceReference all_services = eServiceReference(eServiceReference::idDVB,
			eServiceReference::flagDirectory|eServiceReference::shouldSort,
			-2, -1, 0xFFFFFFFF);

		eWebNavigatorSearchService navlist(result1, channel, *iface);
		Signal1<void, const eServiceReference&> signal;
		signal.connect(slot(navlist, &eWebNavigatorSearchService::addEntry));
		iface->enterDirectory(all_services, signal);
		eDebug("entered");
		iface->leaveDirectory(all_services);
		eDebug("exited");
	}

	if (command == "zap")
	{
		if (result1)
		{
			playService(string2ref(result1));
		}
		else
		{
			content->code = 400;
			content->code_descr = "Function failed.";
			result = "TVBrowser and Enigma service name don't match.";
		}
	}
	else
	{
		if (result1)
		{
			if (command == "add")
			{
				ePlaylistEntry entry(string2ref(result1), eventStartTime, duration, -1, ePlaylistEntry::stateWaiting | ePlaylistEntry::RecTimerEntry | ePlaylistEntry::recDVR);
				entry.service.descr = channel + "/" + description;

				if (eTimerManager::getInstance()->addEventToTimerList(entry) == -1)
				{
					content->code = 400;
					content->code_descr = "Function failed.";
					result = "Timer event could not be added because time of the event overlaps with an already existing event.";
				}
				else
					result = "Timer event was created successfully.";
				eTimerManager::getInstance()->saveTimerList();
			}
			if (command == "delete")
			{
				ePlaylistEntry e(
					string2ref(result1),
					eventStartTime,
					-1, -1, ePlaylistEntry::stateWaiting | ePlaylistEntry::RecTimerEntry | ePlaylistEntry::recDVR);

				eTimerManager::getInstance()->deleteEventFromTimerList(e, true);
				eTimerManager::getInstance()->saveTimerList();
				result = "Timer event deleted successfully.";
			}
		}
		else
		{
			if (command == "add")
			{
				content->code = 400;
				content->code_descr = "Function failed.";
				result = "TVBrowser and Enigma service name don't match.";
			}
			else
				result = "Service of timer event does not exist, or no longer exists.";
		}
	}

	return result;
}
#endif

static eString deleteTimerEvent(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString serviceRef = opt["ref"];
	eString eventType = opt["type"];
	eString eventStartTime = opt["start"];
	eString force = opt["force"];
	eString result;

	eDebug("[ENIGMA_DYN] deleteTimerEvent: serviceRef = %s, type = %s, start = %s", serviceRef.c_str(), eventType.c_str(), eventStartTime.c_str());

	ePlaylistEntry e(
		string2ref(serviceRef),
		atoi(eventStartTime.c_str()),
		-1, -1, atoi(eventType.c_str()));

	int ret = eTimerManager::getInstance()->deleteEventFromTimerList(e, (force == "yes"));

	content->local_header["Content-Type"]="text/html; charset=utf-8";

	if (ret == -1)  // event currently running...
	{
		// ask user if he really wants to do this..
		// then call deleteEventFromtTimerList again.. with true as second parameter..
		// then the running event will aborted
		result = readFile(TEMPLATE_DIR + "queryDeleteTimer.tmp");
		opts.strReplace("force=no", "force=yes");
		if (opts.find("?") != 0)
			opts = "?" + opts;
		result.strReplace("#URL#", "/deleteTimerEvent" + opts);
	}
	else
	{
		eTimerManager::getInstance()->saveTimerList();
		result = readFile(TEMPLATE_DIR + "deleteTimerComplete.tmp");
	}

	return result;
}

static eString genOptions(int start, int end, int delta, int selected)
{
	std::stringstream result;
	for (int i = start; i <= end; i += delta)
	{
		if (i == selected)
			result << "<option selected>";
		else
			result << "<option>";
		result << std::setfill('0') << std::setw(2);
		result << i;
		result << "</option>";
	}
	return result.str();
}

static eString changeTimerEvent(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString result;

	content->local_header["Content-Type"]="text/html; charset=utf-8";
	eDebug("changeTimer: request dirpath=%s opts=%s", dirpath.c_str(), opts.c_str());
	std::map<eString, eString> opt = getRequestOptions(opts, '&');

	int clktype = 0;
	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);

	eString user = opt["user"];

	// to find old event in timerlist..
	eString serviceRef = opt["ref"];
	eString oldEventType = opt["old_type"];
	int oldType = atoi(oldEventType.c_str());
	eString oldStartTime = opt["old_stime"];
	eString newEventType = opt["type"];
	bool repeating = newEventType == "repeating";
	if (repeating)
		oldType |= ePlaylistEntry::isRepeating;
	else
		oldType &= ~ePlaylistEntry::isRepeating;

	eString sday = opt["sday"];
	eString smonth = opt["smonth"];
	eString syear = opt["syear"];
	eString shour = opt["shour"];
	eString smin = opt["smin"];
	eString sampm = opt["sampm"];
	eString eday = opt["eday"];
	eString emonth = opt["emonth"];
	eString eyear = opt["eyear"];
	eString ehour = opt["ehour"];
	eString emin = opt["emin"];
	eString eampm = opt["eampm"];
	eString description = httpUnescape(opt["descr"]);
	eString channel = httpUnescape(opt["channel"]);
	eString after_event = opt["after_event"];
	eString force = opt["force"];
	eString mo = opt["mo"];
	eString tu = opt["tu"];
	eString we = opt["we"];
	eString th = opt["th"];
	eString fr = opt["fr"];
	eString sa = opt["sa"];
	eString su = opt["su"];
	eString action = opt["action"];
	eDebug("[ENIGMA_DYN] changeTimerEvent %s.%s.%s - %s:%s %s, %s.%s.%s - %s:%s %s", sday.c_str(), smonth.c_str(), syear.c_str(), shour.c_str(), smin.c_str(), sampm.c_str(), eday.c_str(), emonth.c_str(), eyear.c_str(), ehour.c_str(), emin.c_str(), eampm.c_str());

	time_t now = time(0)+eDVB::getInstance()->time_difference;
	tm start = *localtime(&now);
	start.tm_isdst = -1;
	if (repeating)
	{
		start.tm_year = 70;  // 1.1.1970
		start.tm_mon = 0;
		start.tm_mday = 1;
	}
	else
	{
		start.tm_mday = atoi(sday.c_str());
		start.tm_mon = atoi(smonth.c_str()) - 1;
		start.tm_year = atoi(syear.c_str()) - 1900;
	}
	start.tm_hour = atoi(shour.c_str());
	start.tm_min = atoi(smin.c_str());
	start.tm_sec = 0;

	tm end = *localtime(&now);
	end.tm_isdst = -1;
	if (repeating)
	{
		end.tm_year = 70;  // 1.1.1970
		end.tm_mon = 0;
		end.tm_mday = 1;
	}
	else
	{
		end.tm_mday = atoi(eday.c_str());
		end.tm_mon = atoi(emonth.c_str()) - 1;
		end.tm_year = atoi(eyear.c_str()) - 1900;
	}
	end.tm_hour = atoi(ehour.c_str());
	end.tm_min = atoi(emin.c_str());
	end.tm_sec = 0;
	if (clktype)
	{
		int ampm = atoi(eampm.c_str());
		if (end.tm_hour == 12)
			end.tm_hour = 0;
		if (!ampm)
			end.tm_hour += 12;
		ampm = atoi(sampm.c_str());
		if (start.tm_hour == 12)
			start.tm_hour = 0;
		if (!ampm)
			start.tm_hour += 12;
	}

	if ( repeating &&   // endTime after 0:00
		end.tm_hour*60+end.tm_min <
		start.tm_hour*60+start.tm_min )
	{
		end.tm_mday++;
	}

	time_t eventStartTime = mktime(&start);
	time_t eventEndTime = mktime(&end);
	int duration = eventEndTime - eventStartTime;

	eServiceReference ref = string2ref(serviceRef);

	ePlaylistEntry oldEvent(
		ref,
		atoi(oldStartTime.c_str()),
		-1, -1, oldType);

	if (oldStartTime < now && eventStartTime >= now)
	{
		oldType &=
			~(ePlaylistEntry::stateRunning|
				ePlaylistEntry::statePaused|
				ePlaylistEntry::stateFinished|
				ePlaylistEntry::stateError|
				ePlaylistEntry::errorNoSpaceLeft|
				ePlaylistEntry::errorUserAborted|
				ePlaylistEntry::errorZapFailed|
				ePlaylistEntry::errorOutdated);
	}

	oldType &= ~(ePlaylistEntry::doGoSleep|ePlaylistEntry::doShutdown);
	oldType |= ePlaylistEntry::stateWaiting;
	oldType |= atoi(after_event.c_str());

	oldType &= ~(ePlaylistEntry::SwitchTimerEntry | ePlaylistEntry::RecTimerEntry);
	oldType &= ~(ePlaylistEntry::recDVR | ePlaylistEntry::recNgrab);
	if (action == "zap")
	{
		oldType |= ePlaylistEntry::SwitchTimerEntry;
	}
	else
	{
		oldType |= ePlaylistEntry::RecTimerEntry;

		if (action == "ngrab")
			oldType |= ePlaylistEntry::recNgrab;
		else
			oldType |= ePlaylistEntry::recDVR;
	}

	if (oldType & ePlaylistEntry::isRepeating)
	{
		if (mo == "on")
			oldType |= ePlaylistEntry::Mo;
		else
			oldType &= ~ePlaylistEntry::Mo;
		if (tu == "on")
			oldType |= ePlaylistEntry::Tue;
		else
			oldType &= ~ePlaylistEntry::Tue;
		if (we == "on")
			oldType |= ePlaylistEntry::Wed;
		else
			oldType &= ~ePlaylistEntry::Wed;
		if (th == "on")
			oldType |= ePlaylistEntry::Thu;
		else
			oldType &= ~ePlaylistEntry::Thu;
		if (fr == "on")
			oldType |= ePlaylistEntry::Fr;
		else
			oldType &= ~ePlaylistEntry::Fr;
		if (sa == "on")
			oldType |= ePlaylistEntry::Sa;
		else
			oldType &= ~ePlaylistEntry::Sa;
		if (su == "on")
			oldType |= ePlaylistEntry::Su;
		else
			oldType &= ~ePlaylistEntry::Su;
	}
	else
	{
		oldType &= ~(	ePlaylistEntry::Mo |
				ePlaylistEntry::Tue |
				ePlaylistEntry::Wed |
				ePlaylistEntry::Thu |
				ePlaylistEntry::Fr |
				ePlaylistEntry::Sa |
				ePlaylistEntry::Su);
	}

	ref.descr = channel + "/" + description;
	ePlaylistEntry newEvent(
		ref,
		eventStartTime,
		duration,
		-1,
		oldType);

	int ret = eTimerManager::getInstance()->modifyEventInTimerList(oldEvent, newEvent, (force == "yes"));

	if (ret == -1)  // event currently running...
	{
		// ask user if he wants to update only after_event action and duration
		// then call modifyEvent again.. with true as third parameter..
		if (user == "")
		{
			result = readFile(TEMPLATE_DIR + "queryEditTimer.tmp");
			opts.strReplace("force=no", "force=yes");
			if (opts.find("?") != 0)
				opts = "?" + opts;
			result.strReplace("#URL#", "/changeTimerEvent" + opts);
		}
		else
			result = "Timer event is already active!";
	}
	else
	{
		if (user == "")
			result = "<script language=\"javascript\">window.close();</script>";
		else
			result = "Timer event changed successfully.";
		eTimerManager::getInstance()->saveTimerList();
	}
	return result;
}

static eString addTimerEvent(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString result;

	content->local_header["Content-Type"]="text/html; charset=utf-8";
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString serviceRef = opt["ref"];
	eString eventStartTimeS = opt["start"];
	eString eventDurationS = opt["duration"];
	eString sday = opt["sday"];
	eString smonth = opt["smonth"];
	eString syear = opt["syear"];
	eString shour = opt["shour"];
	eString smin = opt["smin"];
	eString sampm = opt["sampm"];
	eString eday = opt["eday"];
	eString emonth = opt["emonth"];
	eString eyear = opt["eyear"];
	eString ehour = opt["ehour"];
	eString emin = opt["emin"];
	eString eampm = opt["eampm"];
	eString description = httpUnescape(opt["descr"]);
	description.strReplace("~", "&");
	eString channel = httpUnescape(opt["channel"]);
	eString after_event = opt["after_event"];
	eString timer = opt["timer"];
	eString mo = opt["mo"];
	eString tu = opt["tu"];
	eString we = opt["we"];
	eString th = opt["th"];
	eString fr = opt["fr"];
	eString sa = opt["sa"];
	eString su = opt["su"];
	eString action = opt["action"];
	bool repeating = timer == "repeating";
	eDebug("[ENIGMA_DYN] addTimerEvent %s.%s.%s - %s:%s %s, %s.%s.%s - %s:%s %s", sday.c_str(), smonth.c_str(), syear.c_str(), shour.c_str(), smin.c_str(), sampm.c_str(), eday.c_str(), emonth.c_str(), eyear.c_str(), ehour.c_str(), emin.c_str(), eampm.c_str());

	time_t now = time(0) + eDVB::getInstance()->time_difference;

	int eventDuration = 0;
	time_t eventStartTime, eventEndTime;

	int clktype = 0;
	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);

	if (eventStartTimeS && eventDurationS)
	{
		eventStartTime = atoi(eventStartTimeS.c_str());
		eventDuration = atoi(eventDurationS.c_str());
	}
	else
	{
		tm start = *localtime(&now);
		start.tm_isdst = -1;
		if (repeating)
		{
			start.tm_year = 70;  // 1.1.1970
			start.tm_mon = 0;
			start.tm_mday = 1;
		}
		else
		{
			start.tm_mday = atoi(sday.c_str());
			start.tm_mon = atoi(smonth.c_str()) - 1;
			start.tm_year = atoi(syear.c_str()) - 1900;
		}
		start.tm_hour = atoi(shour.c_str());
		start.tm_min = atoi(smin.c_str());
		start.tm_sec = 0;

		tm end = *localtime(&now);
		end.tm_isdst = -1;
		if (repeating)
		{
			end.tm_year = 70;  // 1.1.1970
			end.tm_mon = 0;
			end.tm_mday = 1;
		}
		else
		{
			end.tm_mday = atoi(eday.c_str());
			end.tm_mon = atoi(emonth.c_str()) - 1;
			end.tm_year = atoi(eyear.c_str()) - 1900;
		}
		end.tm_hour = atoi(ehour.c_str());
		end.tm_min = atoi(emin.c_str());
		end.tm_sec = 0;
		if (clktype)
		{
			int ampm = atoi(eampm.c_str());
			if (end.tm_hour == 12)
				end.tm_hour = 0;
			if (!ampm)
				end.tm_hour += 12;
			ampm = atoi(sampm.c_str());
			if (start.tm_hour == 12)
				start.tm_hour = 0;
			if (!ampm)
				start.tm_hour += 12;
		}
		if ( repeating &&   // endTime after 0:00
			end.tm_hour*60+end.tm_min <
			start.tm_hour*60+start.tm_min )
		{
			end.tm_mday++;
		}

		eventStartTime = mktime(&start);
		eventEndTime = mktime(&end);

		eventDuration = eventEndTime - eventStartTime;
	}

	int timeroffsetstart = 0;
	eConfig::getInstance()->getKey("/enigma/timeroffsetstart", timeroffsetstart);
	int timeroffsetstop = 0;
	eConfig::getInstance()->getKey("/enigma/timeroffsetstop", timeroffsetstop);

	eventStartTime = eventStartTime - (timeroffsetstart * 60);
	eventDuration = eventDuration + (timeroffsetstart * 60) + (timeroffsetstop * 60);

	int type = (after_event) ? atoi(after_event.c_str()) : 0;

	type |= ePlaylistEntry::stateWaiting;

	if (action == "zap")
	{
		type |= ePlaylistEntry::SwitchTimerEntry;
	}
	else
	{
		type |= ePlaylistEntry::RecTimerEntry;

		if (action == "ngrab")
			type |= ePlaylistEntry::recNgrab;
		else
			type |= ePlaylistEntry::recDVR;
	}
	if (repeating)
	{
		type |= ePlaylistEntry::isRepeating;
		if (mo == "on")
			type |= ePlaylistEntry::Mo;
		if (tu == "on")
			type |= ePlaylistEntry::Tue;
		if (we == "on")
			type |= ePlaylistEntry::Wed;
		if (th == "on")
			type |= ePlaylistEntry::Thu;
		if (fr == "on")
			type |= ePlaylistEntry::Fr;
		if (sa == "on")
			type |= ePlaylistEntry::Sa;
		if (su == "on")
			type |= ePlaylistEntry::Su;
	}

	ePlaylistEntry entry(string2ref(serviceRef), eventStartTime, eventDuration, -1, type);
	if (!channel)
	{
		channel = eDVB::getInstance()->settings->getTransponders()->searchService(string2ref(serviceRef))->service_name;
	}
	else
	{
		// remove satellite position e.g. (19.2E)
		if (channel.find("(") == 0)
		{
			unsigned int pos = channel.find(")");
			if ((pos < channel.length() - 2) && (pos != eString::npos))
				channel = channel.right(channel.length() - pos - 2);
		}
	}
	entry.service.descr = channel + "/" + description;

	if (eTimerManager::getInstance()->addEventToTimerList(entry) == -1)
		result += "Timer event could not be added because time of the event overlaps with an already existing event.";
	else
	{
		result += "Timer event was created successfully.";
		eTimerManager::getInstance()->saveTimerList();
	}

	return result;
}

static eString buildAfterEventOpts(int type)
{
	std::stringstream afterOpts;

	// get default action for timer end
	int defaultaction = 0;

	eConfig::getInstance()->getKey("/enigma/timerenddefaultaction", defaultaction);

	// only use default action when creating a new timer (type == 0)
	if (type & ePlaylistEntry::doGoSleep || type & ePlaylistEntry::doShutdown || type == 0 && defaultaction > 0)
	{
		afterOpts << "<option value=\"0\">";
		afterOpts << "Nothing"
		<< "</option>";
	}
 	else
	{
		afterOpts << "<option selected value=\"0\">";
		afterOpts << "Nothing"
		<< "</option>";
	}
	if (type & ePlaylistEntry::doGoSleep || type== 0 && defaultaction == ePlaylistEntry::doGoSleep)
		afterOpts << "<option selected value=\"" << ePlaylistEntry::doGoSleep << "\">";
	else
		afterOpts << "<option value=\"" << ePlaylistEntry::doGoSleep << "\">";
	afterOpts << "Standby"
		<< "</option>";
	if (eSystemInfo::getInstance()->canShutdown())
	{
		if (type & ePlaylistEntry::doShutdown || type == 0 && defaultaction == ePlaylistEntry::doShutdown)
			afterOpts << "<option selected value=\"" << ePlaylistEntry::doShutdown << "\">";
		else
			afterOpts << "<option value=\"" << ePlaylistEntry::doShutdown << "\">";
		afterOpts << "Shutdown"
		<< "</option>";
	}
	return afterOpts.str();
}

static eString showEditTimerEventWindow(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString serviceRef = opt["ref"];
	eString eventStartTime = opt["start"];
	eString eventDuration = opt["duration"];
	eString description = httpUnescape(opt["descr"]);

	// this is only for renamed services (or subservices)... changing this in the edit dialog has no effect to
	// the recording service
	eString channel = httpUnescape(opt["channel"]);
	eString eventType = opt["type"];

	time_t eventStart = atoi(eventStartTime.c_str());
	time_t eventEnd = eventStart + atoi(eventDuration.c_str());
	tm start = *localtime(&eventStart);
#if 0
	// if we allow per minute recording times, edits should not round to 5 minutes...
	if (eventEnd % 300 > 0)
		eventEnd = eventEnd / 300 * 300 + 300;
#endif
	tm end = *localtime(&eventEnd);
	int evType = atoi(eventType.c_str());

	eString result = readFile(TEMPLATE_DIR + "editTimerEvent.tmp");

	result.strReplace("#CSS#", (pdaScreen == 0) ? "webif.css" : "webif_small.css");

	if (evType & ePlaylistEntry::SwitchTimerEntry)
	{
		result.strReplace("#ZAP#", "selected");
	}
	else
	{
		if (evType & ePlaylistEntry::recDVR)
			result.strReplace("#DVR#", "selected");
		else
		if (evType & ePlaylistEntry::recNgrab)
			result.strReplace("#NGRAB#", "selected");
	}

	if (evType & ePlaylistEntry::isRepeating)
	{
		result.strReplace("#REPEATING#", "selected");
		result.strReplace("#REGULAR#", "");
	}
	else
	{
		result.strReplace("#REGULAR#", "selected");
		result.strReplace("#REPEATING#", "");
	}

	result.strReplace("#AFTEROPTS#", buildAfterEventOpts(evType));
	// these three values we need to find the old event in timerlist...
	result.strReplace("#SERVICEREF#", serviceRef);
	result.strReplace("#OLD_TYPE#", eventType);
	result.strReplace("#OLD_STIME#", eventStartTime);

	result.strReplace("#SDAYOPTS#", genOptions(1, 31, 1, start.tm_mday));
	result.strReplace("#SMONTHOPTS#", genOptions(1, 12, 1, start.tm_mon + 1));
	result.strReplace("#SYEAROPTS#", genOptions(start.tm_year + 1900, start.tm_year + 1904, 1, start.tm_year + 1900));
	int clktype = 0;
	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);
	if (clktype)
	{
		result.strReplace("#SHOUROPTS#", genOptions(1, 12, 1, start.tm_hour%12 ? start.tm_hour%12 : 12));
		result.strReplace("#EHOUROPTS#", genOptions(1, 12, 1, end.tm_hour%12 ? end.tm_hour%12 : 12));
		result.strReplace("#SHOWAMPWSTART#", "");
		result.strReplace("#SHOWAMPWSTOP#", "");
		if (start.tm_hour < 12)
		{
			result.strReplace("#SAMCHECKED#", "checked");
			result.strReplace("#SPMCHECKED#", "");
		}
		else
		{
			result.strReplace("#SAMCHECKED#", "");
			result.strReplace("#SPMCHECKED#", "checked");
		}
		if (end.tm_hour < 12)
		{
			result.strReplace("#EAMCHECKED#", "checked");
			result.strReplace("#EPMCHECKED#", "");
		}
		else
		{
			result.strReplace("#EAMCHECKED#", "");
			result.strReplace("#EPMCHECKED#", "checked");
		}
		// result.strReplace("#SAMPM#", genAMPM(end.tm_hour));
		// result.strReplace("#SAMPM#", genAMPM(start.tm_hour));
	}
	else
	{
		result.strReplace("#SHOUROPTS#", genOptions(0, 23, 1, start.tm_hour));
		result.strReplace("#EHOUROPTS#", genOptions(0, 23, 1, end.tm_hour));
		result.strReplace("#SHOWAMPWSTART#", "<!--");
		result.strReplace("#SHOWAMPWSTOP#", "-->");
	}
	result.strReplace("#SMINOPTS#", genOptions(0, 59, 1, start.tm_min));

	result.strReplace("#EDAYOPTS#", genOptions(1, 31, 1, end.tm_mday));
	result.strReplace("#EMONTHOPTS#", genOptions(1, 12, 1, end.tm_mon + 1));
	result.strReplace("#EYEAROPTS#", genOptions(end.tm_year + 1900, end.tm_year + 1904, 1, end.tm_year + 1900));
	result.strReplace("#EMINOPTS#", genOptions(0, 59, 1, end.tm_min));
	result.strReplace("#CHANNEL#", channel);
	result.strReplace("#DESCRIPTION#", description);
	result.strReplace("#MO#", (evType & ePlaylistEntry::Mo) ? "checked" : "");
	result.strReplace("#TU#", (evType & ePlaylistEntry::Tue) ? "checked" : "");
	result.strReplace("#WE#", (evType & ePlaylistEntry::Wed) ? "checked" : "");
	result.strReplace("#TH#", (evType & ePlaylistEntry::Thu) ? "checked" : "");
	result.strReplace("#FR#", (evType & ePlaylistEntry::Fr) ? "checked" : "");
	result.strReplace("#SA#", (evType & ePlaylistEntry::Sa) ? "checked" : "");
	result.strReplace("#SU#", (evType & ePlaylistEntry::Su) ? "checked" : "");
	eTimerManager::getInstance()->saveTimerList();
	return result;
}

static eString showAddTimerEventWindow(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	std::map<eString, eString> opt = getRequestOptions(opts, '&');

	int clktype = 0;
	time_t now = time(0) + eDVB::getInstance()->time_difference;
	tm start = *localtime(&now);
	tm end = *localtime(&now);

	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);

	eString result = readFile(TEMPLATE_DIR + "addTimerEvent.tmp");

	result.strReplace("#AFTEROPTS#", buildAfterEventOpts(0));

	result.strReplace("#SDAYOPTS#", genOptions(1, 31, 1, start.tm_mday));
	result.strReplace("#SMONTHOPTS#", genOptions(1, 12, 1, start.tm_mon + 1));
	result.strReplace("#SYEAROPTS#", genOptions(start.tm_year + 1900, start.tm_year + 1904, 1, start.tm_year + 1900));
	if (clktype)
	{
		result.strReplace("#SHOUROPTS#", genOptions(1, 12, 1, start.tm_hour%12 ? start.tm_hour%12 : 12));
		result.strReplace("#EHOUROPTS#", genOptions(1, 12, 1, end.tm_hour%12 ? end.tm_hour%12 : 12));
		result.strReplace("#SHOWAMPWSTART#", "");
		result.strReplace("#SHOWAMPWSTOP#", "");
		if (start.tm_hour < 12)
		{
			result.strReplace("#SAMCHECKED#", "checked");
			result.strReplace("#SPMCHECKED#", "");
		}
		else
		{
			result.strReplace("#SAMCHECKED#", "");
			result.strReplace("#SPMCHECKED#", "checked");
		}
		if (end.tm_hour < 12)
		{
			result.strReplace("#EAMCHECKED#", "checked");
			result.strReplace("#EPMCHECKED#", "");
		}
		else
		{
			result.strReplace("#EAMCHECKED#", "");
			result.strReplace("#EPMCHECKED#", "checked");
		}
		// result.strReplace("#SAMPM#", genAMPM(end.tm_hour));
		// result.strReplace("#SAMPM#", genAMPM(start.tm_hour));
	}
	else
	{
		result.strReplace("#SHOUROPTS#", genOptions(0, 23, 1, start.tm_hour));
		result.strReplace("#EHOUROPTS#", genOptions(0, 23, 1, end.tm_hour));
		result.strReplace("#SHOWAMPWSTART#", "<!--");
		result.strReplace("#SHOWAMPWSTOP#", "-->");
	}
	result.strReplace("#SMINOPTS#", genOptions(0, 59, 1, start.tm_min));

	result.strReplace("#EDAYOPTS#", genOptions(1, 31, 1, end.tm_mday));
	result.strReplace("#EMONTHOPTS#", genOptions(1, 12, 1, end.tm_mon + 1));
	result.strReplace("#EYEAROPTS#", genOptions(end.tm_year + 1900, end.tm_year + 1904, 1, end.tm_year + 1900));
	result.strReplace("#EMINOPTS#", genOptions(0, 59, 1, end.tm_min));

	result.strReplace("#ZAPDATA#", getZapContent(zap[ZAPMODETV][ZAPSUBMODEBOUQUETS], 2, false, false, true));

	return result;
}

void ezapTimerInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb)
{
	dyn_resolver->addDyn("GET", "/addTimerEvent", addTimerEvent, lockWeb);
#if 1
// cannot find where this one is used. MM
	dyn_resolver->addDyn("GET", "/TVBrowserTimerEvent", addTVBrowserTimerEvent, lockWeb);
#endif
	dyn_resolver->addDyn("GET", "/deleteTimerEvent", deleteTimerEvent, lockWeb);
	dyn_resolver->addDyn("GET", "/showEditTimerEventWindow", showEditTimerEventWindow, lockWeb);
	dyn_resolver->addDyn("GET", "/showAddTimerEventWindow", showAddTimerEventWindow, lockWeb);
	dyn_resolver->addDyn("GET", "/changeTimerEvent", changeTimerEvent, lockWeb);
	dyn_resolver->addDyn("GET", "/cleanupTimerList", cleanupTimerList, lockWeb);
	dyn_resolver->addDyn("GET", "/clearTimerList", clearTimerList, lockWeb);
}

