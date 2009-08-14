/*
	Timer Daemon  -   DBoxII-Project

	Copyright (C) 2002 Dirk Szymanski 'Dirch'
	
	$Id: timerdclient.cpp,v 1.32.2.1 2002/10/24 20:33:23 thegoodguy Exp $

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

#include <stdio.h>

#include <eventserver.h>
#include <timerdclient/timerdclient.h>
#include <zapit/client/basicmessage.h>


bool CTimerdClient::send(const unsigned char command, char* data = NULL, const unsigned int size = 0)
{
	CBasicMessage::Header msgHead;
	msgHead.version = CTimerd::ACTVERSION;
	msgHead.cmd     = command;

	open_connection(TIMERD_UDS_NAME);

	if(!send_data((char*)&msgHead, sizeof(msgHead)))
		return false;

	if(size != 0)
		return send_data(data, size);

	return true;
}

//-------------------------------------------------------------------------

void CTimerdClient::registerEvent(unsigned int eventID, unsigned int clientID, std::string udsName)
{
	CEventServer::commandRegisterEvent msg2;

	msg2.eventID = eventID;
	msg2.clientID = clientID;

	strcpy(msg2.udsName, udsName.c_str());

	send(CTimerd::CMD_REGISTEREVENT, (char*)&msg2, sizeof(msg2));

	close_connection();
}
//-------------------------------------------------------------------------

void CTimerdClient::unRegisterEvent(unsigned int eventID, unsigned int clientID)
{
	CEventServer::commandUnRegisterEvent msg2;

	msg2.eventID = eventID;
	msg2.clientID = clientID;

	send(CTimerd::CMD_UNREGISTEREVENT, (char*)&msg2, sizeof(msg2));

	close_connection();
}
//-------------------------------------------------------------------------

int CTimerdClient::setSleeptimer(time_t announcetime, time_t alarmtime, int timerid)
{
	int timerID;

	if(timerid == 0)
		timerID = getSleeptimerID();
	else
		timerID = timerid;

	if(timerID != 0)
	{
		modifyTimerEvent(timerID, announcetime, alarmtime, 0);
	}
	else
	{
		timerID = addTimerEvent(CTimerd::TIMER_SLEEPTIMER,NULL,announcetime,alarmtime,0);
	}

	return timerID;   
}
//-------------------------------------------------------------------------

int CTimerdClient::getSleeptimerID()
{
	send(CTimerd::CMD_GETSLEEPTIMER);
	CTimerd::responseGetSleeptimer response;
	if(!receive_data((char*)&response, sizeof(CTimerd::responseGetSleeptimer)))
		response.eventID =0;
	close_connection();  
	return response.eventID;
}
//-------------------------------------------------------------------------

int CTimerdClient::getSleepTimerRemaining()
{
	int timerID;
	if((timerID = getSleeptimerID()) != 0)
	{
		CTimerd::responseGetTimer timer;
		getTimer( timer, timerID);
		int min=(((timer.alarmTime + 1 - time(NULL)) / 60)+1); //aufrunden auf nächst größerere Min.
		if(min <1)
			min=1;
		return min;
	}
	else
		return 0;
}
//-------------------------------------------------------------------------

void CTimerdClient::getTimerList( CTimerd::TimerList &timerlist)
{
	send(CTimerd::CMD_GETTIMERLIST);
	timerlist.clear();
	CTimerd::responseGetTimer response;
	while( receive_data((char*)&response, sizeof(CTimerd::responseGetTimer)))
	{
		if(response.eventState != CTimerd::TIMERSTATE_TERMINATED)
			timerlist.insert( timerlist.end(), response);
	}
	close_connection();
}
//-------------------------------------------------------------------------

void CTimerdClient::getTimer( CTimerd::responseGetTimer &timer, unsigned timerID)
{
	send(CTimerd::CMD_GETTIMER, (char*)&timerID, sizeof(timerID));

	CTimerd::responseGetTimer response;
	receive_data((char*)&response, sizeof(CTimerd::responseGetTimer));
	timer = response;
	close_connection();
}
//-------------------------------------------------------------------------


bool CTimerdClient::modifyTimerEvent(int eventid, time_t announcetime, time_t alarmtime, time_t stoptime, CTimerd::CTimerEventRepeat evrepeat)
{
	// set new time values for event eventid

	CTimerd::commandModifyTimer msgModifyTimer;
	msgModifyTimer.eventID = eventid;
	msgModifyTimer.announceTime = announcetime;
	msgModifyTimer.alarmTime = alarmtime;
	msgModifyTimer.stopTime = stoptime;
	msgModifyTimer.eventRepeat = evrepeat;

	send(CTimerd::CMD_MODIFYTIMER, (char*) &msgModifyTimer, sizeof(msgModifyTimer));

	CTimerd::responseStatus response;
	receive_data((char*)&response, sizeof(response));

	close_connection();
	return true;
}
//-------------------------------------------------------------------------

bool CTimerdClient::rescheduleTimerEvent(int eventid, time_t diff)
{
	rescheduleTimerEvent(eventid,diff,diff,diff);
	return true;
}
//-------------------------------------------------------------------------

bool CTimerdClient::rescheduleTimerEvent(int eventid, time_t announcediff, time_t alarmdiff, time_t stopdiff)
{
	CTimerd::commandModifyTimer msgModifyTimer;
	msgModifyTimer.eventID = eventid;
	msgModifyTimer.announceTime = announcediff;
	msgModifyTimer.alarmTime = alarmdiff;
	msgModifyTimer.stopTime = stopdiff;

	send(CTimerd::CMD_RESCHEDULETIMER, (char*) &msgModifyTimer, sizeof(msgModifyTimer));

	CTimerd::responseStatus response;
	receive_data((char*)&response, sizeof(response));

	close_connection();
	return response.status;
}
//-------------------------------------------------------------------------

/*
int CTimerdClient::addTimerEvent( CTimerEventTypes evType, void* data , int min, int hour, int day, int month, CTimerd::CTimerEventRepeat evrepeat)
{
	time_t actTime_t;
	time(&actTime_t);
	struct tm* actTime = localtime(&actTime_t);

	actTime->tm_min = min;
	actTime->tm_hour = hour;

	if (day > 0)
		actTime->tm_mday = day;
	if (month > 0)
		actTime->tm_mon = month -1; 
	
	addTimerEvent(evType,true,data,0,mktime(actTime),0);
}
*/
//-------------------------------------------------------------------------
int CTimerdClient::addTimerEvent( CTimerd::CTimerEventTypes evType, void* data, time_t announcetime, time_t alarmtime,time_t stoptime, CTimerd::CTimerEventRepeat evrepeat)
{

	CTimerd::commandAddTimer msgAddTimer;
	msgAddTimer.alarmTime  = alarmtime;
	msgAddTimer.announceTime = announcetime;
	msgAddTimer.stopTime   = stoptime;
	msgAddTimer.eventType = evType ;
	msgAddTimer.eventRepeat = evrepeat;

	int length;
	if( evType == CTimerd::TIMER_SHUTDOWN || evType == CTimerd::TIMER_SLEEPTIMER )
	{
		length = 0;
	}
	else if(evType == CTimerd::TIMER_NEXTPROGRAM || evType == CTimerd::TIMER_ZAPTO || evType == CTimerd::TIMER_RECORD )
	{
		length = sizeof( CTimerd::EventInfo);
	}
	else if(evType == CTimerd::TIMER_STANDBY)
	{
		length = sizeof(CTimerd::commandSetStandby);
	}
	else if(evType == CTimerd::TIMER_REMIND)
	{
		length = sizeof(CTimerd::commandRemind);
	}
	else
	{
		length = 0;
	}

	send(CTimerd::CMD_ADDTIMER, (char*)&msgAddTimer, sizeof(msgAddTimer));
	if((data != NULL) && (length > 0))
		send_data((char*)data, length);

	CTimerd::responseAddTimer response;
	receive_data((char*)&response, sizeof(response));
	close_connection();

	return( response.eventID);
}
//-------------------------------------------------------------------------

