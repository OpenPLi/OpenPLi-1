/*
	Timer-Daemon  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

   $Id: timermanager.cpp,v 1.44 2002/10/15 22:42:11 Zwen Exp $

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
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sstream>

#include <dbox/fp.h>

#include <timermanager.h>
#include <timerdclient/timerdclient.h>
#include <debug.h>
#include <sectionsdclient/sectionsdMsg.h>
#include <sectionsdclient/sectionsdclient.h>

//CTimerEvent_NextProgram::EventMap CTimerEvent_NextProgram::events;


//------------------------------------------------------------
CTimerManager::CTimerManager()
{
	eventID = 0;
	eventServer = new CEventServer;

	//thread starten
	if(pthread_create (&thrTimer, NULL, timerThread, (void *) this) != 0 )
	{
		dprintf("CTimerManager::CTimerManager create timerThread failed\n");
	}
	dprintf("timermanager created\n");
}

//------------------------------------------------------------
CTimerManager* CTimerManager::getInstance()
{
	static CTimerManager *instance=NULL;
	if(!instance)
		instance = new CTimerManager;
	return instance;
}


//------------------------------------------------------------
void* CTimerManager::timerThread(void *arg)
{
	CTimerManager *timerManager = (CTimerManager*) arg;
	bool saveEvents;
	int wait = ((int)time(NULL)) % 20; // Start at a multiple of 20 sec
	sleep(wait);
	while(1)
	{
		saveEvents = false;
		time_t now = time(NULL);
		dprintf("Timer Thread time: %u\n", (uint) now);

		// fire events who's time has come
		CTimerEvent *event;
		CTimerEventMap::iterator pos = timerManager->events.begin();
		for(;pos != timerManager->events.end();pos++)
		{
			event = pos->second;
			if(debug) event->printEvent();					// print all events (debug)

			if(event->announceTime > 0 && event->eventState == CTimerd::TIMERSTATE_SCHEDULED ) // if event wants to be announced
				if( event->announceTime <= now )	// check if event announcetime has come
				{
					event->setState(CTimerd::TIMERSTATE_PREANNOUNCE);
					event->announceEvent();							// event specific announce handler
					saveEvents = true;
				}

			if(event->alarmTime > 0 && (event->eventState == CTimerd::TIMERSTATE_SCHEDULED || event->eventState == CTimerd::TIMERSTATE_PREANNOUNCE) )	// if event wants to be fired
				if( event->alarmTime <= now )	// check if event alarmtime has come
				{
					event->setState(CTimerd::TIMERSTATE_ISRUNNING);
					event->fireEvent();										// fire event specific handler
					if(event->stopTime == 0)					// if event needs no stop event
						event->setState(CTimerd::TIMERSTATE_HASFINISHED);
					saveEvents = true;
				}

			if(event->stopTime > 0 && event->eventState == CTimerd::TIMERSTATE_ISRUNNING  )		// check if stopevent is wanted
				if( event->stopTime <= now ) // check if event stoptime has come
				{
					event->stopEvent();							//  event specific stop handler
					event->setState(CTimerd::TIMERSTATE_HASFINISHED); 
					saveEvents = true;
				}

			if(event->eventState == CTimerd::TIMERSTATE_HASFINISHED)
			{
				if(event->eventRepeat != CTimerd::TIMERREPEAT_ONCE)
					event->Reschedule();
				else
					event->setState(CTimerd::TIMERSTATE_TERMINATED);
				saveEvents = true;
			}

			if(event->eventState == CTimerd::TIMERSTATE_TERMINATED)				// event is terminated, so delete it
			{
				delete pos->second;										// delete event
				timerManager->events.erase(pos);				// remove from list
				saveEvents = true;
			}
		}
/*	
	minutes = (int) (zeit - time(NULL)) / 60;
	if (ioctl(fd, FP_IOCTL_SET_WAKEUP_TIMER, &minutes)<0)
			perror("FP_IOCTL_SET_WAKEUP_TIMER");
*/

		if(saveEvents)
			timerManager->saveEventsToConfig();
		(debug)?usleep(10 * 1000000):usleep(20 * 1000000);		// sleep for 10 / 20 seconds
	}
	return 0;
}

