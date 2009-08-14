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

#include <global.h>

#include "remotecontrol.h"
#include "neutrino.h"

CRemoteControl::CRemoteControl()
{
	current_channel_id = 	0;
	current_sub_channel_id = 0;
	current_channel_name = 	"";

	zap_completion_timeout = 0;

	current_EPGid =	0;
	next_EPGid = 	0;
	memset(&current_PIDs.PIDs, 0, sizeof(current_PIDs.PIDs) );
	has_ac3 = 	false;
	selected_subchannel = -1;
	needs_nvods = 	false;
	director_mode = 0;
	current_programm_timer = 0;
	is_video_started = true;
	zapCount = 	0;
}

int CRemoteControl::handleMsg(uint msg, uint data)
{
	if ( zap_completion_timeout != 0 )
	{
		// warte auf Meldung vom ZAPIT
    	if ( ( msg == NeutrinoMessages::EVT_ZAP_COMPLETE ) ||
    		 ( msg == NeutrinoMessages:: EVT_ZAP_FAILED ) ||
    		 ( msg == NeutrinoMessages:: EVT_ZAP_ISNVOD ) )
		{
			if ( data != current_channel_id )
			{
				g_Zapit->zapTo_serviceID_NOWAIT( current_channel_id );
				g_Sectionsd->setServiceChanged( current_channel_id, false );

				zap_completion_timeout = getcurrenttime() + 2 * (long long) 1000000;

				return messages_return::handled;
			}
			else
				zap_completion_timeout = 0;
		}
	}
	else
	{
        if ( ( msg == NeutrinoMessages::EVT_ZAP_COMPLETE ) ||
    		 ( msg == NeutrinoMessages:: EVT_ZAP_FAILED ) ||
    		 ( msg == NeutrinoMessages:: EVT_ZAP_ISNVOD ) )
    	{
    		// warte auf keine Meldung vom ZAPIT -> jemand anderer hat das zappen ausgelöst...
    		if ( data != current_channel_id )
    		{
    			current_channel_id = data;
				is_video_started= true;

				current_EPGid = 0;
				next_EPGid = 0;

				memset(&current_PIDs.PIDs, 0, sizeof(current_PIDs.PIDs) );

				current_PIDs.APIDs.clear();
				has_ac3 = false;

				subChannels.clear();
				selected_subchannel = -1;
				director_mode = 0;
				needs_nvods = ( msg == NeutrinoMessages:: EVT_ZAP_ISNVOD );

				g_Sectionsd->setServiceChanged( current_channel_id, true );
				CNeutrinoApp::getInstance()->channelList->adjustToChannelID(current_channel_id);
				if ( g_InfoViewer->is_visible )
					g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR , 0 );
			}
    	}
    	else
	    if ( ( msg == NeutrinoMessages::EVT_ZAP_SUB_COMPLETE ) ||
        	 ( msg == NeutrinoMessages:: EVT_ZAP_SUB_FAILED ) )
        {
    		if ( data != current_sub_channel_id )
    		{
				current_sub_channel_id = data;

				for( unsigned int i= 0; i< subChannels.size(); i++)
					if ((((uint32_t)subChannels[i].original_network_id << 16) | subChannels[i].service_id) == data )
					{
						selected_subchannel = i;
						break;
					}
			}
        }
    }

    if ( msg == NeutrinoMessages::EVT_CURRENTEPG )
	{
		CSectionsdClient::CurrentNextInfo* info_CN = (CSectionsdClient::CurrentNextInfo*) data;

		if ( ( info_CN->current_uniqueKey >> 16) == current_channel_id )
		{
			//CURRENT-EPG für den aktuellen Kanal bekommen!;

			if ( info_CN->current_uniqueKey != current_EPGid )
			{
			    if ( current_EPGid != 0 )
			    {
			    	// ist nur ein neues Programm, kein neuer Kanal

			    	// PIDs neu holen
			    	g_Zapit->getPIDS( current_PIDs );

			    	// APID Bearbeitung neu anstossen
			    	has_unresolved_ctags = true;
			    }

				current_EPGid= info_CN->current_uniqueKey;

				if ( has_unresolved_ctags )
					processAPIDnames();

				if ( info_CN->flags & CSectionsdClient::epgflags::current_has_linkagedescriptors )
					getSubChannels();

				if ( needs_nvods )
					getNVODs();

				if ( current_programm_timer != 0 )
					g_RCInput->killTimer( current_programm_timer );

				time_t end_program= info_CN->current_zeit.startzeit+ info_CN->current_zeit.dauer;
				current_programm_timer = g_RCInput->addTimer( &end_program );

				if ((!is_video_started) && (info_CN->current_fsk == 0))
					g_RCInput->postMsg( NeutrinoMessages::EVT_PROGRAMLOCKSTATUS, 0x100, false );
				else
					g_RCInput->postMsg( NeutrinoMessages::EVT_PROGRAMLOCKSTATUS, info_CN->current_fsk, false );
			}
		}
	    return messages_return::handled;
	}
	else if ( msg == NeutrinoMessages::EVT_NEXTEPG )
	{
		CSectionsdClient::CurrentNextInfo* info_CN = (CSectionsdClient::CurrentNextInfo*) data;

		if ( ( info_CN->next_uniqueKey >> 16) == current_channel_id )
		{
			// next-EPG für den aktuellen Kanal bekommen, current ist leider net da?!;
			if ( info_CN->next_uniqueKey != next_EPGid )
			{
			    next_EPGid= info_CN->next_uniqueKey;

				// timer setzen

	        	if ( current_programm_timer != 0 )
					g_RCInput->killTimer( current_programm_timer );

				time_t end_program= info_CN->next_zeit.startzeit;
				current_programm_timer = g_RCInput->addTimer( &end_program );
			}
		}
		if ( !is_video_started )
			g_RCInput->postMsg( NeutrinoMessages::EVT_PROGRAMLOCKSTATUS, 0x100, false );

	    return messages_return::handled;
	}
	else if ( msg == NeutrinoMessages::EVT_NOEPG_YET )
	{
		if ( data == current_channel_id )
		{
			if ( !is_video_started )
    			g_RCInput->postMsg( NeutrinoMessages::EVT_PROGRAMLOCKSTATUS, 0x100, false );
		}
		return messages_return::handled;
	}
	else if ( ( msg == NeutrinoMessages::EVT_ZAP_COMPLETE ) || ( msg == NeutrinoMessages:: EVT_ZAP_SUB_COMPLETE ) )
	{
		if ( data == (( msg == NeutrinoMessages::EVT_ZAP_COMPLETE )?current_channel_id:current_sub_channel_id) )
		{
			g_Zapit->getPIDS( current_PIDs );
			g_RCInput->postMsg( NeutrinoMessages::EVT_ZAP_GOTPIDS, current_channel_id, false );

			processAPIDnames();
		}
	    return messages_return::handled;
	}
	else if ( msg == NeutrinoMessages::EVT_ZAP_ISNVOD )
	{
		if ( data == current_channel_id )
		{
		    needs_nvods = true;

			if ( current_EPGid != 0)
			{
				getNVODs();
				if ( subChannels.size() == 0 )
					g_Sectionsd->setServiceChanged( current_channel_id, true );
			}
			else
				// EVENT anfordern!
				g_Sectionsd->setServiceChanged( current_channel_id, true );

		}
	    return messages_return::handled;
	}
	else if ( ( msg == NeutrinoMessages::EVT_TIMER ) && ( data == current_programm_timer ) )
	{
		//printf("new program !\n");
		g_RCInput->postMsg( NeutrinoMessages::EVT_NEXTPROGRAM, current_channel_id, false );

 		return messages_return::handled;
	}
	else
		return messages_return::unhandled;
}

