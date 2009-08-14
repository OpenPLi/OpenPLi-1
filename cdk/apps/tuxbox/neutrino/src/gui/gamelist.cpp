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


#include <sstream>
#include <fstream>
#include <iostream>

#include <dirent.h>
#include <dlfcn.h>
#include <config.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <dbox/avia_gt_vbi.h>

#include <global.h>
#include <neutrino.h>

#include "gamelist.h"


void CPlugins::loadPlugins()
{
	frameBuffer = CFrameBuffer::getInstance();
	//printf("[CPlugins] Checking plugins-directory\n");
	printf("[CPlugins] Dir: %s\n", PLUGINDIR "/");

	struct dirent **namelist;

	int number_of_files = scandir(PLUGINDIR, &namelist, 0, alphasort);

	number_of_plugins = 0;
	plugin_list.clear();
	for (int i = 0; i < number_of_files; i++)
	{
		std::string filename;

		filename = namelist[i]->d_name;
		int pos = filename.find(".cfg");
		if (pos > -1)
		{
			number_of_plugins++;

			plugin new_plugin;
			new_plugin.filename = filename.substr(0, pos);
			std::string fname = PLUGINDIR "/";
			new_plugin.cfgfile = fname.append(new_plugin.filename);
			new_plugin.cfgfile.append(".cfg");
			fname = PLUGINDIR "/";
			new_plugin.sofile = fname.append(new_plugin.filename);
			new_plugin.sofile.append(".so");

			parseCfg(&new_plugin);

			plugin_list.insert(plugin_list.end(), new_plugin);
		}
	}
	printf("[CPlugins] %d plugins found...\n", number_of_plugins);

}

CPlugins::~CPlugins()
{
	plugin_list.clear();
}

void CPlugins::addParm(std::string cmd, std::string value)
{
	params[cmd] = value;
}

void CPlugins::addParm(std::string cmd, int value)
{
	char aval[10];
	sprintf( aval, "%d", value );

	addParm(cmd, aval);
}

void CPlugins::setfb(int fd)
{
	addParm(P_ID_FBUFFER, fd);
}

void CPlugins::setrc(int fd)
{
	addParm(P_ID_RCINPUT, fd);
}

void CPlugins::setlcd(int fd)
{
	addParm(P_ID_LCD, fd);
}

void CPlugins::setvtxtpid(int fd)
{
	addParm(P_ID_VTXTPID, fd);
}

void CPlugins::parseCfg(plugin *plugin_data)
{
//	FILE *fd;

	std::ifstream inFile;
	std::string line[20];
	int linecount = 0;

	inFile.open(plugin_data->cfgfile.c_str());

	while(linecount < 20 && getline(inFile, line[linecount++]));

	plugin_data->fb = false;
	plugin_data->rc = false;
	plugin_data->lcd = false;
	plugin_data->vtxtpid = false;
	plugin_data->showpig = false;
	plugin_data->needoffset = false;
	plugin_data->type = 1;

	for (int i = 0; i < linecount; i++)
	{
		std::istringstream iss(line[i]);
		std::string cmd;
		std::string parm;

		getline(iss, cmd, '=');
		getline(iss, parm, '=');

		if (cmd == "pluginversion")
		{
			plugin_data->version = atoi(parm.c_str());
		}
		else if (cmd == "name")
		{
			plugin_data->name = parm;
		}
		else if (cmd == "desc")
		{
			plugin_data->description = parm;
		}
		else if (cmd == "depend")
		{
			plugin_data->depend = parm;
		}
		else if (cmd == "type")
		{
			plugin_data->type = atoi(parm.c_str());
		}
		else if (cmd == "needfb")
		{
			plugin_data->fb = ((parm == "1")?true:false);
		}
		else if (cmd == "needrc")
		{
			plugin_data->rc = ((parm == "1")?true:false);
		}
		else if (cmd == "needlcd")
		{
			plugin_data->lcd = ((parm == "1")?true:false);
		}
		else if (cmd == "needvtxtpid")
		{
			plugin_data->vtxtpid = ((parm == "1")?true:false);
		}
		else if (cmd == "pigon")
		{
			plugin_data->showpig = ((parm == "1")?true:false);
		}
		else if (cmd == "needoffsets")
		{
			plugin_data->needoffset = ((parm == "1")?true:false);
		}
	}

	inFile.close();
}

