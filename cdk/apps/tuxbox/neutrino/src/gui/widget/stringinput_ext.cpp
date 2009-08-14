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
#include "stringinput_ext.h"


CExtendedInput::CExtendedInput(string Name, char* Value, string Hint_1, string Hint_2, CChangeObserver* Observ, bool Localizing)
{
	frameBuffer = CFrameBuffer::getInstance();
	name = Name;
	value = Value;

	hint_1 = Hint_1;
	hint_2 = Hint_2;

	observ = Observ;

	hheight = g_Fonts->menu_title->getHeight();
	mheight = g_Fonts->menu->getHeight();
	iheight = g_Fonts->menu_info->getHeight();

	localizing = Localizing;
	if(localizing)
	{
		width = g_Fonts->menu_title->getRenderWidth( g_Locale->getText(name).c_str())+20;
	}
	else
	{
		width = g_Fonts->menu_title->getRenderWidth( name.c_str())+20;
	}
	height = hheight+ mheight+ 20;

	if ( hint_1.length()> 0 )
		height+= iheight;
	if ( hint_2.length()> 0 )
		height+= iheight;

	x = ((720-width)>>1);
	y = ((500-height)>>1);
}

void CExtendedInput::addInputField( CExtendedInput_Item* fld )
{
	inputFields.insert(inputFields.end(), fld);
}


void CExtendedInput::calculateDialog()
{
	int ix = 0;
	int iy = 0;
	int maxX = 0;
	int maxY = 0;
	selectedChar = -1;
	for(unsigned int i=0; i<inputFields.size();i++)
	{
		inputFields[i]->init( ix, iy);
		inputFields[i]->setDataPointer( &value[i] );
		if ((selectedChar==-1) && (inputFields[i]->isSelectable()))
		{
			selectedChar = i;
		}
		maxX = ix>maxX?ix:maxX;
		maxY = iy>maxY?iy:maxY;
	}

	width = width>maxX+40?width:maxX+40;
	height = height>maxY+hheight+ mheight?height:maxY+hheight+ mheight;

	hintPosY = y + height -10;

	if ( hint_1.length()> 0 )
		height+= iheight;
	if ( hint_2.length()> 0 )
		height+= iheight;

	x = ((720-width)>>1);
	y = ((500-height)>>1);
}


int CExtendedInput::exec( CMenuTarget* parent, string )
{
	onBeforeExec();
	int res = menu_return::RETURN_REPAINT;
	char oldval[inputFields.size()+10], dispval[inputFields.size()+10];

	if (parent)
	{
		parent->hide();
	}

	strcpy(oldval, value);
	paint();

	uint msg; uint data;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );

	bool loop=true;
	while (loop)
	{
		if ( strcmp(value, dispval) != 0)
		{
			g_lcdd->setMenuText(1, value, selectedChar+1);
			strcpy(dispval, value);
		}

		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd, true );

		if (msg==CRCInput::RC_left)
		{
			if(selectedChar>0)
			{
				bool found = false;
				int oldSelectedChar = selectedChar;
				for(int i=selectedChar-1; i>=0;i--)
				{
					if ((!found) && (inputFields[i]->isSelectable()))
					{
						found = true;
						selectedChar = i;
					}
				}
				if(found)
				{
					inputFields[oldSelectedChar]->paint( x+20, y+hheight +20, false );
					inputFields[selectedChar]->paint( x+20, y+hheight +20, true );
					g_lcdd->setMenuText(1, value, selectedChar+1);
				}
			}
		}
		else if (msg==CRCInput::RC_right)
		{
			if(selectedChar< (int) inputFields.size()-1)
			{
				bool found = false;
				int oldSelectedChar = selectedChar;
				for(unsigned int i=selectedChar+1; i<inputFields.size();i++)
				{
					if ((!found) && (inputFields[i]->isSelectable()))
					{
						found = true;
						selectedChar = i;
					}
				}
				if(found)
				{
					inputFields[oldSelectedChar]->paint( x+20, y+hheight +20, false );
					inputFields[selectedChar]->paint( x+20, y+hheight +20, true );
					g_lcdd->setMenuText(1, value, selectedChar+1);
				}
			}
		}
		else if ( ((msg>= 0) && (msg<=9)) || (msg == CRCInput::RC_red) || (msg == CRCInput::RC_green) || (msg == CRCInput::RC_blue) || (msg == CRCInput::RC_yellow)
					|| (msg == CRCInput::RC_up) || (msg == CRCInput::RC_down))
		{
			inputFields[selectedChar]->keyPressed(msg);
			inputFields[selectedChar]->paint( x+20, y+hheight +20, true );
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
		else if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
		{
			loop = false;
			res = menu_return::RETURN_EXIT_ALL;
		}
	}

	hide();

	onAfterExec();

	if ( (observ) && (msg==CRCInput::RC_ok) )
	{
		observ->changeNotify( name, value );
	}

	return res;
}

