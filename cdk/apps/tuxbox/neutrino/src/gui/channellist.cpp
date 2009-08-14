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
#include <system/lastchannel.h>

#include "widget/menue.h"
#include "widget/messagebox.h"

#include "channellist.h"
#include "color.h"
#include "eventlist.h"
#include "infoviewer.h"


int info_height = 0;


CChannelList::CChannel::CChannel(const int _key, const int _number, const std::string& _name, const t_channel_id ids)
{
	key           = _key;
	number        = _number;
	name          = _name;
	channel_id    = ids;
	bAlwaysLocked = false;
}


CChannelList::CChannelList( const std::string &Name )
{
	frameBuffer = CFrameBuffer::getInstance();
	name = Name;
	selected = 0;
	width = 560;
	height = 420;
/*
	theight= g_Fonts->menu_title->getHeight();
	fheight= g_Fonts->channellist->getHeight();
	listmaxshow = (height-theight-0)/fheight;
	height = theight+0+listmaxshow*fheight; // recalc height
	info_height = g_Fonts->channellist->getHeight() + g_Fonts->channellist_descr->getHeight() + 10;
	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-( height+ info_height) ) / 2) + g_settings.screen_StartY;
*/
	liststart = 0;
	tuned=0xfffffff;
	zapProtection= NULL;;
}

CChannelList::~CChannelList()
{
	for(unsigned int count=0;count<chanlist.size();count++)
	{
		delete chanlist[count];
	}
	chanlist.clear();
}

int CChannelList::exec()
{
	int nNewChannel = show();

	if ( nNewChannel > -1)
	{
		zapTo(nNewChannel);
		return menu_return::RETURN_REPAINT;
	}
	else if ( nNewChannel == -1)
	{
		// -1 bedeutet nur REPAINT
		return menu_return::RETURN_REPAINT;
	}
	else
	{
		// -2 bedeutet EXIT_ALL
		return menu_return::RETURN_EXIT_ALL;
	}
}

void CChannelList::updateEvents(void)
{
	CChannelEventList events = g_Sectionsd->getChannelEvents();

	for (uint count=0; count<chanlist.size(); count++)
		chanlist[count]->currentEvent= CChannelEvent();

	for (uint count=0; count<chanlist.size(); count++)
		for ( CChannelEventList::iterator e= events.begin(); e != events.end(); ++e )
			if (chanlist[count]->channel_id == e->serviceID() )
			{
				chanlist[count]->currentEvent= *e;
				break;
			}
}

void CChannelList::addChannel(int key, int number, const std::string& name, t_channel_id ids)
{
	chanlist.push_back(new CChannel(key, number, name, ids));
}

void CChannelList::addChannel(CChannelList::CChannel* chan)
{
	if (chan != NULL)
		chanlist.push_back(chan);
}

CChannelList::CChannel* CChannelList::getChannel( int number)
{
	for (uint i=0; i< chanlist.size();i++)
	{
		if (chanlist[i]->number == number)
			return chanlist[i];
	}
	return(NULL);
}

int CChannelList::getKey(int id)
{
	return chanlist[id]->key;
}

string CChannelList::getActiveChannelName()
{
	if (selected< chanlist.size())
		return chanlist[selected]->name;
	else
		return "";
}

/*
const std::string CChannelList::getActiveChannelID()
{
	string  id;
	char anid[10];
	snprintf( anid, 10, "%x", getActiveChannel_ChannelID() );
	id= anid;
	return id;
}
*/

t_channel_id CChannelList::getActiveChannel_ChannelID()
{
	if (selected< chanlist.size())
		return chanlist[selected]->channel_id;
	else
		return 0;
}

int CChannelList::getActiveChannelNumber()
{
	return selected+1;
}

