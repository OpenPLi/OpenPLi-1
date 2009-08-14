#include <satconfig.h>

#include <enigma_main.h>
#include <lib/base/i18n.h>
#include <lib/gui/eskin.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/eprogress.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/frontend.h>
#include <lib/dvb/dvbwidgets.h>
#include <lib/system/info.h>

eSatelliteConfigurationManager::eSatelliteConfigurationManager(bool init)
	: refresh(0),
	buttonWidget(0),
	w_buttons(0),
	button_close(0), button_new(0), button_erase(0),
	lSatPos(0), lLNB(0), l22Khz(0), lVoltage(0),
	scrollbar(0),
	combo_type(0)
{
	if (init) init_eSatelliteConfigurationManager();
}

void eSatelliteConfigurationManager::init_eSatelliteConfigurationManager()
{
	// Lock remote control to prevent the settings to be saved
	// due to the green button pressed in eWizardSelectLNB.
	eRCInput::getInstance()->lock();

	lSatPos = new eLabel(this);
	lSatPos->setName("lSatPos");

	lLNB = new eLabel(this);
	lLNB->setName("lLnb");
	lLNB->hide();

	l22Khz = new eLabel(this);
	l22Khz->setName("l22khz");
	l22Khz->hide();

	lVoltage = new eLabel(this);
	lVoltage->setName("lVoltage");
	lVoltage->hide();
	
	scrollbar = new eProgress(this);
	scrollbar->setName("scrollbar");
	scrollbar->setStart(0);
	scrollbar->setPerc(100);

	button_close=new eButton(this);
	button_close->setName("close");
	CONNECT(button_close->selected, eSatelliteConfigurationManager::closePressed);

	button_new=new eButton(this);
	button_new->setName("new");
	button_new->hide();
	CONNECT(button_new->selected, eSatelliteConfigurationManager::newPressed);

	button_erase=new eButton(this,0,0);
	button_erase->setName("erase");
	button_erase->hide();
	CONNECT(button_erase->selected, eSatelliteConfigurationManager::erasePressed);

	buttonWidget=new eWidget(this);
	buttonWidget->setName("buttons");	

	combo_type=new eComboBox(this, 7);
	combo_type->setName("type");
	CONNECT(combo_type->selchanged, eSatelliteConfigurationManager::typeChanged);
	new eListBoxEntryText( *combo_type, _("one single satellite"), (void*)0, 0, _("one directly connected LNB"));
	new eListBoxEntryText( *combo_type, _("2 satellites via Toneburst"), (void*)5, 0, _("2 LNBs via Toneburst Signal"));
	new eListBoxEntryText( *combo_type, _("2 satellites via 22kHz (only Highband)"), (void*)6, 0, _("2 LNBs via 22kHz Signal.. special for old analog switches"));
	new eListBoxEntryText( *combo_type, _("2 satellites via DiSEqC A/B"), (void*)1, 0, _("2 LNBs via DiSEqC"));
	new eListBoxEntryText( *combo_type, _("4 satellites via DiSEqC OPT A/B"), (void*)2, 0, _("4 LNBs via DiSEqC"));
	new eListBoxEntryText( *combo_type, _("many satellites via DiSEqC Rotor"), (void*)3, 0, _("1 LNB with DiSEqC Rotor"));
	new eListBoxEntryText( *combo_type, _("non-standard user defined configuration..."), (void*)4, 0, _("special"));

	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "eSatelliteConfigurationManager"))
		eFatal("skin load of \"eSatelliteConfigurationManager\" failed");

	eSize s = buttonWidget->getSize();
	s.setHeight( s.height()*49 );	// set 49 (50 -1 = 49) (corespondent with pageEnds[50] in satconfig.h =max 250sats)

	w_buttons = new eWidget(buttonWidget);
	w_buttons->resize( s );
	w_buttons->move( ePoint(0,0) );

	eTransponderList::getInstance()->reloadNetworks();  // load all networks from satellite.xml or cable.xml
	createControlElements();

	complexity=checkComplexity();
	combo_type->setCurrent( (void*)complexity, true );

	for ( int i = 0; i < 50; i++ )	// set 50 (corespondent with pageEnds[50] in satconfig.h, =max 250sats )
		pageEnds[i]=i*200;

	curScrollPos = 0;

	repositionWidgets();

	CONNECT( eWidget::focusChanged, eSatelliteConfigurationManager::focusChanged );
	setFocus(combo_type);

	/* help text for satellite configuration screen */
	setHelpText(_("\tSatellite Configuration\n\n>>> [MENU] >>> [6] Setup >>> [2] Service Searching >>> [1] Satellite Configuration\n" \
								". . . . . . . . . .\n\nHere you configure your dish setup (DiSEqC, LNB, etc)\n. . . . . . . . . .\n\n" \
								"Usage:\n\n[UP]/[DOWN]\tSelect Inputfield or Button\n\nSatellite Configuration:\n[UP]/[DOWN]\tSelect one of the predefined\n" \
								"\tconfigurations or 'non-standard user\n\tdefined configuration...'\n\nSat/Position:\n" \
								"[UP]/[DOWN]\tSelect the satellite you want to use\n\nLNB:\tLNB Setup menu (for selected LNB)\n\n" \
								"22kHz:\tSelect Hi/Lo,On.Off\n\nVoltage:\tSelect a voltage (H/V,14V,18V,Off)\n\n" \
								"[YELLOW]\tAdd a LNB/Satellite\n\n[RED]\tDelete selected Satellite/LNB\n\n" \
								"[GREEN]\tSave Settings and Close Window\n\n[EXIT]\tClose window without saving changes"));

	eRCInput::getInstance()->unlock();
}

void eSatelliteConfigurationManager::focusChanged( const eWidget* focus )
{
	if ( focus && focus->getName() == "satWidget" )
	{
		int old = curScrollPos;
		for (unsigned int it=0; it < (sizeof(pageEnds)/sizeof(int)); ++it )
		{
			if ( focus->getPosition().y() < pageEnds[it] )
			{
				curScrollPos=it;
				--curScrollPos;
				break;
			}
		}
		if ( curScrollPos != old )
		{
			w_buttons->move( ePoint(0, -pageEnds[curScrollPos]) );
			updateScrollbar(complexity > 2 && complexity < 5);
		}
	}
}

void eSatelliteConfigurationManager::typeChanged(eListBoxEntryText* newtype)
{
	int newcomplexity=(int)newtype->getKey();
	if ( newcomplexity == complexity )
		return;
	// check if the new type is less complex than our current setup...
	int newComp = newcomplexity > 4 ? 1 : newcomplexity;
	int oldComp = checkComplexity();
	if ( oldComp > 4 )
		oldComp=1;
	if ( oldComp > newComp )
	{
		hide();
		int res = eMessageBox::ShowBox(_("Configuration contains some elements\nwhich don't fit into new DiSEqC-Type. Drop these items?"), _("Change DiSEqC-Type..."), eMessageBox::iconWarning|eMessageBox::btYes|eMessageBox::btCancel);
		show();
		if (res != eMessageBox::btYes)
		{
			combo_type->setCurrent((void*)complexity, false);
			return;
		}
	}
	setComplexity(complexity=newcomplexity);
	updateButtons(newcomplexity);
}

