#ifndef __enigma_lcd_h
#define __enigma_lcd_h

#include <lib/gui/ewidget.h>
#ifndef DISABLE_LCD
#include <lib/gui/eprogress.h>
#include <lib/gui/elabel.h>
#include <lib/gui/multipage.h>
#include <src/rds_text.h>

class eService;
class eZapLCDMain;
class eZapLCDMenu;
class eZapLCDScart;
class eZapLCDStandby;
class eZapLCDShutdown;
class eZapLCDSatfind;
class eServiceReferenceDVB;

class eZapLCD: public eWidget
{
	static eZapLCD* instance;
	void init_eZapLCD();
public:
	static eZapLCD* getInstance() { return instance; }
	eZapLCDMain* lcdMain;
	eZapLCDMenu* lcdMenu;
	eZapLCDScart* lcdScart;
	eZapLCDStandby *lcdStandby;
	eZapLCDShutdown *lcdShutdown;
	eZapLCDSatfind *lcdSatfind;
	eZapLCD();
	~eZapLCD();
};

class eZapLCDMain: public eWidget
{
	eProgress *Volume;
	int cur_start,cur_duration;
private:
	void volumeUpdate(int mute_state, int vol);
	void leaveService(const eServiceReferenceDVB &service);
public:
	eLabel *Clock;
	eProgress *Progress;
	void setServiceName(eString name);
	eLabel *ServiceName;
	void setLcdEpgNow(eString name);
	eLabel *LcdEpgNow;
	void setLcdEpgNext(eString name);
	eLabel *LcdEpgNext;
	eZapLCDMain(eWidget *parent);
	void gotRDSText(eString);
};

class eZapLCDMenu: public eWidget
{
public:
	eLabel *Title;
	eWidget *Element;
	eZapLCDMenu(eWidget *parent);
};

class eZapLCDScart: public eWidget
{
	eProgress* volume;
public:
	eZapLCDScart(eWidget *parent);
	void volumeUpdate(int mute_state, int vol);
};

class eZapLCDStandby: public eWidget
{
public:
	eLabel *Clock;
	eZapLCDStandby(eWidget *parent);
};

class eZapLCDShutdown: public eWidget
{
public:
	eZapLCDShutdown(eWidget *parent);
};

class eZapLCDSatfind: public eWidget
{
	eLabel *snrtext, *agctext;
	eProgress *snr, *agc;
public:
	void update(int snr, int agc);
	eZapLCDSatfind(eWidget *parent);
};

#else  // DISABLE_LCD ... dummy class with the same size..

class eZapLCD: public eWidget
{
	static eZapLCD* instance;
public:
	int *dummy[6];
};

#endif // DISABLE_LCD

#endif // __enigma_lcd_h