//------------------------------------------------------------
CTimerEvent* CTimerManager::getNextEvent()
{
	CTimerEvent *erg = events[0];
	CTimerEventMap::iterator pos = events.begin();
	for(;pos!=events.end();pos++)
	{
		if(pos->second <= erg)
		{
			erg = pos->second;
		}
	}
	return erg;
}

//------------------------------------------------------------
int CTimerManager::addEvent(CTimerEvent* evt, bool save)
{
	eventID++;						// increase unique event id
	evt->eventID = eventID;
	events[eventID] = evt;			// insert into events
	if(save)
	{
		saveEventsToConfig();
	}
	return eventID;					// return unique id
}

//------------------------------------------------------------
bool CTimerManager::removeEvent(int eventID)
{
	if(events.find(eventID)!=events.end())							 // if i have a event with this id
	{
		if( (events[eventID]->eventState == CTimerd::TIMERSTATE_ISRUNNING) && (events[eventID]->stopTime > 0) )
			events[eventID]->stopEvent();	// if event is running an has stopTime 

		events[eventID]->eventState = CTimerd::TIMERSTATE_TERMINATED;		// set the state to terminated
		return true;															// so timerthread will do the rest for us
//		delete events[eventID];
	}
	else
		return false;
//	events.erase(eventID);
}

//------------------------------------------------------------
bool CTimerManager::listEvents(CTimerEventMap &Events)
{
	if(!&Events)
		return false;
	Events.clear();
	if(getInstance()->events.size() > 0)
	{
		CTimerEventMap::iterator pos = getInstance()->events.begin();
		for(int i = 0;pos != getInstance()->events.end();pos++,i++)
			Events[pos->second->eventID] = pos->second;
		return true;
	}
	else
		return false;
}

int CTimerManager::modifyEvent(int eventID, time_t announceTime, time_t alarmTime, time_t stopTime, CTimerd::CTimerEventRepeat evrepeat)
{
	if(events.find(eventID)!=events.end())
	{
		CTimerEvent *event = events[eventID];
		event->announceTime = announceTime;
		event->alarmTime = alarmTime;
		event->stopTime = stopTime;
		event->eventState = CTimerd::TIMERSTATE_SCHEDULED;
		event->eventRepeat = evrepeat;
		saveEventsToConfig();
		return eventID;
	}
	else
		return 0;
}

int CTimerManager::modifyEvent(int eventID, uint apid)
{
	dprintf("Modify Event %d apid %u\n",eventID,apid);
	if(events.find(eventID)!=events.end())
	{
		CTimerEvent *event = events[eventID];
		if(event->eventType == CTimerd::TIMER_RECORD)
		{
			((CTimerEvent_Record*) (event))->eventInfo.apid = apid;
			saveEventsToConfig();
			return eventID;
		}
	}
	return 0;
}

int CTimerManager::rescheduleEvent(int eventID, time_t announceTime, time_t alarmTime, time_t stopTime)
{
	if(events.find(eventID)!=events.end())
	{
		CTimerEvent *event = events[eventID];
		if(event->announceTime > 0)
			event->announceTime += announceTime;
		if(event->alarmTime > 0)
			event->alarmTime += alarmTime;
		if(event->stopTime > 0)
			event->stopTime += stopTime;
		event->eventState = CTimerd::TIMERSTATE_SCHEDULED;
		saveEventsToConfig();
		return eventID;
	}
	else
		return 0;
}

