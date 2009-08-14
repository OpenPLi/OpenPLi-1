#include <enigma_event.h>

#include <epgactions.h>
#include <time.h>
#include <timer.h>
#include <lib/base/eerror.h>
#include <lib/driver/rc.h>
#include <lib/gdi/font.h>
#include <lib/gui/elabel.h>
#include <lib/gui/eskin.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/guiactions.h>
#include <lib/system/init_num.h>
#include <epgwindow.h>
#include <enigma_main.h>

struct enigmaEventViewActions
{
	eActionMap map;
	eAction close;
	enigmaEventViewActions():
		map("enigmaEventView", _("enigma event view")),
		close(map, "close", _("closes the Event View"), eAction::prioDialog)
	{
	}
};

eAutoInitP0<enigmaEventViewActions> i_enigmaEventViewActions(eAutoInitNumbers::actions, "enigma event view actions");

void eEventDisplay::nextEvent()
{
	if (*events == --eventlist->end())
		*events = eventlist->begin();
	else
		++(*events);

	if (*events != eventlist->end())
		setEvent(**events);
	else
		setEvent(0);
}

void eEventDisplay::prevEvent()
{
	if (*events == eventlist->begin())
		*events = --eventlist->end();
	else
		--(*events);	

	if (*events != eventlist->end())
		setEvent(**events);
	else
		setEvent(0);
}

int eEventDisplay::eventHandler(const eWidgetEvent &event)
{
	int addtype=-1;
	switch (event.type)
	{
		case eWidgetEvent::evtAction:
			if (event.action == &i_cursorActions->left)
			{
				if (events)
					prevEvent();
				else
					close(1); // this go the prev event and call exec()   (in epgwindow.cpp)
			}
			else if (event.action == &i_cursorActions->right)
			{
				if (events)
					nextEvent();
				else
					close(2);  // this go the next event and call exec()   (in epgwindow.cpp)
			}
			else if (total && event.action == &i_cursorActions->up)
			{
				ePoint curPos = long_description->getPosition();
				if ( curPos.y() < 0 )
				{
					long_description->move( ePoint( curPos.x(), curPos.y() + pageHeight ) );
					updateScrollbar();
				}
			}
			else if (total && event.action == &i_cursorActions->down)
			{
				ePoint curPos = long_description->getPosition();
				if ( (total - pageHeight ) >= abs( curPos.y() - pageHeight ) )
				{
					long_description->move( ePoint( curPos.x(), curPos.y() - pageHeight ) );
					updateScrollbar();
				}
			}
			else if (event.action == &i_enigmaEventViewActions->close)
				close(0);
			else if ( (addtype = i_epgSelectorActions->checkTimerActions( event.action )) != -1 )
				;
			else if ( event.action == &i_epgSelectorActions->removeTimerEvent)
			{
				if ((evt || events) && eTimerManager::getInstance()->removeEventFromTimerList( this, &ref, evt?evt:*events ) )
					timer_icon->hide();
			}
			else
				break;
			/* only add a timer if the event has a title, begintime and endtime */
			if ( ((valid&0x07) == 0x07) && addtype != -1 && (evt || events) && !eTimerManager::getInstance()->eventAlreadyInList(this, evt?*evt:*events, ref) )
			{
				hide();
				eTimerEditView v( evt?*evt:*events, addtype, ref );
				v.show();
				v.exec();
				v.hide();
				checkTimerIcon(evt?evt:*events);
				show();
			}
		return 1;
		default:
			break;
	}
	return eWindow::eventHandler(event);
}

