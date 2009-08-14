/***************************************************************************
    copyright            : (C) 2001 by TheDOC
    email                : thedoc@chatville.de
	homepage			 : www.chatville.de
	modified by			 : -
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
$Log: main.cpp,v $
Revision 1.31.2.1.2.6  2008/08/09 16:41:51  fergy
Cleaning code
Enabled some debug stuff
Enabled some disabled features

Revision 1.31.2.1.2.5  2008/08/07 20:25:30  fergy
Mostly clear of not needed lines
Added back debug messages ( just for dev. )
Enambled some disabled stuff from before
Code cleaning

Revision 1.31.2.1.2.4  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.31.2.1.2.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.32  2003/07/07 23:53:11  thedoc

lcd-support for lcars in HEAD

Revision 1.31  2003/01/05 22:48:34  TheDOC
mtd number

Revision 1.30  2003/01/05 21:42:30  TheDOC
small changes

Revision 1.29  2003/01/05 21:07:09  TheDOC
new version-number and README updated

Revision 1.28  2003/01/05 19:52:47  TheDOC
forgot include

Revision 1.27  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.26  2003/01/05 02:41:53  TheDOC
lcars supports inputdev now

Revision 1.25  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.24  2002/09/18 10:48:37  obi
use devfs devices

Revision 1.23  2002/06/15 02:33:03  TheDOC
some changes + bruteforce-channelscan for cable

Revision 1.22  2002/06/12 23:30:03  TheDOC
basic NVOD should work again

Revision 1.21  2002/06/02 14:23:36  TheDOC
some fixes and changes

Revision 1.20  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.19  2002/05/21 04:37:42  TheDOC
http-update... new web-frontend in http://dbox/file/start.htm... will be main index soon

Revision 1.18  2002/05/20 20:11:13  TheDOC
version fix - greetings to all readers of the tux-cvs-mailinglist :) just post a quick Hello to #dbox2 if you read that message ;)

Revision 1.17  2002/05/20 20:08:12  TheDOC
some new timer and epg-stuff

Revision 1.16  2002/05/18 04:31:02  TheDOC
Warningelimination

Revision 1.15  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.13  2002/03/03 23:06:51  TheDOC
update-fix

Revision 1.12  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.10  2001/12/19 04:48:37  tux
Neue Plugin-Schnittstelle

Revision 1.9  2001/12/19 03:23:01  tux
event mit 16:9-Umschaltung

Revision 1.7  2001/12/17 18:37:05  tux
Finales Settingsgedoens

Revision 1.6  2001/12/17 16:54:47  tux
Settings halb komplett

Revision 1.4  2001/12/17 03:52:41  tux
Netzwerkfernbedienung fertig

Revision 1.3  2001/12/17 01:00:41  tux
scan.cpp fix

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.10  2001/12/12 15:48:35  TheDOC
Segfault with too big numbers fixed (perhaps the oldest bug in lcars *g*)

Revision 1.9  2001/12/12 15:28:10  TheDOC
Segfault after Scan-Bug fixed (forgot scans in running lcars)

Revision 1.7  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.6  2001/11/15 00:43:45  TheDOC
 added

*/
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string>
#include <stdio.h>
#include <config.h>

#include "devices.h"
#include "sdt.h"
#include "zap.h"
#include "nit.h"
#include "pat.h"
#include "pmt.h"
#include "eit.h"
#include "settings.h"
#include "rc.h"
#include "channels.h"
#include "osd.h"
#include "fbClass.h"
#include "checker.h"
#include "teletext.h"
#include "network.h"
#include "scan.h"
#include "hardware.h"
#include "tot.h"
#include "plugins.h"
#include "pig.h"
#include "timer.h"
#include "update.h"
#include "control.h"
#include "variables.h"
#include "ir.h"
#include "lcd.h"
#include "control.h"