void CRemoteControl::getSubChannels()
{
	if ( subChannels.size() == 0 )
	{
		CSectionsdClient::LinkageDescriptorList	linkedServices;
		if ( g_Sectionsd->getLinkageDescriptorsUniqueKey( current_EPGid, linkedServices ) )
		{
			if ( linkedServices.size()> 1 )
			{
				are_subchannels = true;
				for (unsigned int i=0; i< linkedServices.size(); i++)
				{
					subChannels.push_back(CSubService(linkedServices[i].serviceId,
									  linkedServices[i].transportStreamId,
									  linkedServices[i].originalNetworkId,
									  linkedServices[i].name));
				}
				copySubChannelsToZapit();
				g_RCInput->postMsg( NeutrinoMessages::EVT_ZAP_GOT_SUBSERVICES, current_channel_id, false );
			}
		}
	}
}

void CRemoteControl::getNVODs()
{
	if ( subChannels.size() == 0 )
	{
		CSectionsdClient::NVODTimesList	NVODs;
		if ( g_Sectionsd->getNVODTimesServiceKey( current_channel_id, NVODs ) )
		{
			are_subchannels = false;
			for (unsigned int i=0; i< NVODs.size(); i++)
			{
				if ( NVODs[i].zeit.dauer> 0 )
				{
					CSubService newService (NVODs[i].service_id,
								NVODs[i].transport_stream_id,
								NVODs[i].original_network_id,
								NVODs[i].zeit.startzeit, 
								NVODs[i].zeit.dauer);

					CSubServiceListSorted::iterator e= subChannels.begin();
					for(; e!=subChannels.end(); ++e)
					{
						if ( e->startzeit > newService.startzeit )
							break;
					}
					subChannels.insert( e, newService );
				}

			}

			copySubChannelsToZapit();
            g_RCInput->postMsg( NeutrinoMessages::EVT_ZAP_GOT_SUBSERVICES, current_channel_id, false );


            if ( selected_subchannel == -1 )
            {
            	// beim ersten Holen letzten NVOD-Kanal setzen!
				setSubChannel( subChannels.size()- 1 );
			}
			else
			{
				// sollte nur passieren, wenn die aktuelle Sendung vorbei ist?!
				selected_subchannel = -1;
			}
		}
	}
}

