#ifndef DISABLE_LCD

#include <enigma_lcd.h>

#include <time.h>

#include <lib/dvb/decoder.h>
#include <lib/dvb/edvb.h>
#include <lib/driver/eavswitch.h>
#include <lib/dvb/dvb.h>
#include <lib/gdi/lcd.h>
#include <lib/gdi/font.h>
#include <lib/gui/emessage.h>
#include <lib/gui/eskin.h>
#include <enigma.h>

eZapLCD* eZapLCD::instance;

eZapLCD::eZapLCD()
	:eWidget(eZap::getInstance()->getDesktop(eZap::desktopLCD))
{
	init_eZapLCD();
}
void eZapLCD::init_eZapLCD()
{
	instance = this;
	move(ePoint(0, 0));
	cresize(eSize(140, 64));

	lcdMain = new eZapLCDMain(this);
	eDebug("lcdMain created: %p", lcdMain);
	lcdMenu = new eZapLCDMenu(this);
	lcdScart = new eZapLCDScart(this);
	lcdStandby = new eZapLCDStandby(this);
	lcdShutdown = new eZapLCDShutdown(this);
	lcdSatfind = new eZapLCDSatfind(this);
	lcdMenu->hide();
	lcdScart->hide();
	lcdStandby->hide();
	lcdShutdown->hide();
	lcdSatfind->hide();
}

eZapLCD::~eZapLCD()
{
	delete lcdMain;
	delete lcdMenu;
	delete lcdScart;
	delete lcdStandby;
	delete lcdSatfind;
	lcdShutdown->show();
	delete lcdShutdown;
}

eZapLCDMain::eZapLCDMain(eWidget *parent)
	:eWidget(parent, 0)
{
	Volume = new eProgress(this);
	Volume->setName("volume_bar");
	
	Progress = new eProgress(this);
	Progress->setName("progress_bar");
	
	ServiceName = new eLabel(this);
	ServiceName->setName("service_name");
	
	LcdEpgNow = new eLabel(this);
	LcdEpgNow->setName("lcd_now");
	
	LcdEpgNext = new eLabel(this);
	LcdEpgNext->setName("lcd_next");
	
	Clock = new eLabel(this);
	Clock->setName("clock");
	
	if (eSkin::getActive()->build(this, "enigma_lcd_main"))
		eFatal("skin load of \"enigma_lcd\" failed");

	cur_start = cur_duration = -1;
	
	Volume->show();
	Progress->hide();
	Progress->zOrderRaise();	
	
	CONNECT(eAVSwitch::getInstance()->volumeChanged, eZapLCDMain::volumeUpdate);
	CONNECT(eDVB::getInstance()->leaveService, eZapLCDMain::leaveService);
}

void eZapLCDMain::volumeUpdate(int mute_state, int vol)
{
	vol=mute_state?63:vol;
	Volume->setPerc((63-vol)*100/63);
}

void eZapLCDMain::setServiceName(eString name)
{
	static char strfilter[4] = { 0xC2, 0x87, 0x86, 0x00 };
	// filter short name brakets...
	for (eString::iterator it(name.begin()); it != name.end();)
		strchr( strfilter, *it ) ? it = name.erase(it) : it++;

/*	static char stropen[3] = { 0xc2, 0x86, 0x00 };
	static char strclose[3] = { 0xc2, 0x87, 0x00 };
	unsigned int open=eString::npos-1;
	eString shortname;

  while ( (open = name.find(stropen, open+2)) != eString::npos )
	{
		unsigned int close = name.find(strclose, open);
		if ( close != eString::npos )
			shortname+=name.mid( open+2, close-(open+2) );
	}
	if (shortname)
		ServiceName->setText( shortname );
	else*/
		ServiceName->setText(name);
}

void eZapLCDMain::setLcdEpgNow(eString name)
{
	static char strfilter[4] = { 0xC2, 0x87, 0x86, 0x00 };
	// filter short name brakets...
	for (eString::iterator it(name.begin()); it != name.end();)
		strchr( strfilter, *it ) ? it = name.erase(it) : it++;

/*	static char stropen[3] = { 0xc2, 0x86, 0x00 };
	static char strclose[3] = { 0xc2, 0x87, 0x00 };
	unsigned int open=eString::npos-1;
	eString shortname;

  while ( (open = name.find(stropen, open+2)) != eString::npos )
	{
		unsigned int close = name.find(strclose, open);
		if ( close != eString::npos )
			shortname+=name.mid( open+2, close-(open+2) );
	}
	if (shortname)
		LcdEpgNow->setText( shortname );
	else*/
		LcdEpgNow->setText(name);
}