void eSatelliteConfigurationManager::extSetComplexity(int complexity)
{
	combo_type->setCurrent((void*)complexity, true);
	checkComplexity();
	
	// Hack: prevent that the save button is selected by default
	setFocus(combo_type);
}

void eSatelliteConfigurationManager::setComplexity(int complexity)
{
	eDebug("setComplexitiy = %d", complexity);
	int i=0;

	if (eTransponderList::getInstance()->reloadNetworks() < 0)
	{
		/* avoid getting into an endless loop below, when we have no transponders */
		return;
	}

	switch (complexity)
	{
	case 0:
	{
		deleteSatellitesAbove(1);
		while (eTransponderList::getInstance()->getLNBs().size() < 1)
			createSatWidgets( createSatellite() );
		for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); ++it, ++i)
			setSimpleDiseqc(it->getSatelliteList().first(), i);
		break;
	}
	case 5:
	case 6:
	case 1:
		deleteSatellitesAbove(2);
		while (eTransponderList::getInstance()->getLNBs().size() < 2)
			createSatWidgets( createSatellite() );
		for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); ++it, ++i)
		{
			setSimpleDiseqc( it->getSatelliteList().first(), i);
			if ( complexity == 6 )
				entryMap[it->getSatelliteList().first()].hilo->setCurrent(2-i);
		}
		break;
	case 2:
		deleteSatellitesAbove(4);
		while (eTransponderList::getInstance()->getLNBs().size() < 4)
			createSatWidgets( createSatellite() );
		for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); ++it, ++i)
			setSimpleDiseqc(it->getSatelliteList().first(), i);
		break;
	case 3:
	{
		deleteSatellitesAbove(1);

		if ( !eTransponderList::getInstance()->getLNBs().size() )
			createSatWidgets( createSatellite() );

		setSimpleDiseqc( eTransponderList::getInstance()->getLNBs().front().getSatelliteList().front(), 0 );
		unsigned int used=0;
		do
		{
			used = eTransponderList::getInstance()->getLNBs().front().getSatelliteList().size();
			createSatWidgets( createSatellite() );
		}
		while ( used != eTransponderList::getInstance()->getLNBs().front().getSatelliteList().size() );
		break;
	}
	case 4:
		break;
	}
	checkComplexity();
	repositionWidgets();
}

int eSatelliteConfigurationManager::checkComplexity()
{
	int c=0, comp=0, rcomp=0;
	for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); it++)
	{
		if (it->getSatelliteList().size() != 1)
		{
			updateButtons(3);
			if ( eTransponderList::getInstance()->getLNBs().size() > 1)
			{
				eDebug("complexity is 4 since lnb %d has more than one satellite, and we have more than one LNB.", c);
				return 4;
			}
			eDebug("complexity is 3 since lnb %d has more than one satellite, and only one LNB exist.", c);
			return 3;
		} else
			++c;
		int dc=checkDiseqcComplexity(it->getSatelliteList().first());
		rcomp=dc;
		int tmp = dc > 4 ? 2 : dc;
		eDebug("LNB %d has %d", c, dc);
		if (tmp > comp)
			comp=tmp;
	}
	if (c > 4)
		comp=3;
	if ((comp < 2) && c>2)
		comp=2;
	if ((comp < 1) && c>1)
		comp=1;

	updateButtons(comp);
	return rcomp>4?rcomp:comp;
}

bool eSatelliteConfigurationManager::getRotorEnabled()
{
    bool rotorenabled = false;
    for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); it++)
    {
	if (it->getSatelliteList().size() != 1)
	{
	    // more the 1 satellite configured on this LNB, can only work with a rotor I suppose ;-)
	    rotorenabled = true;
	    // Makes no sense to look any further
	    break;
	}
    }
											
    return rotorenabled;
}
											    
void eSatelliteConfigurationManager::updateButtons(int comp)
{
	if (comp < 3 || comp > 4)
	{
		if (lLNB) lLNB->hide();
		if (l22Khz) l22Khz->hide();
		if (lVoltage) lVoltage->hide();
		if (lSatPos)
		{
			lSatPos->hide();
			lSatPos->move( ePoint( 75, lSatPos->getPosition().y() ) );
			lSatPos->show();
		}
		if (button_erase) button_erase->hide();
		if (button_new) button_new->hide();
	}
	else
	{
		if (lSatPos)
		{
			lSatPos->hide();
			lSatPos->move( ePoint( 0, lSatPos->getPosition().y() ) );
			lSatPos->show();
		}
		if (lLNB) lLNB->show();
		if (l22Khz) l22Khz->show();
		if (lVoltage) lVoltage->show();
		if (button_new) button_new->show();
		if (button_erase) button_erase->show();
	}
}

int eSatelliteConfigurationManager::checkDiseqcComplexity(eSatellite *s)
{
	SatelliteEntry *se=0;
	
	if (entryMap.count(s))
		se=&entryMap[s];
	
	if (se)
	{
		if (s->getLNB()->getDiSEqC().DiSEqCMode == eDiSEqC::NONE &&
			s->getLNB()->getDiSEqC().MiniDiSEqCParam == eDiSEqC::NO &&
			s->getSwitchParams().HiLoSignal == eSwitchParameter::ON )
		{
			se->description->setText(_("22kHz On"));
			return 6;
		}
	
		if (s->getLNB()->getDiSEqC().DiSEqCMode == eDiSEqC::NONE &&
			s->getLNB()->getDiSEqC().MiniDiSEqCParam == eDiSEqC::NO &&
			s->getSwitchParams().HiLoSignal == eSwitchParameter::OFF )
		{
			se->description->setText(_("22kHz Off"));
			return 6;
		}
	}

	unsigned int defaultLOFLo;
	unsigned int defaultLOFHi;
	unsigned int defaultLOFThreshold;
	
	eLNB::getRegionDefaultLOFs(&defaultLOFLo, &defaultLOFHi, &defaultLOFThreshold);

	if(s->getLNB()->getLOFHi() != defaultLOFHi ||
		s->getLNB()->getLOFLo() != defaultLOFLo ||
		s->getLNB()->getLOFThreshold() != defaultLOFThreshold )
	{
		eDebug("lof hi, lo or threshold changed");
		return 4;
	}
	if (s->getSwitchParams().VoltageMode != eSwitchParameter::HV)
	{
		eDebug("voltage mode unusual");
		return 4;
	}
	if (s->getSwitchParams().HiLoSignal != eSwitchParameter::HILO)
	{
		eDebug("sig22 mode unusual");
		return 4;
	}

	if (s->getLNB()->getDiSEqC().DiSEqCMode == eDiSEqC::NONE)
	{
		switch (s->getLNB()->getDiSEqC().MiniDiSEqCParam)
		{
			case eDiSEqC::A:
				if (se)
					se->description->setText(_("Toneburst A"));
				return 5;
			case eDiSEqC::B:
				if (se)
					se->description->setText(_("Toneburst B"));
				return 5;
			case eDiSEqC::NO:
				if (se)
					se->description->setText(_("direct connection"));
				return 0;
		}
	}

	if (s->getLNB()->getDiSEqC().DiSEqCMode > eDiSEqC::V1_0)
	{
		eDebug("diseqc mode > 1.0");
		return 4;
	}

	if (s->getLNB()->getDiSEqC().DiSEqCParam > 3)
	{
		eDebug("unusual diseqc parameter");
		return 4;
	}

	if (se)
	{
		switch (s->getLNB()->getDiSEqC().DiSEqCParam)
		{
		case 0:
			se->description->setText(_("DiSEqC AA"));
			break;
		case 1:
			se->description->setText(_("DiSEqC AB"));
			break;
		case 2:
			se->description->setText(_("DiSEqC BA"));
			break;
		case 3:
			se->description->setText(_("DiSEqC BB"));
			break;
		}
	}

			// we have simple 1.0
	if (s->getLNB()->getDiSEqC().DiSEqCParam > 1)
		return 2;
	
	if (s->getLNB()->getDiSEqC().DiSEqCParam)
		return 1;

	return 0;
}

