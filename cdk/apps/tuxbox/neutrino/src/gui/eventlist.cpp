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
#include <neutrino.h>

#include "eventlist.h"


EventList::EventList()
{
	frameBuffer = CFrameBuffer::getInstance();
	selected = 0;
	current_event = 0;

	width  = 580;
	//height = 440;
	height = 480;
	theight  = g_Fonts->eventlist_title->getHeight();
	fheight1 = g_Fonts->eventlist_itemLarge->getHeight();
	{
		int h1 = g_Fonts->eventlist_itemSmall->getHeight();
		int h2 = g_Fonts->eventlist_datetime->getHeight();
		fheight2 = (h1 > h2) ? h1 : h2;
	}
	fheight = fheight1 + fheight2 + 2;
	fwidth1 = g_Fonts->eventlist_datetime->getRenderWidth("DDD, 00:00,  ");
	fwidth2 = g_Fonts->eventlist_itemSmall->getRenderWidth("[999 min] ");


	listmaxshow = (height-theight-0)/fheight;
	height = theight+0+listmaxshow*fheight; // recalc height
	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-height) / 2) + g_settings.screen_StartY;
	liststart = 0;
}


EventList::~EventList()
{
}


void EventList::readEvents(const t_channel_id channel_id, const std::string& channelname)
{
	current_event = (unsigned int)-1;
	evtlist = g_Sectionsd->getEventsServiceKey(channel_id);
    time_t azeit=time(NULL);

	for ( CChannelEventList::iterator e= evtlist.begin(); e != evtlist.end(); ++e )
	{
    	if ( e->startTime > azeit )
    		break;
    	current_event++;
	}

	if ( evtlist.size() == 0 )
	{
		CChannelEvent evt;

		evt.description= g_Locale->getText("epglist.noevents") ;
		evt.eventID = 0;
		evtlist.insert(evtlist.end(), evt);

	}
	if (current_event == (unsigned int)-1)
		current_event = 0;
	selected= current_event;

	return;
}


int EventList::exec(const t_channel_id channel_id, const std::string& channelname)
{
	int res = menu_return::RETURN_REPAINT;

	name = channelname;
	paintHead();
	readEvents(channel_id, channelname);
	paint();

	int oldselected = selected;

	#ifdef USEACTIONLOG
		char buf[1000];
		sprintf((char*) buf, "epg-Eventlist: %08x \"%s\"", channel_id, channelname.c_str() );
		g_ActionLog->println(buf);
	#endif

	uint msg; uint data;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_chanlist );

	bool loop=true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_chanlist );

		if ( msg == (uint) g_settings.key_channelList_pageup )
		{
			selected+=listmaxshow;
			if (selected>evtlist.size()-1)
				selected=0;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if ( msg == (uint) g_settings.key_channelList_pagedown )
		{
			if ((int(selected)-int(listmaxshow))<0)
				selected=evtlist.size()-1;
			else
				selected -= listmaxshow;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if ( msg == CRCInput::RC_up )
		{
			int prevselected=selected;
			if(selected==0)
			{
				selected = evtlist.size()-1;
			}
			else
				selected--;
			paintItem(prevselected - liststart);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;
			if(oldliststart!=liststart)
			{
				paint();
			}
			else
			{
				paintItem(selected - liststart);
			}
		}
		else if ( msg == CRCInput::RC_down )
		{
			int prevselected=selected;
			selected = (selected+1)%evtlist.size();
			paintItem(prevselected - liststart);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;
			if(oldliststart!=liststart)
			{
				paint();
			}
			else
			{
				paintItem(selected - liststart);
			}
		}
		else if ( ( msg == CRCInput::RC_timeout ) ||
			 	  ( msg == (uint) g_settings.key_channelList_cancel ) )
		{
			selected = oldselected;
			loop=false;
		}

		else if ( ( msg == CRCInput::RC_ok ) ||
				  ( msg == CRCInput::RC_left ) ||
				  ( msg == CRCInput::RC_red ) )
		{
			loop= false;
		}
		else if (msg==CRCInput::RC_help || msg==CRCInput::RC_right)
		{
			if ( evtlist[selected].eventID != 0 )
			{
				hide();

				res = g_EpgData->show(channel_id, evtlist[selected].eventID, &evtlist[selected].startTime);
                if ( res == menu_return::RETURN_EXIT_ALL )
                {
                	loop = false;
                }
                else
                {
                	g_RCInput->getMsg( &msg, &data, 0 );

					if ( ( msg != CRCInput::RC_red ) &&
				         ( msg != CRCInput::RC_timeout ) )
					{
						// RC_red schlucken
						g_RCInput->postMsg( msg, data );
					}

					paintHead();
					paint();
				}
			}
		}
		else
		{
			if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
			{
				loop = false;
				res = menu_return::RETURN_EXIT_ALL;
			}
		}
	}

	hide();

	#ifdef USEACTIONLOG
		g_ActionLog->println("epg-Eventlist: closed");
	#endif

	return res;
}

