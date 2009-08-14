/*
 * setup_extra.cpp
 *
 * Copyright (C) 2003 Andreas Monzner <ghostrider@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: setup_extra.cpp,v 1.71 2007/02/20 20:51:48 ghostrider Exp $
 */

/******************************************************************
 * THIS FILE DOES NOT HAVE ANY CODE FOR THE DREAMBOX ANYMORE.     *
 * THE WHOLE MENU HAS BEEN REMOVED. KEEP THIS FILE FOR EASIER CVS *
 * MERGING ONLY.                                                  *
 ******************************************************************/

#include <enigma.h>
#include <setup_extra.h>
#include <lib/gui/emessage.h>
#include <lib/system/info.h>

eExpertSetup::eExpertSetup()
	:eSetupWindow(_("Expert Setup"), 4, 300)
{
	init_eExpertSetup();
}

void eExpertSetup::init_eExpertSetup()
{
	valign();

#ifndef HAVE_DREAMBOX_HARDWARE
	new eListBoxEntryMenuSeparator(&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
//Boot-info
	int bootInfo = 0;
	if (access("/var/etc/.boot_info", R_OK) == 0)
		bootInfo = 1;
	eConfig::getInstance()->setKey("/extras/bootinfo", bootInfo);
	CONNECT_2_1((new eListBoxEntryCheck(&list, _("Show Boot-Info"), "/extras/bootinfo", _("Show Boot-Infos (IP, etc.)")))->selected, eExpertSetup::fileToggle,"/var/etc/.boot_info");
//HW-Sections
	int hwSectionsDisable = 0;
	if (access("/var/etc/.hw_sections", R_OK) == 0)
		hwSectionsDisable = 1;
	eConfig::getInstance()->setKey("/extras/hw_sections_disable", hwSectionsDisable);
	CONNECT_2_1((new eListBoxEntryCheck(&list, _("Disable HW_Sections"), "/extras/hw_sections_disable", _("don't use hardware section filtering")))->selected, eExpertSetup::fileToggle,"/var/etc/.hw_sections");
//Watchdog
	int watchdogDisable = 0;
	if (access("/var/etc/.no_watchdog", R_OK) == 0)
		watchdogDisable = 1;
	eConfig::getInstance()->setKey("/extras/watchdog_disable", watchdogDisable);
	CONNECT_2_1((new eListBoxEntryCheck(&list, _("Disable Watchdog"), "/extras/watchdog_disable", _("don't use the Watchdog")))->selected, eExpertSetup::fileToggle,"/var/etc/.no_watchdog");
//ENX-Watchdog - Philips and Sagem
	if ( eSystemInfo::getInstance()->getHwType() != eSystemInfo::dbox2Nokia )
	{
		int enxWatchdogDisable = 0;
		if (access("/var/etc/.no_enxwatchdog", R_OK) == 0)
			enxWatchdogDisable = 1;
		eConfig::getInstance()->setKey("/extras/enxwatchdog_disable", enxWatchdogDisable);
		CONNECT_2_1((new eListBoxEntryCheck(&list, _("Disable ENX-Watchdog"), "/extras/enxwatchdog_disable", _("don't use the ENX-Watchdog")))->selected, eExpertSetup::fileToggle,"/var/etc/.no_enxwatchdog");
	}
//SPTS-Recording
	int sptsMode = 0;
	if (access("/var/etc/.spts_mode", R_OK) == 0)
		sptsMode = 1;
	eConfig::getInstance()->setKey("/extras/spts_mode", sptsMode);
	CONNECT_2_1((new eListBoxEntryCheck(&list, _("Enable SPTS-Mode"), "/extras/spts_mode", _("use SPTS-Mode (enables TS-recording)")))->selected, eExpertSetup::fileToggle,"/var/etc/.spts_mode");
//File I/O-Options
	int OSyncDisable = 0;
	if (access("/var/etc/.no_o_sync", R_OK) == 0)
		OSyncDisable = 1;
	eConfig::getInstance()->setKey("/extras/O_SYNC_disable", OSyncDisable);
	CONNECT_2_1((new eListBoxEntryCheck(&list, _("Disable O_SYNC"), "/extras/O_SYNC_disable", _("The file/recording is not opened for synchronous I/O")))->selected, eExpertSetup::fileToggle,"/var/etc/.no_o_sync");
//Alternative Frontenddriver for Philips
	if ( eSystemInfo::getInstance()->getHwType() == eSystemInfo::dbox2Philips )
	{
		int tda80xx = 0;
		if (access("/var/etc/.tda80xx", R_OK) == 0)
			tda80xx = 1;
		eConfig::getInstance()->setKey("/extras/tda80xx", tda80xx);
		CONNECT_2_1((new eListBoxEntryCheck(&list, _("New Philips driver"), "/extras/tda80xx", _("use tda80xx driver for Philips boxes")))->selected, eExpertSetup::fileToggle,"/var/etc/.tda80xx");
	}
#endif

	/* help text for expert setup screen */
	setHelpText(_("\tExpert Setup\n\n>>> [MENU] >>> [6] Setup >>> [6] Expert Setup\n. . . . . . . . . .\n\n" \
								"Here you can make some changes to the behavior of your DreamBox (experts only!)\n. . . . . . . . . .\n\n" \
								"Usage:\n\n[UP]/[DOWN]\tSelect Inputfield, Button, or Sub Menu\n\n[OK]\tToggle option on/off\n\n" \
								"[LEFT]/[RIGHT]\tdecrease/increase value\n\n[EXIT]\tSave Settings and Close Window\n\n" \
								"Note: every item has an explanation of its function in the menu itself."));
}

#ifndef HAVE_DREAMBOX_HARDWARE
void eExpertSetup::fileToggle(bool newState, const char* filename)
{
	FILE* test;
	test = fopen(filename,"r");
	if (test != NULL)
	{
		fclose(test);
		::unlink(filename);
	}
	else
	{
		eString cmd = "touch ";
		cmd += filename;
		system(cmd.c_str());
	}
}
#endif