void CTimerManager::saveEventsToConfig()
{
	CConfigFile *config = new CConfigFile(',');
	config->clear();
	CTimerEventMap::iterator pos = events.begin();
	for(;pos != events.end();pos++)
	{
		CTimerEvent *event = pos->second;
		event->saveToConfig(config);
	}
	config->saveConfig(CONFIGFILE);
	delete config;

}
//------------------------------------------------------------
bool CTimerManager::shutdown()
{

	time_t nextAnnounceTime=0;
	CTimerEventMap::iterator pos = events.begin();
	for(;pos != events.end();pos++)
	{
		CTimerEvent *event = pos->second;
		if((event->eventType == CTimerd::TIMER_RECORD ||
			 event->eventType == CTimerd::TIMER_ZAPTO ) &&
			event->eventState == CTimerd::TIMERSTATE_SCHEDULED)
		{
			// Wir wachen nur für Records und Zaptos wieder auf
			if(event->announceTime < nextAnnounceTime || nextAnnounceTime==0)
			{
				nextAnnounceTime=event->announceTime;
			}
		}
	}
	int erg;

	if(nextAnnounceTime!=0)
	{
		int minutes=((nextAnnounceTime-time(NULL))/60)-3; //Wakeup 3 min befor next announce
		if(minutes<1)
			minutes=1; //1 minute is minimum

		int fd = open("/dev/dbox/fp0", O_RDWR);
		if((erg=ioctl(fd, FP_IOCTL_SET_WAKEUP_TIMER, &minutes))<0)
		{
			if(erg==-1)	// Wakeup not supported
			{
				dprintf("Wakeup not supported (%d min.)\n",minutes);
			}
			else
			{
				dprintf("Error setting wakeup (%d)\n",erg);
			}
			return false;
		}
		else
		{
			dprintf("wakeup in %d min. programmed\n",minutes);
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------
//=============================================================
// event functions
//=============================================================
//------------------------------------------------------------
CTimerEvent::CTimerEvent( CTimerd::CTimerEventTypes evtype, time_t announcetime, time_t alarmtime, time_t stoptime, CTimerd::CTimerEventRepeat evrepeat)
{
	eventRepeat = evrepeat;
	eventState = CTimerd::TIMERSTATE_SCHEDULED; 
	eventType = evtype;
	announceTime = announcetime;
	alarmTime = alarmtime;
	stopTime = stoptime;
}

//------------------------------------------------------------
CTimerEvent::CTimerEvent( CTimerd::CTimerEventTypes evtype, int mon, int day, int hour, int min, CTimerd::CTimerEventRepeat evrepeat)
{ 

	time_t mtime = time(NULL);
	struct tm *tmtime = localtime(&mtime);

	if(mon > 0)
		tmtime->tm_mon = mon -1;
	if(day > 0)
		tmtime->tm_mday = day;
	tmtime->tm_hour = hour;
	tmtime->tm_min = min;

	CTimerEvent(evtype, (time_t) 0, mktime(tmtime), (time_t)0, evrepeat);
}
//------------------------------------------------------------
CTimerEvent::CTimerEvent(CTimerd::CTimerEventTypes evtype,CConfigFile *config, int iId)
{
	stringstream ostr;
	ostr << iId;
	string id=ostr.str();
	time_t announcetime=config->getInt32("ANNOUNCE_TIME_"+id);
	time_t alarmtime=config->getInt32("ALARM_TIME_"+id);
	time_t stoptime=config->getInt32("STOP_TIME_"+id);
	CTimerd::CTimerEventRepeat evrepeat=(CTimerd::CTimerEventRepeat)config->getInt32("EVENT_REPEAT_"+id);
	eventRepeat = evrepeat;
	eventState = CTimerd::TIMERSTATE_SCHEDULED; 
	eventType = evtype;
	announceTime = announcetime;
	alarmTime = alarmtime;
	stopTime = stoptime;
	eventState = (CTimerd::CTimerEventStates ) config->getInt32 ("EVENT_STATE_"+id);
	previousState = (CTimerd::CTimerEventStates) config->getInt32("PREVIOUS_STATE_"+id);
}
//------------------------------------------------------------
void CTimerEvent::Reschedule()
{
	int diff = 0;
	int TAG = 60 * 60 * 24;	// sek * min * std
	printf("Reschedule\n");
	switch(eventRepeat)
	{
		case CTimerd::TIMERREPEAT_ONCE :
			break;
		case CTimerd::TIMERREPEAT_DAILY: 
			diff = TAG;
			break;
		case CTimerd::TIMERREPEAT_WEEKLY: 
			diff = TAG * 7;
			break;
		case CTimerd::TIMERREPEAT_BIWEEKLY: 
			diff = TAG * 14;
			break;
		case CTimerd::TIMERREPEAT_FOURWEEKLY: 
			diff = TAG * 28;
			break;
		case CTimerd::TIMERREPEAT_MONTHLY: 
			// hehe, son mist, todo
			diff = TAG * 28;
			break;
		case CTimerd::TIMERREPEAT_BYEVENTDESCRIPTION :
			// todo !!
			break;
		default:
			dprintf("unknown repeat type %d\n",eventRepeat);
	}
	if(diff != 0)
	{
		if(announceTime > 0)
			announceTime += diff;
		if(alarmTime > 0)
			alarmTime += diff;
		if(stopTime > 0)
			stopTime += diff;
		eventState = CTimerd::TIMERSTATE_SCHEDULED;
		dprintf("event %d rescheduled\n",eventID);
	}
	else
	{
		eventState = CTimerd::TIMERSTATE_TERMINATED;
		dprintf("event %d not rescheduled, event will be terminated\n",eventID);
	}
}


//------------------------------------------------------------
void CTimerEvent::printEvent(void)
{
	struct tm *alarmtime, *announcetime;
	dprintf("eventID: %03d type: %d state: %d repeat: %d ",eventID,eventType,eventState,eventRepeat);
	announcetime = localtime(&announceTime);
	dprintf("announce: %u %02d.%02d. %02d:%02d:%02d ",(uint) announceTime,announcetime->tm_mday,announcetime->tm_mon+1,announcetime->tm_hour,announcetime->tm_min,announcetime->tm_sec);
	alarmtime = localtime(&alarmTime);
	dprintf("alarm: %u %02d.%02d. %02d:%02d:%02d ",(uint) alarmTime,alarmtime->tm_mday,alarmtime->tm_mon+1,alarmtime->tm_hour,alarmtime->tm_min,alarmtime->tm_sec);
	switch(eventType)
	{
		case CTimerd::TIMER_ZAPTO :
			dprintf("Zapto: %x epg: %llx\n",static_cast<CTimerEvent_NextProgram*>(this)->eventInfo.channel_id,static_cast<CTimerEvent_NextProgram*>(this)->eventInfo.epgID);
			break;

		case CTimerd::TIMER_RECORD :
			dprintf("Record: %x epg: %llx apid: 0x%04x\n",static_cast<CTimerEvent_NextProgram*>(this)->eventInfo.channel_id,static_cast<CTimerEvent_NextProgram*>(this)->eventInfo.epgID,
					  static_cast<CTimerEvent_NextProgram*>(this)->eventInfo.apid);
			break;

		case CTimerd::TIMER_STANDBY :
			dprintf("standby: %s\n",(static_cast<CTimerEvent_Standby*>(this)->standby_on == 1)?"on":"off");
			break;

		default:
			dprintf("(no extra data)\n");
	}
}
//------------------------------------------------------------
void CTimerEvent::saveToConfig(CConfigFile *config)
{
	vector<int> allIDs;
	allIDs.clear();
	if(config->getString ("IDS")!="")
	{
		// sonst bekomemn wir den bloeden 0er
		allIDs=config->getInt32Vector("IDS");
	}

	allIDs.push_back(eventID);
	//SetInt-Vector haengt komischerweise nur an, deswegen erst loeschen
	config->setString("IDS","");
	config->setInt32Vector ("IDS",allIDs);

	stringstream ostr;
	ostr << eventID;
	string id=ostr.str();
	config->setInt32("EVENT_TYPE_"+id, eventType);
	config->setInt32("EVENT_STATE_"+id, eventState);
	config->setInt32("PREVIOUS_STATE_"+id, previousState);
	config->setInt32("EVENT_REPEAT_"+id, eventRepeat);
	config->setInt32("ANNOUNCE_TIME_"+id, announceTime);
	config->setInt32("ALARM_TIME_"+id, alarmTime);
	config->setInt32("STOP_TIME_"+id, stopTime);

}

//=============================================================
// Shutdown Event
//=============================================================
void CTimerEvent_Shutdown::announceEvent()
{
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_ANNOUNCE_SHUTDOWN,
																				CEventServer::INITID_TIMERD);
}
//------------------------------------------------------------
void CTimerEvent_Shutdown::stopEvent(){}

//------------------------------------------------------------
void CTimerEvent_Shutdown::fireEvent()
{
	dprintf("Shutdown Timer fired\n");
	//event in neutrinos remoteq. schreiben
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_SHUTDOWN,
																				CEventServer::INITID_TIMERD);
}

//------------------------------------------------------------
void CTimerEvent_Shutdown::saveToConfig(CConfigFile *config)
{
	CTimerEvent::saveToConfig(config);
}

//=============================================================
// Sleeptimer Event
//=============================================================
void CTimerEvent_Sleeptimer::announceEvent()
{
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_ANNOUNCE_SLEEPTIMER,
																				CEventServer::INITID_TIMERD);
}
//------------------------------------------------------------
void CTimerEvent_Sleeptimer::stopEvent(){}

