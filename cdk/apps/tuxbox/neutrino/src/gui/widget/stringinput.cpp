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

#include <gui/color.h>

#include "messagebox.h"
#include "stringinput.h"


CStringInput::CStringInput(string Name, char* Value, int Size,  string Hint_1, string Hint_2, char* Valid_Chars, CChangeObserver* Observ, string Icon )
{
	frameBuffer = CFrameBuffer::getInstance();
	name =  Name;
	value = Value;
	size =  Size;

	hint_1 = Hint_1;
	hint_2 = Hint_2;
	validchars = Valid_Chars;
	iconfile = Icon;

	observ = Observ;
	width = (Size*20)+40;

	if (width<420)
		width = 420;

	int neededWidth = g_Fonts->menu_title->getRenderWidth( g_Locale->getText(name).c_str() );
	if ( iconfile != "" )
		neededWidth += 28;
	if (neededWidth+20> width)
		width = neededWidth+20;

	hheight = g_Fonts->menu_title->getHeight();
	mheight = g_Fonts->menu->getHeight();
	iheight = g_Fonts->menu_info->getHeight();

	height = hheight+ mheight+ 50;
	if ( hint_1.length()> 0 )
		height+= iheight;
	if ( hint_2.length()> 0 )
		height+= iheight;

	x = ((720-width)>>1);
	y = ((500-height)>>1);
	selected = 0;
}

void CStringInput::key0_9Pressed(int key)
{
	value[selected]=validchars[key];

	if (selected < (size - 1))
	{
		selected++;
		paintChar(selected - 1);
	}
  
	paintChar(selected);
}

void CStringInput::keyRedPressed()
{
	if ( strstr(validchars, " ")!=NULL )
	{
		value[selected]=' ';

		if (selected < (size - 1))
		{
			selected++;
			paintChar(selected - 1);
		}
  
		paintChar(selected);
	}
}

void CStringInput::keyUpPressed()
{
	int npos = 0;
	for(int count=0;count<(int)strlen(validchars);count++)
		if(value[selected]==validchars[count])
			npos = count;
	npos++;
	if(npos>=(int)strlen(validchars))
		npos = 0;
	value[selected]=validchars[npos];
	paintChar(selected);
}

void CStringInput::keyDownPressed()
{
	int npos = 0;
	for(int count=0;count<(int)strlen(validchars);count++)
		if(value[selected]==validchars[count])
			npos = count;
	npos--;
	if(npos<0)
		npos = strlen(validchars)-1;
	value[selected]=validchars[npos];
	paintChar(selected);
}

void CStringInput::keyLeftPressed()
{
	if(selected>0)
	{
		selected--;
		paintChar(selected+1);
		paintChar(selected);
	}
}

void CStringInput::keyRightPressed()
{
	if (selected < (size - 1))
	{
		selected++;
		paintChar(selected-1);
		paintChar(selected);
	}
}


int CStringInput::exec( CMenuTarget* parent, string )
{
	int res = menu_return::RETURN_REPAINT;
	char oldval[size], dispval[size];

	if (parent)
		parent->hide();

	for(int count=strlen(value)-1;count<size-1;count++)
		strcat(value, " ");
	strcpy(oldval, value);

	paint();

	uint msg; uint data;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );

	bool loop=true;
	while (loop)
	{
		if ( strcmp(value, dispval) != 0)
		{
			g_lcdd->setMenuText(1, value, selected+1);
			strcpy(dispval, value);
		}

		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd, true );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );

		if (msg==CRCInput::RC_left)
		{
			keyLeftPressed();
		}
		else if (msg==CRCInput::RC_right)
		{
			keyRightPressed();
		}
		else if ( ( msg>= 0 ) && ( msg<= 9) )
		{
			key0_9Pressed( msg );
		}
		else if (msg==CRCInput::RC_red)
		{
			keyRedPressed();
		}
		else if ( (msg==CRCInput::RC_green) && ( strstr(validchars, ".")!=NULL ) )
		{
			value[selected]='.';

			if (selected < (size - 1))
			{
				selected++;
				paintChar(selected - 1);
			}
  
			paintChar(selected);
		}
		else if (msg==CRCInput::RC_up)
		{
			keyUpPressed();
		}
		else if (msg==CRCInput::RC_down)
		{
			keyDownPressed();
		}
		else if (msg==CRCInput::RC_ok)
		{
			loop=false;
		}
		else if ( (msg==CRCInput::RC_home) || (msg==CRCInput::RC_timeout) )
		{
			if ( ( strcmp(value, oldval) != 0) &&
			     ( ShowMsg(name, g_Locale->getText("messagebox.discard"), CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbCancel, "", 380 ) == CMessageBox::mbrCancel ) )
				continue;

			strcpy(value, oldval);
			loop=false;
		}
		else
		{
			int r = handleOthers( msg, data );
			if ( r&messages_return::cancel_all )
        	{
            	loop = false;
				res = menu_return::RETURN_EXIT_ALL;
			}
			else if ( r & messages_return::cancel_info )
        	{
            	loop = false;
				res = menu_return::RETURN_EXIT;
			}
			else if ( r & messages_return::unhandled )
			{
				if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
				{
					loop = false;
					res = menu_return::RETURN_EXIT_ALL;
				}
			}
		}
	}

	hide();

	for(int count=size-1;count>=0;count--)
	{
		if((value[count]==' ') || (value[count]==0))
		{
			value[count]=0;
		}
		else
			break;
	}
	value[size]=0;

	if ( (observ) && (msg==CRCInput::RC_ok) )
	{
		observ->changeNotify( name, value );
	}

	return res;
}

