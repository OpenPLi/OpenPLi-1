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

#ifndef __gamelist__
#define __gamelist__

#include <string>
#include <vector>
#include <map>

#include <plugin.h>

#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <daemonc/remotecontrol.h>
#include <system/settings.h>

#include "color.h"
#include "widget/menue.h"


using namespace std;

class CPlugins
{
	private:

		CFrameBuffer	*frameBuffer;

		struct plugin
		{
			std::string filename;
			std::string cfgfile;
			std::string sofile;
			int version;
			std::string name;
			std::string description;
			std::string depend;
			int type;

			bool fb;
			bool rc;
			bool lcd;
			bool vtxtpid;
			int posx, posy, sizex, sizey;
			bool showpig;
			bool needoffset;
		};

		int fb, rc, lcd, pid;
		int number_of_plugins;
		std::string plugin_dir;
		std::vector<struct plugin> plugin_list;

		void parseCfg(plugin *plugin_data);

		std::map<std::string, std::string> params;
	public:

		~CPlugins();

		void loadPlugins();

		void setPluginDir(std::string dir) { plugin_dir = dir; }

		PluginParam* makeParam(const char * const id, PluginParam *next);

		void addParm(std::string cmd, int value);
		void addParm(std::string cmd, std::string value);

		void setfb(int fd);
		void setrc(int fd);
		void setlcd(int fd);
		void setvtxtpid(int fd);

		int getNumberOfPlugins() { return plugin_list.size(); }
		std::string getName(int number) { return plugin_list[number].name; }
		std::string getDescription(int number) { return plugin_list[number].description; }
		int getVTXT(int number) { return plugin_list[number].vtxtpid; }
		int getShowPig(int number) { return plugin_list[number].showpig; }
		int getPosX(int number) { return plugin_list[number].posx; }
		int getPosY(int number) { return plugin_list[number].posy; }
		int getSizeX(int number) { return plugin_list[number].sizex; }
		int getSizeY(int number) { return plugin_list[number].sizey; }
		int getType(int number) { return plugin_list[number].type; }

		void startPlugin(int number);
};


class CGameList : public CMenuTarget
{

	private:

		CFrameBuffer	*frameBuffer;

		struct game
		{
			int	number;
			string	name;
			string	desc;
		};

		unsigned int	liststart;
		unsigned int	listmaxshow;
		unsigned int	selected;
		int		key;
		string		name;
		vector<game*>   gamelist;


		int		fheight; // Fonthoehe Channellist-Inhalt
		int		theight; // Fonthoehe Channellist-Titel

		int		fheight1,fheight2;

		int 		width;
		int 		height;
		int 		x;
		int 		y;

		void paintItem(int pos);
		void paint();
		void paintHead();

	public:

		CGameList( string Name );
		~CGameList();

		void hide();
		int exec(CMenuTarget* parent, string actionKey);
		void runGame(int selected );
};


#endif
