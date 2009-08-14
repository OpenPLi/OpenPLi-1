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

#include "messagebox.h"

#define borderwidth 4


CMessageBox::CMessageBox( string Caption, string Text, CMessageBoxNotifier* Notifier, string Icon, int Width, uint Default, uint ShowButtons )
{
	frameBuffer = CFrameBuffer::getInstance();
	theight= g_Fonts->menu_title->getHeight();
	fheight= g_Fonts->menu->getHeight();
	iconfile = Icon;

	caption = Caption;
	Text = Text+ "\n";
	text.clear();

	int pos;
	do
	{
		pos = Text.find_first_of("\n");
		if ( pos!=-1 )
		{
			text.insert( text.end(), Text.substr( 0, pos ) );
			Text= Text.substr( pos+ 1, uint(-1) );
		}
	} while ( ( pos != -1 ) );
	height = theight+ fheight* ( text.size()+ 3 );

	width = Width;
	if ( width< 450 )
		width = 450;

	int nw= g_Fonts->menu_title->getRenderWidth( g_Locale->getText(caption).c_str() ) + 20;
	if ( iconfile!="" )
		nw+= 30;
	if ( nw> width )
		width= nw;

	for (unsigned int i= 0; i< text.size(); i++)
	{
		int nw= g_Fonts->menu->getRenderWidth( text[i].c_str() ) + 20;
		if ( nw> width )
			width= nw;
	}

	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-height) / 2) + g_settings.screen_StartY;

	notifier = Notifier;
	switch (Default)
	{
		case mbrYes:
			selected = 0;
			break;
		case mbrNo:
			selected = 1;
			break;
		case mbrCancel:
			selected = 2;
			break;
		case mbrBack:
			selected = 2;
			break;
	}
	showbuttons= ShowButtons;
}

void CMessageBox::paintHead()
{

	frameBuffer->paintBoxRel(x,y, width,theight+0, COL_MENUHEAD);
	if ( iconfile!= "" )
	{
		frameBuffer->paintIcon(iconfile.c_str(),x+8,y+5);
		g_Fonts->menu_title->RenderString(x+40, y+theight+0, width- 40, g_Locale->getText(caption).c_str(), COL_MENUHEAD);
	}
	else
		g_Fonts->menu_title->RenderString(x+10, y+theight+0, width- 10, g_Locale->getText(caption), COL_MENUHEAD);

	frameBuffer->paintBoxRel(x,y+theight+0, width,height - theight + 0, COL_MENUCONTENT);
	for (unsigned int i= 0; i< text.size(); i++)
		g_Fonts->menu->RenderString(x+10,y+ theight+ (fheight>>1)+ fheight* (i+ 1), width, text[i].c_str(), COL_MENUCONTENT);

}

void CMessageBox::paintButtons()
{
	//irgendwann alle vergleichen - aber cancel ist sicher der längste
	int MaxButtonTextWidth = g_Fonts->infobar_small->getRenderWidth(g_Locale->getText("messagebox.cancel").c_str());

	int ButtonWidth = 20 + 33 + MaxButtonTextWidth;

//	int ButtonSpacing = 40;
//	int startpos = x + (width - ((ButtonWidth*3)+(ButtonSpacing*2))) / 2;

	int startpos = x + 10;
	int ButtonSpacing = ( width- 20- (ButtonWidth*3) ) / 2;

	int xpos = startpos;
	int color = COL_INFOBAR_SHADOW;

	if ( showbuttons & mbYes )
	{
		if(selected==0)
			color = COL_MENUCONTENTSELECTED;
		frameBuffer->paintBoxRel(xpos, y+height-fheight-20, ButtonWidth, fheight, color);
		frameBuffer->paintIcon("rot.raw", xpos+14, y+height-fheight-15);
		g_Fonts->infobar_small->RenderString(xpos + 43, y+height-fheight+4, ButtonWidth- 53, g_Locale->getText("messagebox.yes"), color);
	}

	xpos = startpos+ButtonWidth+ButtonSpacing;

	if ( showbuttons & mbNo )
	{
		color = COL_INFOBAR_SHADOW;
		if(selected==1)
			color = COL_MENUCONTENTSELECTED;

		frameBuffer->paintBoxRel(xpos, y+height-fheight-20, ButtonWidth, fheight, color);
		frameBuffer->paintIcon("gruen.raw", xpos+14, y+height-fheight-15);
		g_Fonts->infobar_small->RenderString(xpos + 43, y+height-fheight+4, ButtonWidth- 53, g_Locale->getText("messagebox.no"), color);
    }

    xpos = startpos+ButtonWidth*2+ButtonSpacing*2;
    if ( ( showbuttons & mbCancel ) || ( showbuttons & mbBack ) )
	{
		color = COL_INFOBAR_SHADOW;
		if(selected==2)
			color = COL_MENUCONTENTSELECTED;

		frameBuffer->paintBoxRel(xpos, y+height-fheight-20, ButtonWidth, fheight, color);
		frameBuffer->paintIcon("home.raw", xpos+10, y+height-fheight-19);
		g_Fonts->infobar_small->RenderString(xpos + 43, y+height-fheight+4, ButtonWidth- 53, g_Locale->getText( ( showbuttons & mbCancel ) ? "messagebox.cancel" : "messagebox.back" ), color);
	}
}