void eSatelliteConfigurationManager::deleteSatellitesAbove(int n)
{
		// it's all about invalidating ptrlist's iterators on delete :/
	int count=0;
start:
	int index=0;
	for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); (it != eTransponderList::getInstance()->getLNBs().end()); ++it)
		for ( ePtrList<eSatellite>::iterator si = it->getSatelliteList().begin() ; si != it->getSatelliteList().end(); si++)
			if ( it->getSatelliteList().size() > 1 || index++ >= n )
			{
				delSatellite(si, false, true);
				++count;
				goto start;
			}

	if ( count )
	{
		eDVB::getInstance()->settings->removeDVBBouquets();
		eDVB::getInstance()->settings->sortInChannels();
		eDVB::getInstance()->settings->saveBouquets();
	}

	// redraw satellites
	cleanupWidgets();
}

void eSatelliteConfigurationManager::setSimpleDiseqc(eSatellite *s, int diseqcnr)
{
	eDebug("setSimpleDiSEqC %d", complexity );
	s->getSwitchParams().VoltageMode=eSwitchParameter::HV;
	eLNB *lnb=s->getLNB();
	lnb->setDefaultOptions();
	if ( complexity == 6 )
	{
		unsigned int defaultLOFLo;
		eLNB::getRegionDefaultLOFs(&defaultLOFLo, NULL, NULL);
		
		lnb->setLOFLo(defaultLOFLo);
		if ( diseqcnr )
			s->getSwitchParams().HiLoSignal=eSwitchParameter::ON;
		else
			s->getSwitchParams().HiLoSignal=eSwitchParameter::OFF;
	}
	else
		s->getSwitchParams().HiLoSignal=eSwitchParameter::HILO;

	lnb->getDiSEqC().MiniDiSEqCParam=eDiSEqC::NO;
	switch (complexity) // if we have diseqc at all
	{
		case 5:
			if ( diseqcnr )
				lnb->getDiSEqC().MiniDiSEqCParam=eDiSEqC::B;
			else
				lnb->getDiSEqC().MiniDiSEqCParam=eDiSEqC::A;
		case 6:
		case 0:
			lnb->getDiSEqC().DiSEqCParam=0;
			lnb->getDiSEqC().DiSEqCMode=eDiSEqC::NONE;
			break;
		case 4:
			lnb->getDiSEqC().DiSEqCParam=eDiSEqC::AA+diseqcnr;
			lnb->getDiSEqC().DiSEqCMode=eDiSEqC::V1_1;
		case 3:
			lnb->getDiSEqC().DiSEqCParam=eDiSEqC::AA;
			lnb->getDiSEqC().DiSEqCMode=eDiSEqC::V1_2;
			break;
		case 2:
		case 1:
			lnb->getDiSEqC().DiSEqCParam=eDiSEqC::AA+diseqcnr;
			lnb->getDiSEqC().DiSEqCMode=eDiSEqC::V1_0;
			break;
	}
	lnb->getDiSEqC().FastDiSEqC=0;
	lnb->getDiSEqC().DiSEqCRepeats=0;
	lnb->getDiSEqC().SeqRepeat=0;
	lnb->getDiSEqC().SwapCmds=0;	
	lnb->getDiSEqC().uncommitted_cmd=0;
	lnb->getDiSEqC().setRotorDefaultOptions();
}

eSatelliteConfigurationManager::~eSatelliteConfigurationManager()
{
	if (refresh)
		delete refresh;
}

eSatellite *eSatelliteConfigurationManager::getSat4SatCombo( const eComboBox *c )
{
	std::map<eSatellite*, SatelliteEntry>::iterator it ( entryMap.begin() );
	for ( ; it != entryMap.end(); it++)
		if ( it->second.sat == c)
			break;
	return it != entryMap.end()?it->first:0;
}

eSatellite *eSatelliteConfigurationManager::getSat4HiLoCombo( const eComboBox *c )
{
	std::map<eSatellite*, SatelliteEntry>::iterator it ( entryMap.begin() );
	for ( ; it != entryMap.end(); it++)
		if ( it->second.hilo == c)
			break;
	return it != entryMap.end()?it->first:0;
}

eSatellite *eSatelliteConfigurationManager::getSat4VoltageCombo( const eComboBox *c )
{
	std::map<eSatellite*, SatelliteEntry>::iterator it ( entryMap.begin() );
	for ( ; it != entryMap.end(); it++)
		if ( it->second.voltage == c)
			break;
	return it != entryMap.end()?it->first:0;
}

eSatellite *eSatelliteConfigurationManager::getSat4LnbButton( const eButton *b )
{
	std::map<eSatellite*, SatelliteEntry>::iterator it ( entryMap.begin() );
	for ( ; it != entryMap.end(); it++)
		if ( it->second.lnb == b)
			break;
	return it != entryMap.end()?it->first:0;
}

#define DESC_POS_X 370
#define SAT_POS_X  0
#define LNB_POS_X  310
#define HILO_POS_X  370
#define VOLTAGE_POS_X  450

#define POS_Y 0

void eSatelliteConfigurationManager::cleanupWidgets()
{
	for (std::list<SatelliteEntry*>::iterator It( deleteEntryList.begin() ); It != deleteEntryList.end();)
	{
		(**It).sat->hide();
		(**It).lnb->hide();
		(**It).voltage->hide();
		(**It).hilo->hide();
		(**It).fixed->hide();
		(**It).description->hide();
		delete (**It).sat;
		delete (**It).lnb;
		delete (**It).voltage;
		delete (**It).hilo;
		delete (**It).fixed;
		delete (**It).description;

		// search Entry in Map;
		std::map< eSatellite*, SatelliteEntry >::iterator it( entryMap.begin() );
		for ( ; it != entryMap.end() && &it->second != *It ; it++)
			;

		if (it != entryMap.end() )
				entryMap.erase( it );

		It = deleteEntryList.erase(It);
	}
}

