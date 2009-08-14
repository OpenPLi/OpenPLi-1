#include <math.h>
#include <lib/dvb/dvbwidgets.h>
#include <lib/dvb/frontend.h>
#include <lib/dvb/dvb.h>
#include <lib/gui/eskin.h>
#include <lib/gui/enumber.h>
#include <lib/gui/combobox.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/eprogress.h>

eTransponderWidget::eTransponderWidget(eWidget *parent, int edit, int type)
	:eWidget(parent), type(type), edit(edit)
{
	init_eTransponderWidget(parent, edit, type);
}

void eTransponderWidget::init_eTransponderWidget(eWidget *parent, int edit, int type)
{
#ifndef DISABLE_LCD
	LCDTitle=parent->LCDTitle;
	LCDElement=parent->LCDElement;
#endif

	eLabel *l = 0;

	if ( type & deliverySatellite )
	{          
		lsat = new eLabel(this);
		lsat->setName( "lSat" );

		sat=new eComboBox(this, 4, l, edit);
		sat->setName("sat");

		std::map<int,eSatellite*> sats;
		for ( std::list<eLNB>::iterator it( eTransponderList::getInstance()->getLNBs().begin() ); it != eTransponderList::getInstance()->getLNBs().end(); it++)
			for ( ePtrList<eSatellite>::iterator s ( it->getSatelliteList().begin() ); s != it->getSatelliteList().end(); s++)
				sats[s->getOrbitalPosition()]=s;
		for (std::map<int,eSatellite*>::iterator it = sats.begin(); it != sats.end(); ++it)
				new eListBoxEntryText(*sat, it->second->getDescription().c_str(), (void*) it->second);

		CONNECT(sat->selchanged, eTransponderWidget::updated1);
	}

	l = new eLabel(this);
	l->setName( "lFreq" );

	int init[5]={0,0,0,0,0};

	if ( type & deliveryTerrestrial )
	    frequency=new eNumber(this, 6, 0, 9, 1, init, 0, l, edit);
	else
	    frequency=new eNumber(this, 5, 0, 9, 1, init, 0, l, edit);
	
	frequency->setName("frequency");

	inversion=new eCheckbox(this);
	inversion->setName("inversion");

	l = new eLabel(this);
	l->setName( "lPol" );

	polarity=new eListBox<eListBoxEntryText>(this, l, edit);
	polarity->setName("polarity");
	if ( type & deliverySatellite )
	{
		polarityEntry[0]=new eListBoxEntryText(polarity, _("horizontal"), (void*)0);
		polarityEntry[1]=new eListBoxEntryText(polarity, _("vertical"), (void*)1);
	}
	else if ( type & deliveryCable ) // modulation
	{
		polarityEntry[0]=new eListBoxEntryText(polarity, _("Auto"), (void*)0);
		polarityEntry[1]=new eListBoxEntryText(polarity, _("16-QAM"), (void*)1);
		polarityEntry[2]=new eListBoxEntryText(polarity, _("32-QAM"), (void*)2);
		polarityEntry[3]=new eListBoxEntryText(polarity, _("64-QAM"), (void*)3);
		polarityEntry[4]=new eListBoxEntryText(polarity, _("128-QAM"), (void*)4);
		polarityEntry[5]=new eListBoxEntryText(polarity, _("256-QAM"), (void*)5);
	}
	else if ( type & deliveryTerrestrial )  // constellation
	{
		polarityEntry[0]=new eListBoxEntryText(polarity, _("Auto"), (void*)-1);
		polarityEntry[1]=new eListBoxEntryText(polarity, _("QPSK"), (void*)0);
		polarityEntry[2]=new eListBoxEntryText(polarity, _("16-QAM"), (void*)1);
		polarityEntry[3]=new eListBoxEntryText(polarity, _("64-QAM"), (void*)2);
	}

	l = new eLabel(this);
	l->setName( "lFec" );
	fec=new eListBox<eListBoxEntryText>(this, l, edit);
	fec->setName("fec");
	if ( type & (deliverySatellite|deliveryCable) )
	{
		fecEntry[0]=new eListBoxEntryText(fec, "Auto", (void*)0);
		fecEntry[1]=new eListBoxEntryText(fec, "1/2", (void*)1);
		fecEntry[2]=new eListBoxEntryText(fec, "2/3", (void*)2);
		fecEntry[3]=new eListBoxEntryText(fec, "3/4", (void*)3);
		fecEntry[4]=new eListBoxEntryText(fec, "5/6", (void*)4);
		fecEntry[5]=new eListBoxEntryText(fec, "7/8", (void*)5);
		fecEntry[6]=new eListBoxEntryText(fec, "8/9", (void*)6);
	}
	else if ( type & deliveryTerrestrial )  // guard interval
	{
		fecEntry[0]=new eListBoxEntryText(fec, "Auto", (void*)-1);
		fecEntry[1]=new eListBoxEntryText(fec, "1/32", (void*)0);
		fecEntry[2]=new eListBoxEntryText(fec, "1/16", (void*)1);
		fecEntry[3]=new eListBoxEntryText(fec, "1/8", (void*)2);
		fecEntry[4]=new eListBoxEntryText(fec, "1/4", (void*)3);
	}
	CONNECT(fec->selchanged, eTransponderWidget::updated1);
	CONNECT(polarity->selchanged, eTransponderWidget::updated1);

	if ( type & deliveryTerrestrial )
	{
		l = new eLabel(this);
		l->setName("lBandwidth");
		bandwidth=new eListBox<eListBoxEntryText>(this, l, edit);
		bandwidth->setName("Bandwidth");
		bandwidthEntry[0]=new eListBoxEntryText(bandwidth, "8 MHz", (void*)0);
		bandwidthEntry[1]=new eListBoxEntryText(bandwidth, "7 MHz", (void*)1);
		bandwidthEntry[2]=new eListBoxEntryText(bandwidth, "6 MHz", (void*)2);
		CONNECT(bandwidth->selchanged, eTransponderWidget::updated1);
		l = new eLabel(this);
		l->setName("ltmMode");
		tmMode=new eListBox<eListBoxEntryText>(this, l, edit);
		tmMode->setName("tmMode");
		tmModeEntry[0]=new eListBoxEntryText(tmMode, "Auto", (void*)-1);
		tmModeEntry[1]=new eListBoxEntryText(tmMode, "2k", (void*)0);
		tmModeEntry[2]=new eListBoxEntryText(tmMode, "8k", (void*)1);
		CONNECT(tmMode->selchanged, eTransponderWidget::updated1);
		l = new eLabel(this);
		l->setName("lCodeRateLP");
		codeRateLP=new eListBox<eListBoxEntryText>(this, l, edit);
		codeRateLP->setName("codeRateLP");
		codeRateLPEntry[0]=new eListBoxEntryText(codeRateLP, "Auto", (void*)-1);
		codeRateLPEntry[1]=new eListBoxEntryText(codeRateLP, "1/2", (void*)0);
		codeRateLPEntry[2]=new eListBoxEntryText(codeRateLP, "2/3", (void*)1);
		codeRateLPEntry[3]=new eListBoxEntryText(codeRateLP, "3/4", (void*)2);
		codeRateLPEntry[4]=new eListBoxEntryText(codeRateLP, "5/6", (void*)3);
		codeRateLPEntry[5]=new eListBoxEntryText(codeRateLP, "7/8", (void*)4);
		CONNECT(codeRateLP->selchanged, eTransponderWidget::updated1);
		l = new eLabel(this);
		l->setName("lCodeRateHP");
		codeRateHP=new eListBox<eListBoxEntryText>(this, l, edit);
		codeRateHP->setName("codeRateHP");
		codeRateHPEntry[0]=new eListBoxEntryText(codeRateHP, "Auto", (void*)-1);
		codeRateHPEntry[1]=new eListBoxEntryText(codeRateHP, "1/2", (void*)0);
		codeRateHPEntry[2]=new eListBoxEntryText(codeRateHP, "2/3", (void*)1);
		codeRateHPEntry[3]=new eListBoxEntryText(codeRateHP, "3/4", (void*)2);
		codeRateHPEntry[4]=new eListBoxEntryText(codeRateHP, "5/6", (void*)3);
		codeRateHPEntry[5]=new eListBoxEntryText(codeRateHP, "7/8", (void*)4);
		CONNECT(codeRateHP->selchanged, eTransponderWidget::updated1);
	}
	else
	{
		l = new eLabel(this);
		l->setName( "lSymb" );
		symbolrate=new eNumber(this, 5, 0, 9, 1, init, 0, l, edit);
		symbolrate->setName("symbolrate");
		CONNECT_1_0(symbolrate->numberChanged, eTransponderWidget::updated2, 0);	
		CONNECT(symbolrate->selected, eTransponderWidget::nextField0);
	}

	CONNECT_1_0(frequency->numberChanged, eTransponderWidget::updated2, 0);
	CONNECT(frequency->selected, eTransponderWidget::nextField0);
	CONNECT(inversion->checked, eTransponderWidget::updated2);
}

