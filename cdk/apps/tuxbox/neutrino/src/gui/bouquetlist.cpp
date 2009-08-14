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
#include <daemonc/remotecontrol.h>
#include <system/settings.h>

#include "widget/menue.h"

#include "bouquetlist.h"
#include "color.h"
#include "eventlist.h"
#include "infoviewer.h"


CBouquetList::CBouquetList( const std::string &Name )
{
	frameBuffer = CFrameBuffer::getInstance();
	name = Name;
	selected = 0;
	width =  500;
	height = 440;
	theight= g_Fonts->menu_title->getHeight();
	fheight= g_Fonts->channellist->getHeight();
	listmaxshow = (height-theight-0)/fheight;
	height = theight+0+listmaxshow*fheight; // recalc height
	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-height) / 2) + g_settings.screen_StartY;
	liststart = 0;
	tuned=0xfffffff;
}

CBouquetList::~CBouquetList()
{
	Bouquets.clear();
}

CBouquet* CBouquetList::addBouquet(const std::string& name, int BouquetKey, bool locked)
{
	if ( BouquetKey==-1 )
		BouquetKey= Bouquets.size();

	CBouquet* tmp = new CBouquet( BouquetKey, name, locked );
	Bouquets.insert(Bouquets.end(), tmp);
	return(tmp);
}

int CBouquetList::getActiveBouquetNumber()
{
	return selected;
}

int CBouquetList::showChannelList( int nBouquet)
{
	if (nBouquet == -1)
		nBouquet = selected;

	int nNewChannel = Bouquets[nBouquet]->channelList->show();
	if (nNewChannel > -1)
	{
		selected = nBouquet;
		orgChannelList->zapTo(Bouquets[selected]->channelList->getKey(nNewChannel)-1);

		nNewChannel= -2; // exit!
	}

	return nNewChannel;
}

void CBouquetList::adjustToChannel( int nChannelNr)
{
	for (uint i=0; i<Bouquets.size(); i++)
	{
		int nChannelPos = Bouquets[i]->channelList->hasChannel(nChannelNr);
		if (nChannelPos > -1)
		{
			selected = i;
			Bouquets[i]->channelList->setSelected(nChannelPos);
			return;
		}
	}
}


int CBouquetList::activateBouquet( int id, bool bShowChannelList)
{
	int res = menu_return::RETURN_REPAINT;

	selected = id;
	if (bShowChannelList)
	{
		int nNewChannel = Bouquets[selected]->channelList->show();

		if (nNewChannel > -1)
		{
			orgChannelList->zapTo(Bouquets[selected]->channelList->getKey(nNewChannel)-1);
		}
		else if ( nNewChannel == -2 )
		{
			// -2 bedeutet EXIT_ALL
			res = menu_return::RETURN_EXIT_ALL;
		}
	}

	return res;
}

int CBouquetList::exec( bool bShowChannelList)
{
    int res= show();

	if ( res > -1)
	{
		return activateBouquet( selected, bShowChannelList );
	}
	else if ( res == -1)
	{
		// -1 bedeutet nur REPAINT
		return menu_return::RETURN_REPAINT;
	}
	else
	{
		// -2 bedeutet EXIT_ALL
		return menu_return::RETURN_EXIT_ALL;
	}

	return res;
}

int CBouquetList::show()
{
	int res = -1;
	if(Bouquets.size()==0)
	{
		return res;
	}

	int maxpos= 1;
	int i= Bouquets.size();
	while ((i= i/10)!=0)
		maxpos++;

	paintHead();
	paint();

	int oldselected = selected;
	int firstselected = selected+ 1;
	int zapOnExit = false;

	unsigned int chn= 0;
	int pos= maxpos;

	uint msg; uint data;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_chanlist );

	bool loop=true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_chanlist );

		if ( ( msg == CRCInput::RC_timeout ) ||
			 ( msg == (uint) g_settings.key_channelList_cancel ) )
		{
			selected = oldselected;
			loop=false;
		}
		else if ( msg == (uint) g_settings.key_channelList_pageup )
		{
			selected+=listmaxshow;
			if (selected>Bouquets.size()-1)
				selected=0;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if ( msg == (uint) g_settings.key_channelList_pagedown )
		{
			if ((int(selected)-int(listmaxshow))<0)
				selected=Bouquets.size()-1;
			else
				selected -= listmaxshow;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if ( msg == CRCInput::RC_up )
		{
			int prevselected=selected;
			if(selected==0)
			{
				selected = Bouquets.size()-1;
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
		else if ( msg == CRCInput::RC_down )
		{
			int prevselected=selected;
			selected = (selected+1)%Bouquets.size();
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
		else if ( msg == CRCInput::RC_ok )
		{
			zapOnExit = true;
			loop=false;
		}
		else if ( ( msg >= 0 ) && ( msg <= 9 ) )
		{
			//numeric
			if ( pos==maxpos )
			{
				if ( msg == 0)
				{
					chn = firstselected;
					pos = maxpos- 1;
				}
				else
				{
					chn = msg;
					pos = 0;
				}
			}
			else
			{
				chn = chn* 10 + msg;
			}

			if (chn> Bouquets.size())
			{
				chn = firstselected;
				pos = maxpos- 1;
			}

			pos++;

            int prevselected=selected;
			selected = (chn- 1)%Bouquets.size();
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
		else if( ( msg == CRCInput::RC_red ) ||
				 ( msg == CRCInput::RC_green ) ||
				 ( msg == CRCInput::RC_yellow ) ||
				 ( msg == CRCInput::RC_blue ) )
		{
			selected = oldselected;
			g_RCInput->postMsg( msg, data );
			loop=false;
		}
		else
		{
			if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
			{
				loop = false;
				res = -2;
			}
		};
	}
	hide();
	if(zapOnExit)
	{
		return (selected);
	}
	else
	{
		return (res);
	}
}

void CBouquetList::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}


void CBouquetList::paintItem(int pos)
{
	int ypos = y+ theight+0 + pos*fheight;
	int color = COL_MENUCONTENT;
	if (liststart+pos==selected)
	{
		color = COL_MENUCONTENTSELECTED;
	}

	frameBuffer->paintBoxRel(x,ypos, width- 15, fheight, color);
	if(liststart+pos<Bouquets.size())
	{
		CBouquet* bouq = Bouquets[liststart+pos];
		//number - zum direkten hinhüpfen
		char tmp[10];
		sprintf((char*) tmp, "%d", liststart+pos+ 1);

		int numpos = x+5+numwidth- g_Fonts->channellist_number->getRenderWidth(tmp);
		g_Fonts->channellist_number->RenderString(numpos,ypos+fheight, numwidth+5, tmp, color, fheight);

		g_Fonts->channellist->RenderString(x+ 5+ numwidth+ 10, ypos+ fheight, width- numwidth- 20- 15, bouq->channelList->getName().c_str(), color, 0, true); // UTF-8
	}
}

void CBouquetList::paintHead()
{
	frameBuffer->paintBoxRel(x,y, width,theight+0, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10,y+theight+0, width, "Bouquets" /*g_Locale->getText(name).c_str()*/, COL_MENUHEAD);
}

void CBouquetList::paint()
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

	int sbc= ((Bouquets.size()- 1)/ listmaxshow)+ 1;
	float sbh= (sb- 4)/ sbc;
	int sbs= (selected/listmaxshow);

	frameBuffer->paintBoxRel(x+ width- 13, ypos+ 2+ int(sbs* sbh) , 11, int(sbh),  COL_MENUCONTENT+ 3);
}
