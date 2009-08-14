/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	                   and some other guys
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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#include <config.h>

#define NEUTRINO_CPP

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>


#include "global.h"
#include "neutrino.h"

#include "daemonc/remotecontrol.h"

#include "driver/framebuffer.h"
#include "driver/fontrenderer.h"
#include "driver/rcinput.h"

#include "gui/widget/menue.h"
#include "gui/widget/messagebox.h"
#include "gui/widget/hintbox.h"
#include "gui/widget/colorchooser.h"
#include "gui/widget/lcdcontroler.h"
#include "gui/widget/keychooser.h"
#include "gui/widget/stringinput.h"
#include "gui/widget/stringinput_ext.h"

#include "gui/color.h"

#include "gui/bedit/bouqueteditor_bouquets.h"
#include "gui/eventlist.h"
#include "gui/channellist.h"
#include "gui/screensetup.h"
#include "gui/gamelist.h"
#include "gui/infoviewer.h"
#include "gui/epgview.h"
#include "gui/update.h"
#include "gui/scan.h"
#include "gui/favorites.h"
#include "gui/sleeptimer.h"
#include "gui/dboxinfo.h"
#include "gui/timerlist.h"

#include "system/setting_helpers.h"
#include "system/settings.h"
#include "system/debug.h"
#include "system/flashtool.h"


using namespace std;

// Globale Variablen - to use import global.h

// I don't like globals, I would have hidden them in classes,
// but if you wanna do it so... ;)

static void initGlobals(void)
{
	g_fontRenderer = NULL;
	g_Fonts = 	 NULL;

	g_RCInput = 	NULL;
	g_lcdd = 	NULL;
	g_Controld = 	NULL;
	g_Timerd = 	NULL;
	g_Zapit = 	NULL;
	g_RemoteControl = NULL;

	g_EpgData = 	NULL;
	g_InfoViewer = 	NULL;
	g_EventList = 	NULL;

	g_Locale = 	NULL;
	g_PluginList = 	NULL;
}
// Ende globale Variablen


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+                                                                                     +
+          CNeutrinoApp - Constructor, initialize g_fontRenderer                      +
+                                                                                     +
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
CNeutrinoApp::CNeutrinoApp()
	: configfile('\t')
{
	frameBuffer = CFrameBuffer::getInstance();
	frameBuffer->setIconBasePath(DATADIR "/neutrino/icons/");

	vcrControl = CVCRControl::getInstance();

	g_fontRenderer = new fontRenderClass;
	SetupFrameBuffer();

	settingsFile = CONFIGDIR "/neutrino.conf";
	scanSettingsFile = CONFIGDIR "/scan.conf";

	mode = mode_unknown;
	channelList = NULL;
	bouquetList = NULL;
	skipShutdownTimer=false;
}

/*-------------------------------------------------------------------------------------
-                                                                                     -
-           CNeutrinoApp - Destructor                                                 -
-                                                                                     -
-------------------------------------------------------------------------------------*/
CNeutrinoApp::~CNeutrinoApp()
{
	if (channelList)
		delete channelList;
}

CNeutrinoApp* CNeutrinoApp::getInstance()
{
	static CNeutrinoApp* neutrinoApp = NULL;

	if(!neutrinoApp)
	{
		neutrinoApp = new CNeutrinoApp();
		dprintf(DEBUG_DEBUG, "NeutrinoApp Instance created\n");
	}
	return neutrinoApp;
}


void CNeutrinoApp::setupNetwork(bool force)
{
	if((g_settings.networkSetOnStartup) || (force))
	{
		if(!g_settings.network_dhcp)
		{
			dprintf(DEBUG_INFO, "doing network setup...\n");
			//setup network
			setNetworkAddress(g_settings.network_ip, g_settings.network_netmask, g_settings.network_broadcast);
			if(strcmp(g_settings.network_nameserver, "000.000.000.000")!=0)
			{
				setNameServer(g_settings.network_nameserver);
			}
			if(strcmp(g_settings.network_defaultgateway, "000.000.000.000")!=0)
			{
				setDefaultGateway(g_settings.network_defaultgateway);
			}
		}
	}
}
void CNeutrinoApp::testNetwork( )
{
	setupNetwork( true );

	dprintf(DEBUG_INFO, "doing network test...\n");
	//test network
	testNetworkSettings(g_settings.network_ip, g_settings.network_netmask, g_settings.network_broadcast, g_settings.network_defaultgateway, g_settings.network_nameserver,g_settings.network_dhcp);
}

