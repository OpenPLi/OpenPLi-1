#ifndef __scan_h
#define __scan_h

#include <lib/dvb/dvb.h>
#include <lib/gui/ewidget.h>
#include <lib/gui/combobox.h>
#include <lib/gui/statusbar.h>

class eWindow;
class eLabel;
class eProgress;
class eButton;
class eCheckbox;
class eTransponderWidget;
class eFEStatusWidget;
class eDVBEvent;
class eDVBState;
class eTextInputField;

struct scanEntry
{
	tpPacket *packet;
	bool onlyFree;
	bool operator < ( const scanEntry& e ) const
	{
		if ( packet && e.packet )
			return packet->orbital_position < e.packet->orbital_position;
		else
			return 0;
	}
};

class tsSelectType: public eWidget
{
	eListBox<eListBoxEntryMenu> *list;
	void selected(eListBoxEntryMenu *entry);
	int eventHandler( const eWidgetEvent &e );
	eListBoxEntryCheck *check;
	void init_tsSelectType();
public:
	tsSelectType(eWidget *parent);
};

class tsManual: public eWidget
{
	eTransponder transponder;
	eButton *b_start, *b_manual_pids;
	eTransponderWidget *transponder_widget;
	eFEStatusWidget *festatus_widget;
	eCheckbox *c_onlyFree, *c_searchnit, *c_useonit, *c_usebat;
	void start();
	void abort();
	void retune();
	void manual_pids();
	void init_tsManual( eWidget *LCDTitle, eWidget *LCDElement);
public:
	tsManual(eWidget *parent, const eTransponder &transponder, eWidget* LCDTitle=0, eWidget* LCDElement=0);
	eTransponder &getTransponder() { return transponder; }
};


class tsFastscanGUI: public eWidget
{
	eButton *b_start; 
	eStatusBar *status;
#if HDINE1
	eCheckbox *c_hdlist;
#endif
	eCheckbox *c_usenum, *c_usename;
	eListBox<eListBoxEntryText> *l_provider;
	eListBoxEntryText* entrys[3];
	eLabel *warntv, *warnrad;

#if HDINE1
	unsigned int v_hdlist;
#endif
	unsigned int v_provider, v_usenum, v_usename;

	eStatusBar *statusbar;
private:
	void providerChanged( eListBoxEntryText *);
	void checkProvider();
#if HDINE1
	void hdlistChanged(int);
#endif
	void usenumChanged(int);
	void usenameChanged(int);
	void start();
	void abort();
	// int eventHandler( const eWidgetEvent &e );
	void init_tsFastscanGUI(eWidget *parent, eWidget* LCDTitle=0, eWidget* LCDElement=0);
public:
	tsFastscanGUI(eWidget *parent, eWidget* LCDTitle=0, eWidget* LCDElement=0);
	~tsFastscanGUI();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};


class tsTryLock: public eWidget
{
	eButton *b_abort;
	eLabel *l_status;
	int ret, inProgress;
	tpPacket *packet;
	std::list<eTransponder>::iterator current_tp;
	void dvbEvent(const eDVBEvent &event);
	int nextTransponder(int next);
	int eventHandler(const eWidgetEvent &);
	void init_tsTryLock(eWidget *parent, tpPacket *packet, eString ttext);
public:
	tsTryLock(eWidget *parent, tpPacket *tppacket, eString ttext);
};

class tsAutomatic: public eWidget
{
	eButton *b_start;
	eComboBox *l_network;
	eCheckbox *c_onlyFree, *c_nocircular;
	eLabel *l_status;
	std::list<eTransponder>::iterator current_tp, last_tp, first_tp;
	int automatic;
	void start();
	void networkSelected(eListBoxEntryText *l);
	void dvbEvent(const eDVBEvent &event);
	int eventHandler(const eWidgetEvent &event);
	int loadNetworks();
	int nextNetwork(int first=0);
	int nextTransponder(int next);
	int tuneNext(int next);
	int inProgress;
	void init_tsAutomatic();
public:
	void openNetworkCombo();
	tsAutomatic(eWidget *parent);
};

class tsText: public eWidget
{
	eLabel *headline, *body;
	void init_tsText(eString sheadline, eString sbody);
protected:
	int eventHandler(const eWidgetEvent &event);
public:
	tsText(eString headline, eString body, eWidget *parent);
};