void eEventDisplay::setEPGSearchEvent(eServiceReferenceDVB &Ref, EITEvent *event, eString Service)  // EPG search
{
	ref = Ref;
	service = Service;
	time_t time = event->start_time+event->duration/2;
	
	EITEvent *tmp = event->event_id != -1 ? eEPGCache::getInstance()->lookupEvent( ref, event->event_id ) : 0;
	if ( !tmp )
		tmp = eEPGCache::getInstance()->lookupEvent( ref, time );
	
	evt = tmp;
	valid = 0;
	long_description->hide();
	long_description->move( ePoint(0,0) );
	if (evt)
	{
		eString _title, _long_description;
		eString _eventDate;
		eString _eventTime;
		
		tm *begin=event->start_time!=-1?localtime(&event->start_time):0;
		if (begin)
		{
			valid |= 1;
			_eventTime.sprintf("%02d:%02d", begin->tm_hour, begin->tm_min);
			_eventDate=eString().sprintf("%02d.%02d.%4d", begin->tm_mday, begin->tm_mon+1, begin->tm_year+1900);
		}
		time_t endtime=event->start_time+event->duration;
		tm *end=event->start_time!=-1?localtime(&endtime):0;
		if (end)
		{
			valid |= 2;
			_eventTime+=eString().sprintf(" - %02d:%02d", end->tm_hour, end->tm_min);
		}
		
		LocalEventData led;
		led.getLocalData(evt, &_title, &_long_description);

		if (_title)
			valid |= 4;
		else
			_title = _("no information is available");
		if ( !ref.path )
			channel->setText(service);

		eventTime->setText(_eventTime);
		eventDate->setText(_eventDate);

		setText(_title);

#ifndef DISABLE_LCD		
		if (LCDElement)
		LCDElement->setText(_title);
#endif		

		if (!_long_description)
			long_description->setText(_("no description is available"));
		else
			long_description->setText(_long_description);

		checkTimerIcon(evt);
	}
	else
	{
		setText(service);
		long_description->setText(_("no description is available"));
	}
	updateScrollbar();
	long_description->show();
}

eEventDisplay::eEventDisplay(eString service, eServiceReferenceDVB &ref, const ePtrList<EITEvent>* e, EITEvent* evt )
: eWindow(1), service(service), ref(ref), evt(evt)
{
	init_eEventDisplay(e);
}
void eEventDisplay::init_eEventDisplay(const ePtrList<EITEvent>* e)
{
	eventlist=0;
	events=0;

	scrollbar = new eProgress(this);
	scrollbar->setName("scrollbar");
	scrollbar->setStart(0);
	scrollbar->setPerc(100);

	descr = new eWidget(this);
	descr->setName("epg_description");

	eventTime = new eLabel(this);
	eventTime->setName("time");

	eventDate = new eLabel(this);
	eventDate->setName("date");

	channel = new eLabel(this);
	channel->setName("channel");

	timer_icon = new eLabel(this);
	timer_icon->setName("timer_icon");

	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "eventview"))
		eFatal("skin load of \"eventview\" failed");

	long_description=new eLabel(descr);
	long_description->setFlags(RS_WRAP);

	// try to recalc long description label... ( no broken text lines.. )
	float lineheight=fontRenderClass::getInstance()->getLineHeight( long_description->getFont() );
	int lines = (int)(descr->getSize().height() / lineheight);
	pageHeight = (int)(lines * lineheight);
	descr->resize( eSize( descr->getSize().width(), pageHeight+(int)(lineheight/6)));
	long_description->resize(eSize(descr->getSize().width(), pageHeight*16));

#ifndef DISABLE_FILE
	addActionToHelpList( &i_epgSelectorActions->addDVRTimerEvent );
#endif
#ifndef DISABLE_NETWORK
	addActionToHelpList( &i_epgSelectorActions->addNGRABTimerEvent );
#endif
	addActionToHelpList( &i_epgSelectorActions->addSwitchTimerEvent );
	addActionToHelpList( &i_epgSelectorActions->removeTimerEvent );
	addActionToHelpList( &i_enigmaEventViewActions->close );

	if (e)
		setList(*e);
	else if (evt)
		setEvent(evt);
	addActionMap( &i_enigmaEventViewActions->map );
	addActionMap( &i_epgSelectorActions->map );

	/* help text for event EPG screen */
	setHelpText(_("\tEvent EPG\n\n>>> [INFO]\n. . . . . . . . . .\n\n" \
							"The EPG (Electronic Program Guide) Event shows you information of the current and next event of the channel you're watching right now.\n\n" \
							"Please be aware that not all stations provide EPG data!\n. . . . . . . . . .\n\n" \
							"Usage:\n\n[LEFT]/[RIGHT]\tPrevious/Next Event\n\n[RED]\tRemove timer event\n\n" \
							"[GREEN]\tAdd to DVR timer events\n\n[YELLOW]\tAdd to Ngrab-Timer events\n\n" \
							"[BLUE]\tAdd to Switch timer events\n\n[EXIT]\tClose window"));
}

