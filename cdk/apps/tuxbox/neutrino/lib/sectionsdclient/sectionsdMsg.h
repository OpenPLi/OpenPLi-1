#ifndef SECTIONSDMSG_H
#define SECTIONSDMSG_H
//
//  $Id: sectionsdMsg.h,v 1.1.2.2 2003/02/06 20:31:48 thegoodguy Exp $
//
//	sectionsdMsg.h (header file with msg-definitions for sectionsd)
//	(dbox-II-project)
//
//	Copyright (C) 2001 by fnbrd,
//                    2002 by thegoodguy
//
//    Homepage: http://dbox2.elxsi.de
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//


#include <connection/basicmessage.h>
#include <zapit/client/zapittypes.h>  /* t_channel_id */


#define SECTIONSD_UDS_NAME "/tmp/sectionsd.sock"


struct sectionsd
{
	static const CBasicMessage::t_version ACTVERSION = 4;

	struct msgRequestHeader
	{
		char version;
		char command;
		unsigned short dataLength;
	} __attribute__ ((packed)) ;

	struct msgResponseHeader
	{
		unsigned short dataLength;
	} __attribute__ ((packed)) ;

	enum commands
	{
		actualEPGchannelName=0,
		actualEventListTVshort,
		currentNextInformation,
		dumpStatusinformation,
		allEventsChannelName,
		setHoursToCache,
		setEventsAreOldInMinutes,
		dumpAllServices,
		actualEventListRadioshort,
		getNextEPG,
		getNextShort,
		pauseScanning, // for the grabbers ;)
		actualEPGchannelID,
		actualEventListTVshortIDs,
		actualEventListRadioShortIDs,
		currentNextInformationID,
		epgEPGid,
		epgEPGidShort,
		ComponentTagsUniqueKey,
		allEventsChannelID_,
		timesNVODservice,
		getEPGPrevNext,
		getIsTimeSet,
		serviceChanged,
		LinkageDescriptorsUniqueKey,
		pauseSorting,
		CMD_registerEvents,
		CMD_unregisterEvents,
		numberOfCommands        // <- no actual command, end of command marker
	};

	struct commandGetEPGid
	{
		unsigned long long eventid;
		time_t             starttime;
	} __attribute__ ((packed)) ; 

	struct commandSetServiceChanged
	{
		t_channel_id channel_id;
		bool         requestEvent;
	};

	struct responseIsTimeSet
	{
		bool IsTimeSet;
	};

};

