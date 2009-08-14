/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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


#ifndef __remotecontrol__
#define __remotecontrol__

#include <vector>
#include <set>
#include <string>

#include <zapit/client/zapitclient.h>

#include <sectionsdclient/sectionsdMsg.h>


using namespace std;

struct st_rmsg
{
	unsigned char version;
	unsigned char cmd;
	unsigned char param;
	unsigned short param2;
	char param3[30];
};

class CSubService
{
public:
	CSubService(const t_service_id &aservice_id, const t_transport_stream_id &atransport_stream_id, const t_original_network_id &aoriginal_network_id, const string &asubservice_name)
	{
		original_network_id = aoriginal_network_id;
		service_id          = aservice_id;
		transport_stream_id = atransport_stream_id;
		startzeit = 0;
		dauer =     0;
		subservice_name= asubservice_name;
	}
	CSubService(const t_service_id &aservice_id, const t_transport_stream_id &atransport_stream_id, const t_original_network_id &aoriginal_network_id, const time_t &astartzeit, const unsigned adauer)
	{
		original_network_id = aoriginal_network_id;
		service_id          = aservice_id;
		transport_stream_id = atransport_stream_id;
		startzeit=astartzeit;
		dauer=adauer;
		subservice_name= "";
	}

	t_service_id          service_id;
	t_transport_stream_id transport_stream_id;
	t_original_network_id original_network_id;
	time_t                startzeit;
	unsigned              dauer;
	string                subservice_name;
};

typedef std::vector<CSubService> CSubServiceListSorted;

class CRemoteControl
{
	unsigned int            current_programm_timer;
	unsigned long long         zap_completion_timeout;

	void getNVODs();
	void getSubChannels();
	void copySubChannelsToZapit();

public:
	string               current_channel_name;
	t_channel_id            current_channel_id;
	t_channel_id            current_sub_channel_id;
	unsigned long long         current_EPGid;
	unsigned long long         next_EPGid;
	CZapitClient::responseGetPIDs    current_PIDs;

	// APID - Details
	bool              has_ac3;
	bool              has_unresolved_ctags;

	// SubChannel/NVOD - Details
	CSubServiceListSorted         subChannels;
	int               selected_subchannel;
	bool                             are_subchannels;
	bool              needs_nvods;
	int               director_mode;

	// Video / Parental-Lock
	bool              is_video_started;
	unsigned int            zapCount;

	CRemoteControl();
	void zapTo_ChannelID(const t_channel_id channel_id, string channame, bool start_video = true );
	void startvideo();
	void stopvideo();
	void queryAPIDs();
	void setAPID(uint APID);
	void processAPIDnames();
	string setSubChannel(unsigned numSub, bool force_zap = false );
	string subChannelUp();
	string subChannelDown();

	void radioMode();
	void tvMode();

	int handleMsg(uint msg, uint data);
};


#endif