void eSatelliteConfigurationManager::repositionWidgets()
{
	cleanupWidgets();
	int lnbcount=0, y=POS_Y, satcount=0, cnt=0;

	int tmp = complexity;
	if (tmp > 4)
		tmp=1;

	eWidget *old = focus;

	for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); it++)
		satcount += it->getSatelliteList().size();

	std::pair<eSatellite*,int> sats[satcount];
	for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); it++)
	{
		for ( ePtrList<eSatellite>::iterator s ( it->getSatelliteList().begin() ); s != it->getSatelliteList().end(); s++)
		{
			sats[cnt].first=s;
			sats[cnt++].second=lnbcount;
		}
		++lnbcount;
	}

	cnt=0;
	while(cnt < satcount)
	{
		SatelliteEntry& entry = entryMap[ sats[cnt].first ];
		// search eComboBox for this eSatellite and move

		entry.sat->hide();
		entry.lnb->hide();
		entry.voltage->hide();
		entry.hilo->hide();
		entry.fixed->hide();
		entry.description->hide();

		entry.sat->move( ePoint((tmp>2?SAT_POS_X:55), y) );
		entry.fixed->move( ePoint(0, y) );
		entry.description->move( ePoint(DESC_POS_X, y) );
		entry.lnb->move( ePoint(LNB_POS_X, y) );
		entry.lnb->setText( eString().sprintf("%d", sats[cnt].second ) );
		entry.fixed->setShortcut(eString().sprintf("%d", (y - POS_Y) / 40 + 1 ) );
		entry.fixed->setShortcutPixmap(eString().sprintf("%d", (y - POS_Y) / 40 + 1 ));

		entry.hilo->move( ePoint(HILO_POS_X, y) );
		entry.voltage->move( ePoint(VOLTAGE_POS_X, y) );

		entry.sat->show();
		if (tmp>2) // user defined..
		{
			entry.lnb->show();
			entry.voltage->show();
			entry.hilo->show();
		}
		else
		{
			entry.fixed->show();
			entry.description->show();
		}
		y+=40;
		++cnt;
	}
	if ( old )
		setFocus(old);
	updateScrollbar(tmp>2);
}

void eSatelliteConfigurationManager::createControlElements()
{
	if ( !sats )
	{
		for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); it++)
			for ( ePtrList<eSatellite>::iterator s ( it->getSatelliteList().begin() ); s != it->getSatelliteList().end(); s++)
				sats.push_back(*s);
	}
	for (ePtrList<eSatellite>::iterator it(sats.begin()); it != sats.end(); ++it )
		createSatWidgets(*it);
}

void eSatelliteConfigurationManager::lnbSelected(eButton* who)
{
	eWidget *old = focus;
	eDebug("lnb selected");
#ifndef DISABLE_LCD
	eLNBSetup sel( getSat4LnbButton(who), LCDTitle, LCDElement );
#else
	eLNBSetup sel( getSat4LnbButton(who) );
#endif
	hide();
	sel.show();

	if ( sel.exec() )  // the we must reposition widgets
		repositionWidgets();

	sel.hide();
	show();

	if (old)
		setFocus(old);

	return;
}

void eSatelliteConfigurationManager::erasePressed()
{
	eSatellite *s=0;
	if ( focus && focus->getName() == "satWidget" )
	{
		s = getSat4SatCombo( (eComboBox*)focus );
		if (!s)
			s = getSat4HiLoCombo( (eComboBox*)focus );
		if (!s)
			s = getSat4VoltageCombo( (eComboBox*)focus );
		if (!s)
			s = getSat4LnbButton( (eButton*)focus );
	}
	if (!s)
		eDebug("Widget not found");
	else
	{
		eDebug("call delSatellite(s)");
		focusNext(eWidget::focusDirN);
		delSatellite(s);
	}
}

void eSatelliteConfigurationManager::delSatellite( eSatellite* s, bool redraw, bool atomic )
{
	if ( atomic )
		eTransponderList::getInstance()->removeOrbitalPosition(s->getOrbitalPosition());
	else
	{
		eDVB::getInstance()->settings->removeOrbitalPosition(s->getOrbitalPosition());
		eDVB::getInstance()->settings->saveBouquets();
	}
	eLNB* lnb = s->getLNB();
	lnb->deleteSatellite( s );
//	eDebug("Satellite is now removed");
	if ( !lnb->getSatelliteList().size() )   // the lnb that have no more satellites must be deleted
	{
//		eDebug("delete no more used lnb");
		eTransponderList::getInstance()->getLNBs().remove( *s->getLNB() );
	}
//	else
//		eDebug("do not delete lnb");

	deleteEntryList.push_back(&entryMap[ s ]);

	if(!redraw)
		return;

	if (!refresh)
	{
		refresh = new eTimer( eApp );
		CONNECT( refresh->timeout, eSatelliteConfigurationManager::repositionWidgets );
	}
	refresh->start(50, true );
}

void eSatelliteConfigurationManager::satChanged(eComboBox* who, eListBoxEntryText *le)
{
	eSatellite *s=getSat4SatCombo( who );

	if ( le->getKey() && le->getText() )
	{
			// delete old orbital position services
		if ((int)le->getKey() != s->getOrbitalPosition())
			eDVB::getInstance()->settings->removeOrbitalPosition(s->getOrbitalPosition());
		s->setOrbitalPosition( (int) le->getKey() );
		s->setDescription( le->getText() );
	}
	else if (s) // *delete* selected -->> satellite and empty lnbs were now deleted
		delSatellite(s);
}

void eSatelliteConfigurationManager::hiloChanged(eComboBox* who, eListBoxEntryText *le)
{
	getSat4HiLoCombo(who)->getSwitchParams().HiLoSignal = (eSwitchParameter::SIG22) (int) le->getKey();
}

void eSatelliteConfigurationManager::voltageChanged(eComboBox* who, eListBoxEntryText *le)
{
	getSat4VoltageCombo(who)->getSwitchParams().VoltageMode = (eSwitchParameter::VMODE) (int) le->getKey();
}

void eSatelliteConfigurationManager::closePressed()
{
	eTransponderList::getInstance()->writeLNBData();
	eConfig::getInstance()->flush();
	close(1);
}