void CMessageBox::yes()
{
	result = mbrYes;
	if (notifier)
		notifier->onYes();
}

void CMessageBox::no()
{
	result = mbrNo;
	if (notifier)
		notifier->onNo();
}

void CMessageBox::cancel()
{
	if ( showbuttons & mbCancel )
		result = mbrCancel;
	else
		result = mbrBack;
}

void CMessageBox::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}

int CMessageBox::exec(int timeout)
{
	int res = menu_return::RETURN_REPAINT;
    unsigned char pixbuf[(width+ 2* borderwidth) * (height+ 2* borderwidth)];
	frameBuffer->SaveScreen(x- borderwidth, y- borderwidth, width+ 2* borderwidth, height+ 2* borderwidth, pixbuf);

	// clear border
	frameBuffer->paintBackgroundBoxRel(x- borderwidth, y- borderwidth, width+ 2* borderwidth, borderwidth);
	frameBuffer->paintBackgroundBoxRel(x- borderwidth, y+ height, width+ 2* borderwidth, borderwidth);
	frameBuffer->paintBackgroundBoxRel(x- borderwidth, y, borderwidth, height);
	frameBuffer->paintBackgroundBoxRel(x+ width, y, borderwidth, height);

	paintHead();
	paintButtons();

	if ( timeout == -1 )
		timeout = g_settings.timing_epg ;

	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( timeout );
	uint msg; uint data;

	bool loop=true;
	while (loop)
	{

		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( ( (msg==CRCInput::RC_timeout) ||
			   (msg == (uint) g_settings.key_channelList_cancel) ) &&
			 ( ( showbuttons & mbCancel ) || ( showbuttons & mbBack ) ) )
		{
			cancel();
			loop=false;
		}
		else if ( (msg==CRCInput::RC_green) && ( showbuttons & mbNo ) )
		{
			no();
			loop=false;
		}
		else if ( (msg==CRCInput::RC_red) && ( showbuttons & mbYes ) )
		{
			yes();
			loop=false;
		}
		else if(msg==CRCInput::RC_right)
		{
			bool ok = false;
			while (!ok)
			{
				selected++;
				switch (selected)
				{
					case 3:
						selected= -1;
					    break;
					case 0:
						ok = ( showbuttons & mbYes );
						break;
					case 1:
						ok = ( showbuttons & mbNo );
						break;
					case 2:
						ok = ( ( showbuttons & mbCancel ) || ( showbuttons & mbBack ) );
						break;
				}
			}

			paintButtons();
		}
		else if(msg==CRCInput::RC_left)
		{
			bool ok = false;
			while (!ok)
			{
				selected--;
				switch (selected)
				{
					case -1:
						selected= 3;
					    break;
					case 0:
						ok = ( showbuttons & mbYes );
						break;
					case 1:
						ok = ( showbuttons & mbNo );
						break;
					case 2:
						ok = ( ( showbuttons & mbCancel ) || ( showbuttons & mbBack ) );
						break;
				}
			}

			paintButtons();

		}
		else if(msg==CRCInput::RC_ok)
		{
			//exec selected;
			switch (selected)
			{
				case 0: yes();
					break;
				case 1: no();
					break;
				case 2: cancel();
					break;
			}
			loop=false;
		}
		else if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
		{
			loop = false;
			res = menu_return::RETURN_EXIT_ALL;
		}

	}

	frameBuffer->RestoreScreen(x- borderwidth, y- borderwidth, width+ 2* borderwidth, height+ 2* borderwidth, pixbuf);
	return res;
}

int ShowMsg ( string Caption, string Text, uint Default, uint ShowButtons, string Icon, int Width, int timeout )
{
   	CMessageBox* messageBox = new CMessageBox( Caption, Text, NULL, Icon, Width, Default, ShowButtons );
	messageBox->exec( timeout );
	int res= messageBox->result;
	delete messageBox;

	return res;
}