void CNeutrinoApp::showNetwork( )
{
	dprintf(DEBUG_INFO, "showing current network settings...\n");
	showCurrentNetworkSettings();
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  setup Color Sheme (Neutrino)                               *
*                                                                                     *
**************************************************************************************/
void CNeutrinoApp::setupColors_neutrino()
{
	g_settings.menu_Head_alpha = 0x00;
	g_settings.menu_Head_red   = 0x00;
	g_settings.menu_Head_green = 0x0A;
	g_settings.menu_Head_blue  = 0x19;

	g_settings.menu_Head_Text_alpha = 0x00;
	g_settings.menu_Head_Text_red   = 0x5f;
	g_settings.menu_Head_Text_green = 0x46;
	g_settings.menu_Head_Text_blue  = 0x00;

	g_settings.menu_Content_alpha = 0x14;
	g_settings.menu_Content_red   = 0x00;
	g_settings.menu_Content_green = 0x0f;
	g_settings.menu_Content_blue  = 0x23;

	g_settings.menu_Content_Text_alpha = 0x00;
	g_settings.menu_Content_Text_red   = 0x64;
	g_settings.menu_Content_Text_green = 0x64;
	g_settings.menu_Content_Text_blue  = 0x64;

	g_settings.menu_Content_Selected_alpha = 0x14;
	g_settings.menu_Content_Selected_red   = 0x19;
	g_settings.menu_Content_Selected_green = 0x37;
	g_settings.menu_Content_Selected_blue  = 0x64;

	g_settings.menu_Content_Selected_Text_alpha  = 0x00;
	g_settings.menu_Content_Selected_Text_red    = 0x00;
	g_settings.menu_Content_Selected_Text_green  = 0x00;
	g_settings.menu_Content_Selected_Text_blue   = 0x00;

	g_settings.menu_Content_inactive_alpha = 0x14;
	g_settings.menu_Content_inactive_red   = 0x00;
	g_settings.menu_Content_inactive_green = 0x0f;
	g_settings.menu_Content_inactive_blue  = 0x23;

	g_settings.menu_Content_inactive_Text_alpha  = 0x00;
	g_settings.menu_Content_inactive_Text_red    = 55;
	g_settings.menu_Content_inactive_Text_green  = 70;
	g_settings.menu_Content_inactive_Text_blue   = 85;

	g_settings.infobar_alpha = 0x14;
	g_settings.infobar_red   = 0x00;
	g_settings.infobar_green = 0x0e;
	g_settings.infobar_blue  = 0x23;

	g_settings.infobar_Text_alpha = 0x00;
	g_settings.infobar_Text_red   = 0x64;
	g_settings.infobar_Text_green = 0x64;
	g_settings.infobar_Text_blue  = 0x64;
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  setup Color Sheme (classic)                                *
*                                                                                     *
**************************************************************************************/
void CNeutrinoApp::setupColors_classic()
{
	g_settings.menu_Head_alpha = 20;
	g_settings.menu_Head_red   =  5;
	g_settings.menu_Head_green = 10;
	g_settings.menu_Head_blue  = 60;

	g_settings.menu_Head_Text_alpha = 0;
	g_settings.menu_Head_Text_red   = 100;
	g_settings.menu_Head_Text_green = 100;
	g_settings.menu_Head_Text_blue  = 100;

	g_settings.menu_Content_alpha = 20;
	g_settings.menu_Content_red   = 50;
	g_settings.menu_Content_green = 50;
	g_settings.menu_Content_blue  = 50;

	g_settings.menu_Content_Text_alpha = 0;
	g_settings.menu_Content_Text_red   = 100;
	g_settings.menu_Content_Text_green = 100;
	g_settings.menu_Content_Text_blue  = 100;

	g_settings.menu_Content_Selected_alpha = 20;
	g_settings.menu_Content_Selected_red   = 5;
	g_settings.menu_Content_Selected_green = 10;
	g_settings.menu_Content_Selected_blue  = 60;

	g_settings.menu_Content_Selected_Text_alpha  = 0;
	g_settings.menu_Content_Selected_Text_red    = 100;
	g_settings.menu_Content_Selected_Text_green  = 100;
	g_settings.menu_Content_Selected_Text_blue   = 100;

	g_settings.menu_Content_inactive_alpha = 20;
	g_settings.menu_Content_inactive_red   = 50;
	g_settings.menu_Content_inactive_green = 50;
	g_settings.menu_Content_inactive_blue  = 50;

	g_settings.menu_Content_inactive_Text_alpha  = 0;
	g_settings.menu_Content_inactive_Text_red    = 80;
	g_settings.menu_Content_inactive_Text_green  = 80;
	g_settings.menu_Content_inactive_Text_blue   = 80;

	g_settings.infobar_alpha = 20;
	g_settings.infobar_red   = 5;
	g_settings.infobar_green = 10;
	g_settings.infobar_blue  = 60;

	g_settings.infobar_Text_alpha = 0;
	g_settings.infobar_Text_red   = 100;
	g_settings.infobar_Text_green = 100;
	g_settings.infobar_Text_blue  = 100;
}


/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  loadSetup, load the application-settings                   *
*                                                                                     *
**************************************************************************************/
int CNeutrinoApp::loadSetup()
{
	int erg = 0;

	//settings laden - und dabei Defaults setzen!
	if(!configfile.loadConfig(CONFIGDIR "/neutrino.conf"))
	{
		//file existiert nicht
		erg = 1;
	}

	//video
	g_settings.video_Signal = configfile.getInt32( "video_Signal", 0 ); //composite
	g_settings.video_Format = configfile.getInt32( "video_Format", 2 ); //4:3

	//misc
	g_settings.shutdown_real = configfile.getInt32( "shutdown_real", 1 );
	g_settings.shutdown_showclock = configfile.getInt32( "shutdown_showclock", 1 );
	g_settings.show_camwarning = configfile.getInt32( "show_camwarning", 1 );
	strcpy(g_settings.record_safety_time, configfile.getString( "record_safety_time", "00").c_str());

	//audio
	g_settings.audio_AnalogMode = configfile.getInt32( "audio_AnalogMode", 0 );
	g_settings.audio_DolbyDigital = configfile.getInt32( "audio_DolbyDigital", 0 );
	g_settings.audio_SPDIF_Control = configfile.getInt32( "audio_SPDIF_Control", 0 );
	

	//vcr
	g_settings.vcr_AutoSwitch = configfile.getInt32( "vcr_AutoSwitch", 1 );

	//language
	strcpy( g_settings.language, configfile.getString( "language", "deutsch" ).c_str() );

	//timing
	g_settings.timing_menu = configfile.getInt32( "timing_menu", 10 );
	g_settings.timing_chanlist = configfile.getInt32( "timing_chanlist", 10 );
	g_settings.timing_epg = configfile.getInt32( "timing_epg", 8 );
	g_settings.timing_infobar = configfile.getInt32( "timing_infobar", 8 );

	//widget settings
	g_settings.widget_fade = configfile.getInt32( "widget_fade", 1 );

	//colors (neutrino defaultcolors)
	g_settings.menu_Head_alpha = configfile.getInt32( "menu_Head_alpha", 0x00 );
	g_settings.menu_Head_red = configfile.getInt32( "menu_Head_red", 0x00 );
	g_settings.menu_Head_green = configfile.getInt32( "menu_Head_green", 0x0A );
	g_settings.menu_Head_blue = configfile.getInt32( "menu_Head_blue", 0x19 );

	g_settings.menu_Head_Text_alpha = configfile.getInt32( "menu_Head_Text_alpha", 0x00 );
	g_settings.menu_Head_Text_red = configfile.getInt32( "menu_Head_Text_red", 0x5f );
	g_settings.menu_Head_Text_green = configfile.getInt32( "menu_Head_Text_green", 0x46 );
	g_settings.menu_Head_Text_blue = configfile.getInt32( "menu_Head_Text_blue", 0x00 );

	g_settings.menu_Content_alpha = configfile.getInt32( "menu_Content_alpha", 0x14 );
	g_settings.menu_Content_red = configfile.getInt32( "menu_Content_red", 0x00 );
	g_settings.menu_Content_green = configfile.getInt32( "menu_Content_green", 0x0f );
	g_settings.menu_Content_blue = configfile.getInt32( "menu_Content_blue", 0x23 );

	g_settings.menu_Content_Text_alpha = configfile.getInt32( "menu_Content_Text_alpha", 0x00 );
	g_settings.menu_Content_Text_red = configfile.getInt32( "menu_Content_Text_red", 0x64 );
	g_settings.menu_Content_Text_green = configfile.getInt32( "menu_Content_Text_green", 0x64 );
	g_settings.menu_Content_Text_blue = configfile.getInt32( "menu_Content_Text_blue", 0x64 );

	g_settings.menu_Content_Selected_alpha = configfile.getInt32( "menu_Content_Selected_alpha", 0x14 );
	g_settings.menu_Content_Selected_red = configfile.getInt32( "menu_Content_Selected_red", 0x19 );
	g_settings.menu_Content_Selected_green = configfile.getInt32( "menu_Content_Selected_green", 0x37 );
	g_settings.menu_Content_Selected_blue = configfile.getInt32( "menu_Content_Selected_blue", 0x64 );

	g_settings.menu_Content_Selected_Text_alpha = configfile.getInt32( "menu_Content_Selected_Text_alpha", 0x00 );
	g_settings.menu_Content_Selected_Text_red = configfile.getInt32( "menu_Content_Selected_Text_red", 0x00 );
	g_settings.menu_Content_Selected_Text_green = configfile.getInt32( "menu_Content_Selected_Text_green", 0x00 );
	g_settings.menu_Content_Selected_Text_blue = configfile.getInt32( "menu_Content_Selected_Text_blue", 0x00 );

	g_settings.menu_Content_inactive_alpha = configfile.getInt32( "menu_Content_inactive_alpha", 0x14 );
	g_settings.menu_Content_inactive_red = configfile.getInt32( "menu_Content_inactive_red", 0x00 );
	g_settings.menu_Content_inactive_green = configfile.getInt32( "menu_Content_inactive_green", 0x0f );
	g_settings.menu_Content_inactive_blue = configfile.getInt32( "menu_Content_inactive_blue", 0x23 );

	g_settings.menu_Content_inactive_Text_alpha = configfile.getInt32( "menu_Content_inactive_Text_alpha", 0x00 );
	g_settings.menu_Content_inactive_Text_red = configfile.getInt32( "menu_Content_inactive_Text_red", 55 );
	g_settings.menu_Content_inactive_Text_green = configfile.getInt32( "menu_Content_inactive_Text_green", 70 );
	g_settings.menu_Content_inactive_Text_blue = configfile.getInt32( "menu_Content_inactive_Text_blue", 85 );

	g_settings.infobar_alpha = configfile.getInt32( "infobar_alpha", 0x14 );
	g_settings.infobar_red = configfile.getInt32( "infobar_red", 0x00 );
	g_settings.infobar_green = configfile.getInt32( "infobar_green", 0x0e );
	g_settings.infobar_blue = configfile.getInt32( "infobar_blue", 0x23 );

	g_settings.infobar_Text_alpha = configfile.getInt32( "infobar_Text_alpha", 0x00 );
	g_settings.infobar_Text_red = configfile.getInt32( "infobar_Text_red", 0x64 );
	g_settings.infobar_Text_green = configfile.getInt32( "infobar_Text_green", 0x64 );
	g_settings.infobar_Text_blue = configfile.getInt32( "infobar_Text_blue", 0x64 );

	//network
	g_settings.networkSetOnStartup = configfile.getInt32( "networkSetOnStartup", fromflash==true?1:0 );
	g_settings.network_dhcp = configfile.getInt32( "network_dhcp", 1);
	strcpy( g_settings.network_ip, configfile.getString( "network_ip", "10.10.10.100" ).c_str() );
	strcpy( g_settings.network_netmask, configfile.getString( "network_netmask", "255.255.255.0" ).c_str() );
	strcpy( g_settings.network_broadcast, configfile.getString( "network_broadcast", "10.10.10.255" ).c_str() );
	strcpy( g_settings.network_defaultgateway, configfile.getString( "network_defaultgateway", "" ).c_str() );
	strcpy( g_settings.network_nameserver, configfile.getString( "network_nameserver", "" ).c_str() );

	//streaming
	g_settings.network_streaming_use = configfile.getInt32( "network_streaming_use", 0 );
	g_settings.network_streaming_stopplayback = configfile.getInt32( "network_streaming_stopplayback", 0 );
	g_settings.network_streaming_stopsectionsd = configfile.getInt32( "network_streaming_stopsectionsd", 1 );
	strcpy( g_settings.network_streamingserver, configfile.getString( "network_streamingserver", "10.10.10.10").c_str() );
	strcpy( g_settings.network_streamingserverport, configfile.getString( "network_streamingserverport", "4000").c_str() );

	//vcr per ir
	g_settings.vcr_recording = configfile.getInt32( "vcr_recording", 0 );
	strcpy( g_settings.vcr_devicename, configfile.getString( "vcr_devicename", "ORION").c_str() );

	//rc-key configuration
	g_settings.key_tvradio_mode = configfile.getInt32( "key_tvradio_mode", CRCInput::RC_nokey );

	g_settings.key_channelList_pageup = configfile.getInt32( "key_channelList_pageup",  CRCInput::RC_red );
	g_settings.key_channelList_pagedown = configfile.getInt32( "key_channelList_pagedown", CRCInput::RC_green );
	g_settings.key_channelList_cancel = configfile.getInt32( "key_channelList_cancel",  CRCInput::RC_home );

	g_settings.key_quickzap_up = configfile.getInt32( "key_quickzap_up",  CRCInput::RC_up );
	g_settings.key_quickzap_down = configfile.getInt32( "key_quickzap_down",  CRCInput::RC_down );
	g_settings.key_bouquet_up = configfile.getInt32( "key_bouquet_up",  CRCInput::RC_right );
	g_settings.key_bouquet_down = configfile.getInt32( "key_bouquet_down",  CRCInput::RC_left );
	g_settings.key_subchannel_up = configfile.getInt32( "key_subchannel_up",  CRCInput::RC_right );
	g_settings.key_subchannel_down = configfile.getInt32( "key_subchannel_down",  CRCInput::RC_left );

	strcpy( g_settings.repeat_blocker, configfile.getString( "repeat_blocker", g_info.box_Type==3?"150":"25" ).c_str() );
	strcpy( g_settings.repeat_genericblocker, configfile.getString( "repeat_genericblocker", g_info.box_Type==3?"25":"0" ).c_str() );

	//screen configuration
	g_settings.screen_StartX = configfile.getInt32( "screen_StartX", 37 );
	g_settings.screen_StartY = configfile.getInt32( "screen_StartY", 23 );
	g_settings.screen_EndX = configfile.getInt32( "screen_EndX", 668 );
	g_settings.screen_EndY = configfile.getInt32( "screen_EndY", 555 );

	//font configuration
//	g_settings.fontsize_ = configfile.getInt32( "fontsize_",  );
	strcpy( g_settings.fontsize_menu		,  configfile.getString( "fontsize_menu", "20").c_str() );
	strcpy( g_settings.fontsize_menu_title	,  configfile.getString( "fontsize_menu_title", "30").c_str() );
	strcpy( g_settings.fontsize_menu_info	,  configfile.getString( "fontsize_menu_info", "16").c_str() );
	strcpy( g_settings.fontsize_epg_title	,  configfile.getString( "fontsize_epg_title", "25").c_str() );
	strcpy( g_settings.fontsize_epg_info1	,  configfile.getString( "fontsize_epg_info1", "17").c_str() );
	strcpy( g_settings.fontsize_epg_info2	,  configfile.getString( "fontsize_epg_info2", "17").c_str() );
	strcpy( g_settings.fontsize_epg_date	,  configfile.getString( "fontsize_epg_date", "15").c_str() );
	strcpy( g_settings.fontsize_alert		,  configfile.getString( "fontsize_alert", "100").c_str() );
	strcpy( g_settings.fontsize_eventlist_title		,  configfile.getString( "fontsize_eventlist_title", "30").c_str() );
	strcpy( g_settings.fontsize_eventlist_itemlarge, configfile.getString( "fontsize_eventlist_itemlarge", "20").c_str() );
	strcpy( g_settings.fontsize_eventlist_itemsmall, configfile.getString( "fontsize_eventlist_itemsmall", "14").c_str() );
	strcpy( g_settings.fontsize_eventlist_datetime , configfile.getString( "fontsize_eventlist_datetime", "16").c_str() );

//	strcpy( g_settings.fontsize_gamelist_itemlarge	,  configfile.getString( "fontsize_gamelist_itemlarge", "20").c_str() );
//	strcpy( g_settings.fontsize_gamelist_itemsmall	,  configfile.getString( "fontsize_gamelist_itemsmall", "16").c_str() );

	strcpy( g_settings.fontsize_channellist			,  configfile.getString( "fontsize_channellist", "20").c_str() );
	strcpy( g_settings.fontsize_channellist_descr	,  configfile.getString( "fontsize_channellist_descr", "20").c_str() );
	strcpy( g_settings.fontsize_channellist_number , configfile.getString( "fontsize_channellist_number", "14").c_str() );
	strcpy( g_settings.fontsize_channel_num_zap		,  configfile.getString( "fontsize_channel_num_zap", "40").c_str() );

	strcpy( g_settings.fontsize_infobar_number		, configfile.getString( "fontsize_infobar_number", "50").c_str() );
	strcpy( g_settings.fontsize_infobar_channame	, configfile.getString( "fontsize_infobar_channame", "30").c_str() );
	strcpy( g_settings.fontsize_infobar_info		, configfile.getString( "fontsize_infobar_info", "20").c_str() );
	strcpy( g_settings.fontsize_infobar_small		, configfile.getString( "fontsize_infobar_small", "14").c_str() );

	//Software-update
	g_settings.softupdate_mode = configfile.getInt32( "softupdate_mode", 1 );
	strcpy( g_settings.softupdate_currentversion, configfile.getString( "softupdate_currentversion", "" ).c_str() );
	strcpy( g_settings.softupdate_proxyserver, configfile.getString( "softupdate_proxyserver", "" ).c_str() );
	strcpy( g_settings.softupdate_proxyusername, configfile.getString( "softupdate_proxyusername", "" ).c_str() );
	strcpy( g_settings.softupdate_proxypassword, configfile.getString( "softupdate_proxypassword", "" ).c_str() );

	//BouquetHandling
	g_settings.bouquetlist_mode = configfile.getInt32( "bouquetlist_mode", 0 );

	// parentallock
	g_settings.parentallock_prompt = configfile.getInt32( "parentallock_prompt", 0 );
	g_settings.parentallock_lockage = configfile.getInt32( "parentallock_lockage", 12 );
	strcpy( g_settings.parentallock_pincode, configfile.getString( "parentallock_pincode", "0000" ).c_str() );

	if(configfile.getUnknownKeyQueryedFlag() && (erg==0))
	{
		erg = 2;
	}

	if(!scanSettings.loadSettings(scanSettingsFile))
	{
		dprintf(DEBUG_NORMAL,"error while loading scan-settings, using defaults!\n");
		scanSettings.useDefaults();
	}

	return erg;
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  saveSetup, save the application-settings                   *
*                                                                                     *
**************************************************************************************/
void CNeutrinoApp::saveSetup()
{


	if(!scanSettings.saveSettings(scanSettingsFile))
	{
		dprintf(DEBUG_NORMAL, "error while saveing scan-settings!\n");
	}

	//video
	configfile.setInt32( "video_Signal", g_settings.video_Signal );
	configfile.setInt32( "video_Format", g_settings.video_Format );

	//misc
	configfile.setInt32( "shutdown_real", g_settings.shutdown_real );
	configfile.setInt32( "shutdown_showclock", g_settings.shutdown_showclock );
	configfile.setInt32( "show_camwarning", g_settings.show_camwarning );
	configfile.setString( "record_safety_time", g_settings.record_safety_time );

	//audio
	configfile.setInt32( "audio_AnalogMode", g_settings.audio_AnalogMode );
	configfile.setInt32( "audio_DolbyDigital", g_settings.audio_DolbyDigital );
	configfile.setInt32( "audio_SPDIF_Control", g_settings.audio_SPDIF_Control );

	//vcr
	configfile.setInt32( "vcr_AutoSwitch", g_settings.vcr_AutoSwitch );

	//language
	configfile.setString( "language", g_settings.language );

	//timing
	configfile.setInt32( "timing_menu", g_settings.timing_menu );
	configfile.setInt32( "timing_chanlist", g_settings.timing_chanlist );
	configfile.setInt32( "timing_epg", g_settings.timing_epg );
	configfile.setInt32( "timing_infobar", g_settings.timing_infobar );

	//widget settings
	configfile.setInt32( "widget_fade", g_settings.widget_fade );

	//colors
	configfile.setInt32( "menu_Head_alpha", g_settings.menu_Head_alpha );
	configfile.setInt32( "menu_Head_red", g_settings.menu_Head_red );
	configfile.setInt32( "menu_Head_green", g_settings.menu_Head_green );
	configfile.setInt32( "menu_Head_blue", g_settings.menu_Head_blue );

	configfile.setInt32( "menu_Head_Text_alpha", g_settings.menu_Head_Text_alpha );
	configfile.setInt32( "menu_Head_Text_red", g_settings.menu_Head_Text_red );
	configfile.setInt32( "menu_Head_Text_green", g_settings.menu_Head_Text_green );
	configfile.setInt32( "menu_Head_Text_blue", g_settings.menu_Head_Text_blue );

	configfile.setInt32( "menu_Content_alpha", g_settings.menu_Content_alpha );
	configfile.setInt32( "menu_Content_red", g_settings.menu_Content_red );
	configfile.setInt32( "menu_Content_green", g_settings.menu_Content_green );
	configfile.setInt32( "menu_Content_blue", g_settings.menu_Content_blue );

	configfile.setInt32( "menu_Content_Text_alpha", g_settings.menu_Content_Text_alpha );
	configfile.setInt32( "menu_Content_Text_red", g_settings.menu_Content_Text_red );
	configfile.setInt32( "menu_Content_Text_green", g_settings.menu_Content_Text_green );
	configfile.setInt32( "menu_Content_Text_blue", g_settings.menu_Content_Text_blue );

	configfile.setInt32( "menu_Content_Selected_alpha", g_settings.menu_Content_Selected_alpha );
	configfile.setInt32( "menu_Content_Selected_red", g_settings.menu_Content_Selected_red );
	configfile.setInt32( "menu_Content_Selected_green", g_settings.menu_Content_Selected_green );
	configfile.setInt32( "menu_Content_Selected_blue", g_settings.menu_Content_Selected_blue );

	configfile.setInt32( "menu_Content_Selected_Text_alpha", g_settings.menu_Content_Selected_Text_alpha );
	configfile.setInt32( "menu_Content_Selected_Text_red", g_settings.menu_Content_Selected_Text_red );
	configfile.setInt32( "menu_Content_Selected_Text_green", g_settings.menu_Content_Selected_Text_green );
	configfile.setInt32( "menu_Content_Selected_Text_blue", g_settings.menu_Content_Selected_Text_blue );

	configfile.setInt32( "menu_Content_inactive_alpha", g_settings.menu_Content_inactive_alpha );
	configfile.setInt32( "menu_Content_inactive_red", g_settings.menu_Content_inactive_red );
	configfile.setInt32( "menu_Content_inactive_green", g_settings.menu_Content_inactive_green );
	configfile.setInt32( "menu_Content_inactive_blue", g_settings.menu_Content_inactive_blue );

	configfile.setInt32( "menu_Content_inactive_Text_alpha", g_settings.menu_Content_inactive_Text_alpha );
	configfile.setInt32( "menu_Content_inactive_Text_red", g_settings.menu_Content_inactive_Text_red );
	configfile.setInt32( "menu_Content_inactive_Text_green", g_settings.menu_Content_inactive_Text_green );
	configfile.setInt32( "menu_Content_inactive_Text_blue", g_settings.menu_Content_inactive_Text_blue );

	configfile.setInt32( "infobar_alpha", g_settings.infobar_alpha );
	configfile.setInt32( "infobar_red", g_settings.infobar_red );
	configfile.setInt32( "infobar_green", g_settings.infobar_green );
	configfile.setInt32( "infobar_blue", g_settings.infobar_blue );

	configfile.setInt32( "infobar_Text_alpha", g_settings.infobar_Text_alpha );
	configfile.setInt32( "infobar_Text_red", g_settings.infobar_Text_red );
	configfile.setInt32( "infobar_Text_green", g_settings.infobar_Text_green );
	configfile.setInt32( "infobar_Text_blue", g_settings.infobar_Text_blue );

	//network
	configfile.setInt32( "networkSetOnStartup", g_settings.networkSetOnStartup );
	configfile.setInt32( "network_dhcp", g_settings.network_dhcp );
	configfile.setString( "network_ip", g_settings.network_ip );
	configfile.setString( "network_netmask", g_settings.network_netmask );
	configfile.setString( "network_broadcast", g_settings.network_broadcast );
	configfile.setString( "network_defaultgateway", g_settings.network_defaultgateway );
	configfile.setString( "network_nameserver", g_settings.network_nameserver );

	//streaming
	configfile.setInt32( "network_streaming_use", g_settings.network_streaming_use );
	configfile.setInt32( "network_streaming_stopplayback", g_settings.network_streaming_stopplayback );
	configfile.setInt32( "network_streaming_stopsectionsd", g_settings.network_streaming_stopsectionsd );
	configfile.setString( "network_streamingserver", g_settings.network_streamingserver );
	configfile.setString( "network_streamingserverport", g_settings.network_streamingserverport );

	//vcr per ir
	configfile.setInt32( "vcr_recording", g_settings.vcr_recording );
	configfile.setString( "vcr_devicename", g_settings.vcr_devicename );

	//rc-key configuration
	configfile.setInt32( "key_tvradio_mode", g_settings.key_tvradio_mode );

	configfile.setInt32( "key_channelList_pageup", g_settings.key_channelList_pageup );
	configfile.setInt32( "key_channelList_pagedown", g_settings.key_channelList_pagedown );
	configfile.setInt32( "key_channelList_cancel", g_settings.key_channelList_cancel );

	configfile.setInt32( "key_quickzap_up", g_settings.key_quickzap_up );
	configfile.setInt32( "key_quickzap_down", g_settings.key_quickzap_down );
	configfile.setInt32( "key_bouquet_up", g_settings.key_bouquet_up );
	configfile.setInt32( "key_bouquet_down", g_settings.key_bouquet_down );
	configfile.setInt32( "key_subchannel_up", g_settings.key_subchannel_up );
	configfile.setInt32( "key_subchannel_down", g_settings.key_subchannel_down );

	configfile.setString( "repeat_blocker", g_settings.repeat_blocker );
	configfile.setString( "repeat_genericblocker", g_settings.repeat_genericblocker );

	//screen configuration
	configfile.setInt32( "screen_StartX", g_settings.screen_StartX );
	configfile.setInt32( "screen_StartY", g_settings.screen_StartY );
	configfile.setInt32( "screen_EndX", g_settings.screen_EndX );
	configfile.setInt32( "screen_EndY", g_settings.screen_EndY );

	//font configuration
	configfile.setString( "fontsize_menu", g_settings.fontsize_menu );
	configfile.setString( "fontsize_menu_title", g_settings.fontsize_menu_title );
	configfile.setString( "fontsize_menu_info", g_settings.fontsize_menu_info );
	configfile.setString( "fontsize_epg_title", g_settings.fontsize_epg_title );
	configfile.setString( "fontsize_epg_info1", g_settings.fontsize_epg_info1 );
	configfile.setString( "fontsize_epg_info2", g_settings.fontsize_epg_info2 );
	configfile.setString( "fontsize_epg_date", g_settings.fontsize_epg_date );
	configfile.setString( "fontsize_alert", g_settings.fontsize_alert );
	configfile.setString( "fontsize_eventlist_title", g_settings.fontsize_eventlist_title );
	configfile.setString( "fontsize_eventlist_itemlarge", g_settings.fontsize_eventlist_itemlarge );
	configfile.setString( "fontsize_eventlist_itemsmall", g_settings.fontsize_eventlist_itemsmall );
	configfile.setString( "fontsize_eventlist_datetime", g_settings.fontsize_eventlist_datetime );

//	configfile.setString( "fontsize_gamelist_itemlarge", g_settings.fontsize_gamelist_itemlarge );
//	configfile.setString( "fontsize_gamelist_itemsmall", g_settings.fontsize_gamelist_itemsmall );

	configfile.setString( "fontsize_channellist", g_settings.fontsize_channellist );
	configfile.setString( "fontsize_channellist_descr", g_settings.fontsize_channellist_descr );
	configfile.setString( "fontsize_channellist_number", g_settings.fontsize_channellist_number );
	configfile.setString( "fontsize_channel_num_zap", g_settings.fontsize_channel_num_zap );

	configfile.setString( "fontsize_infobar_number", g_settings.fontsize_infobar_number );
	configfile.setString( "fontsize_infobar_channame", g_settings.fontsize_infobar_channame );
	configfile.setString( "fontsize_infobar_info", g_settings.fontsize_infobar_info );
	configfile.setString( "fontsize_infobar_small", g_settings.fontsize_infobar_small );

	//Software-update
	configfile.setInt32( "softupdate_mode", g_settings.softupdate_mode );
	configfile.setString( "softupdate_currentversion", g_settings.softupdate_currentversion );
	configfile.setString( "softupdate_proxyserver", g_settings.softupdate_proxyserver );
	configfile.setString( "softupdate_proxyusername", g_settings.softupdate_proxyusername );
	configfile.setString( "softupdate_proxypassword", g_settings.softupdate_proxypassword );

	//BouquetHandling
	configfile.setInt32( "bouquetlist_mode", g_settings.bouquetlist_mode );

	// parentallock
	configfile.setInt32( "parentallock_prompt", g_settings.parentallock_prompt );
	configfile.setInt32( "parentallock_lockage", g_settings.parentallock_lockage );
	configfile.setString( "parentallock_pincode", g_settings.parentallock_pincode );

	if(configfile.getModifiedFlag())
	{
		dprintf(DEBUG_INFO, "saveing neutrino txt-config\n");
		configfile.saveConfig(CONFIGDIR "/neutrino.conf");
	}
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  firstChannel, get the initial channel                      *
*                                                                                     *
**************************************************************************************/

void CNeutrinoApp::firstChannel()
{
	g_Zapit->getLastChannel(firstchannel.channelNumber, firstchannel.mode);
}

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  doChecks, check if card fits cam		              *
*                                                                                     *
**************************************************************************************/

void CNeutrinoApp::doChecks()
{
	FILE* fd;
	fd = fopen(UCODEDIR "/avia500.ux", "r");
	if(fd)
		fclose(fd);
	bool ucodes_ok= (fd);
	fd = fopen(UCODEDIR "/avia600.ux", "r");
	if(fd)
		fclose(fd);
	ucodes_ok= ucodes_ok||(fd);
	fd = fopen(UCODEDIR "/ucode.bin", "r");
	if(fd)
		fclose(fd);
	ucodes_ok= ucodes_ok&&(fd);
	fd = fopen(UCODEDIR "/cam-alpha.bin", "r");
	if(fd)
		fclose(fd);
	ucodes_ok= ucodes_ok&&(fd);

	if ( !ucodes_ok )
		ShowMsg ( "messagebox.error", g_Locale->getText("ucodes.failure"), CMessageBox::mbrCancel, CMessageBox::mbCancel, "error.raw" );
}


/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  channelsInit, get the Channellist from daemon              *
*                                                                                     *
**************************************************************************************/
void CNeutrinoApp::channelsInit()
{
	dprintf(DEBUG_DEBUG, "doing channelsInit\n");
	//deleting old channelList for mode-switching.

	delete channelList;
	channelList = new CChannelList( "channellist.head" );
	CZapitClient::BouquetChannelList zapitChannels;
	g_Zapit->getChannels( zapitChannels );
	for (uint i=0; i<zapitChannels.size(); i++)
	{
		channelList->addChannel( zapitChannels[i].nr, zapitChannels[i].nr, zapitChannels[i].name, zapitChannels[i].channel_id);
	}
	dprintf(DEBUG_DEBUG, "got channels\n");

	delete bouquetList;
	bouquetList = new CBouquetList( "bouquetlist.head" );
	bouquetList->orgChannelList = channelList;
	CZapitClient::BouquetList zapitBouquets;
	g_Zapit->getBouquets(zapitBouquets, false, true); // UTF-8
	for (uint i=0; i<zapitBouquets.size(); i++)
	{
		bouquetList->addBouquet( zapitBouquets[i].name, zapitBouquets[i].bouquet_nr, zapitBouquets[i].locked);
	}
	dprintf(DEBUG_DEBUG, "got bouquets\n");

	for ( uint i=0; i< bouquetList->Bouquets.size(); i++ )
	{
		CZapitClient::BouquetChannelList zapitChannels;
		g_Zapit->getBouquetChannels( bouquetList->Bouquets[i]->unique_key, zapitChannels);
		for (uint j=0; j<zapitChannels.size(); j++)
		{
			CChannelList::CChannel* channel = channelList->getChannel( zapitChannels[j].nr);

			bouquetList->Bouquets[i]->channelList->addChannel( channel);
			if ( bouquetList->Bouquets[i]->bLocked)
			{
				channel->bAlwaysLocked = true;
			}
		}
	}
	dprintf(DEBUG_DEBUG, "\nAll bouquets-channels received\n");
}


/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  run, the main runloop                                      *
*                                                                                     *
**************************************************************************************/

void CNeutrinoApp::CmdParser(int argc, char **argv)
{
	softupdate = false;
	fromflash = false;
//	g_settings.network_streaming_use = 0;

	#define FONTNAME "Micron"
	#define FONTFILE "micron"

	fontName = FONTNAME;
	fontFile = FONTDIR "/" FONTFILE;
	fontsSizeOffset = 0;

	for(int x=1; x<argc; x++)
	{
		if ( !strcmp(argv[x], "-su"))
		{
			dprintf(DEBUG_NORMAL, "Software update enabled\n");
			softupdate = true;
		}
		else if ( !strcmp(argv[x], "-z"))
		{
			dprintf(DEBUG_NORMAL, "zapitmode is default..\n");
		}
		else if ( !strcmp(argv[x], "-flash"))
		{
			dprintf(DEBUG_NORMAL, "enable flash\n");
			fromflash = true;
		}
		else if ( !strcmp(argv[x], "-font"))
		{
			if ( ( x + 3 ) < argc )
			{
				fontFile = argv[x+ 1];
				fontName = argv[x+ 2];
				fontsSizeOffset = atoi(argv[x+ 3]);
			}
			x+=3;
		}
		else if ( !strcmp(argv[x], "-debuglevel"))
		{
			int dl = atoi(argv[x+ 1]);
			dprintf(DEBUG_NORMAL, "set debuglevel: %d\n", dl);
			setDebugLevel(dl);
			x++;
		}
		else
		{
			dprintf(DEBUG_NORMAL, "Usage: neutrino [-su] [-debuglevel 0..3] [-flash] [-font /fontdir/fontfile fontname fontsize]\n");
			exit(0);
		}
	}
}

void CNeutrinoApp::SetupFrameBuffer()
{
	frameBuffer->init();
	if (frameBuffer->setMode(720, 576, 8))
	{
		dprintf(DEBUG_NORMAL, "Error while setting framebuffer mode\n");
		exit(-1);
	}

	//make 1..8 transparent for dummy painting
	for(int count =0;count<8;count++)
		frameBuffer->paletteSetColor(count, 0x000000, 0xffff);
	frameBuffer->paletteSet();
}

void CNeutrinoApp::SetupFonts()
{
	dprintf(DEBUG_INFO, "FontFile: %s\n", (fontFile+ ".ttf").c_str() );
	dprintf(DEBUG_INFO, "FontName: %s\n", fontName.c_str() );
	dprintf(DEBUG_INFO, "FontSize: %d\n", fontsSizeOffset );

	g_fontRenderer->AddFont((fontFile+ ".ttf").c_str() );
	g_fontRenderer->AddFont((fontFile+ "_bold.ttf").c_str() );
	g_fontRenderer->AddFont((fontFile+ "_italic.ttf").c_str() );

	g_Fonts->menu =         g_fontRenderer->getFont(fontName.c_str(), "Bold", atoi(g_settings.fontsize_menu)); 
	g_Fonts->menu_title =   g_fontRenderer->getFont(fontName.c_str(), "Bold", atoi(g_settings.fontsize_menu_title));
	g_Fonts->menu_info =    g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_menu_info));

	g_Fonts->epg_title =    g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_epg_title) + fontsSizeOffset );
	g_Fonts->epg_info1 =	g_fontRenderer->getFont(fontName.c_str(), "Italic",  atoi(g_settings.fontsize_epg_info1) + 2* fontsSizeOffset );
	g_Fonts->epg_info2 =	g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_epg_info2) + 2* fontsSizeOffset );
	g_Fonts->epg_date =		g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_epg_date) + 2* fontsSizeOffset );

	g_Fonts->alert =		g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_alert) );

	g_Fonts->eventlist_title	 =	g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_eventlist_title));
	g_Fonts->eventlist_itemLarge =	g_fontRenderer->getFont(fontName.c_str(), "Bold",	atoi(g_settings.fontsize_eventlist_itemlarge) + fontsSizeOffset );
	g_Fonts->eventlist_itemSmall =	g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_eventlist_itemsmall) + fontsSizeOffset );
	g_Fonts->eventlist_datetime  =	g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_eventlist_datetime) + fontsSizeOffset );

	g_Fonts->gamelist_itemLarge =	g_fontRenderer->getFont(fontName.c_str(), "Bold", /*atoi(g_settings.fontsize_gamelist_itemlarge)*/ 20 + fontsSizeOffset );
	g_Fonts->gamelist_itemSmall =	g_fontRenderer->getFont(fontName.c_str(), "Regular", /*atoi(g_settings.fontsize_gamelist_itemsmall)*/ 16 + fontsSizeOffset );

	g_Fonts->channellist		=	g_fontRenderer->getFont(fontName.c_str(), "Bold", atoi(g_settings.fontsize_channellist) + fontsSizeOffset );
	g_Fonts->channellist_descr	=	g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_channellist_descr) + fontsSizeOffset );
	g_Fonts->channellist_number =	g_fontRenderer->getFont(fontName.c_str(), "Bold", atoi(g_settings.fontsize_channellist_number) + 2* fontsSizeOffset );
	g_Fonts->channel_num_zap	=	g_fontRenderer->getFont(fontName.c_str(), "Bold", atoi(g_settings.fontsize_channel_num_zap));

	g_Fonts->infobar_number		=	g_fontRenderer->getFont(fontName.c_str(), "Bold", atoi(g_settings.fontsize_infobar_number));
	g_Fonts->infobar_channame	=	g_fontRenderer->getFont(fontName.c_str(), "Bold", atoi(g_settings.fontsize_infobar_channame));
	g_Fonts->infobar_info		=	g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_infobar_info) + fontsSizeOffset );
	g_Fonts->infobar_small		=	g_fontRenderer->getFont(fontName.c_str(), "Regular", atoi(g_settings.fontsize_infobar_small) + fontsSizeOffset );

}