PluginParam* CPlugins::makeParam(const char * const id, PluginParam *next)
{
	// cout << "Adding " << id << " With Value " << params.find(id)->second.c_str() << " and next: " << (int) next << endl;

	PluginParam *startparam = new PluginParam;
	startparam->next = next;
	startparam->id   = strdup(id   );
	startparam->val = new char[params.find(id)->second.length() + 2];
	strcpy(startparam->val, params.find(id)->second.c_str());

	// cout << "Startparam: " << (int) startparam << endl;
	return startparam;
}

void CPlugins::startPlugin(int number)
{
	PluginExec execPlugin;
	char depstring[129];
	char			*argv[20];
	void			*libhandle[20];
	int			argc, i, lcd_fd=-1;
	char			*p;
	char			*np;
	void			*handle;
	char			*error;

	PluginParam *startparam;
	PluginParam *tmpparam;

	startparam = 0;
	tmpparam = startparam;

	setfb( frameBuffer->getFileHandle() );

	if (plugin_list[number].fb)
	{
		// cout << "With FB " << params.find(P_ID_FBUFFER)->second.c_str() <<endl;
		startparam = makeParam(P_ID_FBUFFER, startparam);
		//cout << "New Startparam: " << startparam << endl;
		//cout << "New Tmpparam: " << tmpparam << endl;
	}
	if (plugin_list[number].rc)
	{
		setrc( g_RCInput->getFileHandle() );
		addParm(P_ID_RCBLK_ANF, g_settings.repeat_genericblocker);
		addParm(P_ID_RCBLK_REP, g_settings.repeat_blocker);

		// cout << "With RC " << params.find(P_ID_RCINPUT)->second.c_str() << endl;
		startparam = makeParam(P_ID_RCINPUT, startparam);
		startparam = makeParam(P_ID_RCBLK_ANF, startparam);
		startparam = makeParam(P_ID_RCBLK_REP, startparam);
	}
	else
	{
		g_RCInput->stopInput();
	}

	if (plugin_list[number].lcd)
	{
		// cout << "With LCD " << endl;
		g_lcdd->pause();
		if( (lcd_fd=open("/dev/dbox/lcd0", O_RDWR)) < 0 )
			setlcd( 0 );
		else
			setlcd( lcd_fd );

		startparam = makeParam(P_ID_LCD, startparam);
	}
	if (plugin_list[number].vtxtpid)
	{
		// cout << "With VTXTPID " << params.find(P_ID_VTXTPID)->second.c_str() << endl;
		// versuche, den gtx/enx_vbi zu stoppen
#if 0
        int fd = open("/dev/dbox/vbi0", O_RDWR);
		if (fd > 0)
		{
			ioctl(fd, AVIA_VBI_STOP_VTXT, 0);
			close(fd);
		}
#endif
		startparam = makeParam(P_ID_VTXTPID, startparam);
	}
	if (plugin_list[number].needoffset)
	{
		addParm(P_ID_VFORMAT, g_settings.video_Format);
		addParm(P_ID_OFF_X, g_settings.screen_StartX);
		addParm(P_ID_OFF_Y, g_settings.screen_StartY);
		addParm(P_ID_END_X, g_settings.screen_EndX);
		addParm(P_ID_END_Y, g_settings.screen_EndY);

		// cout << "With OFFSETS " << params.find(P_ID_OFF_X)->second.c_str() << ":" << params.find(P_ID_OFF_Y)->second.c_str() << endl;

		startparam = makeParam(P_ID_VFORMAT, startparam);
		startparam = makeParam(P_ID_OFF_X, startparam);
		startparam = makeParam(P_ID_OFF_Y, startparam);
		startparam = makeParam(P_ID_END_X, startparam);
		startparam = makeParam(P_ID_END_Y, startparam);
	}

	PluginParam *par = startparam;
	for( ; par; par=par->next )
	{
		// printf ("id: %s - val: %s\n", par->id, par->val);
		// printf("%d\n", par->next);
	}

	std::string pluginname = plugin_list[number].filename;

	strcpy(depstring, plugin_list[number].depend.c_str());

	argc=0;
	if ( depstring[0] )
	{
		p=depstring;
		while( 1 )
		{
			argv[ argc ] = p;
			argc++;
			np = strchr(p,',');
			if ( !np )
				break;

			*np=0;
			p=np+1;
			if ( argc == 20 )	// mehr nicht !
				break;
		}
	}

	for( i=0; i<argc; i++ )
	{
		std::string libname = argv[i];
		printf("[CPlugins] try load shared lib : %s\n",argv[i]);
		libhandle[i] = dlopen ( *argv[i] == '/' ?
			argv[i] : (PLUGINDIR "/"+libname).c_str(),
			RTLD_NOW | RTLD_GLOBAL );
		if ( !libhandle )
		{
			fputs (dlerror(), stderr);
			break;
		}
	}
	while ( i == argc )		// alles geladen
	{
		handle = dlopen ( plugin_list[number].sofile.c_str(), RTLD_NOW);
		if (!handle)
		{
			fputs (dlerror(), stderr);
			//should unload libs!
			break;
		}
		execPlugin = (PluginExec) dlsym(handle, "plugin_exec");
		if ((error = dlerror()) != NULL)
		{
			fputs(error, stderr);
			dlclose(handle);
			//should unload libs!
			break;
		}
		printf("[CPlugins] try exec...\n");
		execPlugin(startparam);
		dlclose(handle);
		printf("[CPlugins] exec done...\n");
		//restore framebuffer...


		if (!plugin_list[number].rc)
		{
    		g_RCInput->restartInput();
    		g_RCInput->clearRCMsg();
    	}

	if (plugin_list[number].lcd)
	{
		if(lcd_fd != -1)
			close(lcd_fd);
		g_lcdd->resume();
	}

    	if (plugin_list[number].fb)
    	{
    		frameBuffer->paletteSet();
    		frameBuffer->paintBackgroundBox(0,0,720,576);
    	}

#if 0
    	if (plugin_list[number].vtxtpid)
    	{
    		int vtpid= atoi(params.find(P_ID_VTXTPID)->second.c_str());
    		if (vtpid!=0)
    		{
    			// versuche, den gtx/enx_vbi wieder zu starten
        		int fd = open("/dev/dbox/vbi0", O_RDWR);
				if (fd > 0)
				{
					ioctl(fd, AVIA_VBI_START_VTXT, vtpid);
					close(fd);
				}
			}
		}
#endif
		//redraw menue...
		break;	// break every time - never loop - run once !!!
	}

	/* unload shared libs */
	for( i=0; i<argc; i++ )
	{
		if ( libhandle[i] )
			dlclose(libhandle[i]);
		else
			break;
	}
}