eEventDisplay::~eEventDisplay()
{
	delete events;
	delete eventlist;
}

void eEventDisplay::updateScrollbar()
{
	total = pageHeight;
	int pages=1;
	while( total < long_description->getExtend().height() )
	{
		total += pageHeight;
		pages++;
	}

	int start=-long_description->getPosition().y()*100/total;
	int vis=pageHeight*100/total;
	scrollbar->setParams(start, vis);
	scrollbar->show();
	if (pages == 1)
		total=0;
}

void eEventDisplay::setEvent(EITEvent *event)
{
	valid = 0;
	// update evt.. when setEvent is called from outside..
	if ( evt )  
		evt = event;

	long_description->hide();
	long_description->move( ePoint(0,0) );
	if (event)
	{
		eString _title, _long_description;
		eString _eventDate;
		eString _eventTime;

		if ( ref.path )
		{
			_eventDate.sprintf(_("Original duration: %d min"), event->duration/60 );
			eSize s = eventDate->getSize();
			eSize s2 = eventTime->getSize();
			s2.setHeight(0);
			s+=s2;
			eventDate->resize(s);
		}
		else
		{
			tm *begin=event->start_time!=-1?localtime(&event->start_time):0;
			if (begin)
			{
				valid |= 1;
				_eventTime = getTimeStr(begin, 0);
				_eventDate=eString().sprintf("%02d.%02d.%4d", begin->tm_mday, begin->tm_mon+1, begin->tm_year+1900);
			}
			time_t endtime=event->start_time+event->duration;
			tm *end=event->start_time!=-1?localtime(&endtime):0;
			if (end)
			{
				valid |= 2;
				_eventTime += " - ";
				_eventTime += getTimeStr(end, 0);
			}
		}

		LocalEventData led;
		led.getLocalData(event, &_title, &_long_description);

#ifndef DISABLE_LCD
		if (LCDElement)
			LCDElement->setText(_long_description);
#endif

		if (_title)
			valid |= 4;
		else
			_title = _("no information is available");
		if ( !ref.path )
			channel->setText(service);

		eventTime->setText(_eventTime);
		eventDate->setText(_eventDate);

		setText(_title);

		if (!_long_description)
			long_description->setText(_("no description is available"));
		else
			long_description->setText(_long_description);

		checkTimerIcon(event);
	} 
	else
	{
		setText(service);
		long_description->setText(_("no description is available"));
	}
	updateScrollbar();
	long_description->show();
}

void eEventDisplay::checkTimerIcon( EITEvent *event )
{
	ePlaylistEntry* p;
	if ( event && (p = eTimerManager::getInstance()->findEvent( &ref, event )) )
	{
		if ( p->type & ePlaylistEntry::SwitchTimerEntry )
		{
			gPixmap *pmap = eSkin::getActive()->queryImage("timer_symbol");
			if (!pmap)
				return;
			timer_icon->setPixmap(pmap);
			timer_icon->show();
		}
		else if ( p->type & ePlaylistEntry::RecTimerEntry )
		{
			gPixmap *pmap = eSkin::getActive()->queryImage("timer_rec_symbol");
			if (!pmap)
				return;
			timer_icon->setPixmap(pmap);
			timer_icon->show();
		}
	}
	else
		timer_icon->hide();
}

void eEventDisplay::setList(const ePtrList<EITEvent> &e)
{
	delete eventlist;
	delete events;
	eventlist=new ePtrList<EITEvent>(e);
	events=new ePtrList<EITEvent>::iterator(*eventlist);
	if (*events != eventlist->end())
		setEvent(**events);
	else
		setEvent(0);
}