void CNeutrinoApp::ClearFrameBuffer()
{
	if (frameBuffer->getActive())
		memset(frameBuffer->getFrameBufferPointer(), 255, frameBuffer->getStride()*576);

	//backgroundmode
	frameBuffer->setBackgroundColor(COL_BACKGROUND);
	frameBuffer->useBackground(false);

	//background
	frameBuffer->paletteSetColor(COL_BACKGROUND, 0x000000, 0xffff);
	//Windows Colors
	frameBuffer->paletteSetColor(0x1, 0x010101, 0);
	frameBuffer->paletteSetColor(0x2, 0x800000, 0);
	frameBuffer->paletteSetColor(0x3, 0x008000, 0);
	frameBuffer->paletteSetColor(0x4, 0x808000, 0);
	frameBuffer->paletteSetColor(0x5, 0x000080, 0);
	frameBuffer->paletteSetColor(0x6, 0x800080, 0);
	frameBuffer->paletteSetColor(0x7, 0x008080, 0);
	//	frameBuffer.paletteSetColor(0x8, 0xC0C0C0, 0);
	frameBuffer->paletteSetColor(0x8, 0xA0A0A0, 0);

	//	frameBuffer.paletteSetColor(0x9, 0x808080, 0);
	frameBuffer->paletteSetColor(0x9, 0x505050, 0);

	frameBuffer->paletteSetColor(0xA, 0xFF0000, 0);
	frameBuffer->paletteSetColor(0xB, 0x00FF00, 0);
	frameBuffer->paletteSetColor(0xC, 0xFFFF00, 0);
	frameBuffer->paletteSetColor(0xD, 0x0000FF, 0);
	frameBuffer->paletteSetColor(0xE, 0xFF00FF, 0);
	frameBuffer->paletteSetColor(0xF, 0x00FFFF, 0);
	frameBuffer->paletteSetColor(0x10, 0xFFFFFF, 0);

	frameBuffer->paletteSet();
}

void CNeutrinoApp::InitMainMenu(CMenuWidget &mainMenu, CMenuWidget &mainSettings,  CMenuWidget &audioSettings, CMenuWidget &parentallockSettings,
                                CMenuWidget &networkSettings, CMenuWidget &streamingSettings, CMenuWidget &colorSettings, CMenuWidget &lcdSettings,
								CMenuWidget &keySettings, CMenuWidget &videoSettings, CMenuWidget &languageSettings, CMenuWidget &miscSettings, 
								CMenuWidget &service, CMenuWidget &fontSettings)
{
	dprintf(DEBUG_DEBUG, "init mainmenue\n");
	mainMenu.addItem( new CMenuSeparator() );
	mainMenu.addItem( new CMenuForwarder("mainmenu.tvmode", true, "", this, "tv", true, CRCInput::RC_red, "rot.raw"), true );
	mainMenu.addItem( new CMenuForwarder("mainmenu.radiomode", true, "", this, "radio", true, CRCInput::RC_green, "gruen.raw") );
	mainMenu.addItem( new CMenuForwarder("mainmenu.scartmode", true, "", this, "scart", true, CRCInput::RC_yellow, "gelb.raw") );
	mainMenu.addItem( new CMenuForwarder("mainmenu.games", true, "", new CGameList("mainmenu.games"), "", true, CRCInput::RC_blue, "blau.raw") );

	mainMenu.addItem( new CMenuForwarder("mainmenu.sleeptimer", true, "", new CSleepTimerWidget, "",true) );

	mainMenu.addItem( new CMenuForwarder("mainmenu.shutdown", true, "", this, "shutdown", true, CRCInput::RC_standby, "power.raw") );
	mainMenu.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

/*
	if(g_settings.network_streaming_use)
	{
		CMenuOptionChooser* oj = new CMenuOptionChooser("mainmenu.streaming", &streamstatus, true, this );
		oj->addOption(0, "mainmenu.streaming_start");
		oj->addOption(1, "mainmenu.streaming_stop");
		mainMenu.addItem( oj );
		mainMenu.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	}
*/
	mainMenu.addItem( new CMenuForwarder("mainmenu.settings", true, "", &mainSettings) );
	mainMenu.addItem( new CMenuForwarder("mainmenu.service", true, "", &service) );
//	mainMenu.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
//	mainMenu.addItem( new CMenuForwarder("mainmenu.info", true, "", new CDBoxInfoWidget, "",true) );


	mainSettings.addItem( new CMenuSeparator() );
	mainSettings.addItem( new CMenuForwarder("menu.back") );
	mainSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	mainSettings.addItem( new CMenuForwarder("mainsettings.savesettingsnow", true, "", this, "savesettings") );
	mainSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	mainSettings.addItem( new CMenuForwarder("mainsettings.video", true, "", &videoSettings) );
	mainSettings.addItem( new CMenuForwarder("mainsettings.audio", true, "", &audioSettings) );
	mainSettings.addItem( new CLockedMenuForwarder("parentallock.parentallock", g_settings.parentallock_pincode, true, "", &parentallockSettings) );
	mainSettings.addItem( new CMenuForwarder("mainsettings.network", true, "", &networkSettings) );
	mainSettings.addItem( new CMenuForwarder("mainsettings.streaming", true, "", &streamingSettings) );
	mainSettings.addItem( new CMenuForwarder("mainsettings.language", true, "", &languageSettings ) );
	mainSettings.addItem( new CMenuForwarder("mainsettings.colors", true,"", &colorSettings) );
	mainSettings.addItem( new CMenuForwarder("mainsettings.lcd", true,"", &lcdSettings) );
	mainSettings.addItem( new CMenuForwarder("mainsettings.keybinding", true,"", &keySettings) );
	mainSettings.addItem( new CMenuForwarder("mainsettings.misc", true, "", &miscSettings ) );

}


