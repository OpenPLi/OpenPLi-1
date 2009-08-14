#include <src/time_correction.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/dvbservice.h>
#include <enigma_main.h>

eTimeCorrectionEditWindow::eTimeCorrectionEditWindow( tsref tp )
	:updateTimer(eApp), transponder(tp)
{
	init_eTimeCorrectionEditWindow(tp);
}

void eTimeCorrectionEditWindow::init_eTimeCorrectionEditWindow( tsref tp )
{
	int clktype = 0;
	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);

	setText(_("Time Correction"));
	cresize( eSize(440,290 ) );
	valign();

	eLabel *l = new eLabel(this);
	l->move( ePoint(10,10) );
	l->resize( eSize(200,30));
	l->setText(_("Receiver Time:"));

	lCurTime = new eLabel(this);
	lCurTime->move( ePoint(210,10) );
	lCurTime->resize( eSize(100,30) );

	lCurDate = new eLabel(this);
	lCurDate->move( ePoint(320,10) );
	lCurDate->resize( eSize(120,30) );

	l = new eLabel(this);
	l->move( ePoint(10,50) );
	l->resize( eSize(200,30));
	l->setText(_("Transponder Time:"));

	lTpTime = new eLabel(this);
	lTpTime->move( ePoint(210,50) );
	lTpTime->resize( eSize(100,30) );

	lTpDate = new eLabel(this);
	lTpDate->move( ePoint(320,50) );
	lTpDate->resize( eSize(120,30) );

	l = new eLabel(this);
	l->setText(_("New Time:"));
	l->setFlags( eLabel::flagVCenter );
	l->move( ePoint(10,90) );
	l->resize( eSize(150,35) );

	time_t now = time(0)+eDVB::getInstance()->time_difference;
	tm tmp = *localtime( &now );

	nTime = new eNumber(this, 2, 0, 59, 2, 0, 0, l);
	nTime->resize(eSize(75,35));
	nTime->move(ePoint(210,90));
	nTime->setFlags( eNumber::flagVCenter|eNumber::flagTime|eNumber::flagFillWithZeros );
	nTime->loadDeco();
	nTime->setHelpText(_("enter correct time here"));
	nTime->setNumber(1, tmp.tm_min );
	if (clktype)
	{
		nTime->setNumber(0, tmp.tm_hour%12 ? tmp.tm_hour%12 : 12 );
		nTimeAmpm = new eCheckbox(this);
		nTimeAmpm->resize(eSize(90,35));
		nTimeAmpm->move(ePoint(290,90));
		nTimeAmpm->setText(_("AM/pm"));
		nTimeAmpm->setHelpText(_("Checked is AM, unchecked is PM"));
		nTimeAmpm->setCheck(tmp.tm_hour < 12 ? 1 : 0);
	}
	else
		nTime->setNumber(0, tmp.tm_hour );
	CONNECT( nTime->selected, eTimeCorrectionEditWindow::fieldSelected );

	l = new eLabel(this);
	l->setText(_("New Date:"));
	l->setFlags( eLabel::flagVCenter );
	l->move( ePoint(10,140) );
	l->resize( eSize(150,35) );

	cday = new eComboBox(this);
	cday->move(ePoint(210,140));
	cday->resize(eSize(60,35));
	cday->loadDeco();
	cday->setHelpText(_("press ok to select another day"));

	cmonth = new eComboBox( this );
	cmonth->move(ePoint(280,140));
	cmonth->resize(eSize(60,35));
	cmonth->loadDeco();
	cmonth->setHelpText(_("press ok to select another month"));
	for ( int i = 0; i < 12; i++ )
		new eListBoxEntryText( *cmonth, eString().sprintf("%02d",i+1), (void*)i );
	CONNECT( cmonth->selchanged, eTimeCorrectionEditWindow::monthChanged );

	cyear = new eComboBox(this);
	cyear->move(ePoint(350,140));
	cyear->resize(eSize(80,35));
	cyear->loadDeco();
	cyear->setHelpText(_("press ok to select another year"));
	for ( int i = -1; i < 4; i++ )
		new eListBoxEntryText( *cyear, eString().sprintf("%d",tmp.tm_year+(1900+i)), (void*)(tmp.tm_year+i) );

	cyear->setCurrent( (void*) tmp.tm_year );
	cmonth->setCurrent( (void*) tmp.tm_mon, true );
	cday->setCurrent( (void*) tmp.tm_mday );
	CONNECT( cyear->selchanged, eTimeCorrectionEditWindow::yearChanged );

	bSet=new eButton(this);
	bSet->setText(_("set"));
	bSet->setShortcut("green");
	bSet->setShortcutPixmap("green");

	bSet->move(ePoint(10, clientrect.height()-100));
	bSet->resize(eSize(220,40));
	bSet->setHelpText(_("set new time and close window"));
	bSet->loadDeco();
	CONNECT(bSet->selected, eTimeCorrectionEditWindow::savePressed);

	sbar = new eStatusBar(this);
	sbar->move( ePoint(0, clientrect.height()-50) );
	sbar->resize( eSize( clientrect.width(), 50) );
	sbar->loadDeco();
	CONNECT( updateTimer.timeout, eTimeCorrectionEditWindow::updateTimeDate );
}

