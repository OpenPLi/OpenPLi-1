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


#ifndef __update__
#define __update__

#include <string>

#include <driver/framebuffer.h>

#include "widget/progressstatus.h"
#include "widget/progresswindow.h"
#include "widget/menue.h"


using namespace std;

class CFlashUpdate : public CProgressWindow
{
	private:
		string	BasePath;
		string	ImageFile;
		string	VersionFile;

		string	installedVersion;
		string	newVersion;

		bool getInfo();
		bool getUpdateImage(string version);
		bool checkVersion4Update();

	public:
		CFlashUpdate();
		int exec( CMenuTarget* parent, string actionKey );

};

class CFlashExpert : public CProgressWindow
{
	private:
		int selectedMTD;

		void showMTDSelector(string actionkey);
		void showFileSelector(string actionkey);

		void readmtd(int readmtd);
		void writemtd(string filename, int mtdNumber);

	public:
		CFlashExpert();
		int exec( CMenuTarget* parent, string actionKey );

};


#endif