void CNeutrinoApp::InitScanSettings(CMenuWidget &settings)
{
	dprintf(DEBUG_DEBUG, "init scansettings\n");
	CMenuOptionChooser* ojBouquets = new CMenuOptionChooser("scants.bouquet", &((int)(scanSettings.bouquetMode)), true );
	ojBouquets->addOption( CZapitClient::BM_DELETEBOUQUETS, "scants.bouquet_erase");
	ojBouquets->addOption( CZapitClient::BM_CREATEBOUQUETS, "scants.bouquet_create");
	ojBouquets->addOption( CZapitClient::BM_DONTTOUCHBOUQUETS, "scants.bouquet_leave");

	//kabel-lnb-settings
	if (g_info.fe==1)
	{
		settings.addItem( new CMenuSeparator() );
		settings.addItem( new CMenuForwarder("menu.back") );
		settings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

		CZapitClient::SatelliteList satList;
		g_Zapit->getScanSatelliteList(satList);
		CMenuOptionStringChooser* ojSat = new CMenuOptionStringChooser("satsetup.satellite", (char*)&scanSettings.satNameNoDiseqc, scanSettings.diseqcMode == NO_DISEQC/*, new CSatelliteNotifier*/, NULL, false);
		for ( uint i=0; i< satList.size(); i++)
		{
			ojSat->addOption(satList[i].satName);
			dprintf(DEBUG_DEBUG, "got scanprovider (sat): %s\n", satList[i].satName );
		}

		CMenuOptionChooser* ojDiseqcRepeats = new CMenuOptionChooser("satsetup.diseqcrepeat", &((int)(scanSettings.diseqcRepeat)), scanSettings.diseqcMode != NO_DISEQC/*, new CSatelliteNotifier*/, NULL, false);
		for ( uint i=0; i<=2; i++)
		{
			char ii[2];
			sprintf( ii, "%d", i);
			ojDiseqcRepeats->addOption(i, ii);
		}

		CMenuWidget* extSatSettings = new CMenuWidget("satsetup.extended", "settings.raw");
		extSatSettings->addItem( new CMenuSeparator() );
		extSatSettings->addItem( new CMenuForwarder("menu.back") );
		extSatSettings->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
		CMenuForwarder* ojExtSatSettings = new CMenuForwarder("satsetup.extended", scanSettings.diseqcMode != NO_DISEQC, "", extSatSettings);
		for ( uint i=0; i< satList.size(); i++)
		{
			CMenuOptionChooser* oj = new CMenuOptionChooser( satList[i].satName, scanSettings.diseqscOfSat( satList[i].satName), true/*, new CSatelliteNotifier*/);
			oj->addOption( -1, "options.off");
			for ( int j=0; j<=3; j++)
			{
				char jj[2];
				sprintf( jj, "%d", j);
				oj->addOption( j, jj);
			}
			extSatSettings->addItem( oj);
		}

		CMenuOptionChooser* ojDiseqc = new CMenuOptionChooser("satsetup.disqeqc", &((int)(scanSettings.diseqcMode)), true, new CSatDiseqcNotifier( ojSat, ojExtSatSettings, ojDiseqcRepeats));
		ojDiseqc->addOption( NO_DISEQC,   "satsetup.nodiseqc");
		ojDiseqc->addOption( MINI_DISEQC, "satsetup.minidiseqc");
		ojDiseqc->addOption( DISEQC_1_0,  "satsetup.diseqc10");
		ojDiseqc->addOption( DISEQC_1_1,  "satsetup.diseqc11");
		ojDiseqc->addOption( SMATV_REMOTE_TUNING,  "satsetup.smatvremote");

		settings.addItem( ojDiseqc );
		settings.addItem( ojBouquets);
		settings.addItem( ojSat);
		settings.addItem( ojDiseqcRepeats );
		settings.addItem( ojExtSatSettings);

	}
	else
	{//kabel
		settings.addItem( new CMenuSeparator() );
		settings.addItem( new CMenuForwarder("menu.back") );
		settings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

/*		static int dummy = 0;
		FILE* fd = fopen("/var/etc/.specinv", "r");
		if(fd)
		{
			dummy=1;
			fclose(fd);
		}
		CMenuOptionChooser* ojInv = new CMenuOptionChooser("cablesetup.spectralInversion", &dummy, true, new CCableSpectalInversionNotifier );
		ojInv->addOption(0, "options.off");
		ojInv->addOption(1, "options.on");

		CZapitClient::SatelliteList providerList;
		g_Zapit->getScanSatelliteList(providerList);
		static int cableProvider = 0;
		for ( uint i=0; i<providerList.size(); i++)
		{
			if( !strcmp( providerList[i].satName, scanSettings.satellites[0].name))
			{
				cableProvider = i;
				break;
			}
		}
*/
		CZapitClient::SatelliteList providerList;
		g_Zapit->getScanSatelliteList(providerList);

		CMenuOptionStringChooser* oj = new CMenuOptionStringChooser("cablesetup.provider", (char*)&scanSettings.satNameNoDiseqc, true/*, new CCableProviderNotifier*/);

		for ( uint i=0; i< providerList.size(); i++)
		{
			oj->addOption( providerList[i].satName);
			dprintf(DEBUG_DEBUG, "got scanprovider (cable): %s\n", providerList[i].satName );
		}
		settings.addItem( ojBouquets);
//		settings.addItem( ojInv );
		settings.addItem( oj);
	}

	settings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	settings.addItem( new CMenuForwarder("scants.startnow", true, "", new CScanTs() ) );

}


void CNeutrinoApp::InitServiceSettings(CMenuWidget &service, CMenuWidget &scanSettings)
{
	dprintf(DEBUG_DEBUG, "init serviceSettings\n");
	service.addItem( new CMenuSeparator() );
	service.addItem( new CMenuForwarder("menu.back") );
	service.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	service.addItem( new CMenuForwarder("bouqueteditor.name", true, "", new CBEBouquetWidget()));
	service.addItem( new CMenuForwarder("servicemenu.scants", true, "", &scanSettings ) );
	service.addItem( new CMenuForwarder("servicemenu.ucodecheck", true, "", UCodeChecker ) );
//	service.addItem( new CMenuForwarder("timerlist.name", true, "", new CTimerList() ) );

	//softupdate
	if (softupdate)
	{
		dprintf(DEBUG_DEBUG, "init soft-update-stuff\n");
		CMenuWidget* updateSettings = new CMenuWidget("servicemenu.update", "softupdate.raw", 450);
		updateSettings->addItem( new CMenuSeparator() );
		updateSettings->addItem( new CMenuForwarder("menu.back") );
		updateSettings->addItem( new CMenuSeparator(CMenuSeparator::LINE) );


		//experten-funktionen für mtd lesen/schreiben
		CMenuWidget* mtdexpert = new CMenuWidget("flashupdate.expertfunctions", "softupdate.raw");
		mtdexpert->addItem( new CMenuSeparator() );
		mtdexpert->addItem( new CMenuForwarder("menu.back") );
		mtdexpert->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
		CFlashExpert* fe = new CFlashExpert();
		mtdexpert->addItem( new CMenuForwarder("flashupdate.readflash", true, "", fe, "readflash") );
		mtdexpert->addItem( new CMenuForwarder("flashupdate.writeflash", true, "", fe, "writeflash") );
		mtdexpert->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
		mtdexpert->addItem( new CMenuForwarder("flashupdate.readflashmtd", true, "", fe, "readflashmtd") );
		mtdexpert->addItem( new CMenuForwarder("flashupdate.writeflashmtd", true, "", fe, "writeflashmtd") );
		updateSettings->addItem( new CMenuForwarder("flashupdate.expertfunctions", true, "", mtdexpert ) );

		updateSettings->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
		CMenuOptionChooser *oj = new CMenuOptionChooser("flashupdate.updatemode", &g_settings.softupdate_mode,true);
			oj->addOption(0, "flashupdate.updatemode_manual");
			oj->addOption(1, "flashupdate.updatemode_internet");
		updateSettings->addItem( oj );


		//get current flash-version SBBBYYYYMMTTHHMM -- formatsting
		CConfigFile configfile('\t');
		if(!configfile.loadConfig("/.version"))
		{
			//error default
			strcpy(g_settings.softupdate_currentversion, "0105200205212015");
		}
		else
		{
			strcpy(g_settings.softupdate_currentversion, configfile.getString( "version", "0105200205212015").c_str());
		}
		dprintf(DEBUG_INFO, "current flash-version: %s\n", g_settings.softupdate_currentversion);

		//aktuelle Version anzeigen
		updateSettings->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "flashupdate.currentversion_sep") );

		//updateSettings->addItem( new CMenuForwarder("flashupdate.currentversion", false, (char*) &g_settings.softupdate_currentversion, NULL ));
		CFlashVersionInfo versionInfo(g_settings.softupdate_currentversion);
		static char date[50];
		strcpy(date, versionInfo.getDate().c_str());
		updateSettings->addItem( new CMenuForwarder("flashupdate.currentversiondate", false, (char*) &date, NULL ));
		static char time[50];
		strcpy(time, versionInfo.getTime().c_str());
		updateSettings->addItem( new CMenuForwarder("flashupdate.currentversiontime", false, (char*) &time, NULL ));
		static char baseimage[50];
		strcpy(baseimage, versionInfo.getBaseImageVersion().c_str());
		updateSettings->addItem( new CMenuForwarder("flashupdate.currentversionbaseversion", false, (char*) &baseimage, NULL ));
		static char releasetype[50];
		strcpy(releasetype, versionInfo.getType().c_str());
		updateSettings->addItem( new CMenuForwarder("flashupdate.currentversionsnapshot", false, (char*) &releasetype, NULL ));

		updateSettings->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "flashupdate.proxyserver_sep") );

		CStringInputSMS*	updateSettings_proxy= new CStringInputSMS("flashupdate.proxyserver", g_settings.softupdate_proxyserver, 23,
		                         "flashupdate.proxyserver_hint1", "flashupdate.proxyserver_hint2",
		                         "abcdefghijklmnopqrstuvwxyz0123456789-.: ");
		updateSettings->addItem( new CMenuForwarder("flashupdate.proxyserver", true, g_settings.softupdate_proxyserver, updateSettings_proxy ) );

		CStringInputSMS*	updateSettings_proxyuser= new CStringInputSMS("flashupdate.proxyusername", g_settings.softupdate_proxyusername, 23,
		        "flashupdate.proxyusername_hint1", "flashupdate.proxyusername_hint2",
		        "abcdefghijklmnopqrstuvwxyz0123456789!""§$%&/()=?-. ");
		updateSettings->addItem( new CMenuForwarder("flashupdate.proxyusername", true, g_settings.softupdate_proxyusername, updateSettings_proxyuser ) );

		CStringInputSMS*	updateSettings_proxypass= new CStringInputSMS("flashupdate.proxypassword", g_settings.softupdate_proxypassword, 20,
		        "flashupdate.proxypassword_hint1", "flashupdate.proxypassword_hint2",
		        "abcdefghijklmnopqrstuvwxyz0123456789!""§$%&/()=?-. ");
		updateSettings->addItem( new CMenuForwarder("flashupdate.proxypassword", true, g_settings.softupdate_proxypassword, updateSettings_proxypass ) );

		updateSettings->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
		updateSettings->addItem( new CMenuForwarder("flashupdate.checkupdate", true, "", new CFlashUpdate() ));

		service.addItem( new CMenuForwarder("servicemenu.update", true, "", updateSettings ) );

	}
}

void CNeutrinoApp::InitMiscSettings(CMenuWidget &miscSettings)
{
	dprintf(DEBUG_DEBUG, "init miscsettings\n");
	miscSettings.addItem( new CMenuSeparator() );
	miscSettings.addItem( new CMenuForwarder("menu.back") );
	miscSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "miscsettings.general" ) );

/*	CMenuOptionChooser *oj = new CMenuOptionChooser("miscsettings.boxtype", &g_settings.box_Type, false, NULL, false );
	oj->addOption(1, "Nokia");
	oj->addOption(2, "Sagem");
	oj->addOption(3, "Philips");
	miscSettings.addItem( oj );
*/
	CMenuOptionChooser *oj = new CMenuOptionChooser("miscsettings.shutdown_real", &g_settings.shutdown_real, true );
	oj->addOption(1, "options.off");
	oj->addOption(0, "options.on");
	miscSettings.addItem( oj );

	if (fromflash)
	{
		static int dummy = 0;
		FILE* fd = fopen("/var/etc/.neutrino", "r");
		if(fd)
		{
			dummy=1;
			fclose(fd);
		}
		oj = new CMenuOptionChooser("miscsettings.startneutrinodirect", &dummy, true, new CStartNeutrinoDirectNotifier );
		oj->addOption(0, "options.off");
		oj->addOption(1, "options.on");
		miscSettings.addItem( oj );


		static int fb_destination = 0;
		fd = fopen("/var/tuxbox/boot/ppcboot.conf", "r");
		if(fd)
		{
			char buffer[200];
			if(fgets(buffer,199,fd) != NULL)
			{
				if(strncmp(buffer,"console=",8) == 0)
				{
					if(strncmp(&buffer[8],"null",4)==0)
						fb_destination=0;
					else if(strncmp(&buffer[8],"ttyS0",5)==0)
						fb_destination=1;
					else if(strncmp(&buffer[8],"tty",3)==0)
						fb_destination=2;
				}
				else
					printf("no console string found in ppcboot.conf\n");

			}
			fclose(fd);
		}
		oj = new CMenuOptionChooser("miscsettings.fb_destination", &fb_destination, true, ConsoleDestinationChanger );
		oj->addOption(0, "options.null");
		oj->addOption(1, "options.serial");
		oj->addOption(2, "options.fb");
		miscSettings.addItem( oj );
	}

	keySetupNotifier = new CKeySetupNotifier;
	CStringInput*	keySettings_repeat_genericblocker= new CStringInput("keybindingmenu.repeatblockgeneric", g_settings.repeat_blocker, 3, "repeatblocker.hint_1", "repeatblocker.hint_2", "0123456789 ", keySetupNotifier);
	CStringInput*	keySettings_repeatBlocker= new CStringInput("keybindingmenu.repeatblock", g_settings.repeat_genericblocker, 3, "repeatblocker.hint_1", "repeatblocker.hint_2", "0123456789 ", keySetupNotifier);
	keySetupNotifier->changeNotify("initial", NULL);

	miscSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "keybindingmenu.RC") );
	miscSettings.addItem( new CMenuForwarder("keybindingmenu.repeatblock", true, "", keySettings_repeatBlocker ));
	miscSettings.addItem( new CMenuForwarder("keybindingmenu.repeatblockgeneric", true, "", keySettings_repeat_genericblocker ));
	
	CStringInput*	timerSettings_record_safety_time= new CStringInput("timersettings.record_safety_time", g_settings.record_safety_time, 2, "ipsetup.hint_1", "ipsetup.hint_2","0123456789 ");
	miscSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "timersettings.separator") );
    miscSettings.addItem( new CMenuForwarder("timersettings.record_safety_time", true, g_settings.record_safety_time, timerSettings_record_safety_time ));

	oj = new CMenuOptionChooser("miscsettings.spdif_control", &g_settings.audio_SPDIF_Control, true);
	oj->addOption(0, "options.off");
	oj->addOption(1, "options.on");
	miscSettings.addItem( oj );
}


void CNeutrinoApp::InitLanguageSettings(CMenuWidget &languageSettings)
{
	languageSettings.addItem( new CMenuSeparator() );
	languageSettings.addItem( new CMenuForwarder("menu.back") );
	languageSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	languageSetupNotifier = new CLanguageSetupNotifier;
	CMenuOptionStringChooser* oj = new CMenuOptionStringChooser("languagesetup.select", (char*)&g_settings.language, true, languageSetupNotifier, false);
	//search available languages....

	struct dirent **namelist;
	int n;
	//		printf("scanning locale dir now....(perhaps)\n");

	char *pfad[] = {DATADIR "/neutrino/locale","/var/tuxbox/config/locale"};
	string filen, locale;
	int pos;

	for(int p = 0;p < 2;p++)
	{
		n = scandir(pfad[p], &namelist, 0, alphasort);
		if (n < 0)
		{
			perror("loading locales: scandir");
		}
		else
		{
			for(int count=0;count<n;count++)
			{
				filen = namelist[count]->d_name;
				pos = filen.find(".locale");
				if(pos!=-1)
				{
					locale = filen.substr(0,pos);
					oj->addOption( locale );
				}
				free(namelist[count]);
			}
			free(namelist);
		}
	}
	languageSettings.addItem( oj );
}

void CNeutrinoApp::InitAudioSettings(CMenuWidget &audioSettings, CAudioSetupNotifier* audioSetupNotifier)
{
	dprintf(DEBUG_DEBUG, "init audiosettings\n");
	audioSettings.addItem( new CMenuSeparator() );
	audioSettings.addItem( new CMenuForwarder("menu.back") );
	audioSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CMenuOptionChooser* oj = new CMenuOptionChooser("audiomenu.analogout", &g_settings.audio_AnalogMode, true, audioSetupNotifier);
		oj->addOption(0, "audiomenu.stereo");
		oj->addOption(1, "audiomenu.monoleft");
		oj->addOption(2, "audiomenu.monoright");

	audioSettings.addItem( oj );
		oj = new CMenuOptionChooser("audiomenu.dolbydigital", &g_settings.audio_DolbyDigital, true, audioSetupNotifier);
		oj->addOption(0, "options.off");
		oj->addOption(1, "options.on");
	audioSettings.addItem( oj );
}

void CNeutrinoApp::InitVideoSettings(CMenuWidget &videoSettings, CVideoSetupNotifier* videoSetupNotifier)
{
	dprintf(DEBUG_DEBUG, "init videosettings\n");
	videoSettings.addItem( new CMenuSeparator() );
	videoSettings.addItem( new CMenuForwarder("menu.back") );
	videoSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CMenuOptionChooser* oj = new CMenuOptionChooser("videomenu.videosignal", &g_settings.video_Signal, true, videoSetupNotifier);
	oj->addOption(1, "videomenu.videosignal_rgb");
	oj->addOption(2, "videomenu.videosignal_svideo");
	oj->addOption(0, "videomenu.videosignal_composite");

	videoSettings.addItem( oj );

	oj = new CMenuOptionChooser("videomenu.videoformat", &g_settings.video_Format, true, videoSetupNotifier);
	oj->addOption(2, "videomenu.videoformat_43");
	oj->addOption(1, "videomenu.videoformat_169");
	oj->addOption(0, "videomenu.videoformat_autodetect");

	if (g_settings.video_Format==0) // autodetect has to be initialized
	{
		videoSetupNotifier->changeNotify("videomenu.videoformat", NULL);
	}

	videoSettings.addItem( oj );

	oj = new CMenuOptionChooser("videomenu.vcrswitch", &g_settings.vcr_AutoSwitch, true, NULL);
	oj->addOption(0, "options.off");
	oj->addOption(1, "options.on");
	videoSettings.addItem( oj );

	videoSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	videoSettings.addItem( new CMenuForwarder("videomenu.screensetup", true, "", new CScreenSetup() ) );
}

void CNeutrinoApp::InitParentalLockSettings(CMenuWidget &parentallockSettings)
{
	dprintf(DEBUG_DEBUG, "init parentallocksettings\n");
	parentallockSettings.addItem( new CMenuSeparator() );
	parentallockSettings.addItem( new CMenuForwarder("menu.back") );
	parentallockSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CMenuOptionChooser* oj = new CMenuOptionChooser("parentallock.prompt", &g_settings.parentallock_prompt, true);
	oj->addOption(PARENTALLOCK_PROMPT_NEVER         , "parentallock.never");
	oj->addOption(PARENTALLOCK_PROMPT_ONSTART       , "parentallock.onstart");
	//oj->addOption(PARENTALLOCK_PROMPT_CHANGETOLOCKED, "parentallock.changetolocked");
	oj->addOption(PARENTALLOCK_PROMPT_ONSIGNAL      , "parentallock.onsignal");
	parentallockSettings.addItem( oj );

	oj = new CMenuOptionChooser("parentallock.lockage", &g_settings.parentallock_lockage, true);
	oj->addOption(12, "parentallock.lockage12");
	oj->addOption(16, "parentallock.lockage16");
	oj->addOption(18, "parentallock.lockage18");
	parentallockSettings.addItem( oj );

	CPINChangeWidget* pinChangeWidget = new CPINChangeWidget("parentallock.changepin", g_settings.parentallock_pincode, 4, "parentallock.changepin_hint1", "");
	parentallockSettings.addItem( new CMenuForwarder("parentallock.changepin", true, g_settings.parentallock_pincode, pinChangeWidget));
}

