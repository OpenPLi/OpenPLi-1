/*
	Timer-Daemon  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	$Id: timermanager.h,v 1.26 2002/10/15 20:39:48 woglinde Exp $

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

#ifndef __neutrino_timermanager__
#define __neutrino_timermanager__

#include <stdio.h>
#include <map>

#include <configfile.h>
#include <config.h>

#include <eventserver.h>
#include <timerdclient/timerdtypes.h>

#define CONFIGFILE CONFIGDIR "/timerd.conf"

using namespace std;

class CTimerEvent
{
	public:
		
		int					eventID;			// event identifier
		CTimerd::CTimerEventTypes	eventType;			// Type of event
		CTimerd::CTimerEventStates	eventState;			// actual event state
		CTimerd::CTimerEventStates	previousState;			// previous event state
		CTimerd::CTimerEventRepeat	eventRepeat;

	// time values
		time_t		alarmTime;					// event start time
		time_t		stopTime;					// 0 = one time shot
		time_t		announceTime;				// when should event be announced (0=none)

//		CTimerEvent();
		CTimerEvent( CTimerd::CTimerEventTypes evtype, int mon = 0, int day = 0, int hour = 0, int min = 0, CTimerd::CTimerEventRepeat evrepeat = CTimerd::TIMERREPEAT_ONCE);
		CTimerEvent( CTimerd::CTimerEventTypes evtype, time_t announcetime, time_t alarmtime, time_t stoptime, CTimerd::CTimerEventRepeat evrepeat = CTimerd::TIMERREPEAT_ONCE);
                CTimerEvent( CTimerd::CTimerEventTypes evtype, CConfigFile *config, int iId);
		
		void setState(CTimerd::CTimerEventStates newstate){previousState = eventState; eventState = newstate;};

		int remain_min(time_t t){return (t - time(NULL)) / 60;};
		void printEvent(void);
		virtual void Reschedule();

		virtual void fireEvent(){};
		virtual void stopEvent(){};
		virtual void announceEvent(){};
                virtual void saveToConfig(CConfigFile *config);
};

typedef map<int, CTimerEvent*> CTimerEventMap;

class CTimerEvent_Shutdown : public CTimerEvent
{
	public:
		CTimerEvent_Shutdown( time_t announceTime, time_t alarmTime, CTimerd::CTimerEventRepeat evrepeat = CTimerd::TIMERREPEAT_ONCE) :
                   CTimerEvent(CTimerd::TIMER_SHUTDOWN, announceTime, alarmTime, (time_t) 0, evrepeat ){};
                CTimerEvent_Shutdown(CConfigFile *config, int iId):
                   CTimerEvent(CTimerd::TIMER_SHUTDOWN, config, iId){};
		virtual void fireEvent();
		virtual void announceEvent();
		virtual void stopEvent();
                virtual void saveToConfig(CConfigFile *config);
};

class CTimerEvent_Sleeptimer : public CTimerEvent
{
	public:
		CTimerEvent_Sleeptimer( time_t announceTime, time_t alarmTime, CTimerd::CTimerEventRepeat evrepeat = CTimerd::TIMERREPEAT_ONCE) :
                   CTimerEvent(CTimerd::TIMER_SLEEPTIMER, announceTime, alarmTime, (time_t) 0,evrepeat ){};
                CTimerEvent_Sleeptimer(CConfigFile *config, int iId):
                   CTimerEvent(CTimerd::TIMER_SLEEPTIMER, config, iId){};
		virtual void fireEvent();
		virtual void announceEvent();
		virtual void stopEvent();
                virtual void saveToConfig(CConfigFile *config);
};


class CTimerEvent_Standby : public CTimerEvent
{
	public:
		bool standby_on;

		CTimerEvent_Standby( time_t announceTime, time_t alarmTime, 
									bool sb_on, CTimerd::CTimerEventRepeat evrepeat = CTimerd::TIMERREPEAT_ONCE);
		CTimerEvent_Standby(CConfigFile *config, int iId);
		virtual void fireEvent();
		virtual void announceEvent();
		virtual void stopEvent();
                virtual void saveToConfig(CConfigFile *config);
};

class CTimerEvent_Record : public CTimerEvent
{
	public:
		CTimerd::EventInfo eventInfo;

		CTimerEvent_Record( time_t announceTime, time_t alarmTime, time_t stopTime, 
                          t_channel_id channel_id, unsigned long long epgID=0, 
                          uint apid = 0, CTimerd::CTimerEventRepeat evrepeat = CTimerd::TIMERREPEAT_ONCE);
		CTimerEvent_Record(CConfigFile *config, int iId);
		virtual void fireEvent();
		virtual void announceEvent();
		virtual void stopEvent();
		virtual void saveToConfig(CConfigFile *config);
		virtual void Reschedule();
};

class CTimerEvent_Zapto : public CTimerEvent
{
	public:

		CTimerd::EventInfo eventInfo;

		CTimerEvent_Zapto( time_t announceTime, time_t alarmTime, 
								 t_channel_id channel_id, unsigned long long epgID=0, 
								 CTimerd::CTimerEventRepeat evrepeat = CTimerd::TIMERREPEAT_ONCE);
		CTimerEvent_Zapto(CConfigFile *config, int iId);
		virtual void fireEvent();
		virtual void announceEvent();
		virtual void stopEvent();
		virtual void saveToConfig(CConfigFile *config);
		virtual void Reschedule();
};

class CTimerEvent_NextProgram : public CTimerEvent
{
	public:
		CTimerd::EventInfo eventInfo;

		CTimerEvent_NextProgram( time_t announceTime, time_t alarmTime, time_t stopTime, 
										 t_channel_id channel_id, unsigned long long epgID=0, 
										 CTimerd::CTimerEventRepeat evrepeat = CTimerd::TIMERREPEAT_ONCE);
		CTimerEvent_NextProgram(CConfigFile *config, int iId);
		virtual void fireEvent();
		virtual void announceEvent();
		virtual void stopEvent();
		virtual void saveToConfig(CConfigFile *config);
		virtual void Reschedule();
};

class CTimerEvent_Remind : public CTimerEvent
{
	public:
		char message[REMINDER_MESSAGE_MAXLEN];

		CTimerEvent_Remind( time_t announceTime, time_t alarmTime, 
								  char* msg, CTimerd::CTimerEventRepeat evrepeat = CTimerd::TIMERREPEAT_ONCE);
		CTimerEvent_Remind(CConfigFile *config, int iId);
		virtual void fireEvent();
		virtual void announceEvent();
		virtual void stopEvent();
		virtual void saveToConfig(CConfigFile *config);
};

class CTimerManager
{
	//singleton
	private:
		int					eventID;
		CEventServer		*eventServer;
		CTimerEventMap		events;
		pthread_t			thrTimer;

		CTimerManager();
		static void* timerThread(void *arg);
		CTimerEvent			*nextEvent();

	public:


		static CTimerManager* getInstance();

		CEventServer* getEventServer() {return eventServer;};
		int addEvent(CTimerEvent*,bool save = true);
		bool removeEvent(int eventID);
		CTimerEvent* getNextEvent();
		bool listEvents(CTimerEventMap &Events);
		int modifyEvent(int eventID, time_t announceTime, time_t alarmTime, time_t stopTime, CTimerd::CTimerEventRepeat evrepeat = CTimerd::TIMERREPEAT_ONCE);
		int modifyEvent(int eventID, uint apid);
		int rescheduleEvent(int eventID, time_t announceTime, time_t alarmTime, time_t stopTime);
		void saveEventsToConfig();
		bool shutdown();
};

#endif
