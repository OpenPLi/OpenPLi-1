/*
 * PLi's setup plugin for dreambox
 * Copyright (c) 2004 PLi <peter@dreamvcr.com>
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
 *
 *
 * History
 * ------------
 * 21/03/2005    murks@i-have-a-dreambox.com	changed language selection to array
 * 						see in header file
 */

#include "setup_language.h"

#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


#include <lib/base/i18n.h>
#include <plugin.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/eaudio.h>
#include <lib/gui/elabel.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/emessage.h>
#include <lib/gui/eskin.h>
#include <lib/driver/rc.h>
#include <lib/system/econfig.h>
#include <lib/gui/combobox.h>
#include <lib/gui/listbox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/textinput.h>

#include <fstream>

#define MENUNAME N_("Preferred audio languages")

const char MAX_LANG = 39; // VIZ epgwindow.cpp
extern eString ISOtbl[MAX_LANG][2];

class LanguageSetupFactory : public eCallableMenuFactory
{
public:
	LanguageSetupFactory() : eCallableMenuFactory("LanguageSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new LanguageSetup;
	}
};

LanguageSetupFactory LanguageSetup_factory;

//------------------------------------------------------------------------

LanguageSetup::LanguageSetup() : 
	ePLiWindow(_(MENUNAME), 315),
	oLabel_1(0), oLabel_2(0), oLabel_3(0), oLabel_4(0),
	oLang_1(0), oLang_2(0), oLang_3(0), oLang_4(0)
{
	oLabel_1 = new eLabel (this);
	oLabel_1->setText ((eString)_("Language") + " 1 :");
	oLabel_1->move (ePoint (10, yPos()));
	oLabel_1->resize (eSize (150, widgetHeight()));

	oLang_1 = new eComboBox(this, 4, oLabel_1);
	oLang_1->move(ePoint(130, yPos()));
	oLang_1->resize(eSize(175, widgetHeight()));
	oLang_1->loadDeco();
	oLang_1->setHelpText(_("Select the first language"));
	vCombos.push_back(oLang_1);

	nextYPos(35);
	oLabel_2 = new eLabel (this);
	oLabel_2->setText ((eString)_("Language") + " 2 :");
	oLabel_2->move (ePoint (10, yPos()));
	oLabel_2->resize (eSize (150, widgetHeight()));

	oLang_2 = new eComboBox(this, 4, oLabel_2);
	oLang_2->move(ePoint(130, yPos()));
	oLang_2->resize(eSize(175, widgetHeight()));
	oLang_2->loadDeco();
	oLang_2->setHelpText(_("Select the second language"));
	vCombos.push_back(oLang_2);

	nextYPos(35);
	oLabel_3 = new eLabel (this);
	oLabel_3->setText ((eString)_("Language") + " 3 :");
	oLabel_3->move (ePoint (10, yPos()));
	oLabel_3->resize (eSize (150, widgetHeight()));

	oLang_3 = new eComboBox(this, 4, oLabel_3);
	oLang_3->move(ePoint(130, yPos()));
	oLang_3->resize(eSize(175, widgetHeight()));
	oLang_3->loadDeco();
	oLang_3->setHelpText(_("Select the third language"));
	vCombos.push_back(oLang_3);

	nextYPos(35);
	oLabel_4 = new eLabel (this);
	oLabel_4->setText ((eString)_("Language") + " 4 :");
	oLabel_4->move (ePoint (10, yPos()));
	oLabel_4->resize (eSize (150, widgetHeight()));

	oLang_4 = new eComboBox(this, 4, oLabel_3);
	oLang_4->move(ePoint(130, yPos()));
	oLang_4->resize(eSize(175, widgetHeight()));
	oLang_4->loadDeco();
	oLang_4->setHelpText(_("Select the fourth language"));
	vCombos.push_back(oLang_4);

#if 0 // Preferred EPG languages, needs rework	
	nextYPos(35);
	
	lEPGLangs = new eLabel(this);
	lEPGLangs->move(ePoint(10, yPos()));
	lEPGLangs->resize(eSize(245, widgetHeight())); 
	lEPGLangs->setText(_("Alternative EPG Languages:"));
	
	nextYPos(35);
	eString EpgLang3 = "eng";
	eConfig::getInstance()->getKey("/extras/epgl3", EpgLang3);
	
	lEPGLang3 = new eLabel(this);
   	lEPGLang3->move(ePoint(10, yPos()));
   	lEPGLang3->resize(eSize(150, widgetHeight())); 
   	lEPGLang3->setText((eString)_("Language") + " 1 :");
   	
   	comEPGLang3 = new eComboBox(this, 3, lEPGLang3);
	comEPGLang3->move(ePoint(130, yPos()));
	comEPGLang3->resize(eSize (60, widgetHeight()));
	comEPGLang3->setHelpText(_("Select EPG language, then restart"));
	comEPGLang3->loadDeco();
	
	int item = 0;
	for(int i = 0; i < MAX_LANG; i++)
	{
		new eListBoxEntryText(*comEPGLang3, ISOtbl[i][1], (void*)i);
		if( ISOtbl[i][1] == EpgLang3)
		        item = i;
	}
	comEPGLang3->setCurrent((void *)item);
	
	nextYPos(35);
	eString EpgLang4 = "";
	eConfig::getInstance()->getKey("/extras/epgl4", EpgLang4);
	
	lEPGLang4 = new eLabel(this);
	lEPGLang4->move(ePoint(10, yPos()));
	lEPGLang4->resize(eSize(150, widgetHeight())); 
	lEPGLang4->setText((eString)_("Language") + " 2 :");
   	
	txtEPGLang4=new eTextInputField(this,lEPGLang4); 
	txtEPGLang4->setMaxChars(3); 
	txtEPGLang4->move(ePoint(130, yPos())); 
	txtEPGLang4->resize(eSize(50, widgetHeight()));
	txtEPGLang4->loadDeco(); 
	txtEPGLang4->setText(EpgLang4);
	txtEPGLang4->setHelpText(_("Enter EPG language (3 chars)"));
#endif

	vSelected.clear();

	buildWindow();
	CONNECT (bOK->selected, LanguageSetup::okPressed);

	for (unsigned int iCount(0); iCount < vCombos.size(); iCount++) {
		FillDropDowns(vCombos[iCount]);
		setLanguage(vCombos[iCount], iCount + 1);
	}
}
//------------------------------------------------------------------------