class tsScan: public eWidget
{
	eTimer timer;
	eLabel *status;
	eLabel *timeleft, *service_name, *service_provider, *transponder_data, *services_scanned, *transponder_scanned;
	eProgress *progress;
	int tpLeft, scantime;
	void init_tsScan(eString sattext);
protected:
	int eventHandler(const eWidgetEvent &event);
	void dvbEvent(const eDVBEvent &event);
	void dvbState(const eDVBState &event);
	void updateTime();
	void serviceFound( const eServiceReferenceDVB &, bool );
	void addedTransponder( eTransponder* );
public:
	bool only_new_services;
	int tpScanned, newTVServices, newRadioServices, newDataServices, servicesScanned, newTransponders;
	tsScan(eWidget *parent, eString satText="");
};


class tsFastscan: public eWidget
{

	eTimer timer;
	eLabel *status;
	eLabel *timeleft, *service_name, *service_provider, *transponder_data, *services_scanned, *transponder_scanned;
	eProgress *progress;

	eString providerName, bouquetFilename;
	bool originalNumbering;
	bool useFixedServiceInfo;

	int scantime;
	void init_tsFastscan(eString sattext);
protected:
	int eventHandler(const eWidgetEvent &event);
	void dvbEvent(const eDVBEvent &event);
	void dvbState(const eDVBState &event);
	void serviceFound(int service_type);
	void updateTime();
	void TableProgress(int size, int max);
	void fillBouquet(eBouquet *bouquet, std::map<int, eServiceReferenceDVB> &numbered_channels);
	void parseResult();

public:
	int newTVServices, newRadioServices, newDataServices, servicesScanned;
	tsFastscan(eWidget *parent, eString satText="");
};


class eListBoxEntrySat: public eListBoxEntryText
{
	eTextPara *statePara;
	const eString& redraw(gPainter *, const eRect&, gColor, gColor, gColor, gColor, int );
public:
	enum {
		stateNotScan,
		stateScan,
		stateScanFree
	};
	int state;
	void invalidate();
	tpPacket *getTransponders() { return (tpPacket*) key; }
	eListBoxEntrySat( eListBox<eListBoxEntrySat>*, tpPacket* );
};

class tsMultiSatScan: public eWidget
{
	eButton *start;
	eListBox<eListBoxEntrySat> *satellites;
	void entrySelected( eListBoxEntrySat * );
	void init_tsMultiSatScan();
public:
	tsMultiSatScan(eWidget *parent);
	void getSatsToScan( std::list<scanEntry> &);
};

class TransponderScan: public eWindow
{
	eProgress *progress;
	eLabel *progress_text;
	eStatusBar *statusbar;	
	eWidget *current;
//#ifndef DISABLE_LCD
	eWidget *LCDElement, *LCDTitle;
//#endif
	std::list<scanEntry> toScan;
	eServiceReference service;
	int ret;
	eTimer closeTimer;
	void addService(const eServiceReference &);
	int eventHandler(const eWidgetEvent &);
	void Close();
	unsigned int last_orbital_pos;
	bool remove_new_flags;
	void init_TransponderScan();
public:
	enum tState
	{
		stateMenu,
		stateManual,
		stateAutomatic,
		stateMulti,
		stateBlind,
		stateFastscan,
		stateScan,
		stateMultiScan,
		stateDone,
		stateEnd,
		stateTPEdit	
	};
	TransponderScan( eWidget* LCDTitle=0, eWidget* LCDElement=0, tState initial=stateMenu );
	~TransponderScan();
	int Exec();
private:
	tState stateInitial;
};

class ManualPIDWindow: public eWindow
{
	bool hex;
	eTransponder transponder;
	eServiceReferenceDVB service;
	eTextInputField *name, *provider,
					*vpid, *apid, *pcrpid, *tpid,
					*tsid, *onid, *sid;
	eCheckbox *cNoDVB, *cUseSDT, *cHoldName, *isAC3Pid;
	eButton *bReadNIT, *bSetPIDs, *bStore, *bHexDec;
	PAT *pat;
	NIT *nit;
	void gotPAT(int);
	void gotNIT(int);
	void startNIT();
	void setPIDs();
	void store();
	void hexdec();
	void init_ManualPIDWindow(eTransponder *tp, const eServiceReferenceDVB &ref = eServiceReferenceDVB());
public:
	ManualPIDWindow(eTransponder *tp, const eServiceReferenceDVB &ref = eServiceReferenceDVB() );
	~ManualPIDWindow();
};

#endif