void eTimeCorrectionEditWindow::savePressed()
{
	int clktype = 0;
	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);

	eDVB &dvb = *eDVB::getInstance();
	std::map<tsref,int> &tOffsMap= eTransponderList::getInstance()->TimeOffsetMap;
	time_t now = time(0)+dvb.time_difference;

	tm nowTime = *localtime( &now );
	nowTime.tm_isdst=-1;
	nowTime.tm_hour = nTime->getNumber(0);
	if (clktype)
	{ // 12-hour clock
		if (nowTime.tm_hour == 12)
			nowTime.tm_hour = 0;
		if (!nTimeAmpm->isChecked())
			nowTime.tm_hour += 12;
	}
	nowTime.tm_min = nTime->getNumber(1);
	nowTime.tm_mday = (int)cday->getCurrent()->getKey();
	nowTime.tm_wday = -1;
	nowTime.tm_yday = -1;
	nowTime.tm_mon = (int)cmonth->getCurrent()->getKey();
	nowTime.tm_year = (int)cyear->getCurrent()->getKey();
	time_t newTime = mktime(&nowTime);

	tOffsMap.clear();

	dvb.time_difference=1;
	eDebug("[TIME] set Linux Time");
	timeval tnow;
	gettimeofday(&tnow, 0);
	tnow.tv_sec=newTime;
	settimeofday(&tnow, 0);
	for (ePtrList<eMainloop>::iterator it(eMainloop::existing_loops)
		;it != eMainloop::existing_loops.end(); ++it)
		it->setTimerOffset(newTime-now);

	/*emit*/ dvb.timeUpdated();

// for calc new transponder correction
	eDVBServiceController *sapi = dvb.getServiceAPI();
	if ( sapi )
		sapi->startTDT();

	close(0);
}

void eTimeCorrectionEditWindow::updateTimeDate()
{
	time_t now = time(0)+eDVB::getInstance()->time_difference;
	tm ltime = *localtime( &now );
	lCurTime->setText(getTimeStr(&ltime, gTS_SECS));
	lCurDate->setText(eString().sprintf("%02d.%02d.%04d", ltime.tm_mday, ltime.tm_mon+1, 1900+ltime.tm_year));
	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
	if ( sapi && sapi->tdt && sapi->transponder )
	{
		time_t tpTime = now + sapi->lastTpTimeDifference;
		tm ltime = *localtime( &tpTime );
		lTpTime->setText(getTimeStr(&ltime, gTS_SECS));
		lTpDate->setText(eString().sprintf("%02d.%02d.%04d", ltime.tm_mday, ltime.tm_mon+1, 1900+ltime.tm_year));
	}
	else
	{
		lTpTime->setText(_("no transponder time"));
		lTpDate->setText("");
	}
}

int eTimeCorrectionEditWindow::eventHandler( const eWidgetEvent &event )
{
	switch (event.type)
	{
		case eWidgetEvent::execBegin:
			updateTimer.start(1000);
			setFocus(bSet);
			break;
		case eWidgetEvent::execDone:
		{
			updateTimer.stop();
			break;
		}
		default:
			return eWindow::eventHandler( event );
	}
	return 1;
}

void eTimeCorrectionEditWindow::yearChanged( eListBoxEntryText* )
{
	cmonth->setCurrent( (int) cmonth->getCurrent()->getKey(), true );
	setFocus(cyear);
}

void eTimeCorrectionEditWindow::monthChanged( eListBoxEntryText *e )
{
	if ( e )
	{
		const unsigned char monthdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		int month = (int)e->getKey();
		cday->clear();
		int days = monthdays[month];
		if ( month == 1 && __isleap( (int)cyear->getCurrent()->getKey()) )
			days++;
		for ( int i = 1; i <= days ; i++ )
			new eListBoxEntryText( *cday, eString().sprintf("%02d", i), (void*)i);
		cday->setCurrent( (void*) 1 );
	}
}

