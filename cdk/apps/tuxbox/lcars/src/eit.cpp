/***************************************************************************
    copyright            : (C) 2001 by TheDOC
    email                : thedoc@chatville.de
	homepage			 : www.chatville.de
	modified by			 : -
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
$Log: eit.cpp,v $
Revision 1.15.6.4  2008/08/07 20:25:30  fergy
Mostly clear of not needed lines
Added back debug messages ( just for dev. )
Enambled some disabled stuff from before
Code cleaning

Revision 1.15.6.3  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.15  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.14  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.13  2002/10/14 01:19:15  woglinde


mostly compiler warnings, but I got not all

Revision 1.12  2002/09/18 17:31:03  TheDOC
replaced O_RDONLY with O_RDWR on demux-device-open, stupid me

Revision 1.11  2002/09/18 10:48:37  obi
use devfs devices

Revision 1.10  2002/06/13 01:35:48  TheDOC
NVOD should work now

Revision 1.9  2002/06/08 15:11:47  TheDOC
autostart in yadd added

Revision 1.8  2002/06/08 14:40:23  TheDOC
made lcars tuxvision-compatible

Revision 1.7  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.6  2002/05/20 20:08:12  TheDOC
some new timer and epg-stuff

Revision 1.5  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.4  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <memory.h>
#include <stdio.h>

#include "devices.h"
#include "eit.h"
#include "help.h"
#include "osd.h"
#include "descriptors.h"

#define BSIZE 10000

int eit::start_thread()
{

	int status;

	pthread_mutex_init(&mutex, NULL);
	status = pthread_create( &eitThread,
	                         NULL,
	                         start_eitqueue,
	                         (void *)this );
	return status;

}

void* eit::start_eitqueue( void * this_ptr )
{
	eit *e = (eit *) this_ptr;
	time_t next_time = time(0) + 10000;

	while(1)
	{
		while(e->isEmpty() && time(0) < next_time)
		{
			usleep(1000);
		}
		if (time(0) >= next_time)
			e->addCommand("RECEIVE last");
		e->executeQueue();
		next_time = time(0) + 5;
	}
	return 0;
}

eit::eit(settings *s, osd *o, variables *v)
{
	setting = s;
	number_perspectives = 0;
	osd_obj = o;
	vars = v;
	gotNow = false;
}

void eit::addCommand(std::string command)
{
	while(!command_queue.empty())
		command_queue.pop();
	command_queue.push(command);
}

void eit::executeQueue()
{
	while(!isEmpty())
	{
		executeCommand();
	}
}

void eit::executeCommand()
{
	std::string command = command_queue.front();
	command_queue.pop();

	std::istringstream iss(command);
	std::getline(iss, command, ' ');

	if(command == "RECEIVE")
	{
		std::string SID_str;
		std::getline(iss, SID_str, ' ');

		if (SID_str != "last")
			lastSID = atoi(SID_str.c_str());
		receiveNow(lastSID);

	}
}


void eit::receiveNow(int SID)
{
	long fd, r;
	struct dmx_sct_filter_params flt;
	unsigned char buffer[BSIZE];

	number_perspectives = 0;
	curr_linkage = 0;
	pthread_mutex_lock( &mutex );
	fd=open(DEMUX_DEV, O_RDWR);

	memset (&flt.filter, 0, sizeof (struct dmx_filter));

	eventlist.clear();
	flt.pid            = 0x12;
	flt.filter.filter[0] = 0x4e;
	flt.filter.mask[0] = 0xFF;
	flt.timeout        = 10000;
	flt.timeout        = 0;
	flt.flags          = DMX_IMMEDIATE_START | DMX_CHECK_CRC;

	ioctl(fd, DMX_SET_FILTER, &flt);

	int start_sid, start_section_number;
	r = BSIZE;
	memset (&buffer, 0, BSIZE);
	r = read(fd, buffer, 3);
	int section_length = ((buffer[1] & 0xf) << 8) | buffer[2];
	r = read(fd, buffer + 3, section_length);
	start_sid = (buffer[3] << 8) | buffer[4];
	start_section_number = buffer[6];


	memset (&now, 0, sizeof (struct event));
	memset (&next, 0, sizeof (struct event));
	bool quit = false;
	int sec_count = 0;
	do
	{
		sec_count++;
		r = BSIZE;
		memset (&buffer, 0, BSIZE);

		r = read(fd, buffer, 3);
		int section_length = ((buffer[1] & 0xf) << 8) | buffer[2];
		r = read(fd, buffer + 3, section_length);

		int count = 13;

		if (SID == ((buffer[3] << 8) | buffer[4]))
		{
			event tmp_event;
			memset (&tmp_event, 0, sizeof (struct event));

			int time = (buffer[count + 5] << 16) | (buffer[count + 6] << 8) | buffer[count + 7];
			int mjd = (buffer[count + 3] << 8) | buffer[count + 4];
			time_t starttime = dvbtimeToLinuxTime(mjd, time) + ((*setting).getTimeOffset() * 60L);

			tmp_event.duration = (((buffer[count + 8] & 0xf0) >> 4) * 10 + (buffer[count + 8] & 0xf)) * 3600 + (((buffer[count + 9] & 0xf0) >> 4) * 10 + (buffer[count + 9] & 0xf)) * 60 + (((buffer[count + 10] & 0xf0) >> 4) * 10 + (buffer[count + 10] & 0xf));
			tmp_event.starttime = starttime;

			tmp_event.eventid = (buffer[count + 1] << 8) | buffer[count + 2];
			tmp_event.running_status = ((buffer[count + 11] & 0xe0) >> 5);

			int start = 26;
			int descriptors_length = ((buffer[13 + 11] & 0xf) << 8) | buffer[13 + 12];
			int text_length = 0;
			while (start < 20 + descriptors_length)
			{
				if (buffer[start] == 0x4d) // short_event_descriptor
				{
					std::string tmp_string;
					int event_name_length = buffer[start + 5];
					for (int i = 0; i < event_name_length; i++)
						tmp_event.event_name[i] = buffer[start + 6 + i];
					tmp_event.event_name[event_name_length] = '\0';

					int text_length = buffer[start + 6 + event_name_length];
					for (int i = 0; i < text_length; i++)
						tmp_event.event_short_text[i] = buffer[start + 7 + event_name_length + i];
					tmp_event.event_short_text[text_length] = '\0';


				}
				else if (buffer[start] == 0x4a && ((buffer[count + 11] & 0xe0) >> 5) == 0x04) // linkage
				{
					memset (&linkage_descr[number_perspectives], 0, sizeof (struct linkage));
					tmp_event.linkage_descr[tmp_event.number_perspectives].TS = (buffer[start + 2] << 8) | buffer[start + 3];
					tmp_event.linkage_descr[tmp_event.number_perspectives].ONID = (buffer[start + 4] << 8) | buffer[start + 5];
					tmp_event.linkage_descr[tmp_event.number_perspectives].SID = (buffer[start + 6] << 8) | buffer[start + 7];

					char name[100];
					if (buffer[start + 8] != 0xb0)
					{
						for (int i = 9; i <= buffer[start + 1]; i++)
						{
						}
					}
					else // Formel 1 Perspective - Linkage type 0xb0
					{
						for (int i = 9; i <= buffer[start + 1] + 1; i++)
						{
							name[i - 9] = buffer[start + i]; // Name of Perspective
						}
						name[buffer[start + 1] - 7] = '\0';
						strcpy(tmp_event.linkage_descr[tmp_event.number_perspectives].name, name);
					}
					tmp_event.number_perspectives++;
				}
				else if (buffer[start] == 0x4e) // extended_event_descriptor
				{
					int pos = start + 6 + buffer[start + 6];

					int i;
					for (i = 0; i < buffer[pos + 1]; i++)
						tmp_event.event_extended_text[i + text_length] = buffer[pos + 2 + i];

					text_length += buffer[pos + 1];
				}
				else if (buffer[start] == 0x50) // component_descriptor - audio-names
				{
					tmp_event.component_tag[tmp_event.number_components] = buffer[start + 4];
					tmp_event.stream_content[tmp_event.number_components] = buffer[start + 2] & 0xf;
					tmp_event.component_type[tmp_event.number_components] = buffer[start + 3];

					for (int i = 7; i <= buffer[start + 1]; i++)
					{
						tmp_event.audio_description[tmp_event.number_components][i-7] = buffer[start + 1 + i];
					}
					tmp_event.audio_description[tmp_event.number_components][buffer[start + 1] - 6] = '\0';
					tmp_event.number_components++;

				}
				else if (buffer[start] == 0x55)
				{
					for (int i = 0; i < buffer[start + 1] / 4; i++)
					{
						if (buffer[start + 2 + i * 4] == 'D' && buffer[start + 3 + i * 4] == 'E' && buffer[start + 4 + i * 4] == 'U')
							tmp_event.par_rating = buffer[start + 5 + i * 4];
					}
				}

				start += buffer[start + 1] + 2;


			}
			tmp_event.event_extended_text[text_length] = '\0';
			if (eventlist.count((int)tmp_event.starttime) == 0)
				eventlist.insert(std::pair<int, struct event>((int) tmp_event.starttime, tmp_event));
		}

	} while((!((start_sid == ((buffer[3] << 8) | buffer[4])) && (start_section_number == buffer[6]))) && !quit && (sec_count < 100));
	ioctl(fd,DMX_STOP,0);

	close(fd);
	pthread_mutex_unlock( &mutex );

	std::multimap<int, struct event>::iterator it = eventlist.begin();
	for (int i = 0; i < (int)eventlist.size(); i++)
	{
		if (i == 0)
			now = (*it).second;
		if (i == 1)
			next = (*it).second;
		struct tm *t;
		t = localtime(&(*it).second.starttime);
		char acttime[10];
		strftime(acttime, sizeof acttime, "%H:%M %d.%m", t);
		it++;
	}
	(*osd_obj).setNowTime(now.starttime);

	if (strlen(now.event_name) != 0)
	{
		(*osd_obj).setNowDescription(now.event_name);
	}
	(*osd_obj).setNextTime(next.starttime);
	if (strlen(next.event_name) != 0)
	{
		(*osd_obj).setNextDescription(next.event_name);
	}
	(*osd_obj).setParentalRating(now.par_rating);


	vars->setvalue("%NOWEVENTNAME", now.event_name);
	vars->setvalue("%NOWSHORTTEXT", now.event_short_text);
	vars->setvalue("%NOWEXTENDEDTEXT", now.event_extended_text);
	vars->setvalue("%NOWDESCRIPTION", now.event_extended_text);
	vars->setvalue("%NOWSTARTTIME", now.starttime);
	vars->setvalue("%NOWDURATION", now.duration);
	vars->setvalue("%NOWLINKAGE", now.number_perspectives);
	if (now.number_components > 0)
	{
		vars->setvalue("%NOWAUDIO", now.audio_description[0]);
		act_nowcomponent = 0;
	}
	else
	{
		vars->setvalue("%NOWAUDIO", "N/A");
		act_nowcomponent = -1;
	}
	vars->setvalue("%NOWPARRATING", now.par_rating);

	vars->setvalue("%NEXTEVENTNAME", next.event_name);
	vars->setvalue("%NEXTSHORTTEXT", next.event_short_text);
	vars->setvalue("%NEXTEXTENDEDTEXT", next.event_extended_text);
	vars->setvalue("%NEXTDESCRIPTION", now.event_extended_text);
	vars->setvalue("%NEXTSTARTTIME", next.starttime);
	vars->setvalue("%NEXTDURATION", next.duration);
	vars->setvalue("%NEXTLINKAGE", next.number_perspectives);
	if (next.number_components > 0)
	{
		vars->setvalue("%NEXTAUDIO", next.audio_description[0]);
		act_nextcomponent = 0;
	}
	else
	{
		vars->setvalue("%NEXTAUDIO", "N/A");
		act_nextcomponent = -1;
	}
	vars->setvalue("%NEXTPARRATING", next.par_rating);

	bool found = false;
	for (int i = 0; i < now.number_components; i++)
	{
		if (now.component_tag[i] == audio_comp)
		{
			(*osd_obj).setLanguage(now.audio_description[i]);
			found = true;
		}
	}

	if (!found)
		(*osd_obj).setLanguage("");

	if (now.number_perspectives > 1)
	{
		vars->setvalue("%NUMBER_PERSPECTIVES", now.number_perspectives);
		vars->setvalue("%IS_MULTIPERSPECTIVE", "true");
		osd_obj->setPerspectiveAvailable(true);
	}
	else
	{
		vars->setvalue("%IS_MULTIPERSPECTIVE", "false");
		osd_obj->setPerspectiveAvailable(false);
	}

	gotNow = true;
}

void eit::setAudioComponent(int comp)
{
	audio_comp = comp;

	for (int i = 0; i < now.number_components; i++)
	{
		if (now.component_tag[i] == audio_comp)
		{
			(*osd_obj).setLanguage(now.audio_description[i]);
		}
	}
}

bool eit::isMultiPerspective()
{
	return /*(now.number_perspectives > 1);*/ (now.running_status == 0x4 && now.number_perspectives > 1);
}

