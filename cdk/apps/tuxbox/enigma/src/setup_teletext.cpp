/*
 *  PLi extension to Enigma: teletext settings
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

#include <setup_teletext.h>
#include <lib/dvb/decoder.h>

#ifndef TUXTXT_CFG_STANDALONE
extern "C" int  tuxtxt_stop();
extern "C" void tuxtxt_close();
extern "C" int  tuxtxt_init();
extern "C" void tuxtxt_start(int tpid);
#endif

#define MENUNAME N_("Teletext")

class TeletextSetupFactory : public eCallableMenuFactory
{
public:
	TeletextSetupFactory() : eCallableMenuFactory("TeletextSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new TeletextSetup;
	}
};

TeletextSetupFactory TeletextSetup_factory;

TeletextSetup::TeletextSetup():
	ePLiWindow(_(MENUNAME), 400),
	chk_caching(0)
{
	int enableCaching(0);
	eConfig::getInstance()->getKey("/ezap/extra/teletext_caching", enableCaching);
	
#ifndef TUXTXT_CFG_STANDALONE
	chk_caching = new eCheckbox(this, enableCaching, 1);
	chk_caching->setText(_("Enable teletext caching"));
	chk_caching->move(ePoint(10, yPos()));
	chk_caching->resize(eSize(380, widgetHeight()));
	chk_caching->setHelpText(_("Enable caching of teletext pages to access them faster (also needed for web interface)"));
	chk_caching->loadDeco();
	nextYPos(35);
#endif
	
	eLabel *lbl_note = new eLabel(this);
	lbl_note->setText(_("For more options:\npress MENU key in teletext"));
	lbl_note->move(ePoint(10, yPos()));
	lbl_note->resize(eSize(380, 2 * widgetHeight()));
	lbl_note->loadDeco();

	buildWindow();
	CONNECT(bOK->selected, TeletextSetup::okPressed);
}

void TeletextSetup::okPressed()
{
#ifndef TUXTXT_CFG_STANDALONE
	bool checked = chk_caching->isChecked();
	
	eConfig::getInstance()->setKey("/ezap/extra/teletext_caching", checked ? 1 : 0);

	if(checked)
	{
		tuxtxt_init();
		if(Decoder::current.tpid != -1)
		{
			tuxtxt_start(Decoder::current.tpid);
		}
	}
	else
	{
		if(Decoder::current.tpid != -1)
		{
			tuxtxt_stop();
		}
		tuxtxt_close();
	}
#endif
		
	close(0);
}

void TeletextSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
