#ifndef __rotorconfig_h
#define __rotorconfig_h

#include <list>

#include <scan.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/combobox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/multipage.h>
#include <lib/gui/listbox.h>
#include <lib/dvb/dvb.h>

class eFEStatusWidget;

class RotorConfig: public eWindow
{
	eLNB *lnb;
	eListBox<eListBoxEntryText> *positions;
	eLabel *lLatitude, *lLongitude, *lOrbitalPosition, *lStoredRotorNo, *lDirection, *lDeltaA;
	eNumber *orbital_position, *number, *Latitude, *Longitude, *DeltaA;
	eButton *add, *remove, *save, *next;
	eCheckbox *useGotoXX, *useRotorInPower;
	eComboBox *direction, *LaDirection, *LoDirection;
	eStatusBar* statusbar;
	void onAdd();
	void onRemove();
	void onSavePressed();
	void onNextPressed();
	void numSelected(int*);
	void lnbChanged( eListBoxEntryText* );
	void posChanged( eListBoxEntryText* );
	void gotoXXChanged( int );
	void useRotorInPowerChanged( int );
	void setLNBData( eLNB *lnb );
	void init_RotorConfig( eLNB *lnb );
public:
	RotorConfig( eLNB *lnb );
};

class eRotorManual: public eWindow
{
	eLabel *lSat, *lTransponder, *lDirection, *lMode, *lCounter
							, *Counter, *lRecaclParams;
	eButton *Direction;
	eComboBox *Sat, *Transponder, *Mode;
	eButton *Exit, *Save, *Search;
	eNumber *num, *num1, *num2, *num3;
	eFEStatusWidget *status;
	eLNB *lnb;
	eTimer *retuneTimer;
	eTransponder* transponder;
	bool running;
	int eventHandler( const eWidgetEvent& e);
	void retune();
	void onButtonPressed();
	void onScanPressed();
	void satChanged(eListBoxEntryText *sat);
	void tpChanged(eListBoxEntryText *tp);
	void modeChanged( eListBoxEntryText *e);
	void nextfield(int*);
	void init_eRotorManual(eLNB *lnb);
public:
	int changed;
	eRotorManual(eLNB *lnb);
	~eRotorManual();
};

class eStoreWindow: public eWindow
{
	eLabel *lStorageLoc;
	eNumber *StorageLoc;
	eButton *Store, *Cancel;
	eLNB *lnb;
	int orbital_pos;
	void onStorePressed();
	void nextfield(int*);
	void init_eStoreWindow();
public:
	eStoreWindow(eLNB *lnb, int orbital_pos);
};

#endif