int CChannelList::show()
{
	int res = -1;

	if(chanlist.size()==0)
	{
		//evtl. anzeige dass keine kanalliste....
		return res;
	}
	g_lcdd->setMode(CLcddTypes::MODE_MENU_UTF8, g_Locale->getText(name));

	theight= g_Fonts->menu_title->getHeight();
	fheight= g_Fonts->channellist->getHeight();
	listmaxshow = (height-theight-0)/fheight;
	height = theight+0+listmaxshow*fheight; // recalc height
	info_height = g_Fonts->channellist->getHeight() + g_Fonts->channellist_descr->getHeight() + 10;
	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-( height+ info_height) ) / 2) + g_settings.screen_StartY;

	paintHead();
	updateEvents();
	paint();

	int oldselected = selected;
	int zapOnExit = false;
	bool bShowBouquetList = false;

	uint msg; uint data;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_chanlist );

	bool loop=true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_chanlist );

		if ( ( msg == CRCInput::RC_timeout ) ||
			 ( msg == (uint) g_settings.key_channelList_cancel) )
		{
			selected = oldselected;
			loop=false;
		}
		else if ( msg == (uint) g_settings.key_channelList_pageup )
		{
			selected+=listmaxshow;
			if (selected>chanlist.size()-1)
				selected=0;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if ( msg == (uint) g_settings.key_channelList_pagedown )
		{
			if ((int(selected)-int(listmaxshow))<0)
				selected=chanlist.size()-1;
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
				selected = chanlist.size()-1;
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
			selected = (selected+1)%chanlist.size();
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
		else if ( ( msg == (uint) g_settings.key_bouquet_up ) && ( bouquetList != NULL ) )
		{
			if (bouquetList->Bouquets.size() > 0)
			{
				int nNext = (bouquetList->getActiveBouquetNumber()+1) % bouquetList->Bouquets.size();
				bouquetList->activateBouquet( nNext );
				res = bouquetList->showChannelList();
				loop = false;
			}
		}
		else if ( ( msg == (uint) g_settings.key_bouquet_down ) && ( bouquetList != NULL ) )
		{
			if (bouquetList->Bouquets.size() > 0)
			{
				int nNext = (bouquetList->getActiveBouquetNumber()+bouquetList->Bouquets.size()-1) % bouquetList->Bouquets.size();
				bouquetList->activateBouquet(nNext);
				res = bouquetList->showChannelList();
				loop = false;
			}
		}
		else if ( msg == CRCInput::RC_ok )
		{
			zapOnExit = true;
			loop=false;
		}
		else if ( ( msg == CRCInput::RC_setup ) &&
				  ( bouquetList != NULL ) )
		{
			bShowBouquetList = true;
			loop=false;
		}
		else if( (msg==CRCInput::RC_red) ||
				 (msg==CRCInput::RC_green) ||
				 (msg==CRCInput::RC_yellow) ||
				 (msg==CRCInput::RC_blue) ||
		         (CRCInput::isNumeric(msg)) )
		{
			//pushback key if...
			selected = oldselected;
			g_RCInput->postMsg( msg, data );
			loop=false;
		}
		else if ( msg == CRCInput::RC_help )
		{
			hide();

			if ( g_EventList->exec(chanlist[selected]->channel_id, chanlist[selected]->name ) == menu_return::RETURN_EXIT_ALL )
			{
				res = -2;
				loop = false;
			}
			else
			{
				g_RCInput->getMsg( &msg, &data, 0 );

				if ( ( msg != CRCInput::RC_red ) &&
				     ( msg != CRCInput::RC_timeout ) )
				{
					// RC_red schlucken
					g_RCInput->postMsg( msg, data );
				}

				paintHead();
				paint();
			}

		}
		else
		{
			if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) == messages_return::cancel_all )
			{
				loop = false;
				res = - 2;
			}
		}
	}
	hide();

	if (bShowBouquetList)
	{
		if ( bouquetList->exec( true ) == menu_return::RETURN_EXIT_ALL )
			res = -2;
	}

	g_lcdd->setMode(CLcddTypes::MODE_TVRADIO);

	if(zapOnExit)
	{
		return(selected);
	}
	else
	{
		return(res);
	}

}

void CChannelList::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height+ info_height+ 5);
        clearItem2DetailsLine ();
}

bool CChannelList::showInfo(int pos)
{
	if((pos >= (signed int) chanlist.size()) || (pos<0))
	{
		return false;
	}

	CChannel* chan = chanlist[pos];
	g_InfoViewer->showTitle(pos+1, chan->name, chan->channel_id, true );
	return true;
}