void CNeutrinoApp::InitNetworkSettings(CMenuWidget &networkSettings)
{
	dprintf(DEBUG_DEBUG, "init networksettings\n");
	networkSettings.addItem( new CMenuSeparator() );
	networkSettings.addItem( new CMenuForwarder("menu.back") );
	networkSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CMenuOptionChooser* oj = new CMenuOptionChooser("networkmenu.setuponstartup", &g_settings.networkSetOnStartup, true);
	oj->addOption(0, "options.off");
	oj->addOption(1, "options.on");

	networkSettings.addItem( oj );
	networkSettings.addItem( new CMenuForwarder("networkmenu.test", true, "", this, "networktest") );
	networkSettings.addItem( new CMenuForwarder("networkmenu.show", true, "", this, "networkshow") );
	CMenuForwarder *m0 = new CMenuForwarder("networkmenu.setupnow", g_settings.network_dhcp==0, "", this, "network");
	networkSettings.addItem( m0 );

	networkSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CIPInput*	networkSettings_NetworkIP= new CIPInput("networkmenu.ipaddress", g_settings.network_ip, "ipsetup.hint_1", "ipsetup.hint_2", MyIPChanger);
	CIPInput*	networkSettings_NetMask= new CIPInput("networkmenu.netmask", g_settings.network_netmask, "ipsetup.hint_1", "ipsetup.hint_2");
	CIPInput*	networkSettings_Broadcast= new CIPInput("networkmenu.broadcast", g_settings.network_broadcast, "ipsetup.hint_1", "ipsetup.hint_2");
	CIPInput*	networkSettings_Gateway= new CIPInput("networkmenu.gateway", g_settings.network_defaultgateway, "ipsetup.hint_1", "ipsetup.hint_2");
	CIPInput*	networkSettings_NameServer= new CIPInput("networkmenu.nameserver", g_settings.network_nameserver, "ipsetup.hint_1", "ipsetup.hint_2");

	CMenuForwarder *m1 = new CMenuForwarder("networkmenu.ipaddress", g_settings.network_dhcp==0, g_settings.network_ip, networkSettings_NetworkIP );
	CMenuForwarder *m2 = new CMenuForwarder("networkmenu.netmask", g_settings.network_dhcp==0, g_settings.network_netmask, networkSettings_NetMask );
	CMenuForwarder *m3 = new CMenuForwarder("networkmenu.broadcast", g_settings.network_dhcp==0, g_settings.network_broadcast, networkSettings_Broadcast );
	CMenuForwarder *m4 = new CMenuForwarder("networkmenu.gateway", g_settings.network_dhcp==0, g_settings.network_defaultgateway, networkSettings_Gateway );
	CMenuForwarder *m5 = new CMenuForwarder("networkmenu.nameserver", g_settings.network_dhcp==0, g_settings.network_nameserver, networkSettings_NameServer );

	CDHCPNotifier* dhcpNotifier = new CDHCPNotifier(m1,m2,m3,m4,m5, m0);
	if(g_settings.networkSetOnStartup)
	{
		dhcpNotifier->startStopDhcp();
	}
	oj = new CMenuOptionChooser("dhcp", &g_settings.network_dhcp, true, dhcpNotifier);
		oj->addOption(0, "options.off");
		oj->addOption(1, "options.on");
	networkSettings.addItem( oj );
	networkSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	networkSettings.addItem( m1);
	networkSettings.addItem( m2);
	networkSettings.addItem( m3);

	networkSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	networkSettings.addItem( m4);
	networkSettings.addItem( m5);
}

void CNeutrinoApp::InitStreamingSettings(CMenuWidget &streamingSettings)
{
	dprintf(DEBUG_DEBUG, "init streamingsettings\n");
	streamingSettings.addItem( new CMenuSeparator() );
	streamingSettings.addItem( new CMenuForwarder("menu.back") );
	streamingSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CMenuOptionChooser* oj = new CMenuOptionChooser("streamingmenu.usestreamserver", &g_settings.network_streaming_use, true);
	oj->addOption(0, "options.off");
	oj->addOption(1, "options.on");
	streamingSettings.addItem( oj );

	CIPInput*	streamingSettings_streamingserver= new CIPInput("streamingmenu.streamingserver",  g_settings.network_streamingserver, "ipsetup.hint_1", "ipsetup.hint_2");
	CStringInput*	streamingSettings_streamingserverport= new CStringInput("streamingmenu.streamingserverport", g_settings.network_streamingserverport, 6, "ipsetup.hint_1", "ipsetup.hint_2","0123456789 ");

	streamingSettings.addItem( new CMenuForwarder("streamingmenu.streamingserver", true, g_settings.network_streamingserver,streamingSettings_streamingserver));
	streamingSettings.addItem( new CMenuForwarder("streamingmenu.streamingserverport", true, g_settings.network_streamingserverport,streamingSettings_streamingserverport));

	streamingSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	oj = new CMenuOptionChooser("streamingmenu.stopplayback", &g_settings.network_streaming_stopplayback, true);
	oj->addOption(0, "options.off");
	oj->addOption(1, "options.on");
	streamingSettings.addItem( oj );

	oj = new CMenuOptionChooser("streamingmenu.stopsectionsd", &g_settings.network_streaming_stopsectionsd, true);
	oj->addOption(0, "options.off");
	oj->addOption(1, "options.on");
	streamingSettings.addItem( oj );
	
	streamingSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	CStringInput*	streamingSettings_vcr_devicename= new CStringInputSMS("streamingmenu.vcr_devicename", g_settings.vcr_devicename, 20, "ipsetup.hint_1", "ipsetup.hint_2","abcdefghijklmnopqrstuvwxyz0123456789-. ");
	CMenuForwarder *m1 = new CMenuForwarder("streamingmenu.vcr_devicename",g_settings.vcr_recording==1, g_settings.vcr_devicename, streamingSettings_vcr_devicename );
	CRecordingNotifier* recordingNotifier = new CRecordingNotifier(m1);
	
	oj = new CMenuOptionChooser("streamingmenu.vcr_recording", &g_settings.vcr_recording, true, recordingNotifier);
	oj->addOption(0, "options.off");
	oj->addOption(1, "options.on");
	streamingSettings.addItem( oj );
	
	streamingSettings.addItem( new CMenuForwarder("streamingmenu.vcr_devicename", true, g_settings.vcr_devicename,streamingSettings_vcr_devicename));
	
	CStringInput	*timerSettings_record_safety_time= new CStringInput("timersettings.record_safety_time", g_settings.record_safety_time, 2, "ipsetup.hint_1", "ipsetup.hint_2","0123456789 ");
	streamingSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "timersettings.separator") );
	streamingSettings.addItem( new CMenuForwarder("timersettings.record_safety_time", true, g_settings.record_safety_time, timerSettings_record_safety_time ));

	streamstatus = 0;

}

void CNeutrinoApp::AddFontSettingItem(CMenuWidget &fontSettings, string menuname, char *value)
{
	CStringInput *fontSize;
	fontSize = new CStringInput(menuname, value, 3, "ipsetup.hint_1", "ipsetup.hint_2","0123456789 ",this);
	fontSettings.addItem( new CMenuForwarder(menuname, true, value ,fontSize));
}

void CNeutrinoApp::InitFontSettings(CMenuWidget &fontSettings,CMenuWidget &fontSettings_Channellist , CMenuWidget &fontSettings_Eventlist , CMenuWidget &fontSettings_Infobar ,CMenuWidget &fontSettings_Epg )
{
	fontSettings_Epg.addItem( new CMenuSeparator() );
	fontSettings_Epg.addItem( new CMenuForwarder("menu.back") );
	fontSettings_Epg.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	AddFontSettingItem(fontSettings_Epg, "fontsize.epg_title", g_settings.fontsize_epg_title);
	AddFontSettingItem(fontSettings_Epg, "fontsize.epg_info1", g_settings.fontsize_epg_info1);
	AddFontSettingItem(fontSettings_Epg, "fontsize.epg_info2", g_settings.fontsize_epg_info2);
	AddFontSettingItem(fontSettings_Epg, "fontsize.epg_date", g_settings.fontsize_epg_date);

	fontSettings_Eventlist.addItem( new CMenuSeparator() );
	fontSettings_Eventlist.addItem( new CMenuForwarder("menu.back") );
	fontSettings_Eventlist.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	AddFontSettingItem(fontSettings_Eventlist, "fontsize.eventlist_title", g_settings.fontsize_eventlist_title);
	AddFontSettingItem(fontSettings_Eventlist, "fontsize.eventlist_itemlarge", g_settings.fontsize_eventlist_itemlarge);
	AddFontSettingItem(fontSettings_Eventlist, "fontsize.eventlist_itemsmall", g_settings.fontsize_eventlist_itemsmall);
	AddFontSettingItem(fontSettings_Eventlist, "fontsize.eventlist_datetime", g_settings.fontsize_eventlist_datetime);
//	fontSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
//	AddFontSettingItem(fontSettings, "fontsize.gamelist_itemLarge", g_settings.fontsize_gamelist_itemLarge);
//	AddFontSettingItem(fontSettings, "fontsize.gamelist_itemSmall", g_settings.fontsize_gamelist_itemSmall);

	fontSettings_Channellist.addItem( new CMenuSeparator() );
	fontSettings_Channellist.addItem( new CMenuForwarder("menu.back") );
	fontSettings_Channellist.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	AddFontSettingItem(fontSettings_Channellist, "fontsize.channellist", g_settings.fontsize_channellist);
	AddFontSettingItem(fontSettings_Channellist, "fontsize.channellist_descr", g_settings.fontsize_channellist_descr);
	AddFontSettingItem(fontSettings_Channellist, "fontsize.channellist_number", g_settings.fontsize_channellist_number);
	AddFontSettingItem(fontSettings_Channellist, "fontsize.channel_num_zap", g_settings.fontsize_channel_num_zap);
	
	fontSettings_Infobar.addItem( new CMenuSeparator() );
	fontSettings_Infobar.addItem( new CMenuForwarder("menu.back") );
	fontSettings_Infobar.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	AddFontSettingItem(fontSettings_Infobar, "fontsize.infobar_number", g_settings.fontsize_infobar_number);
	AddFontSettingItem(fontSettings_Infobar, "fontsize.infobar_channame", g_settings.fontsize_infobar_channame);
	AddFontSettingItem(fontSettings_Infobar, "fontsize.infobar_info", g_settings.fontsize_infobar_info);
	AddFontSettingItem(fontSettings_Infobar, "fontsize.infobar_small", g_settings.fontsize_infobar_small);

	fontSettings.addItem( new CMenuSeparator() );
	fontSettings.addItem( new CMenuForwarder("menu.back") );
	fontSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	AddFontSettingItem(fontSettings, "fontsize.alert", g_settings.fontsize_alert);
	fontSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	AddFontSettingItem(fontSettings, "fontsize.menu", g_settings.fontsize_menu);
	AddFontSettingItem(fontSettings, "fontsize.menu_title", g_settings.fontsize_menu_title);
	AddFontSettingItem(fontSettings, "fontsize.menu_info", g_settings.fontsize_menu_info);
	fontSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	fontSettings.addItem( new CMenuForwarder("fontmenu.channellist", true,"", &fontSettings_Channellist) );
	fontSettings.addItem( new CMenuForwarder("fontmenu.eventlist", true,"", &fontSettings_Eventlist) );
	fontSettings.addItem( new CMenuForwarder("fontmenu.epg", true,"", &fontSettings_Epg) );
	fontSettings.addItem( new CMenuForwarder("fontmenu.infobar", true,"", &fontSettings_Infobar) );
}

void CNeutrinoApp::InitColorSettings(CMenuWidget &colorSettings, CMenuWidget &fontSettings )
{
	dprintf(DEBUG_DEBUG, "init colorsettings\n");
	colorSettings.addItem( new CMenuSeparator() );
	colorSettings.addItem( new CMenuForwarder("menu.back") );
	colorSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CMenuWidget *colorSettings_Themes = new CMenuWidget("colorthememenu.head", "settings.raw");
	InitColorThemesSettings(*colorSettings_Themes);

	colorSettings.addItem( new CMenuForwarder("colormenu.themeselect", true, "", colorSettings_Themes) );
	colorSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	colorSettings.addItem( new CMenuForwarder("colormenu.font", true,"", &fontSettings) );
	colorSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CMenuWidget *colorSettings_menuColors = new CMenuWidget("colormenusetup.head", "settings.raw", 400, 400);
	InitColorSettingsMenuColors(*colorSettings_menuColors);
	colorSettings.addItem( new CMenuForwarder("colormenu.menucolors", true, "", colorSettings_menuColors) );

	CMenuWidget *colorSettings_statusbarColors = new CMenuWidget("colormenu.statusbar", "settings.raw");
	InitColorSettingsStatusBarColors(*colorSettings_statusbarColors);
	colorSettings.addItem( new CMenuForwarder("colorstatusbar.head", true, "", colorSettings_statusbarColors) );

	colorSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CMenuOptionChooser* oj = new CMenuOptionChooser("colormenu.fade", &g_settings.widget_fade, true );
	oj->addOption(0, "options.off");
	oj->addOption(1, "options.on");
	colorSettings.addItem( oj );

}

void CNeutrinoApp::InitColorThemesSettings(CMenuWidget &colorSettings_Themes)
{
	dprintf(DEBUG_DEBUG, "init themesettings\n");
	colorSettings_Themes.addItem( new CMenuSeparator() );
	colorSettings_Themes.addItem( new CMenuForwarder("menu.back") );
	colorSettings_Themes.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	colorSettings_Themes.addItem( new CMenuForwarder("colorthememenu.neutrino_theme", true, "", this, "theme_neutrino") );
	colorSettings_Themes.addItem( new CMenuForwarder("colorthememenu.classic_theme", true, "", this, "theme_classic") );
}

void CNeutrinoApp::InitColorSettingsMenuColors(CMenuWidget &colorSettings_menuColors)
{
	dprintf(DEBUG_DEBUG, "init colormenuesettings\n");
	colorSettings_menuColors.addItem( new CMenuSeparator() );
	colorSettings_menuColors.addItem( new CMenuForwarder("menu.back") );

	CColorChooser* chHeadcolor = new CColorChooser("colormenu.background_head", &g_settings.menu_Head_red, &g_settings.menu_Head_green, &g_settings.menu_Head_blue,
	                             &g_settings.menu_Head_alpha, colorSetupNotifier);
	CColorChooser* chHeadTextcolor = new CColorChooser("colormenu.textcolor_head", &g_settings.menu_Head_Text_red, &g_settings.menu_Head_Text_green, &g_settings.menu_Head_Text_blue,
	                                 NULL, colorSetupNotifier);
	CColorChooser* chContentcolor = new CColorChooser("colormenu.background_head", &g_settings.menu_Content_red, &g_settings.menu_Content_green, &g_settings.menu_Content_blue,
	                                &g_settings.menu_Content_alpha, colorSetupNotifier);
	CColorChooser* chContentTextcolor = new CColorChooser("colormenu.textcolor_head", &g_settings.menu_Content_Text_red, &g_settings.menu_Content_Text_green, &g_settings.menu_Content_Text_blue,
	                                    NULL, colorSetupNotifier);
	CColorChooser* chContentSelectedcolor = new CColorChooser("colormenu.background_head", &g_settings.menu_Content_Selected_red, &g_settings.menu_Content_Selected_green, &g_settings.menu_Content_Selected_blue,
	                                        &g_settings.menu_Content_Selected_alpha, colorSetupNotifier);
	CColorChooser* chContentSelectedTextcolor = new CColorChooser("colormenu.textcolor_head", &g_settings.menu_Content_Selected_Text_red, &g_settings.menu_Content_Selected_Text_green, &g_settings.menu_Content_Selected_Text_blue,
	        NULL, colorSetupNotifier);
	CColorChooser* chContentInactivecolor = new CColorChooser("colormenu.background_head", &g_settings.menu_Content_inactive_red, &g_settings.menu_Content_inactive_green, &g_settings.menu_Content_inactive_blue,
	                                        &g_settings.menu_Content_inactive_alpha, colorSetupNotifier);
	CColorChooser* chContentInactiveTextcolor = new CColorChooser("colormenu.textcolor_head", &g_settings.menu_Content_inactive_Text_red, &g_settings.menu_Content_inactive_Text_green, &g_settings.menu_Content_inactive_Text_blue,
	        NULL, colorSetupNotifier);
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "colormenusetup.menuhead") );
	colorSettings_menuColors.addItem( new CMenuForwarder("colormenu.background", true, "", chHeadcolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder("colormenu.textcolor", true, "", chHeadTextcolor ));
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "colormenusetup.menucontent") );
	colorSettings_menuColors.addItem( new CMenuForwarder("colormenu.background", true, "", chContentcolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder("colormenu.textcolor", true, "", chContentTextcolor ));
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "colormenusetup.menucontent_inactive") );
	colorSettings_menuColors.addItem( new CMenuForwarder("colormenu.background", true, "", chContentInactivecolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder("colormenu.textcolor", true, "", chContentInactiveTextcolor));
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "colormenusetup.menucontent_selected") );
	colorSettings_menuColors.addItem( new CMenuForwarder("colormenu.background", true, "", chContentSelectedcolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder("colormenu.textcolor", true, "", chContentSelectedTextcolor ));
}