void eTransponderWidget::nextField0(int *)
{
	focusNext(eWidget::focusDirNext);
}

void eTransponderWidget::updated1(eListBoxEntryText *)
{
 updated();
}

void eTransponderWidget::updated2(int)
{
	updated();
}

int eTransponderWidget::load()
{
	eString name="transpondersettings.";
	switch (type&7)
	{
	case deliveryCable:
		name+="cable";
		break;
	case deliverySatellite:
		name+="satellite";
		break;
	case deliveryTerrestrial:
		name+="terrestrial";
		break;
	default:
		return -2;
	}
	if (eSkin::getActive()->build(this, name.c_str()))
		return -1;
	if ( type & flagNoSat )
	{
		lsat->hide();
		sat->hide();
	}
	if ( type & flagNoInv )
		inversion->hide();
	return 0;
}

struct selectSat
{
	const eTransponder* t;
	eComboBox *l;

	selectSat(const eTransponder *t, eComboBox* l ): t(t), l(l)
	{
	}

	bool operator()(eListBoxEntryText& e)
	{
//		eDebug("we have %d, we want %d",((eSatellite*)e.getKey())->getOrbitalPosition(), t->satellite.orbital_position );
		if ( ((eSatellite*)e.getKey())->getOrbitalPosition() == t->satellite.orbital_position )
		{
			l->setCurrent(&e);
			return 1;
		}
		return 0;
	}
};

