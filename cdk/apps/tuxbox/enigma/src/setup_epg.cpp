#include "setup_epg.h"

#include <plugin.h>
#include <lib/dvb/epgcache.h>

#define MENUNAMEEPG N_("EPG")
#define MENUNAMEMULTI N_("MultiEPG")

class EpgSetupFactory : public eCallableMenuFactory
{
public:
	EpgSetupFactory() : eCallableMenuFactory("EpgSetup", MENUNAMEEPG) {}
	eCallableMenu *createMenu()
	{
		return new EpgSetup;
	}
};

EpgSetupFactory EpgSetup_factory;

class MultiEPGSetupFactory : public eCallableMenuFactory
{
public:
	MultiEPGSetupFactory() : eCallableMenuFactory("MultiEPGSetup", MENUNAMEMULTI) {}
	eCallableMenu *createMenu()
	{
		return new MultiEPGSetup;
	}
};

MultiEPGSetupFactory MultiEPGSetup_factory;

EpgSetup::EpgSetup() : 
	ePLiWindow(_(MENUNAMEEPG), 440)
{
	int epgMaxHours = 4 * 24;
	eConfig::getInstance()->getKey("/extras/epghours", epgMaxHours);

#ifdef ENABLE_MHW_EPG
	int enableMhwEPG(1);
	eConfig::getInstance()->getKey("/extras/mhwepg", enableMhwEPG);
#endif

#ifdef ENABLE_DISH_EPG
	int enableDishEPG(0);
	eConfig::getInstance()->getKey("/extras/disheepg", enableDishEPG);
#endif

	int enableEPGLimit(1);
	eConfig::getInstance()->getKey("/enigma/epgStoreLimit", enableEPGLimit);

	devMountMgr = eDevMountMgr::getInstance();
	networkMountMgr = eNetworkMountMgr::getInstance();

	oldStore = eEPGStore::MEM_STORE;
	storeLocations[eEPGStore::SQLITE_STORE] = eEPGSqlStore::getDefaultStorageDir();
	storeLocations[eEPGStore::MEM_STORE] = eEPGMemStore::getDefaultStorageDir();
	
	eConfig::getInstance()->getKey("/enigma/epgStore", oldStore);
	eConfig::getInstance()->getKey("/enigma/epgSQLiteDir", storeLocations[eEPGStore::SQLITE_STORE]);
	eConfig::getInstance()->getKey("/enigma/epgMemStoreDir", storeLocations[eEPGStore::MEM_STORE]);
	
	oldStoreLocations[eEPGStore::SQLITE_STORE] = storeLocations[eEPGStore::SQLITE_STORE];
	oldStoreLocations[eEPGStore::MEM_STORE] = storeLocations[eEPGStore::MEM_STORE];
	
	// Create combobox for EPG type
	lblStores = new eLabel( this );
	comStores = new eComboBox( this, 2, lblStores );
	
	lblStores->move(ePoint( 10, yPos() ));
	lblStores->resize(eSize( 170, widgetHeight()));
	lblStores->setText((eString)_("EPG data store") + (eString)":" );
	lblStores->loadDeco();
	
	comStores->move(ePoint(225, yPos()));
	comStores->resize(eSize(205, widgetHeight()));
	comStores->setHelpText(_("Select if you want the default Enigma EPG or SQLite database EPG"));
	comStores->clear();
	
	new eListBoxEntryText(*comStores, _("Enigma EPG"), (void *) eEPGStore::MEM_STORE );
	new eListBoxEntryText(*comStores, _("SQLite database"), (void *) eEPGStore::SQLITE_STORE );

	comStores->setCurrent((void*) oldStore);
	comStores->loadDeco();
	CONNECT(comStores->selchanged, EpgSetup::epgStoreChanged);

	nextYPos(35);
	lblStoreLocation = new eLabel(this);
	lblStoreLocation->move(ePoint(10, yPos()));
	lblStoreLocation->resize(eSize(240, widgetHeight()));
	lblStoreLocation->loadDeco();

	// Create combobox for storage location
	comStoreLocation = new eMountSelectionComboBox(
		this, 4, lblStoreLocation, 
		eMountSelectionComboBox::ShowDevices | 
		eMountSelectionComboBox::ShowNetwork |
		eMountSelectionComboBox::ShowCustomLocation);

	comStoreLocation->move(ePoint(225, yPos()));
	comStoreLocation->resize(eSize(205, widgetHeight()));
	comStoreLocation->setHelpText(_("Select EPG store location"));

	comStoreLocation->setCurrentLocation(storeLocations[oldStore]);
	comStoreLocation->loadDeco();
	CONNECT(comStoreLocation->selchanged, EpgSetup::epgLocationChanged);
	
	nextYPos(35);
	lblStorageCheck = new eLabel(this);
	lblStorageCheck->move(ePoint(10, yPos()));
	lblStorageCheck->resize(eSize(420, widgetHeight()));
	lblStorageCheck->loadDeco();

	epgStoreChanged(comStores->getCurrent());
	epgLocationChanged(comStoreLocation->getCurrent());

	nextYPos(35);
	lblNrOfDays = new eLabel(this);
	lblNrOfDays->move(ePoint(10, yPos()));
	lblNrOfDays->resize(eSize(420, widgetHeight()));
	lblNrOfDays->setText(_("Number of EPG days:"));
	lblNrOfDays->loadDeco();

	comNrOfDays = new eComboBox(this, 3, lblNrOfDays);
	comNrOfDays->move(ePoint(225, yPos()));
	comNrOfDays->resize(eSize (60, widgetHeight()));
	comNrOfDays->setHelpText(_("Select number of EPG days"));
	comNrOfDays->loadDeco();

	for(int epgDays = 1; epgDays <= 11; ++epgDays)
	{
		new eListBoxEntryText(*comNrOfDays, eString().sprintf("%d", epgDays), (void*)(epgDays * 24));
	}

	comNrOfDays->setCurrent((void *)epgMaxHours);

#ifdef ENABLE_MHW_EPG
	nextYPos(35);
	chkMhwEPG = new eCheckbox(this, enableMhwEPG, 1);
	chkMhwEPG->setText(_("Enable MHW EPG"));
	chkMhwEPG->move(ePoint(10, yPos()));
	chkMhwEPG->resize(eSize(420, widgetHeight()));
	chkMhwEPG->setHelpText(_("Mediahighway EPG, activate swap space when using with multiple operators"));
	chkMhwEPG->loadDeco();
#endif

#ifdef ENABLE_DISH_EPG
	nextYPos(35);
	chkDishEPG = new eCheckbox(this, enableDishEPG, 1);
	chkDishEPG->setText(_("Enable DISH/BEV EEPG"));
	chkDishEPG->move(ePoint(10, yPos()));
	chkDishEPG->resize(eSize(420, widgetHeight()));
	chkDishEPG->setHelpText(_("DISH/BEV network EEPG, up to 9 days EPG"));
	chkDishEPG->loadDeco();
#endif

	nextYPos(35);
	chkLimitEPG = new eCheckbox(this, enableEPGLimit, 1);
	chkLimitEPG->setText(_("Limit Enigma EPG size"));
	chkLimitEPG->move(ePoint(10, yPos()));
	chkLimitEPG->resize(eSize(420, widgetHeight()));
	chkLimitEPG->setHelpText(_("Limit the Enigma EPG file to 5MB (recommended)"));
	chkLimitEPG->loadDeco();

	buildOKButton();

	rescan = new eButton (this);
	rescan->setText (_("Rescan EPG"));
	rescan->setShortcut ("blue");
	rescan->setShortcutPixmap ("blue");
	rescan->move( ePoint (140, yPos()));
	rescan->resize( eSize(160, 40) );
	rescan->setHelpText (_("Rescan EPG data on current transponder"));
	rescan->loadDeco ();
	CONNECT (rescan->selected, EpgSetup::rescanPressed);

	buildWindow();
	CONNECT (bOK->selected, EpgSetup::okPressed);
}