void CNeutrinoApp::InitColorSettingsStatusBarColors(CMenuWidget &colorSettings_statusbarColors)
{
	dprintf(DEBUG_DEBUG, "init colorstatusbarsettings\n");
	colorSettings_statusbarColors.addItem( new CMenuSeparator() );

	colorSettings_statusbarColors.addItem( new CMenuForwarder("menu.back") );

	CColorChooser* chInfobarcolor = new CColorChooser("colormenu.background_head", &g_settings.infobar_red, &g_settings.infobar_green, &g_settings.infobar_blue,
	                                &g_settings.infobar_alpha, colorSetupNotifier);
	CColorChooser* chInfobarTextcolor = new CColorChooser("colormenu.textcolor_head", &g_settings.infobar_Text_red, &g_settings.infobar_Text_green, &g_settings.infobar_Text_blue,
	                                    NULL, colorSetupNotifier);

	colorSettings_statusbarColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "colorstatusbar.text") );
	colorSettings_statusbarColors.addItem( new CMenuForwarder("colormenu.background", true, "", chInfobarcolor ));
	colorSettings_statusbarColors.addItem( new CMenuForwarder("colormenu.textcolor", true, "", chInfobarTextcolor ));
}

void CNeutrinoApp::InitLcdSettings(CMenuWidget &lcdSettings)
{
	static int lcdpower = g_lcdd->getPower()?1:0;
	static int lcdinverse = g_lcdd->getInverse()?1:0;
	dprintf(DEBUG_DEBUG, "init lcdsettings\n");
	lcdSettings.addItem( new CMenuSeparator() );

	lcdSettings.addItem( new CMenuForwarder("menu.back") );
	lcdSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CLcdControler* lcdsliders = new CLcdControler("lcdmenu.head", NULL);

	CLcdNotifier* lcdnotifier = new CLcdNotifier(&lcdpower,&lcdinverse);

	CMenuOptionChooser* oj = new CMenuOptionChooser("lcdmenu.inverse", &lcdinverse, true, lcdnotifier );
	oj->addOption(0, "options.off");
	oj->addOption(1, "options.on");
	lcdSettings.addItem( oj );

	oj = new CMenuOptionChooser("lcdmenu.power", &lcdpower, true, lcdnotifier );
	oj->addOption(0, "options.off");
	oj->addOption(1, "options.on");
	lcdSettings.addItem( oj );

	lcdSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	lcdSettings.addItem( new CMenuForwarder("lcdmenu.lcdcontroler", true, "", lcdsliders ));
}

void CNeutrinoApp::InitKeySettings(CMenuWidget &keySettings)
{
	dprintf(DEBUG_DEBUG, "init keysettings\n");
	keySettings.addItem( new CMenuSeparator() );
	keySettings.addItem( new CMenuForwarder("menu.back") );

	CKeyChooser*	keySettings_tvradio_mode = new CKeyChooser(&g_settings.key_tvradio_mode, "keybindingmenu.tvradiomode_head", "settings.raw");
	CKeyChooser*	keySettings_channelList_pageup = new CKeyChooser(&g_settings.key_channelList_pageup, "keybindingmenu.pageup_head", "settings.raw");
	CKeyChooser*	keySettings_channelList_pagedown = new CKeyChooser(&g_settings.key_channelList_pagedown, "keybindingmenu.pagedown_head", "settings.raw");
	CKeyChooser*	keySettings_channelList_cancel = new CKeyChooser(&g_settings.key_channelList_cancel, "keybindingmenu.cancel_head", "settings.raw");
	CKeyChooser*	keySettings_quickzap_up = new CKeyChooser(&g_settings.key_quickzap_up, "keybindingmenu.channelup_head",   "settings.raw");
	CKeyChooser*	keySettings_quickzap_down = new CKeyChooser(&g_settings.key_quickzap_down, "keybindingmenu.channeldown_head", "settings.raw");
	CKeyChooser*	keySettings_bouquet_up = new CKeyChooser(&g_settings.key_bouquet_up, "keybindingmenu.bouquetup_head",   "settings.raw");
	CKeyChooser*	keySettings_bouquet_down = new CKeyChooser(&g_settings.key_bouquet_down, "keybindingmenu.bouquetdown_head", "settings.raw");
	CKeyChooser*	keySettings_subchannel_up = new CKeyChooser(&g_settings.key_subchannel_up, "keybindingmenu.subchannelup_head",   "settings.raw");
	CKeyChooser*	keySettings_subchannel_down = new CKeyChooser(&g_settings.key_subchannel_down, "keybindingmenu.subchanneldown_head", "settings.raw");

	keySettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "keybindingmenu.modechange") );
	keySettings.addItem( new CMenuForwarder("keybindingmenu.tvradiomode", true, "", keySettings_tvradio_mode ));

	keySettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "keybindingmenu.channellist") );
	CMenuOptionChooser *oj = new CMenuOptionChooser("keybindingmenu.bouquethandling" , &g_settings.bouquetlist_mode, true );
	oj->addOption(0, "keybindingmenu.bouquetchannels_on_ok");
	oj->addOption(1, "keybindingmenu.bouquetlist_on_ok");
	oj->addOption(2, "keybindingmenu.allchannels_on_ok");
	keySettings.addItem( oj );
	keySettings.addItem( new CMenuForwarder("keybindingmenu.pageup", true, "", keySettings_channelList_pageup ));
	keySettings.addItem( new CMenuForwarder("keybindingmenu.pagedown", true, "", keySettings_channelList_pagedown ));
	keySettings.addItem( new CMenuForwarder("keybindingmenu.cancel", true, "", keySettings_channelList_cancel ));
	keySettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, "keybindingmenu.quickzap") );
	keySettings.addItem( new CMenuForwarder("keybindingmenu.channelup", true, "", keySettings_quickzap_up ));
	keySettings.addItem( new CMenuForwarder("keybindingmenu.channeldown", true, "", keySettings_quickzap_down ));
	keySettings.addItem( new CMenuForwarder("keybindingmenu.bouquetup", true, "", keySettings_bouquet_up ));
	keySettings.addItem( new CMenuForwarder("keybindingmenu.bouquetdown", true, "", keySettings_bouquet_down ));
	keySettings.addItem( new CMenuForwarder("keybindingmenu.subchannelup", true, "", keySettings_subchannel_up ));
	keySettings.addItem( new CMenuForwarder("keybindingmenu.subchanneldown", true, "", keySettings_subchannel_down ));
}

void CNeutrinoApp::SelectNVOD()
{
	if ( g_RemoteControl->subChannels.size()> 0 )
	{
		// NVOD/SubService- Kanal!
		CMenuWidget NVODSelector( g_RemoteControl->are_subchannels?"nvodselector.subservice":"nvodselector.head", "video.raw", 350);

		NVODSelector.addItem( new CMenuSeparator() );

		int count = 0;
		char nvod_id[5];

		for ( CSubServiceListSorted::iterator e=g_RemoteControl->subChannels.begin(); e!=g_RemoteControl->subChannels.end(); ++e)
		{
			sprintf(nvod_id, "%d", count);

			if ( !g_RemoteControl->are_subchannels )
			{
				char nvod_time_a[50], nvod_time_e[50], nvod_time_x[50];
				char nvod_s[100];
				struct  tm *tmZeit;

				tmZeit= localtime( &e->startzeit );
				sprintf(nvod_time_a, "%02d:%02d", tmZeit->tm_hour, tmZeit->tm_min);

				time_t endtime = e->startzeit+ e->dauer;
				tmZeit= localtime(&endtime);
				sprintf(nvod_time_e, "%02d:%02d", tmZeit->tm_hour, tmZeit->tm_min);

				time_t jetzt=time(NULL);
				if (e->startzeit > jetzt)
				{
					int mins=(e->startzeit- jetzt)/ 60;
					sprintf(nvod_time_x, g_Locale->getText("nvod.starting").c_str(), mins);
				}
				else
					if ( (e->startzeit<= jetzt) && (jetzt < endtime) )
					{
						int proz=(jetzt- e->startzeit)*100/ e->dauer;
						sprintf(nvod_time_x, g_Locale->getText("nvod.proz").c_str(), proz);
					}
					else
						nvod_time_x[0]= 0;

				sprintf(nvod_s, "%s - %s %s", nvod_time_a, nvod_time_e, nvod_time_x);
				NVODSelector.addItem( new CMenuForwarder(nvod_s, true, "", NVODChanger, nvod_id, false), (count == g_RemoteControl->selected_subchannel) );
			}
			else
			{
				NVODSelector.addItem( new CMenuForwarder(e->subservice_name, true, "", NVODChanger, nvod_id, false, (count<10)? (count) : CRCInput::RC_nokey ), (count == g_RemoteControl->selected_subchannel) );
			}

			count++;
		}

		if ( g_RemoteControl->are_subchannels )
		{
			NVODSelector.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
			CMenuOptionChooser* oj = new CMenuOptionChooser("nvodselector.directormode", &g_RemoteControl->director_mode, true, NULL, true, CRCInput::RC_yellow, "gelb.raw" );
			oj->addOption(0, "options.off");
			oj->addOption(1, "options.on");
			NVODSelector.addItem( oj );
		}

		NVODSelector.exec(NULL, "");
	}
}

void CNeutrinoApp::SelectAPID()
{
	if ( g_RemoteControl->current_PIDs.APIDs.size()> 1 )
	{
		// wir haben APIDs für diesen Kanal!

		CMenuWidget APIDSelector("apidselector.head", "audio.raw", 300);
		APIDSelector.addItem( new CMenuSeparator() );

		for( unsigned int count=0; count<g_RemoteControl->current_PIDs.APIDs.size(); count++ )
		{
			char apid[5];
			sprintf(apid, "%d", count);
			APIDSelector.addItem( new CMenuForwarder(g_RemoteControl->current_PIDs.APIDs[count].desc, true,
								  "", APIDChanger, apid, false, (count<9)? (count+1) : CRCInput::RC_nokey ), (count == g_RemoteControl->current_PIDs.PIDs.selected_apid) );
		}
		APIDSelector.exec(NULL, "");
	}
}

void CNeutrinoApp::ShowStreamFeatures()
{
	CMenuWidget StreamFeatureSelector("streamfeatures.head", "features.raw", 350);
	StreamFeatureSelector.addItem( new CMenuSeparator() );

	char id[5];
	int cnt = 0;
	int enabled_count = 0;

	for(unsigned int count=0;count < (unsigned int) g_PluginList->getNumberOfPlugins();count++)
	{
    	if ( g_PluginList->getType(count)== 2 )
    	{
    		// zB vtxt-plugins

			sprintf(id, "%d", count);

			bool enable_it = true; //( ( !g_PluginList->getVTXT(count) )  || (g_RemoteControl->current_PIDs.PIDs.vtxtpid!=0) );
			if ( enable_it )
				enabled_count++;

			StreamFeatureSelector.addItem( new CMenuForwarder(g_PluginList->getName(count), enable_it, "",
				StreamFeaturesChanger, id, false, (cnt== 0) ? CRCInput::RC_blue : CRCInput::RC_nokey, (cnt== 0)?"blau.raw":""), (cnt == 0) && enable_it );
			cnt++;
		}
	}

	if (cnt>0)
	{
		StreamFeatureSelector.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	}

	sprintf(id, "%d", -1);

	// -- Add Channel to favorites
	StreamFeatureSelector.addItem( new CMenuForwarder("favorites.menueadd", true, "",
		new CFavorites, id, true, CRCInput::RC_green, "gruen.raw"), false );
	
	// start/stop streaming
	if(g_settings.network_streaming_use || g_settings.vcr_recording)
	{
		CMenuOptionChooser* oj = new CMenuOptionChooser("mainmenu.streaming", &streamstatus, true, this, true, CRCInput::RC_red, "rot.raw" );
		oj->addOption(0, "mainmenu.streaming_start");
		oj->addOption(1, "mainmenu.streaming_stop");
		StreamFeatureSelector.addItem( oj );
		StreamFeatureSelector.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	}
	// -- Timer Liste
	StreamFeatureSelector.addItem( new CMenuForwarder("timerlist.name", true, "", 
		new CTimerList(), id, true, CRCInput::RC_yellow, "gelb.raw"), false );

	// -- Stream Info
	StreamFeatureSelector.addItem( new CMenuForwarder("streamfeatures.info", true, "",
		StreamFeaturesChanger, id, true, CRCInput::RC_help, "help_small.raw"), false );


	StreamFeatureSelector.exec(NULL, "");
}


void CNeutrinoApp::InitZapper()
{

	g_InfoViewer->start();
	g_EpgData->start();

	firstChannel();
	if (firstchannel.mode == 't')
	{
		tvMode();
	}
	else
	{
		radioMode();
	}
}

void CNeutrinoApp::setupStreamingServer(void)
{
	if(g_settings.vcr_recording)
	{
		CVCRControl::CVCRDeviceInfo * info = new CVCRControl::CVCRDeviceInfo;
		info->Name = g_settings.vcr_devicename;
		vcrControl->registerDevice(CVCRControl::DEVICE_VCR,info);

	}
	else
	{
		CVCRControl::CServerDeviceInfo * info = new CVCRControl::CServerDeviceInfo;
		int port;
		sscanf(g_settings.network_streamingserverport, "%d", &port);
		info->ServerAddress = g_settings.network_streamingserver;
		info->ServerPort = port;
		info->StopPlayBack = (g_settings.network_streaming_stopplayback == 1);
		info->StopSectionsd = (g_settings.network_streaming_stopsectionsd == 1);
		info->Name = "ngrab";
		vcrControl->registerDevice(CVCRControl::DEVICE_SERVER,info);
		delete info;
	}
}

