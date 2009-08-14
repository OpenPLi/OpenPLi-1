#ifndef __enigma_event_h
#define __enigma_event_h

#include <lib/gui/ewindow.h>
#include <lib/dvb/si.h>

class eLabel;
class eProgress;
class eServiceReferenceDVB;

class eEventDisplay: public eWindow
{
	eString service;
	eServiceReferenceDVB &ref;
	ePtrList<EITEvent>::iterator *events;
	ePtrList<EITEvent> *eventlist;
	eWidget *descr;
	EITEvent *evt;
	eLabel *long_description, *eventDate, *eventTime, *channel,
					*timer_icon;
	eProgress *scrollbar;
	void nextEvent();
	void prevEvent();
	int total;
	void updateScrollbar();
	void checkTimerIcon(EITEvent *);
	int valid;
	int pageHeight;
	void init_eEventDisplay(const ePtrList<EITEvent>* e);
protected:
	int eventHandler(const eWidgetEvent &event);
public:
	eEventDisplay(eString service, eServiceReferenceDVB &ref, const ePtrList<EITEvent>* e=0, EITEvent* evt=0 );
	~eEventDisplay();
	void setList(const ePtrList<EITEvent> &events);
	void setEvent(EITEvent *event);
	void setEPGSearchEvent(eServiceReferenceDVB &ref, EITEvent *event, eString Service);  // EPG search
};

#endif /* __enigma_event_h */
