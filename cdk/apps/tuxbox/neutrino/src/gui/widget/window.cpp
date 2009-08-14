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

#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>

#include "window.h"


CWindow::CWindow(string title, string icon, CDimension clientWindowDimension, int mode, CPoint windowPosition)
{
	sWindowTitle = title;
	sWindowIcon = icon;
	cClientWindowDimension = clientWindowDimension;
	cWindowOrigin = windowPosition;
	iWindowMode = mode;
	calculateWindow();
}


void CWindow::calculateWindow()
{
	//origin, windowsize usw.. berechnen
	iTitleHeight = g_Fonts->menu_title->getHeight();
	cWindowDimension = cClientWindowDimension;
	if(!(iWindowMode & WINDOW_NOTITLEBAR))
	{
		cWindowDimension.addHeight(iTitleHeight);
	}

	if(iWindowMode & WINDOW_CENTER)
	{
		int x = (((g_settings.screen_EndX-g_settings.screen_StartX) -cWindowDimension.getWidth()) >> 1) + g_settings.screen_StartX;
		int y = (((g_settings.screen_EndY-g_settings.screen_StartY) -cWindowDimension.getHeight()) >> 1) + g_settings.screen_StartY;		
		cWindowOrigin.setXPos(x);
		cWindowOrigin.setYPos(y);
	}
}

void CWindow::setClientWindowDimension(CDimension cientWindowSize)
{
	hide();
	cWindowDimension = cientWindowSize;
	calculateWindow();
	paint();
}

void CWindow::setWindowOrigin(CPoint windowPosition)
{
	hide();
	cWindowOrigin = windowPosition;
	calculateWindow();
	paint();
}

void CWindow::setWindowTitle(string title)
{
	sWindowTitle = title;
	if(isActivated())
	{
		paintHeader();
	}
}

void CWindow::setWindowIcon(string icon)
{
	sWindowIcon = icon;
	if(isActivated())
	{
		paintHeader();
	}
}

void CWindow::paint()
{
	if(isActivated())
	{
		paintHeader();
		paintContent();
	}
}

void CWindow::hide()
{
	if(isActivated())
	{
		CFrameBuffer::getInstance()->paintBackgroundBoxRel(cWindowOrigin, cWindowDimension);
	}
}