void CRemoteControl::processAPIDnames()
{
	has_unresolved_ctags= false;
	has_ac3 = false;

	for(unsigned int count=0; count< current_PIDs.APIDs.size(); count++)
	{
		if ( current_PIDs.APIDs[count].component_tag != 0xFF )
		{
			has_unresolved_ctags= true;
        }
		if ( strlen( current_PIDs.APIDs[count].desc ) == 3 )
		{
			// unaufgeloeste Sprache...
			strcpy( current_PIDs.APIDs[count].desc, getISO639Description( current_PIDs.APIDs[count].desc ) );
		}

		if ( current_PIDs.APIDs[count].is_ac3 )
		{
			strcat( current_PIDs.APIDs[count].desc, " (AC3)");
			has_ac3 = true;
		}
	}

	if ( has_unresolved_ctags )
	{
		if ( current_EPGid != 0 )
		{
			CSectionsdClient::ComponentTagList tags;
			if ( g_Sectionsd->getComponentTagsUniqueKey( current_EPGid, tags ) )
			{
				has_unresolved_ctags = false;
				has_ac3 = false;

				for (unsigned int i=0; i< tags.size(); i++)
				{
					for (unsigned int j=0; j< current_PIDs.APIDs.size(); j++)
					{
						if ( current_PIDs.APIDs[j].component_tag == tags[i].componentTag )
						{
							strncpy( current_PIDs.APIDs[j].desc, tags[i].component.c_str(), 25 );
							if ( current_PIDs.APIDs[j].is_ac3 )
								strncat( current_PIDs.APIDs[j].desc, " (AC3)", 25 );
							current_PIDs.APIDs[j].component_tag = -1;
							break;
						}
					}
				}

				CZapitClient::APIDList::iterator e = current_PIDs.APIDs.begin();
				while ( e != current_PIDs.APIDs.end() )
				{
					if ( e->is_ac3 )
					{
						if ( e->component_tag != -1 )
						{
							current_PIDs.APIDs.erase( e );
							continue;
						}
						else
							has_ac3 = true;
					}
					e++;
				}

				if ( g_settings.audio_DolbyDigital == 1)
				{
					for (unsigned int j=0; j< current_PIDs.APIDs.size(); j++)
						if ( current_PIDs.APIDs[j].is_ac3 )
						{
							setAPID( j );
							break;
						}
				}

				if ( current_PIDs.PIDs.selected_apid >= current_PIDs.APIDs.size() )
				{
                	setAPID( 0 );
				}
			}
		}
	}


	g_RCInput->postMsg( NeutrinoMessages::EVT_ZAP_GOTAPIDS, current_channel_id, false );
}


