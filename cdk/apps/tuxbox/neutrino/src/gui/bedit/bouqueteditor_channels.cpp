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


#include <zapit/client/zapitclient.h>

#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>

#include "bouqueteditor_channels.h"
#include "bouqueteditor_chanselect.h"


CBEChannelWidget::CBEChannelWidget(string Caption, unsigned int Bouquet)
{
	frameBuffer = CFrameBuffer::getInstance();
	selected = 0;
	width =  500;
	height = 440;
	ButtonHeight = 25;
	theight= g_Fonts->menu_title->getHeight();
	fheight= g_Fonts->channellist->getHeight();
	listmaxshow = (height-theight-0)/fheight;
	height = theight+0+listmaxshow*fheight; // recalc height
	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-height) / 2) + g_settings.screen_StartY;
	liststart = 0;
	state = beDefault;
	caption = Caption;
	bouquet = Bouquet;
	mode = CZapitClient::MODE_TV;
}

void CBEChannelWidget::paintItem(int pos)
{
	int ypos = y+ theight+0 + pos*fheight;
	int color = COL_MENUCONTENT;
	if (liststart+pos==selected)
	{
		color = COL_MENUCONTENTSELECTED;
	}

	frameBuffer->paintBoxRel(x,ypos, width- 15, fheight, color);
	if ((liststart+pos==selected) && (state == beMoving))
	{
		frameBuffer->paintIcon("gelb.raw", x + 8, ypos+4);
	}
	if(liststart+pos < Channels.size())
	{
		g_Fonts->channellist->RenderString(x+ 5+ numwidth+ 10, ypos+ fheight, width- numwidth- 20- 15, Channels[liststart+pos].name, color);
	}
}

void CBEChannelWidget::paint()
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

	int sbc= ((Channels.size()- 1)/ listmaxshow)+ 1;
	float sbh= (sb- 4)/ sbc;
	int sbs= (selected/listmaxshow);

	frameBuffer->paintBoxRel(x+ width- 13, ypos+ 2+ int(sbs* sbh) , 11, int(sbh),  COL_MENUCONTENT+ 3);

}

void CBEChannelWidget::paintHead()
{
	frameBuffer->paintBoxRel(x,y, width,theight+0, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10,y+theight+0, width, caption.c_str() , COL_MENUHEAD);
}

void CBEChannelWidget::paintFoot()
{
	int ButtonWidth = width / 4;
	frameBuffer->paintBoxRel(x,y+height, width,ButtonHeight, COL_MENUHEAD);
	frameBuffer->paintHLine(x, x+width,  y, COL_INFOBAR_SHADOW);

	frameBuffer->paintIcon("rot.raw", x+width- 4* ButtonWidth+ 8, y+height+4);
	g_Fonts->infobar_small->RenderString(x+width- 4* ButtonWidth+ 29, y+height+24 - 2, ButtonWidth- 26, g_Locale->getText("bouqueteditor.delete").c_str(), COL_INFOBAR);

	frameBuffer->paintIcon("gruen.raw", x+width- 3* ButtonWidth+ 8, y+height+4);
	g_Fonts->infobar_small->RenderString(x+width- 3* ButtonWidth+ 29, y+height+24 - 2, ButtonWidth- 26, g_Locale->getText("bouqueteditor.add").c_str(), COL_INFOBAR);

	frameBuffer->paintIcon("gelb.raw", x+width- 2* ButtonWidth+ 8, y+height+4);
	g_Fonts->infobar_small->RenderString(x+width- 2* ButtonWidth+ 29, y+height+24 - 2, ButtonWidth- 26, g_Locale->getText("bouqueteditor.move").c_str(), COL_INFOBAR);

	frameBuffer->paintIcon("blau.raw", x+width- ButtonWidth+ 8, y+height+4);
	g_Fonts->infobar_small->RenderString(x+width- ButtonWidth+ 29, y+height+24 - 2, ButtonWidth- 26, g_Locale->getText("bouqueteditor.switchmode").c_str(), COL_INFOBAR);
}

void CBEChannelWidget::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height+ButtonHeight);
}