//
// Description of Commands:
//
// If a command is recognize then sectionsd will always send a response.
// When requested datas are not found the data length of the response is 0.
//
// actualEPGchannelName:
//   data of request:
//     is channel name with trailing 0 (c-string)
//   data of response:
//     is a string (c-string) describing the EPG (for
//     compatibility with old epgd)
//
// actualEventListTVshort:
//   data of request:
//     -
//   data of response:
//     is a string (c-string) with all cached actual events,
//     3 lines per service, first line unique-event-key, second line service name, third line event name
//
// currentNextInformation:
//   data of request:
//     is channel name with trailing 0 (c-string)
//   data of response:
//     is a string (c-string) describing the current/next EPGs
//
// dumpStatusinformation:
//   data of request:
//     -
//   data of response:
//     is a string (c-string) describing current status of sectionsd
//
// allEventsChannelName:
//   data of request:
//     is channel name with trailing 0 (c-string)
//   data of response:
//     is a string (c-string) describing the cached events for the requestet channel
//     1 line per event, format: uniqueEventKey DD.MM HH:MM durationInMinutes Event name
//
// setHoursToCache
//   data of request:
//     unsigned short (hours to cache)
//   data of response:
//     -
//
// setEventsAreOldInMinutes
//   data of request:
//     unsigned short (minutes after events are old (after their end time))
//   data of response:
//     -
//
// dumpAllServicesinformation:
//   data of request:
//     -
//   data of response:
//     is a string (c-string) with all cached services
//     3 lines per service
//     1. line: unique-service-key, service-ID, service-type, eitScheduleFlag (bool),
//              eitPresentFollowingFlag (bool), runningStatus (bool),
//              freeCAmode (bool), number of nvod services
//     2. line: service name
//     3. line: provider name
//
// actualEventListRadioshort:
//   data of request:
//     -
//   data of response:
//     is a string (c-string) with all cached actual events,
//     3 lines per service, first line unique-event-key, second line service name, third line event name
//
// getNextEPG:
//   data of request:
//     8 bytes (long long in 32 bit world) with unique key (binary) of the event for wich the next should be delivered
//     4 bytes with start time (ctime) of the above event
//   data of response:
//     is a string (c-string) describing the EPG:
//     unique key (long long, hex) 0xff name  0xff text  0xff extended text  0xff start time GMT (ctime, hex ) 0xff duration (seconds, hex)
//
// getNextShort:
//   data of request:
//     8 bytes (long long in 32 bit world) with unique key (binary) of the event for wich the next should be delivered
//     4 bytes with start time (ctime) of the above event
//   data of response:
//     is a string (c-string) describing the Event in short terms:
//     1. line unique key (long long, hex), 2. line name, 3. line start time GMT (ctime, hex ), 4 line  duration (seconds, hex)
//
// pauseScanning:
//   data of request:
//     int (1 = pause, 0 = continue)
//   data of response:
//     -
//
// actualEPGchannelID:
//   data of request:
//     is channel ID
//   data of response:
//     is a string (c-string) describing the EPG:
//     unique key (long long, hex) 0xff name  0xff text  0xff extended text  0xff start time GMT (ctime, hex ) 0xff duration (seconds, hex) 0xff
//
// actualEventListTVshortIDs:
//   data of request:
//     -
//   data of response:
//     for every service:
//       1. unique-service-key (4 bytes)
//       2. unique-event-key (8 bytes)
//       3. event name (c-string with 0)
//
// actualEventListRadioShortIDs:
//   data of request:
//     -
//   data of response:
//       1. unique-service-key (4 bytes)
//       2. unique-event-key (8 bytes)
//       3. event name (c-string with 0)
//
// currentNextInformationID:
//   data of request:
//     4 byte channel ID (4 byte, onid<<16+sid)
//     1 byte number of Events (noch nicht implementiert)
//   data of response:
//     is a string (c-string) describing the current/next EPGs
//     every event:
//       1. 8 bytes unique key (unsigned long long),
//       2. struct sectionsdTime (8 bytes)
//       3. name (c-string with 0)
//
// epgEPGid:
//   data of request:
//     unique epg ID (8 byte)
//     time_t starttime GMT (4 bytes)
//   data of response:
//     is a string (c-string) describing the EPG:
//     name 0xff text 0xff extended text 0xff start time GMT (ctime, hex ) 0xff duration (seconds, hex) 0xff
//
// epgEPGidShort:
//   data of request:
//     unique epg ID (8 byte)
//   data of response:
//     is a string (c-string) describing the EPG:
//     name  0xff text  0xff extended text 0xff
//
// CurrentComponentTags
//   - gets all ComponentDescriptor-Tags for the currently running Event
//
//   data of request:
//     is channel ID (4 byte, onid<<16+sid)
//   data of response:
//      for each component-descriptor (zB %02hhx %02hhx %02hhx\n%s\n)
//          componentTag
//          componentType
//          streamContent
//          component.c_str
//
// allEventsChannelID:
//   data of request:
//     is channel ID
//   data of response:
//     is a string (c-string) describing the cached events for the requestet channel
//     1 line per event, format: uniqueEventKey DD.MM HH:MM durationInMinutes Event name
//
// timesNVODservice
//   data of request:
//     is channel ID
//   data of response:
//     for every (sub-)channel
//       channelID (4 byte, onid<<16+sid)
//       transportStreamID (2 byte)
//       start time (4 bytes ctime)
//       duration (4 bytes unsigned)
//
//  getEPGPrevNext
//   data of request:
//     8 bytes (long long in 32 bit world) with unique key (binary) of the event for wich the next should be delivered
//     4 bytes with start time (ctime) of the above event
//   data of response:
//     is a string (c-string) describing the EPG:
//     unique key (long long, hex) 0xff start time GMT (ctime, hex ) for previous event
//     unique key (long long, hex) 0xff start time GMT (ctime, hex ) for next event

//
#endif // SECTIONSDMSG_H