int CNeutrinoApp::run(int argc, char **argv)
{
	CmdParser(argc, argv);

	g_info.box_Type = atoi(getenv("mID"));
	g_info.gtx_ID = -1;
	sscanf(getenv("gtxID"), "%x", &g_info.gtx_ID);
	g_info.enx_ID = -1;
	sscanf(getenv("enxID"), "%x", &g_info.enx_ID);
	g_info.fe = 0;
	sscanf(getenv("fe"), "%x", &g_info.fe);
	//printf("box_Type: %d, gtxID: %d, enxID: %d, fe: %d\n", g_info.box_Type, g_info.gtx_ID, g_info.enx_ID, g_info.fe);




	int loadSettingsErg = loadSetup();

	//timing  (Einheit= 1 sec )
	g_settings.timing_menu = 60;
	g_settings.timing_chanlist = 60;
	g_settings.timing_epg = 2* 60;
	g_settings.timing_infobar = 6;


	g_Fonts = new FontsDef;
	SetupFonts();

	ClearFrameBuffer();

	g_Controld = new CControldClient;
	g_Locale = new CLocaleManager;
	g_RCInput = new CRCInput;
	g_lcdd = new CLcddClient;
	g_Zapit = new CZapitClient;
	g_Sectionsd = new CSectionsdClient;
	g_Timerd = new CTimerdClient;

	g_RemoteControl = new CRemoteControl;
	g_EpgData = new CEpgData;
	g_InfoViewer = new CInfoViewer;
	g_EventList = new EventList;

	g_PluginList = new CPlugins;
	g_PluginList->setPluginDir(PLUGINDIR);

	#ifdef USEACTIONLOG
		g_ActionLog = new CActionLog;
		g_ActionLog->println("neutrino startup");
	#endif

	//printf("\nCNeutrinoApp::run - objects initialized...\n\n");
	g_Locale->loadLocale(g_settings.language);

	colorSetupNotifier	= new CColorSetupNotifier;
	audioSetupNotifier	= new CAudioSetupNotifier;
	videoSetupNotifier	= new CVideoSetupNotifier;
	APIDChanger			= new CAPIDChangeExec;
	UCodeChecker		= new CUCodeCheckExec;
	NVODChanger			= new CNVODChangeExec;
	StreamFeaturesChanger = new CStreamFeaturesChangeExec;
	MyIPChanger			= new CIPChangeNotifier;
	ConsoleDestinationChanger = new CConsoleDestChangeNotifier;

	colorSetupNotifier->changeNotify("initial", NULL);

	setupNetwork();

	// setup streaming server
	if(g_settings.network_streaming_use || g_settings.vcr_recording)
		setupStreamingServer();
	channelList = new CChannelList( "channellist.head" );

	dprintf( DEBUG_NORMAL, "menue setup\n");
	//Main settings
	CMenuWidget mainMenu("mainmenu.head", "mainmenue.raw");
	CMenuWidget mainSettings("mainsettings.head", "settings.raw");
	CMenuWidget languageSettings("languagesetup.head", "language.raw");
	CMenuWidget videoSettings("videomenu.head", "video.raw");
	CMenuWidget audioSettings("audiomenu.head", "audio.raw");
	CMenuWidget parentallockSettings("parentallock.parentallock", "lock.raw", 500);
	CMenuWidget networkSettings("networkmenu.head", "network.raw");
	CMenuWidget streamingSettings("streamingmenu.head", "streaming.raw");
	CMenuWidget colorSettings("colormenu.head", "colors.raw");
	CMenuWidget fontSettings("fontmenu.head", "colors.raw");
	CMenuWidget lcdSettings("lcdmenu.head", "lcd.raw");
	CMenuWidget keySettings("keybindingmenu.head", "keybinding.raw", 400, 460);
	CMenuWidget miscSettings("miscsettings.head", "settings.raw");
	CMenuWidget scanSettings("servicemenu.scants", "settings.raw");
	CMenuWidget service("servicemenu.head", "settings.raw");
	
	CMenuWidget fontSettings_Channellist("fontmenu.channellist", "colors.raw");
	CMenuWidget fontSettings_Eventlist("fontmenu.eventlist", "colors.raw");
	CMenuWidget fontSettings_Infobar("fontmenu.infobar", "colors.raw");
	CMenuWidget fontSettings_Epg("fontmenu.epg", "colors.raw");


	InitMainMenu(mainMenu, mainSettings, audioSettings, parentallockSettings, networkSettings, streamingSettings,
	             colorSettings, lcdSettings, keySettings, videoSettings, languageSettings, miscSettings, 
				 service, fontSettings);

	//service
	InitServiceSettings(service, scanSettings);

	//language Setup
	InitLanguageSettings(languageSettings);

	//misc Setup
	InitMiscSettings(miscSettings);
	miscSettings.setOnPaintNotifier(this);

	//audio Setup
	InitAudioSettings(audioSettings, audioSetupNotifier);

	//video Setup
	InitVideoSettings(videoSettings, videoSetupNotifier);
	videoSettings.setOnPaintNotifier(this);

	// Parentallock settings
	InitParentalLockSettings( parentallockSettings);

	// ScanSettings
	InitScanSettings(scanSettings);

	dprintf( DEBUG_NORMAL, "control event register\n");
	g_Controld->registerEvent(CControldClient::EVT_MUTECHANGED, 222, NEUTRINO_UDS_NAME);
	g_Controld->registerEvent(CControldClient::EVT_VOLUMECHANGED, 222, NEUTRINO_UDS_NAME);
	g_Controld->registerEvent(CControldClient::EVT_MODECHANGED, 222, NEUTRINO_UDS_NAME);
	g_Controld->registerEvent(CControldClient::EVT_VCRCHANGED, 222, NEUTRINO_UDS_NAME);

	dprintf( DEBUG_NORMAL, "sectionsd event register\n");
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_TIMESET, 222, NEUTRINO_UDS_NAME);
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_GOT_CN_EPG, 222, NEUTRINO_UDS_NAME);

	dprintf( DEBUG_NORMAL, "zapit event register\n");
	g_Zapit->registerEvent(CZapitClient::EVT_ZAP_FAILED, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_ZAP_COMPLETE, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_ZAP_COMPLETE_IS_NVOD, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_ZAP_SUB_COMPLETE, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_ZAP_SUB_FAILED, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_SCAN_COMPLETE, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_SCAN_NUM_TRANSPONDERS, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_SCAN_SATELLITE, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_SCAN_NUM_CHANNELS, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_SCAN_PROVIDER, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_RECORDMODE_ACTIVATED, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_RECORDMODE_DEACTIVATED, 222, NEUTRINO_UDS_NAME);
	g_Zapit->registerEvent(CZapitClient::EVT_BOUQUETS_CHANGED, 222, NEUTRINO_UDS_NAME);

	dprintf( DEBUG_NORMAL, "timerd event register\n");
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_SHUTDOWN, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_SHUTDOWN, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_NEXTPROGRAM, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_NEXTPROGRAM, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_STANDBY_ON, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_STANDBY_OFF, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_RECORD, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_RECORD_START, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_RECORD_STOP, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_ZAPTO, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ZAPTO, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_SLEEPTIMER, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_SLEEPTIMER, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_REMIND, 222, NEUTRINO_UDS_NAME);

	//ucodes testen
	doChecks();
	//settins
	if(loadSettingsErg==1)
	{
		dprintf(DEBUG_INFO, "config file missing\n");
		saveSetup();
		ShowHint ( "messagebox.info", g_Locale->getText("settings.noconffile") );
	}
	else if(loadSettingsErg==2)
	{
		dprintf(DEBUG_INFO, "parts of configfile missing\n");
		saveSetup();
		ShowHint ( "messagebox.info", g_Locale->getText("settings.missingoptionsconffile") );
	}

	g_lcdd->setServiceName("Waiting...");
	//init programm
	InitZapper();

	//network Setup
	InitNetworkSettings(networkSettings);

	//streaming Setup
	InitStreamingSettings(streamingSettings);

	//font Setup
	InitFontSettings(fontSettings, fontSettings_Channellist, fontSettings_Eventlist, fontSettings_Infobar, fontSettings_Epg);

	//color Setup
	InitColorSettings(colorSettings, fontSettings);

	//LCD Setup
	InitLcdSettings(lcdSettings);

	//keySettings
	InitKeySettings(keySettings);

	current_volume= g_Controld->getVolume();
	AudioMute( g_Controld->getMute(), true );

	//load Pluginlist
	g_PluginList->loadPlugins();

	RealRun(mainMenu);

	ExitRun();
	return 0;
}


void CNeutrinoApp::RealRun(CMenuWidget &mainMenu)
{
	while( true )
	{
		uint msg; uint data;
		g_RCInput->getMsg( &msg, &data, 100 ); // 10 secs..
/*
		if (msg == NeutrinoMessages::RECORD_START)
		{
			if(CVCRControl::getInstance()->registeredDevices() > 0)
			{
				CVCRControl::CServerDeviceInfo serverinfo;
				serverinfo.StopPlayBack = (g_settings.network_streaming_stopplayback == 1);
				serverinfo.StopSectionsd = (g_settings.network_streaming_stopsectionsd == 1);
				CVCRControl::getInstance()->setDeviceOptions(0,&serverinfo);
				CVCRControl::getInstance()->Record((CTimerd::EventInfo *) data);
				streamstatus = 1;
			}
			else
				printf("Keine vcr Devices registriert\n");
		}

		if ( msg == NeutrinoMessages::RECORD_STOP)
		{
			if(CVCRControl::getInstance()->registeredDevices() > 0)
			{
				if(CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_RECORD || CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_PAUSE)
				{
					CVCRControl::getInstance()->Stop();
					streamstatus=0;
				}
				else
					printf("falscher state\n");
			}
			else
				printf("Keine vcr Devices registriert\n");
		}

		if ( msg == NeutrinoMessages::ZAPTO)
		{
			CTimerd::EventInfo * eventinfo; 
			eventinfo = (CTimerd::EventInfo *) data;
			channelList->zapTo_ChannelID(eventinfo->channel_id);
		}

		if ( msg == NeutrinoMessages::ANNOUNCE_ZAPTO)
		{
			ShowHint ( "messagebox.info", g_Locale->getText("zaptotimer.announce") );
		}
		if ( msg == NeutrinoMessages::ANNOUNCE_RECORD)
		{
			ShowHint ( "messagebox.info", g_Locale->getText("recordtimer.announce") );
		}
		if ( msg == NeutrinoMessages::ANNOUNCE_SLEEPTIMER)
		{
			ShowHint ( "messagebox.info", g_Locale->getText("sleeptimerbox.announce") );
		}
		if ( msg == NeutrinoMessages::SLEEPTIMER)
		{
			if(g_settings.shutdown_real)
				ExitRun();
			else
				standbyMode( true );
		}
		else if ( msg == NeutrinoMessages::ANNOUNCE_SHUTDOWN)
		{
			//TODO: MsgBox mit Ok / Cancel
		}
		else if ( msg == NeutrinoMessages::SHUTDOWN )
		{
			// AUSSCHALTEN...
			ExitRun();
		}
		else if ( msg == NeutrinoMessages::EVT_POPUP )
		{
			ShowHint ( "messagebox.info", string((char *) data) );
		}
		else if ( msg == NeutrinoMessages::EVT_EXTMSG )
		{
			ShowMsg ( "messagebox.info", string((char *) data) , CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw" );
		}
		else if ( msg == NeutrinoMessages::STANDBY_ON )
		{
			if ( mode != mode_standby )
			{
				// noch nicht im Standby-Mode...
				standbyMode( true );
			}
			g_RCInput->clearRCMsg();
		}
		else if ( msg == NeutrinoMessages::STANDBY_OFF )
		{
			if ( mode == mode_standby )
			{
				// WAKEUP
				standbyMode( false );
			}
			g_RCInput->clearRCMsg();
		}
		else if ( msg == NeutrinoMessages::CHANGEMODE )
		{
			if(data == mode_radio)
			{
				if ( mode == mode_tv )
					radioMode();
			}
			if(data == mode_tv)				
			{
				if ( mode == mode_radio )
					tvMode();
			}
			if(data == mode_standby)
			{
				if(mode != mode_standby)
					standbyMode( true );
			}
		}
		else */
		if ( msg == NeutrinoMessages::VCR_ON )
		{
			if  ( mode != mode_scart )
			{
				// noch nicht im Scart-Mode...
				scartMode( true );
			}
		}

		else if ( msg == NeutrinoMessages::VCR_OFF )
		{
			if ( mode == mode_scart )
			{
				// noch nicht im Scart-Mode...
				scartMode( false );
			}
		}
		else
		{
			if ( ( mode == mode_tv ) || ( ( mode == mode_radio ) ) )
			{
				if ( msg == NeutrinoMessages::SHOW_EPG )
				{
					// show EPG

					g_EpgData->show( channelList->getActiveChannel_ChannelID() );

				}
				else if ( msg == (uint) g_settings.key_tvradio_mode )
				{
					if ( mode == mode_tv )
					{
						radioMode();
					}
					else if ( mode == mode_radio )
					{
						tvMode();
					}
				}
				else if ( msg == CRCInput::RC_setup )
				{
					mainMenu.exec(NULL, "");
				}
				if ( msg == CRCInput::RC_ok )
				{
					int bouqMode = g_settings.bouquetlist_mode;//bsmChannels;

					if ((bouquetList!=NULL) && (bouquetList->Bouquets.size() == 0 ))
					{
						dprintf(DEBUG_DEBUG, "bouquets are empty\n");
						bouqMode = bsmAllChannels;
					}
					if ((bouquetList!=NULL) && (bouqMode == 1/*bsmBouquets*/))
					{
						bouquetList->exec(true);
					}
					else if ((bouquetList!=NULL) && (bouqMode == 0/*bsmChannels*/))
					{
						int nNewChannel = bouquetList->Bouquets[bouquetList->getActiveBouquetNumber()]->channelList->show();
						if (nNewChannel>-1)
						{
							channelList->zapTo(bouquetList->Bouquets[bouquetList->getActiveBouquetNumber()]->channelList->getKey(nNewChannel)-1);
						}
					}
					else
					{
						dprintf(DEBUG_DEBUG, "using all channels\n");
						channelList->exec();
					}
				}
				else if ( msg == CRCInput::RC_red )
				{	// eventlist
					g_EventList->exec(channelList->getActiveChannel_ChannelID(), channelList->getActiveChannelName());
				}
				else if ( msg == CRCInput::RC_blue )
				{	// streaminfo
					ShowStreamFeatures();
				}
				else if ( msg == CRCInput::RC_green )
				{	// APID
					SelectAPID();
				}
				else if ( msg == CRCInput::RC_yellow )
				{	// NVODs
					SelectNVOD();
				}
				else if ( ( msg == (uint) g_settings.key_quickzap_up ) || ( msg == (uint) g_settings.key_quickzap_down ) )
				{
					//quickzap
					channelList->quickZap( msg );
				}
				else if ( ( msg == CRCInput::RC_help ) ||
						  ( msg == NeutrinoMessages::SHOW_INFOBAR ) )
				{
					// show Infoviewer
					g_InfoViewer->showTitle( channelList->getActiveChannelNumber(),
					                         channelList->getActiveChannelName(),
				    	                     channelList->getActiveChannel_ChannelID() );
				}
				else if ( ( msg >= CRCInput::RC_0 ) && ( msg <= CRCInput::RC_9 ))
				{ //numeric zap
					if ( g_RemoteControl->director_mode )
					{
						g_RemoteControl->setSubChannel(msg);
						g_InfoViewer->showSubchan();
					}
					else
						channelList->numericZap( msg );
				}
				else if ( msg == (uint) g_settings.key_subchannel_up )
				{
					g_RemoteControl->subChannelUp();
					g_InfoViewer->showSubchan();
				}
				else if ( msg == (uint) g_settings.key_subchannel_down )
				{
					g_RemoteControl->subChannelDown();
					g_InfoViewer->showSubchan();
				}
				else
				{
					handleMsg( msg, data );
				}

			}
			else
			{
				// mode == mode_scart
				if ( msg == CRCInput::RC_home )
				{
					if ( mode == mode_scart )
					{
						// noch nicht im Scart-Mode...
						scartMode( false );
					}
				}
				else
				{
					handleMsg( msg, data );
				}
			}
		}
	}
}

void CNeutrinoApp::showProfiling( string text )
{
	struct timeval tv;

	gettimeofday( &tv, NULL );
	long long now = (long long) tv.tv_usec + (long long)((long long) tv.tv_sec * (long long) 1000000);


	//printf("--> '%s' %f\n", text.c_str(), (now- last_profile_call)/ 1000.);
	last_profile_call = now;
}

int CNeutrinoApp::handleMsg(uint msg, uint data)
{
	int res = 0;

	res = res | g_RemoteControl->handleMsg(msg, data);
	res = res | g_InfoViewer->handleMsg(msg, data);
	res = res | channelList->handleMsg(msg, data);

	if ( res != messages_return::unhandled )
	{
		if ( ( msg>= CRCInput::RC_WithData ) && ( msg< CRCInput::RC_WithData+ 0x10000000 ) )
			delete (unsigned char*) data;
		return ( res & ( 0xFFFFFFFF - messages_return::unhandled ) );
	}

    if ( msg == NeutrinoMessages::EVT_VCRCHANGED )
	{
		if ( g_settings.vcr_AutoSwitch == 1 )
		{
			if ( data != VCR_STATUS_OFF )
				g_RCInput->postMsg( NeutrinoMessages::VCR_ON, 0 );
			else
				g_RCInput->postMsg( NeutrinoMessages::VCR_OFF, 0 );
		}
		return messages_return::handled | messages_return::cancel_info;
	}
	else
	if ( msg == CRCInput::RC_standby )
	{
		// trigger StandBy
		struct timeval tv;
		gettimeofday( &tv, NULL );
		standby_pressed_at = (tv.tv_sec*1000000) + tv.tv_usec;

		if ( mode == mode_standby )
		{
        	g_RCInput->postMsg( NeutrinoMessages::STANDBY_OFF, 0 );
		}
		else if ( !g_settings.shutdown_real )
		{
			int timeout = 5;
			int timeout1 = 5;

			sscanf(g_settings.repeat_blocker, "%d", &timeout);
			timeout = int(timeout/100.0) + 5;
			sscanf(g_settings.repeat_genericblocker, "%d", &timeout1);
			timeout1 = int(timeout1/100.0) + 5;
			if(timeout1>timeout)
				timeout=timeout1;

			uint msg; uint data;
			int diff = 0;
			long long endtime;

			do
			{
				g_RCInput->getMsg( &msg, &data, timeout );

				if ( msg != CRCInput::RC_timeout )
				{
					gettimeofday( &tv, NULL );
					endtime = (tv.tv_sec*1000000) + tv.tv_usec;
					diff = int((endtime - standby_pressed_at)/100000. );
				}

			} while ( ( msg != CRCInput::RC_timeout ) && ( diff < 10 ) );

			g_RCInput->postMsg( ( diff >= 10 ) ? NeutrinoMessages::SHUTDOWN : NeutrinoMessages::STANDBY_ON, 0 );
        }
        else
        {
        	g_RCInput->postMsg( NeutrinoMessages::SHUTDOWN, 0 );
		}
		return messages_return::cancel_all | messages_return::handled;
	}
	else if ( msg == CRCInput::RC_standby_release )
	{
		struct timeval tv;
		gettimeofday( &tv, NULL );
		long long endtime = (tv.tv_sec*1000000) + tv.tv_usec;
		int diff = int((endtime - standby_pressed_at)/100000. );
		if ( diff >= 10 )
		{
        	g_RCInput->postMsg( NeutrinoMessages::SHUTDOWN, 0 );
        	return messages_return::cancel_all | messages_return::handled;
        }
	}
	else if ( ( msg == CRCInput::RC_plus ) ||
			  ( msg == CRCInput::RC_minus ) )
	{
		//volume
		setVolume( msg, ( mode != mode_scart ) );
		return messages_return::handled;
	}
	else if ( msg == CRCInput::RC_spkr )
	{
		//mute
		AudioMute( !current_muted );
		return messages_return::handled;
	}
	else if ( msg == NeutrinoMessages::EVT_VOLCHANGED )
	{
		current_volume = data;
		return messages_return::handled;
	}
	else if ( msg == NeutrinoMessages::EVT_MUTECHANGED )
	{
		AudioMute( (bool)data, true );
		return messages_return::handled;
	}
	else if ( msg == NeutrinoMessages::EVT_RECORDMODE )
	{
		dprintf(DEBUG_DEBUG, "neutino - recordmode %s\n", ( data ) ? "on":"off" );

		if ( ( !g_InfoViewer->is_visible ) && data )
			g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );

		channelsInit();
	}
