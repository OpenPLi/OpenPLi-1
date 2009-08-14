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

#include "epgview.h"

#include "widget/hintbox.h"
#include "widget/messagebox.h"


CEpgData::CEpgData()
{
	frameBuffer = CFrameBuffer::getInstance();
}

void CEpgData::start()
{
	ox = 540;
    sx = (((g_settings.screen_EndX-g_settings.screen_StartX) -ox) / 2) + g_settings.screen_StartX;
    oy = 320;
	topheight= g_Fonts->epg_title->getHeight();
	topboxheight=topheight+6;
	botheight=g_Fonts->epg_date->getHeight();
	botboxheight=botheight+6;
	medlineheight=g_Fonts->epg_info1->getHeight();
	medlinecount=(oy- botboxheight)/medlineheight;

	oy = botboxheight+medlinecount*medlineheight; // recalculate
	sy = (((g_settings.screen_EndY-g_settings.screen_StartY)-(oy- topboxheight) ) / 2) + g_settings.screen_StartY;
	toph = topboxheight;

}

void CEpgData::addTextToArray( string text  )
{
	//printf("line: >%s<\n", text.c_str() );
	if (text==" ")
	{
		emptyLineCount ++;
		if(emptyLineCount<2)
		{
			epgText.insert(epgText.end(), text );
		}
	}
	else
	{
		emptyLineCount = 0;
		epgText.insert(epgText.end(), text );
	}
}

void CEpgData::processTextToArray( string text )
{
	string	aktLine = "";
	string	aktWord = "";
	int	aktWidth = 0;
	text+= " ";
	char* text_= (char*) text.c_str();

	while(*text_!=0)
	{
		if ( (*text_==' ') || (*text_=='\n') || (*text_=='-') || (*text_=='.') )
		{
			//check the wordwidth - add to this line if size ok
			if(*text_=='\n')
			{	//enter-handler
				//printf("enter-");
				addTextToArray( aktLine );
				aktLine = "";
				aktWidth= 0;
			}
			else
			{
				aktWord += *text_;

				int aktWordWidth = g_Fonts->epg_info2->getRenderWidth(aktWord.c_str());
				if((aktWordWidth+aktWidth)<(ox- 20- 15))
				{//space ok, add
					aktWidth += aktWordWidth;
					aktLine += aktWord;
				}
				else
				{//new line needed
					addTextToArray( aktLine );
					aktLine = aktWord;
					aktWidth = aktWordWidth;
				}
				aktWord = "";
			}
		}
		else
		{
			aktWord += *text_;
		}
		text_++;
	}
	//add the rest
	addTextToArray( aktLine + aktWord );
}

void CEpgData::showText( int startPos, int ypos )
{
	int textCount = epgText.size();
	int y=ypos;
	int linecount=medlinecount;
	string t;
	int sb = linecount* medlineheight;
	frameBuffer->paintBoxRel(sx, y, ox- 15, sb, COL_MENUCONTENT);

	for(int i=startPos; i<textCount && i<startPos+linecount; i++,y+=medlineheight)
	{
		t=epgText[i];
		if ( i< info1_lines )
			g_Fonts->epg_info1->RenderString(sx+10, y+medlineheight, ox- 15- 15, t.c_str(), COL_MENUCONTENT);
		else
			g_Fonts->epg_info2->RenderString(sx+10, y+medlineheight, ox- 15- 15, t.c_str(), COL_MENUCONTENT);
	}

	frameBuffer->paintBoxRel(sx+ ox- 15, ypos, 15, sb,  COL_MENUCONTENT+ 1);

	int sbc= ((textCount- 1)/ linecount)+ 1;
	float sbh= (sb- 4)/ sbc;
	int sbs= (startPos+ 1)/ linecount;

	frameBuffer->paintBoxRel(sx+ ox- 13, ypos+ 2+ int(sbs* sbh) , 11, int(sbh),  COL_MENUCONTENT+ 3);

}

