#ifndef __epgwindow_h
#define __epgwindow_h

#include <lib/gui/listbox.h>
#include <lib/dvb/epgcache.h>
#include <libsig_comp.h>

class eListBoxEntryEPG:public eListBoxEntry
{
	friend class eListBox<eListBoxEntryEPG>;
	friend class eEPGSelector;
	static gFont TimeFont, DescrFont;
	static gPixmap *inTimer, *inTimerRec;
	static int timeXSize, dateXSize;
	int TimeYOffs, DescrYOffs;
	eTextPara *paraDate, *paraTime, *paraDescr;
	EITEvent event;
	tm start_time;
	eString descr;
	eString hlp;
	const eString &redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int hilited);
	static int getEntryHeight();
	eServiceReference service;
	void build();
public:
	bool operator<(const eListBoxEntry& ref) const
	{
		return event.start_time < ((eListBoxEntryEPG&)ref).event.start_time;
	}
	eListBoxEntryEPG(EITEvent& evt, eListBox<eListBoxEntryEPG> *listbox, eServiceReference &ref);
	eListBoxEntryEPG(const eit_event_struct* evt, eListBox<eListBoxEntryEPG> *listbox, eServiceReference &ref, int type);
	~eListBoxEntryEPG();
};

class eEPGSelector: public eWindow
{
	eListBox<eListBoxEntryEPG> *events;
	eServiceReferenceDVB current;
private:
	void fillEPGList();
	void entrySelected(eListBoxEntryEPG *entry);
	int eventHandler(const eWidgetEvent &event);
	void fillEPGSearchList(); 		// EPG search
	eString EPGSearchName; 			// EPG search
	int SubRefFlag;				// EPG search
	void init_eEPGSelector(eString* pSearchString);
public:
	eString getEPGSearchName(); 		// EPG search
	eEPGSelector(eString SearchString); 	// EPG Search
	eEPGSelector(const eServiceReferenceDVB &service);
	inline ~eEPGSelector(){};
	Signal3<bool, eEPGSelector*, eServiceReference*, EITEvent *> addEventToTimerList;
	Signal3<bool, eEPGSelector*, eServiceReference*, EITEvent *> removeEventFromTimerList;
};

class LocalEventData
{
	static eString country;
	eString ShortEventName;
	eString ShortEventText;
	eString ExtendedEventText;
public:
	static void invalidate() { first_language = second_language = third_language = fourth_language = country = ""; }
	LocalEventData();
	inline ~LocalEventData(){};
	static eString first_language;
	static eString second_language;
	static eString third_language;
	static eString fourth_language;
	/* Search first for primary, then secondary or finally any other language descriptors */
	void getLocalData(EITEvent *, eString * a=0, eString *b=0); // return short event name, short event text, and extended text
	bool language_exists(EITEvent *, eString);
};

#endif
