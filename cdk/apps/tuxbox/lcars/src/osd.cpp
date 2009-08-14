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
$Log: osd.cpp,v $
Revision 1.10.6.4  2008/08/07 20:25:30  fergy
Mostly clear of not needed lines
Added back debug messages ( just for dev. )
Enambled some disabled stuff from before
Code cleaning

Revision 1.10.6.3  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.10  2002/10/20 02:03:37  TheDOC
Some fixes and stuff

Revision 1.9  2002/06/15 02:33:03  TheDOC
some changes + bruteforce-channelscan for cable

Revision 1.8  2002/06/12 17:46:53  TheDOC
reinsertion readded

Revision 1.7  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.6  2002/05/20 20:08:12  TheDOC
some new timer and epg-stuff

Revision 1.5  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.4  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "osd.h"

int osd::start_thread()
{

	int status;

	status = pthread_create( &osdThread,
	                         NULL,
	                         start_osdqueue,
	                         (void *)this );
	return status;

}

void* osd::start_osdqueue( void * this_ptr )
{
	osd *o = (osd *) this_ptr;

	while(1)
	{
		while(o->isEmpty())
		{
			if (o->proginfo_shown && (time(0) > o->proginfo_hidetime))
			{
				o->hideProgramInfo();
				o->proginfo_hidetime = time(0) + 10000;
			}
			usleep(150);
		}
		o->executeQueue();
	}
	return 0;
}

osd::osd(settings &set, fbClass *f, variables *v) :setting(set)
{
	fb = f;
	vars = v;

	proginfo_shown = false;
	vars->setvalue("%PROGINFO_SHOWN", "false");
	proginfo_hidetime = time(0) + 10000;
	printf("OSD\n");
	//fb->setMode(720, 576, 16);
	fb->clearScreen();

	initPalette();

	for (int i = 0; i <= 17; i++)
	{
		circle[i] = 17 - (int) ((sqrtf((float)1 - (((float)i/(float)17) * ((float)i /(float) 17)))) * (float)17);
	}
	for (int i = 0; i <= 10; i++)
	{
		circlesmall[i] = 10 - (int) ((sqrtf((float)1 - (((float)i/10) * ((float)i / 10)))) * 10);
	}
	for (int i = 0; i <= 12; i++)
	{
		circlemiddle[i] = 12 - (int) ((sqrtf((float)1 - (((float)i/12) * ((float)i / 12)))) * 12);
	}
}

void osd::initPalette()
{

	fb->setFade(1, 88, 20, 228, 255, 255, 255);
	fb->setFade(2, 96, 20, 236, 255, 255, 255);
	fb->setFade(0, 1, 1, 1, 240, 240, 0);
	fb->setFade(3, 0, 0, 255, 0, 0, 0);
	fb->setFade(4, 0, 0, 255, 255, 255, 255);
	fb->setFade(5, 88, 20, 228, 240, 240, 0);
	fb->setFade(6, 88, 20, 228, 0, 240, 0);
	fb->setFade(7, 1, 1, 1, 255, 255, 255);
	fb->setFade(8, 240, 240, 0, 1, 1, 1);
	fb->setFade(9, 255, 255, 255, 255, 0, 0);
	fb->setFade(10, 240, 240, 0, 0, 0, 0);
	fb->setFade(11, 255, 0, 0, 0, 0, 0);
	fb->setFade(12, 0, 255, 0, 0, 0, 0);
}

void osd::loadSkin(std::string filename)
{
	FILE *fd;
	fd = fopen(filename.c_str(), "r");

	char line[200];
	while (fgets(line, 200, fd))
	{
		if (strncmp(line, "---", 3))
		{
			printf("Syntax error in Skin-File\n");
			exit(0);
		}

		command_list list;

		fgets(line, 200, fd);
		std::string command(line);

		do
		{
			fgets(line, 200, fd);
			std::string line_str(line);
			line_str = line_str.substr(0, line_str.length() - 1);
			if (line_str != "***")
				list.insert(list.end(), line_str);
		} while (strncmp(line, "***", 3));

		printf("Command: %s\n", command.c_str());
		if (command == "general:\n")
		{
			for (int i = 0; i < (int) list.size(); i++)
			{
				fb->runCommand(list[i]);
			}
		}
		else if (command == "proginfo_show:\n")
		{
			printf("Proginfo\n");
			setProgramCommandListShow(list);
		}
		else if (command == "proginfo_hide:\n")
		{
			printf("Proginfo2\n");
			setProgramCommandListHide(list);
		}
		else if (command == "proginfo_servicenumber:\n")
		{
			setProgramCommandListServiceNumber(list);
		}
		else if (command == "proginfo_servicename:\n")
		{
			setProgramCommandListServiceName(list);
		}
		else if (command == "proginfo_language:\n")
		{
			setProgramCommandListLanguage(list);
		}
		else if (command == "proginfo_nowtime:\n")
		{
			setProgramCommandListNowTime(list);
		}
		else if (command == "proginfo_nowdescription:\n")
		{
			setProgramCommandListNowDescription(list);
		}
		else if (command == "proginfo_nexttime:\n")
		{
			setProgramCommandListNextTime(list);
		}
		else if (command == "proginfo_nextdescription:\n")
		{
			setProgramCommandListNextDescription(list);
		}
	}


	fclose(fd);
}

void osd::addCommand(std::string command)
{
	command_queue.push(command);
	printf("Command: %s\n", command.c_str());
}

void osd::executeQueue()
{
	while(!isEmpty())
	{
		executeCommand();
	}
}