int main(int argc, char **argv)
{
	variables variables;
	ir ir;

	int key = -1;
	int number = -1;
	std::string font = FONTDIR "/ds9.ttf";
	std::string vtfont = FONTDIR "/ds9.ttf";

	plugins plugins;

	cam cam;
	sdt sdt;
	nit nit;
	cam.readCAID();

	settings settings(&cam);

	settings.setVersion("0.30");

	hardware hardware(&settings, &variables);
	hardware.useDD(false);

	rc rc(&hardware, &settings);

	lcddisplay lcd;
	lcd.loadFont(FONTDIR "/ds9.ttf");

	fbClass fb(&variables);
	fb.setPalette(255, 0, 0, 0, 0xff);
	fb.setTransparent(255);
	fb.clearScreen();
	fb.loadFonts(font, vtfont);
	//fb.setFade(1, 22, 5, 57, 63, 63, 63);
	//fb.fillBox(100, 100, 200, 200, 1);
	//sleep(10);
	//fb.test();

	osd osd(settings, &fb, &variables);
	osd.start_thread();



	update update(&osd, &rc, &settings);
	update.cramfsmtd = 2;

	pig pig;
	pig.hide();

	int test = open(DEMUX_DEV, O_RDWR);
	if (test < 0)
	{
		rc.start_thread();
		osd.createIP();
		osd.setIPDescription("Please enter IP-address!");
		osd.addCommand("SHOW ip");
		osd.addCommand("COMMAND ip position 0");
		do
		{
			key = rc.read_from_rc();
			number = rc.get_number();
			if (number != -1)
			{
				osd.setIP(number);
				osd.setIPNextPosition();
			}
			else if (key == RC_RIGHT)
			{
				osd.setIPNextPosition();
			}
			else if (key == RC_LEFT)
			{
				osd.setIPPrevPosition();
			}
		} while ( key != RC_OK && key != RC_HOME);
		if (key == RC_OK)
		{
			settings.setIP(osd.getIPPart(0), osd.getIPPart(1), osd.getIPPart(2), osd.getIPPart(3));

			char command[100];
			sprintf(command, "ifconfig eth0 %d.%d.%d.%d", osd.getIPPart(0), osd.getIPPart(1), osd.getIPPart(2), osd.getIPPart(3));
			printf("%s\n", command);
			system(command);


		}
		osd.addCommand("HIDE ip");
		osd.createPerspective();
		update.run(UPDATE_UCODES);
		osd.setPerspectiveName("Reboot now!");
		osd.addCommand("SHOW perspective");
		sleep(4);
		exit(1);
	}
	close(test);

	tuner tuner(&settings);
	zap zap(settings, osd, tuner, cam/*, lcd*/);

	pmt pmt;
	pat pat;
	tot tot(&settings);

	eit eit(&settings, &osd, &variables);
	eit.start_thread();

	channels channels(&settings, &pat, &pmt, &eit, &cam, &hardware, &osd, &zap, &tuner, &variables);
	checker checker(&settings, &hardware);
	checker.set_16_9_mode(settings.getVideoFormat());

	//checker.start_16_9_thread();
	checker.startEventThread();

	scan scan(&settings, &pat, &pmt, &nit, &sdt, &osd, &tuner, &channels);

	if (rc.command_available())
	{
		int com = rc.read_from_rc();
		if (com == RC_HELP)
		{
			channels = scan.scanChannels(NORMAL);
			channels.setStuff(&eit, &cam, &hardware, &osd, &zap, &tuner, &variables);
			channels.saveDVBChannels();
			while(rc.command_available())
				rc.read_from_rc();
		}

	}

	teletext teletext(&fb, &rc);

	channels.loadDVBChannels();
	channels.loadTS();

	if (channels.numberChannels() == 0 || channels.numberTransponders() == 0)
	{
		channels = scan.scanChannels();
		channels.setStuff(&eit, &cam, &hardware, &osd, &zap, &tuner, &variables);
		channels.saveDVBChannels();
		channels.saveTS();
	}
	container container(&zap, &channels, &fb, &osd, &settings, &tuner, &pat, &pmt, &eit, &scan);

	channels.setBeginTS();
	while (channels.getCurrentFrequency() == 0)
		channels.setNextTS();

	channels.tuneCurrentTS();

	settings.getEMMpid();
	timer timer(&hardware, &channels, &zap, &tuner, &osd, &variables);
	timer.loadTimer();
	timer.start_thread();
	tot.start_thread();

	int mode = 0; // 0 = Main Menu
	int ipmode;

	int final_number;
	bool finish;

	int channelnumber;
	int old_channel;
	if (argc == 1)
	{
		channelnumber = 0;
		old_channel = 0;
	}
	else
	{
		channelnumber = atoi(argv[1]);
		old_channel = atoi(argv[1]);
	}

	int apid = 0;
	linkage perspective[10];
	int number_perspectives = 0;
	int position;
	int selected;
	int curr_perspective = 0;
	int old_TS = 0;
	pmt_data pmt_entry;
	int ECM = 0;
	event now, next, nowref;
	int component[10];
	char audio_description[20];
	int curr_nvod = 0, nvod_count = 0;
	channel nvods[10];
	int APIDcount = 0;
	int video_component = 0;
	int switchmode = 2;
	int currentepg = 1;
	bool change = true;
	int positionepg = 0;
	bool allowkeys = true;
	bool leave = false;
	bool schedule_read = false;
	char text[20];

	hardware.setOutputMode(settings.getOutputFormat());
	rc.start_thread();

	control control(&osd, &rc, &hardware, &settings, &scan, &channels, &eit, &cam, &zap, &tuner, &update, &timer, &plugins, &checker, &fb, &variables, &ir, &pig, &teletext, &sdt, &lcd);
	
	network network(&zap, &channels, &fb, &osd, &settings, &tuner, &pat, &pmt, &eit, &scan, &rc, &control, &variables);
	network.startThread();


	control.run();
	exit(0);
	do
	{
		
		time_t act_time;

		switch (mode)
		{
		case 0:
			if (channelnumber > channels.numberChannels())
				channelnumber = 0;
			channels.setCurrentChannel(channelnumber);

			if (channels.getCurrentType() > 4 || channels.getCurrentType() < 1)
			{
				channelnumber++;
				continue;
			}
			
			osd.addCommand("SHOW proginfo 5");
			channels.setCurrentOSDProgramInfo();
			printf("Startzapping\n");
			channels.zapCurrentChannel();
			scan.readUpdates();
			printf("End zapping\n");
			schedule_read = false;
			if (channels.getCurrentTXT() != 0)
			{
				osd.addCommand("COMMAND proginfo set_teletext true");			
				teletext.startReinsertion(channels.getCurrentTXT());
			}
			else
				osd.addCommand("COMMAND proginfo set_teletext false");

			printf("Channel: %s\n", channels.getCurrentServiceName().c_str());
			printf("SID: %04x\n",channels.getCurrentSID());
			printf("PMT: %04x\n", pat.getPMT(channels.getCurrentSID()));

			printf("Audio-PIDs: %d\n", channels.getCurrentAPIDcount());
			
			
			channels.receiveCurrentEIT();
			
			mode = 2;
			break;

		case 1: // Wait for key or timeout
			printf("Warten auf timeout...\n");
			act_time = time(0);
			while ((!rc.command_available()) && (time(0) - act_time < 5));
			mode = switchmode;
			break;
		case 2: // Main Key-Loop
			do
			{
				do
				{
					key = rc.read_from_rc();
					number = rc.get_number();
					if (key == RC1_MENU)
					{
						hardware.switch_vcr();
						if (hardware.vcrIsOn())
						{
							allowkeys = false;
							switch(hardware.getVCRStatus())
							{
							case VCR_STATUS_OFF:
								hardware.fnc(0);
								checker.laststat = 0;
								break;
							case VCR_STATUS_ON:
								hardware.fnc(2);
								checker.laststat = 0;
								break;
							case VCR_STATUS_16_9:
								hardware.fnc(1);
								checker.laststat = 1;
								break;
							}
						}
						else 
						{
							allowkeys = true;
							checker.aratioCheck();
							
						}
					}
					if (key == RC1_RIGHT)
					{
						apid++;
						if (apid >= channels.getCurrentAPIDcount())
							apid = 0;
						channels.zapCurrentAudio(apid);
					}
					else if (key == RC1_LEFT)
					{
						apid--;
						if (apid < 0 )
							apid = channels.getCurrentAPIDcount() - 1;
						
						channels.zapCurrentAudio(apid);
					}
					else if (key == RC1_MUTE)
					{
						if (hardware.isMuted())
						{
							osd.setMute(false);
							hardware.switch_mute();
						}
						else
						{
							osd.setMute(true);
							hardware.switch_mute();
						}
					}

				} while(!allowkeys);
			} while (key != RC1_VOLPLUS && key != RC1_VOLMINUS && key != RC1_UP && key != RC1_DOWN && key != RC1_STANDBY && number == -1 && key != RC1_OK && key != RC1_RED && key != RC1_GREEN && key != RC1_BLUE && key != RC1_YELLOW && key != RC1_HELP);
			
			channelnumber = channels.getCurrentChannelNumber();
			if (key == RC1_DOWN)
			{
				old_channel = channelnumber;
				while (++channelnumber < channels.numberChannels() && channels.getType(channelnumber) > 4);
				if (channelnumber >= channels.numberChannels())
					channelnumber = 0;
				apid = 0;
			}
			else if (key == RC1_0)
			{
				int tmp = old_channel;
				old_channel = channelnumber;
				channelnumber = tmp;
				apid = 0;
			}
			else if (key == RC1_UP)
			{
				old_channel = channelnumber;
				while (--channelnumber >= 0 && channels.getType(channelnumber) > 4);
				if (channelnumber < 0 )
					channelnumber = channels.numberChannels() - 1;
				apid = 0;
			}
			else if (key == RC1_VOLPLUS)
			{
				osd.setVolume(63 - hardware.vol_plus(5));
				osd.showVolume();
				mode = 11;
				switchmode = 2;
				continue;	
			}
			else if (key == RC1_VOLMINUS)
			{
				osd.setVolume(63 - hardware.vol_minus(5));
				osd.showVolume();
				mode = 11;
				switchmode = 2;
				continue;
			}
			else if (key == RC1_HELP)
			{
				if (!schedule_read)
				{
					eit.dumpSchedule(channels.getCurrentSID(), &osd);
					schedule_read = true;
				}

				osd.createSchedule();
				eit.dumpSchedule(channels.getCurrentTS(), channels.getCurrentONID(), channels.getCurrentSID(), &osd);
				printf("Wow\n");
				osd.showSchedule(0);
				osd.selectScheduleInformation(0);
				mode = 12;
				switchmode = 2;
				continue;
			}

			mode = 0;
			if (number >= 1)
				mode = 3;
			if (key == RC1_OK)
			{
				mode = 2;
				if (osd.proginfo_shown)
					osd.addCommand("HIDE proginfo");
				else
					osd.addCommand("SHOW proginfo 5");
			}
			else if (key == RC1_RED)
			{
				mode = 4;
			}
			else if (key == RC1_YELLOW)
			{
				osd.createEPG();
				now = eit.getNow();
				next = eit.getNext();
				osd.setEPGEventName(now.event_name);
				osd.setEPGEventShortText(now.event_short_text);
				osd.setEPGEventExtendedText(now.event_extended_text);
				osd.setEPGProgramName(channels.getCurrentServiceName());
				osd.setEPGstarttime(now.starttime);
				osd.setEPGduration(now.duration);
				change = true;
				currentepg = 1;
				positionepg = 0;
				mode = 8;
			}
			else if (key == RC1_BLUE)
			{
					mode = 9;
					switchmode = 2;
			}
			else if (channels.currentIsMultiPerspective() && (key == RC1_GREEN))
			{	
				mode = 5;
				number_perspectives = channels.currentNumberPerspectives();
				channels.parsePerspectives();
			}
			else if ((channels.getCurrentType() == 4) && (key == RC1_GREEN))
			{
				mode = 7;
				curr_nvod = 0;
				osd.createPerspective();
				osd.setPerspectiveName("Reading NVOD-Data...");
				osd.addCommand("SHOW perspective");
				sdt.getNVODs(&channels);
				nvod_count = channels.getCurrentNVODcount();
				{
					
					for (int i = 0; i < nvod_count; i++)
					{
						printf("NVOD: TS: %x - SID: %x\n", channels.getCurrentNVOD_TS(i),channels.getCurrentNVOD_SID(i));
						nvods[i].TS = channels.getCurrentNVOD_TS(i);
						nvods[i].SID = channels.getCurrentNVOD_SID(i);
					}

				}
				
				char message[100];
				sprintf(message, "Please choose nvod (%d - %d)", 1, nvod_count);
				osd.setPerspectiveName(message);
				osd.addCommand("SHOW perspective");
				continue;
			}
			break;
		case 3: // Number entry
			final_number = number;
			finish = false;
			osd.createNumberEntry();
			osd.setNumberEntry(number);
			osd.setNumberText(channels.getServiceName(number));
			osd.addCommand("SHOW number");
			printf("%d\n", final_number);
			do
			{
				act_time = time(0);
				while ((!rc.command_available()) && (time(0) - act_time < 2));
				printf("%d\n", (int) (time(0) - act_time));
				if (time(0) - act_time >= 2)
					finish = true;
				else if (rc.command_available())
				{
					key = rc.read_from_rc();
					int tmp_number = rc.get_number();
					if (tmp_number != -1)
					{
						final_number = final_number * 10 + tmp_number;
						if (final_number > 9999)
							finish = true;
						else
						{
							osd.setNumberEntry(final_number);
							if (final_number < channels.numberChannels() - 1)
								osd.setNumberText(channels.getServiceName(final_number));
							else
								osd.setNumberText(channels.getServiceName(channels.numberChannels() - 1));
							osd.addCommand("SHOW number");
						}
					}
					else if (key == RC1_OK)
						finish = true;
					else if (key == RC1_BLUE && final_number == 7493)
					{
						network.update_enabled = true;
					}
					else if (key == RC1_MUTE)
					{
						if (hardware.isMuted())
						{
							osd.setMute(false);
							hardware.switch_mute();
						}
						else
						{
							osd.setMute(true);
							hardware.switch_mute();
						}
					}
				}
			} while (!finish);
			if (final_number > channels.numberChannels() - 1)
				final_number = channels.numberChannels();
			old_channel = channelnumber;
			channelnumber = final_number;
			osd.addCommand("HIDE number");
			mode = 0;
			break;
		case 4: // Channel-List
			position = (int) (channelnumber / 10);
			selected = channelnumber % 10;
			osd.createList();
			for (int i = position * 10; i < position * 10 + 10; i++)
			{
				osd.addListItem(i, channels.getServiceName(i));
			}
			osd.addCommand("SHOW list");
			char cmd_text[100];
			sprintf(cmd_text, "COMMAND list select_item %d", selected);
			osd.addCommand(cmd_text);
			do
			{
				key = rc.read_from_rc();
				number = rc.get_number();
				if (key == RC1_MENU)
					hardware.switch_vcr();
				if (key == RC1_UP)
				{
					selected--;
					if (selected < 0)
					{
						position--;
						if (position < 0)
							position = (int) (channels.numberChannels() / 10);
						selected = 9;
						osd.createList();
						for (int i = position * 10; i < position * 10 + 10; i++)
						{
							osd.addListItem(i, channels.getServiceName(i));//.substr(0,8));
						}
						osd.addCommand("SHOW list");
					}

					char cmd_text[100];
					sprintf(cmd_text, "COMMAND list select_item %d", selected);
					osd.addCommand(cmd_text);
				}
				if (key == RC1_DOWN)
				{
					selected++;
					if (selected > 9)
					{
						position++;
						if (position > (int)(channels.numberChannels() / 10))
							position = 0;
						selected = 0;
						osd.createList();
						for (int i = position * 10; i < position * 10 + 10; i++)
						{
							osd.addListItem(i, channels.getServiceName(i));//.substr(0,8));
						}
						osd.addCommand("SHOW list");
					}

					char cmd_text[100];
					sprintf(cmd_text, "COMMAND list select_item %d", selected);
					osd.addCommand(cmd_text);
				}
				if (key == RC1_RIGHT)
				{
					position++;
					if (position > (int)(channels.numberChannels() / 10))
						position = 0;
					selected = 0;
					osd.createList();
					for (int i = position * 10; i < position * 10 + 10; i++)
					{
						osd.addListItem(i, channels.getServiceName(i));//.substr(0,8));
					}
					osd.addCommand("SHOW list");
					char cmd_text[100];
					sprintf(cmd_text, "COMMAND list select_item %d", selected);
					osd.addCommand(cmd_text);
				}
				if (key == RC1_LEFT)
				{
					position--;
					if (position < 0)
						position = (int) (channels.numberChannels() / 10);
					selected = 9;
					osd.createList();
					for (int i = position * 10; i < position * 10 + 10; i++)
					{
						osd.addListItem(i, channels.getServiceName(i));//.substr(0,8));
					}
					osd.addCommand("SHOW list");
					char cmd_text[100];
					sprintf(cmd_text, "COMMAND list select_item %d", selected);
					osd.addCommand(cmd_text);
				}
				if (number != -1)
				{
					if (number != 0)
						osd.selectItem(number - 1);
					else
						osd.selectItem(9);
					selected = osd.selectedItem();
				}
				else if (key == RC1_MUTE)
				{
					if (hardware.isMuted())
					{
						osd.setMute(false);
						hardware.switch_mute();
					}
					else
					{
						osd.setMute(true);
						hardware.switch_mute();
					}
				}
			} while (key != RC1_OK && key != RC1_STANDBY && key != RC1_RED && key != RC1_HOME);
			if (key == RC1_RIGHT)
			{
				apid++;
				if (apid >= channels.getCurrentAPIDcount())
					apid = 0;
			}
			osd.addCommand("HIDE list");
			mode = 0;
			if (key == RC1_RED || key == RC1_HOME)
			{
				mode = 2;
			}
			else if (key == RC1_OK)
			{
				old_channel = channelnumber;
				channelnumber = position * 10 + osd.selectedItem();
			}
			break;
		case 5: // linkage
			do
			{
				do
				{
					key = rc.read_from_rc();
					number = rc.get_number();
					if (key == RC1_YELLOW)
						osd.clearScreen();
					else if (key == RC1_MENU)
					{
						hardware.switch_vcr();
						if (hardware.vcrIsOn())
						{
							allowkeys = false;
							switch(hardware.getVCRStatus())
							{
							case VCR_STATUS_OFF:
								hardware.fnc(0);
								checker.laststat = 0;
								break;
							case VCR_STATUS_ON:
								hardware.fnc(2);
								checker.laststat = 0;
								break;
							case VCR_STATUS_16_9:
								hardware.fnc(1);
								checker.laststat = 1;
								break;
							}
						}
						else 
						{
							allowkeys = true;
							checker.aratioCheck();
							
						}
					}
					else if (key == RC1_MUTE)
					{
						if (hardware.isMuted())
						{
							osd.setMute(false);
							hardware.switch_mute();
						}
						else
						{
							osd.setMute(true);
							hardware.switch_mute();
						}
						
					}
				} while(!allowkeys);
				if (key == RC1_RIGHT)
				{
					apid++;
					if (apid >= channels.getCurrentAPIDcount())
						apid = 0;

					channels.zapCurrentAudio(apid);
					zap.zap_audio(perspective[curr_perspective].VPID, perspective[curr_perspective].APID[apid] , ECM, perspective[curr_perspective].SID, perspective[curr_perspective].ONID);
						

				}
				else if (key == RC1_LEFT)
				{
					apid--;
					if (apid < 0 )
						apid = channels.getCurrentAPIDcount() - 1;

					channels.zapCurrentAudio(apid);
					zap.zap_audio(perspective[curr_perspective].VPID, perspective[curr_perspective].APID[apid] , ECM, perspective[curr_perspective].SID, perspective[curr_perspective].ONID);
							
				}
			} while(key != RC1_GREEN && number == -1 && key != RC1_BLUE && key != RC1_STANDBY && key != RC1_DOWN && key != RC1_UP);

			if (key == RC1_GREEN)
			{
				mode = 0;
				continue;
			}
			else
			{
				
				if (number != -1)
				{
					
					if (number < number_perspectives + 1 && number > 0)
						curr_perspective = (number % (number_perspectives + 1)) - 1;

				}
				mode = 6;
				
			}
			if (key == RC1_UP)
			{
				curr_perspective++;
				if (curr_perspective >= number_perspectives)
					curr_perspective = 0;
				mode = 6;
			}
			else if (key == RC1_DOWN)
			{
				curr_perspective--;
				if (curr_perspective <0)
					curr_perspective = number_perspectives - 1;
				mode = 6;
			}
			else if (key == RC1_BLUE)
			{
				mode = 9;
				switchmode = 5;
				continue;
			}

			channels.setPerspective(curr_perspective);

			
			schedule_read = false;
			break;
			
		case 6:
			printf("Waiting for timeout...\n");
			act_time = time(0);
			while ((!rc.command_available()) && (time(0) - act_time < 3));
			osd.addCommand("HIDE perspective");
			mode = 5;
			break;
		case 7: // NVOD
			printf("NVOD\n");

			do
			{
				do
				{
					key = rc.read_from_rc();
					number = rc.get_number();
					if (key == RC1_YELLOW)
						osd.clearScreen();
					else if (key == RC1_MENU)
					{
						hardware.switch_vcr();
						if (hardware.vcrIsOn())
						{
							allowkeys = false;
							switch(hardware.getVCRStatus())
							{
							case VCR_STATUS_OFF:
								hardware.fnc(0);
								checker.laststat = 0;
								break;
							case VCR_STATUS_ON:
								hardware.fnc(2);
								checker.laststat = 0;
								break;
							case VCR_STATUS_16_9:
								hardware.fnc(1);
								checker.laststat = 1;
								break;
							}
						}
						else 
						{
							allowkeys = true;
							checker.aratioCheck();
							
						}
					}
					else if (key == RC1_MUTE)
					{
						if (hardware.isMuted())
						{
							osd.setMute(false);
							hardware.switch_mute();
						}
						else
						{
							osd.setMute(true);
							hardware.switch_mute();
						}
					}
				} while(!allowkeys);
				if (key == RC1_RIGHT)
				{
					apid++;
					if (apid >= APIDcount)
						apid = 0;

					hardware.useDD(nvods[curr_nvod].DD[apid]);
					zap.zap_audio(nvods[curr_nvod].VPID, nvods[curr_nvod].APID[apid] , ECM, nvods[curr_nvod].SID, 0x85);
											
					for (int i = 0; i < now.number_components; i++)
					{
						if (now.component_tag[i] == component[apid])
						{
							strcpy(audio_description, now.audio_description[i]);
						}
					}

					if (channels.getCurrentAPIDcount() > 1)
							osd.setLanguage(audio_description);
				}
				else if (key == RC1_LEFT)
				{
					apid--;
					if (apid < 0 )
						apid = APIDcount - 1;

					hardware.useDD(nvods[curr_nvod].DD[apid]);
					zap.zap_audio(nvods[curr_nvod].VPID, nvods[curr_nvod].APID[apid] , ECM, nvods[curr_nvod].SID, 0x85);
					
						
					for (int i = 0; i < now.number_components; i++)
					{
						if (now.component_tag[i] == component[apid])
						{
							strcpy(audio_description, now.audio_description[i]);
						}
					}

					if (channels.getCurrentAPIDcount() > 1)
							osd.setLanguage(audio_description);
				}
			
			} while(key != RC1_GREEN && number == -1 && key != RC1_STANDBY && key != RC1_BLUE && key != RC1_OK);
			osd.addCommand("HIDE perspective");
			printf("%d\n", number);
			if (key == RC1_GREEN)
			{
				mode = 0;
				continue;
			}
			if (key == RC1_BLUE)
			{
				mode = 9;
				switchmode = 7;
				continue;
			}
			else if (key == RC1_OK)
			{
				switchmode = 7;
				mode = 1;
				if (next.starttime <= time(0))
				{
					eit.receiveNow(nvods[curr_nvod].SID);
				}
				if (osd.proginfo_shown)
					osd.addCommand("HIDE proginfo");
				else
					osd.addCommand("SHOW proginfo 5");
				break;
			}
			else
			{
				
				if (number != -1)
				{
					
					if (number < nvod_count + 1)
						curr_nvod = (number % (nvod_count + 1)) - 1;

				}				
			}

			printf("----------------------\n");
			printf("APID: %d\n", apid);
			printf("Current NVOD: %d\n", curr_nvod);
			if (old_TS != nvods[curr_nvod].TS)
				channels.tune(nvods[curr_nvod].TS, nvods[curr_nvod].ONID);
			
			printf("Tuning to TS: %d\n", nvods[curr_nvod].TS);
			
			
			zap.close_dev();
			
			old_TS = (channels.getCurrentTS(), channels.getCurrentONID());

			pat.readPAT();
			
			ECM = 0;
			
			memset (&pmt_entry, 0, sizeof (struct pmt_data));
			if (pat.getPMT(nvods[curr_nvod].SID) != 0)
				pmt_entry = pmt.readPMT(pat.getPMT(nvods[curr_nvod].SID));

			APIDcount = 0;	
			for (int i = 0; i < pmt_entry.pid_counter; i++)
			{
				if (pmt_entry.type[i] == 0x02)
					nvods[curr_nvod].VPID = pmt_entry.PID[i];
				else if (pmt_entry.type[i] == 0x04 || pmt_entry.type[i] == 0x03)
				{
					printf("an APID: %04x\n", pmt_entry.PID[i]);
					nvods[curr_nvod].DD[APIDcount] = false;
					nvods[curr_nvod].APID[APIDcount++] = pmt_entry.PID[i];
				}
				else if (pmt_entry.type[i] == 0x06)
				{
					printf("on APID: %04x\n", pmt_entry.PID[i]);
					nvods[curr_nvod].DD[APIDcount] = true;
					nvods[curr_nvod].APID[APIDcount++] = pmt_entry.PID[i];
					
				}
				printf("type: %d - PID: %04x\n", pmt_entry.type[i], pmt_entry.PID[i]);
			}

			printf("ECMs: %d\n", pmt_entry.ecm_counter);
				
			for (int i = 0; i < pmt_entry.ecm_counter; i++)
			{
				if (settings.getCAID() == pmt_entry.CAID[i])
					ECM = pmt_entry.ECM[i];
				printf("CAID: %04x - ECM: %04x\n", pmt_entry.CAID[i], pmt_entry.ECM[i]);

			}
			
			hardware.useDD(nvods[curr_nvod].DD[0]);
			if (APIDcount == 1)
			{
				zap.zap_to(pmt_entry, channels.getCurrentVPID(), channels.getCurrentAPID(), channels.getCurrentPCR(), channels.getCurrentECM(), channels.getCurrentSID(), channels.getCurrentONID(), channels.getCurrentTS(), channels.getCurrentAPID(1), channels.getCurrentAPID(2));
			}
			else if (APIDcount == 2)
				zap.zap_to(pmt_entry, channels.getCurrentVPID(), channels.getCurrentAPID(), channels.getCurrentPCR(), channels.getCurrentECM(), channels.getCurrentSID(), channels.getCurrentONID(), channels.getCurrentTS(), channels.getCurrentAPID(1), channels.getCurrentAPID(2));
			else
				zap.zap_to(pmt_entry, channels.getCurrentVPID(), channels.getCurrentAPID(), channels.getCurrentPCR(), channels.getCurrentECM(), channels.getCurrentSID(), channels.getCurrentONID(), channels.getCurrentTS(), channels.getCurrentAPID(1), channels.getCurrentAPID(2));
			
			schedule_read = false;

			eit.receiveNow(nvods[curr_nvod].SID);
			now = eit.getNow();
			next = eit.getNext();

			strcpy(audio_description, "");
			
			for (int i = 0; i < next.number_components; i++)
			{
				printf("Component_tag: %x\n", nowref.component_tag[i]);
				if (next.component_tag[i] == video_component)
				{
					printf("Video_component_type: %d\n", next.component_type[i]);
				}
				else if (now.component_tag[i] == component[apid])
				{
					strcpy(audio_description, next.audio_description[i]);
				}
			}
			
			for (int i = 0; i < now.number_components; i++)
			{
				printf("Component_tag: %x\n", nowref.component_tag[i]);
				if (now.component_tag[i] == video_component)
				{
					printf("Video_component_type: %d\n", now.component_type[i]);
				}
				else if (now.component_tag[i] == component[apid])
				{
					strcpy(audio_description, now.audio_description[i]);
				}
			}
			
			if (channels.getCurrentAPIDcount() > 1)
				osd.setLanguage(audio_description);

			osd.setNowTime(now.starttime);
			osd.setNextTime(next.starttime);
			osd.addCommand("SHOW proginfo 5");
			switchmode = 7;
			mode = 1;

			break;

		case 8:
			mode = 2;
			if (change)
			{
				osd.addCommand("SHOW epg");
				change = false;
			}
			
			do
			{
				key = rc.read_from_rc();
				if (key == RC1_MUTE)
				{
					if (hardware.isMuted())
					{
						osd.setMute(false);
						hardware.switch_mute();
					}
					else
					{
						osd.setMute(true);
						hardware.switch_mute();
					}
				}
				else if (key == RC1_DOWN)
				{
					osd.addCommand("SHOW epg_next_page");
				}
				else if (key == RC1_UP)
				{
					osd.addCommand("SHOW epg_prev_page");
				}
				number = rc.get_number();
			} while(key != RC1_RIGHT && key != RC1_LEFT && key != RC1_YELLOW && key != RC1_HOME && key != RC1_OK && key != RC1_STANDBY);
			if (key == RC1_RIGHT)
			{
				if (currentepg != 2)
				{
					osd.setEPGEventName(next.event_name);
					osd.setEPGEventShortText(next.event_short_text);
					osd.setEPGEventExtendedText(next.event_extended_text);
					osd.setEPGstarttime(next.starttime);
					osd.setEPGduration(next.duration);
					currentepg = 2;
					change = true;
					positionepg = 0;
				}
				mode = 8;
			}
			else if (key == RC1_LEFT)
			{
				if (currentepg != 1)
				{
					osd.setEPGEventName(now.event_name);
					osd.setEPGEventShortText(now.event_short_text);
					osd.setEPGEventExtendedText(now.event_extended_text);
					osd.setEPGstarttime(now.starttime);
					osd.setEPGduration(now.duration);
					currentepg = 1;
					change = true;
					positionepg = 0;
				}
				mode = 8;
			}

			else 
				osd.addCommand("HIDE epg");
			break;
		case 9: // Main menu
			osd.createMenu();
			osd.setMenuTitle("Main Menu");

			osd.addMenuEntry(1, "About");

			osd.addMenuEntry(2, "Timer");

			osd.addMenuEntry(3, "Recording");


			osd.addMenuEntry(4, "PID");
			
			if (channels.getCurrentTXT() != 0)
				osd.addMenuEntry(5, "Teletext");
			
			osd.addMenuEntry(6, "Plug-Ins");
						
			osd.addMenuEntry(0, "Setup", 3);

			if (update.cramfsmtd)
				osd.addMenuEntry(7, "Manual Update");
			if (update.cramfsmtd)
				osd.addMenuEntry(8, "Internet Update");
			//osd.addMenuEntry(8, "Visual Setup");

			osd.addMenuEntry(9, "General Setup");
			osd.addCommand("SHOW menu");

			osd.addCommand("COMMAND menu select next");
			
		
			do
			{
				do
				{
					key = rc.read_from_rc();
					number = rc.get_number();
					if (key == RC1_DOWN)
					{
						osd.selectNextEntry();
					}
					if (key == RC1_UP)
					{
						osd.selectPrevEntry();
					}
					else if (key == RC1_MUTE)
					{
						if (hardware.isMuted())
						{
							osd.setMute(false);
							hardware.switch_mute();
						}
						else
						{
							osd.setMute(true);
							hardware.switch_mute();
						}
					}
				} while(key != RC1_OK && key != RC1_HOME && key != RC1_BLUE && key != RC1_LEFT && key != RC1_RIGHT && number == -1);
				
				if (key == RC1_OK)
				{
					number = osd.menuSelectedIndex();
				}
				
				leave = true;
				if (number == 1)
				{
					osd.addCommand("HIDE menu");
					osd.addCommand("SHOW about");
					rc.read_from_rc();
					osd.addCommand("HIDE about");
					continue;
				}
				else if (number == 2)
				{
					mode = 17;
				}
				else if (number == 3)
				{
					zap.dmx_stop();
					mode = 15;
				}
				else if (number == 4)
				{
					mode = 15;
				}
				else if (number == 5)
				{
					printf("Teletext\n");
					if (channels.getCurrentTXT() != 0)
					{
						teletext.getTXT(channels.getCurrentTXT());
					}
				}
				else if (number == 6)
				{
					mode = 16;
				}
				else if (number == 7 && update.cramfsmtd)
				{
					printf("7 pressed\n");
					update.run(UPDATE_MANUALFILES);
				}
				else if (number == 8 && update.cramfsmtd)
				{
					printf("8 pressed\n");
					update.run(UPDATE_INET);
				}
				else if (number == 9)
				{
					mode = 10;
				}
				else if (key == RC1_HOME || key == RC1_BLUE || key == RC1_LEFT || key == RC1_RIGHT)
				{
					mode = switchmode;
				}
				else
					leave = false;
			} while (!leave);
			osd.addCommand("HIDE menu");
			
			break;
		case 10: // General Setup
			osd.createMenu();
			osd.setMenuTitle("General Setup");

			osd.addMenuEntry(1, "16:9 Format", 2);
			osd.addSwitchParameter(0, "Letterbox"); // 2
			osd.addSwitchParameter(0, "Panscan"); // 1
			osd.addSwitchParameter(0, "Centercut"); // 0
			osd.setSelected(0, checker.get_16_9_mode()); // Centercut

			osd.addMenuEntry(2, "RC Repeat", 1);
			osd.setSelected(1, settings.getRCRepeat());
			
			osd.addMenuEntry(3, "Scart", 2);
			osd.addSwitchParameter(2, "FBAS"); // 1
			osd.addSwitchParameter(2, "RGB"); // 0
			if (hardware.getfblk() == OUTPUT_FBAS)
				osd.setSelected(2, 1);
			else 
				osd.setSelected(2, 0);

			osd.addMenuEntry(4, "Support old RC", 1);
			osd.setSelected(3, settings.getSupportOldRc());

			osd.addMenuEntry(5, "Switch on VCR", 1);
			osd.setSelected(4, settings.getSwitchVCR());

			osd.addMenuEntry(0, "Scan-Options", 3);

			osd.addMenuEntry(6, "Update-Channel-Scan");

			osd.addMenuEntry(7, "FULL Channel-Scan");

			osd.addMenuEntry(8, "Channel-Scan");
			
			osd.addMenuEntry(0, "Setup-Stuff", 3);
			
			osd.addMenuEntry(9, "Box-Setup");

			osd.addMenuEntry(10, "Save Settings");

			osd.addCommand("SHOW menu");

			osd.addCommand("COMMAND menu select next");
			
			leave = false;
			do
			{
				do
				{
					key = rc.read_from_rc();
					number = rc.get_number();
					if (key == RC1_DOWN)
					{
						osd.selectNextEntry();
					}
					if (key == RC1_UP)
					{
						osd.selectPrevEntry();
					}
					if (key == RC1_OK)
						number = osd.menuSelectedIndex();
					if (number == 1)
					{
						if (osd.getSelected(0) == 2)
							osd.setSelected(0, 0);
						else
							osd.setSelected(0, osd.getSelected(0) + 1);
						checker.set_16_9_mode(osd.getSelected(0));
						settings.setVideoFormat(osd.getSelected(0));
						
						osd.selectEntry(0);
					}
					else if (number == 2)
					{
						osd.setSelected(1, !osd.getSelected(1));
						settings.setRcRepeat(osd.getSelected(1));
						osd.selectEntry(1);
					}
					else if (number == 3)
					{
						if (osd.getSelected(2) == 1)
							osd.setSelected(2, 0);
						else
							osd.setSelected(2, osd.getSelected(2) + 1);
						
						if (osd.getSelected(2) == 0)
						{
							hardware.setOutputMode(OUTPUT_RGB);
							settings.setOutputFormat(OUTPUT_RGB);
						}
						else if (osd.getSelected(2) == 1)
						{
							hardware.setOutputMode(OUTPUT_FBAS);
							settings.setOutputFormat(OUTPUT_FBAS);
						}
						
						
						osd.selectEntry(2);
					}
					else if (number == 4)
					{
						osd.setSelected(3, !osd.getSelected(3));
						settings.setSupportOldRc(osd.getSelected(3));
						if (osd.getSelected(1))
						{
							settings.setRcRepeat(true);
							osd.setSelected(1, true);
							osd.drawMenuEntry(1);
						}
						osd.selectEntry(3);
					}
					else if (number == 5)
					{
						osd.setSelected(4, !osd.getSelected(4));
						settings.setSwitchVCR(osd.getSelected(4));
						osd.selectEntry(4);
					}
					else if (key == RC1_MUTE)
					{
						if (hardware.isMuted())
						{
							osd.setMute(false);
							hardware.switch_mute();
						}
						else
						{
							osd.setMute(true);
							hardware.switch_mute();
						}
					}
				} while(key != RC1_OK && key != RC1_HOME && key != RC1_RIGHT && key != RC1_LEFT && key != RC1_BLUE && number == -1);
				
				if (key == RC1_OK)
				{
					number = osd.menuSelectedIndex();
				}
				
				leave = true;

				if (key == RC1_HOME || key == RC1_BLUE)
				{
					mode = switchmode;
				}
				else if (key == RC1_RIGHT || key == RC1_LEFT)
				{
					mode = 9;
				}
				else if (number == 6)
				{
					osd.addCommand("HIDE menu");
					scan.updateChannels(&channels);
					channels.saveDVBChannels();
					osd.addCommand("SHOW menu");


				}
				else if (number == 7)
				{
					osd.addCommand("HIDE menu");
					channels = scan.scanChannels(true);
					channels.setStuff(&eit, &cam, &hardware, &osd, &zap, &tuner, &variables);
					channels.saveDVBChannels();
					osd.addCommand("SHOW menu");
				}
				else if (number == 8)
				{
					osd.addCommand("HIDE menu");
					channels = scan.scanChannels();
					channels.setStuff(&eit, &cam, &hardware, &osd, &zap, &tuner, &variables);
					channels.saveDVBChannels();
					osd.addCommand("SHOW menu");
				}
				else if (number == 9)
				{
					osd.addCommand("HIDE menu");
					mode = 13;
				}
				else if (number == 10)
				{
					settings.saveSettings();
				}
				else
					leave = false;
			} while (!leave);
			osd.addCommand("HIDE menu");
			
			break;
		case 11: // Wait for key or timeout
			printf("Wait for timeout...\n");
			act_time = time(0);
			while ((!rc.command_available()) && (time(0) - act_time < 3));
			osd.hideVolume();
			mode = switchmode;
			break;
		case 12:
			do
			{
				key = rc.read_from_rc();
				if (key == RC1_MUTE)
				{
					if (hardware.isMuted())
					{
						osd.setMute(false);
						hardware.switch_mute();
					}
					else
					{
						osd.setMute(true);
						hardware.switch_mute();
					}
				}
				else if (key == RC1_GREEN)
				{
					int selectedeventid = osd.getSelectedSchedule();
					event tmp_event = eit.getEvent(selectedeventid);
					timer.addTimer(tmp_event.starttime, 2, tmp_event.event_name, tmp_event.duration = 0, channels.getCurrentChannelNumber());
					timer.saveTimer();
				}
				else if (key == RC1_DOWN)
				{
					osd.selectNextScheduleInformation();
				}
				else if (key == RC1_UP)
				{
					osd.selectPrevScheduleInformation();
				}
				else if (key == RC1_RIGHT)
				{
					osd.nextSchedulePage();
				}
				else if (key == RC1_LEFT)
				{
					osd.prevSchedulePage();
				}
				
				number = rc.get_number();
			} while(key != RC1_OK && key != RC1_HOME && key != RC1_HELP);
			osd.addCommand("HIDE schedule");;
			mode = switchmode;
			if (key == RC1_OK)
			{
				change = true;
				currentepg = 1;
				positionepg = 0;
				mode = 8;
				
				event tmp_event;
				int selectedeventid = osd.getSelectedSchedule();
				if (selectedeventid == 0)
				{
					mode = 2;
					continue;
				}
				tmp_event = eit.getEvent(selectedeventid);
				osd.createEPG();
				osd.setEPGEventName(tmp_event.event_name);
				osd.setEPGEventShortText(tmp_event.event_short_text);
				osd.setEPGEventExtendedText(tmp_event.event_extended_text);
				osd.setEPGProgramName(channels.getCurrentServiceName());
				osd.setEPGstarttime(tmp_event.starttime);
				osd.setEPGduration(tmp_event.duration);
				
			}
			break;
		case 13: // Box Setup
			osd.createMenu();
			osd.setMenuTitle("Box Setup");

			osd.addMenuEntry(1, "IP Setup");
			osd.addMenuEntry(2, "Gateway Setup");
			osd.addMenuEntry(3, "DNS Setup");
			if (update.cramfsmtd)
			{
				osd.addMenuEntry(0, "LCARS UPDATE", 3);
				osd.addMenuEntry(8, "CramFS MTD", 2);
			
				osd.addSwitchParameter(4, "6"); // 6
				osd.addSwitchParameter(4, "5"); // 5
				osd.addSwitchParameter(4, "4"); // 4
				osd.addSwitchParameter(4, "3"); // 3
				osd.addSwitchParameter(4, "2"); // 2
				osd.addSwitchParameter(4, "1"); // 1
				osd.addSwitchParameter(4, "0"); // 0
				osd.setSelected(4, update.cramfsmtd);
				osd.addMenuEntry(9, "LCARS Update Server IP");
			}

			osd.addCommand("SHOW menu");

			osd.addCommand("COMMAND menu select next");
			
			leave = false;
			do
			{
				do
				{
					key = rc.read_from_rc();
					number = rc.get_number();
					if (key == RC1_DOWN)
					{
						osd.selectNextEntry();
					}
					if (key == RC1_UP)
					{
						osd.selectPrevEntry();
					}
					if (key == RC1_OK)
						number = osd.menuSelectedIndex();
					if (number == 8 && update.cramfsmtd)
					{
						if (osd.getSelected(4) >= 6)
							osd.setSelected(4, 0);
						else
							osd.setSelected(4, osd.getSelected(4) + 1);
						
						update.cramfsmtd = osd.getSelected(4);
						osd.selectEntry(4);
					}
					else if (key == RC1_MUTE)
					{
						if (hardware.isMuted())
						{
							osd.setMute(false);
							hardware.switch_mute();
						}
						else
						{
							osd.setMute(true);
							hardware.switch_mute();
						}
					}
				} while(key != RC1_OK && key != RC1_HOME && key != RC1_RIGHT && key != RC1_LEFT && key != RC1_BLUE && number == -1);
				
				if (key == RC1_OK)
				{
					number = osd.menuSelectedIndex();
				}
				
				leave = true;

				if (key == RC1_HOME || key == RC1_BLUE)
				{
					mode = switchmode;
				}
				else if (key == RC1_RIGHT || key == RC1_LEFT)
				{
					mode = 10;
				}
				else if (number == 1)
				{
					osd.addCommand("HIDE menu");
					mode = 14;
					ipmode = 1;

				}
				else if (number == 2)
				{
					osd.addCommand("HIDE menu");
					mode = 14;
					ipmode = 3;

				}
				else if (number == 3)
				{
					osd.addCommand("HIDE menu");
					mode = 14;
					ipmode = 4;

				}
				else if (number == 9 && update.cramfsmtd)
				{
					osd.addCommand("HIDE menu");
					mode = 14;
					ipmode = 2;

				}
				else
					leave = false;
			} while (!leave);
			osd.addCommand("HIDE menu");
			
			break;
		case 14: // IP Setup
			osd.createIP();
			if (ipmode == 1)
			{
				osd.setIPn(0, settings.getIP(0));
				osd.setIPn(1, settings.getIP(1));
				osd.setIPn(2, settings.getIP(2));
				osd.setIPn(3, settings.getIP(3));
				osd.setIPDescription("Please enter IP-address!");
			}
			else if (ipmode == 2 && update.cramfsmtd)
			{
				osd.setIPn(0, settings.getserverIP(0));
				osd.setIPn(1, settings.getserverIP(1));
				osd.setIPn(2, settings.getserverIP(2));
				osd.setIPn(3, settings.getserverIP(3));
				osd.setIPDescription("Please enter LCARS-Server IP-address!");
			}
			else if (ipmode == 3)
			{
				osd.setIPn(0, settings.getgwIP(0));
				osd.setIPn(1, settings.getgwIP(1));
				osd.setIPn(2, settings.getgwIP(2));
				osd.setIPn(3, settings.getgwIP(3));
				osd.setIPDescription("Please enter gateway's IP-address!");
			}
			else if (ipmode == 4)
			{
				osd.setIPn(0, settings.getdnsIP(0));
				osd.setIPn(1, settings.getdnsIP(1));
				osd.setIPn(2, settings.getdnsIP(2));
				osd.setIPn(3, settings.getdnsIP(3));				
				osd.setIPDescription("Please enter DNS-IP-address!");
			}
			osd.addCommand("SHOW ip");
			osd.addCommand("COMMAND ip position 0");
			
			do
			{
				key = rc.read_from_rc();
				number = rc.get_number();

				if (number != -1)
				{
					osd.setIP(number);
					osd.setIPNextPosition();
				}
				else if (key == RC1_RIGHT)
				{
					osd.setIPNextPosition();
				}
				else if (key == RC1_LEFT)
				{
					osd.setIPPrevPosition();
				}
			} while ( key != RC1_OK && key != RC1_HOME);
			mode = 13;
			if (key == RC1_OK)
			{
				if (ipmode == 1)
				{
					settings.setIP(osd.getIPPart(0), osd.getIPPart(1), osd.getIPPart(2), osd.getIPPart(3));
				}
				else if (ipmode == 2)
				{
					settings.setserverIP(osd.getIPPart(0), osd.getIPPart(1), osd.getIPPart(2), osd.getIPPart(3));
					update.ip[0] = osd.getIPPart(0);
					update.ip[1] = osd.getIPPart(1);
					update.ip[2] = osd.getIPPart(2);
					update.ip[3] = osd.getIPPart(3);
				}
				if (ipmode == 3)
				{
					settings.setgwIP(osd.getIPPart(0), osd.getIPPart(1), osd.getIPPart(2), osd.getIPPart(3));					
				}
				if (ipmode == 4)
				{
					settings.setdnsIP(osd.getIPPart(0), osd.getIPPart(1), osd.getIPPart(2), osd.getIPPart(3));
					
				}
			}
			osd.addCommand("HIDE ip");
			
			break;
		case 15: // PIDs
			osd.createMenu();
			osd.setMenuTitle("PIDs");
			
			sprintf(text, "VPID: %04x", channels.getCurrentVPID());
			osd.addMenuEntry(1, text);
			
			sprintf(text, "APID: %04x", channels.getCurrentAPID(0));
			osd.addMenuEntry(2, text);

			if (channels.getCurrentAPIDcount() > 1)
			{
				sprintf(text, "APID: %04x", channels.getCurrentAPID(1));
				osd.addMenuEntry(2, text);
			}

			sprintf(text, "SID: %04x", channels.getCurrentSID());
			osd.addMenuEntry(3, text);

			sprintf(text, "PMT: %04x", channels.getCurrentPMT());
			osd.addMenuEntry(4, text);

			sprintf(text, "TXT: %04x", channels.getCurrentTXT());
			osd.addMenuEntry(5, text);

			sprintf(text, "TS: %04x", channels.getCurrentTS());
			osd.addMenuEntry(6, text);

			sprintf(text, "ONID: %04x", channels.getCurrentONID());
			osd.addMenuEntry(7, text);

			sprintf(text, "PCR: %04x", channels.getCurrentPCR());
			osd.addMenuEntry(8, text);
			
			sprintf(text, "ECM: %04x", ECM);
			osd.addMenuEntry(9, text);

			sprintf(text, "Type: %04x", channels.getCurrentType());
			osd.addMenuEntry(10, text);
			
			sprintf(text, "CAID: %04x", settings.getCAID());
			osd.addMenuEntry(11, text);

			sprintf(text, "EMM: %04x", settings.getEMMpid(channels.getCurrentTS()));
			osd.addMenuEntry(12, text);

			osd.addCommand("SHOW menu");

			osd.addCommand("COMMAND menu select next");
			
			leave = false;
			do
			{
				do
				{
					key = rc.read_from_rc();
					number = rc.get_number();
					if (key == RC1_DOWN)
					{
						osd.selectNextEntry();
					}
					if (key == RC1_UP)
					{
						osd.selectPrevEntry();
					}
					if (key == RC1_OK)
						number = osd.menuSelectedIndex();
					else if (key == RC1_MUTE)
					{
						if (hardware.isMuted())
						{
							osd.setMute(false);
							hardware.switch_mute();
						}
						else
						{
							osd.setMute(true);
							hardware.switch_mute();
						}
					}
				} while(key != RC1_OK && key != RC1_HOME && key != RC1_RIGHT && key != RC1_LEFT && key != RC1_BLUE && number == -1);
				
				if (key == RC1_OK)
				{
					number = osd.menuSelectedIndex();
				}
				
				leave = true;

				if (key == RC1_HOME || key == RC1_BLUE)
				{
					mode = switchmode;
				}
				else if (key == RC1_RIGHT || key == RC1_LEFT)
				{
					mode = 9;
				}
				else
					leave = false;
			} while (!leave);
			osd.addCommand("HIDE menu");
			
			break;
		case 16: // Plugins
			plugins.loadPlugins();
			osd.createMenu();
			osd.setMenuTitle("Plug-Ins");
			printf ("NOP: %d\n", plugins.getNumberOfPlugins());
			for (int i = 0; i < plugins.getNumberOfPlugins(); i++)
			{
				
				osd.addMenuEntry(i + 1, plugins.getName(i));
			}
			osd.addCommand("SHOW menu");
			osd.addCommand("COMMAND menu select next");

			do
			{
				do
				{
					key = rc.read_from_rc();
					number = rc.get_number();
					if (key == RC1_DOWN)
					{
						osd.selectNextEntry();
					}
					if (key == RC1_UP)
					{
						osd.selectPrevEntry();
					}
					if (key == RC1_OK)
						number = osd.menuSelectedIndex();
					else if (key == RC1_MUTE)
					{
						if (hardware.isMuted())
						{
							osd.setMute(false);
							hardware.switch_mute();
						}
						else
						{
							osd.setMute(true);
							hardware.switch_mute();
						}
					}
				} while (key != RC1_OK && key != RC1_HOME && key != RC1_RIGHT && key != RC1_LEFT && key != RC1_BLUE && number == -1);
				if (key == RC1_OK)
				{
					number = osd.menuSelectedIndex();
				}
				
				leave = true;
				if (key == RC1_HOME || key == RC1_BLUE)
				{
					mode = switchmode;
				}
				else if (key == RC1_RIGHT || key == RC1_LEFT)
				{
					mode = 9;
				}
				else if (number <= plugins.getNumberOfPlugins() && number > 0)
				{
					teletext.stopReinsertion();
					if (plugins.getShowPig(number - 1))
					{
						pig.hide();	
						pig.set_size(plugins.getSizeX(number - 1), plugins.getSizeY(number - 1));
						pig.show();
						pig.set_xy(plugins.getPosX(number - 1), plugins.getPosY(number - 1));
					}

					plugins.setfb(fb.getHandle());
					plugins.setrc(rc.getHandle());
					printf("Handle: %d\n", rc.getHandle());
					plugins.setlcd(-1);
					plugins.setvtxtpid(channels.getCurrentTXT());
					rc.stoprc();
					plugins.startPlugin(number - 1);
					if (plugins.getShowPig(number - 1))
					{
						pig.hide();
					}
					rc.startrc();
					rc.restart();
					osd.initPalette();
					usleep(400000);
					fb.clearScreen();
					teletext.startReinsertion(channels.getCurrentTXT());
				}
				else
					leave = false;
			} while (!leave);
			osd.addCommand("HIDE menu");

			

			break;
		case 17: // Time
			osd.createMenu();
			osd.setMenuTitle("Timer");
			osd.addCommand("COMMAND menu set_size 200");

			timer.dumpTimer();

			osd.addCommand("SHOW menu");

			osd.addCommand("COMMAND menu select next");
			
			leave = false;
			do
			{
				do
				{
					key = rc.read_from_rc();
					number = rc.get_number();
					if (key == RC1_DOWN)
					{
						osd.selectNextEntry();
					}
					if (key == RC1_UP)
					{
						osd.selectPrevEntry();
					}
					if (key == RC1_OK)
						number = osd.menuSelectedIndex();
					else if (key == RC1_MUTE)
					{
						if (hardware.isMuted())
						{
							osd.setMute(false);
							hardware.switch_mute();
						}
						else
						{
							osd.setMute(true);
							hardware.switch_mute();
						}
					}
				} while(key != RC1_OK && key != RC1_HOME && key != RC1_RIGHT && key != RC1_LEFT && key != RC1_BLUE && number == -1 && key != RC1_RED);
				
				if (key == RC1_OK)
				{
					number = osd.menuSelectedIndex();
				}
				else if (key == RC1_RED)
				{
					number = osd.menuSelectedIndex();
					timer.rmTimer(timer.getDumpedChannel(number), timer.getDumpedStarttime(number));
					timer.saveTimer();
					osd.addCommand("HIDE menu");
					osd.createMenu();
					osd.setMenuTitle("Timer");
					osd.addCommand("COMMAND menu set_size 200");

					timer.dumpTimer();
					
					osd.addCommand("SHOW menu");
					osd.addCommand("COMMAND menu select next");
				}

				leave = true;

				if (key == RC1_HOME || key == RC1_BLUE)
				{
					mode = switchmode;
				}
				else if (key == RC1_RIGHT || key == RC1_LEFT)
				{
					mode = 9;
				}
				else
					leave = false;
			} while (!leave);
			osd.addCommand("HIDE menu");
			
			break;

		}

	} while (key != RC1_STANDBY);

	int fpfd = open("/dev/dbox/fp0", O_RDWR);
	int on_time = (int) ((timer.getTime() - time(0)) / 60);
	if (timer.getNumberTimer() > 0)
	{
		if (on_time < 1 && on_time > 0)
			on_time = 1;
		else
			on_time--;
	}
	else
		on_time = 0;

	printf("on time: %d\n", timer.getTime() - time(0));
	printf("on time: %d\n", on_time);
	ioctl(fpfd, FP_IOCTL_SET_WAKEUP_TIMER, &on_time);
	ioctl(fpfd, FP_IOCTL_GET_WAKEUP_TIMER, &on_time);
	printf("on time: %d\n", on_time);
	sleep(1);
	ioctl(fpfd,FP_IOCTL_POWEROFF);
	close(fpfd);
	hardware.shutdown();
}
