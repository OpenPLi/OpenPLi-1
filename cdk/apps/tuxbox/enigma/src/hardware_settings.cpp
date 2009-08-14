/*
 *  PLi extension to Enigma: hardware settings
 *
 *  Copyright (C) 2007 dAF2000 <David@daf2000.nl>
 *  Copyright (C) 2007 PLi team <http://www.pli-images.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <hardware_settings.h>

#define MENUNAME N_("Hardware settings")

class eHardwareSettingsFactory : public eCallableMenuFactory
{
public:
	eHardwareSettingsFactory() : eCallableMenuFactory("eHardwareSettings", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eHardwareSettings;
	}
};

eHardwareSettingsFactory eHardwareSettings_factory;

eHardwareSettings::eHardwareSettings()
	:eSetupWindow(_(MENUNAME), 6, 350)
{
	int entry = 1;

	valign();

#ifdef ENABLE_RFMOD
	addCallableMenuEntry("eZapRFmodSetup", _("open UHF Modulator settings"), &entry);
#endif
#ifndef DISABLE_HDD
#ifndef DISABLE_FILE
	addCallableMenuEntry("eHarddiskSetup", _("open harddisk settings"), &entry);
#endif
#endif
	addCallableMenuEntry("UsbSetup", _("open USB stick settings"), &entry);
#ifdef ENABLE_KEYBOARD
	addCallableMenuEntry("eZapKeyboardSetup", _("open keyboard settings"), &entry);
#endif
	addCallableMenuEntry("eZapRCSetup", _("open remote control setup"), &entry);
#ifndef DISABLE_CI
	addCallableMenuEntry("enigmaCI", _("Setup the common interface module"), &entry);
#endif

	CONNECT(list.selected, eHardwareSettings::entrySelected);
}

void eHardwareSettings::entrySelected(eListBoxEntryMenu* item)
{
	if(item && item->getKey())
	{
		hide();
		eCallableMenuFactory::showMenu((char*)item->getKey(), LCDTitle, LCDElement);
		show();
	}
}

void eHardwareSettings::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