void eit::beginLinkage()
{
	curr_linkage = 0;
}

linkage eit::nextLinkage()
{
	return (now.linkage_descr[curr_linkage++]);
}
void eit::readSchedule(int SID, osd *osd)
{
	long fd, r;
	struct dmx_sct_filter_params flt;
	unsigned char buffer[BSIZE];

	(*osd).createPerspective();
	(*osd).setPerspectiveName("Reading Scheduling Information...");
	(*osd).showPerspective();

	fd=open(DEMUX_DEV, O_RDWR);
	
	memset (&flt.filter, 0, sizeof (struct dmx_filter));
	
	eventlist.clear();

	flt.pid            = 0x12;
	flt.filter.filter[0] = 0x50;
	flt.filter.mask[0] = 0xF0;
	flt.timeout        = 0;
	flt.flags          = DMX_IMMEDIATE_START | DMX_CHECK_CRC | DMX_ONESHOT;
	
	ioctl(fd, DMX_SET_FILTER, &flt);
	
	memset (&buffer, 0, BSIZE);
	r = read(fd, buffer, 3);
	int section_length = ((buffer[1] & 0xf) << 8) | buffer[2];
	r = read(fd, buffer + 3, section_length);

	int number_tables = (buffer[13] & 0xf) + 1;
	bool finished[number_tables];
	int starting_element[number_tables];
	bool quit;

	for (int i = 0; i < number_tables ; i++)
	{
		starting_element[i] = -1;
		finished[i] = false;
	}

	do
	{
		memset (&buffer, 0, BSIZE);
		r = read(fd, buffer, 3);
		int section_length = ((buffer[1] & 0xf) << 8) | buffer[2];
		r = read(fd, buffer + 3, section_length);
	
		if (SID == ((buffer[3] << 8) | buffer[4]))
		{
			if (!(finished[buffer[0] & 0xf]))
			{
				if (starting_element[buffer[0] & 0xf] == buffer[6])
				{
					finished[buffer[0] & 0xf] = true;
				}
				else
				{
					if (starting_element[buffer[0] & 0xf] == -1)
					{
						starting_element[buffer[0] & 0xf] = buffer[6];
					}
					
					int count = 13;
					do
					{
					event tmp_event;
					memset (&tmp_event, 0, sizeof (struct event));

					int time = (buffer[count + 5] << 16) | (buffer[count + 6] << 8) | buffer[count + 7];
					int mjd = (buffer[count + 3] << 8) | buffer[count + 4];
					time_t starttime = dvbtimeToLinuxTime(mjd, time) + ((*setting).getTimeOffset() * 60L);
	
					tmp_event.duration = (((buffer[count + 8] & 0xf0) >> 4) * 10 + (buffer[count + 8] & 0xf)) * 3600 + (((buffer[count + 9] & 0xf0) >> 4) * 10 + (buffer[count + 9] & 0xf)) * 60 + (((buffer[count + 10] & 0xf0) >> 4) * 10 + (buffer[count + 10] & 0xf));
					tmp_event.starttime = starttime;

					tmp_event.eventid = (buffer[count + 1] << 8) | buffer[count + 2];
					tmp_event.running_status = ((buffer[count + 11] & 0xe0) >> 5);
	
					int start = count + 13;
					int descriptors_length = ((buffer[count + 11] & 0xf) << 8) | buffer[count + 12];
					int text_length = 0;
					while (start < count + descriptors_length)
					{
						if (buffer[start] == 0x4d) // short_event_descriptor
						{	
							std::string tmp_string;
							int event_name_length = buffer[start + 5];
							for (int i = 0; i < event_name_length; i++)
								tmp_event.event_name[i] = buffer[start + 6 + i];
							tmp_event.event_name[event_name_length] = '\0';
			
							int text_length = buffer[start + 6 + event_name_length];
							for (int i = 0; i < text_length; i++)
								tmp_event.event_short_text[i] = buffer[start + 7 + event_name_length + i];
							tmp_event.event_short_text[text_length] = '\0';
					
	
						}
						else if (buffer[start] == 0x4a && ((buffer[count + 11] & 0xe0) >> 5) == 0x04) // linkage
						{
							memset (&linkage_descr[number_perspectives], 0, sizeof (struct linkage));
							linkage_descr[number_perspectives].TS = (buffer[start + 2] << 8) | buffer[start + 3];
							linkage_descr[number_perspectives].ONID = (buffer[start + 4] << 8) | buffer[start + 5];
							linkage_descr[number_perspectives].SID = (buffer[start + 6] << 8) | buffer[start + 7];
							
							char name[100];
							if (buffer[start + 8] != 0xb0)
							{
								for (int i = 9; i <= buffer[start + 1]; i++)
								{
								}
							}
							else // Formel 1 Perspective - Linkage type 0xb0
							{
								for (int i = 9; i <= buffer[start + 1] + 1; i++)
								{
									name[i - 9] = buffer[start + i]; // Name of Perspective
								}
								name[buffer[start + 1] - 7] = '\0';
								strcpy(linkage_descr[number_perspectives].name, name);
							}
							number_perspectives++;
						}
						else if (buffer[start] == 0x4e) // extended_event_descriptor
						{
							int pos = start + 6 + buffer[start + 6];
							
							int i;
							for (i = 0; i < buffer[pos + 1]; i++)
								tmp_event.event_extended_text[i + text_length] = buffer[pos + 2 + i];
						
							text_length += buffer[pos + 1];
						}
						else if (buffer[start] == 0x50) // component_descriptor - audio-names
						{
							tmp_event.component_tag[tmp_event.number_components] = buffer[start + 4];
							tmp_event.stream_content[tmp_event.number_components] = buffer[start + 2] & 0xf;
							tmp_event.component_type[tmp_event.number_components] = buffer[start + 3];
		
							for (int i = 7; i <= buffer[start + 1]; i++)
							{
								tmp_event.audio_description[tmp_event.number_components][i-7] = buffer[start + 1 + i];
							}
							tmp_event.audio_description[tmp_event.number_components][buffer[start + 1] - 6] = '\0';
							tmp_event.number_components++;
			
						}

						start += buffer[start + 1] + 2;
		
						
					}
			
					tmp_event.event_extended_text[text_length] = '\0';
					if (eventlist.count((int)tmp_event.starttime) == 0)
						eventlist.insert(std::pair<int, struct event>((int) tmp_event.starttime, tmp_event));
					count += count + descriptors_length - 1;
					
					} while (count < r - 2);
					sleep(10);
				}
			}
		}
		quit = true;
		for (int i = 0; i < number_tables; i++)
		{
			if (!finished[i])
				quit = false;
		}
	} while(!quit);
	(*osd).hidePerspective();
}
void eit::dumpSchedule(int TS, int ONID, int SID, osd *osd)
{
	struct sid new_sid;
	new_sid.SID = SID;
	new_sid.TS = TS;
	new_sid.ONID = ONID;

	for (std::map<int, struct event>::iterator it = eventid_event.begin(); it != eventid_event.end(); ++it)
	{
		event tmp_event = it->second;
		osd->addScheduleInformation(tmp_event.starttime, tmp_event.event_name, tmp_event.eventid);
	}
	std::multimap<struct sid, std::multimap<time_t, int>, ltstr>::iterator it = sid_eventid.find(new_sid);
	{
		for (std::multimap<time_t, int>::iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); ++it2)
		{
			event tmp_event = eventid_event.find(it2->second)->second;
			if ((tmp_event.starttime + tmp_event.duration) >= time(0))
			{
				osd->addScheduleInformation(tmp_event.starttime, tmp_event.event_name, tmp_event.eventid);
			}
		}
	}
}