//------------------------------------------------------------
void CTimerEvent_Sleeptimer::fireEvent()
{
	dprintf("Sleeptimer Timer fired\n");
	//event in neutrinos remoteq. schreiben
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_SLEEPTIMER,
																				CEventServer::INITID_TIMERD);
}

//------------------------------------------------------------
void CTimerEvent_Sleeptimer::saveToConfig(CConfigFile *config)
{
	CTimerEvent::saveToConfig(config);
}
//=============================================================
// Standby Event
//=============================================================
CTimerEvent_Standby::CTimerEvent_Standby( time_t announceTime, time_t alarmTime, 
														bool sb_on, 
														CTimerd::CTimerEventRepeat evrepeat): 
CTimerEvent(CTimerd::TIMER_STANDBY, announceTime, alarmTime, (time_t) 0, evrepeat)
{
	standby_on = sb_on;
}
CTimerEvent_Standby::CTimerEvent_Standby(CConfigFile *config, int iId):
CTimerEvent(CTimerd::TIMER_STANDBY, config, iId)
{
	stringstream ostr;
	ostr << iId;
	string id=ostr.str();
	standby_on = config->getBool("STANDBY_ON_"+id);
}
//------------------------------------------------------------

void CTimerEvent_Standby::announceEvent(){}
//------------------------------------------------------------
void CTimerEvent_Standby::stopEvent(){}
//------------------------------------------------------------