void CRemoteControl::copySubChannelsToZapit()
{
	CZapitClient::subServiceList 		zapitList;
	CZapitClient::commandAddSubServices	zapitSubChannel;

	for(CSubServiceListSorted::iterator e=subChannels.begin(); e!=subChannels.end(); ++e)
	{
		zapitSubChannel.original_network_id = e->original_network_id;
		zapitSubChannel.service_id          = e->service_id;
		zapitSubChannel.transport_stream_id = e->transport_stream_id;
		zapitList.push_back(zapitSubChannel);
	}
	g_Zapit->setSubServices( zapitList );
}


void CRemoteControl::setAPID( uint APID )
{
	if ((current_PIDs.PIDs.selected_apid == APID ) || (APID < 0) || (APID >= current_PIDs.APIDs.size()) )
		return;

	current_PIDs.PIDs.selected_apid = APID;
	g_Zapit->setAudioChannel( APID );
	#ifdef USEACTIONLOG
		char buf[1000];
		sprintf((char*) buf, "select audio: \"%s\"", current_PIDs.APIDs[APID].desc );
		g_ActionLog->println(buf);
	#endif
}

string CRemoteControl::setSubChannel(unsigned numSub, bool force_zap )
{
	if ((numSub < 0) || (numSub >= subChannels.size()))
		return "";

	if (( (uint) selected_subchannel == numSub ) && (!force_zap))
		return "";

	selected_subchannel = numSub;

	t_original_network_id original_network_id = subChannels[numSub].original_network_id;
	t_service_id          service_id          = subChannels[numSub].service_id;
	current_sub_channel_id                    = CREATE_CHANNEL_ID;

	g_Zapit->zapTo_subServiceID_NOWAIT( current_sub_channel_id );

	string perspectiveName = subChannels[numSub].subservice_name;

	#ifdef USEACTIONLOG
		char buf[1000];
		if(perspectiveName!="")
		{
			sprintf((char*) buf, "perspective change: \"%s\"", perspectiveName.c_str() );
			g_ActionLog->println(buf);
		}
		else
		{
			struct  tm *tmZeit;
			tmZeit= localtime( &subChannels[numSub].startzeit );
			sprintf((char*) buf, "select nvod: (%02d:%02d)", tmZeit->tm_hour, tmZeit->tm_min );
			g_ActionLog->println(buf);
		}
	#endif

	return perspectiveName;
}

string CRemoteControl::subChannelUp()
{
	return setSubChannel( (selected_subchannel + 1) % subChannels.size());
}

string CRemoteControl::subChannelDown()
{
	if (selected_subchannel == 0 )
	{
		return setSubChannel(subChannels.size() - 1);
	}
	else
	{
		return setSubChannel(selected_subchannel - 1);
	}
}

void CRemoteControl::zapTo_ChannelID(t_channel_id channel_id, string channame, bool start_video )
{
	current_channel_id = channel_id;
	current_channel_name = channame;
	is_video_started= start_video;

	current_sub_channel_id = 0;
	current_EPGid = 0;
	next_EPGid = 0;

	memset(&current_PIDs.PIDs, 0, sizeof(current_PIDs.PIDs) );

	current_PIDs.APIDs.clear();
	has_ac3 = false;

	subChannels.clear();
	selected_subchannel = -1;
	needs_nvods = false;
	director_mode = 0;
	zapCount++;

	#ifdef USEACTIONLOG
		if(channame!="")
		{
			char buf[1000];
			sprintf((char*) buf, "zapto: %08x \"%s\"", channel_id, channame.c_str() );
			g_ActionLog->println(buf);
		}
	#endif

	unsigned long long now = getcurrenttime();
	if ( zap_completion_timeout < now )
	{
		g_Zapit->zapTo_serviceID_NOWAIT(channel_id);
		g_Sectionsd->setServiceChanged( current_channel_id, false );

		zap_completion_timeout = now + 2 * (long long) 1000000;
		if ( current_programm_timer != 0 )
		{
			g_RCInput->killTimer( current_programm_timer );
			current_programm_timer = 0;
		}
	}
}


void CRemoteControl::startvideo()
{
	if ( !is_video_started )
	{
		is_video_started= true;
		g_Zapit->startPlayBack();
	}
}

void CRemoteControl::stopvideo()
{
	if ( is_video_started )
	{
		is_video_started= false;
		g_Zapit->stopPlayBack();
	}
}

void CRemoteControl::radioMode()
{
	g_Zapit->setMode( CZapitClient::MODE_RADIO );
}

void CRemoteControl::tvMode()
{
	g_Zapit->setMode( CZapitClient::MODE_TV );
}
