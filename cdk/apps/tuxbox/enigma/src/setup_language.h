/* * PLi's setup plugin for dreambox
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
 */

#ifndef __setup_language_h
#define __setup_language_h

#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/statusbar.h>
#include <lib/base/console.h>
#include <lib/gui/combobox.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/ePLiWindow.h>
#include <callablemenu.h>
#include <lib/gui/textinput.h>

/**
 * language Table
 *
 * array[][0]    Menutext
 * array[][1-4]  Language Text in sysconfig
 *
 * ./enigma/include/lib/dvb/iso639.h
 *
 */


#define   langArrayLength 29
#define   langArrayDepth 9
static    eString langArray[][9]  ={    {"Default"}
					,{"Arabic","Arab"}
					,{"Basque"}
					,{"Danish"}
					,{"Croatian"}
					,{"Czech", "zvuk"}
					,{"Dutch"}
					,{"English","Englisch","Audio 3","stereo englisch"}
					,{"English (AC3)"}
					,{"Finnish"}
					,{"French"}
					,{"German","Deutsch","stereo deutsch"}
					,{"Greek","greek"}
					,{"Greek Modern","Modern","Modern(1453-)","1453"}
					,{"Hungarian"}
					,{"Letzeburgesch"}
					,{"Italian","Italiano"}
					,{"Norwegian"}
					,{"Polish","Polski"}
					,{"Portuguese"}
					,{"Persian"}
					,{"Romanian"}
					,{"Russian"}
					,{"Serbian"}
					,{"Slovak","slk","slo"}
					,{"Spanish"}
					,{"Swedish"}
					,{"Turkish","Audio 2"}
					,{"Original","Originalton","Tonoption 2","orj","dos","ory","org","esl","qaa"}
				  };

class LanguageSetup : public ePLiWindow, public eCallableMenu
{
	public:
		LanguageSetup();
		void okPressed();

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);

		eLabel* oLabel_1;
		eLabel* oLabel_2;
		eLabel* oLabel_3;
		eLabel* oLabel_4;
		eComboBox* oLang_1;
		eComboBox* oLang_2;
		eComboBox* oLang_3;
		eComboBox* oLang_4;
#if 0 // Preferred EPG languages, needs rework	
		eLabel* lEPGLangs;
		eLabel* lEPGLang3;
		eLabel* lEPGLang4;
		eComboBox* comEPGLang3; 
		eTextInputField* txtEPGLang4;
#endif		
	private:
		std::vector<eComboBox*> vCombos;
		std::vector<int> vSelected;

		void FillDropDowns(eComboBox*& oCombo);
		void setLanguage(eComboBox*& oCombo, const int& iLanguageNr);

		bool AllreadySelected(const int iNumber);
};

#endif /* __setup_language_h */