void CTimerdClient::removeTimerEvent( int evId)
{
	CTimerd::commandRemoveTimer msgRemoveTimer;

	msgRemoveTimer.eventID  = evId;

	send(CTimerd::CMD_REMOVETIMER, (char*) &msgRemoveTimer, sizeof(msgRemoveTimer));

	close_connection();  
}
//-------------------------------------------------------------------------

bool CTimerdClient::isTimerdAvailable()
{
	if(!send(CTimerd::CMD_TIMERDAVAILABLE))
		return false;

	CTimerd::responseAvailable response;
	bool ret=receive_data((char*)&response, sizeof(response));
	close_connection();
	return ret;
}
//-------------------------------------------------------------------------
bool CTimerdClient::shutdown()
{
	send(CTimerd::CMD_SHUTDOWN);

	CTimerd::responseStatus response;
	receive_data((char*)&response, sizeof(response));

	close_connection();
	return response.status;
}
//-------------------------------------------------------------------------
void CTimerdClient::modifyTimerAPid(int eventid, uint apid)
{
	CTimerd::commandSetAPid data;
	data.eventID=eventid;
	data.apid=apid;
	send(CTimerd::CMD_SETAPID, (char*) &data, sizeof(data)); 
	close_connection();
}

//-------------------------------------------------------------------------