void CExtendedInput::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height);
}

void CExtendedInput::paint()
{
	frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD);
	if(localizing)
	{
		g_Fonts->menu_title->RenderString(x+ 10, y+ hheight, width- 10, g_Locale->getText(name).c_str(), COL_MENUHEAD);
	}
	else
	{
		g_Fonts->menu_title->RenderString(x+ 10, y+ hheight, width- 10, name.c_str(), COL_MENUHEAD);
	}
	frameBuffer->paintBoxRel(x, y+ hheight, width, height-hheight, COL_MENUCONTENT);

	if ( hint_1.length()> 0 )
	{
		if(localizing)
		{
			g_Fonts->menu_info->RenderString(x+ 20, hintPosY, width- 20, g_Locale->getText(hint_1).c_str(), COL_MENUCONTENT);
		}
		else
		{
			g_Fonts->menu_info->RenderString(x+ 20, hintPosY, width- 20, hint_1.c_str(), COL_MENUCONTENT);
		}
		if ( hint_2.length()> 0 )
		{
			if(localizing)
			{
				g_Fonts->menu_info->RenderString(x+ 20, hintPosY + iheight, width- 20, g_Locale->getText(hint_2).c_str(), COL_MENUCONTENT);
			}
			else
			{
				g_Fonts->menu_info->RenderString(x+ 20, hintPosY + iheight, width- 20, hint_2.c_str(), COL_MENUCONTENT);
			}

		}
	}

	for(unsigned int i=0; i<inputFields.size();i++)
	{
		inputFields[i]->paint( x+20, y+hheight +20, (i== (unsigned int) selectedChar) );
	}


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


CExtendedInput_Item_Char::CExtendedInput_Item_Char(string Chars, bool Selectable )
{
	frameBuffer = CFrameBuffer::getInstance();
	idx = 20;
	idy = g_Fonts->menu->getHeight();
	allowedChars = Chars;
	selectable = Selectable;
}

void CExtendedInput_Item_Char::init(int &x, int &y)
{
	ix = x;
	iy = y;
	x += idx;
}

void CExtendedInput_Item_Char::setAllowedChars( string ac )
{
	allowedChars = ac;
}

void CExtendedInput_Item_Char::paint(int x, int y, bool focusGained )
{
	int startx = ix + x;
	int starty = iy + y;

	int color = COL_MENUCONTENT;
	if (focusGained)
		color = COL_MENUCONTENTSELECTED;

	frameBuffer->paintBoxRel( startx, starty, idx, idy, COL_MENUCONTENT+4);
	frameBuffer->paintBoxRel( startx+1, starty+1, idx-2, idy-2, color);

	char text[2];
	text[0] = *data;
	text[1] = 0;
	int xfpos = startx + 1 + ((idx- g_Fonts->menu->getRenderWidth( text ))>>1);

	g_Fonts->menu->RenderString(xfpos,starty+idy, idx, text, color);
}

bool CExtendedInput_Item_Char::isAllowedChar( char ch )
{
	return ( (int) allowedChars.find(ch) != -1);
}

int CExtendedInput_Item_Char::getCharID( char ch )
{
	return allowedChars.find(ch);
}

void CExtendedInput_Item_Char::keyPressed( int key )
{
	if(isAllowedChar( (char) '0' + key))
	{
		*data = (char) '0' + key;
		g_RCInput->postMsg( CRCInput::RC_right, 0 );
		return;
	}
	else
	{
		unsigned int pos = getCharID( *data );
		if (key==CRCInput::RC_up)
		{
			if(pos<allowedChars.size()-1)
			{
				*data = allowedChars[pos+1];
			}
			else
			{
				*data = allowedChars[0];
			}
		}
		else if (key==CRCInput::RC_down)
		{
			if(pos>0)
			{
				*data = allowedChars[pos-1];
			}
			else
			{
				*data = allowedChars[allowedChars.size()-1];
			}
		}
	}
}

//-----------------------------#################################-------------------------------------------------------

CIPInput::CIPInput(string Name, char* Value, string Hint_1, string Hint_2, CChangeObserver* Observ)
	: CExtendedInput(Name, Value, Hint_1, Hint_2, Observ)
{
	frameBuffer = CFrameBuffer::getInstance();
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Spacer(20) );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Spacer(20) );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Spacer(20) );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_newLiner(30) );
	calculateDialog();
}

