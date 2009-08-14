/*
 * enigma_setup.cpp
 *
 * Copyright (C) 2002 Felix Domke <tmbinc@tuxbox.org>
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
 * $Id: enigma_setup.cpp,v 1.45 2009/02/03 18:54:33 dbluelle Exp $
 */

#include <enigma_setup.h>
#include <lib/dvb/edvb.h>
#include <lib/gui/emessage.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>
#include <sys/vfs.h>
#include <enigma_plugins.h>
#include <enigma_main.h>
#include <enigma.h>
#include <parentallock.h>
#include <ppanel.h>
#include <callablemenu.h>

#define MENUNAME N_("Dreambox settings")
const char* SOFTWAREPANEL = "/var/etc/software.xml";

/*----------------------------------------------------------------------------*/

class eZapSetupFactory : public eCallableMenuFactory
{
public:
	eZapSetupFactory() : eCallableMenuFactory("eZapSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eZapSetupWrapper;
	}
};

void eZapSetup::init_eZapSetup()
{
	if(!eSystemInfo::getInstance()->isOpenEmbedded())
	{
		setNrOfBars(1); // var bar
	}

	valign();
	int entry = 1;
	
	addCallableMenuEntry("eZapBouquetSetup", _("Create or edit bouquets"), &entry);
	addCallableMenuEntry("eZapScan", _("Setup and find satellites or scan for transponders"), &entry);
	new eListBoxEntryMenuSeparator(&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
	addCallableMenuEntry("eSystemSettings", _("Change internal settings like time, EPG and debugging settings"), &entry);
	addCallableMenuEntry("eHardwareSettings", _("Change hardware settings like harddisk, USB-stick or keyboard"), &entry);
	addCallableMenuEntry("eInterfaceSettings", _("Change user interface settings like OSD, LCD and languages"), &entry);
	new eListBoxEntryMenuSeparator(&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
	addCallableMenuEntry("eEmuConfig", _("Configure the softcam"), &entry);
	addCallableMenuEntry("eSoftwareManagement", _("Install and remove software"), &entry);

	// Set /var text and progress bar
	updateProgressBar();

	CONNECT(list.selected, eZapSetup::entrySelected);
	/* emit */ setupHook(this, &entry);
}

eZapSetupFactory eZapSetup_factory;

void eZapSetupWrapper::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	if(pinCheck::getInstance()->checkPin(pinCheck::setup))
	{
		int i = 0;
		do
		{
			eZapSetup setup;
#ifndef DISABLE_LCD
			setup.setLCD(lcdTitle, lcdElement);
#endif
			setup.show();
			i = setup.exec();
			setup.hide();
		} while(i == -1); // to redisplay Setup after language change
	}
}

/*----------------------------------------------------------------------------*/

class eSoftwareManagementFactory : public eCallableMenuFactory
{
public:
	eSoftwareManagementFactory() : eCallableMenuFactory("eSoftwareManagement", _("Software management")) {}
	eCallableMenu *createMenu()
	{
		return new eSoftwareManagement;
	}
};

eSoftwareManagementFactory eSoftwareManagement_factory;

void eSoftwareManagement::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	if(pinCheck::getInstance()->checkPin(pinCheck::setup))
	{
		PPanel::runPPanel(SOFTWAREPANEL, lcdTitle, lcdElement);

		if(access("/tmp/reloadUserBouquets", F_OK) == 0)
		{
			eZapMain::getInstance()->reloadSettings();
			unlink("/tmp/reloadUserBouquets");
		}
	}
}

/*----------------------------------------------------------------------------*/

eZapSetup::eZapSetup()
	:ProgressSetupWindow(_(MENUNAME), 9, 400)
{
	init_eZapSetup();
}

void eZapSetup::entrySelected(eListBoxEntryMenu* item)
{
	if(item)
	{
		hide();
		eCallableMenuFactory::showMenu((char*)item->getKey(), LCDTitle, LCDElement);
		show();
		
		updateProgressBar();
	}
}

void eZapSetup::updateProgressBar(void)
{
	int blocksPercentUsed = 0;
	char progressText[100];

	if (!eSystemInfo::getInstance()->isOpenEmbedded())
	{
		blocksPercentUsed = getFsFullPerc("/var");
		sprintf(progressText, _("/var used %d%%"), blocksPercentUsed);
		setProgressLabel((eString)progressText, 0);
		setProgressBar(blocksPercentUsed, 0);
	}
}

int eZapSetup::getFsFullPerc(const char* filesystem)
{
	long blocksUsed = 0;
	long blocksPercentUsed = 0;
	struct statfs s;
	
	if(statfs(filesystem, &s) == 0)
	{
		if((s.f_blocks > 0))
		{
			blocksUsed = s.f_blocks - s.f_bfree;
			blocksPercentUsed = 0;
			if(blocksUsed + s.f_bavail)
			{
				blocksPercentUsed = (int)((((long long) blocksUsed) * 100 +
					(blocksUsed + s.f_bavail)/2) / (blocksUsed + s.f_bavail));
			}
		}
	}
	return blocksPercentUsed;
}