void EventList::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}

void EventList::paintItem(unsigned int pos)
{
	int color;
	int ypos = y+ theight+0 + pos*fheight;
	string datetime1_str, datetime2_str, duration_str;

	if (liststart+pos==selected)
	{
		color = COL_MENUCONTENTSELECTED;
	}
	else if (liststart+pos == current_event )
	{
		color = COL_MENUCONTENT+ 1; //COL_MENUCONTENTINACTIVE+ 4;
	}
	else
	{
		color = COL_MENUCONTENT;
	}

	frameBuffer->paintBoxRel(x,ypos, width- 15, fheight, color);

	if(liststart+pos<evtlist.size())
	{
		if ( evtlist[liststart+pos].eventID != 0 )
		{
			char tmpstr[256];
			struct tm *tmStartZeit = localtime(&evtlist[liststart+pos].startTime);


			strftime(tmpstr, sizeof(tmpstr), "date.%a", tmStartZeit );
			datetime1_str = std::string( g_Locale->getText(tmpstr) );

			strftime(tmpstr, sizeof(tmpstr), ". %H:%M, ", tmStartZeit );
			datetime1_str += std::string( tmpstr );

			strftime(tmpstr, sizeof(tmpstr), " %d. ", tmStartZeit );
			datetime2_str = std::string( tmpstr );
			strftime(tmpstr,sizeof(tmpstr), "date.%b", tmStartZeit );
			datetime2_str += std::string( g_Locale->getText(tmpstr) );
			datetime2_str += std::string(".");

        	sprintf(tmpstr, "[%d min]", evtlist[liststart+pos].duration / 60 );
        	duration_str = std::string( tmpstr );
        }

		// 1st line
		g_Fonts->eventlist_datetime->RenderString(x+5,         ypos+ fheight1+3, fwidth1+5,
		        datetime1_str.c_str(), color);
		g_Fonts->eventlist_datetime->RenderString(x+5+fwidth1, ypos+ fheight1+3, width-fwidth1-10- 20,
		        datetime2_str.c_str(), color);

		int seit = ( evtlist[liststart+pos].startTime - time(NULL) ) / 60;
		if ( (seit> 0) && (seit<100) && (duration_str.length()!=0) )
		{
			char beginnt[100];
			sprintf((char*) &beginnt, "in %d min", seit);
			int w= g_Fonts->eventlist_itemSmall->getRenderWidth(beginnt) + 10;

			g_Fonts->eventlist_itemSmall->RenderString(x+width-fwidth2-5- 20- w, ypos+ fheight1+3, fwidth2, beginnt, color);
		}
		g_Fonts->eventlist_itemSmall->RenderString(x+width-fwidth2-5- 20, ypos+ fheight1+3, fwidth2,
		        duration_str.c_str(), color);
		// 2nd line
		g_Fonts->eventlist_itemLarge->RenderString(x+ 20, ypos+ fheight, width- 25- 20,
		        evtlist[liststart+pos].description.c_str(), color);
	}
}

void EventList::paintHead()
{
	char l_name[100];
	snprintf(l_name, sizeof(l_name), g_Locale->getText("epglist.head").c_str(), name.c_str() );

	frameBuffer->paintBoxRel(x,y, width,theight+0, COL_MENUHEAD);
	g_Fonts->eventlist_title->RenderString(x+10,y+theight+1, width, l_name, COL_MENUHEAD);

}

void EventList::paint()
{
	liststart = (selected/listmaxshow)*listmaxshow;

	if (evtlist[0].eventID != 0)
		frameBuffer->paintIcon("help.raw", x+ width- 30, y+ 5 );

	for(unsigned int count=0;count<listmaxshow;count++)
	{
		paintItem(count);
	}

	int ypos = y+ theight;
	int sb = fheight* listmaxshow;
	frameBuffer->paintBoxRel(x+ width- 15,ypos, 15, sb,  COL_MENUCONTENT+ 1);

	int sbc= ((evtlist.size()- 1)/ listmaxshow)+ 1;
	float sbh= (sb- 4)/ sbc;
	int sbs= (selected/listmaxshow);

	frameBuffer->paintBoxRel(x+ width- 13, ypos+ 2+ int(sbs* sbh) , 11, int(sbh),  COL_MENUCONTENT+ 3);
}