void eit::dumpSchedule(int SID, osd *osd)
{
	long fd, r;
	struct dmx_sct_filter_params flt;
	unsigned char buffer[BSIZE];
	time_eventid.clear();
	eventid_event.clear();

	osd->createPerspective();
	osd->setPerspectiveName("Reading Scheduling Information...");
	osd->addCommand("SHOW perspective");

	pthread_mutex_lock( &mutex );
	fd=open(DEMUX_DEV, O_RDWR);

	memset (&flt.filter, 0, sizeof (struct dmx_filter));

	flt.pid            = 0x12;
	flt.filter.filter[0] = 0x50;
	flt.filter.mask[0] = 0xF0;
	flt.timeout        = 10000;
	flt.flags          = DMX_IMMEDIATE_START | DMX_CHECK_CRC;

	ioctl(fd, DMX_SET_FILTER, &flt);

	memset (&buffer, 0, BSIZE);
	r = read(fd, buffer, 3);
	int section_length = ((buffer[1] & 0xf) << 8) | buffer[2];
	r = read(fd, buffer + 3, section_length);

	bool quit = false;
	int number_tables = (buffer[13] & 0xf) + 1;
	bool finished[number_tables];
	int starting_element[number_tables];

	for (int i = 0; i < number_tables ; i++)
	{
		starting_element[i] = -1;
		finished[i] = false;
	}

	int timeout = time(0) + 20;
	do
	{
		memset (&buffer, 0, BSIZE);

		r = read(fd, buffer, 3);
		int section_length = ((buffer[1] & 0xf) << 8) | buffer[2];
		r = read(fd, buffer + 3, section_length);

		if (r == 0)
			continue;


		eit_header tmp_header;
		memcpy (&tmp_header, &buffer, sizeof(struct eit_header));


		if (!(finished[buffer[0] & 0xf]))
		{
			if (starting_element[buffer[0] & 0xf] == tmp_header.section_number && tmp_header.service_id == SID)
			{
				finished[buffer[0] & 0xf] = true;
			}
			else
			{
				if (starting_element[buffer[0] & 0xf] == -1 && tmp_header.service_id == SID)
				{
					starting_element[buffer[0] & 0xf] = tmp_header.section_number;
				}
				event tmp_event;
				memset (&tmp_event, 0, sizeof (struct event));

				tmp_event.TS = tmp_header.transport_stream_id;
				tmp_event.ONID = tmp_header.original_network_id;
				tmp_event.SID = tmp_header.service_id;

				int end_counter = 0;
				int sec_counter = 0;

				while(sizeof(struct eit_header) + end_counter < (unsigned int) (r - 1))
				{
					event_header tmp_event_header;
					memcpy (&tmp_event_header, &(buffer[sizeof(struct eit_header) + end_counter]), sizeof(struct event_header));
					int i = sizeof(struct eit_header) + end_counter;
					time_t starttime = dvbtimeToLinuxTime(tmp_event_header.start_time_mjd, tmp_event_header.start_time_time) + ((*setting).getTimeOffset() * 60L);
					tmp_event.starttime = starttime;
					tmp_event.eventid = tmp_event_header.event_id;
					tmp_event.duration = (((buffer[i + 7] & 0xf0) >> 4) * 10 + (buffer[i + 7] & 0xf)) * 3600 + (((buffer[i + 8] & 0xf0) >> 4) * 10 + (buffer[i + 8] & 0xf)) * 60 + (((buffer[i + 9] & 0xf0) >> 4) * 10 + (buffer[i + 9] & 0xf));
					tmp_event.running_status = tmp_event_header.running_status;

					int ext_event_length = 0;

					int desc_counter = sizeof(struct eit_header) + sizeof(struct event_header);
					int end_desc = sizeof(struct eit_header) + sizeof(struct event_header) + tmp_event_header.descriptors_loop_length;
					while (desc_counter < end_desc)
					{
						if (buffer[desc_counter + end_counter] == 0x4d)
						{
							short_event_descriptor_header short_event_descriptor;
							memcpy(&short_event_descriptor, &buffer[desc_counter + end_counter], sizeof(struct short_event_descriptor_header));

							int text_position = desc_counter + sizeof(struct short_event_descriptor_header);
							for (int i = 0; i < short_event_descriptor.event_name_length; i++)
							{
								tmp_event.event_name[i] = buffer[text_position + end_counter + i];
							}
							tmp_event.event_name[short_event_descriptor.event_name_length] = '\0';


							text_position +=  short_event_descriptor.event_name_length + 1;
							int text_length = buffer[text_position + end_counter - 1];

							for (int i = 0; i < text_length; i++)
							{
								tmp_event.event_short_text[i] = buffer[text_position + end_counter + i];
							}
							tmp_event.event_short_text[text_length] = '\0';

						}
						else if (buffer[desc_counter + end_counter] == 0x4a && ((buffer[counter + 11] & 0xe0) >> 5) == 0x04) // linkage
						{
							tmp_event.number_perspectives++;
						}
						else if (buffer[desc_counter + end_counter] == 0x4e)
						{
							extended_event_descriptor_header extended_event_descriptor;
							memcpy(&extended_event_descriptor, &buffer[desc_counter + end_counter], sizeof(struct extended_event_descriptor_header));

							int text_position = desc_counter + sizeof(struct extended_event_descriptor_header) + extended_event_descriptor.length_of_items + 1;
							int text_length = buffer[text_position + end_counter - 1];

							for (int i = 0; i < text_length; i++)
							{
								tmp_event.event_extended_text[ext_event_length++] = buffer[text_position + end_counter + i];
							}

						}
						else if (buffer[desc_counter + end_counter] == 0x50)
						{
							int start = desc_counter + end_counter;
							tmp_event.component_tag[tmp_event.number_components] = buffer[start + 4];
							tmp_event.stream_content[tmp_event.number_components] = buffer[start + 2] & 0xf;
							tmp_event.component_type[tmp_event.number_components] = buffer[start + 3];

							for (int i = 7; i <= buffer[start + 1]; i++)
							{
								tmp_event.audio_description[tmp_event.number_components][i-7] = buffer[start + 1 + i];
							}
							tmp_event.audio_description[tmp_event.number_components][buffer[start + 1] - 6] = '\0';
							tmp_event.number_components++;

						}
						else if (buffer[desc_counter + end_counter] == 0x55)
						{
							int start = desc_counter + end_counter;
							for (int i = 0; i < buffer[start + 1] / 4; i++)
							{
								if (buffer[start + 2 + i * 4] == 'D' && buffer[start + 3 + i * 4] == 'E' && buffer[start + 4 + i * 4] == 'U')
									tmp_event.par_rating = buffer[start + 5 + i * 4];
							}
						}


						desc_counter += buffer[desc_counter + 1 + end_counter] + 2;
					}
					tmp_event.event_extended_text[ext_event_length] = '\0';

					if (eventid_event.count(tmp_event.eventid) == 0)
					{
						struct sid new_sid;
						new_sid.SID = tmp_event.SID;
						new_sid.TS = tmp_event.TS;
						new_sid.ONID = tmp_event.ONID;

						if (sid_eventid.count(new_sid) == 0)
						{
							std::multimap<time_t, int> new_timelist;
							new_timelist.insert(std::pair<time_t, int>(tmp_event.starttime, tmp_event.eventid));
							sid_eventid.insert(std::pair<struct sid, std::multimap<time_t, int> >(new_sid, new_timelist));
						}
						else
						{
							(*sid_eventid.find(new_sid)).second.insert(std::pair<time_t, int>(tmp_event.starttime, tmp_event.eventid));
						}
						if (tmp_event.SID == SID)
						{
							eventid_event[tmp_event.eventid] = tmp_event; //.insert(std::pair<int, struct event>(tmp_event.eventid, tmp_event));
							time_eventid[tmp_event.starttime] = tmp_event.eventid;
						}
					}

					end_counter += tmp_event_header.descriptors_loop_length + sizeof(struct event_header);
					sec_counter++;
				}
			}
		}
		quit = true;
		for (int i = 0; i < number_tables; i++)
		{
			if (!finished[i])
			{
				quit = false;
			}
		}
	} while(!quit && (time(0) < timeout));
	close(fd);
	pthread_mutex_unlock( &mutex );

	osd->addCommand("HIDE perspective");
}

