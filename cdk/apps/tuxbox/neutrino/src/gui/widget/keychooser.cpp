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

#include <gui/color.h>

#include "keychooser.h"


CKeyChooser::CKeyChooser( int* Key, string title, string Icon )
		: CMenuWidget(title, Icon)
{
	frameBuffer = CFrameBuffer::getInstance();
	key = Key;
	keyChooser = new CKeyChooserItem("keychooser.head", key);
	keyDeleter = new CKeyChooserItemNoKey(key);

	addItem( new CMenuSeparator(CMenuSeparator::STRING, " ") );
	addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	addItem( new CMenuForwarder("menu.back") );
	addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	addItem( new CMenuForwarder("keychoosermenu.setnew", true, "", keyChooser) );
	addItem( new CMenuForwarder("keychoosermenu.setnone", true, "", keyDeleter) );
}


CKeyChooser::~CKeyChooser()
{
	delete keyChooser;
	delete keyDeleter;
}


void CKeyChooser::paint()
{
	CMenuWidget::paint();

	string text = g_Locale->getText("keychoosermenu.currentkey") + ": " + CRCInput::getKeyName(*key);
	g_Fonts->menu->RenderString(x+ 10, y+ 65, width, text.c_str(), COL_MENUCONTENT);
}

//*****************************
CKeyChooserItem::CKeyChooserItem(string Name, int *Key)
{
	frameBuffer = CFrameBuffer::getInstance();
	name = Name;
	key = Key;
	width = 350;
	hheight = g_Fonts->menu_title->getHeight();
	mheight = g_Fonts->menu->getHeight();
	height = hheight+2*mheight;
	x=((720-width) >> 1) -20;
	y=(576-height)>>1;
}


int CKeyChooserItem::exec(CMenuTarget* parent, string)
{
	int res = menu_return::RETURN_REPAINT;

	if (parent)
	{
		parent->hide();
	}
	paint();

	g_RCInput->clearRCMsg();

	uint msg; uint data;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );

	bool loop=true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg == CRCInput::RC_timeout )
			loop = false;
		else
		{
			if ( ( msg >= 0 ) && ( msg <= CRCInput::RC_MaxRC ) )
			{
				loop = false;
				*key = msg;
			}
			else if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
			{
				res = menu_return::RETURN_EXIT_ALL;
				loop = false;
			}
		}
	}

	hide();
	return res;
}

void CKeyChooserItem::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height);
}

void CKeyChooserItem::paint()
{

	frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+ 10, y+ hheight, width, g_Locale->getText(name).c_str(), COL_MENUHEAD);
	frameBuffer->paintBoxRel(x, y+ hheight, width, height-hheight, COL_MENUCONTENT);

	//paint msg...
	g_Fonts->menu->RenderString(x+ 10, y+ hheight+ mheight, width, g_Locale->getText("keychooser.text1").c_str(), COL_MENUCONTENT);
	g_Fonts->menu->RenderString(x+ 10, y+ hheight+ mheight* 2, width, g_Locale->getText("keychooser.text2").c_str(), COL_MENUCONTENT);
}
