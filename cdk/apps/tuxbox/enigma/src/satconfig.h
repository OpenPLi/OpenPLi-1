#ifndef __satconfig_h
#define __satconfig_h

#include <list>

#include <scan.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/combobox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/multipage.h>
#include <lib/gui/listbox.h>
#include <lib/dvb/dvb.h>

class eFEStatusWidget;
class eTransponderWidget;

struct SatelliteEntry
{
	eComboBox *sat, *voltage, *hilo;
	eLabel *fixed, *description;
	eButton *lnb;
};

class eSatelliteConfigurationManager: public eWindow
{
	eTimer* refresh;
	eWidget *buttonWidget;
	eWidget *w_buttons;
	eButton *button_close, *button_new, *button_erase;
	eLabel *lSatPos, *lLNB, *l22Khz, *lVoltage;
	eProgress *scrollbar;
	eComboBox *combo_type;
	eMultipage satPages;
	std::list<SatelliteEntry*> deleteEntryList;
	int pageEnds[50]; //max 50*40=250 sats
	int curScrollPos;
	ePtrList<eSatellite> sats;

	int complexity;
 	int eventHandler(const eWidgetEvent &event);
	std::map< eSatellite*, SatelliteEntry > entryMap;
	eSatellite *getSat4SatCombo( const eComboBox* );
	eSatellite *getSat4HiLoCombo( const eComboBox* );
	eSatellite *getSat4VoltageCombo( const eComboBox* );
	eSatellite *getSat4LnbButton( const eButton* );
	void createControlElements();
	eComboBox *createSatWidgets(eSatellite* sat);
	eSatellite *createSatellite();
	void updateButtons(int comp);
	void cleanupWidgets();
	void setComplexity(int complexity);
	void repositionWidgets();
	void closePressed();
	void erasePressed();
	void newPressed();
	void updateScrollbar(int show);
	void lnbSelected(eButton *who);
	void delSatellite( eSatellite* sat, bool redraw=true, bool atomic=false );
	void satChanged(eComboBox *who, eListBoxEntryText *le);
	void hiloChanged(eComboBox *who, eListBoxEntryText *le);
	void voltageChanged(eComboBox *who, eListBoxEntryText *le);
	void focusChanged( const eWidget* focus );
	void addSatellitesToCombo( eButton*);
	void deleteSatellite(eSatellite *s);
	
	void typeChanged(eListBoxEntryText* newtype);
	int checkComplexity(); // check overall complexity
	int checkDiseqcComplexity(eSatellite *s);
	void deleteSatellitesAbove(int nr);
	
		// according to "complexity" (works only for complexity <= 2, of course)
	void setSimpleDiseqc(eSatellite *s, int diseqcnr);
	void init_eSatelliteConfigurationManager();
public:
	void extSetComplexity(int complexity);
	bool getRotorEnabled(); // returns true if a rotor is in use (more satellites configured on the same LNB)
	eSatelliteConfigurationManager(bool init=true);
	~eSatelliteConfigurationManager();
};

class eLNBPage;
class eDiSEqCPage;
                  
class eLNBSetup : public eWindow // Selitor = "Sel"ector + Ed"itor" :-)
{
	eMultipage mp;
	eDiSEqCPage *DiSEqCPage;
	eLNBPage *LNBPage;
	eSatellite* sat;
	void onSave();
	void onNext() { mp.next(); }
	void onPrev() { mp.prev(); }  
	int eventHandler(const eWidgetEvent &event);
	eServiceReference service;
	void init_eLNBSetup(eWidget* lcdTitle, eWidget* lcdElement );
public:
	eLNBSetup( eSatellite *sat, eWidget* lcdTitle=0, eWidget* lcdElement=0 );
};

class eLNBPage : public eWidget
{
	friend class eLNBSetup;
	struct selectlnb;
	eSatellite *sat;
	eLabel *lMapping;
	eComboBox *lnb_list;
//	eListBox<eListBoxEntryText> *lnb_list;
	eNumber *lofH, *lofL, *threshold;
	eButton *save; 	 // use this LNB for Satelite and close LNBSelitor
	eButton *next; // shows the DiSEqC Configuration Dialog
	eCheckbox *increased_voltage, *relais_12V_out;
	eStatusBar *statusbar;
    
	void numSelected(int*);
	void lnbChanged( eListBoxEntryText* );
	void init_eLNBPage( eWidget *parent);
public:
	eLNBPage( eWidget *parent, eSatellite *sat );
};

class eDiSEqCPage : public eWidget
{
	friend class eLNBSetup;
	eSatellite *sat;
	eComboBox *DiSEqCMode, *DiSEqCParam, *MiniDiSEqCParam, *DiSEqCRepeats,
						*ucInput;
	eButton *save; 	 // use this LNB for Satelite and close LNBSelitor
	eButton *prev; // shows the LNB Configuration Dialog
//	eButton *next; // shows the Rotor Setup (for non GotoXX Rotors)
	eCheckbox *SeqRepeat, *SwapCmds,
						*FastDiSEqC; // sends no DiSEqC when only hi/lo or H/V Changed
	eLabel *lDiSEqCRepeats, *lDiSEqCParam, *lucInput;
	eStatusBar *statusbar;
         
	void lnbChanged( eListBoxEntryText* );
	void DiSEqCModeChanged( eListBoxEntryText* );
	void numSelected(int*);
	void init_eDiSEqCPage( eWidget *parent, eSatellite *sat );
public:
	eDiSEqCPage( eWidget *parent, eSatellite *sat );
};

#endif
