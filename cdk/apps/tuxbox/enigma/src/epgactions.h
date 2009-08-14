#ifndef __src_epgactions_h
#define __src_epgactions_h

#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/dvb/serviceplaylist.h>

struct epgSelectorActions
{
	eActionMap map;
	eAction addDVRTimerEvent, addNGRABTimerEvent, addSwitchTimerEvent,
		removeTimerEvent, showExtendedInfo, searchEPG; // EPG search ... added "searchEPG"
	epgSelectorActions()
		:map("epgSelector", _("EPG selector")),
		addDVRTimerEvent(map, "addDVRTimerEvent", _("add this event as DVR Event to timer list"), eAction::prioDialog ),
		addNGRABTimerEvent(map, "addNGRABTimerEvent", _("add this event as NGRAB Event to timer list"), eAction::prioDialog ),
		addSwitchTimerEvent(map, "addSwitchTimerEvent", _("add this event as simple Switch Event to timer list"), eAction::prioDialog ),
		removeTimerEvent(map, "removeTimerEvent", _("remove this event from timer list"), eAction::prioDialog ),
		showExtendedInfo(map, "showExtendedInfo", _("show extended event information"), eAction::prioDialog ),
		searchEPG(map, "searchEPG", _("search for event information") , eAction::prioDialog ) // EPG search
	{
	}
	int checkTimerActions( const void *action )
	{
		int ret = -1;
#ifndef DISABLE_FILE
		if ( eSystemInfo::getInstance()->canRecordTS()
			&& action == &addDVRTimerEvent )
			ret = ePlaylistEntry::RecTimerEntry |
								ePlaylistEntry::recDVR|
								ePlaylistEntry::stateWaiting;
		else
#endif
#ifndef DISABLE_NETWORK
		if ( eSystemInfo::getInstance()->hasNetwork()
			&& action == &addNGRABTimerEvent )
			ret = ePlaylistEntry::RecTimerEntry|
								ePlaylistEntry::recNgrab|
								ePlaylistEntry::stateWaiting;
		else
#endif
		if (action == &addSwitchTimerEvent)
			ret = ePlaylistEntry::SwitchTimerEntry|
								ePlaylistEntry::stateWaiting;
		return ret;
	}
};

extern eAutoInitP0<epgSelectorActions> i_epgSelectorActions;

#endif