void EpgSetup::epgStoreChanged(eListBoxEntryText *sel)
{
	if(sel && lblStoreLocation && comStoreLocation)
	{
		int storeSelection = (int)sel->getKey();
		
		switch(storeSelection)
		{
			case eEPGStore::MEM_STORE:
				lblStoreLocation->setText((eString)_("Enigma EPG location") + (eString)":" );
				break;
				
			case eEPGStore::SQLITE_STORE:
				lblStoreLocation->setText((eString)_("SQLite EPG location") + (eString)":" );
				break;
				
			default:
				break;
		}
		
		comStoreLocation->setCurrentLocation(storeLocations[storeSelection]);
		epgLocationChanged(comStoreLocation->getCurrent());
	}
}

void EpgSetup::epgLocationChanged(eListBoxEntryText *sel)
{
	int storeSelection = (int)comStores->getCurrent()->getKey();

	comStoreLocation->locationChanged(sel);
	storeLocations[storeSelection] = comStoreLocation->getCurrentLocation();
	checkStorage();
}

void EpgSetup::checkStorage()
{
	if(comStores && lblStorageCheck)
	{
		eEPGStore::StorageCheckEnum storeResult = eEPGStore::STORAGE_NO_SPACE;
		
		switch((int)comStores->getCurrent()->getKey())
		{
			case eEPGStore::MEM_STORE:
				storeResult = eEPGMemStore::checkStorage(storeLocations[eEPGStore::MEM_STORE]);
				break;
				
			case eEPGStore::SQLITE_STORE:
				storeResult = eEPGSqlStore::checkStorage(storeLocations[eEPGStore::SQLITE_STORE]);
				break;
				
			default:
				break;
		}
		
		if(storeResult == eEPGStore::STORAGE_OK)
		{
			lblStorageCheck->setText((eString)_("This directory is suitable for storage"));
		}
		else if(storeResult == eEPGStore::STORAGE_NO_SPACE)
		{
			lblStorageCheck->setText((eString)_("This directory has not enough space"));
		}
		else if(storeResult == eEPGStore::STORAGE_NO_DIR)
		{
			lblStorageCheck->setText((eString)_("This directory does not exist"));
		}
	}
}