int CChannelList::handleMsg(uint msg, uint data)
{
	if ( msg == NeutrinoMessages::EVT_PROGRAMLOCKSTATUS )
	{
		// 0x100 als FSK-Status zeigt an, dass (noch) kein EPG zu einem Kanal der NICHT angezeigt
		// werden sollte (vorgesperrt) da ist

		//printf("program-lock-status: %d\n", data);

		if ( g_settings.parentallock_prompt == PARENTALLOCK_PROMPT_ONSIGNAL )
		{
			if ( zapProtection != NULL )
				zapProtection->fsk = data;
			else
			{
				if ( data>= (uint) g_settings.parentallock_lockage )
				{
					if ( ( chanlist[selected]->last_unlocked_EPGid != g_RemoteControl->current_EPGid ) ||
						 ( g_RemoteControl->current_EPGid == 0 ) )
					{
						g_RemoteControl->stopvideo();
						zapProtection = new CZapProtection( g_settings.parentallock_pincode, data );

						if ( zapProtection->check() )
						{
							g_RemoteControl->startvideo();

							// merken fürs nächste hingehen
							chanlist[selected]->last_unlocked_EPGid= g_RemoteControl->current_EPGid;
						}
						delete zapProtection;
						zapProtection = NULL;
					}
				}
				else
					g_RemoteControl->startvideo();
			}
		}

		return messages_return::handled;
	}
    else
		return messages_return::unhandled;
}


//
// -- Zap to channel with channel_id
//
bool CChannelList::zapTo_ChannelID(const t_channel_id channel_id)
{
	for (unsigned int i=0; i<chanlist.size(); i++) {
		if (chanlist[i]->channel_id == channel_id) {
			zapTo (i);
			return true;
		}
	}

    return false;
}

bool CChannelList::adjustToChannelID(const t_channel_id channel_id)
{
	unsigned int i;

	for (i=0; i<chanlist.size(); i++) {
		if (chanlist[i]->channel_id == channel_id)
		{
			selected= i;
//			CChannel* chan = chanlist[selected];
			lastChList.store (selected);

			tuned = i;
			if (bouquetList != NULL)
				bouquetList->adjustToChannel( getActiveChannelNumber());
			return true;
		}
	}
	return false;
}

void CChannelList::zapTo(int pos)
{
	if (chanlist.size() == 0)
	{
		ShowMsg ( "messagebox.error", g_Locale->getText("channellist.nonefound"), CMessageBox::mbrCancel, CMessageBox::mbCancel, "error.raw" );
		return;
	}
	if ( (pos >= (signed int) chanlist.size()) || (pos< 0) )
	{
		pos = 0;
	}

	selected= pos;
	CChannel* chan = chanlist[selected];
	lastChList.store (selected);

	if ( pos!=(int)tuned )
	{
		tuned = pos;
		g_RemoteControl->zapTo_ChannelID(chan->channel_id, chan->name, !chan->bAlwaysLocked );
	}
	g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );

	if (bouquetList != NULL)
		bouquetList->adjustToChannel( getActiveChannelNumber());
}