void eZapLCDMain::setLcdEpgNext(eString name)
{
	static char strfilter[4] = { 0xC2, 0x87, 0x86, 0x00 };
	// filter short name brakets...
	for (eString::iterator it(name.begin()); it != name.end();)
		strchr( strfilter, *it ) ? it = name.erase(it) : it++;

/*	static char stropen[3] = { 0xc2, 0x86, 0x00 };
	static char strclose[3] = { 0xc2, 0x87, 0x00 };
	unsigned int open=eString::npos-1;
	eString shortname;

  while ( (open = name.find(stropen, open+2)) != eString::npos )
	{
		unsigned int close = name.find(strclose, open);
		if ( close != eString::npos )
			shortname+=name.mid( open+2, close-(open+2) );
	}
	if (shortname)
		LcdEpgNext->setText( shortname );
	else*/
		LcdEpgNext->setText(name);
}

void eZapLCDMain::leaveService(const eServiceReferenceDVB &service)
{
	if (Decoder::locked==2)  // timer zap in background
		return;
	Progress->hide();
	ServiceName->setText("");
}

void eZapLCDMain::gotRDSText(eString text)
{
	//If available, display RDS-Data instead of Servicename.
	//Fixme: Disable autoformat, add scolling.
	ServiceName->setText(text);
}

eZapLCDMenu::eZapLCDMenu(eWidget *parent)
	:eWidget(parent, 0)
{	
	if (eSkin::getActive()->build(this, "enigma_lcd_menu"))
		eFatal("skin load of \"enigma_lcd_menu\" failed");

	ASSIGN(Title, eLabel, "title");
	ASSIGN(Element, eLabel, "element");
}

void eZapLCDScart::volumeUpdate(int mute_state, int vol)
{
	vol=mute_state?63:vol;
	volume->setPerc((63-vol)*100/63);
}

eZapLCDScart::eZapLCDScart(eWidget *parent)
	:eWidget(parent, 0)
{	
	volume = new eProgress(this);
	volume->setName("volume_bar");

	CONNECT(eAVSwitch::getInstance()->volumeChanged, eZapLCDScart::volumeUpdate);

	if (eSkin::getActive()->build(this, "enigma_lcd_scart"))
		eFatal("skin load of \"enigma_lcd_scart\" failed");

	volume->zOrderRaise();	
}

eZapLCDStandby::eZapLCDStandby(eWidget *parent)
	:eWidget(parent, 0)
{
	Clock = new eLabel(this);
	Clock->setName("clock");
	Clock->show();

	if (eSkin::getActive()->build(this, "enigma_lcd_standby"))
		eFatal("skin load of \"enigma_lcd_standby\" failed");
}

eZapLCDSatfind::eZapLCDSatfind(eWidget *parent)
	:eWidget(parent, 0)
{
	snrtext = new eLabel(this);
	snrtext->setName("snrtext");
	snr = new eProgress(this);
	snr->setName("snr");

	agctext = new eLabel(this);
	agctext->setName("agctext");
	agc = new eProgress(this);
	agc->setName("agc");

	if (eSkin::getActive()->build(this, "enigma_lcd_satfind"))
		eFatal("skin load of \"enigma_lcd_satfind\" failed");
}

void eZapLCDSatfind::update(int snr, int agc)
{
	eString snrtxt;
	snrtxt.sprintf("%d%%", snr);
	this->snr->setPerc(snr);
	this->snrtext->setText(snrtxt);

	eString agctxt;
	agctxt.sprintf("%d%%", agc);
	this->agc->setPerc(agc);
	this->agctext->setText(agctxt);
}

eZapLCDShutdown::eZapLCDShutdown(eWidget *parent): eWidget(parent, 0)
{	
	if (eSkin::getActive()->build(this, "enigma_lcd_shutdown"))
		eFatal("skin load of \"enigma_lcd_shutdown\" failed");
}

#endif // DISABLE_LCD
