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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#include <global.h>
#include <neutrino.h>

#include "listbox.h"


CListBox::CListBox()
{
	frameBuffer = CFrameBuffer::getInstance();
	caption = "";
	saveBoxCaption = "";
	saveBoxText = 	 "";
	liststart = 0;
	selected =  0;
	width =  400;
	height = 420;
	ButtonHeight = 25;
	toSave = false;
	theight= g_Fonts->menu_title->getHeight();
	fheight= g_Fonts->channellist->getHeight();
	listmaxshow = (height-theight-0)/fheight;
	height = theight+0+listmaxshow*fheight; // recalc height
	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-height) / 2) + g_settings.screen_StartY;
}

void CListBox::setTitle( string title )
{
	caption = title;
}

void CListBox::setSaveDialogText(string title, string text)
{
	saveBoxCaption = title;
	saveBoxText = text;

}

void CListBox::setModified( bool modified)
{
	toSave = modified;
}

void CListBox::paint()
{
	liststart = (selected/listmaxshow)*listmaxshow;
	int lastnum =  liststart + listmaxshow;

	if(lastnum<10)
		numwidth = g_Fonts->channellist_number->getRenderWidth("0");
	else if(lastnum<100)
		numwidth = g_Fonts->channellist_number->getRenderWidth("00");
	else if(lastnum<1000)
		numwidth = g_Fonts->channellist_number->getRenderWidth("000");
	else if(lastnum<10000)
		numwidth = g_Fonts->channellist_number->getRenderWidth("0000");
	else // if(lastnum<100000)
		numwidth = g_Fonts->channellist_number->getRenderWidth("00000");

	for(unsigned int count=0;count<listmaxshow;count++)
	{
		paintItem(count);
	}

	int ypos = y+ theight;
	int sb = fheight* listmaxshow;
	frameBuffer->paintBoxRel(x+ width- 15,ypos, 15, sb,  COL_MENUCONTENT+ 1);

	int sbc= ((getItemCount()- 1)/ listmaxshow)+ 1;
	float sbh= (sb- 4)/ sbc;
	int sbs= (selected/listmaxshow);

	frameBuffer->paintBoxRel(x+ width- 13, ypos+ 2+ int(sbs* sbh) , 11, int(sbh),  COL_MENUCONTENT+ 3);
}

void CListBox::paintHead()
{
	frameBuffer->paintBoxRel(x,y, width,theight+0, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10,y+theight+0, width, caption.c_str() , COL_MENUHEAD);
}

void CListBox::paintFoot()
{
	int ButtonWidth = width / 4;
	frameBuffer->paintBoxRel(x,y+height, width,ButtonHeight, COL_MENUHEAD);
	frameBuffer->paintHLine(x, x+width,  y, COL_INFOBAR_SHADOW);

	frameBuffer->paintIcon("ok.raw", x+width- 4* ButtonWidth+ 8, y+height+1);
	g_Fonts->infobar_small->RenderString(x+width- 4* ButtonWidth+ 38, y+height+24 - 2, width, "edit", COL_INFOBAR);

	frameBuffer->paintIcon("gruen.raw", x+width- 3* ButtonWidth+ 8, y+height+4);
	g_Fonts->infobar_small->RenderString(x+width- 3* ButtonWidth+ 29, y+height+24 - 2, width- 26, "add", COL_INFOBAR);

	frameBuffer->paintIcon("rot.raw", x+width- 2* ButtonWidth+ 8, y+height+4);
	g_Fonts->infobar_small->RenderString(x+width- 2* ButtonWidth+ 29, y+height+24 - 2, width- 26, "remove", COL_INFOBAR);

	frameBuffer->paintIcon("home.raw", x+width - ButtonWidth+ 8, y+height+1);
	g_Fonts->infobar_small->RenderString(x+width - ButtonWidth+ 38, y+height+24 - 2, width, "ready", COL_INFOBAR);
}

void CListBox::paintItem(int pos)
{
	paintItem(liststart+pos, pos, (liststart+pos==selected) );
}

void CListBox::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height+ButtonHeight);
}

unsigned int	CListBox::getItemCount()
{
	return 10;
}

int CListBox::getItemHeight()
{
	return fheight;
}

void CListBox::paintItem(unsigned int itemNr, int paintNr, bool selected)
{
	int ypos = y+ theight + paintNr*getItemHeight();
	int color = COL_MENUCONTENT;
	if (selected)
	{
		color = COL_MENUCONTENTSELECTED;
	}
	frameBuffer->paintBoxRel(x,ypos, width- 15, getItemHeight(), color);
	g_Fonts->channellist->RenderString(x + 10, ypos+ fheight, width-20, "demo", color);
}

int CListBox::exec(CMenuTarget* parent, string actionKey)
{
	int res = menu_return::RETURN_REPAINT;
	selected=0;

	if (parent)
	{
		parent->hide();
	}

	paintHead();
	paint();
	paintFoot();

	bool loop=true;
	toSave = false;
	while (loop)
	{
		uint msg; uint data;
		g_RCInput->getMsg( &msg, &data, g_settings.timing_epg );

		if (( msg == (uint) g_settings.key_channelList_cancel) || ( msg ==CRCInput::RC_home))
		{
			loop = false;
		}
		else if ( msg ==CRCInput::RC_up)
		{
			if(getItemCount()!=0)
			{
				int prevselected=selected;
				if(selected==0)
				{
					selected = getItemCount()-1;
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
		}
		else if ( msg ==CRCInput::RC_down)
		{
			if(getItemCount()!=0)
			{
				int prevselected=selected;
				selected = (selected+1)%getItemCount();
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
		}
		else if ( msg == (uint) g_settings.key_channelList_pageup)
		{
			selected+=listmaxshow;
			if (selected>getItemCount()-1)
				selected=0;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if ( msg == (uint) g_settings.key_channelList_pagedown)
		{
			if ((int(selected)-int(listmaxshow))<0)
				selected=getItemCount()-1;
			else
				selected -= listmaxshow;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if( msg ==CRCInput::RC_ok)
		{
			onOkKeyPressed();
		}
		else if ( msg ==CRCInput::RC_red)
		{
			onRedKeyPressed();
		}
		else if ( msg ==CRCInput::RC_green)
		{
			onGreenKeyPressed();
		}
		else if ( msg ==CRCInput::RC_yellow)
		{
			onYellowKeyPressed();
		}
		else if ( msg ==CRCInput::RC_blue)
		{
			onBlueKeyPressed();
		}
		else
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
			// kein canceling...
		}
	}

	//want2save?
	if((toSave) && (saveBoxCaption!="") && (saveBoxText!=""))
	{
		if( ShowMsg ( saveBoxCaption, saveBoxText, CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo ) == CMessageBox::mbrYes )
		{
			onSaveData();
		}
	}

	hide();
	return res;
}