// CGameList ...

CGameList::CGameList(string Name)
{
	frameBuffer = CFrameBuffer::getInstance();
	name = Name;
	selected = 0;
	width = 500;
	height = 440;
	theight= g_Fonts->menu_title->getHeight();
	//
	fheight1= g_Fonts->gamelist_itemLarge->getHeight();
	fheight2= g_Fonts->gamelist_itemSmall->getHeight();
	fheight = fheight1 + fheight2 + 2;
	//
	listmaxshow = (height-theight-0)/fheight;
	height = theight+0+listmaxshow*fheight; // recalc height
	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-height) / 2) + g_settings.screen_StartY;
	liststart = 0;
}

CGameList::~CGameList()
{
	for(unsigned int count=0;count<gamelist.size();count++)
    {
    	delete gamelist[count];
	}
    gamelist.clear();

}


int CGameList::exec(CMenuTarget* parent, string actionKey)
{
	int res = menu_return::RETURN_REPAINT;

	if (parent)
	{
		parent->hide();
	}

	paintHead();

	//scan4games here!
    for(unsigned int count=0;count<gamelist.size();count++)
	{
    	delete gamelist[count];
	}
    gamelist.clear();

    game* tmp = new game();
    tmp->name = g_Locale->getText("menu.back");
    gamelist.insert(gamelist.end(), tmp);

	for(unsigned int count=0;count < (unsigned int)g_PluginList->getNumberOfPlugins();count++)
	{
    	if ( g_PluginList->getType(count)== 1 )
    	{
    		tmp = new game();
    		tmp->number = count;
    		tmp->name = g_PluginList->getName(count);
    		tmp->desc = g_PluginList->getDescription(count);
    		gamelist.insert(gamelist.end(), tmp);
    	}
	}

	paint();

    uint msg; uint data;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );

	bool loop=true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );

		if ( ( msg == CRCInput::RC_timeout ) ||
			 ( msg == (uint) g_settings.key_channelList_cancel ) )
		{
			loop=false;
		}
		else if ( msg == (uint) g_settings.key_channelList_pageup )
		{
			selected+=listmaxshow;
			if (selected>gamelist.size()-1)
				selected=0;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if ( msg == (uint) g_settings.key_channelList_pagedown )
		{
			if ((int(selected)-int(listmaxshow))<0)
				selected=gamelist.size()-1;
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
				selected = gamelist.size()-1;
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
			selected = (selected+1)%(gamelist.size());
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
		else if ( msg == CRCInput::RC_ok )
		{
			if(selected==0)
			{
				loop=false;
			}
			else
			{//exec the plugin :))
				runGame( selected );
			}
		}
		else if( (msg== CRCInput::RC_red) ||
				 (msg==CRCInput::RC_green) ||
				 (msg==CRCInput::RC_yellow) ||
				 (msg==CRCInput::RC_blue)  ||
		         (CRCInput::isNumeric(msg)) )
		{
			g_RCInput->postMsg( msg, data );
			loop=false;
		}
		else if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
		{
			loop = false;
			res = menu_return::RETURN_EXIT_ALL;
		}
	}
	hide();
	return res;
}

