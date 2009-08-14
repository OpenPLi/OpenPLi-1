/*
 * enigma_scan.cpp
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
 * $Id: enigma_scan.cpp,v 1.27 2009/02/07 10:06:31 dbluelle Exp $
 */

#include <enigma_scan.h>

#include <satconfig.h>
#include <rotorconfig.h>
#include <scan.h>
#include <satfind.h>
#include <tpeditwindow.h>
#include <enigma_plugins.h>
#include <lib/base/i18n.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/frontend.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>
#include <lib/gui/emessage.h>
#include <lib/system/info.h>

#define MENUNAME N_("Satellites and transponders")

class eZapScanFactory : public eCallableMenuFactory
{
public:
	eZapScanFactory() : eCallableMenuFactory("eZapScan", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eZapScan;
	}
	
	bool isAvailable()
	{
		return eSystemInfo::getInstance()->getFEType() != eSystemInfo::feUnknown;
	}
};

eZapScanFactory eZapScan_factory;

eZapScan::eZapScan()
	:eSetupWindow(_(MENUNAME),
	eSystemInfo::getInstance()->getFEType()
		== eSystemInfo::feSatellite ? 10 : 8, 400)
{
	init_eZapScan();
}
void eZapScan::init_eZapScan()
{
	int entry=0;
	valign();
	if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )  // only when a sat box is avail we shows a satellite config
	{
		CONNECT((new eListBoxEntryMenu(&list, _("Satellite Configuration"), eString().sprintf("(%d) %s", ++entry, _("open satellite config"))))->selected, eZapScan::sel_satconfig);
		CONNECT((new eListBoxEntryMenu(&list, _("Satfind"), eString().sprintf("(%d) %s", ++entry, _("open the satfinder"))))->selected, eZapScan::sel_satfind);
		// Only show Motor setup when Motor is enabled
		eSatelliteConfigurationManager satconfig(false);
		if (satconfig.getRotorEnabled())
		{
			CONNECT((new eListBoxEntryMenu(&list, _("Rotor Setup"), eString().sprintf("(%d) %s", ++entry, _("open Rotor Setup"))))->selected, eZapScan::sel_rotorConfig);
		}
		CONNECT((new eListBoxEntryMenu(&list, _("Transponder Edit"), eString().sprintf("(%d) %s", ++entry, _("for automatic scan"))))->selected, eZapScan::sel_transponderEdit);
	}
	else if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feTerrestrial )
	{
		CONNECT((new eListBoxEntryMenu(&list, _("Signalfind"), eString().sprintf("(%d) %s", ++entry, _("open the signalfinder"))))->selected, eZapScan::sel_satfind);
		(new eListBoxEntryCheck(&list, _("Disable 5V"), "/elitedvb/DVB/config/disable_5V", _("disable 5V for passive terrerstrial antennas")))
			->selected.connect( slot(*eFrontend::getInstance(), &eFrontend::setTerrestrialAntennaVoltage) );
	}
	if ( eSystemInfo::getInstance()->getFEType() != eSystemInfo::feCable )
		new eListBoxEntryMenuSeparator(&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
	CONNECT((new eListBoxEntryMenu(&list, _("Automatic Transponder Scan"), eString().sprintf("(%d) %s", ++entry, _("open automatic transponder scan"))))->selected, eZapScan::sel_autoScan);
	if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )  // only when a sat box is avail we shows a satellite config
		CONNECT((new eListBoxEntryMenu(&list, _("Automatic Multisat Scan"), eString().sprintf("(%d) %s", ++entry, _("open automatic multisat transponder scan"))))->selected, eZapScan::sel_multiScan);

	CONNECT((new eListBoxEntryMenu(&list, _("Manual Transponder Scan"), eString().sprintf("(%d) %s", ++entry, _("open manual transponder scan"))))->selected, eZapScan::sel_manualScan);
	if ( eFrontend::getInstance()->canBlindScan() && eZapPlugins(eZapPlugins::StandardPlugin).execPluginByName("enigma_blindscan.cfg", true) == "OK" )
		CONNECT((new eListBoxEntryMenu(&list, _("Satellite Blindscan"), eString().sprintf("(%d) %s", ++entry, _("open transponder blindscan"))))->selected, eZapScan::sel_blindScan);

	new eListBoxEntryMenuSeparator(&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true);

	new eListBoxEntryCheck(&list, _("Disable background scanning"), "/elitedvb/extra/disableSDTScan", _("Don't look for new services in the background"));
}

void eZapScan::sel_satfind()
{
	hide();
	eSatfind s(eFrontend::getInstance());
	s.show();
	s.exec();
	s.hide();
	show();
}