void CTimerEvent_Standby::fireEvent()
{
	dprintf("Standby Timer fired: %s\n",standby_on?"on":"off");
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				(standby_on)?CTimerdClient::EVT_STANDBY_ON:CTimerdClient::EVT_STANDBY_OFF,
																				CEventServer::INITID_TIMERD);
}

//------------------------------------------------------------
void CTimerEvent_Standby::saveToConfig(CConfigFile *config)
{
	CTimerEvent::saveToConfig(config);
	stringstream ostr;
	ostr << eventID;
	string id=ostr.str();
	config->setBool("STANDBY_ON_"+id,standby_on);
}
//=============================================================
// Record Event
//=============================================================
CTimerEvent_Record::CTimerEvent_Record( time_t announceTime, time_t alarmTime, time_t stopTime, 
													 t_channel_id channel_id, unsigned long long epgID, uint apid,
													 CTimerd::CTimerEventRepeat evrepeat) :
CTimerEvent(CTimerd::TIMER_RECORD, announceTime, alarmTime, stopTime, evrepeat)
{
	eventInfo.epgID = epgID;
	eventInfo.channel_id = channel_id;
	eventInfo.apid = apid;
}
//------------------------------------------------------------
CTimerEvent_Record::CTimerEvent_Record(CConfigFile *config, int iId):
CTimerEvent(CTimerd::TIMER_RECORD, config, iId)
{
	stringstream ostr;
	ostr << iId;
	string id=ostr.str();
	eventInfo.epgID = config->getInt64("EVENT_INFO_EPG_ID_"+id);
	eventInfo.channel_id = config->getInt32("EVENT_INFO_ONID_SID_"+id);
	eventInfo.apid = config->getInt32("EVENT_INFO_APID_"+id);
}
//------------------------------------------------------------
void CTimerEvent_Record::announceEvent()
{
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_ANNOUNCE_RECORD,
																				CEventServer::INITID_TIMERD,
																				&eventInfo, sizeof(CTimerd::EventInfo));
	dprintf("Record announcement\n"); 
}
//------------------------------------------------------------
void CTimerEvent_Record::stopEvent()
{
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_RECORD_STOP,
																				CEventServer::INITID_TIMERD);
	dprintf("Recording stopped\n"); 
}
//------------------------------------------------------------