int CChannelList::numericZap(int key)
{
	int res = menu_return::RETURN_REPAINT;

	if(chanlist.size()==0)
	{
		ShowMsg ( "messagebox.error", g_Locale->getText("channellist.nonefound"), CMessageBox::mbrCancel, CMessageBox::mbCancel, "error.raw" );
		return res;
	}

	//schneller zap mit "0" taste zwischen den letzten beiden sendern...
	if( key == 0 )
	{
		int  ch;

		if( (ch=lastChList.getlast(1)) != -1)
		{
			if ((unsigned int)ch != tuned)
			{
				//printf("quicknumtune(0)\n");
				lastChList.clear_storedelay (); // ignore store delay
				zapTo(ch);		        // zap to last
			}
		}
		return res;
	}

	int ox=300;
	int oy=200;
	int sx = 4* g_Fonts->channel_num_zap->getRenderWidth(widest_number)+ 14;
	int sy = g_Fonts->channel_num_zap->getHeight()+6;
	char valstr[10];
	int chn=key;
	int lastchan= -1;
	int pos=1;
	uint msg; uint data;
	bool doZap = true;


	while(1)
	{
		if (lastchan != chn)
		{
			sprintf((char*) &valstr, "%d", chn);
			while(strlen(valstr)<4)
				strcat(valstr,"·");   //"_"

			frameBuffer->paintBoxRel(ox, oy, sx, sy, COL_INFOBAR);

			for (int i=3; i>=0; i--)
			{
				valstr[i+ 1]= 0;
				g_Fonts->channel_num_zap->RenderString(ox+7+ i*((sx-14)>>2), oy+sy-3, sx, &valstr[i], COL_INFOBAR);
			}

			showInfo(chn- 1);
			lastchan= chn;
		}


		g_RCInput->getMsg( &msg, &data, 30 );

		if ( msg == CRCInput::RC_timeout )
		{
			if ( ( chn > (int)chanlist.size() ) || (chn == 0) )
				chn = tuned + 1;
			break;
		}
		else if ( ( msg >= 0 ) && ( msg <= 9 ) )
		{ //numeric
			if ( pos==4 )
			{
				chn = msg;
				pos = 0;
			}
			else
				chn = chn* 10 + msg;

			pos++;
		}
		else if ( msg == CRCInput::RC_ok )
		{
			if ( ( chn > (signed int) chanlist.size() ) || ( chn == 0 ) )
			{
				chn = tuned + 1;
			}
			break;
		}
		else if ( msg == (uint) g_settings.key_quickzap_down )
		{
			if ( chn == 1 )
				chn = chanlist.size();
			else
			{
				chn--;

				if (chn > (int)chanlist.size())
					chn = (int)chanlist.size();
			}
		}
		else if ( msg == (uint) g_settings.key_quickzap_up )
		{
			chn++;

			if (chn > (int)chanlist.size())
				chn = 1;
		}
		else if ( ( msg == CRCInput::RC_home ) ||
				  ( msg == CRCInput::RC_left ) ||
				  ( msg == CRCInput::RC_right) )
		{
			// Abbruch ohne Channel zu wechseln
			doZap = false;
			break;
		}
		else if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
		{
			doZap = false;
			res = menu_return::RETURN_EXIT_ALL;
			break;
		}
	}

	frameBuffer->paintBackgroundBoxRel(ox, oy, sx, sy);

	if ( doZap )
	{
		chn--;
		if (chn<0)
			chn=0;
		zapTo( chn );
	}
	else
	{
		g_InfoViewer->killTitle();
	}

	return res;
}

void CChannelList::quickZap(int key)
{
        if(chanlist.size()==0)
        {
                //evtl. anzeige dass keine kanalliste....
                return;
        }

        if (key==g_settings.key_quickzap_down)
        {
                if(selected==0)
                        selected = chanlist.size()-1;
                else
                        selected--;
                //                              CChannel* chan = chanlist[selected];
        }
        else if (key==g_settings.key_quickzap_up)
        {
                selected = (selected+1)%chanlist.size();
                //                      CChannel* chan = chanlist[selected];
        }

        zapTo( selected );
}

int CChannelList::hasChannel(int nChannelNr)
{
	for (uint i=0;i<chanlist.size();i++)
	{
		if (getKey(i) == nChannelNr)
			return(i);
	}
	return(-1);
}

// for adjusting bouquet's channel list after numzap or quickzap
void CChannelList::setSelected( int nChannelNr)
{
	selected = nChannelNr;
}