string GetGenre( char contentClassification )
{
	string res= "UNKNOWN";
	char subClass[2];
	sprintf( subClass, "%d", (contentClassification&0x0F) );

	switch (contentClassification&0x0F0)
	{
		case 0x010: {
						res="MOVIE.";
						if ( (contentClassification&0x0F)< 9 )
							res+= subClass;
						else
							res+= "0";
					 	break;
					}
		case 0x020: {
						res="NEWS.";
						if ( (contentClassification&0x0F)< 5 )
							res+= subClass;
						else
							res+= "0";
					 	break;
					}
		case 0x030: {
						res="SHOW.";
						if ( (contentClassification&0x0F)< 4 )
							res+= subClass;
						else
							res+= "0";
					 	break;
					}
		case 0x040: {
						res="SPORTS.";
						if ( (contentClassification&0x0F)< 12 )
							res+= subClass;
						else
							res+= "0";
					 	break;
					}
		case 0x050: {
						res="CHILDRENs_PROGRAMMES.";
						if ( (contentClassification&0x0F)< 6 )
							res+= subClass;
						else
							res+= "0";
					 	break;
					}
		case 0x060: {
						res="MUSIC_DANCE.";
						if ( (contentClassification&0x0F)< 7 )
							res+= subClass;
						else
							res+= "0";
					 	break;
					}
		case 0x070: {
						res="ARTS.";
						if ( (contentClassification&0x0F)< 12 )
							res+= subClass;
						else
							res+= "0";
					 	break;
					}
		case 0x080: {
						res="SOZIAL_POLITICAL.";
						if ( (contentClassification&0x0F)< 4 )
							res+= subClass;
						else
							res+= "0";
					 	break;
					}
		case 0x090: {
						res="DOCUS_MAGAZINES.";
						if ( (contentClassification&0x0F)< 8 )
							res+= subClass;
						else
							res+= "0";
					 	break;
					}
		case 0x0A0: {
						res="TRAVEL_HOBBIES.";
						if ( (contentClassification&0x0F)< 8 )
							res+= subClass;
						else
							res+= "0";
					 	break;
					}
	}
	return g_Locale->getText("GENRE."+res);
}


