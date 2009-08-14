#ifndef __epgsearch_h
#define __epgsearch_h 

#include <stdio.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/listbox.h>
#include <lib/gui/combobox.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/serviceplaylist.h>
#include <src/epgwindow.h>
#include <lib/gui/textinput.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/ePLiWindow.h>

struct SEARCH_EPG_DATA
{
	eServiceReference ref;
	eString name;
	int start_time;
	int duration;
	eString title;
	
};

typedef std::vector<SEARCH_EPG_DATA> SearchEPGDATA;

struct EPGSEARCHDATA
{
	int EventStart;
	eString title;
	EITEvent *event;
	eServiceReference ref;
};
typedef std::vector<EPGSEARCHDATA> vecEPGSearch;

struct EPGSEARCHLIST
{
	eServiceReference ref;
	eString name;
	vecEPGSearch EPGSearchData;
};
typedef std::vector<EPGSEARCHLIST> EPGSearchList;

class eEPGSearch: public ePLiWindow
{	
	eTextInputField *InputName;
	eStatusBar *status;
	eCheckbox *chkAllServices, *chkCurrentService, *chkExactMatch, *chkCaseSensitive;
	eComboBox *cboGenre;
	
        eLabel *lb_Caption, *lb_Caption2, *lb_Caption3, *lb_Caption4;
	eString sServiceReference;
	eString sServiceReferenceSearch;
	int canCheck;
	

	int intAllServices;
	int intExactMatch;
	int intCaseSensitive;
	eButton *bt_abort, *bt_seach;
	
	void Search();
	eString Titel;
	void chkAllServicesStateChanged(int state);
	void chkCurrentServiceStateChanged(int state);
	void cboGenreChanged(eListBoxEntryText*);
	int Searching(eString SearchName);
	eString SearchName;

public:
/*	eEPGSearch(eServiceReference ref, EITEvent e);*/
	eEPGSearch(eServiceReference ref, eString CurrentEventName);
	eEPGSearch();
	eString getSearchName();
	int EPGSearching(eString SearchName, eServiceReference SearchRef, int AllServices, int ExactMatch, int CaseSensitive, int genre);
};

class eEPGSearchDATA
{
	static eEPGSearchDATA *instance;
	int numCounter;
	eServiceReference oldService;

public:
	static eEPGSearchDATA *getInstance() { return instance; }

	eEPGSearchDATA();
	SearchEPGDATA getSearchData();
	void clearList();
	eServiceReference getService();
};

#endif /* __epgsearch_h */

