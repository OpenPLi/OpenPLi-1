/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

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


#ifndef __progresswindow__
#define __progresswindow__

#include <string>

#include <driver/framebuffer.h>

#include "progressstatus.h"
#include "menue.h"


using namespace std;

class CProgressWindow : public CMenuTarget, public CProgress_StatusViewer
{
	protected:

		CFrameBuffer	*frameBuffer;
		string		caption;

		int x;
		int y;
		int width;
		int height;
		int hheight; // head font height
		int mheight; // menu font height
		int globalstatus; 
		int globalstatusX;
		int globalstatusY;
		int localstatusY;
		int statusTextY;
		string statusText;


		//----------------------------

		virtual void paint();

	public:

		CProgressWindow();
		void setTitle( string title );
		virtual void hide();

		virtual int exec( CMenuTarget* parent, string actionKey );

		virtual void showGlobalStatus(int prog);
		virtual int getGlobalStatus();
		virtual void showLocalStatus(int prog);
		virtual void showStatusMessage(string text);

};


#endif