void CChannelList::paintDetails(int index)
{
	if ( chanlist[index]->currentEvent.description== "" )
	{
		frameBuffer->paintBackgroundBoxRel(x, y+ height, width, info_height);
	}
	else
	{
		// löschen
		frameBuffer->paintBoxRel(x, y+ height, width, info_height, COL_MENUCONTENTDARK);

		char cNoch[50];
		char cSeit[50];

        struct		tm *pStartZeit = localtime(&chanlist[index]->currentEvent.startTime);
        unsigned 	seit = ( time(NULL) - chanlist[index]->currentEvent.startTime ) / 60;
        sprintf( cSeit, g_Locale->getText("channellist.since").c_str(), pStartZeit->tm_hour, pStartZeit->tm_min); //, seit );
        int seit_len= g_Fonts->channellist_descr->getRenderWidth(cSeit);

        int noch = ( chanlist[index]->currentEvent.startTime + chanlist[index]->currentEvent.duration - time(NULL)   ) / 60;
        if ( (noch< 0) || (noch>=10000) )
        	noch= 0;
        sprintf( cNoch, "(%d / %d min)", seit, noch );
        int noch_len= g_Fonts->channellist_number->getRenderWidth(cNoch);

		string text1= chanlist[index]->currentEvent.description;
		string text2= chanlist[index]->currentEvent.text;

		int xstart = 10;
		if ( g_Fonts->channellist->getRenderWidth(text1.c_str())> (width - 30 - seit_len) )
		{
			// zu breit, Umbruch versuchen...
		    int pos;
		    do
		    {
				pos = text1.find_last_of("[ -.]+");
				if ( pos!=-1 )
					text1 = text1.substr( 0, pos );
			} while ( ( pos != -1 ) && ( g_Fonts->channellist->getRenderWidth(text1.c_str())> (width - 30 - seit_len) ) );

			string text3= chanlist[index]->currentEvent.description.substr(text1.length()+ 1).c_str();
			if ( text2 != "" )
				text3= text3+ " · ";

			xstart+= g_Fonts->channellist->getRenderWidth(text3.c_str());
			g_Fonts->channellist->RenderString(x+ 10, y+ height+ 5+ 2* fheight, width - 30- noch_len, text3.c_str(), COL_MENUCONTENTDARK);
		}

		if ( text2 != "" )
		{
			while ( text2.find_first_of("[ -.+*#?=!$%&/]+") == 0 )
				text2 = text2.substr( 1 );
			text2 = text2.substr( 0, text2.find("\n") );
			g_Fonts->channellist_descr->RenderString(x+ xstart, y+ height+ 5+ 2* fheight, width- xstart- 20- noch_len, text2.c_str(), COL_MENUCONTENTDARK);
		}

		g_Fonts->channellist->RenderString(x+ 10, y+ height+ 5+ fheight, width - 30 - seit_len, text1.c_str(), COL_MENUCONTENTDARK);
		g_Fonts->channellist_descr->RenderString(x+ width- 10- seit_len, y+ height+ 5+ fheight, seit_len, cSeit, COL_MENUCONTENTDARK);

		g_Fonts->channellist_number->RenderString(x+ width- 10- noch_len, y+ height+ 5+ 2* fheight- 2, noch_len, cNoch, COL_MENUCONTENTDARK);
	}
}


//
// -- Decoreline to connect ChannelDisplayLine with ChannelDetail display
// -- 2002-03-17 rasc
//

void CChannelList::clearItem2DetailsLine ()

{
 paintItem2DetailsLine (-1, 0);
}

void CChannelList::paintItem2DetailsLine (int pos, int ch_index)
{
	#define ConnectLineBox_Width	16

	int xpos  = x - ConnectLineBox_Width;
	int ypos1 = y + theight+0 + pos*fheight;
	int ypos2 = y + height;
	int ypos1a = ypos1 + (fheight/2)-2;
	int ypos2a = ypos2 + (info_height/2)-2;
	unsigned char col1 = COL_MENUCONTENT+6;
	unsigned char col2 = COL_MENUCONTENT+1;


	// Clear
	frameBuffer->paintBackgroundBoxRel(xpos,y, ConnectLineBox_Width, height+info_height);

	// paint Line if detail info (and not valid list pos)
	if (pos >= 0 &&  chanlist[ch_index]->currentEvent.description != "")
	{
		// 1. col thick line
		frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-4, ypos1, 4,fheight,     col1);
		frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-4, ypos2, 4,info_height, col1);

		frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-16, ypos1a, 4,ypos2a-ypos1a, col1);

		frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-16, ypos1a, 12,4, col1);
		frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-16, ypos2a, 12,4, col1);

		// 2. col small line
		frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-4, ypos1, 1,fheight,     col2);
		frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-4, ypos2, 1,info_height, col2);

		frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-16, ypos1a, 1,ypos2a-ypos1a+4, col2);

		frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-16, ypos1a, 12,1, col2);
		frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-12, ypos2a, 8,1, col2);

		// -- small Frame around infobox
                frameBuffer->paintBoxRel(x,         ypos2, 2,info_height, col1);
                frameBuffer->paintBoxRel(x+width-2, ypos2, 2,info_height, col1);
                frameBuffer->paintBoxRel(x        , ypos2, width-2,2,     col1);
                frameBuffer->paintBoxRel(x        , ypos2+info_height-2, width-2,2, col1);

	}

}




