#ifndef __src_upgrade_h
#define __src_upgrade_h

#include <lib/gui/ewindow.h>
#include <lib/base/ebase.h>

class eLabel;
class eNumber;
class eProgress;
class eButton;
class eFrontend;
class eCheckbox;
class eComboBox;
class eTransponder;
class eListBoxEntryText;

class eSatfind: public eWindow
{
	eProgress *p_snr, *p_agc, *p_ber;
	eLabel *lsnr_num, *lsync_num, *lber_num;
	eCheckbox *c_sync, *c_lock;
	eComboBox *sat, *transponder;
	eTimer updateTimer;
	eButton *ok;
	eFrontend *fe;
	eTransponder *current;
	int status;
	int eventHandler( const eWidgetEvent& e);
	void satChanged(eListBoxEntryText *sat);
	void tpChanged( eListBoxEntryText *tp );
	void RotorRunning(int);
	void tunedIn(eTransponder *, int );
	void init_eSatfind();
public:
	eSatfind(eFrontend*);
	void closeWnd();
	void update();
};

#endif