eComboBox* eSatelliteConfigurationManager::createSatWidgets( eSatellite *s )
{
	if (!s)
		return 0;
	SatelliteEntry sat;

	eComboBox* newSat = new eComboBox(w_buttons, 6, lSatPos );
	newSat->setName("satWidget");
	sat.sat=newSat;

	newSat->loadDeco();
	newSat->resize(eSize(300, 30));
	newSat->setHelpText( _("press ok to select another satellite"));

	int err;
	if ( (err = newSat->setCurrent( (void*) s->getOrbitalPosition() ) ) )
		if (err == eComboBox::E_COULDNT_FIND)  // hmm current entry not in Combobox... we add it manually
			newSat->setCurrent( new eListBoxEntryText( *newSat, s->getDescription(), (void*) s->getOrbitalPosition() ) );
	CONNECT( newSat->selected_id, eSatelliteConfigurationManager::addSatellitesToCombo );
	CONNECT( newSat->selchanged_id, eSatelliteConfigurationManager::satChanged );

	eButton* b = new eButton(w_buttons, lLNB);
	sat.lnb=b;
	b->setName("satWidget");
	b->loadDeco();

	b->resize(eSize(50, 30));
	b->setHelpText( _("press ok to goto LNB config"));
	CONNECT(b->selected_id, eSatelliteConfigurationManager::lnbSelected);

	sat.fixed=new eLabel(w_buttons);
//	sat.fixed->setText(eString().sprintf(_("Sat %d"), index));
		// don't ask...
	sat.fixed->setShortcutFocus(sat.sat);
	sat.fixed->resize(eSize(130, 30));

	sat.description=new eLabel(w_buttons);
	sat.description->resize(eSize(245, 30));

	eComboBox *c = new eComboBox(w_buttons, 3, l22Khz);
	c->setName("satWidget");
	sat.hilo=c;
	c->loadDeco();

	c->resize( eSize( 70, 30 ) );
	c->setHelpText( _("press ok to select another 22kHz mode") );
	new eListBoxEntryText( *c, "Hi/Lo", (void*)eSwitchParameter::HILO, 0, _("22kHz signal is automatically switched") );
	new eListBoxEntryText( *c, "On", (void*)eSwitchParameter::ON, 0, _("22kHz is always enabled (high band)") );
	new eListBoxEntryText( *c, "Off", (void*)eSwitchParameter::OFF, 0, _("22kHz is always disabled (low band)") );
	c->setCurrent( (void*) (int) s->getSwitchParams().HiLoSignal );
	CONNECT( c->selchanged_id, eSatelliteConfigurationManager::hiloChanged);

	c = new eComboBox(w_buttons, 3, lVoltage );
	c->setName("satWidget");
	sat.voltage=c;
	c->loadDeco();

	c->resize( eSize( 70, 30 ) );
	c->setHelpText( _("press ok to select another LNB Voltage mode") );
	new eListBoxEntryText( *c, "H/V", (void*)eSwitchParameter::HV, 0, _("Voltage is automatically changed") );
	new eListBoxEntryText( *c, "14V", (void*)eSwitchParameter::_14V, 0, _("Voltage is always 14V (vertical)") );
	new eListBoxEntryText( *c, "18V", (void*)eSwitchParameter::_18V, 0, _("Voltage is always 18V (horizontal)") );
	new eListBoxEntryText( *c, "off", (void*)eSwitchParameter::_0V, 0, _("Voltage is always off") );
	c->setCurrent( (void*) (int) s->getSwitchParams().VoltageMode );
	CONNECT( c->selchanged_id, eSatelliteConfigurationManager::voltageChanged);
	entryMap.insert( std::pair <eSatellite*, SatelliteEntry> ( s, sat ) );
	return newSat;
}

void eSatelliteConfigurationManager::addSatellitesToCombo(eButton* btn)
{
	eComboBox *combo =  (eComboBox*) btn;
	if ( combo->getCount() == 1 )
	{
		int opos = (int) combo->getCurrent()->getKey();
		for (std::list<tpPacket>::const_iterator i(eTransponderList::getInstance()->getNetworks().begin()); i != eTransponderList::getInstance()->getNetworks().end(); ++i)
			if ( i->possibleTransponders.size() && i->orbital_position != opos )
				new eListBoxEntryText( *combo, i->name, (void*) i->orbital_position );
	}
}

void eSatelliteConfigurationManager::newPressed()
{
	// here we must add the new Comboboxes and the button to the hash maps...
	eComboBox *newSat = createSatWidgets(createSatellite());
	if ( newSat )
	{
		repositionWidgets();
		setFocus(newSat);
	}
}

eSatellite *eSatelliteConfigurationManager::createSatellite()
{
	std::list<tpPacket>::const_iterator i( eTransponderList::getInstance()->getNetworks().begin());
	// we search the next unused Satellite in list...
	int found=0;
	for (; i != eTransponderList::getInstance()->getNetworks().end(); ++i)
	{
		std::map< eSatellite*, SatelliteEntry > :: iterator it ( entryMap.begin() );
		for ( ; it != entryMap.end(); it++)
		{
			if ( i->orbital_position == it->first->getOrbitalPosition() )
				break;  // test the next...
		}
		if ( it == entryMap.end() )  // all Entrys have been checked...
		{
			found++;
			break;
		}
	}
	if ( !found )
	{
		eMessageBox("No more unused satellites in satellites.xml. Please add a new satellite with possible transponder(s).","Could not add satellite." );
		return 0;
	}

	eLNB *lnb;
			// if not in user-defined mode,

	int tmp = complexity;
	if (tmp > 4)
		tmp=1;

	if ( (tmp < 3) || !eTransponderList::getInstance()->getLNBs().size() )  // lnb list is empty ?
	{
		eTransponderList::getInstance()->getLNBs().push_back( eLNB(*eTransponderList::getInstance()) );
		lnb = &eTransponderList::getInstance()->getLNBs().back();
		lnb->setDefaultOptions();
	}
	else // we use the last lnb in the list for the new Satellite
		lnb = &eTransponderList::getInstance()->getLNBs().back();

	eSatellite *satellite = lnb->addSatellite( i->orbital_position );
	satellite->setDescription(i->name);

	if ( (tmp < 3) || !eTransponderList::getInstance()->getLNBs().size() )  // lnb list is empty ?
		setSimpleDiseqc( satellite, 0 );

	eSwitchParameter &sParams = satellite->getSwitchParams();
	sParams.VoltageMode = eSwitchParameter::HV;
	sParams.HiLoSignal = eSwitchParameter::HILO;
	return satellite;
}

void eSatelliteConfigurationManager::updateScrollbar(int show)
{
	if (!show)
	{
		scrollbar->hide();
		return;
	}
	int total=entryMap.size()*40; // w_buttons->getSize().height();
	if (total < buttonWidget->getSize().height())
		total=buttonWidget->getSize().height();
	if (!total)
		total=1;
	int start=-w_buttons->getPosition().y()*100/total;
	int vis=buttonWidget->getSize().height()*100/total;
	scrollbar->setParams(start,vis);
	scrollbar->show();
}

int eSatelliteConfigurationManager::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
#ifndef DISABLE_LCD
	case eWidgetEvent::execBegin:
		w_buttons->setLCD( LCDTitle, LCDElement );
	break;
#endif
	case eWidgetEvent::evtAction:
		if (event.action == &i_focusActions->left)
			focusNext(eWidget::focusDirW);
		else if (event.action == &i_focusActions->right)
			focusNext(eWidget::focusDirE);
		else if (event.action == &i_focusActions->up)
			focusNext(eWidget::focusDirN);
		else if (event.action == &i_focusActions->down)
			focusNext(eWidget::focusDirS);
		else
			return eWindow::eventHandler(event);
		break;
	default:
		return eWindow::eventHandler(event);
		break;
	}
  return 1;
}