void eZapScan::sel_autoScan()
{
#ifndef DISABLE_LCD
	TransponderScan setup(LCDTitle, LCDElement, TransponderScan::stateAutomatic);
#else
	TransponderScan setup(0,0,TransponderScan::stateAutomatic);
#endif
	hide();
	setup.exec();
	show();
}

void eZapScan::sel_multiScan()
{
#ifndef DISABLE_LCD
	TransponderScan setup(LCDTitle, LCDElement, TransponderScan::stateMulti);
#else
	TransponderScan setup(0,0,TransponderScan::stateMulti);
#endif
	hide();
	setup.exec();
	show();
}

void eZapScan::sel_manualScan()
{
#ifndef DISABLE_LCD
	TransponderScan setup(LCDTitle, LCDElement, TransponderScan::stateManual);
#else
	TransponderScan setup(0,0,TransponderScan::stateManual);
#endif
	hide();
	setup.exec();
	show();
}

void eZapScan::sel_blindScan()
{
	hide();
	eZapPlugins(eZapPlugins::StandardPlugin).execPluginByName("enigma_blindscan.cfg");
	show();
}

void eZapScan::sel_satconfig()
{
	hide();
	eSatelliteConfigurationManager satconfig;
#ifndef DISABLE_LCD
	satconfig.setLCD(LCDTitle, LCDElement);
#endif
	satconfig.show();
	satconfig.exec();
	satconfig.hide();
	show();
}

eLNB* eZapScan::getRotorLNB(int silent)
{
	int c=0;
	std::list<eLNB>::iterator RotorLnb = eTransponderList::getInstance()->getLNBs().end();
	std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin());
	for (; it != eTransponderList::getInstance()->getLNBs().end(); it++ )
	{
		if ( it->getDiSEqC().DiSEqCMode == eDiSEqC::V1_2 )
		{
			if (!c++)
				RotorLnb=it;
		}
	}
	if ( c > 1 )  // we have more than one LNBs with DiSEqC 1.2
	{
		eMessageBox::ShowBox(_("DiSEqC 1.2 is enabled on more than one LNB, please select the LNB the motor is connected to"), _("Info"), eMessageBox::iconWarning|eMessageBox::btOK );
		eLNBSelector sel;
		sel.show();
		int ret = sel.exec();
		sel.hide();
		return (eLNB*) ret;
	}
	else if ( !c )
	{
		if (!silent)
		{
			eMessageBox::ShowBox( _("Found no LNB with DiSEqC 1.2 enabled,\nplease goto Satellite Config first, and enable DiSEqC 1.2"), _("Warning"), eMessageBox::iconWarning|eMessageBox::btOK );
		}
		return 0;
	}
	else // only one lnb with DiSEqC 1.2 is found.. this is correct :)
		return &(*RotorLnb);
}

void eZapScan::sel_transponderEdit()
{
	hide();
	eTransponderEditWindow wnd;
#ifndef DISABLE_LCD
	wnd.setLCD(LCDTitle, LCDElement);
#endif
	wnd.show();
	wnd.exec();
	wnd.hide();
	show();
}

void eZapScan::sel_rotorConfig()
{
	hide();
	eLNB* lnb = getRotorLNB(0);
	if (lnb)
	{
		RotorConfig c(lnb);
#ifndef DISABLE_LCD
		c.setLCD( LCDTitle, LCDElement );
#endif
		c.show();
		c.exec();
		c.hide();
	}
	show();
}

int eZapScan::getDiseqc12LnbCount()
{
	int count = 0;
	std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin());
	for (; it != eTransponderList::getInstance()->getLNBs().end(); it++ )
	{
		if ( it->getDiSEqC().DiSEqCMode == eDiSEqC::V1_2 )
		{
			count++;
		}
	}
	return count;
}

void eZapScan::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

eLNBSelector::eLNBSelector()
	:eListBoxWindow<eListBoxEntryText>(_("Select LNB"), 5, 300, true)
{
	init_eLNBSelector();
}
void eLNBSelector::init_eLNBSelector()
{
	valign();
	int cnt=0;
	for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin()); it != eTransponderList::getInstance()->getLNBs().end(); it++, cnt++ )
	{
		if ( it->getDiSEqC().DiSEqCMode == eDiSEqC::V1_2 )
			new eListBoxEntryText( &list, eString().sprintf("LNB %d", cnt), (void*)&(*it), 0, eString().sprintf(_("use LNB %d for Motor"), cnt ).c_str());
	}
	CONNECT( list.selected, eLNBSelector::selected );
}

void eLNBSelector::selected( eListBoxEntryText *e )
{
	if ( e && e->getKey() )
		close((int)e->getKey());
	else
		close(0);
}

