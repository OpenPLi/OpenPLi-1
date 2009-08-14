#include "enigma_bouquet.h"
#include <enigma.h>
#include <enigma_main.h>
#include <sselect.h>
#include <parentallock.h>

#include <lib/dvb/servicestructure.h>
#include <lib/dvb/frontend.h>
#include <lib/gui/actions.h>
#include <lib/system/info.h>

#define MENUNAME N_("Bouquets")

class eZapBouquetSetupFactory : public eCallableMenuFactory
{
public:
	eZapBouquetSetupFactory() : eCallableMenuFactory("eZapBouquetSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eZapBouquetSetup;
	}
};

eZapBouquetSetupFactory eZapBouquetSetup_factory;

class eModeSelector: public eListBoxWindow<eListBoxEntryText>
{
	void entrySelected(eListBoxEntryText *s);
	void init_eModeSelector();;
public:
	eModeSelector();
};

eModeSelector::eModeSelector()
	:eListBoxWindow<eListBoxEntryText>(_("Bouquet Type"), 5, 400)
{
	init_eModeSelector();
}
void eModeSelector::init_eModeSelector()
{
	valign();
	new eListBoxEntryText( &list, _("TV"), (void*)  eZapMain::modeTV );
	new eListBoxEntryText( &list, _("Radio"), (void*) eZapMain::modeRadio );
#ifndef DISABLE_FILE
	new eListBoxEntryText( &list, _("File"), (void*) eZapMain::modeFile );
#endif
	CONNECT( list.selected, eModeSelector::entrySelected );
}

void eModeSelector::entrySelected( eListBoxEntryText *e )
{
	if ( e )
		close( (int)e->getKey() );
	else
		close(-1);
}

eZapBouquetSetup::eZapBouquetSetup()
	:eSetupWindow(_(MENUNAME), 4, 430)
{
	init_eZapBouquetSetup();
}
void eZapBouquetSetup::init_eZapBouquetSetup()
{
	valign();
	int entry=0;
	CONNECT((new eListBoxEntryMenu(&list, _("Create new bouquet"), eString().sprintf("(%d) %s", ++entry, _("create new empty bouquet"))))->selected, eZapBouquetSetup::createNewEmptyBouquet );
	CONNECT((new eListBoxEntryMenu(&list, _("Add services to bouquet"), eString().sprintf("(%d) %s", ++entry, _("add multiple services to a specific bouquet"))))->selected, eZapBouquetSetup::editModeSelected );
	CONNECT((new eListBoxEntryMenu(&list, _("Edit bouquets"), eString().sprintf("(%d) %s", ++entry, _("sort,rename,delete services/bouquets "))))->selected, eZapBouquetSetup::editSelected );
	if ( pinCheck::getInstance()->getParentalEnabled() )
	{
		//new eListBoxEntryMenuSeparator(&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
		CONNECT((new eListBoxEntryMenu(&list, _("Lock/Unlock Services"), eString().sprintf("(%d) %s", ++entry,_("lock,unlock services (parental locking)"))))->selected, eZapBouquetSetup::lockUnlockServices );
	}
//	CONNECT((new eListBoxEntryMenu(&list, _("Duplicate Sat/Provider/Bouquet"), _("copy specific bouquet/provider/sat to bouquet list")))->selected, eZapBouquetSetup::editSelected );
}

void eZapBouquetSetup::editModeSelected()
{
	hide();
	eModeSelector m;
#ifndef DISABLE_LCD
	m.setLCD(LCDTitle, LCDElement);
#endif
	m.show();
	int ret = m.exec();
	m.hide();
	if ( ret != -1 )
	{
		eServiceSelector &sel = *eZap::getInstance()->getServiceSelector();

		if ( eZapMain::getInstance()->toggleEditMode(&sel,ret) )
		{
			show();
			return;
		}

		// save ColorButtons State.. and disable
		int ButtonsWasVisible=0;
		eConfig::getInstance()->getKey("/ezap/serviceselector/showButtons", ButtonsWasVisible );

		if ( ButtonsWasVisible ) // must show buttons
			eConfig::getInstance()->setKey("/ezap/serviceselector/showButtons", (int)0 );

		eActionMapList::getInstance()->deactivateStyle("sselect_default");

		// save current serviceselector Path
		eServicePath p = sel.getPath();

		// save current selection
		eServiceReference ref = sel.getSelected();

		// set new service selector path... ( bouquet list root... )

		// when satellite frontend exist we show the satellites root
		if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )
		{
			if ( ret == eZapMain::modeTV )
				sel.setPath( eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, (1<<4)|(1<<1) ));
			else if ( ret == eZapMain::modeRadio )
				sel.setPath( eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, 1<<2) );