int CEpgData::show(const t_channel_id channel_id, unsigned long long id, time_t* startzeit, bool doLoop )
{
	int res = menu_return::RETURN_REPAINT;

	int height;
	height = g_Fonts->epg_date->getHeight();
	if (doLoop)
	{
		frameBuffer->paintBoxRel(g_settings.screen_StartX, g_settings.screen_StartY, 50, height+5, COL_INFOBAR);
		g_Fonts->epg_date->RenderString(g_settings.screen_StartX+10, g_settings.screen_StartY+height, 40, "-@-", COL_INFOBAR);
	}

	GetEPGData(channel_id, id, startzeit );
	if (doLoop)
	{
		evtlist = g_Sectionsd->getEventsServiceKey(channel_id);
		frameBuffer->paintBackgroundBoxRel(g_settings.screen_StartX, g_settings.screen_StartY, 50, height+5);
	}

	if(epgData.title.length()==0)
	{
/*
		//no epg-data found :(
		char *text = (char*) g_Locale->getText("epgviewer.notfound").c_str();
		int oy = 30;
		int ox = g_Fonts->epg_info2->getRenderWidth(text)+30;
		int sx = (((g_settings.screen_EndX- g_settings.screen_StartX)-ox) / 2) + g_settings.screen_StartX;
		int sy = (((g_settings.screen_EndY- g_settings.screen_StartY)-oy) / 2) + g_settings.screen_StartY;
		height = g_Fonts->epg_info2->getHeight();
		frameBuffer->paintBoxRel(sx, sy, ox, height+ 10, COL_INFOBAR_SHADOW); //border
		frameBuffer->paintBoxRel(sx+ 1, sy+ 1, ox- 2, height+ 8, COL_MENUCONTENT);
		g_Fonts->epg_info2->RenderString(sx+15, sy+height+5, ox-30, text, COL_MENUCONTENT);


		uint msg; uint data;
		g_RCInput->getMsg( &msg, &data, 20 );
		CNeutrinoApp::getInstance()->handleMsg( msg, data );

		frameBuffer->paintBackgroundBoxRel(sx, sy, ox, height+10);
*/
		ShowHint ( "messagebox.info", g_Locale->getText("epgviewer.notfound"), "info.raw" );

		return res;
	}



	int pos;
	string text1 = epgData.title;
	string text2 = "";
	if ( g_Fonts->epg_title->getRenderWidth(text1.c_str())> 520 )
    {
    	do
    	{
			pos = text1.find_last_of("[ .]+");
			if ( pos!=-1 )
				text1 = text1.substr( 0, pos );
		} while ( ( pos != -1 ) && ( g_Fonts->epg_title->getRenderWidth(text1.c_str())> 520 ) );
        text2 = epgData.title.substr(text1.length()+ 1, uint(-1) );
	}

	int oldtoph= toph;

	if (text2!="")
		toph = 2* topboxheight;
	else
		toph = topboxheight;


	if ( (oldtoph> toph) && (!doLoop) )
	{
		frameBuffer->paintBackgroundBox (sx, sy- oldtoph- 1, sx+ ox, sy- toph);
	}

	if(epgData.info1.length()!=0)
	{
		processTextToArray( epgData.info1.c_str() );
	}
	info1_lines = epgText.size();

	//scan epg-data - sort to list
	if ( ( epgData.info2.length()==0 ) && (info1_lines == 0) )
	{
		epgData.info2= g_Locale->getText("epgviewer.nodetailed");
	}

	processTextToArray( epgData.info2.c_str() );

	if (epgData.fsk > 0)
	{
		char _tfsk[11];
		sprintf (_tfsk, "FSK: ab %d", epgData.fsk );
		processTextToArray( _tfsk );
	}

	if (epgData.contentClassification.length()> 0)
		processTextToArray( GetGenre(epgData.contentClassification[0]) );
//	processTextToArray( epgData.userClassification.c_str() );


	// -- display more screenings on the same channel
	// -- 2002-05-03 rasc
	processTextToArray("\n") ;
	processTextToArray(g_Locale->getText("epgviewer.More_Screenings")+":");
	FollowScreenings(channel_id, epgData.title);


	//show the epg
	frameBuffer->paintBoxRel(sx, sy- toph, ox, toph, COL_MENUHEAD);
	g_Fonts->epg_title->RenderString(sx+10, sy- toph+ topheight+ 3, ox-15, text1.c_str(), COL_MENUHEAD);
	if (text2!="")
		g_Fonts->epg_title->RenderString(sx+10, sy- toph+ 2* topheight+ 3, ox-15, text2.c_str(), COL_MENUHEAD);

	//show date-time....
	frameBuffer->paintBoxRel(sx, sy+oy-botboxheight, ox, botboxheight, COL_MENUHEAD);
	string fromto;
	int widthl,widthr;
	fromto= epg_start+ " - "+ epg_end;

	widthl = g_Fonts->epg_date->getRenderWidth(fromto.c_str());
	g_Fonts->epg_date->RenderString(sx+40,  sy+oy-3, widthl, fromto.c_str(), COL_MENUHEAD);
	widthr = g_Fonts->epg_date->getRenderWidth(epg_date.c_str());
	g_Fonts->epg_date->RenderString(sx+ox-40-widthr,  sy+oy-3, widthr, epg_date.c_str(), COL_MENUHEAD);

	int showPos = 0;
	textCount = epgText.size();
	int textypos = sy;
	showText(showPos, textypos);

	// show Timer Event Buttons
	showTimerEventBar (true);

	//show progressbar
	if ( epg_done!= -1 )
	{
		int pbx = sx + 10 + widthl + 10 + ((ox-104-widthr-widthl-10-10-20)>>1);
		frameBuffer->paintBoxRel(pbx, sy+oy-height, 104, height-6, COL_MENUCONTENT+6);
		frameBuffer->paintBoxRel(pbx+2, sy+oy-height+2, 100, height-10, COL_MENUCONTENT);
		frameBuffer->paintBoxRel(pbx+2, sy+oy-height+2, epg_done, height-10, COL_MENUCONTENT+3);
	}

	GetPrevNextEPGData( epgData.eventID, &epgData.epg_times.startzeit );
	if (prev_id != 0)
	{
		frameBuffer->paintBoxRel(sx+ 5, sy+ oy- botboxheight+ 4, botboxheight- 8, botboxheight- 8,  COL_MENUCONTENT+ 3);
		g_Fonts->epg_date->RenderString(sx+ 10, sy+ oy- 3, widthr, "<", COL_MENUCONTENT+ 3);
	}

	if (next_id != 0)
	{
		frameBuffer->paintBoxRel(sx+ ox- botboxheight+ 8- 5, sy+ oy- botboxheight+ 4, botboxheight- 8, botboxheight- 8,  COL_MENUCONTENT+ 3);
		g_Fonts->epg_date->RenderString(sx+ ox- botboxheight+ 8, sy+ oy- 3, widthr, ">", COL_MENUCONTENT+ 3);
	}

	if ( doLoop )
	{
		bool loop=true;
		int scrollCount;

		uint msg; uint data;
		unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_epg );
		CTimerdClient * timerdclient;
		while(loop)
		{
			g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

			scrollCount = medlinecount;

			switch ( msg )
			{
				case CRCInput::RC_left:
// $$$ BUG scrollpos passt u.U. nicht zu screen
					if (prev_id != 0)
					{
						frameBuffer->paintBoxRel(sx+ 5, sy+ oy- botboxheight+ 4, botboxheight- 8, botboxheight- 8,  COL_MENUCONTENT+ 1);
						g_Fonts->epg_date->RenderString(sx+ 10, sy+ oy- 3, widthr, "<", COL_MENUCONTENT+ 1);

						show(channel_id, prev_id, &prev_zeit, false);
					}
					break;

				case CRCInput::RC_right:
// $$$ BUG scrollpos passt u.U. nicht zu screen
					if (next_id != 0)
					{
						frameBuffer->paintBoxRel(sx+ ox- botboxheight+ 8- 5, sy+ oy- botboxheight+ 4, botboxheight- 8, botboxheight- 8,  COL_MENUCONTENT+ 1);
						g_Fonts->epg_date->RenderString(sx+ ox- botboxheight+ 8, sy+ oy- 3, widthr, ">", COL_MENUCONTENT+ 1);

						show(channel_id, next_id, &next_zeit, false);
					}
					break;

				case CRCInput::RC_down:
					if(showPos+scrollCount<textCount)
					{
						showPos += scrollCount;
						showText(showPos,textypos);
					}
					break;

				case CRCInput::RC_up:
					showPos -= scrollCount;
					if(showPos<0)
						showPos = 0;
					else
						showText(showPos,textypos);
					break;

				// 31.05.2002 dirch		record timer
				case CRCInput::RC_red:
					if(g_settings.network_streaming_use)
					{
						timerdclient = new CTimerdClient;
						if(timerdclient->isTimerdAvailable())
						{
							timerdclient->addRecordTimerEvent(channel_id, epgData.eventID, 
																		 epgData.epg_times.startzeit - (atoi(g_settings.record_safety_time)*60),
																		 epgData.epg_times.startzeit - (ANNOUNCETIME + (atoi(g_settings.record_safety_time)*60)),
																		 epgData.epg_times.startzeit + epgData.epg_times.dauer );
							ShowMsg ( "timer.eventrecord.title", g_Locale->getText("timer.eventrecord.msg"), CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw");
						}
						else
							printf("timerd not available\n");
						delete timerdclient;
					}					
					break;

				// 31.05.2002 dirch		zapto timer
				case CRCInput::RC_yellow:
					// $$ EPG ID muss noch mit rein...
					timerdclient = new CTimerdClient;
					if(timerdclient->isTimerdAvailable())
					{
						timerdclient->addZaptoTimerEvent(channel_id, epgData.eventID, epgData.epg_times.startzeit, epgData.epg_times.startzeit - ANNOUNCETIME, 0);
						printf("Added ZaptoTimerEvent for channel_id: %08x epgID: %llu\n", channel_id, epgData.eventID);
						ShowMsg ( "timer.eventtimed.title", g_Locale->getText("timer.eventtimed.msg"), CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw");
					}
					else
						printf("timerd not available\n");
					delete timerdclient;
					break;


				case CRCInput::RC_ok:
				case CRCInput::RC_help:
				case CRCInput::RC_timeout:
					loop = false;
					break;

				default:
					// konfigurierbare Keys handlen...
					if ( msg == (uint) g_settings.key_channelList_cancel )
						loop = false;
					else
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
	}
	return res;
}

void CEpgData::hide()
{
	frameBuffer->paintBackgroundBox (sx, sy- toph, sx+ ox, sy+ oy);
        showTimerEventBar (false);
	#ifdef USEACTIONLOG
		g_ActionLog->println("epg: closed");
	#endif
}

void CEpgData::GetEPGData(const t_channel_id channel_id, unsigned long long id, time_t* startzeit )
{
	epgText.clear();
	emptyLineCount = 0;

	bool res;

	if ( id!= 0 )
		res = g_Sectionsd->getEPGid( id, *startzeit, &epgData );
	else
		res = g_Sectionsd->getActualEPGServiceKey(channel_id, &epgData );

	if ( res )
	{
		struct tm *pStartZeit = localtime(&(epgData.epg_times).startzeit);
		char temp[11];
		strftime( temp, sizeof(temp), "%d.%m.%Y", pStartZeit);
		epg_date= temp;
		strftime( temp, sizeof(temp), "%H:%M", pStartZeit);
		epg_start= temp;

		long int uiEndTime((epgData.epg_times).startzeit+ (epgData.epg_times).dauer);
		struct tm *pEndeZeit = localtime((time_t*)&uiEndTime);
		strftime( temp, sizeof(temp), "%H:%M", pEndeZeit);
		epg_end= temp;

		epg_done= -1;
		if (( time(NULL)- (epgData.epg_times).startzeit )>= 0 )
		{
			unsigned nProcentagePassed=(unsigned)((float)(time(NULL)-(epgData.epg_times).startzeit)/(float)(epgData.epg_times).dauer*100.);
			if (nProcentagePassed<= 100)
				epg_done= nProcentagePassed;
		}
	}

	#ifdef USEACTIONLOG
		char buf[1000];
		sprintf((char*) buf, "epg: %08x %s %s - %s, \"%s\"", channel_id, epgData.date.c_str(), epgData.start.c_str(), epgData.end.c_str(), epgData.title.c_str() );
		g_ActionLog->println(buf);
	#endif
}

void CEpgData::GetPrevNextEPGData( unsigned long long id, time_t* startzeit )
{
	prev_id= 0;
	next_id= 0;

	for ( unsigned int i= 0; i< evtlist.size(); i++ )
	{
		//printf("%d %llx/%llx - %x %x\n", i, evtlist[i].eventID, id, evtlist[i].startTime, *startzeit);
    	if ( ( evtlist[i].eventID == id ) && ( evtlist[i].startTime == *startzeit ) )
    	{
        	if ( i > 0 )
        	{
        		prev_id= evtlist[i- 1].eventID;
        		prev_zeit= evtlist[i- 1].startTime;
        	}
 			if ( i < ( evtlist.size()- 1 ) )
        	{
        		next_id= evtlist[i+ 1].eventID;
        		next_zeit= evtlist[i+ 1].startTime;
        	}
			break;
    	}
	}

}


//
// -- get following screenings of this program title
// -- yek! a better class design would be more helpfull
// -- BAD THING: Cross channel screenings will not be shown
// --            $$$TODO
// -- 2002-05-03 rasc
//

int CEpgData::FollowScreenings (const t_channel_id channel_id, string title)

{
  CChannelEventList::iterator e;
  time_t		curtime;
  struct  tm		*tmStartZeit;
  string		datetime_str;
  string		screening_dates;
  int			count;
  char			tmpstr[256];


  	count = 0;
	screening_dates = "";
	// alredy read: evtlist = g_Sectionsd->getEventsServiceKey( channel_id );
    	curtime = time(NULL);

	for ( e= evtlist.begin(); e != evtlist.end(); ++e )
	{
	    	if (e->startTime <= curtime) continue;
		if (! e->eventID) continue;
		if (e->description == title) {
			count++;
			tmStartZeit = localtime(&(e->startTime));

			strftime(tmpstr, sizeof(tmpstr), "date.%a", tmStartZeit );
			datetime_str = std::string( g_Locale->getText(tmpstr) );
			datetime_str += std::string(".");

			strftime(tmpstr, sizeof(tmpstr), "  %d.", tmStartZeit );
			datetime_str += std::string( tmpstr );

			strftime(tmpstr,sizeof(tmpstr), "date.%b", tmStartZeit );
			datetime_str += std::string( g_Locale->getText(tmpstr) );

			strftime(tmpstr, sizeof(tmpstr), ".  %H:%M ", tmStartZeit );
			datetime_str += std::string( tmpstr );

			// this is quick&dirty
			screening_dates += "    ";
			screening_dates += datetime_str;
			screening_dates += "\n";
		}
	}

	if (count) processTextToArray( screening_dates );
	else       processTextToArray( "---\n" );

	return count;
}


//
// -- Just display or hide TimerEventbar
// -- 2002-05-13 rasc
//

void CEpgData::showTimerEventBar (bool show)

{
  int  x,y,w,h;
  int  cellwidth;		// 4 cells
  int  h_offset, pos;

  w = ox;
  h = 30;
  x = sx;
  y = sy + oy;
  h_offset = 5;
  cellwidth = w / 4;


    frameBuffer->paintBackgroundBoxRel(x,y,w,h);
    // hide only?
    if (! show) return;

    // frameBuffer->paintBoxRel(x,y,w,h, COL_INFOBAR_SHADOW+1);
    frameBuffer->paintBoxRel(x,y,w,h, COL_MENUHEAD);



    // Button: Timer Record & Channelswitch 
	if(g_settings.network_streaming_use)		// display record button only if streamingserver_use
	{
		pos = 0;
		frameBuffer->paintIcon("rot.raw", x+8+cellwidth*pos, y+h_offset );
		g_Fonts->infobar_small->RenderString(x+29+cellwidth*pos, y+h-h_offset, w-30, g_Locale->getText("timerbar.recordevent").c_str(), COL_INFOBAR);
	}
    // Button: Timer Channelswitch
    pos = 1;
    frameBuffer->paintIcon("gelb.raw", x+8+cellwidth*pos, y+h_offset );
    g_Fonts->infobar_small->RenderString(x+29+cellwidth*pos, y+h-h_offset, w-30, g_Locale->getText("timerbar.channelswitch").c_str(), COL_INFOBAR);
}
