/*
 *  PLi extension to Enigma: interface settings
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

#include <interface_settings.h>
#include <wizard_language.h>

#define MENUNAME N_("User interface settings")

class eInterfaceSettingsFactory : public eCallableMenuFactory
{
public:
	eInterfaceSettingsFactory() : eCallableMenuFactory("eInterfaceSettings", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eInterfaceSettings;
	}
};

eInterfaceSettingsFactory eInterfaceSettings_factory;

eInterfaceSettings::eInterfaceSettings()
	:eSetupWindow(_(MENUNAME), 8, 350)
{
	int entry = 1;

	valign();

	addCallableMenuEntry("ExtraOSDSetup", _("Set several OSD options and behaviour"), &entry);
	addCallableMenuEntry("PluginOffsetScreen", _("Center the OSD on your screen"), &entry);
	addCallableMenuEntry("eZapOsdSetup", _("Set the OSD transparency, brightness and contrast"), &entry);
	addCallableMenuEntry("eSkinSetup", _("Select an OSD skin"), &entry);
	//addCallableMenuEntry("eWizardLanguage", _("Select the language used in the Dreambox menus"), &entry);
	new eListBoxEntryMenu(&list, _("OSD language"),
		eString().sprintf("(%d) %s", entry++, _("Select the language used in the Dreambox menus")), 0, (void*)"eWizardLanguage");
	addCallableMenuEntry("MultiEPGSetup", _("Set several MultiEPG settings"), &entry);
#ifndef DISABLE_LCD
	addCallableMenuEntry("eZapLCDSetup", _("Setup LCD screen, like brightness and contrast"), &entry);
#endif
	addCallableMenuEntry("LanguageSetup", _("Select preferred audio languages while watching TV"), &entry);

	CONNECT(list.selected, eInterfaceSettings::entrySelected);
}

void eInterfaceSettings::entrySelected(eListBoxEntryMenu* item)
{
	if(item && item->getKey())
	{
		if(!strcmp((char*)item->getKey(), "eWizardLanguage"))
		{
			// For some strange reason eWizardLanguage cannot be run using
			// the factory without a fatal "eTextPara: still referenced".
			eWidget* oldfocus = focus;
			hide();
			eWizardLanguage::run();
			show();
			setFocus(oldfocus);
		}
		else
		{
			hide();
			eCallableMenuFactory::showMenu((char*)item->getKey(), LCDTitle, LCDElement);
			show();
		}
	}
}

void eInterfaceSettings::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
