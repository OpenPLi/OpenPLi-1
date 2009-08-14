/*
	webserver  -   DBoxII-Project

	Copyright (C) 2001/2002 Dirk Szymanski 'Dirch'

	$Id: webdbox.cpp,v 1.42 2002/10/16 10:30:47 dirch Exp $

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


#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

#include <neutrinoMessages.h>

#include "webdbox.h"
#include "webserver.h"
#include "request.h"
#include "helper.h"
#include "debug.h"

#define SA struct sockaddr
#define SAI struct sockaddr_in


//-------------------------------------------------------------------------
void CWebDbox::UpdateBouquets(void)
{
	GetBouquets();
	for(unsigned int i = 1; i <= BouquetList.size();i++)
		GetBouquet(i);
	GetChannelList();
}
//-------------------------------------------------------------------------

void CWebDbox::ZapTo(string target)
{
	t_channel_id channel_id = atoi(target.c_str());
	if(channel_id == Zapit->getCurrentServiceID())
	{
		dprintf("Kanal ist aktuell\n");
		return;
	}
	int status = Zapit->zapTo_serviceID(channel_id);
	dprintf("Zapto Status: %d\n",status);
	Sectionsd->setServiceChanged(channel_id,false);

}
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Konstruktor und destruktor
//-------------------------------------------------------------------------

CWebDbox::CWebDbox(CWebserver *server)
{
	Parent=server;
//	standby_mode=false;

	Controld = new CControldClient();
	Sectionsd = new CSectionsdClient();
	Zapit = new CZapitClient();
	Timerd = new CTimerdClient();

	ControlAPI = new CControlAPI(this);
	WebAPI = new CWebAPI(this);
	BouqueteditAPI = new CBouqueteditAPI(this);

	UpdateBouquets();

	Dbox_Hersteller[1] = "Nokia";
	Dbox_Hersteller[2] = "Sagem";
	Dbox_Hersteller[3] = "Philips";
	videooutput_names[0] = "Composite";
	videooutput_names[1] = "RGB";
	videooutput_names[2] = "S-Video";
	videoformat_names[0] = "automatic";
	videoformat_names[1] = "16:9";
	videoformat_names[2] = "4:3";
	audiotype_names[1] =  "single channel";
	audiotype_names[2] = "dual channel";
	audiotype_names[3] = "joint stereo";
	audiotype_names[4] = "stereo";

	EventServer = new CEventServer;
	EventServer->registerEvent2( NeutrinoMessages::SHUTDOWN, CEventServer::INITID_HTTPD, "/tmp/neutrino.sock");
	EventServer->registerEvent2( NeutrinoMessages::STANDBY_ON, CEventServer::INITID_HTTPD, "/tmp/neutrino.sock");
	EventServer->registerEvent2( NeutrinoMessages::STANDBY_OFF, CEventServer::INITID_HTTPD, "/tmp/neutrino.sock");
	EventServer->registerEvent2( NeutrinoMessages::EVT_POPUP, CEventServer::INITID_HTTPD, "/tmp/neutrino.sock");
	EventServer->registerEvent2( NeutrinoMessages::EVT_EXTMSG, CEventServer::INITID_HTTPD, "/tmp/neutrino.sock");
	EventServer->registerEvent2( NeutrinoMessages::CHANGEMODE, CEventServer::INITID_HTTPD, "/tmp/neutrino.sock");

}
//-------------------------------------------------------------------------

CWebDbox::~CWebDbox()
{
	if(BouqueteditAPI)
		delete BouqueteditAPI;
	if(WebAPI)
		delete WebAPI;
	if(ControlAPI)
		delete ControlAPI;

	if(Controld)
		delete Controld;
	Controld = NULL;
	if(Sectionsd)
		delete Sectionsd;
	Sectionsd = NULL;
	if(Zapit)
		delete Zapit;
	Zapit = NULL;
	if(Timerd)
		delete Timerd;
	Timerd = NULL;
	if(EventServer)
		delete EventServer;
	EventServer = NULL;
}

//-------------------------------------------------------------------------
// Get functions
//-------------------------------------------------------------------------

bool CWebDbox::GetStreamInfo(int bitInfo[10])
{
	char *key,*tmpptr,buf[100];
	int value, pos=0;

	FILE* fd = fopen("/proc/bus/bitstream", "rt");
	if (fd==NULL)
	{
		aprintf("error while opening proc-bitstream\n" );
		return false;
	}

	fgets(buf,29,fd);//dummy
	while(!feof(fd))
	{
		if(fgets(buf,29,fd)!=NULL)
		{
			buf[strlen(buf)-1]=0;
			tmpptr=buf;
			key=strsep(&tmpptr,":");
			for(;tmpptr[0]==' ';tmpptr++);
			value=atoi(tmpptr);
			bitInfo[pos]= value;
			pos++;
		}
	}
	fclose(fd);

	return true;
}

//-------------------------------------------------------------------------

void CWebDbox::GetChannelEvents()
{
	eList = Sectionsd->getChannelEvents();
	CChannelEventList::iterator eventIterator;

    for( eventIterator = eList.begin(); eventIterator != eList.end(); eventIterator++ )
		ChannelListEvents[(*eventIterator).serviceID()] = &(*eventIterator);
}
//-------------------------------------------------------------------------
string CWebDbox::GetServiceName(t_channel_id channel_id)
{
	for(unsigned int i = 0; i < ChannelList.size();i++)
		if( ChannelList[i].channel_id == channel_id)
			return ChannelList[i].name;
	return "";
}

//-------------------------------------------------------------------------
bool CWebDbox::GetBouquets(void)
{
	BouquetList.clear();
	Zapit->getBouquets(BouquetList,true); 
	return true;
}

//-------------------------------------------------------------------------
bool CWebDbox::GetBouquet(unsigned int BouquetNr)
{
	BouquetsList[BouquetNr].clear();
	Zapit->getBouquetChannels(BouquetNr - 1, BouquetsList[BouquetNr]);
	return true;
}
//-------------------------------------------------------------------------

bool CWebDbox::GetChannelList(void)
{
	ChannelList.clear();
	Zapit->getChannels(ChannelList);
	return true;
}
//-------------------------------------------------------------------------

void CWebDbox::timerEventType2Str(CTimerd::CTimerEventTypes type, char *str,int len)
{
   switch(type)
   {
      case CTimerd::TIMER_SHUTDOWN : strncpy(str, "Shutdown",len);
         break;
      case CTimerd::TIMER_NEXTPROGRAM : strncpy(str, "Next Program", len);
         break;
      case CTimerd::TIMER_ZAPTO : strncpy(str, "Zap to", len);
         break;
      case CTimerd::TIMER_STANDBY : strncpy(str, "Standby", len);
         break;
      case CTimerd::TIMER_RECORD : strncpy(str, "Record", len);
         break;
      case CTimerd::TIMER_REMIND : strncpy(str, "Remind", len);
         break;
      case CTimerd::TIMER_SLEEPTIMER: strncpy(str, "Sleeptimer", len);
         break;
      default: strncpy(str, "Unknown", len);
   }
   str[len]=0;
}
//-------------------------------------------------------------------------
void CWebDbox::timerEventRepeat2Str(CTimerd::CTimerEventRepeat rep, char *str,int len)
{
   switch(rep)
   {
      case CTimerd::TIMERREPEAT_ONCE : strncpy(str, "Once",len);
         break;
      case CTimerd::TIMERREPEAT_DAILY : strncpy(str, "Daily",len);
         break;
      case CTimerd::TIMERREPEAT_WEEKLY : strncpy(str, "Weekly",len);
         break;
      case CTimerd::TIMERREPEAT_BIWEEKLY : strncpy(str, "Biweekly",len);
         break;
      case CTimerd::TIMERREPEAT_FOURWEEKLY : strncpy(str, "Fourweekly",len);
         break;
      case CTimerd::TIMERREPEAT_MONTHLY : strncpy(str, "Monthly",len);
         break;
      case CTimerd::TIMERREPEAT_BYEVENTDESCRIPTION : strncpy(str, "By event desc.",len);
         break;

      default: strncpy(str, "Unknown", len);
   }
   str[len]=0;
}
//-------------------------------------------------------------------------