void CChannelList::paintItem(int pos)
{
	int ypos = y+ theight+0 + pos*fheight;
	int color = COL_MENUCONTENT;
	if (liststart+pos==selected)
	{
		color = COL_MENUCONTENTSELECTED;
		paintDetails(liststart+pos);
		paintItem2DetailsLine (pos, liststart+pos);
	}

	frameBuffer->paintBoxRel(x,ypos, width- 15, fheight, color);
	if(liststart+pos<chanlist.size())
	{
		CChannel* chan = chanlist[liststart+pos];
		//number
		char tmp[10];
		sprintf((char*) tmp, "%d", chan->number);

		if (liststart+pos==selected)
		{
			g_lcdd->setMenuText(0, chan->name );
			g_lcdd->setMenuText(1, chan->currentEvent.description );
		}

		int numpos = x+5+numwidth- g_Fonts->channellist_number->getRenderWidth(tmp);
		g_Fonts->channellist_number->RenderString(numpos,ypos+fheight, numwidth+5, tmp, color, fheight);
		if(strlen(chan->currentEvent.description.c_str()))
		{
			char nameAndDescription[100];
			snprintf(nameAndDescription, sizeof(nameAndDescription), "%s · ", chan->name.c_str());

			unsigned int ch_name_len= g_Fonts->channellist->getRenderWidth(nameAndDescription);
			unsigned int ch_desc_len= g_Fonts->channellist_descr->getRenderWidth(chan->currentEvent.description.c_str());

			if ( (width- numwidth- 20- 15- ch_name_len)< ch_desc_len )
				ch_desc_len = (width- numwidth- 20- 15- ch_name_len);
			if (ch_desc_len< 0)
				ch_desc_len = 0;

			g_Fonts->channellist->RenderString(x+ 5+ numwidth+ 10, ypos+ fheight, width- numwidth- 20- 15, nameAndDescription, color);


			// rechtsbündig - auskommentiert
			// g_Fonts->channellist_descr->RenderString(x+ width- 15- ch_desc_len, ypos+ fheight, ch_desc_len, chan->currentEvent.description.c_str(), color);

			// linksbündig
			g_Fonts->channellist_descr->RenderString(x+ 5+ numwidth+ 10+ ch_name_len+ 5, ypos+ fheight, ch_desc_len, chan->currentEvent.description.c_str(), color);
		}
		else
			//name
			g_Fonts->channellist->RenderString(x+ 5+ numwidth+ 10, ypos+ fheight, width- numwidth- 20- 15, chan->name.c_str(), color);
	}
}

void CChannelList::paintHead()
{
	string strCaption = g_Locale->getText(name);

/*	if (strCaption == "")
	{
		strCaption = name;
	}
*/
	frameBuffer->paintBoxRel(x,y, width,theight+0, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10,y+theight+0, width- 65, strCaption.c_str(), COL_MENUHEAD, 0, true); // UTF-8

	frameBuffer->paintIcon("help.raw", x+ width- 30, y+ 5 );
	if (bouquetList!=NULL)
		frameBuffer->paintIcon("dbox.raw", x+ width- 60, y+ 5 );
}

void CChannelList::paint()
{
	g_Sectionsd->setPauseSorting( true );

	liststart = (selected/listmaxshow)*listmaxshow;
	int lastnum =  chanlist[liststart]->number + listmaxshow;

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

	int sbc= ((chanlist.size()- 1)/ listmaxshow)+ 1;
	float sbh= (sb- 4)/ sbc;
	int sbs= (selected/listmaxshow);

	frameBuffer->paintBoxRel(x+ width- 13, ypos+ 2+ int(sbs* sbh) , 11, int(sbh),  COL_MENUCONTENT+ 3);

	g_Sectionsd->setPauseSorting( false );
}

CChannelList::CChannel* CChannelList::getChannelFromChannelID(const t_channel_id channel_id)
{
	for (uint i=0; i< chanlist.size();i++)
	{
		if (chanlist[i]->channel_id == channel_id)
			return chanlist[i];
	}
	return(NULL);
}