void LanguageSetup::okPressed()
{
#ifdef DEBUG
	printf ("okPressed\n");
#endif

	std::stringstream oNewKey("");

	for (unsigned int iCount(0); iCount < vCombos.size(); iCount++) {
		int key = ((int)vCombos[iCount]->getCurrent()->getKey());

		if (key == 0 || key >= langArrayLength || !langArray[key])
			continue;
		for (unsigned int x = 0; x < langArrayDepth; x++) {
			if (!langArray[key][x])
				break;
			oNewKey << langArray[key][x] << "#";
		}
	}

	if (oNewKey.str().length() > 0) {
		eConfig::getInstance()->setKey("/extras/audiochannelspriority", oNewKey.str().c_str());
	}
	
#if 0 // Preferred EPG languages, needs rework	
	eConfig::getInstance()->setKey("/extras/epgl3", (ISOtbl[(const int)comEPGLang3->getCurrent()->getKey()][1]).c_str());
	eConfig::getInstance()->setKey("/extras/epgl4", (txtEPGLang4->getText()).c_str());
#endif

	close (0);
}
//------------------------------------------------------------------------

void LanguageSetup::FillDropDowns(eComboBox*& oCombo)
{
   if (oCombo) {
		for (unsigned int i = 0;  i < langArrayLength; i++) {
			new eListBoxEntryText(*oCombo, langArray[i][0], (void*)i);
		}
		oCombo->setCurrent(0);
	}
}
//------------------------------------------------------------------------

void LanguageSetup::setLanguage(eComboBox*& oCombo, const int& iLanguageNr)
{
	char *audiochannelspriority = 0;
	eConfig::getInstance()->getKey("/extras/audiochannelspriority", audiochannelspriority);

	if (audiochannelspriority)
	{
		int idxCount = 0;
		int idxFound = -1;

		char *audiochannel = strtok(audiochannelspriority, "#");

		while (audiochannel)
		{
			for (uint i = 0; i < langArrayLength; i++)
			{
				if (langArray[i][0] == audiochannel)
				{
					idxCount++;
					idxFound = i;
					break;
				}
			}
			if (idxCount == iLanguageNr)
			{
				break;
			}
			audiochannel = strtok(NULL, "#");
		}

		if (idxFound != -1 && !AllreadySelected(idxFound)) {
			oCombo->setCurrent((void*)idxFound);
			vSelected.push_back(idxFound);
		}
		free(audiochannelspriority);
	}
}
//------------------------------------------------------------------------

bool LanguageSetup::AllreadySelected(const int iNumber)
{
	bool fResult = false;
	for (unsigned int i = 0; i < vSelected.size(); i++) {
		if (iNumber == vSelected[i]) {
			fResult = true;
			break;
		}
	}
	return fResult;
}

void LanguageSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