int eTransponderWidget::setTransponder(const eTransponder *transponder)
{
	if (!transponder)
		return -1;
	
	switch (type&7)
	{
	case deliveryCable:
		if (!transponder->cable.valid)
			return -1;
		if (transponder->satellite.fec >= 0 && transponder->satellite.fec < 7)
			fec->setCurrent(fecEntry[transponder->satellite.fec]);
		else
			fec->setCurrent(fecEntry[0]);
		frequency->setNumber(transponder->cable.frequency/1000);
		symbolrate->setNumber(transponder->cable.symbol_rate/1000);
		inversion->setCheck(transponder->cable.inversion==1);
		if ( transponder->cable.modulation >=1 && transponder->cable.modulation < 6 )
			polarity->setCurrent(polarityEntry[transponder->cable.modulation]);
		else
			polarity->setCurrent(polarityEntry[0]);
		break;
	case deliverySatellite:
		if (!transponder->satellite.valid)
			return -1;
		frequency->setNumber(transponder->satellite.frequency/1000);

		if (transponder->satellite.fec >= 0 && transponder->satellite.fec < 7)
			fec->setCurrent(fecEntry[transponder->satellite.fec]);
		else
			fec->setCurrent(fecEntry[0]);

		polarity->setCurrent(polarityEntry[transponder->satellite.polarisation&1]);
		symbolrate->setNumber(transponder->satellite.symbol_rate/1000);
		inversion->setCheck(transponder->satellite.inversion==1);

		if ( sat->forEachEntry(selectSat(transponder, sat)) != eListBoxBase::OK )
			sat->setCurrent(0);
		break;
	case deliveryTerrestrial:
		if (!transponder->terrestrial.valid)
			return -1;
		frequency->setNumber(transponder->terrestrial.centre_frequency/1000);
		inversion->setCheck(transponder->terrestrial.inversion==1);

		if (transponder->terrestrial.constellation >= 0 && transponder->terrestrial.constellation < 3)
			polarity->setCurrent(polarityEntry[transponder->terrestrial.constellation+1]);
		else
			polarity->setCurrent(polarityEntry[0]);

		if (transponder->terrestrial.guard_interval >= 0 && transponder->terrestrial.guard_interval < 4)
			fec->setCurrent(fecEntry[transponder->terrestrial.guard_interval+1]);
		else
			fec->setCurrent(fecEntry[0]);

		if (transponder->terrestrial.bandwidth >= 0 && transponder->terrestrial.bandwidth < 3)
			bandwidth->setCurrent(bandwidthEntry[transponder->terrestrial.bandwidth]);
		else
			bandwidth->setCurrent(bandwidthEntry[0]);

		if ( transponder->terrestrial.transmission_mode >= 0 && transponder->terrestrial.transmission_mode < 3)
			tmMode->setCurrent(tmModeEntry[transponder->terrestrial.transmission_mode+1]);
		else
			tmMode->setCurrent(tmModeEntry[0]);

		if (transponder->terrestrial.code_rate_lp >= 0 && transponder->terrestrial.code_rate_lp < 5)
			codeRateLP->setCurrent(codeRateLPEntry[transponder->terrestrial.code_rate_lp+1]);
		else
			codeRateLP->setCurrent(codeRateLPEntry[0]);

		if (transponder->terrestrial.code_rate_hp >= 0 && transponder->terrestrial.code_rate_hp < 5)
			codeRateHP->setCurrent(codeRateHPEntry[transponder->terrestrial.code_rate_hp+1]);
		else
			codeRateHP->setCurrent(codeRateHPEntry[0]);
		break;
	default:
		break;
	}
	return 0;
}