void EpgSetup::okPressed()
{
	eConfig::getInstance()->setKey("/enigma/epgMemStoreDir", storeLocations[eEPGStore::MEM_STORE]);
	eConfig::getInstance()->setKey("/enigma/epgSQLiteDir", storeLocations[eEPGStore::SQLITE_STORE]);

	if ( comStores->getCurrent() )
	{
		int selStore = (int) comStores->getCurrent()->getKey();
		if((selStore != oldStore) || (storeLocations[selStore] != oldStoreLocations[selStore]))
		{
			// Other store selected or location changed, switch store
			eConfig::getInstance()->setKey( "/enigma/epgStore", selStore );
			eEPGCache::getInstance()->messages.send( eEPGCache::Message( eEPGCache::Message::reloadStore ));
		}
	}

	eConfig::getInstance()->setKey("/extras/epghours", (const int)comNrOfDays->getCurrent()->getKey());

#ifdef ENABLE_MHW_EPG
	eConfig::getInstance()->setKey("/extras/mhwepg", chkMhwEPG->isChecked() ? 1 : 0);
#endif

#ifdef ENABLE_DISH_EPG
	eConfig::getInstance()->setKey("/extras/disheepg", chkDishEPG->isChecked() ? 1 : 0);
#endif

	eConfig::getInstance()->setKey("/enigma/epgStoreLimit", chkLimitEPG->isChecked() ? 1 : 0);

  	close (0);
}

void EpgSetup::rescanPressed()
{
	eEPGCache::getInstance()->messages.send( eEPGCache::Message( eEPGCache::Message::forceEpgScan ));
  	close (0);
}

void EpgSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

MultiEPGSetup::MultiEPGSetup() : 
	ePLiWindow(_(MENUNAMEMULTI), 320)
{
	unsigned int iNumServices = 8;
	int iFontIncrement = 0;
	int enableEPGColours = 0;
	eConfig::getInstance()->getKey("/elitedvb/multiepg/numservices", iNumServices);
	eConfig::getInstance()->getKey("/elitedvb/multiepg/fontIncrement", iFontIncrement);
	eConfig::getInstance()->getKey("/elitedvb/multiepg/catcolor", enableEPGColours);

	eNumServicesLabel = new eLabel (this);
	eNumServicesLabel->setText (_("MultiEPG channels:"));
	eNumServicesLabel->move (ePoint (10, yPos()));
	eNumServicesLabel->resize (eSize (170, widgetHeight()));

	eNumServices = new eNumber (this, 1, 1, 16, 2, (int *)&iNumServices, 0, eNumServicesLabel);
	eNumServices->move (ePoint (250, yPos()));
	eNumServices->resize (eSize (60, widgetHeight()));
	eNumServices->setHelpText (_("Number of channels per MultiEPG page to show"));
	eNumServices->loadDeco ();

	nextYPos(35);
	eFontIncrementLabel = new eLabel (this);
	eFontIncrementLabel->setText (_("MultiEPG entry fontsize:"));
	eFontIncrementLabel->move (ePoint (10, yPos()));
	eFontIncrementLabel->resize (eSize (200, widgetHeight()));

	eFontIncrement = new eComboBox(this, 3, eFontIncrementLabel);
	eFontIncrement->move (ePoint (250, yPos()));
	eFontIncrement->resize (eSize (60, widgetHeight()));
	eFontIncrement->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	for (int i = 0; i < 17; i++)  // range -6...+10
	{
		new eListBoxEntryText(*eFontIncrement, eString().sprintf("%d", i - 6), (void*)(i - 6));
	}
	eFontIncrement->setCurrent((void *) iFontIncrement);
	eFontIncrement->setHelpText (_("Increase/decrease entry fontsize with this number of points"));
	eFontIncrement->loadDeco();

	nextYPos(35);
	chkEPGColours = new eCheckbox(this, enableEPGColours, 1);
	chkEPGColours->setText(_("Enable EPG genre colours"));
	chkEPGColours->move(ePoint(10, yPos()));
	chkEPGColours->resize(eSize(300, widgetHeight()));
	chkEPGColours->setHelpText(_("Make the MultiEPG entries coloured depending on the genre"));
	chkEPGColours->loadDeco();

	buildWindow();
	CONNECT (bOK->selected, MultiEPGSetup::okPressed);
}

void MultiEPGSetup::okPressed()
{
	if(eNumServices) eConfig::getInstance()->setKey("/elitedvb/multiepg/numservices", (const unsigned int) eNumServices->getNumber());
	if(eFontIncrement) eConfig::getInstance()->setKey("/elitedvb/multiepg/fontIncrement", (const int) eFontIncrement->getCurrent()->getKey());
	if(chkEPGColours) eConfig::getInstance()->setKey("/elitedvb/multiepg/catcolor", chkEPGColours->isChecked() ? 1 : 0);

	close(0);
}

void MultiEPGSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
