/*
 * Ronald's setup plugin for dreambox
 * Copyright (c) 2004 Ronaldd <Ronaldd@sat4all.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "setup_extra_osd.h"

#include <plugin.h>

#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <sselect.h>
#include <enigma.h>
#include <lib/base/i18n.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/frontend.h>

#define MENUNAME N_("OSD options")

class ExtraOSDSetupFactory : public eCallableMenuFactory
{
	public:
		ExtraOSDSetupFactory() : eCallableMenuFactory("ExtraOSDSetup", MENUNAME) {}
		eCallableMenu *createMenu()
		{
			return new ExtraOSDSetup;
		}
};

ExtraOSDSetupFactory ExtraOSDSetup_factory;

ExtraOSDSetup::ExtraOSDSetup()
	:eSetupWindow(_(MENUNAME), 13, 600)
{
	valign();
	list.setFlags(list.getFlags()|eListBoxBase::flagNoPageMovement);
	list.setColumns(2);

	new eListBoxEntryCheck(&list, _("OSD on zap"), "/ezap/osd/showOSDOnSwitchService", _("Show OSD after a zap"));
	new eListBoxEntryCheck(&list, _("Extra OSD"), "/ezap/osd/OSDExtraInfo", _("Show extra info in OSD on OK"));
	new eListBoxEntryCheck(&list, _("Autohide OSD on OK"), "/ezap/osd/enableAutohideOSDOn", _("Autohide OSD when pressing OK button"));
	new eListBoxEntryCheck(&list, _("Autohide radio/mp3 OSD"), "/ezap/osd/hideinradiomode", _("Autohide the OSD display in radio/mp3 mode"));
	new eListBoxEntryCheck(&list, _("SNR in dB"), "/pli/SNRdB", _("Show SNR values in dB instead of %"));
	if(eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite)
	{
		CONNECT((new eListBoxEntryCheck(&list, _("Improved BER"), "/pli/ImprovedBER", _("Show improved BER (Bit Error Rate) regards to the outer FEC and in a value from 0-6 represents errorrate in ai power of 10 (6 represents 1 bit in a million, 3 represents 1 biterror in a thousand)")))->selected, ExtraOSDSetup::improvedBERChanged);
	}
	new eListBoxEntryCheck(&list, _("Show plicons"), "/elitedvb/multiepg/plicons", _("Show plicons in EPG and skin"));
	new eListBoxEntryCheck(&list, _("Use Mini-Zap"), "/ezap/osd/miniZap", _("Show smaller OSD when zapping (if supported by the skin)"));
	new eListBoxEntryCheck(&list, _("Expert OSD"), "/ezap/osd/OSDVerboseInfo", _("Show other extra info in OSD on OK"));
	new eListBoxEntryCheck(&list, _("Expert OSD on zap"), "/ezap/osd/OSDVerboseInfoOnZap", _("Show other extra info in OSD when zapping"));
	new eListBoxEntryCheck(&list, _("Seconds in OSD clock"), "/ezap/osd/clockSeconds", _("Seconds in OSD clock"));
	new eListBoxEntryCheck(&list, _("No picture in radio/mp3"), "/ezap/osd/hidebginradiomode", _("No background picture in radio/mp3 mode"));
	new eListBoxEntryCheck(&list, _("Listbox OSD main menu"), "/ezap/osd/simpleMainMenu", _("Show the Main menu in normal listbox style"));
	CONNECT((new eListBoxEntryCheck(&list,_("Serviceselector help buttons"),"/ezap/serviceselector/showButtons",_("Show coloured help buttons in service selector")))->selected, ExtraOSDSetup::colorbuttonsChanged );
	if(eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite)
	{
		new eListBoxEntryCheck(&list, _("Show Sat position"), "/extras/showSatPos", _("Show satellite position in the infobar"));
	}
	new eListBoxEntryCheck(&list, _("Auto show Infobar"), "/ezap/osd/showOSDOnEITUpdate", _("Always show infobar when new event info is available"));
	new eListBoxEntryCheck(&list, _("Show remaining Time"), "/ezap/osd/showCurrentRemaining", _("Show event remaining time in the infobar"));
	new eListBoxEntryCheck(&list, _("Hide shortcut icons"), "/ezap/osd/hideshortcuts", _("Don't show shortcut icons in menus"));
	new eListBoxEntryCheck(&list, _("Skip confirmations"), "/elitedvb/extra/profimode", _("Skip some confirmations"));
	new eListBoxEntryCheck(&list, _("Hide error windows"), "/elitedvb/extra/hideerror", _("Don't show zap error messages like service not found"));
	if(eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000)
	{
		CONNECT((new eListBoxEntryCheck(&list,_("Enable fast zapping"),"/elitedvb/extra/fastzapping",_("Enables faster zapping.. but with visible sync")))->selected, ExtraOSDSetup::fastZappingChanged);
	}
	new eListBoxEntryCheck(&list, _("Enable Zapping History"), "/elitedvb/extra/extzapping", _("Don't care about actual mode when zapping in history list"));
	new eListBoxEntryCheck(&list, _("Auto bouquet change"), "/elitedvb/extra/autobouquetchange", _("Change into next bouquet when end of current bouquet is reached"));

	timeout_infobar = new eListBoxEntryMulti(&list, _("infobar timeout (left, right)"));
	timeout_infobar->add((eString)"  " + eString().sprintf(_("Infobar timeout %d sec"), 2) + (eString)" >", 2);
	timeout_infobar->add((eString)"< " + eString().sprintf(_("Infobar timeout %d sec"), 3) + (eString)" >", 3);
	timeout_infobar->add((eString)"< " + eString().sprintf(_("Infobar timeout %d sec"), 4) + (eString)" >", 4);
	timeout_infobar->add((eString)"< " + eString().sprintf(_("Infobar timeout %d sec"), 5) + (eString)" >", 5);
	timeout_infobar->add((eString)"< " + eString().sprintf(_("Infobar timeout %d sec"), 6) + (eString)" >", 6);
	timeout_infobar->add((eString)"< " + eString().sprintf(_("Infobar timeout %d sec"), 7) + (eString)" >", 7);
	timeout_infobar->add((eString)"< " + eString().sprintf(_("Infobar timeout %d sec"), 8) + (eString)" >", 8);
	timeout_infobar->add((eString)"< " + eString().sprintf(_("Infobar timeout %d sec"), 9) + (eString)" >", 9);
	timeout_infobar->add((eString)"< " + eString().sprintf(_("Infobar timeout %d sec"), 10) + (eString)" >", 10);
	timeout_infobar->add((eString)"< " + eString().sprintf(_("Infobar timeout %d sec"), 11) + (eString)" >", 11);
	timeout_infobar->add((eString)"< " + eString().sprintf(_("Infobar timeout %d sec"), 12) + (eString)"  ", 12);
	int timeoutInfobar = 6;
	eConfig::getInstance()->getKey("/enigma/timeoutInfobar", timeoutInfobar);
	timeout_infobar->setCurrent(timeoutInfobar);
	CONNECT(list.selchanged, ExtraOSDSetup::selInfobarChanged);
}

void ExtraOSDSetup::improvedBERChanged(bool b)
{
	eFrontend::getInstance()->setBERMode((int)b);
}

void ExtraOSDSetup::colorbuttonsChanged(bool b)
{
	eServiceSelector *sel = eZap::getInstance()->getServiceSelector();
	sel->setStyle(sel->getStyle(), true);
}

void ExtraOSDSetup::fastZappingChanged(bool b)
{
	Decoder::setFastZap(b);
}

void ExtraOSDSetup::selInfobarChanged(eListBoxEntryMenu* e)
{
	if(e == (eListBoxEntryMenu*)timeout_infobar)
	{
		eConfig::getInstance()->setKey("/enigma/timeoutInfobar", (int)e->getKey());
	}
}

void ExtraOSDSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
