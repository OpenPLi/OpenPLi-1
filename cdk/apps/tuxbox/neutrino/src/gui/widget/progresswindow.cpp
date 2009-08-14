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


#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <gui/color.h>

#include "progresswindow.h"


CProgressWindow::CProgressWindow()
{
	frameBuffer = CFrameBuffer::getInstance();
	width = 420;
	hheight = g_Fonts->menu_title->getHeight();
	mheight = g_Fonts->menu->getHeight();
	height = hheight+5*mheight+20;

	globalstatus = -1;
	statusText = "";

	x= ( ( ( g_settings.screen_EndX- g_settings.screen_StartX ) - width ) >> 1 ) + g_settings.screen_StartX;
	y=(576-height)>>1;
}

void CProgressWindow::setTitle( string title )
{
	caption = title;
}


void CProgressWindow::showGlobalStatus(int prog)
{
	if(prog>100)
	{
		prog = 100;
	}
	if(prog<0)
	{
		prog=0;
	}
	if(globalstatus==prog)
	{
		return;
	}
	globalstatus = prog;

	int pos = x+10;
	if(prog!=0)
	{
		pos += int( float(width-20)/100.0*prog);
		//vordergrund
		frameBuffer->paintBox(x+10, globalstatusY,pos, globalstatusY+10, COL_MENUCONTENT +7);
	}
	//hintergrund
	frameBuffer->paintBox(pos, globalstatusY, x+width-10, globalstatusY+10, COL_MENUCONTENT +2);
}

void CProgressWindow::showLocalStatus(int prog)
{
	static int lastprog = -1;

	if(prog>100)
	{
		prog = 100;
	}
	if(prog<0)
	{
		prog=0;
	}

	if(lastprog==prog)
	{
		return;
	}
	lastprog = prog;

	int pos = x+10;
	if(prog!=0)
	{
		pos += int( float(width-20)/100.0*prog);
		//vordergrund
		frameBuffer->paintBox(x+10, localstatusY,pos, localstatusY+10, COL_MENUCONTENT +7);
	}
	//hintergrund
	frameBuffer->paintBox(pos, localstatusY, x+width-10, localstatusY+10, COL_MENUCONTENT +2);
}

void CProgressWindow::showStatusMessage(string text)
{
	statusText = text;
	frameBuffer->paintBox(x, statusTextY-mheight, x+width, statusTextY,  COL_MENUCONTENT);
	g_Fonts->menu->RenderString(x+10, statusTextY, width-20, text.c_str(), COL_MENUCONTENT);
}


int CProgressWindow::getGlobalStatus()
{
	return globalstatus;
}


void CProgressWindow::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}

void CProgressWindow::paint()
{
	int ypos=y;
	frameBuffer->paintBoxRel(x, ypos, width, hheight, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10, ypos+ hheight, width- 10, g_Locale->getText(caption).c_str(), COL_MENUHEAD);
	frameBuffer->paintBoxRel(x, ypos+ hheight, width, height- hheight, COL_MENUCONTENT);

	ypos+= hheight + (mheight >>1);
	statusTextY = ypos+mheight;
	showStatusMessage(statusText);

	ypos+= mheight;
	localstatusY = ypos+ mheight-20;
	showLocalStatus(0);
	ypos+= mheight+10;

	g_Fonts->menu->RenderString(x+ 10, ypos+ mheight, width- 10, g_Locale->getText("flashupdate.globalprogress").c_str() , COL_MENUCONTENT);
	ypos+= mheight;

	globalstatusY = ypos+ mheight-20;
	ypos+= mheight >>1;
	showGlobalStatus(globalstatus);

}

int CProgressWindow::exec( CMenuTarget* parent, string actionKey )
{
	if(parent)
	{
		parent->hide();
	}
	paint();

	return menu_return::RETURN_REPAINT;
}
