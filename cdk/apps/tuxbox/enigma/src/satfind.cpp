#include "satfind.h"
#include "enigma_lcd.h"
#include <lib/base/ebase.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/elabel.h>
#include <lib/gui/eskin.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/combobox.h>
#include <lib/dvb/frontend.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/dvb.h>
#include <lib/system/info.h>
#include <math.h>

eSatfind::eSatfind(eFrontend *fe)
	:eWindow(0), updateTimer(eApp), fe(fe), current(0)
{
	init_eSatfind();
}
void eSatfind::init_eSatfind()
{
	p_snr=new eProgress(this);
	p_snr->setName("snr");

	p_agc=new eProgress(this);
	p_agc->setName("agc");

	p_ber=new eProgress(this);
	p_ber->setName("ber");

	c_sync=new eCheckbox(this, 0, 0);
	c_sync->setName("sync");

	c_lock=new eCheckbox(this, 0, 0);
	c_lock->setName("lock");

	lsnr_num=new eLabel(this);
	lsnr_num->setName("snr_num");

	lsync_num=new eLabel(this);
	lsync_num->setName("agc_num");

	lber_num=new eLabel(this);
	lber_num->setName("ber_num");

	sat = new eComboBox(this, 3);
	sat->setName("sat");
	CONNECT(sat->selchanged, eSatfind::satChanged );

	transponder = new eComboBox(this, 5);
	transponder->setName("transponder");

	CONNECT(updateTimer.timeout, eSatfind::update);

	eLabel *l = new eLabel(this);
	l->setName("lSat");

	if (eSkin::getActive()->build(this, "eSatfind"))
		return;

	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();

	if (sapi && sapi->transponder)
		current = sapi->transponder;

	eListBoxEntryText *sel=0;

	if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feTerrestrial )
	{
		setText(_("Signalfind"));
		l->setText(_("Region:"));
	}

	std::map<int,eSatellite*> sats;
	for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); it++)
		for ( ePtrList<eSatellite>::iterator s ( it->getSatelliteList().begin() ); s != it->getSatelliteList().end(); s++)
			sats[s->getOrbitalPosition()]=s;

	for ( std::list<tpPacket>::const_iterator i(eTransponderList::getInstance()->getNetworks().begin()); i != eTransponderList::getInstance()->getNetworks().end(); ++i)
		if ( ( sats.find(i->orbital_position) != sats.end()) || (eSystemInfo::getInstance()->getFEType() != eSystemInfo::feSatellite) )
		{
			if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite && current && i->orbital_position == current->satellite.orbital_position )
				sel = new eListBoxEntryText(*sat, i->name, (void*)&*i);
			else
				new eListBoxEntryText(*sat, i->name, (void*)&*i);
		}

	if ( sat->getCount() )
	{
		if ( sel )
			sat->setCurrent(sel,true);
		else
			sat->setCurrent(0,true);
	}

	CONNECT( eFrontend::getInstance()->s_RotorRunning, eSatfind::RotorRunning );
	CONNECT( eFrontend::getInstance()->tunedIn, eSatfind::tunedIn );

	CONNECT(transponder->selchanged, eSatfind::tpChanged);

	/* help text for satfinder screen */
	setHelpText(_("\tSatFind\n\n>>> [MENU] >>> [6] Setup\n>>> Service Searching >>> SatFind\n. . . . . . . . . .\n\n" \
								"Here you can see the quality of the signal your receiver is getting from your dish/lnb(s).\n. . . . . . . . . .\n\n" \
								"Usage:\n\nSNR\tSignal to Noise Ratio\n\nAGC\tAutomatic Gain Control\n\nBER\tBit Error Rate\n\n" \
								"SATTELITE\tSelect satellite to monitor\n\nTRANSPONDER\tSelect transponder to monitor\n\n[OK]/[EXIT]\tClose window"));
}

void eSatfind::RotorRunning(int)
{
	updateTimer.stop();
}

void eSatfind::satChanged(eListBoxEntryText *sat)
{
	transponder->clear();
	if (sat && sat->getKey())
	{
		eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
		eTransponder *tp=0;

		if (sapi && sapi->transponder)
			tp = sapi->transponder;

		eListBoxEntryText *sel=0;

		tpPacket *i = (tpPacket*) (sat->getKey());

		switch(eSystemInfo::getInstance()->getFEType())
		{
			case eSystemInfo::feSatellite:
				for (std::list<eTransponder>::const_iterator it( i->possibleTransponders.begin() ); it != i->possibleTransponders.end(); it++)
				{
					if ( tp && *tp == *it )
						sel = new eListBoxEntryText( *transponder, eString().sprintf("%d / %d / %c", it->satellite.frequency/1000, it->satellite.symbol_rate/1000, it->satellite.polarisation?'V':'H' ), (void*)&(*it) );
					else
						new eListBoxEntryText( *transponder, eString().sprintf("%d / %d / %c", it->satellite.frequency/1000, it->satellite.symbol_rate/1000, it->satellite.polarisation?'V':'H' ), (void*)&(*it) );
				}
				break;
			case eSystemInfo::feTerrestrial:
				for (std::list<eTransponder>::const_iterator it( i->possibleTransponders.begin() ); it != i->possibleTransponders.end(); it++)
				{
					if ( tp && *tp == *it )
						sel = new eListBoxEntryText( *transponder, eString().sprintf("%d kHz", it->terrestrial.centre_frequency/1000), (void*)&(*it) );
					else
						new eListBoxEntryText( *transponder, eString().sprintf("%d kHz", it->terrestrial.centre_frequency/1000), (void*)&(*it) );
				}
				break;
		}

		if (transponder->getCount())
		{
			if ( sel )
				transponder->setCurrent(sel,true);
			else
				transponder->setCurrent(0,true);
		}
	}
}

void eSatfind::tpChanged( eListBoxEntryText *tp )
{
	updateTimer.stop();
	if (tp && tp->getKey())
	{
		if ( current && *current == *((eTransponder*)tp->getKey()))
			return;
		current = (eTransponder*)(tp->getKey());
		current->tune();
	}
	else
		current = 0;
}

int eSatfind::eventHandler( const eWidgetEvent& e)
{
	switch (e.type)
	{
		case eWidgetEvent::execBegin:
#ifndef DISABLE_LCD
			eZapLCD::getInstance()->lcdMenu->hide();
			eZapLCD::getInstance()->lcdSatfind->show();
#endif
			updateTimer.start(250, true);
			break;
		case eWidgetEvent::execDone:
			updateTimer.stop();
#ifndef DISABLE_LCD
			eZapLCD::getInstance()->lcdSatfind->hide();
			eZapLCD::getInstance()->lcdMenu->show();
#endif
			break;
		default:
			return eWindow::eventHandler(e);
	}
	return 0;
}

void eSatfind::tunedIn(eTransponder *, int error )
{
	update();
}

void eSatfind::update()
{                    
	int snrrel,agcrel,berrel;
	eString snrstring,agcstring,berstring;	
	fe->getStatus(status,snrrel,snrstring,agcrel,agcstring,berrel,berstring);	
	p_agc->setPerc(agcrel);
	p_snr->setPerc(snrrel);
	p_ber->setPerc(berrel);
	lsnr_num->setText(snrstring);
	lsync_num->setText(agcstring);
	lber_num->setText(berstring);
	c_lock->setCheck(!!(status & FE_HAS_LOCK));
	c_sync->setCheck(!!(status & FE_HAS_SYNC));
#ifndef DISABLE_LCD
	eZapLCD::getInstance()->lcdSatfind->update(snrrel,agcrel);
#endif
	updateTimer.start(250,true);
}
