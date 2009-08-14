/*
 * system_settings.cpp
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
 * $Id: system_settings.cpp,v 1.9 2006/02/05 23:41:01 pieterg Exp $
 */

#include <system_settings.h>
#include <lib/gui/emessage.h>
#include <lib/system/info.h>
#include <callablemenu.h>
#include <key_mapping.h>

#define MENUNAMESYSTEM N_("System settings")
#define MENUNAMEFACTORY N_("Factory reset")

//implemented in upgrade.cpp
extern bool erase(char mtd[30], const char *titleText);

class eSystemSettingsFactory : public eCallableMenuFactory
{
public:
	eSystemSettingsFactory() : eCallableMenuFactory("eSystemSettings", MENUNAMESYSTEM) {}
	eCallableMenu *createMenu()
	{
		return new eSystemSettings;
	}
};

eSystemSettingsFactory eSystemSettings_factory;

class FactoryResetFactory : public eCallableMenuFactory
{
public:
	FactoryResetFactory() : eCallableMenuFactory("FactoryReset", MENUNAMEFACTORY) {}
	eCallableMenu *createMenu()
	{
		return new FactoryReset;
	}

	bool isAvailable()
	{
		return eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000;
	}
};

FactoryResetFactory FactoryReset_factory;

eSystemSettings::eSystemSettings()
	:eSetupWindow(_(MENUNAMESYSTEM), 9, 520)
{
	valign();
	int entry = 1;
	
	list.setColumns(2);

	// First column
	addCallableMenuEntry("eZapVideoSetup", _("Set screen aspect ratio, TV system and more"), &entry);
	addCallableMenuEntry("eTimeSettings", _("Set time zone, kind of clock and time correction"), &entry);
	addCallableMenuEntry("RecordingPlayingSetup", _("Set recording and playing options for recorded media"), &entry);
	addCallableMenuEntry("EpgSetup", _("Configure the Electronic Program Guide"), &entry);
	addCallableMenuEntry("eParentalSetup", _("Lock channels and setup screens"), &entry);
	addCallableMenuEntry("TeletextSetup", _("Set teletext options"), &entry);
	addCallableMenuEntry("WebIfSetup", _("Configure the web interface"), &entry);
	addCallableMenuEntry("BackupScreen", _("Backup and restore your Dreambox"), &entry);
	addCallableMenuEntry("");

	// Second column
#ifndef DISABLE_NETWORK
	addCallableMenuEntry("eZapNetworkSetup", _("Configure your ethernet network"), &entry);
	addCallableMenuEntry("eMountSetup", _("Configure your external mounts to access them on the Dreambox"), &entry);
#endif
	addCallableMenuEntry("ServicesSetup", _("Select which services to run after a reboot"), &entry);
#ifdef TARGET_CDK
	addCallableMenuEntry("SwapAndVarSetup", _("Enlarge your system memory by swap or more space for programs in /var"), &entry);
#else
	addCallableMenuEntry("SwapAndVarSetup", _("Enlarge your system memory by swap"), &entry);
#endif
#ifndef DISABLE_NETWORK
	addCallableMenuEntry("ENgrabSetup", _("Configure a Ngrab stream server"), &entry);
	addCallableMenuEntry("eSoftwareUpdate", _("Install a new Dreambox image"), &entry);
#endif
	addCallableMenuEntry("FactoryReset", _("Reset all settings to factory defaults"), &entry);
	addCallableMenuEntry("DebugSetup", _("Set several debugging options"), &entry);
	addCallableMenuEntry("KeyMapping", _("Assign functionality, menus or plugins to the colour buttons"), &entry);

	CONNECT(list.selected, eSystemSettings::entrySelected);
}

void eSystemSettings::entrySelected(eListBoxEntryMenu* item)
{
	if(item && item->getKey())
	{
		hide();
		eCallableMenuFactory::showMenu((char*)item->getKey(), LCDTitle, LCDElement);
		show();
	}
}

void eSystemSettings::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

void FactoryReset::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	 int ret = eMessageBox::ShowBox(
		_("**WARNING**\n"
		"You are about to do a factory reset. You will lose ALL of your configuration "
		"settings and the default configuration will be used. Your Dreambox will then reboot.\n\n"
		"Are you REALLY sure?"),
		_(MENUNAMEFACTORY),
		eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion,
		eMessageBox::btNo);
		
	if(ret == eMessageBox::btYes) 
	{
		switch(eSystemInfo::getInstance()->getHwType())
		{
			case eSystemInfo::DM500PLUS:
			case eSystemInfo::DM600PVR:
			case eSystemInfo::DM7020:
				system("rm -R /etc/enigma && killall -9 enigma");
				break;
				
			case eSystemInfo::DM7000:
			case eSystemInfo::DM500:
			case eSystemInfo::DM5620:
			case eSystemInfo::DM5600:
			case eSystemInfo::TR_DVB272S:
				// first do systemcall to unmove current /var
				system("/bin/movevar.sh unmove");
				erase("/dev/mtd/1", _("Factory reset..."));
				system("reboot");
				break;
				
			default: 
				eDebug("factory reset not implemented for this hardware!!\n");
		}
	}
}