eLNBSetup::eLNBSetup( eSatellite* sat, eWidget* lcdTitle, eWidget* lcdElement )
	:sat(sat), service(eServiceInterface::getInstance()->service)
{
	init_eLNBSetup(lcdTitle, lcdElement );
}
void eLNBSetup::init_eLNBSetup(eWidget* lcdTitle, eWidget* lcdElement )
{
	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "eLNBSetup"))
		eFatal("skin load of \"eLNBSetup\" failed");
	LNBPage = new eLNBPage( this, sat );
	DiSEqCPage = new eDiSEqCPage( this, sat );
#ifndef DISABLE_LCD
	DiSEqCPage->setLCD( lcdTitle, lcdElement );
	LNBPage->setLCD( lcdTitle, lcdElement );
#endif
	DiSEqCPage->hide();
	LNBPage->hide();
	mp.addPage(LNBPage);
	mp.addPage(DiSEqCPage);
// here we can not use the Makro CONNECT ... slot (*this, .... is here not okay..
	LNBPage->lnb_list->selchanged.connect( slot( *LNBPage, &eLNBPage::lnbChanged) );
	LNBPage->lnb_list->selchanged.connect( slot( *DiSEqCPage, &eDiSEqCPage::lnbChanged) );
//	CONNECT( DiSEqCPage->next->selected, eLNBSetup::onNext );
	CONNECT( LNBPage->next->selected, eLNBSetup::onNext );
	CONNECT( DiSEqCPage->prev->selected, eLNBSetup::onPrev );  
	CONNECT( LNBPage->save->selected, eLNBSetup::onSave );
	CONNECT( DiSEqCPage->save->selected, eLNBSetup::onSave);
} 

void eLNBSetup::onSave()
{
	eLNB *p = (eLNB*) LNBPage->lnb_list->getCurrent()->getKey();

	if ( !p )  // then we must create new LNB; (New is selected)
	{
		eTransponderList::getInstance()->getLNBs().push_back( eLNB( *eTransponderList::getInstance() ) );  // add new LNB
		p = &eTransponderList::getInstance()->getLNBs().back();   // get address from the new lnb
		p->getDiSEqC().setRotorDefaultOptions();
//		eDebug("now we have a new LNB Created = %p", p );
	}
/*	else
		eDebug("do not create LNB");*/

	p->setLOFLo( LNBPage->lofL->getNumber() * 1000 );
	p->setLOFHi( LNBPage->lofH->getNumber() * 1000 );
	p->setLOFThreshold( LNBPage->threshold->getNumber() * 1000 );
	p->setIncreasedVoltage( LNBPage->increased_voltage->isChecked() );
	p->set12VOut( LNBPage->relais_12V_out->isChecked() );

	p->getDiSEqC().MiniDiSEqCParam = (eDiSEqC::tMiniDiSEqCParam) (int) DiSEqCPage->MiniDiSEqCParam->getCurrent()->getKey();
	p->getDiSEqC().DiSEqCMode = (eDiSEqC::tDiSEqCMode) (int) DiSEqCPage->DiSEqCMode->getCurrent()->getKey();
	p->getDiSEqC().DiSEqCParam = (int) DiSEqCPage->DiSEqCParam->getCurrent()->getKey();
	p->getDiSEqC().DiSEqCRepeats = (int) DiSEqCPage->DiSEqCRepeats->getCurrent()->getKey();
	p->getDiSEqC().FastDiSEqC = (int) DiSEqCPage->FastDiSEqC->isChecked();
	p->getDiSEqC().SeqRepeat = DiSEqCPage->SeqRepeat->isChecked();
	p->getDiSEqC().SwapCmds = DiSEqCPage->SwapCmds->isChecked();	
	p->getDiSEqC().uncommitted_cmd = (int)DiSEqCPage->ucInput->getCurrent()->getKey();

	if ( p != sat->getLNB() )  // the satellite must removed from the old lnb and inserts in the new
	{
		p->addSatellite( sat->getLNB()->takeSatellite( sat ) );
/*		eDebug("remove satellite from lnb... now %i satellites left", sat->getLNB()->getSatelliteList().size() );
		eDebug("added satellite to lnb... now %i satellites in lnb", p->getSatelliteList().size() );		*/

		if ( !sat->getLNB()->getSatelliteList().size() )   // the lnb that have no more satellites must delete
		{
//			eDebug("delete no more used lnb");
		  eTransponderList::getInstance()->getLNBs().remove( *sat->getLNB() );
		}
/*		else
			eDebug("lnb not deleted");*/

		// now we must set the LNB Pointer in eSatellite...
		sat->setLNB(p);

		close(-1); // we must reposition control elements in eSatelliteConfigurationManager
	}
	else
		close(0); // we must not reposition...

	if ( service )
	{
		eServiceInterface::getInstance()->stop();
		eFrontend::getInstance()->savePower();
		eZapMain::getInstance()->playService(service, eZapMain::psDontAdd|eZapMain::psSetMode );
	}
	else
		eFrontend::getInstance()->InitDiSEqC();
	eTransponderList::getInstance()->writeLNBData();
	eConfig::getInstance()->flush();
}

int eLNBSetup::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::execBegin:
		mp.first();
		LNBPage->lnbChanged( LNBPage->lnb_list->getCurrent() );   // fake selchanged for initialize lofl, lofh, threshold...
		DiSEqCPage->lnbChanged( LNBPage->lnb_list->getCurrent() );   // fake selchanged for initialize lofl, lofh, threshold...
	break;
	default:
		break;
	}
	return eWindow::eventHandler(event);
}

struct eLNBPage::selectlnb
{
	const eLNB *lnb;
	eComboBox *lb;

	selectlnb(const eLNB* lnb, eComboBox* lb): lnb(lnb), lb(lb)
	{
	}

	bool operator()(const eListBoxEntryText& s)
	{
		if ( lnb == (eLNB*)s.getKey() )
		{
			lb->setCurrent(&s,true);
			return 1;
		}
		return 0;
	}
};

eLNBPage::eLNBPage( eWidget *parent, eSatellite* sat )
  :eWidget(parent), sat(sat)
{
	init_eLNBPage(parent);
}
void eLNBPage::init_eLNBPage( eWidget *parent)
{
#ifndef DISABLE_LCD
	LCDTitle=parent->LCDTitle;
	LCDElement=parent->LCDElement;
#endif  
	lnb_list = new eComboBox(this,8);
	lnb_list->setName("lnblist");

	eLabel *l = new eLabel(this);
	l->setName("lLofL");
	lofL = new eNumber(this, 5, 0, 9, 1, 0, 0, l);  // todo descr label im skin mit name versehen für lcd anzeige
	lofL->setName("lofL");

	l = new eLabel(this);
	l->setName("lLofH");
	lofH = new eNumber(this, 5, 0, 9, 1, 0, 0, l);  // todo descr label im skin mit name versehen für lcd anzeige
	lofH->setName("lofH");

	l = new eLabel(this);
	l->setName("lThreshold");
	threshold = new eNumber(this, 5, 0 ,9, 1, 0, 0, l);
	threshold->setName("threshold");

	increased_voltage = new eCheckbox( this );
	increased_voltage->setName("increased_voltage");

	relais_12V_out = new eCheckbox( this );
	relais_12V_out->setName("relais_12V_out");
                                       
	save = new eButton(this);
	save->setName("save");

	next = new eButton(this);
	next->setName("next");

	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "eLNBPage"))
		eFatal("skin load of \"eLNBPage\" failed");

	// add all LNBs

	int i=0;
	for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); it++)
		new eListBoxEntryText(*lnb_list, eString().sprintf("LNB %i", i++), (void*)&(*it) );

	// fill with new lnbs
	while (i < 32)
		new eListBoxEntryText(*lnb_list, eString().sprintf("LNB %i", i++), (void*) 0 );

	lnb_list->forEachEntry( selectlnb( sat->getLNB(), lnb_list ) );

	CONNECT( lofL->selected, eLNBPage::numSelected);
	CONNECT( lofH->selected, eLNBPage::numSelected);
	CONNECT( threshold->selected, eLNBPage::numSelected);

	if ( eSystemInfo::getInstance()->getHwType() != eSystemInfo::DM7020 )
		relais_12V_out->hide();
 // on exec we begin in eventHandler execBegin
}