int CStringInput::handleOthers( uint msg, uint data )
{
	return messages_return::unhandled;
}

void CStringInput::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height);
}

void CStringInput::paint()
{
	frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD);

	int iconoffset= (iconfile!="")?28:0;
	g_Fonts->menu_title->RenderString(x+ 10+ iconoffset, y+ hheight, width- 10- iconoffset, g_Locale->getText(name).c_str(), COL_MENUHEAD);
	if ( iconoffset> 0 )
		frameBuffer->paintIcon(iconfile.c_str(),x+8,y+5);

	frameBuffer->paintBoxRel(x, y+ hheight, width, height- hheight, COL_MENUCONTENT);




	if ( hint_1.length()> 0 )
	{
		g_Fonts->menu_info->RenderString(x+ 20, y+ hheight+ mheight+ iheight+ 40, width- 20, g_Locale->getText(hint_1).c_str(), COL_MENUCONTENT);
		if ( hint_2.length()> 0 )
			g_Fonts->menu_info->RenderString(x+ 20, y+ hheight+ mheight+ iheight* 2+ 40, width- 20, g_Locale->getText(hint_2).c_str(), COL_MENUCONTENT);
	}

	for (int count=0;count<size;count++)
		paintChar(count);

}

void CStringInput::paintChar(int pos, const char c)
{
	const int xs = 20;
	int ys = mheight;
	int xpos = x+ 20+ pos* xs;
	int ypos = y+ hheight+ 25;

	char ch[2] = {c, 0};

	int color = COL_MENUCONTENT;
	if (pos==selected)
		color = COL_MENUCONTENTSELECTED;

	frameBuffer->paintBoxRel(xpos, ypos, xs, ys, COL_MENUCONTENT+ 4);
	frameBuffer->paintBoxRel(xpos+ 1, ypos+ 1, xs- 2, ys- 2, color);

	int xfpos = xpos + ((xs- g_Fonts->menu->getRenderWidth(ch))>>1);

	g_Fonts->menu->RenderString(xfpos,ypos+ys, width, ch, color);
}

void CStringInput::paintChar(int pos)
{
	if(pos<(int)strlen(value))
		paintChar(pos, value[pos]);
}

CStringInputSMS::CStringInputSMS(string Name, char* Value, int Size, string Hint_1, string Hint_2, char* Valid_Chars, CChangeObserver* Observ, string Icon)
		: CStringInput(Name, Value, Size, Hint_1, Hint_2, Valid_Chars, Observ, Icon)
{
	lastKey = -1;				// no key pressed yet
	const char CharList[10][10] = { "0 -/()<>=",	// 9 characters
					"1.,:!?",
					"abc2ä",
					"def3",
					"ghi4",
					"jkl5",
					"mno6ö",
					"pqrs7ß",
					"tuv8ü",
					"wxyz9" };

	for (int i = 0; i < 10; i++)
	{
		int j = 0;
		for (int k = 0; k < (int) strlen(CharList[i]); k++)
			if (strchr(Valid_Chars, CharList[i][k]) != NULL)
				Chars[i][j++] = CharList[i][k];
		if (j == 0)
			Chars[i][j++] = ' ';	// prevent empty char lists 
		arraySizes[i] = j;
	}

	height+=260;
	y = ((500-height)>>1);
}


void CStringInputSMS::key0_9Pressed(int key)
{
	if (lastKey != key)
	{
		if ((lastKey != -1) &&		// there is a last key
			(selected < (size- 1)))	// we can shift the cursor one field to the right
		{
			selected++;
			paintChar(selected - 1);
		}
		keyCounter = 0;
	}
	else
		keyCounter = (keyCounter + 1) % arraySizes[key];
	value[selected] = Chars[key][keyCounter];
	paintChar(selected);
	lastKey = key;
}

void CStringInputSMS::keyRedPressed()		// switch between lower & uppercase
{
	if (((value[selected]>='a') && (value[selected]<='z')) ||
		((value[selected]>='A') && (value[selected]<='Z')))
	value[selected] ^= 32;

	paintChar(selected);
}

void CStringInputSMS::keyUpPressed()
{}

void CStringInputSMS::keyDownPressed()
{}

void CStringInputSMS::keyLeftPressed()
{
	lastKey = -1;				// no key pressed yet
	CStringInput::keyLeftPressed();
}