void CGameList::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}

void CGameList::paintItem(int pos)
{
	int ypos = (y+theight) + pos*fheight;
	int itemheight = fheight;

	int color = COL_MENUCONTENT;
	if (liststart+pos==selected)
	{
		color = COL_MENUCONTENTSELECTED;
	}

	if(liststart+pos==0)
	{	//back is half-height...
		itemheight = (fheight / 2) + 3;
		frameBuffer->paintBoxRel(x,ypos+itemheight, width, 15, COL_MENUCONTENT);
		frameBuffer->paintBoxRel(x+10,ypos+itemheight+5, width-20, 1, COL_MENUCONTENT+5);
		frameBuffer->paintBoxRel(x+10,ypos+itemheight+6, width-20, 1, COL_MENUCONTENT+2);
	}
	else
	{
		ypos -= (fheight / 2) - 15;
	}
    frameBuffer->paintBoxRel(x,ypos, width, itemheight, color);


	if(liststart+pos<gamelist.size())
	{
    	game* aktgame = gamelist[liststart+pos];
		g_Fonts->gamelist_itemLarge->RenderString(x+10, ypos+fheight1+3, width-20, aktgame->name.c_str(), color);
		g_Fonts->gamelist_itemSmall->RenderString(x+20, ypos+fheight,    width-20, aktgame->desc.c_str(), color);
	}
}

void CGameList::paintHead()
{
	frameBuffer->paintBoxRel(x,y, width,theight, COL_MENUHEAD);
	frameBuffer->paintIcon("games.raw",x+8,y+5);
	g_Fonts->menu_title->RenderString(x+38,y+theight+1, width, g_Locale->getText(name).c_str(), COL_MENUHEAD);
}

void CGameList::paint()
{
	liststart = (selected/listmaxshow)*listmaxshow;

	for(unsigned int count=0;count<listmaxshow;count++)
	{
		paintItem(count);
	}
}

void CGameList::runGame(int selected )
{
	#ifdef USEACTIONLOG
		g_ActionLog->println("mode: game, " + gamelist[selected]->name);
	#endif

	g_PluginList->setvtxtpid( g_RemoteControl->current_PIDs.PIDs.vtxtpid );
	g_PluginList->startPlugin( gamelist[selected]->number );

    //redraw menue...
    paintHead();
    paint();
}