void eLNBPage::lnbChanged( eListBoxEntryText *lnb )
{
	unsigned int l1;
	unsigned int l2;
	unsigned int l3;
	int incVoltage = 0;
	int relais12V = 0;
	
	eLNB::getRegionDefaultLOFs(&l1, &l2, &l3);
	
	if ( lnb && lnb->getKey() )
	{
		l1 = ((eLNB*)lnb->getKey())->getLOFLo();
		l2 = ((eLNB*)lnb->getKey())->getLOFHi();
		l3 = ((eLNB*)lnb->getKey())->getLOFThreshold();
		incVoltage = ((eLNB*)lnb->getKey())->getIncreasedVoltage();
		relais12V = ((eLNB*)lnb->getKey())->get12VOut();
	}
	lofL->setNumber( l1 / 1000 );
	lofH->setNumber( l2 / 1000 );
	increased_voltage->setCheck( incVoltage );
	relais_12V_out->setCheck( relais12V );
	threshold->setNumber( l3 / 1000 );
}

void eLNBPage::numSelected(int*)
{
	focusNext( eWidget::focusDirNext );
}

eDiSEqCPage::eDiSEqCPage( eWidget *parent, eSatellite *sat)
	:eWidget(parent), sat(sat)
{
	init_eDiSEqCPage(parent, sat);
}

void eDiSEqCPage::init_eDiSEqCPage( eWidget *parent, eSatellite *sat)
{
#ifndef DISABLE_LCD
	LCDTitle=parent->LCDTitle;
	LCDElement=parent->LCDElement;
#endif
	eLabel *l = new eLabel(this);
	l->setName("lMiniDiSEqCPara");
	MiniDiSEqCParam = new eComboBox( this, 4, l );
	MiniDiSEqCParam->setName("MiniDiSEqCParam");
	new eListBoxEntryText( *MiniDiSEqCParam, _("None"), (void*)eDiSEqC::NO, 0, _("sends no tone burst") );
	new eListBoxEntryText( *MiniDiSEqCParam, "A", (void*)eDiSEqC::A, 0, _("sends modulated tone burst") );
	new eListBoxEntryText( *MiniDiSEqCParam, "B", (void*)eDiSEqC::B, 0, _("sends unmodulated tone burst") );

	l = new eLabel(this);
	l->setName("lDiSEqCMode");
	DiSEqCMode = new eComboBox( this, 4, l );
	DiSEqCMode->setName("DiSEqCMode");
	// *DiSEqCMode... here we use the operator eListBox* from eComboBox !
	new eListBoxEntryText( *DiSEqCMode, "None", (void*)eDiSEqC::NONE, 0, _("Disable DiSEqC") );
	new eListBoxEntryText( *DiSEqCMode, "Version 1.0", (void*)eDiSEqC::V1_0, 0, _("Use DiSEqC Version 1.0") );
	new eListBoxEntryText( *DiSEqCMode, "Version 1.1", (void*)eDiSEqC::V1_1, 0, _("Use DiSEqC Version 1.1") );
	new eListBoxEntryText( *DiSEqCMode, "Version 1.2", (void*)eDiSEqC::V1_2, 0, _("Use DiSEqC Version 1.2") );
	// no SMATV at the moment... we can do this when anyone ask...
	// 	new eListBoxEntryText( *DiSEqCMode, "SMATV", (void*)eDiSEqC::SMATV, 0, _("Use SMATV Remote Tuning") );

	lDiSEqCParam = new eLabel(this);
	lDiSEqCParam->setName("lDiSEqCParam");
	DiSEqCParam = new eComboBox( this, 4, lDiSEqCParam );
	DiSEqCParam->setName("DiSEqCParam");
	new eListBoxEntryText( *DiSEqCParam, "A/A", (void*)eDiSEqC::AA, 0, _("sends DiSEqC cmd A/A") );
	new eListBoxEntryText( *DiSEqCParam, "A/B", (void*)eDiSEqC::AB, 0, _("sends DiSEqC cmd A/B") );
	new eListBoxEntryText( *DiSEqCParam, "B/A", (void*)eDiSEqC::BA, 0, _("sends DiSEqC cmd B/A") );
	new eListBoxEntryText( *DiSEqCParam, "B/B", (void*)eDiSEqC::BB, 0, _("sends DiSEqC cmd B/B") );
	new eListBoxEntryText( *DiSEqCParam, "None", (void*)eDiSEqC::SENDNO, 0, _("sends no committed DiSEqC cmd") );
	new eListBoxEntryText( *DiSEqCParam, "1", (void*)0xF0, 0, _("sends switch cmd 1") );
	new eListBoxEntryText( *DiSEqCParam, "2", (void*)0xF1, 0, _("sends switch cmd 2") );
	new eListBoxEntryText( *DiSEqCParam, "3", (void*)0xF2, 0, _("sends switch cmd 3") );
	new eListBoxEntryText( *DiSEqCParam, "4", (void*)0xF3, 0, _("sends switch cmd 4") );
	new eListBoxEntryText( *DiSEqCParam, "5", (void*)0xF4, 0, _("sends switch cmd 5") );
	new eListBoxEntryText( *DiSEqCParam, "6", (void*)0xF5, 0, _("sends switch cmd 6") );
	new eListBoxEntryText( *DiSEqCParam, "7", (void*)0xF6, 0, _("sends switch cmd 7") );
	new eListBoxEntryText( *DiSEqCParam, "8", (void*)0xF7, 0, _("sends switch cmd 8") );
	new eListBoxEntryText( *DiSEqCParam, "9", (void*)0xF8, 0, _("sends switch cmd 9") );
	new eListBoxEntryText( *DiSEqCParam, "10", (void*)0xF9, 0, _("sends switch cmd 10") );
	new eListBoxEntryText( *DiSEqCParam, "11", (void*)0xFA, 0, _("sends switch cmd 11") );
	new eListBoxEntryText( *DiSEqCParam, "12", (void*)0xFB, 0, _("sends switch cmd 12") );
	new eListBoxEntryText( *DiSEqCParam, "13", (void*)0xFC, 0, _("sends switch cmd 13") );
	new eListBoxEntryText( *DiSEqCParam, "14", (void*)0xFD, 0, _("sends switch cmd 14") );
	new eListBoxEntryText( *DiSEqCParam, "15", (void*)0xFE, 0, _("sends switch cmd 15") );
	new eListBoxEntryText( *DiSEqCParam, "16", (void*)0xFF, 0, _("sends switch cmd 16") );

	lDiSEqCRepeats = new eLabel(this);
	lDiSEqCRepeats->setName("lDiSEqCRepeats");
	DiSEqCRepeats = new eComboBox( this, 4, lDiSEqCRepeats );
	DiSEqCRepeats->setName("DiSEqCRepeats");
	new eListBoxEntryText( *DiSEqCRepeats, _("None"), (void*)0, 0, _("sends no DiSEqC repeats") );
	new eListBoxEntryText( *DiSEqCRepeats, _("One"), (void*)1, 0, _("sends one repeat") );
	new eListBoxEntryText( *DiSEqCRepeats, _("Two"), (void*)2, 0, _("sends two repeats") );
	new eListBoxEntryText( *DiSEqCRepeats, _("Three"), (void*)3, 0, _("sends three repeats") );

	lucInput = new eLabel(this);
	lucInput->setName("lucInput");

	ucInput = new eComboBox( this, 8, lucInput );
	ucInput->setName("ucInput");
	new eListBoxEntryText( *ucInput, _("None"), (void*)0, 0, _("sends no uncommitted switch command") );
	new eListBoxEntryText( *ucInput, _("Input 1"), (void*)240, 0, _("select uncommitted switch Input 1") );
	new eListBoxEntryText( *ucInput, _("Input 2"), (void*)241, 0, _("select uncommitted switch Input 2") );
	new eListBoxEntryText( *ucInput, _("Input 3"), (void*)242, 0, _("select uncommitted switch Input 3") );
	new eListBoxEntryText( *ucInput, _("Input 4"), (void*)243, 0, _("select uncommitted switch Input 4") );
	new eListBoxEntryText( *ucInput, _("Input 5"), (void*)244, 0, _("select uncommitted switch Input 5") );
	new eListBoxEntryText( *ucInput, _("Input 6"), (void*)245, 0, _("select uncommitted switch Input 6") );
	new eListBoxEntryText( *ucInput, _("Input 7"), (void*)246, 0, _("select uncommitted switch Input 7") );
	new eListBoxEntryText( *ucInput, _("Input 8"), (void*)247, 0, _("select uncommitted switch Input 8") );
	new eListBoxEntryText( *ucInput, _("Input 9"), (void*)248, 0, _("select uncommitted switch Input 9") );
	new eListBoxEntryText( *ucInput, _("Input 10"), (void*)249, 0, _("select uncommitted switch Input 10") );
	new eListBoxEntryText( *ucInput, _("Input 11"), (void*)250, 0, _("select uncommitted switch Input 11") );
	new eListBoxEntryText( *ucInput, _("Input 12"), (void*)251, 0, _("select uncommitted switch Input 12") );
	new eListBoxEntryText( *ucInput, _("Input 13"), (void*)252, 0, _("select uncommitted switch Input 13") );
	new eListBoxEntryText( *ucInput, _("Input 14"), (void*)253, 0, _("select uncommitted switch Input 14") );
	new eListBoxEntryText( *ucInput, _("Input 15"), (void*)254, 0, _("select uncommitted switch Input 15") );
	new eListBoxEntryText( *ucInput, _("Input 16"), (void*)255, 0, _("select uncommitted switch Input 16") );
    
	SeqRepeat = new eCheckbox(this);
	SeqRepeat->setName("SeqRepeat");

	FastDiSEqC = new eCheckbox(this);
	FastDiSEqC->setName("FastDiSEqC");

	SwapCmds = new eCheckbox(this);
	SwapCmds->setName("SwapCmds");

/*	next = new eButton(this);
	next->setName("next");*/
          
	prev = new eButton(this);
	prev->setName("prev");
  
	save = new eButton(this);
	save->setName("save");

	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "eDiSEqCPage"))
		eFatal("skin load of \"eDiSEqCPage\" failed");

	CONNECT( DiSEqCMode->selchanged, eDiSEqCPage::DiSEqCModeChanged );
	addActionMap(&i_focusActions->map);
}