int CBEChannelWidget::exec(CMenuTarget* parent, string actionKey)
{
	int res = menu_return::RETURN_REPAINT;

	if (parent)
	{
		parent->hide();
	}

	Channels.clear();
	g_Zapit->getBouquetChannels( bouquet, Channels, mode);
	paintHead();
	paint();
	paintFoot();

	channelsChanged = false;
	uint msg; uint data;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_epg );

	bool loop=true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_epg );

		if ((msg==CRCInput::RC_timeout) || (msg == (uint) g_settings.key_channelList_cancel))
		{
			if (state == beDefault)
			{
				loop = false;
			}
			else if (state == beMoving)
			{
				cancelMoveChannel();
			}
		}
		//
		// -- For more convenience: include browsing of list (paging)  (rasc, 2002-04-02)
		// -- The keys should be configurable. Problem is: red/green key, which is the
		// -- default in neutrino is used as a function key here... so use left/right
		//
		else if (msg==CRCInput::RC_up || msg==CRCInput::RC_left)
		{
			int step = 0;
			int prev_selected = selected;

			step = (msg == CRCInput::RC_left) ? listmaxshow : 1;  // browse or step 1
			selected -= step;
			if((prev_selected-step) < 0)		// because of uint
			{
				selected = Channels.size()-1;
			}

			if (state == beDefault)
			{
				paintItem(prev_selected - liststart);
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
			else if (state == beMoving)
			{
				internalMoveChannel(prev_selected, selected);
			}
		}
		else if (msg==CRCInput::RC_down || msg==CRCInput::RC_right)
		{
			int step = 0;
			int prev_selected = selected;

			step = (msg == CRCInput::RC_right) ? listmaxshow : 1;  // browse or step 1
			selected += step;

			if(selected >= Channels.size())
			{
				selected = 0;
			}

			if (state == beDefault)
			{
				paintItem(prev_selected - liststart);
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
			else if (state == beMoving)
			{
				internalMoveChannel(prev_selected, selected);
			}
		}
		else if(msg==CRCInput::RC_red)
		{
			if (state == beDefault)
				deleteChannel();
		}
		else if(msg==CRCInput::RC_green)
		{
			if (state == beDefault)
				addChannel();
		}
		else if(msg==CRCInput::RC_blue)
		{
			if (state == beDefault)
			{
				if (mode == CZapitClient::MODE_TV)
					mode = CZapitClient::MODE_RADIO;
				else
					mode = CZapitClient::MODE_TV;
				Channels.clear();
				g_Zapit->getBouquetChannels( bouquet, Channels, mode);
				paint();
			}
		}
		else if(msg==CRCInput::RC_yellow)
		{
			liststart = (selected/listmaxshow)*listmaxshow;
			if (state == beDefault)
				beginMoveChannel();
			paintItem(selected - liststart);
		}
		else if(msg==CRCInput::RC_ok)
		{
			if (state == beDefault)
			{
			}
			else if (state == beMoving)
			{
				finishMoveChannel();
			}
		}
		else if( CRCInput::isNumeric(msg) )
		{
			if (state == beDefault)
			{
				//kein pushback - wenn man versehentlich wo draufkommt is die edit-arbeit umsonst
				//selected = oldselected;
				//g_RCInput->postMsg( msg, data );
				//loop=false;
			}
			else if (state == beMoving)
			{
				cancelMoveChannel();
			}
		}
		else
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
			// kein canceling...
		}
	}
	hide();
	return res;
}

void CBEChannelWidget::deleteChannel()
{
	g_Zapit->removeChannelFromBouquet( bouquet, Channels[selected].channel_id);
	Channels.clear();
	g_Zapit->getBouquetChannels( bouquet, Channels, mode);
	if (selected >= Channels.size())
		selected--;
	channelsChanged = true;
	paint();
}

void CBEChannelWidget::addChannel()
{
	printf("new\n");
	CBEChannelSelectWidget* channelSelectWidget = new CBEChannelSelectWidget(caption, bouquet, mode);
	printf("exec\n");
	channelSelectWidget->exec(this, "");
	printf("exec done\n");
	if (channelSelectWidget->hasChanged())
	{
		channelsChanged = true;
		Channels.clear();
		g_Zapit->getBouquetChannels( bouquet, Channels, mode);
	}
	delete channelSelectWidget;
	paintHead();
	paint();
	paintFoot();
}

void CBEChannelWidget::beginMoveChannel()
{
	state = beMoving;
	origPosition = selected;
	newPosition = selected;
}

void CBEChannelWidget::finishMoveChannel()
{
	state = beDefault;
	if (newPosition != origPosition)
	{
		g_Zapit->moveChannel( bouquet, origPosition+1, newPosition+1, mode);
		Channels.clear();
		g_Zapit->getBouquetChannels( bouquet, Channels, mode);
		channelsChanged = true;
	}
	paint();
}

void CBEChannelWidget::cancelMoveChannel()
{
	state = beDefault;
	internalMoveChannel( newPosition, origPosition);
}

void CBEChannelWidget::internalMoveChannel( unsigned int fromPosition, unsigned int toPosition)
{
	if ( (int) toPosition == -1 ) return;
	if ( toPosition == Channels.size()) return;

	CZapitClient::responseGetBouquetChannels Channel = Channels[fromPosition];
	if (fromPosition < toPosition)
	{
		for (unsigned int i=fromPosition; i<toPosition; i++)
			Channels[i] = Channels[i+1];
	}
	else if (fromPosition > toPosition)
	{
		for (unsigned int i=fromPosition; i>toPosition; i--)
			Channels[i] = Channels[i-1];
	}
	Channels[toPosition] = Channel;
	selected = toPosition;
	newPosition = toPosition;
	paint();
}

bool CBEChannelWidget::hasChanged()
{
	return (channelsChanged);
}
