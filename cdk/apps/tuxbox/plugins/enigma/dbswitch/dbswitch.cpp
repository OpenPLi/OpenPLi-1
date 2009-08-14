/*
	DBSwitch - Enigma Plugin

	Copyright (C) 2005 'mechatron' (mechatron@gmx.net)

	Homepage: http://mechatron.6x.to/

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

#include "dbswitch.h"
#include "rcinput.h"

extern "C" int plugin_exec( PluginParam *par );

dbswitch::dbswitch()
	: rc_status(eApp)
{
	int fd=eSkin::getActive()->queryValue("fontsize", 20);

	char *tmpcode, *tmpcolor;
	if ( eConfig::getInstance()->getKey("/elitedvb/plugins/db_switch/code", tmpcode) ) tmpcode="2069";
	if ( eConfig::getInstance()->getKey("/elitedvb/plugins/db_switch/color", tmpcolor) ) tmpcolor="yellow";
	codeentry=tmpcode; color_bt=tmpcolor;

	cmove(ePoint(220, 200));
	cresize(eSize(260, 130));
	setText("DB-Switch v0.2.5");

	eLabel *f=new eLabel(this);
	f->setText("Code for Return");
	f->move(ePoint(10,10));
	f->resize(eSize(180, fd+10));

	code=new eTextInputField(this);
	code->move(ePoint(190, 10));
	code->resize(eSize(60, fd+10));
	code->setMaxChars(4);
	code->setUseableChars("1234567890");
	code->setText(codeentry);
	code->loadDeco();

	con_yellow=new eButton(this);
	con_yellow->move(ePoint(10, 70));
	con_yellow->resize(eSize(110, fd+10));
	con_yellow->setShortcut("yellow");
	con_yellow->setShortcutPixmap("yellow");
	con_yellow->setText("Start");
	con_yellow->loadDeco();
	con_yellow->hide();
	CONNECT(con_yellow->selected, dbswitch::start);

	con_green=new eButton(this);
	con_green->move(ePoint(10, 70));
	con_green->resize(eSize(110, fd+10));
	con_green->setShortcut("green");
	con_green->setShortcutPixmap("green");
	con_green->setText("Start");
	con_green->loadDeco();
	con_green->hide();
	CONNECT(con_green->selected, dbswitch::start);

	eButton *col_bt=new eButton(this);
	col_bt->move(ePoint(140, 70));
	col_bt->resize(eSize(110, fd+10));
	col_bt->setShortcut("blue");
	col_bt->setShortcutPixmap("blue");
	col_bt->setText("switch");
	col_bt->loadDeco();
	CONNECT(col_bt->selected, dbswitch::col_press);

	CONNECT( rc_status.timeout, dbswitch::status);

	col_press();
}
dbswitch::~dbswitch()
{
	if(code->getText())		eConfig::getInstance()->setKey("/elitedvb/plugins/db_switch/code", code->getText().c_str());
	if(color_bt == "yellow")	eConfig::getInstance()->setKey("/elitedvb/plugins/db_switch/color", "green");
	else				eConfig::getInstance()->setKey("/elitedvb/plugins/db_switch/color", "yellow");
}
void dbswitch::status()
{
	if(rcplay::getInstance()->state == rcplay::PLAY) rc_status.start(1000, true);
	else
	{
#ifndef DISABLE_LCD
		eDBoxLCD::getInstance()->unlock();
#endif
		eRCInput::getInstance()->unlock();
		//show();
		close(0);
	}
}

void dbswitch::col_press()
{
	if(color_bt=="yellow") { con_green->hide(); con_yellow->show(); setFocus(con_yellow); color_bt="green"; }
	else { con_green->show(); con_yellow->hide(); setFocus(con_green); color_bt="yellow"; }
}

void dbswitch::start()
{
	if(code->getText())
	{
		rcplay::getInstance()->play(code->getText().c_str());
		rc_status.start(1000, true);
		hide();
		eRCInput::getInstance()->lock();
#ifndef DISABLE_LCD
		eDBoxLCD::getInstance()->lock();
#endif
	}
}

//-----------------rcplay----------------

CRCInput *rc;
CRCInput *getRc ()  { return rc; }

int rcplay::rcloop(const char *code_str)
{
	int Status=0;
	state = PLAY;

	rc = new CRCInput();
	char tmp_code[4]; strcpy(tmp_code,code_str);

#ifndef DISABLE_LCD
	display.draw_string(1, 10, "RC - Switch");
	display.draw_string(1, 20, "---------------");
	display.draw_string(1, 30, "CODE is :");
	display.draw_string(80, 30, tmp_code);
	display.draw_string(1, 40, "Entry is:      ");
	display.draw_string(1, 50, "               ");
	display.update();
#endif

	int update=0;
	int done=0;
	int a=0,b=0,c=0,d=0;
	char nv[3];
	while (!done)
	{
		if (!update)
		{
			int code = rc->getKey();
			switch (code)
			{
				case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:
				{
					a=b;b=c;c=d;d=code;
					sprintf(nv, "%d%d%d%d", a,b,c,d);
					update=1;
					break;
				}
			}
		}
		else
		{
#ifndef DISABLE_LCD
			display.draw_string(80, 40, nv);
			display.update();
#endif
			update=0;
		}
		if(strcmp(nv,code_str)==0) done=1;
	}

#ifndef DISABLE_LCD
	display.draw_string(80, 40, nv);
	display.draw_string(1, 50, "     EXIT");
	display.update();
#endif
	return(Status);
}
rcplay* rcplay::getInstance()
{
	static rcplay* rcplayer = NULL;
	if(rcplayer == NULL) rcplayer = new rcplay();
	return rcplayer;
}


void* rcplay::PlayThread(void * str)
{
	int Status = rcplay::getInstance()->rcloop((char *)str);
	if(Status > 0) eDebug("[DBSWITCH] Error");
	rcplay::getInstance()->state = STOP;
	pthread_exit(0);
	return NULL;
}

bool rcplay::play(const char *str)
{
	if(true)
	{
		state = PLAY;
		if (pthread_create (&thrPlay, NULL, PlayThread,(void *) str) != 0 )
		{
			perror("rcplay: pthread_create(PlayThread)");
			return false;
		}
	}
	else
		PlayThread((void *) str);

	return true;
}

rcplay::rcplay()
{
	state = STOP;
}

int plugin_exec( PluginParam *par )
{
	dbswitch dlg; dlg.show(); dlg.exec(); dlg.hide(); return 0;
}