void eDiSEqCPage::numSelected(int*)
{
	focusNext( eWidget::focusDirNext );
}

void eDiSEqCPage::DiSEqCModeChanged( eListBoxEntryText* e )
{
	switch( (int) e->getKey() )
	{
		case eDiSEqC::V1_2:
//			next->show();
		case eDiSEqC::V1_1:
			lDiSEqCRepeats->show();
			DiSEqCRepeats->show();
			lucInput->show();
			ucInput->show();
			SwapCmds->show();
		case eDiSEqC::V1_0:
			lDiSEqCParam->show();
			FastDiSEqC->show();
			DiSEqCParam->show();
			SeqRepeat->show();
		break;
	}
	switch ( (int) e->getKey() )
	{
		default:
		case eDiSEqC::NONE:
			FastDiSEqC->hide();
			SeqRepeat->hide();
			DiSEqCParam->hide();
			lDiSEqCParam->hide();
		case eDiSEqC::V1_0:
			lucInput->hide();
			ucInput->hide();
			lDiSEqCRepeats->hide();
			DiSEqCRepeats->hide();
			SwapCmds->hide();
		case eDiSEqC::V1_1:
//			next->hide();
		case eDiSEqC::V1_2: // hide nothing
		break;
  }
}

void eDiSEqCPage::lnbChanged( eListBoxEntryText *lnb )
{
	if ( lnb && lnb->getKey() )
	{
		eDiSEqC &diseqc = ((eLNB*)lnb->getKey())->getDiSEqC();
		DiSEqCMode->setCurrent( (void*) (int) diseqc.DiSEqCMode );
		MiniDiSEqCParam->setCurrent( (void*) diseqc.MiniDiSEqCParam );
		DiSEqCParam->setCurrent( (void*) diseqc.DiSEqCParam );
		DiSEqCRepeats->setCurrent( (void*) diseqc.DiSEqCRepeats );
		FastDiSEqC->setCheck( (int) diseqc.FastDiSEqC );
		SeqRepeat->setCheck( (int) diseqc.SeqRepeat );
		SwapCmds->setCheck( (int) diseqc.SwapCmds );	
		ucInput->setCurrent( (void*) diseqc.uncommitted_cmd );
	}
	else
	{
		DiSEqCMode->setCurrent( 0 );
		DiSEqCParam->setCurrent( 0 );
		MiniDiSEqCParam->setCurrent( 0 );
		DiSEqCRepeats->setCurrent( 0 );
		FastDiSEqC->setCheck( 0 );
		SeqRepeat->setCheck( 0 );
		SwapCmds->setCheck( 0 );
		ucInput->setCurrent( (void*) 0 );
	}
	DiSEqCModeChanged( (eListBoxEntryText*) DiSEqCMode->getCurrent() );
}