void osd::executeCommand()
{
	std::string command_from_queue = command_queue.front();
	std::string command;

	command_queue.pop();

	std::istringstream iss(command_from_queue);
	std::getline(iss, command, ' ');

	if(command == "CLEARSCREEN")
	{
		clearScreen();
	}
	else if (command == "CREATE")
	{
		std::string command2;
		std::getline(iss, command2, ' ');
		if (command2 == "list")
		{
			createList();
		}
		else if (command2 == "proginfo")
		{
			createProgramInfo();
		}
		else if (command2 == "epg")
		{
			createEPG();
		}
		else if (command2 == "number")
		{
			createNumberEntry();
		}
		else if (command2 == "perspective")
		{
			createPerspective();
		}
		else if (command2 == "menu")
		{
			createMenu();
		}
		else if (command2 == "scan")
		{
			createScan();
		}
		else if (command2 == "schedule")
		{
			createSchedule();
		}
		else if (command2 == "ip")
		{
			createIP();
		}
	}
	else if (command == "COMMAND")
	{
		std::string command2;
		std::getline(iss, command2, ' ');
		std::string parms[30];
		int parm_count = 0;

		while(std::getline(iss, parms[parm_count++], ' '));

		if (command2 == "list")
		{
			if (parms[0] == "add_item")
			{
				addListItem(atoi(parms[1].c_str()), parms[2]);
			}
			else if (parms[0] == "select_item")
			{
				selectItem(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "select_next_item")
			{
				selectNextItem();
			}
		}
		else if (command2 == "proginfo")
		{
			if (parms[0] == "set_service_name")
			{
				std::string name;
				std::istringstream iss(command_from_queue);
				std::getline(iss, name, ' ');
				std::getline(iss, name, ' ');
				std::getline(iss, name, ' ');
				std::getline(iss, name);

				setServiceName(name);
			}
			else if (parms[0] == "set_language_name")
			{
				setLanguage(parms[1]);
			}
			else if (parms[0] == "set_service_number")
			{
				setServiceNumber(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "set_channels_available")
			{
				if (parms[1] == "true")
					setChannelsAvailable(true);
				else
					setChannelsAvailable(false);
			}
			else if (parms[0] == "set_perspective_available")
			{
				if (parms[1] == "true")
					setPerspectiveAvailable(true);
				else
					setPerspectiveAvailable(false);
			}
			else if (parms[0] == "set_epg_available")
			{
				if (parms[1] == "true")
					setEPGAvailable(true);
				else
					setEPGAvailable(false);
			}
			else if (parms[0] == "set_menu_available")
			{
				if (parms[1] == "true")
					setMenuAvailable(true);
				else
					setMenuAvailable(false);
			}
			else if (parms[0] == "set_teletext")
			{
				if (parms[1] == "true")
					setTeletext(true);
				else
					setTeletext(false);
			}
			else if (parms[0] == "set_now_starttime")
			{
				setNowTime(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "set_now_description")
			{
				setNowDescription(parms[1]);
			}
			else if (parms[0] == "set_next_starttime")
			{
				setNextTime(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "set_next_description")
			{
				setNextDescription(parms[1]);
			}
			else if (parms[0] == "set_parental_rating")
			{
				setParentalRating(atoi(parms[1].c_str()));
			}
		}
		else if (command2 == "epg")
		{
			if (parms[0] == "set_event_name")
			{
				setEPGEventName(parms[1]);
			}
			else if (parms[0] == "set_short_text")
			{
				setEPGEventShortText(parms[1]);
			}
			else if (parms[0] == "set_extended_text")
			{
				setEPGEventExtendedText(parms[1]);
			}
			else if (parms[0] == "set_description")
			{
				setEPGDescription(parms[1]);
			}
			else if (parms[0] == "set_service_name")
			{
				setEPGProgramName(parms[1]);
			}
			else if (parms[0] == "set_start_time")
			{
				setEPGstarttime(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "set_duration")
			{
				setEPGduration(atoi(parms[1].c_str()));
			}
		}
		else if (command2 == "number")
		{
			if (parms[0] == "number")
			{
				setNumberEntry(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "add")
			{
				addNumberEntry(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "text")
			{
				if (!vars->isavailable(parms[1]))
					setNumberText(parms[1]);
				else
					setNumberText(vars->getvalue(parms[1]));
			}
		}
		else if (command2 == "perspective")
		{
			if (parms[0] == "name")
			{
				std::string new_string;
				for (int i = 1; i < parm_count; i++)
				{
					if (!vars->isavailable(parms[i]))
						new_string.append(parms[i]);
					else
						new_string.append(vars->getvalue(parms[i]));
					new_string.append(" ");
				}

				setPerspectiveName(new_string);
			}
		}
		else if (command2 == "menu")
		{
			if (parms[0] == "add_entry")
			{
				addMenuEntry(atoi(parms[1].c_str()), parms[2], atoi(parms[3].c_str()));
			}
			else if (parms[0] == "add_switch_param")
			{
				addSwitchParameter(atoi(parms[1].c_str()), parms[2]);
			}
			else if (parms[0] == "set_size")
			{
				setMenuSize(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "set_selected")
			{
				setSelected(atoi(parms[1].c_str()), atoi(parms[2].c_str()));
			}
			else if (parms[0] == "menu_title")
			{
				setMenuTitle(parms[1]);
			}
			else if (parms[0] == "draw_entry")
			{
				if (parms[2] == "true")
					drawMenuEntry(atoi(parms[1].c_str()), true);
				else
					drawMenuEntry(atoi(parms[1].c_str()), false);
			}
			else if (parms[0] == "select")
			{
				if (parms[1] == "next")
					selectNextEntry();
				else if (parms[1] == "prev")
					selectPrevEntry();
				else
					selectEntry(atoi(parms[1].c_str()));
			}
		}
		else if (command2 == "vol")
		{
			if (parms[0] == "set")
			{
				setVolume(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "mute")
			{
				if (parms[1] == "true")
					setMute(true);
				else
					setMute(false);
			}
		}
		else if (command2 == "scan")
		{
			if (parms[0] == "progress")
			{
				setScanProgress(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "number_channels")
			{
				setScanChannelNumber(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "number_TS")
			{
				setScanTSNumber(atoi(parms[1].c_str()));
			}
		}
		else if (command2 == "schedule")
		{
			if (parms[0] == "add_information")
			{
			}
			else if (parms[0] == "select")
			{
				if (parms[1] == "next")
					selectNextScheduleInformation();
				else if (parms[1] == "prev")
					selectPrevScheduleInformation();
				else
				{
					if (parms[2] == "true")
						selectScheduleInformation(atoi(parms[1].c_str()), true);
					else
						selectScheduleInformation(atoi(parms[1].c_str()), false);
				}
			}
			else if (parms[0] == "page")
			{
				if (parms[1] == "next")
					nextSchedulePage();
				else if (parms[1] == "prev")
					prevSchedulePage();
			}
		}
		else if (command2 == "ip")
		{
			if (parms[0] == "set")
			{
				setIP(atoi(parms[1].c_str()));
			}
			else if (parms[0] == "draw_position")
			{
				drawIPPosition(atoi(parms[1].c_str()), atoi(parms[2].c_str()));
			}
			else if (parms[0] == "description")
			{
				setIPDescription(parms[1]);
			}
			else if (parms[0] == "position")
			{
				if (parms[1] == "next")
					setIPNextPosition();
				else if (parms[1] == "prev")
					setIPPrevPosition();
				else
					setIPPosition(atoi(parms[1].c_str()));
			}
		}
	}
	else if (command == "SHOW")
	{
		std::string command2;
		std::getline(iss, command2, ' ');

		if (proginfo_shown && (command2 != "proginfo"))
		{
			hideProgramInfo();
		}

		if (command2 == "list")
		{
			showList();
		}
		else if (command2 == "proginfo")
		{
			std::string duration;
			std::getline(iss, duration, ' ');
			showProgramInfo();
			proginfo_hidetime = time(0) + atoi(duration.c_str());
		}
		else if (command2 == "epg")
		{
			showEPG();
		}
		else if (command2 == "number")
		{
			showNumberEntry();
		}
		else if (command2 == "perspective")
		{
			showPerspective();
		}
		else if (command2 == "menu")
		{
			showMenu();
		}
		else if (command2 == "vol")
		{
			showVolume();
		}
		else if (command2 == "scan")
		{
			showScan();
			//printf("+-+-+-+-+-+ Aufruf Channelscan\n");
		}
		else if (command2 == "schedule")
		{
			std::string page;
			std::getline(iss, page, ' ');
			showSchedule(atoi(page.c_str()));
		}
		else if (command2 == "ip")
		{
			showIP();
		}
		else if (command2 == "about")
		{
			showAbout();
		}

	}
	else if (command == "HIDE")
	{
		std::string command2;
		std::getline(iss, command2, ' ');
		if (command2 == "list")
		{
			hideList();
		}
		else if (command2 == "proginfo")
		{
			hideProgramInfo();
		}
		else if (command2 == "epg")
		{
			hideEPG();
		}
		else if (command2 == "number")
		{
			hideNumberEntry();
		}
		else if (command2 == "perspective")
		{
			hidePerspective();
		}
		else if (command2 == "menu")
		{
			hideMenu();
		}
		else if (command2 == "vol")
		{
			hideVolume();
		}
		else if (command2 == "scan")
		{
			hideScan();
		}
		else if (command2 == "schedule")
		{
			hideSchedule();
		}
		else if (command2 == "ip")
		{
			hideIP();
		}
		else if (command2 == "about")
		{
			hideAbout();
		}
	}
	else
	{
		printf("--------->UNKNOWN OSD-COMMAND<----------\n");
	}

}

void osd::clearScreen()
{
	fb->clearScreen();
}

void osd::createList()
{


	selected = 0;
	numberItems = 0;

}

int osd::numberPossibleListItems()
{
	return 10;
}

void osd::addListItem(int index, std::string  name)
{
	list[numberItems].index = index;
	list[numberItems].name = name;
	numberItems++;
}

void osd::selectNextItem()
{
}

void osd::selectItem(int number)
{
	char buffer[100];
	fb->setTextSize(0.4);
	for (int j = 0; j <= 17; j++)
	{
		fb->fillBox(33 + circle[17 - j], 85 + selected * 40 + j, 50, 85 + selected * 40 + j + 1, 1);
	}
	for (int j = 18; j < 35; j++)
	{
		fb->fillBox(33 + circle[j - 18], 85 + selected * 40 + j, 50, 85 + selected * 40 + j + 1, 1);
	}
	fb->fillBox(33, 103 + selected * 40, 50, 104 + selected * 40, 1);
	fb->fillBox(50, 85 + selected * 40, 70, 120 + selected * 40, 1);
	fb->fillBox(300, 85 + selected * 40, 320, 120 + selected * 40, 1);
	fb->fillBox(70, 85 + selected * 40, 300, 120 + selected * 40, 0);
	strcpy(buffer, list[selected].name.c_str());
	fb->putText(75, 97 + selected * 40, 0, buffer, 225);
	selected = number;
	for (int j = 0; j <= 17; j++)
	{
		fb->fillBox(33 + circle[17 - j], 85 + selected * 40 + j, 50, 85 + selected * 40 + j + 1, 0);
	}
	for (int j = 18; j < 35; j++)
	{
		fb->fillBox(33 + circle[j - 18], 85 + selected * 40 + j, 50, 85 + selected * 40 + j + 1, 0);
	}
	fb->fillBox(33, 103 + selected * 40, 50, 104 + selected * 40, 0);
	fb->fillBox(50, 85 + selected * 40, 70, 120 + selected * 40, 0);
	fb->fillBox(300, 85 + selected * 40, 320, 120 + selected * 40, 0);
	fb->fillBox(70, 85 + selected * 40, 300, 120 + selected * 40, 1);
	strcpy(buffer, list[selected].name.c_str());
	fb->putText(75, 97 + selected * 40, 1, buffer, 225);
}

int osd::selectedItem()
{
	return selected;
}

void osd::showList()
{
	char buffer[100];

	fb->setTextSize(0.4);
	for (int i = 0; i < numberItems; i++)
	{
		for (int j = 0; j <= 17; j++)
		{
			fb->fillBox(33 + circle[17 - j], 85 + i * 40 + j, 50, 85 + i * 40 + j + 1, 1);
		}
		for (int j = 18; j < 35; j++)
		{
			fb->fillBox(33 + circle[j - 18], 85 + i * 40 + j, 50, 85 + i * 40 + j + 1, 1);
		}
		fb->fillBox(33, 102 + selected * 40, 50, 106 + selected * 40, 1);
		fb->fillBox(50, 85 + i * 40, 70, 120 + i * 40, 1);
		fb->fillBox(300, 85 + i * 40, 320, 120 + i * 40, 1);
		fb->fillBox(323, 85 + i * 40, 330, 120 + i * 40, 2);
		fb->fillBox(70, 85 + i * 40, 300, 120 + i * 40, 0);
		strcpy(buffer, list[i].name.c_str());
		fb->putText(75, 97 + i * 40, 0, buffer, 225);
	}
}

void osd::hideList()
{
	fb->fillBox(30, 85, 330, 120 + 9 * 40, -1);
}


void osd::createProgramInfo()
{
	serviceName = "";
	serviceNumber = 0;
	channelsAvailable = true;
	perspectiveAvailable = false;
	epgAvailable = true;
	menuAvailable = true;
	teletext = false;
	nowTime = 0;
	nextTime = 0;
	nowDescription = "";
	nextDescription = "";
	language[0] = '\0';
	par_rating = 0;
}

void osd::setServiceName(std::string  name)
{
	serviceName = name;
	vars->setvalue("%SERVICENAME%", name);
	vars->addEvent("OSD_PROGINFO_SERVICENAME");

	if (proginfo_shown)
	{
		fb->setTextSize(0.4);
		if (prog_com_list_servicename.size() < 1)
		{
			fb->fillBox(130, 30, 330, 50, 0);
			fb->putText(135, 32, 0, serviceName, 195);
		}
		else
		{
			for (int i = 0; i < (int) prog_com_list_servicename.size(); i++)
			{
				fb->runCommand(prog_com_list_servicename[i]);
			}
		}
	}
}

void osd::setServiceNumber(int number)
{
	serviceNumber = number;
	vars->setvalue("%SERVICENUMBER%", number);
	vars->addEvent("OSD_PROGINFO_SERVICENUMBER");

	if (proginfo_shown)
	{
		fb->setTextSize(0.4);
		if (prog_com_list_servicenumber.size() < 1)
		{
			fb->fillBox(75, 30, 120, 50, 0);
			fb->putText(77, 32, 0, serviceNumber, 43);
		}
		else
		{
			for (int i = 0; i < (int) prog_com_list_servicenumber.size(); i++)
			{
				fb->runCommand(prog_com_list_servicenumber[i]);
			}
		}
	}
}

void osd::setChannelsAvailable(bool available)
{
	channelsAvailable = available;
}

void osd::setTeletext(bool available)
{
	teletext = available;
	vars->addEvent("OSD_PROGINFO_TELETEXT");

	if (proginfo_shown)
	{
		fb->setTextSize(0.4);
		if (teletext)
		{
			fb->fillBox(80, 401, 120, 419, 0);
			fb->putText(82, 402, 0, "TXT");
		}
		else
			fb->fillBox(80, 401, 120, 419, 1);
	}
}

void osd::setPerspectiveAvailable(bool available)
{
	perspectiveAvailable = available;
	vars->addEvent("OSD_PROGINFO_PERSPECTIVE");

	if (vars->getvalue("%SHOWHELP") == "true")
	{
		if (proginfo_shown)
		{
			if (perspectiveAvailable)
			{
				fb->setTextSize(0.34);
				fb->fillBox(300, 383, 385, 400, 7);
				fb->fillBox(283, 383, 300, 400, 12);
				fb->putText(302, 385, 7, "Perspective");
			}
			else if (NVODAvailable = false)
				fb->fillBox(283, 383, 385, 400, -1);
		}
	}
}

void osd::setNVODAvailable(bool available)
{
	NVODAvailable = available;

	if (vars->getvalue("%SHOWHELP") == "true")
	{
		if (proginfo_shown)
		{
			if (perspectiveAvailable)
			{
				fb->setTextSize(0.34);
				fb->fillBox(300, 383, 385, 400, 7);
				fb->fillBox(283, 383, 300, 400, 12);
				fb->putText(302, 385, 7, "NVOD");
			}
			else
				fb->fillBox(283, 383, 385, 400, -1);
		}
	}
}

void osd::setEPGAvailable(bool available)
{
	epgAvailable = available;
}

void osd::setMenuAvailable(bool available)
{
	menuAvailable = available;
}

void osd::setNowTime(time_t starttime)
{
	nowTime = starttime;
	char nowtime[10];
	struct tm *t;
	t = localtime(&nowTime);
	strftime(nowtime, sizeof nowtime, "%H:%M", t);
	vars->setvalue("%NOWTIME%", nowtime);

	vars->addEvent("OSD_PROGINFO_NOWTIME");

	if (proginfo_shown)
	{
		fb->setTextSize(0.4);
		if (prog_com_list_nowtime.size() < 1)
		{
			fb->fillBox(65, 425, 125, 445, 0);
			fb->putText(70, 427, 0, nowtime); // Uhrzeit des laufenden Programms
		}
		else
		{
			for (int i = 0; i < (int) prog_com_list_nowtime.size(); i++)
			{
				fb->runCommand(prog_com_list_nowtime[i]);
			}
		}
	}
}

void osd::setNowDescription(std::string  description)
{
	nowDescription = description;
	vars->setvalue("%NOWDESCRIPTION%", nowDescription);
	vars->addEvent("OSD_PROGINFO_NOWDESCRIPTION");

	if (proginfo_shown)
	{
		fb->setTextSize(0.4);
		if (prog_com_list_nowdescription.size() < 1)
		{
			fb->fillBox(149, 425, 650, 445, 0);
			fb->putText(150, 427, 0, nowDescription, 490);
		}
		else
		{
			for (int i = 0; i < (int) prog_com_list_nowdescription.size(); i++)
			{
				fb->runCommand(prog_com_list_nowdescription[i]);
			}
		}
	}
}

void osd::setNextTime(time_t starttime)
{
	nextTime = starttime;
	char nexttime[10];
	struct tm *t;
	t = localtime(&nextTime);
	strftime(nexttime, sizeof nexttime, "%H:%M", t);
	vars->setvalue("%NEXTTIME%", nexttime);
	vars->addEvent("OSD_PROGINFO_NEXTTIME");


	if (proginfo_shown)
	{
		fb->setTextSize(0.4);
		if (prog_com_list_nexttime.size() < 1)
		{
			fb->fillBox(65, 495, 125, 515, 0);
			fb->putText(70, 497, 0, nexttime);
		}
		else
		{
			for (int i = 0; i < (int) prog_com_list_nexttime.size(); i++)
			{
				fb->runCommand(prog_com_list_nexttime[i]);
			}
		}
	}
}

void osd::setNextDescription(std::string  description)
{
	nextDescription = description;
	vars->setvalue("%NEXTDESCRIPTION%", nextDescription);
	vars->addEvent("OSD_PROGINFO_NEXTDESCRIPTION");


	if (proginfo_shown)
	{
		fb->setTextSize(0.4);
		if (prog_com_list_nextdescription.size() < 1)
		{
			fb->fillBox(149, 495, 650, 515, 0);
			fb->putText(150, 497, 0, nextDescription, 490);
		}
		else
		{
			for (int i = 0; i < (int) prog_com_list_nextdescription.size(); i++)
			{
				fb->runCommand(prog_com_list_nextdescription[i]);
			}
		}
	}
}

void osd::setLanguage(std::string language_name)
{
	strcpy(language, language_name.c_str());
	vars->setvalue("%LANGUAGE%", language);
	vars->addEvent("OSD_PROGINFO_LANGUAGE");

	if (proginfo_shown)
	{
		fb->setTextSize(0.4);
		if (prog_com_list_language.size() < 1)
		{
			if (language_name != "")
			{
				fb->fillBox(160, 401, 300, 419, 0);
				fb->putText(165, 402, 0, language);
			}
			else
				fb->fillBox(160, 401, 300, 419, 1);
		}
		else
		{
			for (int i = 0; i < (int) prog_com_list_language.size(); i++)
			{
				fb->runCommand(prog_com_list_language[i]);
			}
		}
	}
}

void osd::setParentalRating(int rating)
{
	par_rating = rating;
	vars->setvalue("%PARENTALRATING%", rating);
	vars->addEvent("OSD_PROGINFO_PARENTALRATING");

	if (proginfo_shown)
	{
		if (par_rating != 0)
		{
			fb->fillBox(310, 401, 390, 419, 0);
			char rattext[10];
			sprintf(rattext, "FSK: %d", par_rating + 3);
			fb->setTextSize(0.4);
			fb->putText(315, 402, 0, rattext);
		}
		else
			fb->fillBox(310, 401, 390, 419, 1);
	}
}

void osd::showProgramInfo()
{
	if (proginfo_shown == true)
		return;
	proginfo_shown = true;
	vars->setvalue("%PROGINFO_SHOWN", "true");

	char nowtime[10], nexttime[10], acttime[10];
	char number[10];
	time_t act_time = time(0);
	struct tm *t;
	t = localtime(&act_time);
	strftime(acttime, sizeof acttime, "%H:%M", t);
	t = localtime(&nowTime);
	strftime(nowtime, sizeof nowtime, "%H:%M", t);
	t = localtime(&nextTime);
	strftime(nexttime, sizeof nexttime, "%H:%M", t);

	vars->setvalue("%SERVICENAME%", serviceName);
	vars->setvalue("%SERVICENUMBER%", serviceNumber);
	vars->setvalue("%NOWTIME%", nowtime);
	vars->setvalue("%NEXTTIME%", nexttime);
	vars->setvalue("%ACTTIME%", acttime);
	vars->setvalue("%NOWDESCRIPTION%", nowDescription);
	vars->setvalue("%NEXTDESCRIPTION%", nextDescription);
	vars->setvalue("%LANGUAGE%", language);

	vars->addEvent("OSD_PROGINFO_SHOW");

	if (prog_com_list_show.size() < 1)
	{
		printf("%d - %s\n", serviceNumber, serviceName.c_str());
		printf("Now (%d): %s\n", (int)nowTime, nowDescription.c_str());
		printf("Next (%d): %s\n", (int)nextTime, nextDescription.c_str());

		char pname[30], now[200], next[200];

		fb->fillBox(65, 420, 650, 550, 0);
		for (int j = 0; j <= 10; j++)
		{
			fb->fillBox(60 + circlesmall[10 - j], 30 + j, 70, 30 + j + 1, 1);
			fb->fillBox(60 + circlesmall[10 - j], 400 + j, 70, 400 + j + 1, 1);
			fb->fillBox(650, 400 + j, 660 - circlesmall[10 - j], 400 + j + 1, 1);
			fb->fillBox(650 - circlesmall[10 - j], 420 + j, 650, 420 + j + 1, 1);
			fb->fillBox(130 - circlesmall[10 - j], 490 + j, 130, 490 + j + 1, 1);
			fb->fillBox(130 - circlesmall[10 - j], 420 + j, 130, 420 + j + 1, 1);
			fb->fillBox(650 - circlesmall[10 - j], 490 + j, 650, 490 + j + 1, 1);
			fb->fillBox(650 - circlesmall[j], 470 + j, 650, 470 + j + 1, 1);
			fb->fillBox(130 - circlesmall[j], 470 + j, 130, 470 + j + 1, 1);
			fb->fillBox(140, 470 + j, 140 + circlesmall[j], 470 + j + 1, 1);
			fb->fillBox(140, 490 + j, 140 + circlesmall[10 - j], 490 + j + 1, 1);
			fb->fillBox(140, 420 + j, 140 + circlesmall[10 - j], 420 + j + 1, 1);
		}
		for (int j = 11; j < 20; j++)
		{
			fb->fillBox(60 + circlesmall[j - 11], 30 + j, 70, 30 + j + 1, 1);
			fb->fillBox(60 + circlesmall[j - 11], 400 + j, 70, 400 + j + 1, 1);

		}
		fb->fillBox(70, 30, 75, 50, 1);
		fb->fillBox(75, 30, 120, 50, 0);
		fb->fillBox(120, 30, 130, 50, 1);
		fb->fillBox(130, 30, 330, 50, 0);
		fb->fillBox(330, 30, 550, 50, 1);
		fb->fillBox(550, 30, 630, 50, 0);
		fb->fillBox(630, 30, 670, 50, 1);

		fb->fillBox(70, 400, 650, 420, 1);
		fb->fillBox(650, 410, 660, 550, 1);
		fb->fillBox(70, 480, 660, 490, 1);
		fb->fillBox(130, 420, 140, 550, 1);

		fb->setTextSize(0.4);

		strcpy(pname, serviceName.c_str());
		fb->putText(135, 32, 0, pname, 195);

		sprintf(number, "%d", serviceNumber);
		fb->putText(77, 32, 0, number, 43);

		fb->putText(560, 32, 0, acttime);

		fb->putText(70, 427, 0, nowtime);
		fb->putText(70, 497, 0, nexttime);

		strcpy(now, nowDescription.c_str());
		strcpy(next, nextDescription.c_str());

		if (strlen(language) > 0)
		{
			fb->fillBox(160, 401, 300, 419, 0);
			fb->putText(165, 402, 0, language);
		}

		if (teletext)
		{
			fb->fillBox(80, 401, 120, 419, 0);
			fb->putText(82, 402, 0, "TXT");
		}

		if (par_rating != 0)
		{
			fb->fillBox(310, 401, 390, 419, 0);
			char rattext[10];
			sprintf(rattext, "FSK: %d", par_rating + 3);
			fb->putText(315, 402, 0, rattext);
		}


		fb->putText(150, 427, 0, now, 490);

		fb->putText(150, 497, 0, next, 490);

		if (vars->getvalue("%SHOWHELP") == "true")
		{

			fb->setTextSize(0.34);
			if (channelsAvailable)
			{
				fb->fillBox(200, 383, 285, 400, 7);
				fb->fillBox(183, 383, 200, 400, 11);
				fb->putText(202, 385, 7, "Channels");
			}

			if (perspectiveAvailable)
			{
				fb->fillBox(300, 383, 385, 400, 7);
				fb->fillBox(283, 383, 300, 400, 12);
				fb->putText(302, 385, 7, "Perspective");
			}

			if (NVODAvailable)
			{
				fb->fillBox(300, 383, 385, 400, 7);
				fb->fillBox(283, 383, 300, 400, 12);
				fb->putText(302, 385, 7, "NVOD");
			}

			if (epgAvailable)
			{
				fb->fillBox(400, 383, 485, 400, 7);
				fb->fillBox(383, 383, 400, 400, 10);
				fb->putText(402, 385, 7, "EPG Info");
			}

			if (menuAvailable)
			{
				fb->fillBox(500, 383, 585, 400, 7);
				fb->fillBox(483, 383, 500, 400, 3);
				fb->putText(502, 385, 7, "Menu");
			}
		}

		char text[100];
		sprintf(text, "CAID: %x", setting.getCAID());
	}
	else
	{
		for (int i = 0; i < (int) prog_com_list_show.size(); i++)
		{
			fb->runCommand(prog_com_list_show[i]);
		}
	}

}

void osd::hideProgramInfo()
{
	proginfo_shown = false;
	vars->addEvent("OSD_PROGINFO_HIDE");
	vars->setvalue("%PROGINFO_SHOWN", "false");

	if (prog_com_list_hide.size() < 1)
	{
		printf("Hide program info\n");
		fb->fillBox(30, 28, 670, 52, -1);
		fb->fillBox(30, 380, 719, 575, -1);
	}
	else
	{
		for (int i = 0; i < (int) prog_com_list_hide.size(); i++)
		{
			fb->runCommand(prog_com_list_hide[i]);
		}
	}
}

void osd::createNumberEntry()
{
	number = -1;
	numberText = "";

}

void osd::setNumberEntry(int setnumber)
{
	number = setnumber;
	vars->setvalue("%NUMBERENTRY", number);
}

void osd::addNumberEntry(int setnumber)
{
	if (number == -1)
		number = 0;
	number = number * 10 + setnumber;
	vars->setvalue("%NUMBERENTRY", number);
}

void osd::setNumberText(std::string  text)
{
	numberText = text;
}

void osd::showNumberEntry()
{
	char buffer[100];

	int tmp_number = atoi(vars->getvalue("%NUMBERENTRY").c_str());
	if (tmp_number != number)
		number = tmp_number;

	fb->setTextSize(0.7);
	for (int j = 0; j <= 17; j++)
	{
		fb->fillBox(33 + circle[17 - j], 30 + j, 50, 30 + j + 1, 1);
		fb->fillBox(320, 30 + j, 338 - circle[17 - j], 30 + j + 1, 1);
	}
	for (int j = 18; j < 35; j++)
	{
		fb->fillBox(33 + circle[j - 18], 30 + j, 50, 30 + j + 1, 1);
		fb->fillBox(320, 30 + j, 338 - circle[j - 18], 30 + j + 1, 1);
	}
	for (int j = 0; j <= 10; j++)
	{
		fb->fillBox(33 + circlesmall[10 - j], 75 + j, 43, 75 + j + 1, 1);
		fb->fillBox(320, 75 + j, 330 - circlesmall[10 - j], 75 + j + 1, 1);

	}
	for (int j = 11; j < 20; j++)
	{
		fb->fillBox(33 + circlesmall[j - 11], 75 + j, 43, 75 + j + 1, 1);
		fb->fillBox(320, 75 + j, 330 - circlesmall[j - 11], 75 + j + 1, 1);

	}
	fb->fillBox(50, 30, 75, 65, 1);
	fb->fillBox(75, 30, 125, 65, 0);
	fb->fillBox(125, 30, 135, 65, 1);
	fb->fillBox(135, 30, 185, 65, 0);
	fb->fillBox(185, 30, 195, 65, 1);
	fb->fillBox(195, 30, 245, 65, 0);
	fb->fillBox(245, 30, 255, 65, 1);
	fb->fillBox(255, 30, 305, 65, 0);
	fb->fillBox(305, 30, 320, 65, 1);

	fb->fillBox(43, 75, 320, 95, 0);

	if (number > 999)
	{
		sprintf(buffer, "%d", (int) (number / 1000) % 10);
		fb->putText(90, 35, 0, buffer);
	}
	if (number > 99)
	{
		sprintf(buffer, "%d", (int) (number / 100) % 10);
		fb->putText(150, 35, 0, buffer);
	}
	if (number > 9)
	{
		sprintf(buffer, "%d", (int) (number / 10) % 10);
		fb->putText(210, 35, 0, buffer);
	}
	if (number >= 0)
	{
		sprintf(buffer, "%d", (int) (number / 1) % 10);
		fb->putText(270, 35, 0, buffer);
	}

	fb->setTextSize(0.4);
	strcpy(buffer, numberText.c_str());
	fb->putText(45, 80, 0, buffer, 275);
}

void osd::hideNumberEntry()
{
	printf("Hide number entry\n");
	fb->fillBox(33, 30, 350, 100, -1);
}

void osd::createPerspective()
{
	perspective_name = "";
}

void osd::setPerspectiveName(std::string  name)
{
	perspective_name = name;
}

void osd::showPerspective()
{
	for (int j = 0; j <= 10; j++)
	{
		fb->fillBox(150 + circlesmall[10 - j], 500 + j, 160, 500 + j + 1, 1);
		fb->fillBox(560, 500 + j, 570 - circlesmall[10 - j], 500 + j + 1, 1);

	}
	for (int j = 11; j < 20; j++)
	{
		fb->fillBox(150 + circlesmall[j - 11], 500 + j, 160, 500 + j + 1, 1);
		fb->fillBox(560, 500 + j, 570 - circlesmall[j - 11], 500 + j + 1, 1);

	}
	fb->fillBox(160, 500, 170, 520, 1);
	fb->fillBox(170, 500, 550, 520, 0);
	fb->fillBox(550, 500, 560, 520, 1);

	char pname[400];
	strcpy(pname, perspective_name.c_str());
	fb->setTextSize(0.4);
	fb->runCommand("SETTEXTSIZE 0.4");
	std::stringstream ostr;
	ostr << "PUTTEXT 175 504 0 370 0 " << perspective_name << std::ends;
	std::string tmp = ostr.str();
	fb->runCommand(tmp.c_str());
	fb->putText(175, 504, 0, perspective_name.c_str(), 370);
}

void osd::hidePerspective()
{
	fb->fillBox(150, 500, 580, 520, -1);
}


void osd::createEPG()
{
	event_name = "";
	event_short_text = "";
	event_extended_text = "";
	description = "";
	linkage = 0;
	audio = "";
	FSK = 0;
}

void osd::setEPGEventName(std::string  input)
{
	event_name = input;
}

void osd::setEPGEventShortText(std::string  input)
{
	event_short_text = input;
}

void osd::setEPGEventExtendedText(std::string  input)
{
	event_extended_text = input;
}

void osd::setEPGDescription(std::string  input)
{
	description = input;
}

void osd::setEPGProgramName(std::string  input)
{
	programname = input;
}

void osd::setEPGstarttime(time_t input)
{
	starttime = input;
}

void osd::setEPGduration(int input)
{
	duration = input;
}

void osd::setEPGlinkage(int input)
{
	linkage = input;
}

void osd::setEPGaudio(std::string input)
{
	audio = input;
}

void osd::setEPGfsk(int input)
{
	FSK = input;
}

void osd::showEPG()
{
	char text[1000];

	for (int j = 0; j <= 12; j++)
	{
		fb->fillBox(88 + circlemiddle[12 - j], 45 + j, 100, 45 +  j + 1, 5);
		fb->fillBox(88 + circlemiddle[j], 57 + j, 100, 57 +  j + 1, 5);
		fb->fillBox(600, 45 + j, 612 - circlemiddle[12 - j], 45 +  j + 1, 5);

	}
	fb->fillBox(100, 45, 120, 69, 5);
	fb->fillBox(120, 45, 480, 69, 7);
	fb->fillBox(480, 45, 490, 69, 5);
	fb->fillBox(490, 45, 560, 69, 7);
	fb->fillBox(560, 45, 600, 69, 5);
	fb->setTextSize(0.45);
	fb->putText(125, 50, 7, event_name, 350);
	time_t act_time = time(0);
	struct tm *t;
	t = localtime(&act_time);
	strftime(text, 6, "%H:%M", t);
	fb->putText(495, 50, 7, text);

	fb->fillBox(600, 57, 612, 530, 5);

	fb->setTextSize(0.4);
	fb->fillBox(100, 75, 120, 99, 5);
	fb->fillBox(120, 75, 400, 99, 0);
	fb->fillBox(400, 75, 410, 99, 5);
	fb->fillBox(410, 75, 480, 99, 0);
	fb->fillBox(480, 75, 490, 99, 5);
	fb->fillBox(490, 75, 560, 99, 0);
	fb->fillBox(560, 75, 580, 99, 5);
	fb->putText(125, 80, 0, programname, 270);

	fb->fillBox(100, 475, 120, 500, 5);
	fb->fillBox(120, 475, 350, 500, 0);
	fb->fillBox(350, 475, 370, 500, 5);

	t = localtime(&starttime);
	strftime(text, 6, "%H:%M", t);
	fb->putText(415, 80, 0, text);
	strftime(text, 24, "%A, %m/%d/%Y", t);
	fb->putText(125, 480, 0, text);

	time_t endtime = starttime + duration;
	t = localtime(&endtime);
	strftime(text, 6, "%H:%M", t);
	fb->putText(495, 80, 0, text);

	fb->setTextSize(0.45);
	fb->fillBox(100, 105, 120, 129, 5);
	fb->fillBox(120, 105, 560, 129, 7);
	fb->fillBox(560, 105, 600, 129, 5);
	fb->putText(125, 110, 7, event_short_text, 430);

	fb->fillBox(100, 135, 110, 470, 5);
	fb->fillBox(110, 135, 580, 470, 7);
	fb->fillBox(580, 135, 590, 470, 5);

	fb->fillBox(370, 475, 570, 500, 7);
	fb->fillBox(570, 475, 590, 500, 5);
	fb->putText(375, 480, 7, "Linkage:");
	fb->putText(500, 480, 7, linkage);

	fb->fillBox(100, 505, 120, 530, 5);
	fb->fillBox(120, 505, 350, 530, 7);
	fb->putText(125, 510, 7, audio);

	fb->fillBox(350, 505, 370, 530, 5);
	fb->fillBox(370, 505, 570, 530, 7);
	fb->fillBox(570, 505, 590, 530, 5);
	fb->putText(375, 510, 7, "FSK:");
	if (FSK == 0)
		fb->putText(500, 510, 7, 0);
	else
		fb->putText(500, 510, 7, FSK + 3);

	nlcounter = 0;
	int last = 0;
	int length = 0;
	printf("Start\n");
	for (int i = 0; i < (int) event_extended_text.length(); i++)
	{
		if (event_extended_text[i] == ' ')
		{
			last = i;
			length += fb->getWidth(event_extended_text[i]);
		}
		else if (event_extended_text[i] == '\n')
		{
			newlines[nlcounter++] = i;
			length = 0;
			i++;
		}
		else
		{

			length += fb->getWidth(event_extended_text[i]);
			if (length >= 350)
			{
				i = last;
				length = 0;
				newlines[nlcounter++] = i;
			}
		}

	}
	printf("%d\n", nlcounter);

	last = 0;
	int i;
	int stop;
	if (nlcounter > 16)
	{
		stop = 15;
	}
	else
	{
		stop = nlcounter;
	}
	for (i = 0; i < stop; i++)
	{
		printf("%d - %d\n", i, newlines[i]);
		fb->putText(115, 150 + 20 * i, 7, event_extended_text.substr(last, newlines[i] - last));
		last = newlines[i] + 1;
	}
	if (newlines[i - 1] < (int)event_extended_text.length() && nlcounter < 16)
	{
		fb->putText(115, 150 + 20 * (i), 7, event_extended_text.substr(newlines[i - 1] + 1));
	}
	printf("End\n");
	shown = 0;
}

void osd::showPrevEPGPage()
{
	shown--;
	if (shown < 0)
	{
		shown++;
		return;
	}

	fb->fillBox(100, 135, 110, 470, 5);
	fb->fillBox(110, 135, 580, 470, 7);
	fb->fillBox(580, 135, 590, 470, 5);

	int last = newlines[shown * 15];
	int i;
	int stop;
	if (nlcounter - (shown * 16) > 16)
	{
		stop = 15 * (shown + 1);
	}
	else
	{
		stop = nlcounter;
	}
	for (i = shown * 15; i < stop; i++)
	{
		printf("%d - %d\n", i, newlines[i]);
		fb->putText(115, 150 + 20 * (i - shown * 15), 7, event_extended_text.substr(last, newlines[i] - last));
		last = newlines[i] + 1;
	}
	if (newlines[i - 1] < (int)event_extended_text.length() && nlcounter - (shown * 16) < 16)
	{
		fb->putText(115, 150 + 20 * (i - shown * 15), 7, event_extended_text.substr(newlines[i - 1] + 1));
	}
}

void osd::showNextEPGPage()
{
	shown++;
	if (shown * 15 > nlcounter)
	{
		shown--;
		return;
	}

	fb->fillBox(100, 135, 110, 470, 5);
	fb->fillBox(110, 135, 580, 470, 7);
	fb->fillBox(580, 135, 590, 470, 5);

	int last = newlines[shown * 15];
	int i;
	int stop;
	if (nlcounter - (shown * 16) > 16)
	{
		stop = 15 * (shown + 1);
	}
	else
	{
		stop = nlcounter;
	}
	for (i = shown * 15; i < stop; i++)
	{
		printf("%d - %d\n", i, newlines[i]);
		fb->putText(115, 150 + 20 * (i - shown * 15), 7, event_extended_text.substr(last, newlines[i] - last));
		last = newlines[i] + 1;
	}
	if (newlines[i - 1] < (int)event_extended_text.length() && nlcounter - (shown * 16) < 16)
	{
		fb->putText(115, 150 + 20 * (i - shown * 15), 7, event_extended_text.substr(newlines[i - 1] + 1));
	}
}

void osd::hideEPG()
{
	fb->clearScreen();
}

void osd::createMenu()
{
	selected_entry = -1;
	number_menu_entries = 0;
	addCommand("COMMAND menu set_size 0");
	for (int i = 0; i < 20; i++)
	{
		menu[i].switches.clear();
		menu[i].selected = 0;
	}
}

/*
types:
0 - Normal entry (default)
1 - Select entry
2 - Switching entry
*/
void osd::addMenuEntry(int index, std::string  caption, int type)
{
	menu[number_menu_entries].index = index;
	menu[number_menu_entries].caption = caption;
	menu[number_menu_entries].type = type;
	menu[number_menu_entries].switches.clear();
	number_menu_entries++;
}

void osd::addSwitchParameter(int number, std::string  parameter)
{
	menu[number].switches.insert(menu[number].switches.begin(), parameter);
}

void osd::setSelected(int number, int sel)
{
	menu[number].selected = sel;
}

void osd::setMenuTitle(std::string  title)
{
	menu_title = title;
}

void osd::selectEntry(int number)
{
	if (number_menu_entries == 0)
		return;
	if (number == 0)
		return;
	if (selected_entry != -1)
		drawMenuEntry(selected_entry);
	for (int i = 0; i < number_menu_entries; i++)
		if (menu[i].index == number)
		{
			selected_entry = i;
			drawMenuEntry(i, true);
			return;
		}
	drawMenuEntry(selected_entry, true);

}

void osd::selectNextEntry()
{
	if (number_menu_entries == 0)
		return;
	if (selected_entry != -1)
		drawMenuEntry(selected_entry);
	do
	{
		selected_entry++;
		if (selected_entry >= number_menu_entries)
			selected_entry = 0;
	} while(menu[selected_entry].type == 3);
	drawMenuEntry(selected_entry, true);
}

void osd::selectPrevEntry()
{
	if (number_menu_entries == 0)
		return;
	if (selected_entry != -1)
		drawMenuEntry(selected_entry);
	do
	{
		selected_entry--;
		if (selected_entry < 0)
			selected_entry = number_menu_entries - 1;
	} while(menu[selected_entry].type == 3);
	drawMenuEntry(selected_entry, true);
}

int osd::menuSelectedIndex()
{
	return menu[selected_entry].index;
}

void osd::drawMenuEntry(int number, bool selected)
{
	int color1 = 5;
	int color2 = 0;
	int color3 = 6;
	if (selected)
	{
		color1 = 0;
		color2 = 5;
		color3 = 7;
	}
	if (menu[number].type != 3)
	{
		fb->fillBox(100, 75 + number * 30, 110, 100 + number * 30, color1);
		fb->fillBox(110, 75 + number * 30, 180, 100 + number * 30, color2);
		fb->fillBox(180, 75 + number * 30, 440 + menu_size, 100 + number * 30, color1);
		fb->putText(190, 80 + number * 30, color1, menu[number].caption, 250 + menu_size);
		fb->putText(125, 80 + number * 30, color2, menu[number].index);
		if (menu[number].type == 1)
		{
			fb->fillBox(420, 80 + number * 30, 435, 95 + number * 30, color2);
			if (!menu[number].selected)
				fb->fillBox(423, 83 + number * 30, 432, 92 + number * 30, color1);
		}
		if (menu[number].type == 2)
		{
			fb->putText(435, 80 + number * 30, color3, menu[number].switches[menu[number].selected], -1, 1);
		}
	}
	else
	{
		fb->fillBox(100, 75 + number * 30, 452, 100 + number * 30, color1);
		fb->putText(130, 80 + number * 30, color1, menu[number].caption);
	}
}

void osd::showMenu()
{
	fb->setTextSize(0.45);
	fb->fillBox(100, 45, 452 + menu_size, 70, 5);
	fb->fillBox(452 + menu_size, 57, 464 + menu_size, 75 + number_menu_entries * 30, 5);
	fb->fillBox(125, 45, 350, 70, 0);
	fb->putText(130, 50, 0, menu_title);
	for (int j = 0; j <= 12; j++)
	{
		fb->fillBox(88 + circlemiddle[12 - j], 45 + j, 100, 45 +  j + 1, 5);
		fb->fillBox(88 + circlemiddle[j], 57 + j, 100, 57 +  j + 1, 5);
		fb->fillBox(452 + menu_size, 45 + j, 464 - circlemiddle[12 - j] + menu_size, 45 +  j + 1, 5);
	}

	for (int i = 0; i < number_menu_entries; i++)
	{
		drawMenuEntry(i);
	}
}

void osd::hideMenu()
{
	fb->fillBox(88, 45, 464 + menu_size, 575, -1);
}

void osd::setMute(bool mute)
{
	if (mute)
	{
		for (int j = 0; j <= 12; j++)
		{
			fb->fillBox(500 + circlemiddle[12 - j], 75 + j, 513, 75 +  j + 1, 5);
			fb->fillBox(500 + circlemiddle[j], 87 + j, 513, 87 +  j + 1, 5);
			fb->fillBox(585, 75 + j, 598 - circlemiddle[12 - j], 75 +  j + 1, 5);
			fb->fillBox(585, 87 + j, 598 - circlemiddle[j], 87 +  j + 1, 5);

		}
		fb->fillBox(513, 75, 585, 99, 0);
		fb->setTextSize(0.45);
		fb->putText(515, 80, 0, "Silence");

	}
	else
	{
		fb->fillBox(500, 75, 598, 100, -1);
	}
}

void osd::showVolume()
{
	for (int j = 0; j <= 10; j++)
	{
		fb->fillBox(500 + circlesmall[10 - j], 110 + j, 510, 110 + j + 1, 5);
		fb->fillBox(500 + circlesmall[j], 120 + j, 510, 120 + j + 1, 5);
		fb->fillBox(500 + circlesmall[10 - j], 125 + j, 510, 125 + j + 1, 5);
		fb->fillBox(500 + circlesmall[j], 135 + j, 510, 135 + j + 1, 5);

		fb->fillBox(588, 110 + j, 598 - circlesmall[10 - j], 110 + j + 1, 5);
		fb->fillBox(588, 120 + j, 598 - circlesmall[j], 120 + j + 1, 5);
		fb->fillBox(588, 125 + j, 598 - circlesmall[10 - j], 125 + j + 1, 5);
		fb->fillBox(588, 135 + j, 598 - circlesmall[j], 135 + j + 1, 5);
	}
	fb->fillBox(510, 110, 588, 145, 0);

	int slider = (int) (((float)volume / 63) * 10);

	for (int i = 0; i < slider; i++)
	{
		fb->fillBox(515 + i * 7, 115, 519 + i * 7, 125, 8);
		fb->fillBox(515 + i * 7, 130, 519 + i * 7, 140, 8);
	}

	for (int i = slider + 1; i < 10; i++)
	{
		fb->fillBox(515 + i * 7, 115, 519 + i * 7, 125, 0);
		fb->fillBox(515 + i * 7, 130, 519 + i * 7, 140, 0);
	}
}

void osd::hideVolume()
{
	fb->fillBox(500, 110, 598, 145, -1);
}

void osd::createScan()
{
	percentage = 0;
	channel_count = 0;
	TS_count = 0;
	fb->setTextSize(0.5);
}

void osd::setScanProgress(int percent)
{
	percentage = percent;

	for (int i = 0; i < (int) ((float)percentage / 5); i++)
	{
		fb->fillBox(225 + i * 13, 465, 231 + i * 13, 485, 8);
	}

	for (int i = (int) ((float)percentage / 5) + 1; i < 20; i++)
	{
		fb->fillBox(225 + i * 13, 465, 231 + i * 13, 485, 0);
	}



	char perc[5];

	sprintf(perc, "%d", percentage);
	strcat(perc, " %");

	fb->fillBox(330, 430, 400, 459, 5);
	fb->putText(330, 435, 5, perc);
}

void osd::setScanChannelNumber(int number)
{
	channel_count = number;

	fb->fillBox(410, 335, 500, 365, 5);
	fb->putText(480, 340, 5, channel_count, -1, 1);
}

void osd::setScanTSNumber(int number)
{
	TS_count = number;

	fb->fillBox(410, 370, 500, 400, 5);
	fb->putText(480, 375, 5, TS_count, -1, 1);
}

void osd::showScan()
{
	printf("+-+-+-+-+ Draw Channelscan\n");
	fb->fillBox(200, 300, 500, 500, 5);
	fb->fillBox(210, 460, 490, 490, 0);
	fb->putText(300, 305, 5, "Channel-Scan");
	fb->fillBox(200, 325, 500, 327, 0);
	fb->fillBox(200, 400, 500, 402, 0);

	fb->putText(220, 340, 5, "Found Channels:");
	fb->putText(220, 375, 5, "Found TSs:");

	fb->putText(480, 375, 5, TS_count, -1, 1);
}

void osd::hideScan()
{
	fb->fillBox(200, 300, 500, 500, -1);
}

void osd::createSchedule()
{
	sched.clear();
	selected_sched = 0;
}

void osd::addScheduleInformation(time_t starttime, std::string description, int eventid)
{
	printf("Add: %s\n", description.c_str());
	scheduling tmp_sched;
	tmp_sched.starttime = starttime;
	tmp_sched.description = description;
	tmp_sched.eventid = eventid;
	sched.insert(sched.end(), tmp_sched);
}

void osd::selectScheduleInformation(int select, bool redraw)
{
	printf("Selected %d from %d\n", select, selected_sched);
	if (redraw)
	printf("With redraw\n");
	else
	printf("Without redraw\n");

	if (sched.size() == 0)
		return;

	char text[20];
	struct tm *t;

	if (redraw)
	{
		fb->fillBox(100, 103 + selected_sched * 20, 630, 124 + selected_sched * 20, 5);

		t = localtime(&(sched[shown_page * 15 + selected_sched].starttime));
		strftime(text, 11, "%a, %H:%M", t);
		fb->putText(105, 107 + selected_sched * 20, 5, text);
		fb->putText(210, 107 + selected_sched * 20, 5, sched[shown_page * 15 + selected_sched].description, 410);
	}

	selected_sched = select;

	fb->fillBox(103, 104 + selected_sched * 20, 204, 124 + selected_sched * 20, 0);
	fb->fillBox(207, 104 + selected_sched * 20, 625, 124 + selected_sched * 20, 0);
	t = localtime(&(sched[shown_page * 15 + selected_sched].starttime));
	strftime(text, 11, "%a, %H:%M", t);
	fb->putText(105, 107 + selected_sched * 20, 0, text);
	fb->putText(210, 107 + selected_sched * 20, 0, sched[shown_page * 15 + selected_sched].description, 410);


}

int osd::numberSchedulePages()
{
	printf("9\n");
	return (int) ((float)sched.size() / 15) + 1;
}

void osd::showSchedule(int page)
{
	printf("Begin showing\n");
	shown_page = page;
	printf("1\n");
	if (shown_page < 0)
	{
		printf("2\n");
		shown_page = 0;
		return;
	}
	else if (shown_page >= numberSchedulePages())
	{
		printf("3\n");
		shown_page = numberSchedulePages() - 1;
		printf("10\n");
		return;
	}

	fb->fillBox(100, 100, 630, 420, 5);
	printf("4\n");
	int max = 15;
	if ((page + 1) * 15 > (int)sched.size())
		max = sched.size() % 15;
	printf("Page: %d - Sched.size: %d\n", page, sched.size());
	fb->setTextSize(0.4);
	printf("Max: %d\n", max);
	for (int i = 0; i < max; i++)
	{

		char text[20];
		struct tm *t;
		t = localtime(&(sched[page * 15 + i].starttime));
		strftime(text, 11, "%a, %H:%M", t);
		fb->putText(105, 107 + i * 20, 5, text);
		fb->putText(210, 107 + i * 20, 5, sched[page * 15 + i].description, 410);
	}
	printf("End showing\n");
}

void osd::hideSchedule()
{
	fb->fillBox(100, 100, 630, 420, -1);
}

void osd::selectNextScheduleInformation()
{

	int max = 15;
	if ((shown_page + 1) * 15 > (int)sched.size())
		max = sched.size() % 15;

	printf("Max: %d - Selected_sched: %d\n shown_page: %d\n numberSchedulePages: %d\n", max, selected_sched, shown_page, numberSchedulePages());

	if (selected_sched + 1 < max)
		selectScheduleInformation(selected_sched + 1);
	else
	{
		if (shown_page + 1 >= numberSchedulePages())
			return;
		showSchedule(shown_page + 1);
		selectScheduleInformation(0, false);
	}
}

void osd::selectPrevScheduleInformation()
{
	int max = 15;
	if ((shown_page + 1) * 15 > (int)sched.size())
		max = sched.size() % 15;

	if (selected_sched - 1 > -1)
		selectScheduleInformation(selected_sched - 1);
	else
	{
		if (shown_page - 1 < 0)
			return;
		showSchedule(shown_page - 1);
		selectScheduleInformation(max - 1, false);
	}

}

void osd::nextSchedulePage()
{
	if (shown_page + 1 >= numberSchedulePages())
		return;
	showSchedule(shown_page + 1);
	selectScheduleInformation(0, false);
}

void osd::prevSchedulePage()
{
	if (shown_page - 1 < 0)
		return;
	showSchedule(shown_page - 1);

	int max = 15;
	if ((shown_page + 1) * 15 > (int)sched.size())
		max = sched.size() % 15;

	selectScheduleInformation(max - 1, false);
}

int osd::getSelectedSchedule()
{
	if (sched.size() == 0)
		return 0;
	return sched[shown_page * 15 + selected_sched].eventid;
}

void osd::createIP()
{
	ip_position = 0;
	for (int i = 0; i < 12; i++)
	{
		ip[i] = 0;
	}
}

void osd::setIP(unsigned char number)
{
	ip[ip_position] = number;
	drawIPPosition(ip_position, 0);
}

void osd::setIPn(unsigned char position, unsigned char number)
{
	ip[position * 3 + 2] = number % 10;
	ip[position * 3 + 1] = (unsigned char)(number % 100 / 10);
	ip[position * 3] = (unsigned char) (number % 1000 / 100);
}

void osd::drawIPPosition(int position, int color)
{
	fb->fillBox(90 + position * 40 + ((int)((float)position / 3)) * 10, 300, 125 + position * 40 + ((int)((float)position / 3)) * 10, 335, color);
	fb->putText(97 + position * 40 + ((int)((float)position / 3)) * 10, 310, color, ip[position]);
}

void osd::setIPPosition(unsigned char position)
{
	ip_position = position;

	drawIPPosition(ip_position, 0);
}

void osd::setIPNextPosition()
{
	drawIPPosition(ip_position, 5);
	ip_position++;
	if (ip_position > 11)
		ip_position = 0;

	setIPPosition(ip_position);
}

void osd::setIPPrevPosition()
{
	drawIPPosition(ip_position, 5);

	ip_position--;
	if (ip_position < 0)
		ip_position = 11;

	setIPPosition(ip_position);
}

void osd::setIPDescription(std::string descr)
{
	ip_description = descr;
}

void osd::showIP()
{
	for (int j = 0; j <= 10; j++)
	{
		fb->fillBox(150 + circlesmall[10 - j], 200 + j, 160, 200 + j + 1, 1);
		fb->fillBox(560, 200 + j, 570 - circlesmall[10 - j], 200 + j + 1, 1);
	}

	for (int j = 11; j < 20; j++)
	{
		fb->fillBox(150 + circlesmall[j - 11], 200 + j, 160, 200 + j + 1, 1);
		fb->fillBox(560, 200 + j, 570 - circlesmall[j - 11], 200 + j + 1, 1);
	}
	fb->fillBox(160, 200, 170, 220, 1);
	fb->fillBox(170, 200, 550, 220, 0);
	fb->fillBox(550, 200, 560, 220, 1);

	fb->setTextSize(0.4);
	fb->putText(175, 202, 0, ip_description, 370);

	fb->setTextSize(0.6);
	for (int j = 0; j <= 17; j++)
	{
		fb->fillBox(70 + circle[17 - j], 300 + j, 87, 300 + j + 1, 1);
		fb->fillBox(600, 300 + j, 618 - circle[17 - j], 300 + j + 1, 1);
	}
	for (int j = 18; j < 35; j++)
	{
		fb->fillBox(70 + circle[j - 18], 300 + j, 87, 300 + j + 1, 1);
		fb->fillBox(600, 300 + j, 618 - circle[j - 18], 300 + j + 1, 1);
	}

	for (int i = 0; i < 12; i++)
	{
		drawIPPosition(i, 5);
	}
}

void osd::hideIP()
{
	fb->clearScreen();
}

int osd::getIPPart(int number)
{
	return (ip[number * 3] * 100 + ip[number * 3 + 1] * 10 + ip[number * 3 + 2]);
}

void osd::showAbout()
{
	fb->fillBox(110, 100, 630, 420, 5);

	fb->setTextSize(1);

	fb->putText(220, 200, 5, setting.getVersion());

	fb->setTextSize(0.5);
	fb->putText(260, 250, 5, "GUI coded by TheDOC");
	fb->putText(170, 270, 5, "Drivers and stuff by the Tuxbox-Team");

	fb->setTextSize(0.4);
	fb->putText(175, 330, 5, "Info's about LCARS: http://cablelinux.info");
	fb->putText(180, 350, 5, "Info's about Dreambox: http://www.dream-multimedia-tv.de");
}

void osd::hideAbout()
{
	fb->fillBox(110, 100, 630, 420, -1);
}
