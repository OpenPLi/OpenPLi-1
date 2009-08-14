/*
 *  PLi extension to Enigma: web interface settings
 *
 *  Copyright (C) 2007 dAF2000 <daf2000@ditadres.nl>
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

#include <setup_webif.h>
#include <enigma.h>
#include <lib/system/econfig.h>

#define MENUNAME N_("Web interface")

class WebIfSetupFactory : public eCallableMenuFactory
{
public:
	WebIfSetupFactory() : eCallableMenuFactory("WebIfSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new WebIfSetup;
	}
};

WebIfSetupFactory WebIfSetup_factory;

WebIfSetup::WebIfSetup():
	ePLiWindow(_(MENUNAME), 390),
	lblPort(NULL), numPort(NULL), eShowChanEPG(NULL), eLockWebIf(NULL), eZapStream(NULL)
{
	int webifport = 80;
	int lockwebif = 0;
	int iShowChanEPG = 0;
	int zapstream = 0;

	eConfig::getInstance()->getKey("/elitedvb/network/webifport", webifport);
	eConfig::getInstance()->getKey("/ezap/webif/lockWebIf", lockwebif);
	eConfig::getInstance()->getKey("/ezap/webif/showChannelEPG", iShowChanEPG);
	eConfig::getInstance()->getKey("/ezap/webif/useZapStream", zapstream);

	lblPort = new eLabel(this);
	lblPort->setText(_("Web interface port:"));
	lblPort->move(ePoint(10, yPos()));
	lblPort->resize(eSize(300, widgetHeight()));

	numPort = new eNumber(this, 1, 0, 65536, 5, 0, 0, lblPort);
	numPort->move(ePoint(200, yPos()));
	numPort->resize(eSize(70, widgetHeight()));
	numPort->setFlags(eNumber::flagDrawPoints);
	numPort->setHelpText(_("Enter port of the Web Interface (0..9, left, right)"));
	numPort->setNumber(webifport);
	numPort->loadDeco();
	
	nextYPos(35);
	eLockWebIf = new eCheckbox(this, lockwebif, 1);
	eLockWebIf->setText(_("Use HTTP authentication"));
	eLockWebIf->move(ePoint(10, yPos()));
	eLockWebIf->resize(eSize(370, widgetHeight()));
	eLockWebIf->setHelpText(_("Enables the http (user/password) authentication"));

	nextYPos();
	eShowChanEPG = new eCheckbox(this, iShowChanEPG, 1);
	eShowChanEPG->setText(_("Show EPG in webif channel lists"));
	eShowChanEPG->move(ePoint(10, yPos()));
	eShowChanEPG->resize(eSize(370, widgetHeight()));
	eShowChanEPG->setHelpText(_("Show EPG in webif/webx-tv channel lists"));

	nextYPos();
	eZapStream = new eCheckbox(this, zapstream, 1);
	eZapStream->setText(_("Use zapstream"));
	eZapStream->move(ePoint(10, yPos()));
	eZapStream->resize(eSize(370, widgetHeight()));
	eZapStream->setHelpText(_("Use the more intelligent 'zapstream', instead of 'streamts'"));

	buildWindow();
	CONNECT(bOK->selected, WebIfSetup::okPressed);
}

void WebIfSetup::okPressed()
{
	int oldport = 80;
	int newport = numPort->getNumber();
	int oldauth = 0;
	int newauth = eLockWebIf->isChecked();
	int zapstream = eZapStream->isChecked();

	eConfig::getInstance()->getKey("/elitedvb/network/webifport", oldport);
	eConfig::getInstance()->setKey("/elitedvb/network/webifport", newport);
	
	eConfig::getInstance()->getKey("/ezap/webif/lockWebIf", oldauth);
	eConfig::getInstance()->setKey("/ezap/webif/lockWebIf", newauth);
	eConfig::getInstance()->setKey("/ezap/webif/useZapStream", zapstream);

	if((oldport != newport) || (oldauth != newauth))
	{
		eZap::getInstance()->reconfigureHTTPServer();
	}
	
	eConfig::getInstance()->setKey("/ezap/webif/showChannelEPG", 
		eShowChanEPG->isChecked() ? 1 : 0);
	
	close(0);
}

void WebIfSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