event eit::getEvent(int eventid)
{
	return (*eventid_event.find(eventid)).second;
}

void eit::dumpEvent(int eventid)
{
	if (eventid_event.count(eventid) == 0)
		return;
	event tmp_event = eventid_event.find(eventid)->second;
	vars->setvalue("%EVENTNAME", tmp_event.event_name);
	vars->setvalue("%SHORTTEXT", tmp_event.event_short_text);
	vars->setvalue("%EXTENDEDTEXT", tmp_event.event_extended_text);
	vars->setvalue("%STARTTIME", tmp_event.starttime);
	vars->setvalue("%DURATION", tmp_event.duration);
	vars->setvalue("%LINKAGE", tmp_event.number_perspectives);
	if (tmp_event.number_components > 0)
	{
		vars->setvalue("%AUDIO", tmp_event.audio_description[0]);
		act_schedcomponent = 0;
	}
	else
	{
		vars->setvalue("%AUDIO", "N/A");
		act_schedcomponent = -1;
	}

	vars->setvalue("%PARRATING", tmp_event.par_rating);
}

void eit::dumpNextEvent(int eventid)
{
	if (eventid_event.count(eventid) == 0)
		return;
	std::map<int, struct event>::iterator it = eventid_event.find(eventid);

	if (time_eventid.count(it->second.starttime) == 0)
		return;
	std::map<time_t, int>::iterator it2 = time_eventid.find(it->second.starttime);
	if (it2 == time_eventid.end())
		it2 = time_eventid.begin();
	else
		it2++;

	if (eventid_event.count(it2->second) == 0)
		return;
	std::map<int, struct event>::iterator it3 = eventid_event.find(it2->second);

	event tmp_event = it3->second;
	vars->setvalue("%EVENTNAME", tmp_event.event_name);
	vars->setvalue("%SHORTTEXT", tmp_event.event_short_text);
	vars->setvalue("%EXTENDEDTEXT", tmp_event.event_extended_text);
	vars->setvalue("%STARTTIME", tmp_event.starttime);
	vars->setvalue("%DURATION", tmp_event.duration);
	vars->setvalue("%EVENTID", tmp_event.eventid);
	vars->setvalue("%LINKAGE", tmp_event.number_perspectives);
	if (tmp_event.number_components > 0)
	{
		vars->setvalue("%AUDIO", tmp_event.audio_description[0]);
		act_schedcomponent = 0;
	}
	else
	{
		vars->setvalue("%AUDIO", "N/A");
		act_schedcomponent = -1;
	}
	vars->setvalue("%PARRATING", tmp_event.par_rating);
}

