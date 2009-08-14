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

#ifndef __setup_epg_h
#define __setup_epg_h

#include <lib/gui/combobox.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/textinput.h>
#include <lib/dvb/epgstore.h>
#include <lib/gui/ePLiWindow.h>
#include <enigma_mount.h>
#include <callablemenu.h>

class EpgSetup : public ePLiWindow, public eCallableMenu
{
public:
	EpgSetup ();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);

private:
	eButton *rescan;
	eLabel *lblStores;
	eLabel *lblStoreLocation;
	eLabel *lblStorageCheck;
	eLabel *lblNrOfDays;
	eTextInputField *tiStoreLocation;
	eComboBox *comStores;
	eMountSelectionComboBox *comStoreLocation;
	eComboBox *comNrOfDays;
	eCheckbox *chkMhwEPG;
	eCheckbox *chkDishEPG;
	eCheckbox *chkLimitEPG;
	int oldStore;
	eString storeLocations[eEPGStore::NR_OF_STORES];
	eString oldStoreLocations[eEPGStore::NR_OF_STORES];
	eDevMountMgr *devMountMgr;
	eNetworkMountMgr *networkMountMgr;
	int nrDevMounts;
	int nrNetworkMounts;
	
	void okPressed();
	void rescanPressed();
	void epgStoreChanged(eListBoxEntryText *sel);
	void epgLocationChanged(eListBoxEntryText *sel);
	void checkStorage();
};

class MultiEPGSetup : public ePLiWindow, public eCallableMenu
{
	public:
		MultiEPGSetup();

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
		
	private:
		eNumber *eNumServices;
		eLabel *eNumServicesLabel;
		eComboBox *eFontIncrement;
		eLabel *eFontIncrementLabel;
		eCheckbox *chkEPGColours;

		void okPressed();
};

#endif
