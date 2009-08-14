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

#include <driver/rcinput.h>

#include "color.h"
#include "scan.h"

#include "widget/menue.h"
#include "widget/messagebox.h"


CScanTs::CScanTs()
{
	frameBuffer = CFrameBuffer::getInstance();
	width = 400;
	hheight = g_Fonts->menu_title->getHeight();
	mheight = g_Fonts->menu->getHeight();
	height = hheight+5*mheight;
	x=((720-width) >> 1) -20;
	y=(576-height)>>1;
}

int CScanTs::exec(CMenuTarget* parent, string)
{
	if (!frameBuffer->getActive())
		return menu_return::RETURN_EXIT_ALL;

	frameBuffer->loadPal("scan.pal", 37, COL_MAXFREE);
	frameBuffer->loadPicture2Mem("scan.raw", frameBuffer->getFrameBufferPointer());

	g_Sectionsd->setPauseScanning( true );

	g_Zapit->setDiseqcType( CNeutrinoApp::getInstance()->getScanSettings().diseqcMode);
	g_Zapit->setDiseqcRepeat( CNeutrinoApp::getInstance()->getScanSettings().diseqcRepeat);
	g_Zapit->setScanBouquetMode( CNeutrinoApp::getInstance()->getScanSettings().bouquetMode);

	CZapitClient::ScanSatelliteList satList;
	CNeutrinoApp::getInstance()->getScanSettings().toSatList( satList);
	g_Zapit->setScanSatelliteList( satList);

	bool finish = !(g_Zapit->startScan());

	paint();

	int ypos=y;
	frameBuffer->paintBoxRel(x, ypos+ hheight, width, height- hheight, COL_MENUCONTENT);
	ypos= y+ hheight + (mheight >>1);
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, g_Locale->getText("scants.transponders").c_str(), COL_MENUCONTENT);
	ypos+= mheight;
	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, g_Locale->getText("scants.services").c_str(), COL_MENUCONTENT);
	ypos+= mheight;
	if (g_info.fe==1)
	{	//sat only
		g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width, g_Locale->getText("scants.actsatellite").c_str(), COL_MENUCONTENT);
	}

	int xpos1 = x+20 + g_Fonts->menu->getRenderWidth(g_Locale->getText("scants.transponders").c_str());
	int xpos2 = x+20 + g_Fonts->menu->getRenderWidth(g_Locale->getText("scants.services").c_str());
	int xpos3 = x+20 + g_Fonts->menu->getRenderWidth(g_Locale->getText("scants.actsatellite").c_str());

	frameBuffer->loadPal("radar.pal", 17, 37);
	int pos = 0;

	ypos= y+ hheight + (mheight >>1);

	uint msg; uint data;

	while (!finish)
	{
		char filename[30];
		sprintf(filename, "radar%d.raw", pos);
		pos = (pos+1)%10;
		frameBuffer->paintIcon8(filename, x+300,ypos+15, 17);

		unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd_MS( 250 );
		msg = CRCInput::RC_nokey;

		while ( ! ( msg == CRCInput::RC_timeout ) )
		{
			g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

			if ( msg == NeutrinoMessages::EVT_SCAN_SATELLITE )
			{
				frameBuffer->paintBox(xpos3, ypos+ 2* mheight, x+width-105, ypos+ 2* mheight+mheight, COL_MENUCONTENT);
				g_Fonts->menu->RenderString(xpos3, ypos+ 3*mheight, width, (char*)data, COL_MENUCONTENT);
				delete (unsigned char*) data;
			}
            else
			if ( msg == NeutrinoMessages::EVT_SCAN_NUM_CHANNELS )
			{
				char cb[10];
				sprintf(cb, "%d", data);
				frameBuffer->paintBox(xpos2, ypos+ mheight, x+width-105, ypos+ mheight+mheight, COL_MENUCONTENT);
				g_Fonts->menu->RenderString(xpos2, ypos+ 2* mheight, width, cb, COL_MENUCONTENT);
			}
			else
			if ( msg == NeutrinoMessages::EVT_SCAN_NUM_TRANSPONDERS )
			{
				char cb[10];
				sprintf(cb, "%d", data);
				frameBuffer->paintBox(xpos1, ypos, x+width-105, ypos+mheight, COL_MENUCONTENT);
				g_Fonts->menu->RenderString(xpos1, ypos+ mheight, width, cb, COL_MENUCONTENT);
			}
			else
			if ( msg == NeutrinoMessages::EVT_SCAN_PROVIDER )
			{
				frameBuffer->paintBoxRel(x+ 10, ypos+ 3* mheight+2, width-20, mheight, COL_MENUCONTENT);
				g_Fonts->menu->RenderString(x+ 10, ypos+ 4* mheight, width-20, (char*)data, COL_MENUCONTENTINACTIVE, 0, true); // UTF-8
				delete (unsigned char*) data;
			}
			else
			if ( msg == NeutrinoMessages::EVT_SCAN_COMPLETE )
			{
				finish= true;
				msg = CRCInput::RC_timeout;
			}
			else
			if ( ( msg>= CRCInput::RC_WithData ) && ( msg< CRCInput::RC_WithData+ 0x10000000 ) )
				delete (unsigned char*) data;
		}
	}

	hide();
	g_Sectionsd->setPauseScanning( false );
	ShowMsg ( "messagebox.info", g_Locale->getText("scants.finished"), CMessageBox::mbBack, CMessageBox::mbBack, "info.raw" );

	return menu_return::RETURN_REPAINT;
}

void CScanTs::hide()
{
	frameBuffer->loadPal("radiomode.pal", 18, COL_MAXFREE);
	frameBuffer->paintBackgroundBoxRel(0,0, 720,576);
}


void CScanTs::paint()
{
	int ypos=y;
	frameBuffer->paintBoxRel(x, ypos, width, hheight, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10, ypos+ hheight, width, g_Locale->getText("scants.head").c_str(), COL_MENUHEAD);
	frameBuffer->paintBoxRel(x, ypos+ hheight, width, height- hheight, COL_MENUCONTENT);
}