//	else if ( ( msg == NeutrinoMessages::EVT_BOUQUETSCHANGED ) ||   // EVT_BOUQUETSCHANGED: initiated by zapit
//		  ( msg == NeutrinoMessages::EVT_SERVICESCHANGED ) )    // EVT_SERVICESCHANGED: no longer used
	else if ( msg == NeutrinoMessages::EVT_BOUQUETSCHANGED )        // EVT_BOUQUETSCHANGED: initiated by zapit
	{
		t_channel_id old_id = channelList->getActiveChannel_ChannelID();

		channelsInit();

		if ((old_id == 0) || (!(channelList->adjustToChannelID(old_id))))
			channelList->zapTo(0);

		return messages_return::handled;
	}

		if (msg == NeutrinoMessages::RECORD_START)
		{
			if(CVCRControl::getInstance()->registeredDevices() > 0)
			{
				CVCRControl::CServerDeviceInfo serverinfo;
				serverinfo.StopPlayBack = (g_settings.network_streaming_stopplayback == 1);
				serverinfo.StopSectionsd = (g_settings.network_streaming_stopsectionsd == 1);
				CVCRControl::getInstance()->setDeviceOptions(0,&serverinfo);

				if (CVCRControl::getInstance()->Record((CTimerd::EventInfo *) data))
               streamstatus = 1;
            else
               streamstatus = 0;
			}
			else
				printf("Keine vcr Devices registriert\n");
			return messages_return::handled | messages_return::cancel_all;
		}

		if ( msg == NeutrinoMessages::RECORD_STOP)
		{
			if(CVCRControl::getInstance()->registeredDevices() > 0)
			{
				if(CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_RECORD || CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_PAUSE)
				{
					CVCRControl::getInstance()->Stop();
					streamstatus=0;
				}
				else
					printf("falscher state\n");
			}
			else
				printf("Keine vcr Devices registriert\n");
			return messages_return::handled;
		}

		if ( msg == NeutrinoMessages::ZAPTO)
		{
			CTimerd::EventInfo * eventinfo; 
			eventinfo = (CTimerd::EventInfo *) data;
			channelList->zapTo_ChannelID(eventinfo->channel_id);
			return messages_return::handled;
		}

		if ( msg == NeutrinoMessages::ANNOUNCE_ZAPTO)
		{
			if ( mode == mode_standby )
			{
				// WAKEUP
				standbyMode( false );
			}
			if  ( mode != mode_scart )
				ShowHint ( "messagebox.info", g_Locale->getText("zaptotimer.announce") );
			return messages_return::handled;
		}
		if ( msg == NeutrinoMessages::ANNOUNCE_RECORD)
		{
			if  ( mode != mode_scart )
				ShowHint ( "messagebox.info", g_Locale->getText("recordtimer.announce") );
			CTimerd::EventInfo * eventinfo; 
			eventinfo = (CTimerd::EventInfo *) data;
//			channelList->zapTo_ChannelID(eventinfo->channel_id); // dann umschalten
//				g_Zapit->zapTo_serviceID(eventinfo->channel_id);		
			return messages_return::handled;
		}
		if ( msg == NeutrinoMessages::ANNOUNCE_SLEEPTIMER)
		{
			if  ( mode != mode_scart )
				ShowHint ( "messagebox.info", g_Locale->getText("sleeptimerbox.announce") );
			return messages_return::handled;
		}
		if ( msg == NeutrinoMessages::SLEEPTIMER)
		{
			if(g_settings.shutdown_real)
				ExitRun();
			else
				standbyMode( true );
			return messages_return::handled;
		}
		else if ( msg == NeutrinoMessages::STANDBY_ON )
		{
			if ( mode != mode_standby )
			{
				// noch nicht im Standby-Mode...
				standbyMode( true );
			}
			g_RCInput->clearRCMsg();
			return messages_return::handled;
		}
		else if ( msg == NeutrinoMessages::STANDBY_OFF )
		{
			if ( mode == mode_standby )
			{
				// WAKEUP
				standbyMode( false );
			}
			g_RCInput->clearRCMsg();
			return messages_return::handled;
		}
		else if ( msg == NeutrinoMessages::ANNOUNCE_SHUTDOWN)
		{
			if  ( mode != mode_scart )
				skipShutdownTimer = (ShowMsg ( "messagebox.info", 
														 g_Locale->getText("shutdowntimer.announce") , 
														 CMessageBox::mbrNo, 
														 CMessageBox::mbYes | CMessageBox::mbNo, "",450,5)
											==CMessageBox::mbrYes);
		}
		else if ( msg == NeutrinoMessages::SHUTDOWN )
		{
			// AUSSCHALTEN...
			if(!skipShutdownTimer)
			{
				ExitRun();
			}
			else
			{
				skipShutdownTimer=false;
			}
			return messages_return::handled;
		}
		else if ( msg == NeutrinoMessages::EVT_POPUP )
		{
			if  ( mode != mode_scart )
				ShowHint ( "messagebox.info", string((char *) data) );
			return messages_return::handled;
		}
		else if ( msg == NeutrinoMessages::EVT_EXTMSG )
		{
			if  ( mode != mode_scart )
				ShowMsg ( "messagebox.info", string((char *) data) , CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw" );
			return messages_return::handled;
		}
		else if ( msg == NeutrinoMessages::REMIND)
		{
			string text = (char*)data;
			string::size_type pos;
			while((pos=text.find("/",0))!=string::npos)
			{
				text.replace(pos,1,"\n");
			}
			if  ( mode != mode_scart )
				ShowMsg ( "timerlist.type.remind", text , CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw",0 );
			return messages_return::handled;
		}
		else if ( msg == NeutrinoMessages::CHANGEMODE )
		{
			if(data == mode_radio)
			{
				if ( mode == mode_tv )
					radioMode();
			}
			if(data == mode_tv)				
			{
				if ( mode == mode_radio )
					tvMode();
			}
			if(data == mode_standby)
			{
				if(mode != mode_standby)
					standbyMode( true );
			}
		}

	if ( ( msg>= CRCInput::RC_WithData ) && ( msg< CRCInput::RC_WithData+ 0x10000000 ) )
		delete (unsigned char*) data;

	return messages_return::unhandled;
}



void CNeutrinoApp::ExitRun()
{
	#ifdef USEACTIONLOG
		g_ActionLog->println("neutrino shutdown");
	#endif

	dprintf(DEBUG_INFO, "exit\n");
/*  moved to controld
	//shutdown screen
	g_lcdd->shutdown();
	// timerd beenden und wakeup programmieren
	g_Timerd->shutdown();
*/
	for(int x=0;x<256;x++)
		frameBuffer->paletteSetColor(x, 0x000000, 0xffff);
	frameBuffer->paletteSet();

	if (frameBuffer->getActive())
		frameBuffer->loadPicture2Mem("shutdown.raw", frameBuffer->getFrameBufferPointer() );
	frameBuffer->loadPal("shutdown.pal");

	saveSetup();
	g_Controld->shutdown();
	sleep(55555);
}

bool CNeutrinoApp::onPaintNotify(string MenuName)
{
	if(MenuName == "videomenu.head")
	{//aktuelle werte vom controld holen...
		g_settings.video_Signal = g_Controld->getVideoOutput();
		g_settings.video_Format = g_Controld->getVideoFormat();
	}

	return false;
}

void CNeutrinoApp::AudioMute( bool newValue, bool isEvent )
{
	int dx = 40;
	int dy = 40;
	int x = g_settings.screen_EndX-dx;
	int y = g_settings.screen_StartY;

	if ( newValue != current_muted )
	{
		current_muted = newValue;

		if ( !isEvent )
		{
			if ( current_muted )
				g_Controld->Mute((g_settings.audio_SPDIF_Control==1));
			else
				g_Controld->UnMute((g_settings.audio_SPDIF_Control==1));
		}
	}

	if ( isEvent && ( mode != mode_scart ) )
	{
		// anzeigen NUR, wenn es vom Event kommt
		if ( current_muted )
		{
			frameBuffer->paintBoxRel(x, y, dx, dy, COL_INFOBAR);
			frameBuffer->paintIcon("mute.raw", x+5, y+5);
		}
		else
			frameBuffer->paintBackgroundBoxRel(x, y, dx, dy);
	}
}

void CNeutrinoApp::setVolume(int key, bool bDoPaint)
{
	int dx = 256;
	int dy = 40;
	int x = (((g_settings.screen_EndX- g_settings.screen_StartX)- dx) / 2) + g_settings.screen_StartX;
	int y = g_settings.screen_EndY- 100;

	unsigned char* pixbuf = NULL;

	if (bDoPaint)
	{
		pixbuf= new unsigned char[ dx * dy ];
		if (pixbuf!= NULL)
			frameBuffer->SaveScreen(x, y, dx, dy, pixbuf);
		frameBuffer->paintIcon("volume.raw",x,y, COL_INFOBAR);
	}

	uint msg = key;
	uint data;

	unsigned long long timeoutEnd;

	do
	{
		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_infobar/ 2 );

		if (msg==CRCInput::RC_plus)
		{
			if (current_volume<100)
			{
				current_volume += 5;
			}
			g_Controld->setVolume(current_volume,(g_settings.audio_SPDIF_Control==1));
		}
		else if (msg==CRCInput::RC_minus)
		{
			if (current_volume>0)
			{
				current_volume -= 5;
			}
			g_Controld->setVolume(current_volume,(g_settings.audio_SPDIF_Control==1));
		}
		else
		{
			if ( (msg!=CRCInput::RC_ok) || (msg!=CRCInput::RC_home) )
			{
				if ( handleMsg( msg, data ) & messages_return::unhandled )
				{
					g_RCInput->postMsg( msg, data );

					msg= CRCInput::RC_timeout;
				}
			}
		}

		if (bDoPaint)
		{
			int vol = current_volume<<1;
			frameBuffer->paintBoxRel(x+40, y+12, 200, 15, COL_INFOBAR+1);
			frameBuffer->paintBoxRel(x+40, y+12, vol, 15, COL_INFOBAR+3);
        }

		if ( msg != CRCInput::RC_timeout )
		{
			g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );
		}

	}
	while ( msg != CRCInput::RC_timeout );

    if ( (bDoPaint) && (pixbuf!= NULL) )
		frameBuffer->RestoreScreen(x, y, dx, dy, pixbuf);
}

void CNeutrinoApp::tvMode( bool rezap )
{
	if( mode == mode_tv )
	{
		return;
	}
	else if( mode == mode_scart )
	{
		g_Controld->setScartMode( 0 );
	}
	else if( mode == mode_standby )
	{
		g_lcdd->setMode(CLcddTypes::MODE_TVRADIO);
		g_Controld->videoPowerDown(false);
	}

	mode = mode_tv;
	#ifdef USEACTIONLOG
		g_ActionLog->println("mode: tv");
	#endif

	//printf( "tv-mode\n" );

	if (frameBuffer->getActive())
		memset(frameBuffer->getFrameBufferPointer(), 255, frameBuffer->getStride()*576);
	frameBuffer->useBackground(false);

    if ( rezap )
	{
		g_RemoteControl->tvMode();
		firstChannel();
		channelsInit();
		channelList->zapTo( firstchannel.channelNumber -1 );
	}
}

void CNeutrinoApp::scartMode( bool bOnOff )
{
	#ifdef USEACTIONLOG
		g_ActionLog->println( ( bOnOff ) ? "mode: scart on" : "mode: scart off" );
	#endif

	//printf( ( bOnOff ) ? "mode: scart on\n" : "mode: scart off\n" );

	if ( bOnOff )
	{
		// SCART AN
		if (frameBuffer->getActive())
			memset(frameBuffer->getFrameBufferPointer(), 255, frameBuffer->getStride()*576);
		g_Controld->setScartMode( 1 );

		lastMode = mode;
		mode = mode_scart;
	}
	else
	{
	    // SCART AUS
		g_Controld->setScartMode( 0 );

        mode = mode_unknown;

		//re-set mode
		if ( lastMode == mode_radio )
		{
			radioMode( false );
		}
		else if ( lastMode == mode_tv )
		{
			tvMode( false );
		}
		else if ( lastMode == mode_standby )
		{
			standbyMode( true );
		}
	}
}

void CNeutrinoApp::standbyMode( bool bOnOff )
{
	#ifdef USEACTIONLOG
		g_ActionLog->println( ( bOnOff ) ? "mode: standby on" : "mode: standby off" );
	#endif

	//printf( ( bOnOff ) ? "mode: standby on\n" : "mode: standby off\n" );

	if ( bOnOff )
	{
		// STANDBY AN

		if( mode == mode_scart )
		{
			g_Controld->setScartMode( 0 );
		}

		if (frameBuffer->getActive())
			memset(frameBuffer->getFrameBufferPointer(), 255, frameBuffer->getStride()*576);

		g_lcdd->setMode(CLcddTypes::MODE_STANDBY);
		g_Controld->videoPowerDown(true);

		lastMode = mode;
		mode = mode_standby;
	}
	else
	{
	    // STANDBY AUS

		g_lcdd->setMode(CLcddTypes::MODE_TVRADIO);
		g_Controld->videoPowerDown(false);

		mode = mode_unknown;

		//re-set mode
		if ( lastMode == mode_radio )
		{
			radioMode( false );
		}
		else
		{
			tvMode( false );
		}
	}
}

void CNeutrinoApp::radioMode( bool rezap)
{
	if ( mode==mode_radio )
	{
		return;
	}
	else if( mode == mode_scart )
	{
		g_Controld->setScartMode( 0 );
	}
	else if( mode == mode_standby )
	{
		g_lcdd->setMode(CLcddTypes::MODE_TVRADIO);
		g_Controld->videoPowerDown(false);
	}

	mode = mode_radio;
	#ifdef USEACTIONLOG
		g_ActionLog->println("mode: radio");
	#endif

	frameBuffer->loadPal("radiomode.pal", 18, COL_MAXFREE);
	frameBuffer->loadBackground("radiomode.raw");
	frameBuffer->useBackground(true);
	frameBuffer->paintBackground();

	if ( rezap )
	{
		g_RemoteControl->radioMode();
		firstChannel();
		channelsInit();
		channelList->zapTo( firstchannel.channelNumber -1 );
	}
}


/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  exec, menuitem callback (shutdown)                         *
*                                                                                     *
**************************************************************************************/
int CNeutrinoApp::exec( CMenuTarget* parent, string actionKey )
{
	//	printf("ac: %s\n", actionKey.c_str());
	int returnval = menu_return::RETURN_REPAINT;

	if(actionKey=="theme_neutrino")
	{
		setupColors_neutrino();
		colorSetupNotifier->changeNotify("initial", NULL);
	}
	else if(actionKey=="theme_classic")
	{
		setupColors_classic();
		colorSetupNotifier->changeNotify("initial", NULL);
	}
	else if(actionKey=="shutdown")
	{
		ExitRun();
	}
	else if(actionKey=="tv")
	{
		tvMode();
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="radio")
	{
		radioMode();
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="scart")
	{
		g_RCInput->postMsg( NeutrinoMessages::VCR_ON, 0 );
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="network")
	{
		setupNetwork( true );
	}
	else if(actionKey=="networktest")
	{
		 testNetwork( );
	}
	else if(actionKey=="networkshow")
	{
		showNetwork( );
	}

	else if(actionKey=="savesettings")
	{
		g_Controld->saveSettings();
		saveSetup();
	}

	return returnval;
}

/**************************************************************************************
*                                                                                     *
*          changeNotify - features menu streaming server start / stop                 *
*                                                                                     *
**************************************************************************************/
bool CNeutrinoApp::changeNotify(string OptionName, void *Data)
{
	printf("OptionName: %s\n",OptionName.c_str());
	if (OptionName.substr(0,9).compare("fontsize.") == 0)
	{
		SetupFonts();
	}
	else
	{
		CTimerd::EventInfo eventinfo;

		if(CVCRControl::getInstance()->registeredDevices() > 0)
		{
			if(streamstatus == 1)
			{
				eventinfo.channel_id = g_RemoteControl->current_channel_id;
				eventinfo.epgID = g_RemoteControl->current_EPGid;
            eventinfo.apid = 0;

				CVCRControl::CServerDeviceInfo serverinfo;
				serverinfo.StopPlayBack = (g_settings.network_streaming_stopplayback == 1);
				serverinfo.StopSectionsd = (g_settings.network_streaming_stopsectionsd == 1);
				CVCRControl::getInstance()->setDeviceOptions(0,&serverinfo);

				if (CVCRControl::getInstance()->Record(&eventinfo)==false)
            {
               streamstatus=0;
               return false;
            }
			}
			else
			{
				CVCRControl::getInstance()->Stop();
			}
			return true;
		}
		else
			printf("Keine Streamingdevices registriert\n");
	}
	return false;
}



/**************************************************************************************
*                                                                                     *
*          Main programm - no function here                                           *
*                                                                                     *
**************************************************************************************/
int main(int argc, char **argv)
{
	setDebugLevel(DEBUG_NORMAL);
	dprintf( DEBUG_NORMAL, "NeutrinoNG $Id: neutrino.cpp,v 1.344 2002/10/16 16:25:24 dirch Exp $\n\n");

	//dhcp-client beenden, da sonst neutrino beim hochfahren stehenbleibt
	system("killall -9 udhcpc >/dev/null 2>/dev/null");

	tzset();
	initGlobals();
	return CNeutrinoApp::getInstance()->run(argc, argv);
}