void CTimerEvent_Record::fireEvent()
{
	// Set EPG-ID if not set
	if(eventInfo.epgID == 0)
	{
		dprintf("EPG-ID not set, trying now\n");
		CSectionsdClient* sdc = new CSectionsdClient();
		CEPGData e;
		sdc->getActualEPGServiceKey(eventInfo.channel_id, &e );
		dprintf("EPG-ID found %llu:%d(%s)\n",e.eventID,(int)e.epg_times.startzeit,e.title.c_str());     
		eventInfo.epgID = e.eventID ;
	}
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_RECORD_START,
																				CEventServer::INITID_TIMERD,
																				&eventInfo, sizeof(CTimerd::EventInfo));
	dprintf("Record Timer fired\n"); 
}

//------------------------------------------------------------
void CTimerEvent_Record::saveToConfig(CConfigFile *config)
{
	CTimerEvent::saveToConfig(config);
	stringstream ostr;
	ostr << eventID;
	string id=ostr.str();
	config->setInt64("EVENT_INFO_EPG_ID_"+id,eventInfo.epgID);
	config->setInt32("EVENT_INFO_ONID_SID_"+id,eventInfo.channel_id);
	config->setInt32("EVENT_INFO_APID_"+id,eventInfo.apid);
}
//------------------------------------------------------------
void CTimerEvent_Record::Reschedule()
{
	// clear eogId on reschedule
	eventInfo.epgID = 0;
	CTimerEvent::Reschedule();
}
//=============================================================
// Zapto Event
//=============================================================
CTimerEvent_Zapto::CTimerEvent_Zapto( time_t announceTime, time_t alarmTime, 
												  t_channel_id channel_id, unsigned long long epgID, 
												  CTimerd::CTimerEventRepeat evrepeat) :
CTimerEvent(CTimerd::TIMER_ZAPTO, announceTime, alarmTime, (time_t) 0, evrepeat)
{
	eventInfo.epgID = epgID;
	eventInfo.channel_id = channel_id;
}
//------------------------------------------------------------
CTimerEvent_Zapto::CTimerEvent_Zapto(CConfigFile *config, int iId):
CTimerEvent(CTimerd::TIMER_ZAPTO, config, iId)
{
	stringstream ostr;
	ostr << iId;
	string id=ostr.str();
	eventInfo.epgID = config->getInt64("EVENT_INFO_EPG_ID_"+id);
	eventInfo.channel_id = config->getInt32("EVENT_INFO_ONID_SID_"+id);
}
//------------------------------------------------------------
void CTimerEvent_Zapto::announceEvent()
{
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_ANNOUNCE_ZAPTO,
																				CEventServer::INITID_TIMERD);
}
//------------------------------------------------------------
void CTimerEvent_Zapto::stopEvent(){}
//------------------------------------------------------------

void CTimerEvent_Zapto::fireEvent()
{
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_ZAPTO,
																				CEventServer::INITID_TIMERD,
																				&eventInfo, sizeof(CTimerd::EventInfo));
}


//------------------------------------------------------------
void CTimerEvent_Zapto::saveToConfig(CConfigFile *config)
{
	CTimerEvent::saveToConfig(config);
	stringstream ostr;
	ostr << eventID;
	string id=ostr.str();
	config->setInt64("EVENT_INFO_EPG_ID_"+id,eventInfo.epgID);
	config->setInt32("EVENT_INFO_ONID_SID_"+id,eventInfo.channel_id);
}
//------------------------------------------------------------
void CTimerEvent_Zapto::Reschedule()
{
	// clear eogId on reschedule
	eventInfo.epgID = 0;
	CTimerEvent::Reschedule();
}
//=============================================================
// NextProgram Event
//=============================================================
CTimerEvent_NextProgram::CTimerEvent_NextProgram( time_t announceTime, time_t alarmTime, time_t stopTime, 
																  t_channel_id channel_id, unsigned long long epgID, 
																  CTimerd::CTimerEventRepeat evrepeat) :
