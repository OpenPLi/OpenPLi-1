#ifndef __src_lib_dvb_dvbwidgets_h
#define __src_lib_dvb_dvbwidgets_h

#include <lib/gui/ewidget.h>
#include <lib/gui/listbox.h>
#include <lib/base/ebase.h>

class eNumber;
class eTransponder;
class eCheckbox;
class eProgress;
class eFrontend;
class eComboBox;

class eTransponderWidget: public eWidget
{
	eLabel *lsat;
	eNumber *frequency, *symbolrate;
	eCheckbox *inversion;
	int type, edit;
	eListBoxEntryText *fecEntry[7], *polarityEntry[6], 
	*codeRateLPEntry[6], *codeRateHPEntry[6], *bandwidthEntry[5], *tmModeEntry[4];

	eListBox<eListBoxEntryText> *fec/*guard*/, 
				*polarity/*Modulation/Constellation*/,
				*bandwidth, *tmMode, *codeRateLP, *codeRateHP;
	eComboBox *sat;
	void nextField0(int *);
	void updated1(eListBoxEntryText *);
	void updated2(int);
	void init_eTransponderWidget(eWidget *parent, int edit, int type);
public:
	enum type
	{
		deliveryCable=1, deliverySatellite=2, deliveryTerrestrial=4, flagNoSat=8, flagNoInv=16
	};
	Signal0<void> updated;
	eTransponderWidget(eWidget *parent, int edit, int type);
	int load();
	int setTransponder(const eTransponder *transponder);
	int getTransponder(eTransponder *transponder);
};

class eFEStatusWidget: public eWidget
{
	eProgress *p_snr, *p_agc, *p_ber;
	eCheckbox *c_sync, *c_lock;
	eLabel *lsnr_num, *lsync_num, *lber_num;
	eFrontend *fe;
	eTimer updatetimer;
	void update();
	int eventHandler(const eWidgetEvent &);
	void init_eFEStatusWidget();
public:
	eFEStatusWidget(eWidget *parent, eFrontend *fe);
};

#endif