#ifndef DISABLE_FILE
			else
				sel.setPath( eZapMain::getInstance()->getRoot(eZapMain::listAll, eZapMain::modeFile) );
#endif
		}
		else  // cable or dvb-t
		{
			if ( ret == eZapMain::modeTV )
				sel.setPath( eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, (1<<4)|(1<<1), 0xFFFFFFFF) );
			else if ( ret == eZapMain::modeRadio )
				sel.setPath( eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, 1<<2, 0xFFFFFFFF) );
#ifndef DISABLE_FILE
			else
				sel.setPath( eZapMain::getInstance()->getRoot(eZapMain::listAll, eZapMain::modeFile) );
#endif
		}

		// save current service selector style
		int sstyle = sel.getStyle();

		// set to SingleColumn Style
		sel.setStyle( eServiceSelector::styleSingleColumn, true );

		sel.choose(-1);

		if ( ButtonsWasVisible ) // must show buttons
			eConfig::getInstance()->setKey("/ezap/serviceselector/showButtons", (int)1 );

		// set old Style
		sel.setStyle( sstyle, true );

		// set old path
		sel.setPath(p,ref);

		eActionMapList::getInstance()->activateStyle("sselect_default");
	}
	show();
}

void eZapBouquetSetup::editSelected()
{
	// save ColorButtons State
	int ButtonsWasVisible=0;
	eConfig::getInstance()->getKey("/ezap/serviceselector/showButtons", ButtonsWasVisible );
	if ( !ButtonsWasVisible ) // must show buttons
	{
		ButtonsWasVisible=2;
		eConfig::getInstance()->setKey("/ezap/serviceselector/showButtons", (int)1 );
	}

	eActionMapList::getInstance()->deactivateStyle("sselect_default");

	// enable edit actions
	eActionMapList::getInstance()->activateStyle("sselect_edit");

	eServiceSelector &sel = *eZap::getInstance()->getServiceSelector();

	// save current serviceselector Path
	eServicePath p = sel.getPath();

	// save current selection
	eServiceReference ref = sel.getSelected();

	// set new service selector path... ( bouquet list root... )
	sel.setPath( eServiceStructureHandler::getRoot(eServiceStructureHandler::modeBouquets),eServiceReference() );

	// save current service selector style
	int sstyle = sel.getStyle();

	// set to SingleColumn Style
	sel.setStyle( eServiceSelector::styleSingleColumn, true );

	// set edit mode key descriptions ( delete, mark, rename )
	sel.setKeyDescriptions(true);

	hide();

	// open service selector
	sel.choose(-1);

	show();

	// restore state of color buttons
	if ( ButtonsWasVisible == 2)  // buttons was not visible.. hide
		eConfig::getInstance()->setKey("/ezap/serviceselector/showButtons", (int)0 );

	// restore old ssel style
	sel.setStyle( sstyle, true );

	// restore old actions
	eActionMapList::getInstance()->deactivateStyle("sselect_edit");
	eActionMapList::getInstance()->activateStyle("sselect_default");

	sel.setPath( p, ref );
}

void eZapBouquetSetup::createNewEmptyBouquet()
{
	hide();
	eModeSelector m;
#ifndef DISABLE_LCD
	m.setLCD(LCDTitle, LCDElement);
#endif
	m.show();
	int ret = m.exec();
	m.hide();
	if ( ret != -1 )
		eZapMain::getInstance()->createEmptyBouquet( ret );
	show();
}

void eZapBouquetSetup::lockUnlockServices()
{
	if ( pinCheck::getInstance()->checkPin(pinCheck::parental))
	{
		eZap::getInstance()->getServiceSelector()->plockmode = 1;
		eZap::getInstance()->getServiceSelector()->choose(-1);
		eZap::getInstance()->getServiceSelector()->plockmode = 0;
	}
}

void eZapBouquetSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