CTimerEvent(CTimerd::TIMER_NEXTPROGRAM, announceTime, alarmTime, stopTime, evrepeat)
{
	eventInfo.epgID = epgID;
	eventInfo.channel_id = channel_id;
}
//------------------------------------------------------------
CTimerEvent_NextProgram::CTimerEvent_NextProgram(CConfigFile *config, int iId):
CTimerEvent(CTimerd::TIMER_NEXTPROGRAM, config, iId)
{
	stringstream ostr;
	ostr << iId;
	string id=ostr.str();
	eventInfo.epgID = config->getInt64("EVENT_INFO_EPG_ID_"+id);
	eventInfo.channel_id = config->getInt32("EVENT_INFO_ONID_SID_"+id);
}
//------------------------------------------------------------

void CTimerEvent_NextProgram::announceEvent()
{
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_ANNOUNCE_NEXTPROGRAM,
																				CEventServer::INITID_TIMERD,
																				&eventInfo,
																				sizeof(eventInfo));
}
//------------------------------------------------------------
void CTimerEvent_NextProgram::stopEvent(){}
//------------------------------------------------------------

void CTimerEvent_NextProgram::fireEvent()
{
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_NEXTPROGRAM,
																				CEventServer::INITID_TIMERD,
																				&eventInfo,
																				sizeof(eventInfo));
}

//------------------------------------------------------------
void CTimerEvent_NextProgram::saveToConfig(CConfigFile *config)
{
	CTimerEvent::saveToConfig(config);
	stringstream ostr;
	ostr << eventID;
	string id=ostr.str();
	config->setInt64("EVENT_INFO_EPG_ID_"+id,eventInfo.epgID);
	config->setInt32("EVENT_INFO_ONID_SID_"+id,eventInfo.channel_id);
}
//------------------------------------------------------------
void CTimerEvent_NextProgram::Reschedule()
{
	// clear eogId on reschedule
	eventInfo.epgID = 0;
	CTimerEvent::Reschedule();
}
//=============================================================
// Remind Event
//=============================================================
CTimerEvent_Remind::CTimerEvent_Remind( time_t announceTime, time_t alarmTime, 
													 char* msg, CTimerd::CTimerEventRepeat evrepeat) :
CTimerEvent(CTimerd::TIMER_REMIND, announceTime, alarmTime, (time_t) 0, evrepeat)
{
	memset(message, 0, sizeof(message));
	strncpy(message, msg, sizeof(message)-1);
}
//------------------------------------------------------------
CTimerEvent_Remind::CTimerEvent_Remind(CConfigFile *config, int iId):
CTimerEvent(CTimerd::TIMER_REMIND, config, iId)
{
	stringstream ostr;
	ostr << iId;
	string id=ostr.str();
	strcpy(message, config->getString("MESSAGE_"+id).c_str());
}
//------------------------------------------------------------
void CTimerEvent_Remind::announceEvent(){}
//------------------------------------------------------------
void CTimerEvent_Remind::stopEvent(){}
//------------------------------------------------------------

void CTimerEvent_Remind::fireEvent()
{
	CTimerManager::getInstance()->getEventServer()->sendEvent(
																				CTimerdClient::EVT_REMIND,
																				CEventServer::INITID_TIMERD,
																				message,REMINDER_MESSAGE_MAXLEN);
}

//------------------------------------------------------------
void CTimerEvent_Remind::saveToConfig(CConfigFile *config)
{
	CTimerEvent::saveToConfig(config);
	stringstream ostr;
	ostr << eventID;
	string id=ostr.str();
	config->setString("MESSAGE_"+id,message);
}
//=============================================================
