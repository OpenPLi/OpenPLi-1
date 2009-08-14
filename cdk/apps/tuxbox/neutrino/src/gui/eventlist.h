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


#ifndef __EVENTLIST_HPP__
#define __EVENTLIST_HPP__

#include <string>
#include <vector>

#include <sectionsdclient/sectionsdclient.h>

#include <daemonc/remotecontrol.h>
#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <system/settings.h>

#include "color.h"
#include "infoviewer.h"

#include "widget/menue.h"


using namespace std;

class EventList
{
	private:
		CFrameBuffer	*frameBuffer;
        CChannelEventList	evtlist;
		void readEvents(const t_channel_id channel_id, const std::string& channelname); // I really don't like handling names
		unsigned int	selected;
		unsigned int	current_event;
		unsigned int	liststart;
		unsigned int	listmaxshow;
		unsigned int	numwidth;
		int		fheight; // Fonthoehe Channellist-Inhalt
		int		fheight1,fheight2;
		int		fwidth1,fwidth2;
		int		theight; // Fonthoehe Channellist-Titel

		int		key;
		string		name;

		int 		width;
		int 		height;
		int 		x;
		int 		y;

		void paintItem(unsigned pos);
		void paint();
		void paintHead();
		void hide();

	public:
		EventList();
		~EventList();
		int exec(const t_channel_id channel_id, const string& channelname);
};


#endif