void CStringInputSMS::keyRightPressed()
{
	lastKey = -1;				// no key pressed yet
	CStringInput::keyRightPressed();
}

void CStringInputSMS::paint()
{
	CStringInput::paint();

	frameBuffer->paintIcon("numericpad.raw", x+20+140, y+ hheight+ mheight+ iheight* 3+ 30, COL_MENUCONTENT);

	frameBuffer->paintBoxRel(x,y+height-25, width,25, COL_MENUHEAD);
	frameBuffer->paintHLine(x, x+width,  y+height-25, COL_INFOBAR_SHADOW);

	frameBuffer->paintIcon("rot.raw", x+8, y+height-25+1);
	g_Fonts->infobar_small->RenderString(x+38, y+height-25+24 - 2, width, g_Locale->getText("stringinput.caps").c_str(), COL_INFOBAR);

}

void CPINInput::paintChar(int pos)
{
	CStringInput::paintChar(pos, (value[pos] == ' ') ? ' ' : '*');
}

int CPINInput::exec( CMenuTarget* parent, string )
{
	int res = menu_return::RETURN_REPAINT;

	if (parent)
		parent->hide();

	for(int count=strlen(value)-1;count<size-1;count++)
		strcat(value, " ");

	paint();

	bool loop = true;
	uint msg; uint data;

	while(loop)
	{
		g_RCInput->getMsg( &msg, &data, 300 );

		if (msg==CRCInput::RC_left)
		{
			keyLeftPressed();
		}
		else if (msg==CRCInput::RC_right)
		{
			keyRightPressed();
		}
		else if ( ( msg>= 0 ) && ( msg<= 9) )
		{
			int old_selected = selected;
			key0_9Pressed( msg );
			if ( old_selected == ( size- 1 ) )
				loop=false;
		}
		else if ( (msg==CRCInput::RC_up) ||
				  (msg==CRCInput::RC_down) )
		{
			g_RCInput->postMsg( msg, data );
			res = menu_return::RETURN_EXIT;
			loop=false;
		}
		else if ( (msg==CRCInput::RC_home) || (msg==CRCInput::RC_timeout) || (msg==CRCInput::RC_ok) )
		{
			loop=false;
		}
		else
		{
			int r = handleOthers( msg, data );
			if ( r & messages_return::cancel_all )
        	{
            	loop = false;
				res = menu_return::RETURN_EXIT_ALL;
			}
			else if ( r & messages_return::cancel_info )
        	{
            	loop = false;
				res = menu_return::RETURN_EXIT;
			}
			else if ( r & messages_return::unhandled )
			{
				if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & ( messages_return::cancel_all | messages_return::cancel_info ) )
				{
					loop = false;
					res = menu_return::RETURN_EXIT_ALL;
				}
			}
		}

	}

	hide();

	for(int count=size-1;count>=0;count--)
	{
		if((value[count]==' ') || (value[count]==0))
		{
			value[count]=0;
		}
		else
			break;
	}
	value[size]=0;

	if ( (observ) && (msg==CRCInput::RC_ok) )
	{
		observ->changeNotify( name, value );
	}

	return res;
}

int CPLPINInput::handleOthers( uint msg, uint data )
{
	int res = messages_return::unhandled;

	if ( msg == NeutrinoMessages::EVT_PROGRAMLOCKSTATUS )
	{
		// trotzdem handlen
		CNeutrinoApp::getInstance()->handleMsg( msg, data );

		if ( data != (uint) fsk )
			res = messages_return::cancel_info;
	}

	return res;
}

#define borderwidth 4

int CPLPINInput::exec( CMenuTarget* parent, string )
{

	unsigned char* pixbuf= new unsigned char[(width+ 2* borderwidth) * (height+ 2* borderwidth)];
	if (pixbuf!= NULL)
		frameBuffer->SaveScreen(x- borderwidth, y- borderwidth, width+ 2* borderwidth, height+ 2* borderwidth, pixbuf);

	// clear border
	frameBuffer->paintBackgroundBoxRel(x- borderwidth, y- borderwidth, width+ 2* borderwidth, borderwidth);
	frameBuffer->paintBackgroundBoxRel(x- borderwidth, y+ height, width+ 2* borderwidth, borderwidth);
	frameBuffer->paintBackgroundBoxRel(x- borderwidth, y, borderwidth, height);
	frameBuffer->paintBackgroundBoxRel(x+ width, y, borderwidth, height);

	char hint[100];
	if ( fsk == 0x100 )
		strcpy(hint, g_Locale->getText("parentallock.lockedsender").c_str());
	else
		sprintf(hint, g_Locale->getText("parentallock.lockedprogram").c_str(), fsk );

	hint_1= hint;

	int res = CPINInput::exec ( parent, "" );

	if (pixbuf!= NULL)
	{
		frameBuffer->RestoreScreen(x- borderwidth, y- borderwidth, width+ 2* borderwidth, height+ 2* borderwidth, pixbuf);
		delete pixbuf;
	}

	return ( res );
}
