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


#ifndef __bouquetlist__
#define __bouquetlist__

#include <string>
#include <vector>

#include <driver/framebuffer.h>
#include <system/lastchannel.h>

#include "channellist.h"


using namespace std;

typedef enum bouquetSwitchMode
{
    bsmBouquets,	// pressing OK shows list of all Bouquets
    bsmChannels,	// pressing OK shows list of all channels of active bouquets
    bsmAllChannels	// OK shows lsit of all channels
} BouquetSwitchMode;

class CBouquet
{

	public:
		int				unique_key;
		bool			bLocked;
		CChannelList*	channelList;

		CBouquet( int Unique_key=-1, const std::string& Name="", bool locked=false)
		{
			unique_key = Unique_key;
			bLocked = locked;
			channelList = new CChannelList( Name );
		}

		~CBouquet()
		{
			delete channelList;
		}
};


class CBouquetList
{
	private:
		CFrameBuffer		*frameBuffer;

		unsigned int		selected;
		unsigned int		tuned;
		unsigned int		liststart;
		unsigned int		listmaxshow;
		unsigned int		numwidth;
		unsigned int		maxpos;
		int					fheight; // Fonthoehe Bouquetlist-Inhalt
		int					theight; // Fonthoehe Bouquetlist-Titel

		string				name;

		int		width;
		int		height;
		int		x;
		int		y;

		void paintItem(int pos);
		void paint();
		void paintHead();
		void hide();

	public:
		CBouquetList( const std::string& Name="" );
		~CBouquetList();

		vector<CBouquet*>	Bouquets;

		CChannelList* orgChannelList;
		CBouquet* addBouquet(const std::string& name, int BouquetKey=-1, bool locked=false );
		int getActiveBouquetNumber();
		int activateBouquet( int id, bool bShowChannelList = false);
		int show();
		int showChannelList( int nBouquet = -1);
		void adjustToChannel( int nChannelNr);
		int exec( bool bShowChannelList);
};


#endif
