#ifndef __sectionsdclient__
#define __sectionsdclient__
/*
  Client-Interface für zapit  -   DBoxII-Project

  $Id: sectionsdclient.h,v 1.17.2.2 2003/02/06 20:32:18 thegoodguy Exp $

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

#include <string>
#include <vector>

#include <connection/basicclient.h>
#include <zapit/client/zapittypes.h>  /* t_channel_id, t_service_id, t_original_network_id, t_transport_stream_id; */


class CShortEPGData
{
 public:
	std::string title;
	std::string info1;
	std::string info2;

	CShortEPGData()
		{
			title = "";
			info1 = "";
			info2 = "";
		};
};

class CEPGData;

class CChannelEvent
{
 public:
	unsigned           serviceID( void ) const { return ( eventID>>16 ); }
	unsigned long long eventID;
	std::string        description;
	std::string        text;
	time_t             startTime;
	unsigned           duration;
};

typedef std::vector<CChannelEvent> CChannelEventList;

class CSectionsdClient : private CBasicClient
{
 private:
	virtual const unsigned char   getVersion   () const;
	virtual const          char * getSocketName() const;

	int readResponse(char* data = NULL, int size = 0);
	bool send(const unsigned char command, const char* data = NULL, const unsigned int size = 0);

 public:
	enum events
		{
			EVT_TIMESET,
			EVT_GOT_CN_EPG
		};
	
	struct epgflags {
		enum
		{
			has_anything = 0x01,
			has_later = 0x02,
			has_current = 0x04,
			not_broadcast = 0x08,
			has_next = 0x10,
			has_no_current= 0x20,
			current_has_linkagedescriptors= 0x40
		};
	};
	
	struct responseGetComponentTags
	{
		std::string   component;        // Text aus dem Component Descriptor
		unsigned char componentType;    // Component Descriptor
		unsigned char componentTag;     // Component Descriptor
		unsigned char streamContent;    // Component Descriptor
	};
	typedef std::vector<responseGetComponentTags> ComponentTagList;

	struct responseGetLinkageDescriptors
	{
		std::string           name;
		t_transport_stream_id transportStreamId;
		t_original_network_id originalNetworkId;
		t_service_id          serviceId;
	};
	typedef std::vector<responseGetLinkageDescriptors> LinkageDescriptorList;

	struct sectionsdTime
	{
		time_t startzeit;
		unsigned dauer;
	} __attribute__ ((packed)) ;

	struct responseGetNVODTimes
	{
		t_service_id                    service_id;
		t_original_network_id           original_network_id;
		t_transport_stream_id           transport_stream_id;
		CSectionsdClient::sectionsdTime zeit;
	};
	typedef std::vector<responseGetNVODTimes> NVODTimesList;

	struct responseGetCurrentNextInfoChannelID
	{
		unsigned long long              current_uniqueKey;
		CSectionsdClient::sectionsdTime current_zeit;
		std::string                     current_name;
		char                            current_fsk;
		unsigned long long              next_uniqueKey;
		CSectionsdClient::sectionsdTime next_zeit;
		std::string                     next_name;
		unsigned                        flags;
	};

	struct CurrentNextInfo : public responseGetCurrentNextInfoChannelID
	{};



	bool getComponentTagsUniqueKey(const unsigned long long uniqueKey, CSectionsdClient::ComponentTagList& tags);

	bool getLinkageDescriptorsUniqueKey(const unsigned long long uniqueKey, CSectionsdClient::LinkageDescriptorList& descriptors);

	bool getNVODTimesServiceKey(const t_channel_id channel_id, CSectionsdClient::NVODTimesList& nvod_list);

	bool getCurrentNextServiceKey(const t_channel_id channel_id, CSectionsdClient::responseGetCurrentNextInfoChannelID& current_next);

	bool getIsTimeSet();

	void setEventsAreOldInMinutes(const unsigned short minutes);
	
	void setPauseScanning(const bool doPause);

	void setPauseSorting(const bool doPause);

	void setServiceChanged(const t_channel_id channel_id, const bool requestEvent);

	CChannelEventList getChannelEvents();

	CChannelEventList getEventsServiceKey(const t_channel_id channel_id);

	bool getEPGid(const unsigned long long eventid, const time_t starttime, CEPGData * epgdata);

	bool getActualEPGServiceKey(const t_channel_id channel_id, CEPGData * epgdata);

	bool getEPGidShort(const unsigned long long eventid, CShortEPGData * epgdata);


	/*
	  ein beliebiges Event anmelden
	*/
	void registerEvent(const unsigned int eventID, const unsigned int clientID, const std::string udsName);

	/*
	  ein beliebiges Event abmelden
	*/
	void unRegisterEvent(const unsigned int eventID, const unsigned int clientID);

};

class CEPGData
{
 public:
	unsigned long long              eventID;
	CSectionsdClient::sectionsdTime	epg_times;
	std::string                     title;
	std::string                     info1;
	std::string                     info2;
	char                            fsk;
	std::string                     contentClassification;
	std::string                     userClassification;

	CEPGData()
		{
			eventID               =  0;
			title                 = "";
			info1                 = "";
			info2                 = "";
			fsk                   =  0;
			contentClassification = "";
			userClassification    = "";
		};

};

#endif
