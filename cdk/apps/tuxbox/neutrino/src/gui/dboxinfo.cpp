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


#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include "dboxinfo.h"


CDBoxInfoWidget::CDBoxInfoWidget()
{
	frameBuffer =	CFrameBuffer::getInstance();
	width = 	600;
	hheight = 	g_Fonts->menu_title->getHeight();
	mheight = 	g_Fonts->menu->getHeight();
	height = 	hheight+13*mheight+ 10;

    x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-height) / 2) + g_settings.screen_StartY;
}


int CDBoxInfoWidget::exec(CMenuTarget* parent, string)
{
	if (parent)
	{
		parent->hide();
	}
	paint();

	int res = g_RCInput->messageLoop();

	hide();
	return res;
}

void CDBoxInfoWidget::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}

void CDBoxInfoWidget::paint()
{
	int ypos=y;
	frameBuffer->paintBoxRel(x, ypos, width, hheight, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10, ypos+ hheight+1, width, g_Locale->getText("dboxinfo.head").c_str(), COL_MENUHEAD);
	frameBuffer->paintBoxRel(x, ypos+ hheight, width, height- hheight, COL_MENUCONTENT);

	ypos+= hheight + (mheight >>1);


	FILE* fd = fopen("/proc/cpuinfo", "rt");
	if (fd==NULL)
	{
		printf("error while opening proc-cpuinfo\n" );
		return;
	}

	char buf[256];

	while(!feof(fd))
	{
		if(fgets(buf,255,fd)!=NULL)
		{
			g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);

			ypos+= mheight;
			
		}
	}
/*
	int bitInfo[10];

	char *key,*tmpptr,buf[100], buf2[100];
	int value, pos=0;
	fgets(buf,29,fd);//dummy
	while(!feof(fd))
	{
		if(fgets(buf,29,fd)!=NULL)
		{
			buf[strlen(buf)-1]=0;
			tmpptr=buf;
			key=strsep(&tmpptr,":");
			for(;tmpptr[0]==' ';tmpptr++)
				;
			value=atoi(tmpptr);
			//printf("%s: %d\n",key,value);
			bitInfo[pos]= value;
			pos++;
		}
	}
	fclose(fd);


	//paint msg...
	sprintf((char*) buf, "%s: %dx%d", g_Locale->getText("streaminfo.resolution").c_str(), bitInfo[0], bitInfo[1] );
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);

	ypos+= mheight;

	sprintf((char*) buf, "%s: %d bit/sec", g_Locale->getText("streaminfo.bitrate").c_str(), bitInfo[4]*50);
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);

	ypos+= mheight;


	switch ( bitInfo[2] )
	{
			case 2:
			sprintf((char*) buf, "%s: 4:3", g_Locale->getText("streaminfo.aratio").c_str() );
			break;
			case 3:
			sprintf((char*) buf, "%s: 16:9", g_Locale->getText("streaminfo.aratio").c_str());
			break;
			case 4:
			sprintf((char*) buf, "%s: 2.21:1", g_Locale->getText("streaminfo.aratio").c_str());
			break;
			default:
			sprintf((char*) buf, "%s", g_Locale->getText("streaminfo.aratio_unknown").c_str());
	}
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);

	ypos+= mheight;


	switch ( bitInfo[3] )
	{
			case 3:
			sprintf((char*) buf, "%s: 25fps", g_Locale->getText("streaminfo.framerate").c_str());
			break;
			case 6:
			sprintf((char*) buf, "%s: 50fps", g_Locale->getText("streaminfo.framerate").c_str());
			break;
			default:
			sprintf((char*) buf, "%s", g_Locale->getText("streaminfo.framerate_unknown").c_str());
	}
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);

	ypos+= mheight;


	switch ( bitInfo[6] )
	{
			case 1:
			sprintf((char*) buf, "%s: single channel", g_Locale->getText("streaminfo.audiotype").c_str());
			break;
			case 2:
			sprintf((char*) buf, "%s: dual channel", g_Locale->getText("streaminfo.audiotype").c_str());
			break;
			case 3:
			sprintf((char*) buf, "%s: joint stereo", g_Locale->getText("streaminfo.audiotype").c_str());
			break;
			case 4:
			sprintf((char*) buf, "%s: stereo", g_Locale->getText("streaminfo.audiotype").c_str());
			break;
			default:
			sprintf((char*) buf, "%s", g_Locale->getText("streaminfo.audiotype_unknown").c_str());
	}
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);
	ypos+= mheight+ 10;

	CZapitClient::CCurrentServiceInfo si = g_Zapit->getCurrentServiceInfo();

	//onid
	sprintf((char*) buf, "%s: 0x%04x", "onid", si.onid);
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);
	ypos+= mheight;

	//sid
	sprintf((char*) buf, "%s: 0x%04x", "sid", si.sid);
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);
	ypos+= mheight;

	//tsid
	sprintf((char*) buf, "%s: 0x%04x", "tsid", si.tsid);
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);
	ypos+= mheight;

	//tsfrequenz
	sprintf((char*) buf, "%s: %dkhz %s", "tsf", si.tsfrequency, si.polarisation==1?"(v)":"(h)");
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);
	ypos+= mheight;

	//vpid
	if ( g_RemoteControl->current_PIDs.PIDs.vpid == 0 )
		sprintf((char*) buf, "%s: %s", "vpid", g_Locale->getText("streaminfo.not_available").c_str() );
	else
		sprintf((char*) buf, "%s: 0x%04x", "vpid", g_RemoteControl->current_PIDs.PIDs.vpid );
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);
	ypos+= mheight;

	//apid	
	if ( g_RemoteControl->current_PIDs.APIDs.size() == 0 )
		sprintf((char*) buf, "%s: %s", "apid(s)", g_Locale->getText("streaminfo.not_available").c_str() );
	else
	{
		sprintf((char*) buf, "%s: ", "apid(s)" );
		for (unsigned int i= 0; i< g_RemoteControl->current_PIDs.APIDs.size(); i++)
		{
			sprintf((char*) buf2, " 0x%04x",  g_RemoteControl->current_PIDs.APIDs[i].pid );

			if (i > 0)
			strcat((char*) buf, ",");

			strcat((char*) buf, buf2);
		}
	}
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);
	ypos+= mheight;

	//vtxtpid
	if ( g_RemoteControl->current_PIDs.PIDs.vtxtpid == 0 )
        	sprintf((char*) buf, "%s: %s", "vtxtpid", g_Locale->getText("streaminfo.not_available").c_str() );
	else
        	sprintf((char*) buf, "%s: 0x%04x", "vtxtpid", g_RemoteControl->current_PIDs.PIDs.vtxtpid );
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, buf, COL_MENUCONTENT);
	ypos+= mheight;
*/
}
