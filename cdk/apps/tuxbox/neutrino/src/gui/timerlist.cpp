/*
	Neutrino-GUI  -   DBoxII-Project

	Timerliste by Zwen
	
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

#include "eventlist.h"
#include "timerlist.h"
#include "color.h"
#include "infoviewer.h"

#include "widget/menue.h"
#include "widget/messagebox.h"
#include "widget/hintbox.h"
#include "widget/stringinput.h"

#define info_height 60


class CTimerListNewNotifier : public CChangeObserver
{
private:
	CMenuForwarder* m1;
	CMenuOptionChooser* m2;
	CMenuOptionChooser* m3;
	CMenuForwarder* m4;
	char* display;
	int* iType;
	time_t* stopTime;
public:
	CTimerListNewNotifier( int* Type, time_t* time,CMenuForwarder* a1, CMenuOptionChooser* a2, 
								  CMenuOptionChooser* a3, CMenuForwarder* a4, char* d)
	{
		m1 = a1;
		m2 = a2;
		m3 = a3;
		m4 = a4;
		display=d;
		iType=Type;
		stopTime=time;
	}
	bool changeNotify(string OptionName, void* dummy)
	{
		CTimerd::CTimerEventTypes type = (CTimerd::CTimerEventTypes) *iType;
		if(type == CTimerd::TIMER_RECORD)
		{
			*stopTime=(time(NULL)/60)*60;
			struct tm *tmTime2 = localtime(stopTime);
			sprintf( display, "%02d.%02d.%04d %02d:%02d", tmTime2->tm_mday, tmTime2->tm_mon+1,
						tmTime2->tm_year+1900,
						tmTime2->tm_hour, tmTime2->tm_min);
			m1->setActive (true);
		}
		else
		{
			*stopTime=0;
			strcpy(display,"                ");
			m1->setActive (false);
		}
		if(type == CTimerd::TIMER_RECORD ||
			type == CTimerd::TIMER_ZAPTO ||
			type == CTimerd::TIMER_NEXTPROGRAM)
			m2->setActive(true);
		else
			m2->setActive(false);
		if(type == CTimerd::TIMER_STANDBY)
			m3->setActive(true);
		else
			m3->setActive(false);
		if(type == CTimerd::TIMER_REMIND)
			m4->setActive(true);
		else
			m4->setActive(false);
		return true;
	}
};


CTimerList::CTimerList()
{
	frameBuffer = CFrameBuffer::getInstance();
	visible = false;
	selected = 0;
	width = 505;
	buttonHeight = 25;
	theight= g_Fonts->menu_title->getHeight();
	fheight= g_Fonts->menu->getHeight();
	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-( height+ info_height) ) / 2) + g_settings.screen_StartY;
	liststart = 0;
	Timer = new CTimerdClient();
	skipEventID=0;
}

CTimerList::~CTimerList()
{
	timerlist.clear();
	delete Timer;
}

int CTimerList::exec(CMenuTarget* parent, string actionKey)
{
	if(actionKey=="modifytimer")
	{
		timerlist[selected].announceTime = timerlist[selected].alarmTime -60;
		Timer->modifyTimerEvent (timerlist[selected].eventID, timerlist[selected].announceTime, 
										 timerlist[selected].alarmTime, 
										 timerlist[selected].stopTime, timerlist[selected].eventRepeat);
		if(timerlist[selected].eventType == CTimerd::TIMER_RECORD)
		{
			uint apid=strtol(m_apid,NULL, 16);
			Timer->modifyTimerAPid(timerlist[selected].eventID,apid);
		}
		return menu_return::RETURN_EXIT;
	}
	if(actionKey=="newtimer")
	{
		timerNew.announceTime=timerNew.alarmTime-60;
		CTimerd::EventInfo eventinfo;
		eventinfo.epgID=0;
		eventinfo.channel_id=timerNew.channel_id;
		eventinfo.apid=0;
		timerNew.standby_on = (timerNew_standby_on == 1);
		void *data=NULL;
		if(timerNew.eventType == CTimerd::TIMER_STANDBY)
			data=&(timerNew.standby_on);
		else if(timerNew.eventType==CTimerd::TIMER_NEXTPROGRAM || 
				  timerNew.eventType==CTimerd::TIMER_ZAPTO ||
				  timerNew.eventType==CTimerd::TIMER_RECORD)
			data= &eventinfo;
		else if(timerNew.eventType==CTimerd::TIMER_REMIND)
			data= timerNew.message;
		Timer->addTimerEvent(timerNew.eventType,data,timerNew.announceTime,timerNew.alarmTime,
									timerNew.stopTime,timerNew.eventRepeat);
		return menu_return::RETURN_EXIT;
	}
/*	if(actionKey.substr(0,3)=="SC!")
	{
		printf("SC: %s\n",actionKey.substr(3).c_str());
	}*/


	if(parent)
	{
		parent->hide();
	}

