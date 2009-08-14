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


#ifndef __hintbox__
#define __hintbox__

#include <string>
#include <vector>

#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>

#include "menue.h"


using namespace std;

class CHintBox
{
	private:
		CFrameBuffer			*frameBuffer;
		int				width;
		int				height;
		int				x;
		int				y;
		int				fheight;
		int				theight;
		string				caption;
		vector<string>			text;
		string				iconfile;

		unsigned char*			pixbuf;


	public:
		CHintBox( string Caption, string Text, string Icon="info.raw", int Width = 500 );

		void paint( bool saveScreen = true );
		void hide();
};

int ShowHint ( string Caption, string Text, string Icon="info.raw", int Width = 450, int timeout = -1 );


#endif
