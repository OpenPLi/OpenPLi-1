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


#ifndef __infoview__
#define __infoview__

#include <sectionsdclient/sectionsdclient.h>

#include <driver/rcinput.h>
#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>
#include <system/settings.h>

#include "gui/color.h"


using namespace std;

class CInfoViewer
{
	private:
		CFrameBuffer	*frameBuffer;

		bool		gotTime;
		bool		recordModeActive;

		int		InfoHeightY;
		int		InfoHeightY_Info;
		bool		showButtonBar;

		int		BoxEndX;
		int		BoxEndY;
		int		BoxStartX;
		int		BoxStartY;
		int		ButtonWidth;

		int		ChanWidth;
		int		ChanHeight;
		int		ChanInfoX;

		string		CurrentChannel;
		CSectionsdClient::CurrentNextInfo	info_CurrentNext;
        	t_channel_id    channel_id;

		char aspectRatio;

		uint	sec_timer_id;
		uint 	fadeTimer;

		void show_Data( bool calledFromEvent = false );
		void paintTime( bool show_dot, bool firstPaint );


		void showButton_Audio();
		void showButton_SubServices();

		void showIcon_16_9();
		void showIcon_VTXT();
		void showRecordIcon( bool show );

		void showFailure();
	public:

		bool	is_visible;

		CInfoViewer();

		void start();

		void showTitle( int ChanNum, string Channel, const t_channel_id new_channel_id = 0, bool calledFromNumZap = false );
		void killTitle();
		CSectionsdClient::CurrentNextInfo getEPG(const t_channel_id for_channel_id);

		void showSubchan();

		int handleMsg(uint msg, uint data);
};


#endif
