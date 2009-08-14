/*
 * software_update.cpp
 *
 * Copyright (C) 2003 Andreas Monzner <ghost@tuxbox.org>
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
 * $Id: software_update.cpp,v 1.7 2009/02/07 10:06:31 dbluelle Exp $
 */

#include <software_update.h>
#include <flashtool.h>
#include <lib/gui/emessage.h>
#include <lib/system/info.h>
#include <upgrade.h>
#include <enigma.h>

#define MENUNAME N_("Software update")

class eSoftwareUpdateFactory : public eCallableMenuFactory
{
public:
	eSoftwareUpdateFactory() : eCallableMenuFactory("eSoftwareUpdate", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eSoftwareUpdate;
	}

	bool isAvailable()
	{
		return eSystemInfo::getInstance()->hasNetwork();
	}
};

eSoftwareUpdateFactory eSoftwareUpdate_factory;

eSoftwareUpdate::eSoftwareUpdate()
	:eSetupWindow(_(MENUNAME), 3, 400)
{
	init_eSoftwareUpdate();
}
void eSoftwareUpdate::init_eSoftwareUpdate()
{
	valign();
#ifndef DISABLE_NETWORK
	int entry=0;
	if(!eSystemInfo::getInstance()->isOpenEmbedded())
	{
		CONNECT((new eListBoxEntryMenu(&list, _("Internet Update"), eString().sprintf("(%d) %s", ++entry, _("open internet update")) ))->selected, eSoftwareUpdate::internet_update );
		CONNECT((new eListBoxEntryMenu(&list, _("Manual Update"), eString().sprintf("(%d) %s", ++entry, _("open manual update")) ))->selected, eSoftwareUpdate::manual_update );
#ifdef ENABLE_FLASHTOOL
		CONNECT((new eListBoxEntryMenu(&list, _("Expert Flash Save/Restore"), eString().sprintf("(%d) %s", ++entry, _("open expert flash tool")) ))->selected, eSoftwareUpdate::flash_tool);
#endif
	}
	else
	{
		CONNECT((new eListBoxEntryMenu(&list, _("Prepare box for new image"), eString().sprintf("(%d) %s", ++entry, _("put your dreambox in **STOP** mode")) ))->selected, eSoftwareUpdate::stop_mode );
	}
#endif
}

#ifndef DISABLE_NETWORK
void eSoftwareUpdate::internet_update()
{
	hide();
	eUpgrade up(false);
#ifndef DISABLE_LCD
	up.setLCD(LCDTitle, LCDElement);
#endif
	up.show();
	up.exec();
	up.hide();
	show();
}

void eSoftwareUpdate::manual_update()
{
	hide();
	int ret = eMessageBox::ShowBox(_("Upload your Image via FTP or Samba to the '/tmp' folder."
										"Then rename it to 'root.cramfs' and press ok."
										"In the upcomming list select 'manual update' and follow the instructions."), _("Manual update"), eMessageBox::iconInfo|eMessageBox::btOK );
	if ( ret == eMessageBox::btOK )
	{
		eUpgrade up(true);
#ifndef DISABLE_LCD
		up.setLCD(LCDTitle, LCDElement);
#endif
		up.show();
		up.exec();
		up.hide();
	}
	show();
}

void eSoftwareUpdate::stop_mode()
{
	eMessageBox messagebox(_("Are you sure you want to put\n"
				"your box in **STOP** mode?\n"
				"When box is in **STOP** mode\n"
				"you need to connect to box via\n"
				"your webbrowser and flash the new image\n"
				"Make sure you make a backup of\n"
				"your settings before continuing"), _("Flash new image"), eMessageBox::iconInfo | eMessageBox::btYes | eMessageBox::btNo, eMessageBox::btNo );

	messagebox.show();
	int ret = messagebox.exec();
	messagebox.hide();
	if ( ret == eMessageBox::btYes )
	{
		system("mount /boot -o remount,rw && echo 1 > /boot/stop");
		// Now reboot the box
		eZap::getInstance()->quit(4);
	}
}

#endif

#ifdef ENABLE_FLASHTOOL
void eSoftwareUpdate::flash_tool()
{
	hide();
	eFlashtoolMain setup;
#ifndef DISABLE_LCD
	setup.setLCD(LCDTitle, LCDElement);
#endif
	setup.show();
	setup.exec();
	setup.hide();
	show();
}
#endif

void eSoftwareUpdate::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
