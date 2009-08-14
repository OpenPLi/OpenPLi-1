/*
  Client-Interface für zapit  -   DBoxII-Project

  $Id: sectionsdclient.cpp,v 1.26.2.2 2003/03/11 12:53:13 thegoodguy Exp $

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

#include <sectionsdclient/sectionsdclient.h>
#include <sectionsdclient/sectionsdMsg.h>


const unsigned char   CSectionsdClient::getVersion   () const
{
	return sectionsd::ACTVERSION;
}

const          char * CSectionsdClient::getSocketName() const
{
	return SECTIONSD_UDS_NAME;
}

int CSectionsdClient::readResponse(char* data, int size)
{
	struct sectionsd::msgResponseHeader responseHeader;
    receive_data((char*)&responseHeader, sizeof(responseHeader));

	if ( data != NULL )
	{
		if ( responseHeader.dataLength != size )
			return -1;
		else
			return receive_data(data, size);
	}
	else
		return responseHeader.dataLength;
}


bool CSectionsdClient::send(const unsigned char command, const char* data, const unsigned int size)
{
	sectionsd::msgRequestHeader msgHead;

	msgHead.version    = getVersion();
	msgHead.command    = command;
	msgHead.dataLength = size;

	open_connection(); // if the return value is false, the next send_data call will return false, too

        if (!send_data((char*)&msgHead, sizeof(msgHead)))
            return false;

        if (size != 0)
            return send_data(data, size);

        return true;
}

void CSectionsdClient::registerEvent(const unsigned int eventID, const unsigned int clientID, const std::string udsName)
{
	CEventServer::commandRegisterEvent msg2;

	msg2.eventID = eventID;
	msg2.clientID = clientID;

	strcpy(msg2.udsName, udsName.c_str());
	
	send(sectionsd::CMD_registerEvents, (char*)&msg2, sizeof(msg2));

	close_connection();
}

void CSectionsdClient::unRegisterEvent(const unsigned int eventID, const unsigned int clientID)
{
	CEventServer::commandUnRegisterEvent msg2;

	msg2.eventID = eventID;
	msg2.clientID = clientID;

	send(sectionsd::CMD_unregisterEvents, (char*)&msg2, sizeof(msg2));

	close_connection();
}

bool CSectionsdClient::getIsTimeSet()
{
	sectionsd::responseIsTimeSet rmsg;

	if (send(sectionsd::getIsTimeSet))
	{
		readResponse((char*)&rmsg, sizeof(rmsg));
		close_connection();

		return rmsg.IsTimeSet;
	}
	else
	{
		close_connection();
		return false;
	}
}


void CSectionsdClient::setEventsAreOldInMinutes(const unsigned short minutes)
{
	send(sectionsd::setEventsAreOldInMinutes, (char*)&minutes, sizeof(minutes));

	readResponse();
	close_connection();
}

void CSectionsdClient::setPauseSorting(const bool doPause)
{
	int PauseIt = (doPause) ? 1 : 0;

	send(sectionsd::pauseSorting, (char*)&PauseIt, sizeof(PauseIt));

	readResponse();
	close_connection();
}

void CSectionsdClient::setPauseScanning(const bool doPause)
{
	int PauseIt = (doPause) ? 1 : 0;

	send(sectionsd::pauseScanning, (char*)&PauseIt, sizeof(PauseIt));

	readResponse();
	close_connection();
}

void CSectionsdClient::setServiceChanged(const t_channel_id channel_id, const bool requestEvent)
{
	sectionsd::commandSetServiceChanged msg;

	msg.channel_id   = channel_id;
	msg.requestEvent = requestEvent; 

	send(sectionsd::serviceChanged, (char *)&msg, sizeof(msg));

	readResponse();
	close_connection();
}


bool CSectionsdClient::getComponentTagsUniqueKey(const unsigned long long uniqueKey, CSectionsdClient::ComponentTagList& tags)
{
	if (send(sectionsd::ComponentTagsUniqueKey, (char*)&uniqueKey, sizeof(uniqueKey)))
	{
		tags.clear();

		int nBufSize = readResponse();

		char* pData = new char[nBufSize];
		receive_data(pData, nBufSize);
		char* dp = pData;

		int	count= *(int *) pData;
		dp+= sizeof(int);

		CSectionsdClient::responseGetComponentTags response;
		for (int i= 0; i<count; i++)
		{
			response.component = dp;
			dp+= strlen(dp)+1;
			response.componentType = *(unsigned char *) dp;
			dp+=sizeof(unsigned char);
			response.componentTag = *(unsigned char *) dp;
			dp+=sizeof(unsigned char);
			response.streamContent = *(unsigned char *) dp;
			dp+=sizeof(unsigned char);

			tags.insert( tags.end(), response);
		}
		close_connection();

		return true;
	}
	else
	{
		close_connection();
		return false;
	}
}

bool CSectionsdClient::getLinkageDescriptorsUniqueKey(const unsigned long long uniqueKey, CSectionsdClient::LinkageDescriptorList& descriptors)
{
	if (send(sectionsd::LinkageDescriptorsUniqueKey, (char*)&uniqueKey, sizeof(uniqueKey)))
	{
		descriptors.clear();

		int nBufSize = readResponse();

		char* pData = new char[nBufSize];
		receive_data(pData, nBufSize);
		char* dp = pData;

		int	count= *(int *) pData;
		dp+= sizeof(int);

		CSectionsdClient::responseGetLinkageDescriptors response;
		for (int i= 0; i<count; i++)
		{
			response.name = dp;
			dp+= strlen(dp)+1;
			response.transportStreamId = *(unsigned short *) dp;
			dp+=sizeof(unsigned short);
			response.originalNetworkId = *(unsigned short *) dp;
			dp+=sizeof(unsigned short);
			response.serviceId = *(unsigned short *) dp;
			dp+=sizeof(unsigned short);

			descriptors.insert( descriptors.end(), response);
		}
		close_connection();
		return true;
	}
	else
	{
		close_connection();
		return false;
	}
}

bool CSectionsdClient::getNVODTimesServiceKey(const t_channel_id channel_id, CSectionsdClient::NVODTimesList& nvod_list)
{
	if (send(sectionsd::timesNVODservice, (char*)&channel_id, sizeof(channel_id)))
	{
		nvod_list.clear();

		int nBufSize = readResponse();

		char* pData = new char[nBufSize];
		receive_data(pData, nBufSize);
		char* dp = pData;

		CSectionsdClient::responseGetNVODTimes response;

		while( dp< pData+ nBufSize )
		{
			response.service_id = *(t_service_id *) dp;			dp += sizeof(t_service_id);
			response.original_network_id = *(t_original_network_id *) dp;	dp += sizeof(t_original_network_id);
			response.transport_stream_id = *(t_transport_stream_id *) dp;	dp += sizeof(t_transport_stream_id);
			response.zeit = *(CSectionsdClient::sectionsdTime*) dp;		dp += sizeof(CSectionsdClient::sectionsdTime);

			nvod_list.insert( nvod_list.end(), response);
		}
		close_connection();
		return true;
	}
	else
	{
		close_connection();
		return false;
	}
}


bool CSectionsdClient::getCurrentNextServiceKey(const t_channel_id channel_id, CSectionsdClient::responseGetCurrentNextInfoChannelID& current_next)
{
	if (send(sectionsd::currentNextInformationID, (char*)&channel_id, sizeof(channel_id)))
	{
		int nBufSize = readResponse();

		char* pData = new char[nBufSize];
		receive_data(pData, nBufSize);
		char* dp = pData;

		// current
		current_next.current_uniqueKey = *((unsigned long long *)dp);
		dp+= sizeof(unsigned long long);
		current_next.current_zeit = *(CSectionsdClient::sectionsdTime*) dp;
		dp+= sizeof(CSectionsdClient::sectionsdTime);
		current_next.current_name = dp;
		dp+=strlen(dp)+1;

		// next
		current_next.next_uniqueKey = *((unsigned long long *)dp);
		dp+= sizeof(unsigned long long);
		current_next.next_zeit = *(CSectionsdClient::sectionsdTime*) dp;
		dp+= sizeof(CSectionsdClient::sectionsdTime);
		current_next.next_name = dp;
		dp+=strlen(dp)+1;

		current_next.flags = *(unsigned*) dp;
		dp+= sizeof(unsigned);

		current_next.current_fsk = *(char*) dp;

		close_connection();
		return true;
	}
	else
	{
		current_next.flags = 0;
		close_connection();
		return false;
	}
}



CChannelEventList CSectionsdClient::getChannelEvents()
{
	CChannelEventList eList;

	if (send(sectionsd::actualEventListTVshortIDs))
	{
		int nBufSize = readResponse();

		if( nBufSize > 0)
		{
			char* pData = new char[nBufSize];
			receive_data(pData, nBufSize);

			char* dp = pData;

			while(dp < pData + nBufSize)
			{
				CChannelEvent aEvent;

				aEvent.eventID = *((unsigned long long *) dp);
				dp+=sizeof(aEvent.eventID);

				aEvent.startTime = *((time_t *) dp);
				dp+=sizeof(aEvent.startTime);

				aEvent.duration = *((unsigned *) dp);
				dp+=sizeof(aEvent.duration);

				aEvent.description= dp;
				dp+=strlen(dp)+1;

				aEvent.text= dp;
				dp+=strlen(dp)+1;

				eList.push_back(aEvent);
			}
			delete[] pData;
		}
	}
	close_connection();
	return eList;
}

CChannelEventList CSectionsdClient::getEventsServiceKey(const t_channel_id channel_id)
{
	CChannelEventList eList;

	if (send(sectionsd::allEventsChannelID_, (char*)&channel_id, sizeof(channel_id)))
	{
		int nBufSize = readResponse();

		if( nBufSize > 0)
		{
			char* pData = new char[nBufSize];
			receive_data(pData, nBufSize);

			char* dp = pData;

			while(dp < pData + nBufSize)
			{
				CChannelEvent aEvent;

				aEvent.eventID = *((unsigned long long *) dp);
				dp+=sizeof(aEvent.eventID);

				aEvent.startTime = *((time_t *) dp);
				dp+=sizeof(aEvent.startTime);

				aEvent.duration = *((unsigned *) dp);
				dp+=sizeof(aEvent.duration);

				aEvent.description= dp;
				dp+=strlen(dp)+1;

				aEvent.text= dp;
				dp+=strlen(dp)+1;

				eList.push_back(aEvent);
			}
			delete[] pData;
		}
	}

	close_connection();
	return eList;
}

bool CSectionsdClient::getActualEPGServiceKey(const t_channel_id channel_id, CEPGData * epgdata)
{
	epgdata->title = "";

	if (send(sectionsd::actualEPGchannelID, (char*)&channel_id, sizeof(channel_id)))
	{
		int nBufSize = readResponse();
		if( nBufSize > 0)
		{
			char* pData = new char[nBufSize];
			receive_data(pData, nBufSize);

			close_connection();

			char* dp = pData;


			epgdata->eventID = *((unsigned long long *)dp);
			dp+= sizeof(epgdata->eventID);

			epgdata->title = dp;
			dp+=strlen(dp)+1;
			epgdata->info1 = dp;
			dp+=strlen(dp)+1;
			epgdata->info2 = dp;
			dp+=strlen(dp)+1;
			epgdata->contentClassification = dp;
			dp+=strlen(dp)+1;
			epgdata->userClassification = dp;
			dp+=strlen(dp)+1;
			epgdata->fsk = *dp++;

			epgdata->epg_times.startzeit = ((CSectionsdClient::sectionsdTime *) dp)->startzeit;
			epgdata->epg_times.dauer = ((CSectionsdClient::sectionsdTime *) dp)->dauer;
			dp+= sizeof(CSectionsdClient::sectionsdTime);

			delete[] pData;
			return true;
		}
		else
			printf("no response from sectionsd\n");
	}

	close_connection();

	return false;
}


bool CSectionsdClient::getEPGid(const unsigned long long eventid, const time_t starttime, CEPGData * epgdata)
{
	sectionsd::commandGetEPGid msg;

	msg.eventid   = eventid;
	msg.starttime = starttime; 

	if (send(sectionsd::epgEPGid, (char *)&msg, sizeof(msg)))
	{
		int nBufSize = readResponse();
		if (nBufSize > 0)
		{
			char* pData = new char[nBufSize];
			receive_data(pData, nBufSize);
			close_connection();

			char* dp = pData;


			epgdata->eventID = *((unsigned long long *)dp);
			dp+= sizeof(epgdata->eventID);

			epgdata->title = dp;
			dp+=strlen(dp)+1;
			epgdata->info1 = dp;
			dp+=strlen(dp)+1;
			epgdata->info2 = dp;
			dp+=strlen(dp)+1;
			epgdata->contentClassification = dp;
			dp+=strlen(dp)+1;
			epgdata->userClassification = dp;
			dp+=strlen(dp)+1;
			epgdata->fsk = *dp++;

			epgdata->epg_times.startzeit = ((CSectionsdClient::sectionsdTime *) dp)->startzeit;
			epgdata->epg_times.dauer = ((CSectionsdClient::sectionsdTime *) dp)->dauer;
			dp+= sizeof(CSectionsdClient::sectionsdTime);

			delete[] pData;
			return true;
		}
		else
			printf("no response from sectionsd\n");
	}

	close_connection();

	return false;
}


bool CSectionsdClient::getEPGidShort(const unsigned long long eventid, CShortEPGData * epgdata)
{
	if (send(sectionsd::epgEPGidShort, (char*)&eventid, sizeof(eventid)))
	{
		int nBufSize = readResponse();
		if( nBufSize > 0)
		{
			char* pData = new char[nBufSize];
			receive_data(pData, nBufSize);

			close_connection();

			char* dp = pData;

			for(int i = 0; i < nBufSize;i++)
				if(((unsigned char)pData[i]) == 0xff)
					pData[i] = 0;

			epgdata->title = dp;
			dp+=strlen(dp)+1;
			epgdata->info1 = dp;
			dp+=strlen(dp)+1;
			epgdata->info2 = dp;
			dp+=strlen(dp)+1;
//			printf("titel: %s\n",epgdata->title.c_str());


			delete[] pData;
			return true;
		}
		else
			printf("no response from sectionsd\n");
	}

	close_connection();
	
	return false;
}