//	updateEvents();
	channellist.clear();
	int nNewChannel = show();

	if( nNewChannel > -1)
	{
		return menu_return::RETURN_REPAINT;
	}
	else if( nNewChannel == -1)
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

void CTimerList::updateEvents(void)
{
	timerlist.clear();
	Timer->getTimerList (timerlist);
	//Remove last deleted event from List
	CTimerd::TimerList::iterator timer = timerlist.begin();
	for(; timer != timerlist.end();timer++)
	{
		if(timer->eventID==skipEventID)
		{
			timerlist.erase(timer);
			break;
		}
	}
	sort(timerlist.begin(), timerlist.end());

	height = 450;
	listmaxshow = (height-theight-0)/(fheight*2);
	height = theight+0+listmaxshow*fheight*2;	// recalc height
	if(timerlist.size() < listmaxshow)
	{
		listmaxshow=timerlist.size();
		height = theight+0+listmaxshow*fheight*2;	// recalc height
	}
	if(selected==timerlist.size() && timerlist.size()!=0)
	{
		selected=timerlist.size()-1;
		liststart = (selected/listmaxshow)*listmaxshow;
	}
	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-( height+ info_height) ) / 2) + g_settings.screen_StartY;
}


int CTimerList::show()
{
	int res = -1;

	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );
	uint msg; uint data;

	bool loop=true;
	bool update=true;
	while(loop)
	{
		if(update)
		{
			hide();
			updateEvents();
			update=false;
			if(timerlist.size()==0)
			{
				//evtl. anzeige dass keine kanalliste....
				/* ShowHint ( "messagebox.info", g_Locale->getText("timerlist.empty"));		
				 return -1;*/
			}
			paint();
		}
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );

		if( ( msg == CRCInput::RC_timeout ) ||
			 ( msg == CRCInput::RC_home) )
		{ //Exit after timeout or cancel key
			loop=false;
		}
		else if( msg == CRCInput::RC_up && timerlist.size() > 0)
		{
			int prevselected=selected;
			if(selected==0)
			{
				selected = timerlist.size()-1;
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
		else if( msg == CRCInput::RC_down && timerlist.size() > 0)
		{
			int prevselected=selected;
			selected = (selected+1)%timerlist.size();
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
		else if( msg == CRCInput::RC_ok && timerlist.size() > 0)
		{
			// OK button
			modifyTimer();  
			update=true;
		}
		else if(msg==CRCInput::RC_red && timerlist.size() > 0)
		{
			Timer->removeTimerEvent(timerlist[selected].eventID);
			skipEventID=timerlist[selected].eventID;
			update=true;
		}
		else if(msg==CRCInput::RC_green)
		{
			newTimer();
			update=true;
		}
		else if(msg==CRCInput::RC_yellow)
		{
			update=true;
		}
		else if((msg==CRCInput::RC_blue)||
				  (msg==CRCInput::RC_setup) ||
				  (CRCInput::isNumeric(msg)) )
		{
			//pushback key if...
			g_RCInput->postMsg( msg, data );
			loop=false;
		}
		else if( msg == CRCInput::RC_help )
		{
			// help key
		}
		else
		{
			if( CNeutrinoApp::getInstance()->handleMsg( msg, data ) == messages_return::cancel_all )
			{
				loop = false;
				res = - 2;
			}
		}
	}
	hide();

	//g_lcdd->setMode(CLcddTypes::MODE_TVRADIO, g_Locale->getText(name) );

	return(res);
}

void CTimerList::hide()
{
	if(visible)
	{
		frameBuffer->paintBackgroundBoxRel(x, y, width, height+ info_height+ 5);
		visible = false;
	}
}

void CTimerList::paintItem(int pos)
{
	int ypos = y+ theight+0 + pos*fheight*2;
	int color;
	if(pos % 2)
		color = COL_MENUCONTENTDARK;
	else
		color	= COL_MENUCONTENT;

	if(liststart+pos==selected)
	{
		color = COL_MENUCONTENTSELECTED;
	}

	frameBuffer->paintBoxRel(x,ypos, width-15, 2*fheight, color);
	if(liststart+pos<timerlist.size())
	{
		CTimerd::responseGetTimer & timer = timerlist[liststart+pos];
		char zAlarmTime[25] = {0};
		struct tm *alarmTime = localtime(&(timer.alarmTime));
		strftime(zAlarmTime,20,"%d.%m. %H:%M",alarmTime);
		char zStopTime[25] = {0};
		struct tm *stopTime = localtime(&(timer.stopTime));
		strftime(zStopTime,20,"%d.%m. %H:%M",stopTime);
		g_Fonts->menu->RenderString(x+10,ypos+fheight, 160, zAlarmTime, color, fheight);
		if(timer.stopTime != 0)
		{
			g_Fonts->menu->RenderString(x+10,ypos+2*fheight, 160, zStopTime, color, fheight);
		}
		g_Fonts->menu->RenderString(x+170,ypos+fheight, 165, convertTimerRepeat2String(timer.eventRepeat), color, fheight);
		g_Fonts->menu->RenderString(x+335,ypos+fheight, 155, convertTimerType2String(timer.eventType), color, fheight);
		string zAddData("");
		switch(timer.eventType)
		{
			case CTimerd::TIMER_NEXTPROGRAM :
			case CTimerd::TIMER_ZAPTO :
			case CTimerd::TIMER_RECORD :
				{
					zAddData=convertChannelId2String(timer.channel_id);
					if(timer.apid!=0)
					{
						char capid[20];
						sprintf(capid, "%04x", timer.apid);
						zAddData+= " ("+string(capid)+")";
					}
				}
				break;
			case CTimerd::TIMER_STANDBY :
				{
					if(timer.standby_on)
						zAddData = g_Locale->getText("timerlist.standby.on");
					else
						zAddData	= g_Locale->getText("timerlist.standby.off");
				}
				break;
			case CTimerd::TIMER_REMIND :
				{
					zAddData = timer.message ;
				}
				break;
			default:{}
		}
		g_Fonts->menu->RenderString(x+170,ypos+2*fheight, 320, zAddData, color, fheight);
		// LCD Display
		if(liststart+pos==selected)
		{
			string line1 = convertTimerType2String(timer.eventType);
			string line2 = zAlarmTime;
			switch(timer.eventType)
			{
				case CTimerd::TIMER_RECORD :
					line2+= " -";
					line2+= zStopTime+6;
				case CTimerd::TIMER_NEXTPROGRAM :
				case CTimerd::TIMER_ZAPTO :
					{
						line1+=" ";
						line1+=convertChannelId2String(timer.channel_id);
					}
					break;
				case CTimerd::TIMER_STANDBY :
					{
						if(timer.standby_on)
							line1+=" ON";
						else
							line1+=" OFF";
					}
					break;
				default:{}
			}
			g_lcdd->setMenuText(0, line1 );
			g_lcdd->setMenuText(1, line2 );
		}
	}
}

void CTimerList::paintHead()
{
	string strCaption = g_Locale->getText("timerlist.name");
	int real_width=width;
	if(timerlist.size()<=listmaxshow)
	{
		real_width-=15; //no scrollbar
	}
	frameBuffer->paintBoxRel(x,y, real_width,theight+0, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10,y+theight+0, real_width- 20, strCaption.c_str(), COL_MENUHEAD);

/*	frameBuffer->paintIcon("help.raw", x+ width- 30, y+ 5 );
	if (bouquetList!=NULL)
		frameBuffer->paintIcon("dbox.raw", x+ width- 60, y+ 5 );*/
}

void CTimerList::paintFoot()
{
	int real_width=width;
	if(timerlist.size()<=listmaxshow)
	{
		real_width-=15; //no scrollbar
	}
	int ButtonWidth = (real_width-28) / 4;
	frameBuffer->paintBoxRel(x,y+height, real_width,buttonHeight, COL_MENUHEAD);
	frameBuffer->paintHLine(x, x+real_width,  y, COL_INFOBAR_SHADOW);

	if(timerlist.size()>0)
	{
		frameBuffer->paintIcon("rot.raw", x+real_width- 4* ButtonWidth - 20, y+height+4);
		g_Fonts->infobar_small->RenderString(x+real_width- 4* ButtonWidth, y+height+24 - 2, ButtonWidth- 26, g_Locale->getText("timerlist.delete").c_str(), COL_INFOBAR);

		frameBuffer->paintIcon("ok.raw", x+real_width- 1* ButtonWidth - 30, y+height);
		g_Fonts->infobar_small->RenderString(x+real_width-1 * ButtonWidth , y+height+24 - 2, ButtonWidth- 26, g_Locale->getText("timerlist.modify").c_str(), COL_INFOBAR);

	}

	frameBuffer->paintIcon("gruen.raw", x+real_width- 3* ButtonWidth - 30, y+height+4);
	g_Fonts->infobar_small->RenderString(x+real_width- 3* ButtonWidth - 10, y+height+24 - 2, ButtonWidth- 26, g_Locale->getText("timerlist.new").c_str(), COL_INFOBAR);

	frameBuffer->paintIcon("gelb.raw", x+real_width- 2* ButtonWidth - 30, y+height+4);
	g_Fonts->infobar_small->RenderString(x+real_width- 2* ButtonWidth - 10, y+height+24 - 2, ButtonWidth- 26, g_Locale->getText("timerlist.reload").c_str(), COL_INFOBAR);

}

void CTimerList::paint()
{
	liststart = (selected/listmaxshow)*listmaxshow;

	g_lcdd->setMode(CLcddTypes::MODE_MENU, g_Locale->getText("timerlist.name") );

	paintHead();
	for(unsigned int count=0;count<listmaxshow;count++)
	{
		paintItem(count);
	}

	if(timerlist.size()>listmaxshow)
	{
		int ypos = y+ theight;
		int sb = 2*fheight* listmaxshow;
		frameBuffer->paintBoxRel(x+ width- 15,ypos, 15, sb,  COL_MENUCONTENT+ 1);

		int sbc= ((timerlist.size()- 1)/ listmaxshow)+ 1;
		float sbh= (sb- 4)/ sbc;
		int sbs= (selected/listmaxshow);

		frameBuffer->paintBoxRel(x+ width- 13, ypos+ 2+ int(sbs* sbh) , 11, int(sbh),  COL_MENUCONTENT+ 3);
	}

	paintFoot();
	visible = true;
}

string CTimerList::convertTimerType2String(CTimerd::CTimerEventTypes type)
{
	switch(type)
	{
		case CTimerd::TIMER_SHUTDOWN : return g_Locale->getText("timerlist.type.shutdown");
		case CTimerd::TIMER_NEXTPROGRAM : return g_Locale->getText("timerlist.type.nextprogram");
		case CTimerd::TIMER_ZAPTO : return g_Locale->getText("timerlist.type.zapto");
		case CTimerd::TIMER_STANDBY : return g_Locale->getText("timerlist.type.standby");
		case CTimerd::TIMER_RECORD : return g_Locale->getText("timerlist.type.record");
		case CTimerd::TIMER_REMIND : return g_Locale->getText("timerlist.type.remind");
		case CTimerd::TIMER_SLEEPTIMER: return g_Locale->getText("timerlist.type.sleeptimer");
		default: return g_Locale->getText("timerlist.type.unknown");
	}
}

string CTimerList::convertTimerRepeat2String(CTimerd::CTimerEventRepeat rep)
{
	switch(rep)
	{
		case CTimerd::TIMERREPEAT_ONCE : return g_Locale->getText("timerlist.repeat.once");
		case CTimerd::TIMERREPEAT_DAILY : return g_Locale->getText("timerlist.repeat.daily");
		case CTimerd::TIMERREPEAT_WEEKLY : return g_Locale->getText("timerlist.repeat.weekly");
		case CTimerd::TIMERREPEAT_BIWEEKLY : return g_Locale->getText("timerlist.repeat.biweekly");
		case CTimerd::TIMERREPEAT_FOURWEEKLY : return g_Locale->getText("timerlist.repeat.fourweekly");
		case CTimerd::TIMERREPEAT_MONTHLY : return g_Locale->getText("timerlist.repeat.monthly");
		case CTimerd::TIMERREPEAT_BYEVENTDESCRIPTION : return g_Locale->getText("timerlist.repeat.byeventdescription");
		default: return g_Locale->getText("timerlist.repeat.unknown");
	}
}
string CTimerList::convertChannelId2String(t_channel_id id)
{
	string name=g_Locale->getText("timerlist.program.unknown");

	if(channellist.size()==0)
	{
		CZapitClient *Zapit = new CZapitClient();
		Zapit->getChannels(channellist);
		delete Zapit;
	}
	CZapitClient::BouquetChannelList::iterator channel = channellist.begin();
	for(; channel != channellist.end();channel++)
	{
		if(channel->channel_id==id)
		{
			name=channel->name;
			break;
		}
	}
	return name;
}

void CTimerList::modifyTimer()
{
	CTimerd::responseGetTimer* timer=&timerlist[selected];
	CMenuWidget timerSettings("timerlist.menumodify", "settings.raw");
	timerSettings.addItem( new CMenuSeparator() );
	timerSettings.addItem( new CMenuForwarder("menu.back") );
	timerSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	char type[80];
	strcpy(type,convertTimerType2String(timer->eventType ).c_str()); 
	CMenuForwarder *m0 = new CMenuForwarder("timerlist.type", false, type);
	timerSettings.addItem( m0);

	CDateInput* timerSettings_alarmTime= new CDateInput("timerlist.alarmtime", &timer->alarmTime , "ipsetup.hint_1", "ipsetup.hint_2");
	CMenuForwarder *m1 = new CMenuForwarder("timerlist.alarmtime", true, timerSettings_alarmTime->getValue (), timerSettings_alarmTime );
	timerSettings.addItem( m1);

	if(timer->stopTime != 0)
	{
		CDateInput* timerSettings_stopTime= new CDateInput("timerlist.stoptime", &timer->stopTime , "ipsetup.hint_1", "ipsetup.hint_2");
		CMenuForwarder *m2 = new CMenuForwarder("timerlist.stoptime", true, timerSettings_stopTime->getValue (), timerSettings_stopTime );
		timerSettings.addItem( m2);
	}

	CMenuOptionChooser* m3 = new CMenuOptionChooser("timerlist.repeat", &((int)timer->eventRepeat ), true);
	m3->addOption((int)CTimerd::TIMERREPEAT_ONCE , "timerlist.repeat.once");
	m3->addOption((int)CTimerd::TIMERREPEAT_DAILY , "timerlist.repeat.daily");
	m3->addOption((int)CTimerd::TIMERREPEAT_WEEKLY , "timerlist.repeat.weekly");
	m3->addOption((int)CTimerd::TIMERREPEAT_BIWEEKLY , "timerlist.repeat.biweekly");
	m3->addOption((int)CTimerd::TIMERREPEAT_FOURWEEKLY , "timerlist.repeat.fourweekly");
	m3->addOption((int)CTimerd::TIMERREPEAT_MONTHLY , "timerlist.repeat.monthly");

	timerSettings.addItem(m3);
	if(timer->eventType ==  CTimerd::TIMER_RECORD)
	{
		sprintf(m_apid,"%04x",timer->apid);
		CStringInput*  timerSettings_apid= new CStringInput("timerlist.apid", m_apid , 4, "ipsetup.hint_1", "ipsetup.hint_2", "0123456789ABCDEF");
		CMenuForwarder *m4 = new CMenuForwarder("timerlist.apid", true, m_apid, timerSettings_apid );
		timerSettings.addItem( m4);
	}
	timerSettings.addItem( new CMenuForwarder("timerlist.save", true, "", this, "modifytimer") );

	if(timerSettings.exec(this,"")==menu_return::RETURN_EXIT_ALL)
		g_RCInput->postMsg( CRCInput::RC_setup, 0 );
}

void CTimerList::newTimer()
{
	// Defaults
	timerNew.eventType = CTimerd::TIMER_SHUTDOWN ;
	timerNew.eventRepeat = CTimerd::TIMERREPEAT_ONCE ;
	timerNew.alarmTime = (time(NULL)/60)*60;
	timerNew.stopTime = 0;
	timerNew.channel_id = 0;
	strcpy(timerNew.message, "");
	timerNew_standby_on =false;

	CMenuWidget timerSettings("timerlist.menunew", "settings.raw");
	timerSettings.addItem( new CMenuSeparator() );
	timerSettings.addItem( new CMenuForwarder("menu.back") );
	timerSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	CDateInput* timerSettings_alarmTime= new CDateInput("timerlist.alarmtime", &(timerNew.alarmTime) , "ipsetup.hint_1", "ipsetup.hint_2");
	CMenuForwarder *m1 = new CMenuForwarder("timerlist.alarmtime", true, timerSettings_alarmTime->getValue (), timerSettings_alarmTime );

	CDateInput* timerSettings_stopTime= new CDateInput("timerlist.stoptime", &(timerNew.stopTime) , "ipsetup.hint_1", "ipsetup.hint_2");
	CMenuForwarder *m2 = new CMenuForwarder("timerlist.stoptime", false, timerSettings_stopTime->getValue (), timerSettings_stopTime );

	CMenuOptionChooser* m3 = new CMenuOptionChooser("timerlist.repeat", &((int)timerNew.eventRepeat ), true); 
	m3->addOption((int)CTimerd::TIMERREPEAT_ONCE , "timerlist.repeat.once");
	m3->addOption((int)CTimerd::TIMERREPEAT_DAILY , "timerlist.repeat.daily");
	m3->addOption((int)CTimerd::TIMERREPEAT_WEEKLY , "timerlist.repeat.weekly");
	m3->addOption((int)CTimerd::TIMERREPEAT_BIWEEKLY , "timerlist.repeat.biweekly");
	m3->addOption((int)CTimerd::TIMERREPEAT_FOURWEEKLY , "timerlist.repeat.fourweekly");
	m3->addOption((int)CTimerd::TIMERREPEAT_MONTHLY , "timerlist.repeat.monthly");

	CMenuOptionChooser* m4 = new CMenuOptionChooser("timerlist.channel", &((int) timerNew.channel_id) , false); 
	if(channellist.size()==0)
	{
		CZapitClient *Zapit = new CZapitClient();
		Zapit->getChannels(channellist);
		delete Zapit;
	}
	CZapitClient::BouquetChannelList::iterator channel = channellist.begin();
	m4->addOption (0, "---");
	for(; channel != channellist.end();channel++)
	{
		m4->addOption((int)channel->channel_id , channel->name);
	}
//	CMenuWidget mc ("xxx", "settings.raw");
//	CMenuForwarder* m4 = new CMenuForwarder("timerlist.channel", true, timerNew_channel_name, &mc); 


	CMenuOptionChooser* m5 = new CMenuOptionChooser("timerlist.standby", &timerNew_standby_on , false); 
	m5->addOption(0 , "timerlist.standby.off");
	m5->addOption(1 , "timerlist.standby.on");

	CStringInput*  timerSettings_msg= new CStringInputSMS("timerlist.message", timerNew.message, 30,"","",
																			"abcdefghijklmnopqrstuvwxyz0123456789-.,:!?/");
	CMenuForwarder *m6 = new CMenuForwarder("timerlist.message", false, "", timerSettings_msg );

	CTimerListNewNotifier* notifier = new CTimerListNewNotifier(&((int)timerNew.eventType ),
																					&timerNew.stopTime,m2,m4,m5,m6,
																					timerSettings_stopTime->getValue ());
	CMenuOptionChooser* m0 = new CMenuOptionChooser("timerlist.type", &((int)timerNew.eventType ), true, notifier); 
	m0->addOption((int)CTimerd::TIMER_SHUTDOWN, "timerlist.type.shutdown");
	//m0->addOption((int)CTimerd::TIMER_NEXTPROGRAM, "timerlist.type.nextprogram");
	m0->addOption((int)CTimerd::TIMER_ZAPTO, "timerlist.type.zapto");
	m0->addOption((int)CTimerd::TIMER_STANDBY, "timerlist.type.standby");
	m0->addOption((int)CTimerd::TIMER_RECORD, "timerlist.type.record");
	m0->addOption((int)CTimerd::TIMER_SLEEPTIMER, "timerlist.type.sleeptimer");
	m0->addOption((int)CTimerd::TIMER_REMIND, "timerlist.type.remind");


	timerSettings.addItem( m0);
	timerSettings.addItem( m1);
	timerSettings.addItem( m2);
	timerSettings.addItem( m3);
	timerSettings.addItem( m4);
	timerSettings.addItem( m5);
	timerSettings.addItem( m6);
	timerSettings.addItem( new CMenuForwarder("timerlist.save", true, "", this, "newtimer") );
	strcpy(timerSettings_stopTime->getValue (), "                ");
	if(timerSettings.exec(this,"")==menu_return::RETURN_EXIT_ALL)
		g_RCInput->postMsg( CRCInput::RC_setup, 0 );
}
