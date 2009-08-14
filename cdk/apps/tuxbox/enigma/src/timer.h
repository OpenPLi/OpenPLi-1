#ifndef __apps__enigma__timer_h
#define __apps__enigma__timer_h

#include <lib/dvb/serviceplaylist.h>
#include <lib/gui/listbox.h>
#include <lib/gui/emessage.h>
#include <sselect.h>
#include <epgwindow.h>

#undef WRITE_LOGFILE

class eTextInputField;
class eNumber;
class eButton;
class eComboBox;
class eCheckbox;

class eTimerManager: public Object
{
	static eTimerManager *instance;
	FILE *logfile;
// eTimerManager actionHandler stuff
	enum
	{
		zap, prepareEvent, startCountdown, setNextEvent,
		startEvent, pauseEvent, restartEvent, stopEvent,
		startRecording, stopRecording, restartRecording,
		pauseRecording, spinUpHarddiscs, oldService,
		updateDuration
	} nextAction;

	eTimer actionTimer;  // to start timer related actions
	void actionHandler(); // the action Handler
///////////////////////////

// for multiple use timer and connection objects..
	eTimer timer;
	Connection conn, conn2;

// the timerlist self...
	ePlaylist *timerlist;
	eServiceReference timerlistref;
	eServiceReference playbackRef;

//	nextStarting event, or the current running Event
	std::list<ePlaylistEntry>::iterator nextStartingEvent;

	bool setdeepstandbywakeup;

// all methods are NOT always connected to the eDVB Signals
	void switchedService( const eServiceReferenceDVB&, int err );
	void leaveService( const eServiceReferenceDVB& );

	long getSecondsToBegin();
	long getSecondsToEnd();

#ifdef WRITE_LOGFILE
	void writeToLogfile( const char *str );
	void writeToLogfile( eString str );
#endif
	void init_eTimerManager();
public:
	enum { erase, update };
	eTimerManager();
	~eTimerManager();
	static eTimerManager *getInstance() { return instance; }
	bool updateRunningEvent( int duration, int after_event );
	bool removeEventFromTimerList( eWidget *w, const ePlaylistEntry& entry, int type=erase );
	bool removeEventFromTimerList( eWidget *w, const eServiceReference *ref, const EITEvent *evt);
	void cleanupEvents();
	void clearEvents();
	bool addEventToTimerList( eWidget *w, const eServiceReference *ref, const EITEvent *evt, int type = ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recDVR|ePlaylistEntry::stateWaiting, const ePlaylistEntry *exclude=0 );
	bool addEventToTimerList( eWidget *w, const ePlaylistEntry& entry, const ePlaylistEntry *exclude=0 );
	int addEventToTimerList(const ePlaylistEntry& entry);
	int deleteEventFromTimerList(const ePlaylistEntry& entry, bool force=false);
	int modifyEventInTimerList(const ePlaylistEntry& old_entry, const ePlaylistEntry& new_entry, bool force=false);
	bool eventAlreadyInList( eWidget *w, EITEvent &e, eServiceReference &ref );
	void abortEvent(int err);
	void loadTimerList();
	void saveTimerList();
	void timeChanged();
	int getTimerCount() { return timerlist->getConstList().size(); }
	ePlaylistEntry* findEvent( eServiceReference *service, EITEvent *evt );
	void disableDeepstandbyWakeup() { setdeepstandbywakeup=false; }
	template <class Z>
	void forEachEntry(Z ob)
	{
		if (timerlist)
			for (std::list<ePlaylistEntry>::iterator i(timerlist->getList().begin()); i != timerlist->getList().end(); ++i)
				ob(&*i);
	}
};

class eListBoxEntryTimer: public eListBoxEntry
{
	friend class eListBox<eListBoxEntryTimer>;
	friend class eTimerListView;
	friend struct _selectEvent;
	static gFont TimeFont, DescrFont;
	static gPixmap *ok, *failed;
	static int timeXSize, dateXSize, dateYSize;
	int TimeYOffs, DescrYOffs;
	eTextPara *paraDate, *paraTime, *paraDescr, *paraService;
	ePlaylistEntry *entry;
	const eString &redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int hilited);
	static int getEntryHeight();
	eString hlp;
public:
	bool operator < ( const eListBoxEntry& ref ) const
	{
		return entry->time_begin < ((eListBoxEntryTimer&)ref).entry->time_begin;
	}
	eListBoxEntryTimer(eListBox<eListBoxEntryTimer> *listbox, ePlaylistEntry *entry);
	~eListBoxEntryTimer();
};

class eTimerListView:public eWindow
{
	eListBox<eListBoxEntryTimer>* events;
	eButton *add, *erase, *cleanup;
	void init_eTimerListView();
public:
	eTimerListView();
	void fillTimerList();
	void entrySelected(eListBoxEntryTimer *entry);
	void addPressed();
	void erasePressed();
	void cleanupPressed();
};

class eTimerEditView: public eWindow
{
	eCheckbox *multiple, *cMo, *cTue, *cWed, *cThu, *cFr, *cSa, *cSu;
	eComboBox *bday, *bmonth, *byear, *eday, *emonth, *eyear, *type, *after_event;
	eComboBox *bAmpm, *eAmpm;
	eTextInputField *event_name;
	eLabel *lBegin, *lEnd;
	eNumber *btime, *etime;
	eButton *bSelectService, *bApply, *bScanEPG;
	tm beginTime, endTime;
	eServiceReference tmpService;
	ePlaylistEntry *curEntry;
	int event_id;
private:
	void scanEPGPressed();
	void multipleChanged( int );
	void setMultipleCheckboxes( int type );
	void createWidgets();
	void fillInData( time_t begTime, int duration, int type, eServiceReference& ref );
	void applyPressed();
	void eventNameSelected();
	void showServiceSelector();
	void comboBoxClosed( eComboBox *combo,  eListBoxEntryText* );
	void updateDateTime( const tm& beginTime, const tm& endTime, int what );
	void updateDay( eComboBox* dayCombo, int year, int month, int day );
	void focusNext(int*)
	{
		eWidget::focusNext(eWidget::focusDirNext);
	}
	bool getData( time_t& beginTime, int& duration );
	int eventHandler( const eWidgetEvent &e );
	void changeTime( int dir );
	void init_eTimerEditView( ePlaylistEntry* e);
public:
	eTimerEditView(ePlaylistEntry* e=0);
	eTimerEditView( const EITEvent &e, int type, eServiceReference ref );
};

#endif
