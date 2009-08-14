#ifndef __time_correction_h
#define __time_correction_h

#include <lib/gui/enumber.h>
#include <lib/gui/combobox.h>
#include <lib/gui/echeckbox.h>
#include <lib/dvb/dvb.h>

class eTimeCorrectionEditWindow: public eWindow
{
	eTimer updateTimer;
	eLabel *lCurTime, *lCurDate, *lTpTime, *lTpDate;
	eComboBox *cday, *cmonth, *cyear;
	eButton *bSet, *bReject;
	eNumber *nTime;
	eCheckbox *nTimeAmpm;
	eStatusBar *sbar;
	tsref transponder;
	int eventHandler( const eWidgetEvent &e );
	void savePressed();
	void updateTimeDate();
	void monthChanged( eListBoxEntryText* );
	void yearChanged( eListBoxEntryText* );
	void fieldSelected(int *){focusNext(eWidget::focusDirNext);}
	void init_eTimeCorrectionEditWindow( tsref tp );
public:
	eTimeCorrectionEditWindow( tsref tp );
};

#endif