void CIPInput::onBeforeExec()
{
	if(strcmp(value,"")==0)
	{
		strcpy(value, "000.000.000.000");
		printf("[neutrino] value-before(2): %s\n", value);
		return;
	}
	int _ip[4];
	sscanf( value, "%d.%d.%d.%d", &_ip[0], &_ip[1], &_ip[2], &_ip[3] );
	sprintf( value, "%03d.%03d.%03d.%03d", _ip[0], _ip[1], _ip[2], _ip[3]);
}

void CIPInput::onAfterExec()
{
	int _ip[4];
	sscanf( value, "%3d.%3d.%3d.%3d", &_ip[0], &_ip[1], &_ip[2], &_ip[3] );
	sprintf( value, "%d.%d.%d.%d", _ip[0], _ip[1], _ip[2], _ip[3]);
	if(strcmp(value,"0.0.0.0")==0)
	{
		strcpy(value, "");
	}
}

//-----------------------------#################################-------------------------------------------------------
CDateInput::CDateInput(string Name, time_t* Time, string Hint_1, string Hint_2, CChangeObserver* Observ)
	: CExtendedInput(Name, "", Hint_1, Hint_2, Observ)
{
	time=Time;
	value= new char[20];
	struct tm *tmTime = localtime(time);
	sprintf( value, "%02d.%02d.%04d %02d:%02d", tmTime->tm_mday, tmTime->tm_mon+1,
				tmTime->tm_year+1900,
				tmTime->tm_hour, tmTime->tm_min);
	
	frameBuffer = CFrameBuffer::getInstance();
	addInputField( new CExtendedInput_Item_Char("0123") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char(".",false) );
	addInputField( new CExtendedInput_Item_Char("01") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char(".",false) );
	addInputField( new CExtendedInput_Item_Char("2",false) );
	addInputField( new CExtendedInput_Item_Char("0",false) );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Spacer(20) );
	addInputField( new CExtendedInput_Item_Char("012") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_Char(":",false) );
	addInputField( new CExtendedInput_Item_Char("012345") );
	addInputField( new CExtendedInput_Item_Char("0123456789") );
	addInputField( new CExtendedInput_Item_newLiner(30) );
	calculateDialog();
}
CDateInput::~CDateInput()
{
	delete value;
}
void CDateInput::onBeforeExec()
{
	struct tm *tmTime = localtime(time);
	sprintf( value, "%02d.%02d.%04d %02d:%02d", tmTime->tm_mday, tmTime->tm_mon+1,
				tmTime->tm_year+1900,
				tmTime->tm_hour, tmTime->tm_min);
}

void CDateInput::onAfterExec()
{
	struct tm tmTime;
	sscanf( value, "%02d.%02d.%04d %02d:%02d", &tmTime.tm_mday, &tmTime.tm_mon,
				&tmTime.tm_year,
				&tmTime.tm_hour, &tmTime.tm_min);
	tmTime.tm_mon-=1;
	tmTime.tm_year-=1900;
	tmTime.tm_sec=0;

	if(tmTime.tm_year>129)
      tmTime.tm_year=129;
   if(tmTime.tm_year<0)
      tmTime.tm_year=0;
   if(tmTime.tm_mon>11)
      tmTime.tm_mon=11;
   if(tmTime.tm_mon<0)
      tmTime.tm_mon=0;
   if(tmTime.tm_mday>31) //-> eine etwas laxe pruefung, aber mktime biegt das wieder grade
      tmTime.tm_mday=31;
   if(tmTime.tm_mday<1)
      tmTime.tm_mday=1;
   if(tmTime.tm_hour>23)
      tmTime.tm_hour=23;
   if(tmTime.tm_hour<0)
      tmTime.tm_hour=0;
   if(tmTime.tm_min>59)
      tmTime.tm_min=59;
   if(tmTime.tm_min<0)
      tmTime.tm_min=0;
   if(tmTime.tm_sec>59)
      tmTime.tm_sec=59;
   if(tmTime.tm_sec<0)
      tmTime.tm_sec=0;
	*time=mktime(&tmTime);
	struct tm *tmTime2 = localtime(time);
	sprintf( value, "%02d.%02d.%04d %02d:%02d", tmTime2->tm_mday, tmTime2->tm_mon+1,
				tmTime2->tm_year+1900,
				tmTime2->tm_hour, tmTime2->tm_min);
}