void eit::dumpPrevEvent(int eventid)
{
	if (eventid_event.count(eventid) == 0)
		return;
	std::map<int, struct event>::iterator it = eventid_event.find(eventid);

	if (time_eventid.count(it->second.starttime) == 0)
		return;
	std::map<time_t, int>::iterator it2 = time_eventid.find(it->second.starttime);
	if (it2 == time_eventid.begin())
		it2 = time_eventid.end();
	else
		it2--;

	if (eventid_event.count(it2->second) == 0)
		return;
	std::map<int, struct event>::iterator it3 = eventid_event.find(it2->second);

	event tmp_event = it3->second;
	vars->setvalue("%EVENTNAME", tmp_event.event_name);
	vars->setvalue("%SHORTTEXT", tmp_event.event_short_text);
	vars->setvalue("%EXTENDEDTEXT", tmp_event.event_extended_text);
	vars->setvalue("%STARTTIME", tmp_event.starttime);
	vars->setvalue("%DURATION", tmp_event.duration);
	vars->setvalue("%EVENTID", tmp_event.eventid);
	vars->setvalue("%LINKAGE", tmp_event.number_perspectives);
	if (tmp_event.number_components > 0)
	{
		vars->setvalue("%AUDIO", tmp_event.audio_description[0]);
		act_schedcomponent = 0;
	}
	else
	{
		vars->setvalue("%AUDIO", "N/A");
		act_schedcomponent = -1;
	}
	vars->setvalue("%PARRATING", tmp_event.par_rating);
}