int eTransponderWidget::getTransponder(eTransponder *transponder)
{
	switch (type&7)
	{
	case deliveryCable:
		eDebug("deliveryCable");
		transponder->setCable(frequency->getNumber()*1000, symbolrate->getNumber()*1000, inversion->isChecked(), (int)polarity->getCurrent()->getKey(), (int)fec->getCurrent()->getKey() );
		return 0;
	case deliverySatellite:
		eDebug("deliverySatellite");
		eDebug("setting to: %d %d %d %d %d %d",
			frequency->getNumber(),
			symbolrate->getNumber(),
			(int)polarity->getCurrent()->getKey(),
			(int)fec->getCurrent()->getKey(),
			sat->getCurrent() && sat->getCurrent()->getKey()?
				((eSatellite*)sat->getCurrent()->getKey())->getOrbitalPosition() : 0,
			inversion->isChecked());
		transponder->setSatellite(
			frequency->getNumber()*1000, 
			symbolrate->getNumber()*1000,
			(int)polarity->getCurrent()->getKey(), 
			(int)fec->getCurrent()->getKey(),
			sat->getCurrent() && sat->getCurrent()->getKey() ?
				((eSatellite*)sat->getCurrent()->getKey())->getOrbitalPosition()
				: 0,
			inversion->isChecked());
		return 0;
	case deliveryTerrestrial:
		transponder->setTerrestrial(
			frequency->getNumber()*1000,
			(int)bandwidth->getCurrent()->getKey(),
			(int)polarity->getCurrent()->getKey(), //constellation
			transponder->terrestrial.hierarchy_information,
			(int)codeRateLP->getCurrent()->getKey(),
			(int)codeRateHP->getCurrent()->getKey(),
			(int)fec->getCurrent()->getKey(), // guard_interval
			(int)tmMode->getCurrent()->getKey(),
			inversion->isChecked());
		return 0;
	default:
		return -1;
	}
}

int eFEStatusWidget::eventHandler(const eWidgetEvent &event)
{
//	eDebug("fe status widget: event %d", event.type);
	switch (event.type)
	{
	case eWidgetEvent::gotFocus:
	case eWidgetEvent::willShow:
		updatetimer.start(500);
		break;
	case eWidgetEvent::lostFocus:
	case eWidgetEvent::willHide:
		updatetimer.stop();
		break;
	default:
		return eWidget::eventHandler(event);
	}
	return 1;
}

eFEStatusWidget::eFEStatusWidget(eWidget *parent, eFrontend *fe): eWidget(parent), fe(fe), updatetimer(eApp)
{
	init_eFEStatusWidget();
}
void eFEStatusWidget::init_eFEStatusWidget()
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


	CONNECT(updatetimer.timeout, eFEStatusWidget::update);

	if (eSkin::getActive()->build(this, "eFEStatusWidget"))
		return;
}

void eFEStatusWidget::update()
{
	int status,snrrel,agcrel,berrel;
	eString snrstring,agcstring,berstring;
	fe->getStatus(status,snrrel, snrstring, agcrel, agcstring, berrel, berstring);
	p_agc->setPerc(agcrel);
	p_snr->setPerc(snrrel);
	p_ber->setPerc(berrel);
	lsnr_num->setText(snrstring);
	lsync_num->setText(agcstring);
	lber_num->setText(berstring);	
	c_lock->setCheck(!!(status & FE_HAS_LOCK));
	c_sync->setCheck(!!(status & FE_HAS_SYNC));
}
