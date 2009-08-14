/***************************************************************************
    copyright            : (C) 2001 by TheDOC
    email                : thedoc@chatville.de
	homepage			 : www.chatville.de
	modified by			 : -
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
$Log: plugins.h,v $
Revision 1.9.2.1.2.4  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.9.2.1.2.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.10  2004/06/16 08:46:45  thegoodguy
fix compilation (untested)

Revision 1.9  2003/01/05 19:52:47  TheDOC
forgot include

Revision 1.8  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.7  2002/05/20 20:08:12  TheDOC
some new timer and epg-stuff

Revision 1.6  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.5  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.4  2001/12/19 04:48:37  tux
Neue Plugin-Schnittstelle

Revision 1.3  2001/12/17 01:00:34  tux
scan.cpp fix

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef PLUGINS_H
#define PLUGINS_H

#include <string>
#include <dirent.h>
#include <vector>
#include <map>
#include <dlfcn.h> 
#include <sstream>
#include <sstream>
#include <fstream>
#include <iostream>

#include <plugin.h>

#include <config.h>
#include "devices.h"

class plugins
{
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
		bool vformat;
		bool offsets;

	};

	int fb, rc, lcd, pid;
	int number_of_plugins;
	std::string plugin_dir;
	std::vector<struct plugin> plugin_list;

	void parseCfg(plugin *plugin_data);

	std::map<std::string, std::string> params;
public:
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

	void startPlugin(int number);
};

#endif