void eit::dumpNextSchedulingComponent()
{
	int eventid = atoi(vars->getvalue("%EVENTID").c_str());
	if (eventid_event.count(eventid) == 0)
		return;
	event tmp_event = eventid_event.find(eventid)->second;

	if (act_schedcomponent != -1)
	{
		act_schedcomponent++;
		if (act_schedcomponent >= tmp_event.number_components)
			act_schedcomponent = 0;
		vars->setvalue("%AUDIO", tmp_event.audio_description[act_schedcomponent]);
	}
}

void eit::dumpPrevSchedulingComponent()
{
	int eventid = atoi(vars->getvalue("%EVENTID").c_str());
	if (eventid_event.count(eventid) == 0)
		return;
	event tmp_event = eventid_event.find(eventid)->second;

	if (act_schedcomponent != -1)
	{
		act_schedcomponent--;
		if (act_schedcomponent < 0 )
			act_schedcomponent = tmp_event.number_components - 1;
		vars->setvalue("%AUDIO", tmp_event.audio_description[act_schedcomponent]);
	}
}

void eit::dumpNextNowComponent()
{
	if (act_nowcomponent != -1)
	{
		act_nowcomponent++;
		if (act_nowcomponent >= now.number_components)
			act_nowcomponent = 0;
		vars->setvalue("%NOWAUDIO", now.audio_description[act_nowcomponent]);
	}
}

void eit::dumpPrevNowComponent()
{
	if (act_nowcomponent != -1)
	{
		act_nowcomponent--;
		if (act_nowcomponent < 0)
			act_nowcomponent = now.number_components - 1;
		vars->setvalue("%NOWAUDIO", now.audio_description[act_nowcomponent]);
	}
}

void eit::dumpNextNextComponent()
{
	if (act_nextcomponent != -1)
	{
		act_nextcomponent++;
		if (act_nextcomponent >= next.number_components)
			act_nextcomponent = 0;
		vars->setvalue("%NEXTAUDIO", next.audio_description[act_nextcomponent]);
	}
}

void eit::dumpPrevNextComponent()
{
	if (act_nextcomponent != -1)
	{
		act_nextcomponent--;
		if (act_nextcomponent < 0)
			act_nextcomponent = next.number_components - 1;
		vars->setvalue("%NEXTAUDIO", next.audio_description[act_nextcomponent]);
	}
}
