/*
 * time_settings.cpp
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
 * $Id: time_settings.cpp,v 1.6 2009/02/07 10:06:31 dbluelle Exp $
 */

#include <time_settings.h>
#include <setup_timezone.h>
#include <time_correction.h>
#include <timer.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/dvbservice.h>
#include <lib/gui/emessage.h>

#define MENUNAME N_("Time")

class eTimeSettingsFactory : public eCallableMenuFactory
{
public:
	eTimeSettingsFactory() : eCallableMenuFactory("eTimeSettings", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eTimeSettings;
	}
};

eTimeSettingsFactory eTimeSettings_factory;

eTimeSettings::eTimeSettings()
	:eSetupWindow(_(MENUNAME), 4, 300),
	i12hourClock(0), systemTime(0), clockType(NULL), timeSource(NULL)
{
	init_eTimeSettings();
}
void eTimeSettings::init_eTimeSettings()
{
	valign();
	int entry=0;

	list.setFlags(list.getFlags()|eListBoxBase::flagNoPageMovement);

	CONNECT((new eListBoxEntryMenu(&list, _("Time zone configuration"), eString().sprintf("(%d) %s", ++entry, _("Select local time zone")) ))->selected, eTimeSettings::time_zone);
	CONNECT((new eListBoxEntryMenu(&list, _("Time correction"), eString().sprintf("(%d) %s", ++entry, _("Correct the time received from the satellites")) ))->selected, eTimeSettings::time_correction);
	clockType = new eListBoxEntryMulti(&list, _("Select 12 or 24 hours clock (left, right)"));
	clockType->add((eString)"  " + (eString)(_("24 hours clock")) + (eString)" >", 0);
	clockType->add((eString)"< " + (eString)(_("12 hours clock")) + (eString)"  ", 1);

	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", i12hourClock) ;
	clockType->setCurrent(i12hourClock);
	CONNECT(list.selchanged, eTimeSettings::selClockTypeChanged);

	timeSource = new eListBoxEntryMulti(&list, _("Select time sync method (left, right)"));
	timeSource->add((eString)"  " + (eString)(_("system time")) + (eString)" >", 1);
	timeSource->add((eString)"< " + (eString)(_("transponder time")) + (eString)"  ", 0);

	eConfig::getInstance()->getKey("/elitedvb/extra/useSystemTime", systemTime) ;
	timeSource->setCurrent(systemTime);
	CONNECT(list.selchanged, eTimeSettings::selTimesourceChanged);
}

void eTimeSettings::selClockTypeChanged(eListBoxEntryMenu* e)
{
	if(e == (eListBoxEntryMenu*)clockType)
	{
		eConfig::getInstance()->setKey("/ezap/osd/12hourClock", (int)clockType->getKey());
	}
}

void eTimeSettings::selTimesourceChanged(eListBoxEntryMenu* e)
{
	if(e == (eListBoxEntryMenu*)timeSource)
	{
		eConfig::getInstance()->setKey("/elitedvb/extra/useSystemTime", (int)timeSource->getKey());
	}
}

void eTimeSettings::time_zone()
{
	hide();
	eZapTimeZoneSetup setup;
#ifndef DISABLE_LCD
	setup.setLCD(LCDTitle, LCDElement);
#endif
	setup.show();
	setup.exec();
	setup.hide();
	show();
}

void eTimeSettings::time_correction()
{
	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
	if (sapi && sapi->transponder)
  {
		tsref ref = *sapi->transponder;
		hide();
		eTimeCorrectionEditWindow w(ref);
#ifndef DISABLE_LCD
		w.setLCD(LCDTitle, LCDElement);
#endif
		w.show();
		w.exec();
		w.hide();
		show();
	}
	else
	{
		hide();
		eMessageBox::ShowBox( _("To change time correction you must tune first to any transponder"), _("time correction change error"), eMessageBox::btOK|eMessageBox::iconInfo );
		show();
	}
}

void eTimeSettings::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
