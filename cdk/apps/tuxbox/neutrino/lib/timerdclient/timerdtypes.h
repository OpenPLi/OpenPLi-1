/*
	Timer-Daemon  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	$Id: timerdtypes.h,v 1.3.2.1 2002/10/24 20:33:23 thegoodguy Exp $

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


#ifndef __timerdtypes__
#define __timerdtypes__

#include <vector>

#include <zapit/client/zapittypes.h>


#define REMINDER_MESSAGE_MAXLEN 31
#define TIMERD_UDS_NAME "/tmp/timerd.sock"

class CTimerd
{
	public:
		enum CTimerEventRepeat 
		{ 
			TIMERREPEAT_ONCE = 0,
			TIMERREPEAT_DAILY, 
			TIMERREPEAT_WEEKLY, 
			TIMERREPEAT_BIWEEKLY, 
			TIMERREPEAT_FOURWEEKLY, 
			TIMERREPEAT_MONTHLY, 
			TIMERREPEAT_BYEVENTDESCRIPTION
		};

		enum CTimerEventTypes
		{
			TIMER_SHUTDOWN = 1,
			TIMER_NEXTPROGRAM,
			TIMER_ZAPTO,
			TIMER_STANDBY,
			TIMER_RECORD,
			TIMER_REMIND,
			TIMER_SLEEPTIMER
		};
		
		enum CTimerEventStates 
		{ 
			TIMERSTATE_SCHEDULED, 
			TIMERSTATE_PREANNOUNCE, 
			TIMERSTATE_ISRUNNING, 
			TIMERSTATE_HASFINISHED, 
			TIMERSTATE_TERMINATED 
		};

		static const char ACTVERSION = 1;

		enum commands
		{
			CMD_ADDTIMER = 1,
			CMD_REMOVETIMER,
			CMD_GETTIMER,
			CMD_GETTIMERLIST,
			CMD_MODIFYTIMER,
			CMD_GETSLEEPTIMER,
			CMD_RESCHEDULETIMER,

			CMD_REGISTEREVENT,
			CMD_UNREGISTEREVENT,
			CMD_TIMERDAVAILABLE,
			CMD_SHUTDOWN,
			CMD_SETAPID
		};

		struct commandAddTimer
		{
			CTimerEventTypes	eventType;
			CTimerEventRepeat eventRepeat;
			time_t							alarmTime;
			time_t							announceTime;
			time_t							stopTime;			
		};


		struct responseAddTimer
		{
			int   eventID;
		};

		struct commandRemoveTimer
		{
			int   eventID;
		};

		struct commandSetAPid
		{
			int   eventID;
			uint  apid;
		};

		struct responseAvailable
		{
			bool available;
		};
		
		struct commandGetTimer
		{
			int   eventID;
		};

		struct responseGetSleeptimer
		{
			int   eventID;
		};

		struct commandSetStandby
		{
			bool standby_on;
		};

		struct commandModifyTimer
		{
			int			eventID;
			time_t		announceTime;
			time_t		alarmTime;
			time_t		stopTime;
			CTimerEventRepeat	eventRepeat;
		};

		struct commandRemind
		{
			char message[REMINDER_MESSAGE_MAXLEN];
		};

		struct EventInfo
		{
			unsigned long long epgID;
			t_channel_id       channel_id;
			uint               apid;
		};

		struct responseGetTimer
		{		
			int								eventID;
			CTimerEventTypes	eventType;
			CTimerEventStates	eventState;
			CTimerEventRepeat	eventRepeat;
			time_t							alarmTime;
			time_t							announceTime;
			time_t							stopTime;
			t_channel_id channel_id; //only filled if applicable
			unsigned long long epgID; //only filled if applicable
			uint apid; //only filled if applicable
			bool standby_on; //only filled if applicable
			char message[REMINDER_MESSAGE_MAXLEN]; //only filled if applicable
			bool operator< (const responseGetTimer& a) const
			{
				return this->alarmTime < a.alarmTime ;
			}
		};
		
		typedef std::vector<responseGetTimer> TimerList;

		struct responseStatus
		{
			bool status;
		};

	
};
#endif
