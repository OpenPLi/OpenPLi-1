#include <algorithm>
#include <list>

#include <enigma.h>
#include <enigma_epg.h>
#include <enigma_main.h>
#include <enigma_plugins.h>
#include <enigma_event.h>
#include <sselect.h>

#include <lib/picviewer/pictureviewer.h>
#include <lib/base/i18n.h>
#include <lib/driver/rc.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/service.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/frontend.h>
#include <lib/dvb/serviceplaylist.h>
#include <lib/dvb/record.h>
#include <lib/dvb/servicemp3.h>
#include <lib/gdi/font.h>
#include <lib/gui/actions.h>
#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>
#include <lib/gui/numberactions.h>
#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <enigma_streamer.h>
#include <enigma_picmanager.h>
#include <epgwindow.h>
#include <parentallock.h>
#include "epgsearch.h" // EPG search

gFont eListBoxEntryService::serviceFont;
gFont eListBoxEntryService::descrFont;
gFont eListBoxEntryService::numberFont;
gPixmap *eListBoxEntryService::folder=0;
gPixmap *eListBoxEntryService::marker=0;
gPixmap *eListBoxEntryService::locked=0;
gPixmap *eListBoxEntryService::newfound=0;
int eListBoxEntryService::maxNumSize=0;
std::set<eServiceReference> eListBoxEntryService::hilitedEntrys;
eListBoxEntryService *eListBoxEntryService::selectedToMove=0;
int eListBoxEntryService::nownextEPG = 0;
eStreamer streamer;
bool streaming = false;
eServiceReference streamingRef;

struct EPGStyleSelectorActions
{
	eActionMap map;
	eAction infoPressed;
	EPGStyleSelectorActions():
		map("EPGStyleSelector", _("EPG Style Selector")),
		infoPressed(map, "infoPressed", _("open the EPG with selected style"), eAction::prioDialog)
	{
	}
};
eAutoInitP0<EPGStyleSelectorActions> i_EPGStyleSelectorActions(eAutoInitNumbers::actions, "EPG Style Selector actions");

eEPGStyleSelector::eEPGStyleSelector(int ssel)
		:eListBoxWindow<eListBoxEntryText>(_("EPG Style"), 5, 350, true)
		,ssel(ssel)
{
	init_eEPGStyleSelector();
}
void eEPGStyleSelector::init_eEPGStyleSelector()
{
	addActionMap( &i_EPGStyleSelectorActions->map );
	valign();
	int last=1;
	if ( ssel )
		eConfig::getInstance()->getKey("/ezap/serviceselector/lastEPGStyle", last);
	else
		eConfig::getInstance()->getKey("/ezap/lastEPGStyle", last);
	eListBoxEntryText *sel[4]; // EPG search 3 -> 4
	sel[0] = new eListBoxEntryText(&list,_("Channel EPG"), (void*)1, 0, _("open EPG for selected Channel") );
	sel[1] = new eListBoxEntryText(&list,_("Multi EPG"), (void*)2, 0, _("open EPG for a number of channels") );

	// only show external EPG Entry when it realy exist..
	eZapPlugins plugins(eZapPlugins::StandardPlugin);
	if ( plugins.execPluginByName("extepg.cfg",true) == "OK"
		|| plugins.execPluginByName("_extepg.cfg",true) == "OK" )
		sel[2] = new eListBoxEntryText(&list,_("External EPG"), (void*)3, 0, _("open external plugin EPG") );
	sel[3] = new eListBoxEntryText(&list,_("EPG Search"), (void*)4, 0, _("Search event in EPG cache") ); // EPG search
	list.setCurrent(sel[last-1]);
	CONNECT( list.selected, eEPGStyleSelector::entrySelected );
}

int eEPGStyleSelector::eventHandler( const eWidgetEvent &event )
{
	switch (event.type)
	{
		case eWidgetEvent::evtAction:
			if ( event.action == &i_EPGStyleSelectorActions->infoPressed )
				entrySelected( list.getCurrent() );
			else
				break;
			return 1;
		default:
			break;
	}
	return eWindow::eventHandler( event );
}

void eEPGStyleSelector::entrySelected( eListBoxEntryText* e )
{
	if (e)
	{
		if ( ssel )
		{
			if ( (int)e->getKey() <= 3)  	// EPG search
				eConfig::getInstance()->setKey("/ezap/serviceselector/lastEPGStyle", (int)e->getKey());
		}
		else
		{
			if ( (int)e->getKey() <= 3)	// EPG search
				eConfig::getInstance()->setKey("/ezap/lastEPGStyle", (int)e->getKey());
		}
		close( (int)e->getKey() );
	}
	else
		close(-1);
}

struct serviceSelectorActions
{
	eActionMap map;
	eAction nextBouquet, prevBouquet, pathUp, showInfo, showEPGSelector, showMenu,
			addService, addServiceToUserBouquet, modeTV, modeRadio,
			modeFile, toggleStyle, toggleFocus, gotoPrevMarker, gotoNextMarker,
			showAll, showSatellites, showProvider, showBouquets, deletePressed,
			markPressed, renamePressed, insertPressed;
	serviceSelectorActions():
		map("serviceSelector", _("service selector")),
		nextBouquet(map, "nextBouquet", _("switch to next bouquet"), eAction::prioDialogHi),
		prevBouquet(map, "prevBouquet", _("switch to previous bouquet"), eAction::prioDialogHi),
		pathUp(map, "pathUp", _("go up a directory"), eAction::prioDialog),
		showInfo(map, "showInfo", _("shows information about the highlighted channel"), eAction::prioDialog),
		showEPGSelector(map, "showEPGSelector", _("shows the EPG selector for the highlighted channel"), eAction::prioDialog),
		showMenu(map, "showMenu", _("show service selector menu"), eAction::prioDialog),
		addService(map, "addService", _("add service to playlist"), eAction::prioDialog),
		addServiceToUserBouquet(map, "addServiceToUserBouquet", _("add service to a specific bouquet"), eAction::prioDialog),
		modeTV(map, "modeTV", _("switch to TV mode"), eAction::prioDialog),
		modeRadio(map, "modeRadio", _("switch to Radio mode"), eAction::prioDialog),
		modeFile(map, "modeFile", _("switch to File mode"), eAction::prioDialog),
		toggleStyle(map, "toggleStyle", _("toggle between classic and multi column style"), eAction::prioDialog),
		toggleFocus(map, "toggleFocus", _("toggle focus between service and bouquet list (in combi column style)"), eAction::prioDialog),
		gotoPrevMarker(map, "gotoPrevMarker", _("go to the prev Marker if exist.. else go to first service"), eAction::prioDialogHi),
		gotoNextMarker(map, "gotoNextMarker", _("go to the next Marker if exist.. else go to last service"), eAction::prioDialogHi),

		showAll(map, "showAll", _("show all services"), eAction::prioDialog),
		showSatellites(map, "showSatellites", _("show satellite list"), eAction::prioDialog),
		showProvider(map, "showProvider", _("show provider list"), eAction::prioDialog),
		showBouquets(map, "showBouquets", _("show bouquet list"), eAction::prioDialog),

		deletePressed(map, "delete", _("delete selected entry"), eAction::prioDialog),
		markPressed(map, "mark", _("mark selected entry for move"), eAction::prioDialog),
		renamePressed(map, "rename", _("rename selected entry"), eAction::prioDialog),
		insertPressed(map, "marker", _("create new marker entry"), eAction::prioDialog)
	{
	}
};
eAutoInitP0<serviceSelectorActions> i_serviceSelectorActions(eAutoInitNumbers::actions, "service selector actions");

eListBoxEntryService::eListBoxEntryService(eListBoxExt<eListBoxEntryService> *lb, const eServiceReference &service, int flags, int num)
	:eListBoxEntry((eListBox<eListBoxEntry>*)lb), numPara(0),
	namePara(0), descrPara(0), nameXOffs(0), flags(flags),
	num(num), curEventId(-1), service(service)
{
	init_eListBoxEntryService();
}
void eListBoxEntryService::init_eListBoxEntryService()
{
	static char strfilter[4] = { 0xC2, 0x87, 0x86, 0x00 };
	if (!(flags & flagIsReturn))
	{
#if 0
		sort=eString().sprintf("%06d", service->service_number);
#else
		if( service.descr )
			sort = service.descr.c_str();
		else
		{
			const eService *pservice=eServiceInterface::getInstance()->addRef(service);
			if ( pservice )
			{
				sort=pservice?pservice->service_name.c_str():"";
				eServiceInterface::getInstance()->removeRef(service);
			}
		}
		sort.upper();

		// filter short name brakets...
		for (eString::iterator it(sort.begin()); it != sort.end();)
			strchr( strfilter, *it ) ? it = sort.erase(it) : it++;

#endif
	} else
		sort="";
}

eListBoxEntryService::~eListBoxEntryService()
{
	invalidate();
}

int eListBoxEntryService::getEntryHeight()
{
	if (!descrFont.pointSize)
		descrFont = eSkin::getActive()->queryFont("eServiceSelector.Entry.Description");

	return calcFontHeight(serviceFont)+4;
}

void eListBoxEntryService::invalidate()
{
	if (numPara)
	{
		numPara->destroy();
		numPara=0;
	}
	if (descrPara)
	{
		descrPara->destroy();
		descrPara=0;
	}
	if (namePara)
	{
		namePara->destroy();
		namePara=0;
	}
}

void eListBoxEntryService::invalidateDescr()
{
	if (descrPara)
	{
		descrPara->destroy();
		descrPara=0;
	}
}
struct eListBoxEntryService_countServices
{
	Signal1<void,const eServiceReference&> &callback;
	int type;
	int DVBNamespace;
	int* count;
	bool onlyNew;
	eListBoxEntryService_countServices(int type, int DVBNamespace, int* count,bool onlyNew=false)
	: callback(callback), type(type), DVBNamespace(DVBNamespace), count(count),onlyNew(onlyNew)
	{
	}
	void operator()(const eServiceReference &service)
	{
		eService *s = eTransponderList::getInstance()->searchService( service );
		if ( !s )  // dont show "removed services"
			return;
		else if ( s->dvb && s->dvb->dxflags & eServiceDVB::dxDontshow )
			return;
		if ( onlyNew && !(s->dvb && s->dvb->dxflags & eServiceDVB::dxNewFound ) )
			return;
		int t = ((eServiceReferenceDVB&)service).getServiceType();
		int nspace = ((eServiceReferenceDVB&)service).getDVBNamespace().get()&0xFFFF0000;
		if (t < 0)
			t=0;
		if (t >= 31)
			t=31;
		if ( type & (1<<t) && // right dvb service type
				 ( (DVBNamespace==(int)0xFFFFFFFF) || // ignore namespace
				 ( (DVBNamespace&(int)0xFFFF0000) == nspace ) // right satellite
				 )
			 )
			 (*count)++;
	}
};

// service selector
const eString &eListBoxEntryService::redraw(gPainter *rc, const eRect &rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int hilited)
{
	bool b;

	if ( (b = (hilited == 2)) )
		hilited = 0;

	if (this == selectedToMove)
		drawEntryRect(rc, rect, eSkin::getActive()->queryColor("eServiceSelector.entrySelectedToMove"), coActiveF, coNormalB, coNormalF, hilited ); 
	else  // draw with hilighted colours
		drawEntryRect(rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, hilited );

	const eService *pservice=eServiceInterface::getInstance()->addRef(service);

	std::set<eServiceReference>::iterator it = hilitedEntrys.find( service );
	if ( it != hilitedEntrys.end() )
		rc->setForegroundColor( eSkin::getActive()->queryColor("eServiceSelector.entryHilited") );

	int newNameXOffs = nameXOffs;
	if ( service.flags & eServiceReference::isDirectory && folder )  // we draw the folder pixmap
	{
		int ypos = (rect.height() - folder->y) / 2;
		rc->blit(*folder, ePoint(rect.left() + nameXOffs, rect.top() + ypos), eRect(), gPixmap::blitAlphaTest);
		newNameXOffs += folder->x + 20;
	}
	else if ( service.flags & eServiceReference::isMarker && marker ) // marker
	{
		int ypos = (rect.height() - marker->y) / 2;
		rc->blit(*marker, ePoint(rect.left() + nameXOffs, rect.top() + ypos ), eRect(), gPixmap::blitAlphaTest);
		newNameXOffs += marker->x + 20;
	}
	else if (flags & flagShowNumber && listbox->getColumns() == 1 ) // one column list
	{
		int n=-1;
		if (flags & flagOwnNumber)
			n=num;
		else if ( pservice && pservice->dvb )
			n=pservice->dvb->service_number;

		if (n != -1)
		{
			if (!numPara)
			{
				numPara = new eTextPara( eRect( 0, 0, maxNumSize + getEntryHeight()/2, rect.height() ) ); // width num of service
				numPara->setFont( numberFont );
				numPara->renderString( eString().setNum(n) );
				numPara->realign(eTextPara::dirRight);
				numYOffs = ((rect.height() - numPara->getBoundBox().height()) / 2 ) - numPara->getBoundBox().top();
				nameXOffs = maxNumSize + getEntryHeight()/2 + numPara->getBoundBox().height();  // for position of next column
				/* update newNameXOffs, because nameXOffs has changed */
				newNameXOffs = nameXOffs;
			}
			rc->renderPara(*numPara, ePoint( rect.left(), rect.top() + numYOffs ) ); // service number
		}
	}

	if ( pservice && pservice->dvb && pservice->dvb->dxflags & eServiceDVB::dxNewFound && newfound )
	{
		int ypos = (rect.height() - locked->y) / 2;
		rc->blit( *newfound, ePoint(rect.left() + nameXOffs, rect.top()+ypos ), eRect(), gPixmap::blitAlphaTest);
		newNameXOffs += newfound->x+5;
	}

	if (!namePara)
	{
		eString sname;
		if (service.descr.length())
			sname=service.descr;
		else if (pservice)
		{
			int count = 0;
			switch (service.type)
			{
			case eServiceReference::idDVB:
				switch (service.data[0])
				{
				case -2:  // all TV or all Radio Services
					eTransponderList::getInstance()->forEachServiceReference(eListBoxEntryService_countServices(service.data[1], service.data[2], &count));
					sname= eString().sprintf("%s (%d)",pservice->service_name.c_str(),count);
					break;
				case -5:  // all TV or all Radio Services (only services marked as new)
					eTransponderList::getInstance()->forEachServiceReference(eListBoxEntryService_countServices(service.data[1], service.data[2], &count, true ));
					sname= eString().sprintf("%s (%d)",pservice->service_name.c_str(),count);
					break;
				default:
					sname=pservice->service_name;
					break;
				}
				break;
			default:
				sname=pservice->service_name;
				break;
			}
		}
		else if (flags & flagIsReturn)
			sname=_("[GO UP]");
		else
			sname=_("(removed service)");

		namePara = new eTextPara( eRect( 0, 0, rect.width()-newNameXOffs, rect.height() ) );
		namePara->setFont( serviceFont );
		namePara->renderString( sname );
		if (flags & flagIsReturn )
			namePara->realign(eTextPara::dirCenter);
		nameYOffs = ((rect.height() - namePara->getBoundBox().height()) / 2 ) - namePara->getBoundBox().top();	
	}
	// we can always render namePara
	rc->renderPara(*namePara, ePoint( rect.left() + newNameXOffs, rect.top() + nameYOffs ) ); // service name

	if ( service.isLocked() && locked && pinCheck::getInstance()->getParentalEnabled())  
	{
		int ypos = (rect.height() - locked->y) / 2;
		rc->blit( *locked, ePoint(newNameXOffs+namePara->getBoundBox().width()+10, rect.top()+ypos ), eRect(), gPixmap::blitAlphaTest);
	}

	if ( listbox->getColumns() == 1 )
	{
		int perc = -1;
		if (//  !descrPara &&  
					service.type == eServiceReference::idDVB &&
					(!(service.flags & eServiceReference::isDirectory)) &&
					(!service.path.size()) )  // recorded dvb streams
		{
			if (pservice && service.type == eServiceReference::idDVB && !(service.flags & eServiceReference::isDirectory) )
			{
				EITEvent *e=eEPGCache::getInstance()->lookupEvent((const eServiceReferenceDVB&)service);
				if (e)
				{
					if (eListBoxEntryService::nownextEPG)
					{
						time_t t = e->start_time+e->duration+61;
						delete e;
						e = eEPGCache::getInstance()->lookupEvent((const eServiceReferenceDVB&)service,t);
					}
					if (e)
					{
						eString sdescr = "";
						LocalEventData led;
						led.getLocalData(e, &sdescr);
						if (sdescr.length())
						{
							if (eListBoxEntryService::nownextEPG)
							{
								tm *t=localtime(&e->start_time);
								eString nexttime;
								nexttime.sprintf("%02d:%02d", t->tm_hour, t->tm_min);
								sdescr='('+nexttime+' '+sdescr+')';
							}
							else
								if (e->start_time != -1)
								{
									time_t endtime = e->start_time + e->duration;
									time_t now = time(0) + eDVB::getInstance()->time_difference;
#if no
									tm *begin = localtime(&e->start_time);
									sdescr += " - " + getTimeStr(begin, 0) +
										eString().sprintf(" - %d min", e->duration/60);
#endif
									if ((e->start_time <= now) && (now < endtime))
									{
										time_t left = endtime - now;
										perc = left * 100 / e->duration;
#if no
										sdescr += eString().sprintf(" - +%d min (%d%%)", left/60, perc);
#endif
									}
								}
								curEventId = e->event_id;

								descrXOffs = newNameXOffs+namePara->getBoundBox().width();
								if ( service.isLocked() && locked && pinCheck::getInstance()->getParentalEnabled() )
									descrXOffs += locked->x;
								if (numPara)
									descrXOffs += numPara->getBoundBox().height();
								if (descrPara)
									descrPara->destroy();
								descrPara = new eTextPara( eRect( 0, 0, rect.width()-52-descrXOffs, rect.height() ) );
								descrPara->setFont( descrFont );
								descrPara->renderString( sdescr);
								descrYOffs = ((rect.height() - descrPara->getBoundBox().height()) / 2 ) - descrPara->getBoundBox().top();
						}
						delete e;
					}
				}    
			}
		}
		if (descrPara)  // only render descr Para, when avail...
		{
			if (perc >=0)  // displaying borders in same color as channelnumber and name
			{
#define PB_BorderWidth 2
#define PB_Width 50
#define PB_Height 6
				rc->line(ePoint(rect.right() - 52, rect.top() + 3), ePoint(rect.right() + 2, rect.top() + 3));
				rc->line(ePoint(rect.right() - 52, rect.top() + 4), ePoint(rect.right() + 2, rect.top() + 4));
				rc->line(ePoint(rect.right() - 52, rect.top() + rect.height() - 4), ePoint(rect.right() + 2, rect.top() + rect.height() - 4));
				rc->line(ePoint(rect.right() - 52, rect.top() + rect.height() - 3), ePoint(rect.right() + 2, rect.top() + rect.height() - 3));
				rc->line(ePoint(rect.right() - 52, rect.top() + 5), ePoint(rect.right() - 52, rect.top() + rect.height() - 5));
				rc->line(ePoint(rect.right() - 51, rect.top() + 5), ePoint(rect.right() - 51, rect.top() + rect.height() - 5));
				rc->line(ePoint(rect.right() +  1, rect.top() + 5), ePoint(rect.right() +  1, rect.top() + rect.height() - 5));
				rc->line(ePoint(rect.right() +  2, rect.top() + 5), ePoint(rect.right() +  2, rect.top() + rect.height() - 5));
			}
			if ( hilited )
				rc->setForegroundColor( eSkin::getActive()->queryColor("eServiceSelector.highlight.epg.foreground") ); // selected  
			else
				rc->setForegroundColor( eSkin::getActive()->queryColor("epg.time.background") ); // not selected
			rc->renderPara(*descrPara, ePoint( rect.left()+descrXOffs, rect.top() + descrYOffs ) ); // service EPG in serviceselector
			if (perc >=0)  // displaying bars in select service
			{
				for (int i = 5; i < rect.height() - 5; i++)
					rc->line(ePoint(rect.right() - 50, rect.top() + i),
					 	ePoint(rect.right() - 50 + ((100 - perc)>>1), rect.top() + i));
				rc->setForegroundColor( eSkin::getActive()->queryColor("eStatusBar.background") );
				for (int i = 5; i < rect.height() - 5; i++)
					rc->line(ePoint(rect.right() - (perc>>1), rect.top() + i),
					 	ePoint(rect.right(), rect.top() + i));
			}
		}
	}

	if (b)
		drawEntryBorder(rc, rect, coActiveB, coActiveF, coNormalB, coNormalF );

	if ( pservice )
		eServiceInterface::getInstance()->removeRef(service);

	return sort;
}

void eServiceSelector::setKeyDescriptions( bool editMode )
{
	if (!(key[0] && key[1] && key[2] && key[3]))
		return;

	if ( editMode )
	{
		key[0]->setText(_("delete"));
		key[1]->setText(_("mark"));
		key[2]->setText(_("rename"));
		key[3]->setText(_("marker"));
		return;
	}

	switch (eZapMain::getInstance()->getMode())
	{
		case eZapMain::modeTV:
		case eZapMain::modeRadio:
			key[0]->setText(_("All Services"));
			if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )
				key[1]->setText(_("Satellites"));
			else
				key[1]->setText("");
			key[2]->setText(_("Providers"));
			key[3]->setText(_("Bouquets"));
			break;
#ifndef DISABLE_FILE
		case eZapMain::modeFile:
			key[0]->setText(_("My Dreambox"));
			key[1]->setText(_("Movies"));
			key[2]->setText(_("Playlist"));
			key[3]->setText(_("Bouquets"));
			break;
#endif
	}
}

extern bool onSameTP(const eServiceReferenceDVB& ref1, const eServiceReferenceDVB &ref2); // implemented in timer.cpp

void eServiceSelector::addService(const eServiceReference &ref)
{
#ifndef DISABLE_FILE
	if ( eZap::getInstance()->getServiceSelector() == this &&
		eZapMain::getInstance()->getMode() != eZapMain::modeFile )
	{
		eServiceReferenceDVB temp;
		eServiceReferenceDVB &Ref = (eServiceReferenceDVB&) ref;
		eServiceReferenceDVB &rec =
			eDVB::getInstance()->recorder && !eZapMain::getInstance()->isRecordingPermanentTimeshift() ? eDVB::getInstance()->recorder->recRef :
			streaming ? (eServiceReferenceDVB&) streamingRef :
			temp;
		if ( rec && !onSameTP(Ref,rec) )
			return;
	}
#endif
	if (serviceentryflags & eListBoxEntryService::flagSameTransponder)
	{
		// only add Services on current transponder
		eServiceReferenceDVB &Ref = (eServiceReferenceDVB&) ref;
		eServiceReferenceDVB &cur = (eServiceReferenceDVB&) eServiceInterface::getInstance()->service;

		if ( !onSameTP(Ref,cur) )
			return;
	}

	if ( ref.isLocked() && (pinCheck::getInstance()->pLockActive() & 2) )
		return;

	int flags=serviceentryflags & ~eListBoxEntryService::flagSameTransponder;

	if ( ref.flags & eServiceReference::isDirectory)
		flags &= ~ eListBoxEntryService::flagShowNumber;
	new eListBoxEntryService(services, ref, flags);
}

void eServiceSelector::addBouquet(const eServiceReference &ref)
{
	if ( ref.isLocked() && (pinCheck::getInstance()->pLockActive() & 2) )
		return;
	if ( ref.flags & eServiceReference::canDescent && ref.flags & eServiceReference::isDirectory )
		new eListBoxEntryService(bouquets, ref, serviceentryflags);
}

struct renumber
{
	int &num;
	bool invalidate;
	ePlaylist *pl;
	std::list<ePlaylistEntry>::const_iterator it;
	renumber(int &num, ePlaylist *pl, bool invalidate=false)
		:num(num), invalidate(invalidate), pl(pl), it(pl->getConstList().begin())
	{
	}
	bool operator()(eListBoxEntryService& s)
	{
		if (!s.service || s.service.flags == eServiceReference::isMarker )
			return 0;
		if ( !(s.service.flags & (eServiceReference::isDirectory)) )
		{
			while ( !(it->service == s.service) )
			{
				if ( !(it->service.flags & eServiceReference::isMarker) )
					++num;
				++it;
			}
			s.num = num;
			s.invalidate();
		}
		return 0;
	}
};

void eServiceSelector::fillServiceList(const eServiceReference &_ref)
{
	eString windowDescr;
	eServicePath p = path;

	eServiceReference ref;
	streaming = eStreamer::getInstance()->getServiceReference(streamingRef);

	// build complete path ... for window titlebar..
	do
	{
		eServiceReference b=p.current();
		const eService *pservice=eServiceInterface::getInstance()->addRef(b);
		if (pservice && pservice->service_name.length() )
			windowDescr=pservice->service_name+" > "+windowDescr;
		ref = p.current();
		p.up();
		eServiceInterface::getInstance()->removeRef(b);
	}
	while ( ref != p.current() );
	if (windowDescr.rfind(">") != eString::npos)
		windowDescr.erase( windowDescr.rfind(">") );
	setText( windowDescr );

	ref=_ref;

	services->beginAtomic();
	services->clearList();

	if ( !movemode )
	{
		if ( path.size() > 1 && style != styleCombiColumn )
		{
			goUpEntry = new eListBoxEntryService(services, eServiceReference(), eListBoxEntryService::flagIsReturn);
		}
		else
			goUpEntry = 0;
	}

	eServiceInterface *iface=eServiceInterface::getInstance();
	ASSERT(iface);

	Signal1<void,const eServiceReference&> signal;
	CONNECT(signal, eServiceSelector::addService);

	serviceentryflags=eListBoxEntryService::flagShowNumber;
	if (ref.data[0] == -6 && eZapMain::getInstance()->getMode() != eZapMain::modeFile)
	{
		serviceentryflags|=eListBoxEntryService::flagSameTransponder;
	}
	if ( eZap::getInstance()->getServiceSelector() == this
		&& (eDVB::getInstance()->recorder || ref.data[0] == -6)  && !eZapMain::getInstance()->isRecordingPermanentTimeshift()
		&& eZapMain::getInstance()->getMode() != eZapMain::modeFile )
	{
		int mask = eZapMain::getInstance()->getMode() == eZapMain::modeTV ? (1<<4)|(1<<1) : ( 1<<2 );
		eServiceReference bla(eServiceReference::idDVB,
				eServiceReference::flagDirectory|eServiceReference::shouldSort,
				-2, mask, 0xFFFFFFFF );
		iface->enterDirectory(bla, signal);
		iface->leaveDirectory(bla);
		services->sort();
	}
	else
	{
		if (ref.type == eServicePlaylistHandler::ID) // playlists have own numbers
			serviceentryflags|=eListBoxEntryService::flagOwnNumber;
		iface->enterDirectory(ref, signal);
		iface->leaveDirectory(ref);	// we have a copy.
	}

	if (ref.flags & eServiceReference::shouldSort)
		services->sort();

	if (serviceentryflags & eListBoxEntryService::flagOwnNumber)
	{
		int num = this == eZap::getInstance()->getServiceSelector() ?
			getFirstBouquetServiceNum(ref,-1) :
			getFirstBouquetServiceNum(ref,path.bottom().data[1] == (1<<2) ?
				eZapMain::modeRadio : path.bottom().data[1] == (1<<1)|(1<<4) ?
				eZapMain::modeTV : eZapMain::modeFile );
		ePlaylist *pl = (ePlaylist*) eServiceInterface::getInstance()->addRef( path.top() );
		if ( pl )
		{
			services->forEachEntry( renumber(num, pl) );
			eServiceInterface::getInstance()->removeRef( path.top() );
		}
	}

/*	// now we calc the x size of the biggest number we have;
	if (num)
	{
		eTextPara  *tmp = new eTextPara( eRect(0, 0, 100, 50) );
		tmp->setFont( eListBoxEntryService::numberFont );
		tmp->renderString( eString().setNum( num ) );
		eListBoxEntryService::maxNumSize = tmp->getBoundBox().width()+10;
		tmp->destroy();
	}
	else */
		eListBoxEntryService::maxNumSize=45;

	services->endAtomic();
}

void eServiceSelector::shuffle()
{
	services->beginAtomic();
	services->moveSelection( eListBoxBase::dirFirst );
	eListBoxEntryService* p(services->getCurrent());
	while ( p && ( p->flags&eListBoxEntryService::flagIsReturn || p->service.flags & eServiceReference::isDirectory))
	{
		p = services->goNext();
	}
	services->shuffle();
	services->invalidateContent();
	services->endAtomic();
}

void eServiceSelector::updateNumbers()
{
	int num = (this == eZap::getInstance()->getServiceSelector()) ?
		getFirstBouquetServiceNum(path.top(),-1) :
		getFirstBouquetServiceNum(path.top(),path.bottom().data[1] == (1<<2) ?
			eZapMain::modeRadio : path.bottom().data[1] == (1<<1)|(1<<4) ?
			eZapMain::modeTV : eZapMain::modeFile );
	services->beginAtomic();
	ePlaylist *pl = (ePlaylist*) eServiceInterface::getInstance()->addRef( path.top() );
	if ( pl )
	{
		services->forEachEntry( renumber(num, pl, true) );
		eServiceInterface::getInstance()->removeRef( path.top() );
	}
	services->invalidateContent();
	services->endAtomic();
}

void eServiceSelector::fillBouquetList( const eServiceReference& _ref)
{
	bouquets->beginAtomic();
	bouquets->clearList();

	eServiceInterface *iface=eServiceInterface::getInstance();
	ASSERT(iface);
	
	Signal1<void,const eServiceReference&> signal;
	CONNECT(signal, eServiceSelector::addBouquet );
	
	eServiceReference ref=_ref;
	
	iface->enterDirectory(ref, signal);
	iface->leaveDirectory(ref);	// we have a copy.

	if (ref.flags & eServiceReference::shouldSort)
		bouquets->sort();

	bouquets->endAtomic();
}

struct moveFirstChar
{
	char c;

	moveFirstChar(char c): c(c)
	{
	}

	bool operator()(const eListBoxEntryService& s)
	{
		if (s.sort[0] == c)
		{
			( (eListBox<eListBoxEntryService>*) s.listbox)->setCurrent(&s);
			return 1;
		}
		return 0;
	}
};

struct moveServiceNum
{
	int num;

	moveServiceNum(int num): num(num)
	{
	}

	bool operator()(const eListBoxEntryService& s)
	{
		if (s.num == num)
		{
			( (eListBox<eListBoxEntryService>*) s.listbox)->setCurrent(&s);
			return 1;
		}
		return 0;
	}
};

bool eServiceSelector::selectService(int num)
{
	return services->forEachEntry( moveServiceNum( num ) ) == eListBoxBase::OK;
}

struct findServiceNum
{
	int& num;
	const eServiceReference& service;

	findServiceNum(const eServiceReference& service, int& num): num(num), service(service)
	{
	}

	bool operator()(const eListBoxEntryService& s)
	{
		if (s.service == service)
		{
			num=s.getNum();
			return 1;
		}
		return 0;
	}
};

int eServiceSelector::getServiceNum( const eServiceReference &ref )
{
	int ret=-1;
	services->forEachEntry( findServiceNum(ref, ret ) );
	return ret;
}

void eServiceSelector::gotoChar(char c)
{
//	eDebug("gotoChar %d", c);
	switch(c)
	{
		case 2:// A,B,C
			if (BrowseChar >= 'A' && BrowseChar < 'C')
				BrowseChar++;
			else
				BrowseChar = 'A';
			break;

		case 3:// D,E,F
			if (BrowseChar >= 'D' && BrowseChar < 'F')
				BrowseChar++;
			else
				BrowseChar = 'D';
			break;

		case 4:// G,H,I
			if (BrowseChar >= 'G' && BrowseChar < 'I')
				BrowseChar++;
			else
				BrowseChar = 'G';
		break;

		case 5:// J,K,L
			if (BrowseChar >= 'J' && BrowseChar < 'L')
				BrowseChar++;
			else
				BrowseChar = 'J';
			break;

		case 6:// M,N,O
			if (BrowseChar >= 'M' && BrowseChar < 'O')
				BrowseChar++;
			else
				BrowseChar = 'M';
			break;

		case 7:// P,Q,R,S
			if (BrowseChar >= 'P' && BrowseChar < 'S')
				BrowseChar++;
			else
				BrowseChar = 'P';
			break;

		case 8:// T,U,V
			if (BrowseChar >= 'T' && BrowseChar < 'V')
				BrowseChar++;
			else
				BrowseChar = 'T';
			break;

		case 9:// W,X,Y,Z
			if (BrowseChar >= 'W' && BrowseChar < 'Z')
				BrowseChar++;
			else
				BrowseChar = 'W';
			break;
	}
	if (BrowseChar != 0)
	{
		BrowseTimer.start(5000);
		services->beginAtomic();
		services->forEachEntry(moveFirstChar(BrowseChar));
		services->endAtomic();
	}
}

struct updateEPGChangedService
{
	int cnt;
	eEPGCache* epg;
	const tmpMap *updatedEntrys;
	bool redrawOnly;
	updateEPGChangedService( const tmpMap *u, bool redrawOnly=false ):
		cnt(0), epg(eEPGCache::getInstance()), updatedEntrys(u), redrawOnly(redrawOnly)
	{
	}

	bool operator()(eListBoxEntryService& l)
	{
		if ( l.service.type == eServiceReference::idDVB && !( l.service.flags & eServiceReference::isDirectory) )
		{
			tmpMap::const_iterator it;
			if (updatedEntrys)
				it = updatedEntrys->find( (const eServiceReferenceDVB&)l.service );
			if ( (updatedEntrys && it != updatedEntrys->end()) )  // entry is updated
			{
				eventDataPtr e = eEPGCache::getInstance()->getEventDataPtr((const eServiceReferenceDVB&)l.service, (time_t)0 );
				if (e)
				{
					if ( e->getEventID() != l.curEventId )
					{
						if ( redrawOnly )
							((eListBox<eListBoxEntryService>*) l.listbox)->invalidateEntry(cnt);
						else
							l.invalidateDescr();
					}
				}
			}
		}
		cnt++;
		return 0;
	}
};

void eServiceSelector::EPGUpdated()
{
	eEPGCache *epgcache = eEPGCache::getInstance();
	epgcache->Lock();
	tmpMap *tmp = epgcache->getUpdatedMap();
	services->forEachEntry( updateEPGChangedService( tmp ) );
	services->forEachVisibleEntry( updateEPGChangedService( tmp, true ) );
	tmp->clear();
	epgcache->Unlock();
}

struct invalidateServiceDescr
{
	invalidateServiceDescr()
	{
	}
  	 
	bool operator()(eListBoxEntryService& l)
	{
		l.invalidateDescr();
		return 0;
	}
};

void eServiceSelector::SwitchNowNext()
{
	eListBoxEntryService::nownextEPG = 1-eListBoxEntryService::nownextEPG;
	services->forEachEntry( invalidateServiceDescr() );
	services->invalidate();
	updateCi();
}

void eServiceSelector::pathUp()
{
	if (!movemode)
	{
		if (path.size() > ( (style == styleCombiColumn) ? focus==bouquets? 2 : 3 : 1) )
		{
			services->beginAtomic();
			eServiceReference last=path.current();
			path.up();
			actualize();
			selectService( last );
			services->endAtomic();
		} else if ( style == styleCombiColumn && bouquets->isVisible() )
			setFocus(bouquets);
	}
}
struct moveServicePath
{
	eString path;

	moveServicePath(eString path): path(path)
	{
	}

	bool operator()(const eListBoxEntryService& s)
	{
		if (s.service.path == path)
		{
			( (eListBox<eListBoxEntryService>*) s.listbox)->setCurrent(&s);
			return 1;
		}
		return 0;
	}
};

void eServiceSelector::serviceSelected(eListBoxEntryService *entry)
{
	if (entry)
	{
		if (entry->flags & eListBoxEntryService::flagIsReturn)
		{
			// dont change path in radio and tv mode when recording is running
			if ( eZap::getInstance()->getServiceSelector() == this
				&& eDVB::getInstance()->recorder  && !eZapMain::getInstance()->isRecordingPermanentTimeshift()
				&& eZapMain::getInstance()->getMode() != eZapMain::modeFile )
				return;
			pathUp();
			return;
		}
		eServiceReference ref=entry->service;

#ifndef DISABLE_FILE
#ifdef HAVE_DREAMBOX_HARDWARE
		if ((ref.type == 0x2000) && (eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000)) // picviewer
#else
		if (ref.type == 0x2000) // picviewer
#endif
		{
			hide();
                	// eDebug("[sselect] Opening Path %s\n", ref.path.c_str());
			ePictureViewer e(ref.path);
#ifndef DISABLE_LCD
			e.setLCD( LCDTitle, LCDElement );
#endif
			e.show();
			e.exec();
			e.hide();
			eString path = e.GetCurrentFile();
			services->forEachEntry( moveServicePath( path ) ); 
			show();
			return;
		}
#endif

		if (plockmode)
		{
			int doit=1;
			if ( ref.flags & eServiceReference::isDirectory )
			{
				hide();
				int ret = eMessageBox::ShowBox(_("Select No or press Abort to lock/unlock the complete directory"), _("Enter Directory"),  eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btNo );
				if ( ret == eMessageBox::btYes )
					doit=0;
				show();
			}
			if ( doit )
			{
				if ( ref.isLocked() )
					ref.unlock();
				else
					ref.lock();
				invalidateCurrent();
			}
			else
				enterDirectory(ref);
			return;
		}

		if (movemode)
		{
			const std::set<eString> &styles =
				eActionMapList::getInstance()->getCurrentStyles();

			if (eListBoxEntryService::selectedToMove)
			{
				eListBoxEntryService *next=services->getNext();
				/*emit*/moveEntry(path.current(), ref, next ? next->service : eServiceReference());
				services->setMoveMode(0);
				eListBoxEntryService::selectedToMove=0;
				services->beginAtomic();
				actualize();
				selectService(ref);
				services->endAtomic();
				if ( styles.find("sselect_edit") != styles.end() )
					eZapMain::getInstance()->toggleMoveMode(this);
				return;
			}
			else if ( ref )
			{
				services->setMoveMode(1);
				eListBoxEntryService::selectedToMove=entry;
				services->invalidateCurrent();
				return;
			}
		}

		if (entry->service.flags & eServiceReference::isMarker )
			return;

		if (ref.flags & eServiceReference::isDirectory)
			enterDirectory(ref);
		else if (editMode) // edit user bouquet mode
		{
			eServiceReference &ref = services->getCurrent()->service;
			std::set<eServiceReference>::iterator it = eListBoxEntryService::hilitedEntrys.find( ref );
			if ( it == eListBoxEntryService::hilitedEntrys.end() )
			{
				/*emit*/ addServiceToUserBouquet(&selected, 1);
				eListBoxEntryService::hilitedEntrys.insert(ref);
			}
			else
			{
				/*emit*/ removeServiceFromUserBouquet( this );
				eListBoxEntryService::hilitedEntrys.erase(ref);
			}
			services->invalidateCurrent();
			return;
		}
		else
		{
			result=&entry->service;
			close(0);
		}
	}
}

void eServiceSelector::bouquetSelected(eListBoxEntryService*)
{
	if ( services->isVisible() )
		setFocus(services);
}

void eServiceSelector::serviceSelChanged(eListBoxEntryService *entry)
{
	if (entry)
	{
		selected = (((eListBoxEntryService*)entry)->service);
		ci->clear();

		if ( (!(selected.flags & eServiceReference::flagDirectory)) &&
				 (( selected.type == eServiceReference::idDVB )
#ifndef DISABLE_FILE
			|| 	( selected.type == eServiceReference::idUser &&
						( (selected.data[0] == eMP3Decoder::codecMPG)
							|| (selected.data[0] == eMP3Decoder::codecMP3)
							|| (selected.data[0] == eMP3Decoder::codecFLAC)
							|| (selected.data[0] == eMP3Decoder::codecOGG) ) )
			|| 	( selected.type == 0x2000) // images, jpg png etc
#endif
							) )
			ciDelay.start(selected.path.size() ? 100 : 500, true );
	}
}

void eServiceSelector::updateCi()
{
	if ( isVisible() )
		ci->update((const eServiceReferenceDVB&)selected);
}

void eServiceSelector::showRecordingInfo(eString path)
{
	ePtrList<EITEvent> events;
	eString filename = path;
	filename.erase(filename.length() - 2, 2);
	filename += "eit";
	int fd = ::open(filename.c_str(), O_RDONLY);
	if (fd >= 0)
	{
		__u8 buf[4096];
		int rd = ::read(fd, buf, 4096);
		::close(fd);
		if (rd > 12 /*EIT_LOOP_SIZE*/)
		{
			EITEvent *evt = new EITEvent((eit_event_struct*)buf, ((eServiceReferenceDVB&)selected).getServiceID().get(), EIT::typeNowNext);
			events.push_back(evt);
		}
	}
	eEventDisplay eventDisplay(selected.descr, (eServiceReferenceDVB&)selected, &events);
	eventDisplay.show();
	eventDisplay.exec();
	eventDisplay.hide();
	events.setAutoDelete(true);
	show();
}

void eServiceSelector::forEachServiceRef( Signal1<void,const eServiceReference&> callback, bool fromBeg )
{
	eListBoxEntryService *safe = services->getCurrent(),
											 *p, *beg;
	if ( fromBeg )
	{
		services->moveSelection( eListBoxBase::dirFirst );
		beg = services->getCurrent();
	}
	else
		beg = safe;
	p = beg;
	do
	{
		if (!p)
			break;
		if ( !(p->flags & eListBoxEntryService::flagIsReturn) )
			callback(p->service);
		p=services->goNext();
	}
	while ( p && p != beg );

	if ( fromBeg )
		services->setCurrent(safe);
}

int eServiceSelector::eventHandler(const eWidgetEvent &event)
{
	int num=0;
	eServicePath enterPath;
	switch (event.type)
	{
		case eWidgetEvent::evtAction:
			if (event.action == &i_numberActions->key1 && !movemode)
			{
				if ( serviceentryflags&eListBoxEntryService::flagOwnNumber && !editMode )
					num=1;
			}
			else if (event.action == &i_numberActions->key2 && !movemode)
			{
				if ( serviceentryflags&eListBoxEntryService::flagOwnNumber && !editMode )
					num=2;
				else
					gotoChar(2);
			}
			else if (event.action == &i_numberActions->key3 && !movemode)
			{
				if ( serviceentryflags&eListBoxEntryService::flagOwnNumber && !editMode )
					num=3;
				else
					gotoChar(3);
			}
			else if (event.action == &i_numberActions->key4 && !movemode)
			{
				if ( serviceentryflags&eListBoxEntryService::flagOwnNumber && !editMode )
					num=4;
				else
					gotoChar(4);
			}
			else if (event.action == &i_numberActions->key5 && !movemode)
			{
				if ( serviceentryflags&eListBoxEntryService::flagOwnNumber && !editMode )
					num=5;
				else
					gotoChar(5);
			}
			else if (event.action == &i_numberActions->key6 && !movemode)
			{
				if ( serviceentryflags&eListBoxEntryService::flagOwnNumber && !editMode )
					num=6;
				else
					gotoChar(6);
			}
			else if (event.action == &i_numberActions->key7 && !movemode)
			{
				if ( serviceentryflags&eListBoxEntryService::flagOwnNumber && !editMode )
					num=7;
				else
					gotoChar(7);
			}
			else if (event.action == &i_numberActions->key8 && !movemode)
			{
				if ( serviceentryflags&eListBoxEntryService::flagOwnNumber && !editMode )
					num=8;
				else
					gotoChar(8);
			}
			else if (event.action == &i_numberActions->key9 && !movemode)
			{
				if ( serviceentryflags&eListBoxEntryService::flagOwnNumber && !editMode )
					num=9;
				else
					gotoChar(9);
			}
			else if (event.action == &i_serviceSelectorActions->prevBouquet && !movemode && path.size() > 1)
			{
				// dont change path in radio and tv mode when recording is running
				if ( eZap::getInstance()->getServiceSelector() == this
					&& eDVB::getInstance()->recorder && !eZapMain::getInstance()->isRecordingPermanentTimeshift()
					&& eZapMain::getInstance()->getMode() != eZapMain::modeFile )
					return 1;
				ci->clear();
				services->beginAtomic();
				if (style == styleCombiColumn)
				{
					if (bouquets->goPrev()->flags & eListBoxEntryService::flagIsReturn)
						bouquets->goPrev();
				} else
				{
					eServiceReference last=path.current();
					path.up();
					fillServiceList(path.current());
					selectService( last );
					eListBoxEntryService* p=0;
					do
						p = services->goPrev();
					while ( p && ( p->flags&eListBoxEntryService::flagIsReturn || 
									( p->service != last && 
										!(p->service.flags & eServiceReference::canDescent) &&
										!(p->service.flags & eServiceReference::isDirectory))));
					if (p)
					{
						path.down( p->service );
						fillServiceList( p->service );
						if (!selectService( eServiceInterface::getInstance()->service ))
							next();
					}
				}
				services->endAtomic();
			}
			else if (event.action == &i_serviceSelectorActions->nextBouquet && !movemode && path.size()>1 )
			{
				// dont change path in radio and tv mode when recording is running
				if ( eZap::getInstance()->getServiceSelector() == this
					&& eDVB::getInstance()->recorder  && !eZapMain::getInstance()->isRecordingPermanentTimeshift()
					&& eZapMain::getInstance()->getMode() != eZapMain::modeFile )
					return 1;
				ci->clear();
				services->beginAtomic();
				if (style == styleCombiColumn)
				{
					if (bouquets->goNext()->flags & eListBoxEntryService::flagIsReturn)
						bouquets->goNext();
				} else
				{
					eServiceReference last=path.current();
					path.up();
					fillServiceList(path.current());
					selectService( last );
					eListBoxEntryService* p=0;
					do
						p = services->goNext();
					while ( p && ( p->flags&eListBoxEntryService::flagIsReturn || 
									( p->service != last && 
										!(p->service.flags & eServiceReference::canDescent) &&
										!(p->service.flags & eServiceReference::isDirectory))));
					if (p)
					{
						path.down( p->service );
						fillServiceList( p->service );
						if (!selectService( eServiceInterface::getInstance()->service ))
							next();
					}
				}
				services->endAtomic();
			}
			else if (event.action == &i_serviceSelectorActions->showInfo
				&& !movemode && !editMode && this == eZap::getInstance()->getServiceSelector() )
			{
				hide();
#ifndef DISABLE_FILE
#ifdef HAVE_DREAMBOX_HARDWARE
				if ((selected.type == 0x2000) && (eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000))  // Picture
#else
				if (selected.type == 0x2000) // Picture
#endif
				{
					ePicViewerSettings f;
#ifndef DISABLE_LCD
					f.setLCD( LCDTitle, LCDElement );
#endif
					f.show();
					f.exec();
					f.hide();

					show();
				}
				else if (selected.path)
				{
					showRecordingInfo(selected.path);
				}
				else
#endif
				{
					ePtrList<EITEvent> events;
					EITEvent *e = 0;
					e = eEPGCache::getInstance()->lookupEvent((eServiceReferenceDVB&)selected);
					if (e)
					{
						events.push_back(e);
					}
					eEventDisplay eventDisplay(selected.descr, (eServiceReferenceDVB&)selected, &events);
					eventDisplay.show();
					eventDisplay.exec();
					eventDisplay.hide();
					events.setAutoDelete(true);
					show();
				}
			}
			else if (event.action == &i_serviceSelectorActions->showEPGSelector && !isFileSelector
				&& !movemode && !editMode && this == eZap::getInstance()->getServiceSelector() )
			{
				hide();
#ifndef DISABLE_FILE
#ifdef HAVE_DREAMBOX_HARDWARE
				if ((selected.type == 0x2000) && (eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000))  // Picture
#else
				if (selected.type == 0x2000) // Picture
#endif
				{
					ePicViewerSettings f;
#ifndef DISABLE_LCD
					f.setLCD( LCDTitle, LCDElement );
#endif
					f.show();
					f.exec();
					f.hide();

					show();
				}
				else if (selected.path)
				{
					showRecordingInfo(selected.path);
				}
				else
#endif
				{
					eEPGStyleSelector e(1);
#ifndef DISABLE_LCD
					e.setLCD( LCDTitle, LCDElement );
#endif
					e.show();
					int ret = e.exec();
					e.hide();
					switch ( ret )
					{
						case 1:
							/*emit*/ showEPGList((eServiceReferenceDVB&)selected);
							show();
							break;
						case 2:
							showMultiEPG();
							break;
						case 4:					// EPG Search 
							EPGSearchEvent(selected);	// EPG Search 
							break;				// EPG Search 
						default:
							show();
							break;
					}
				}
			}
			else if (event.action == &i_serviceSelectorActions->pathUp)
			{
				// dont change path in radio and tv mode when recording is running
				if ( eZap::getInstance()->getServiceSelector() == this
					&& eDVB::getInstance()->recorder  && !eZapMain::getInstance()->isRecordingPermanentTimeshift()
					&& eZapMain::getInstance()->getMode() != eZapMain::modeFile )
					return 1;
				pathUp();
			}
			else if (event.action == &i_serviceSelectorActions->toggleStyle && !movemode && !editMode)
			{
				int newStyle = lastSelectedStyle;
				if (newStyle == styleMultiColumn)
					newStyle = styleCombiColumn;
				else
					newStyle++;
				setStyle(lastSelectedStyle=newStyle);
			}
			else if (event.action == &i_serviceSelectorActions->toggleFocus && !movemode && path.size() > 1)
			{
				if ( style == styleCombiColumn )
					if (focus == services)
						setFocus( bouquets );
					else
						setFocus( services );
				if ( style == styleSingleColumn )
						SwitchNowNext();
			}
			else if (event.action == &i_serviceSelectorActions->showMenu  && !isFileSelector && focus != bouquets && !plockmode )
			{
				hide();
				/*emit*/ showMenu(this);
				show();
			}
			else if (event.action == &i_serviceSelectorActions->addService && !isFileSelector && !movemode && !editMode)
				/*emit*/ addServiceToPlaylist(selected);
			else if (event.action == &i_serviceSelectorActions->addServiceToUserBouquet && !isFileSelector && !movemode && !editMode)
			{
				hide();
				/*emit*/ addServiceToUserBouquet(&selected, 0);
				show();
			}
			else if (event.action == &i_serviceSelectorActions->modeTV  && !isFileSelector && !movemode && !editMode)
			{
				if ( this == eZap::getInstance()->getServiceSelector() )
					/*emit*/ setMode(eZapMain::modeTV);
				else
				{
					setPath(eServiceReference(eServiceReference::idDVB,
						eServiceReference::flagDirectory|eServiceReference::shouldSort,
						-2, (1<<4)|(1<<1), 0xFFFFFFFF ));
				}
			}
			else if (event.action == &i_serviceSelectorActions->modeRadio && !isFileSelector && !movemode && !editMode)
			{
				if ( this == eZap::getInstance()->getServiceSelector() )
					/*emit*/ setMode(eZapMain::modeRadio);
				else
				{
					setPath(eServiceReference(eServiceReference::idDVB,
						eServiceReference::flagDirectory|eServiceReference::shouldSort,
						-2, 1<<2, 0xFFFFFFFF ));
				}
			}
#ifndef DISABLE_FILE
			else if (event.action == &i_serviceSelectorActions->modeFile  && !isFileSelector && !movemode && !editMode)
				/*emit*/ setMode(eZapMain::modeFile);
#endif
			else if (event.action == &i_serviceSelectorActions->gotoPrevMarker)
			{
				ePlaylist *p = 0;
				if ( path.current().type == eServicePlaylistHandler::ID )
					p = (ePlaylist*) eServicePlaylistHandler::getInstance()->addRef(path.current());
				if ( p )
				{
					std::list<ePlaylistEntry>::const_iterator it =
						std::find( p->getConstList().begin(), p->getConstList().end(), selected );
					if ( it != p->getConstList().end() )
					{
						for (--it ; it != p->getConstList().end(); --it )
							if ( it->service.flags & eServiceReference::isMarker )
							{
								selectService( it->service );
								break;
							}
					}
					if ( it == p->getConstList().end() )
						services->moveSelection(services->dirFirst);
				}
				else
					services->moveSelection(services->dirFirst);
			}
			else if (event.action == &i_serviceSelectorActions->gotoNextMarker)
			{
				ePlaylist *p = 0;
				if ( path.current().type == eServicePlaylistHandler::ID )
					p = (ePlaylist*) eServicePlaylistHandler::getInstance()->addRef(path.current());
				if ( p )
				{
					std::list<ePlaylistEntry>::const_iterator it;
					eListBoxEntryService *cur = services->getCurrent();
					if ( cur && cur->flags & eListBoxEntryService::flagIsReturn )
						it = p->getConstList().begin();
					else
						it = std::find( p->getConstList().begin(), p->getConstList().end(), selected );
					if ( it != p->getConstList().end() )
					{
						for ( ++it ; it != p->getConstList().end(); ++it )
							if ( it->service.flags & eServiceReference::isMarker )
							{
								selectService( it->service );
								break;
							}
					}
					if ( it == p->getConstList().end() )
						services->moveSelection(services->dirLast);
				}
				else
					services->moveSelection(services->dirLast);
			}
			else if (event.action == &i_serviceSelectorActions->showAll && !movemode)
			{
				if ( this == eZap::getInstance()->getServiceSelector() )
					enterPath = /*emit*/ getRoot(listAll, -1);
				else
					enterPath = /*emit*/ getRoot(listAll, path.bottom().data[1] == (1<<2) ? eZapMain::modeRadio :
						path.bottom().data[1] == (1<<1)|(1<<4) ? eZapMain::modeTV : eZapMain::modeFile );
				if ( style == styleCombiColumn && eZapMain::getInstance()->getMode() == eZapMain::modeFile )
					enterPath.down(eServiceReference());
			}
			else if (event.action == &i_serviceSelectorActions->showSatellites && !movemode)
			{
				if ( this == eZap::getInstance()->getServiceSelector() )
					enterPath = /*emit*/ getRoot(listSatellites,-1);
				else
					enterPath = /*emit*/ getRoot(listSatellites, path.bottom().data[1] == (1<<2) ? eZapMain::modeRadio :
					path.bottom().data[1] == (1<<1)|(1<<4) ? eZapMain::modeTV : eZapMain::modeFile );

				// On Cable and Terrestrial Boxes dont handle green button.. expect in file mode..
				if ( eSystemInfo::getInstance()->getFEType() != eSystemInfo::feSatellite && enterPath.size() &&
					enterPath.bottom().data[0] == -4 ) // Satellite list..
					enterPath=eServicePath(); // clear enterPath

				if ( style == styleCombiColumn && eZapMain::getInstance()->getMode() != eZapMain::modeFile )
					enterPath.down(eServiceReference());
			}
			else if (event.action == &i_serviceSelectorActions->showProvider && !movemode)
			{
				if ( this == eZap::getInstance()->getServiceSelector() )
					enterPath = /*emit*/ getRoot(listProvider,-1);
				else
					enterPath = /*emit*/ getRoot(listProvider, path.bottom().data[1] == (1<<2) ? eZapMain::modeRadio :
						path.bottom().data[1] == (1<<1)|(1<<4) ? eZapMain::modeTV : eZapMain::modeFile );
				if ( style == styleCombiColumn && eZapMain::getInstance()->getMode() != eZapMain::modeFile )
					enterPath.down(eServiceReference());
			}
			else if (event.action == &i_serviceSelectorActions->showBouquets && !movemode)
			{
				if ( this == eZap::getInstance()->getServiceSelector() )
					enterPath = /*emit*/ getRoot(listBouquets,-1);
				else
					enterPath = /*emit*/ getRoot(listBouquets, path.bottom().data[1] == (1<<2) ? eZapMain::modeRadio :
						path.bottom().data[1] == (1<<1)|(1<<4) ? eZapMain::modeTV : eZapMain::modeFile );
				if ( style == styleCombiColumn )
					enterPath.down(eServiceReference());
			}
			else if (event.action == &i_cursorActions->cancel)
			{
				if (movemode)
					eZapMain::getInstance()->toggleMoveMode(this);
				if (editMode)
					eZapMain::getInstance()->toggleEditMode(this);
				if (eListBoxEntryService::nownextEPG)
					SwitchNowNext();
				break;
			}
			else if ( event.action == &i_serviceSelectorActions->markPressed )
			{
				if ( services->getCurrent() && !(services->getCurrent()->flags & eListBoxEntryService::flagIsReturn) )
				{
					if (path.current().type == eServicePlaylistHandler::ID)
					{
						if ( movemode )
							serviceSelected( services->getCurrent() );
						else
						{
							eZapMain::getInstance()->toggleMoveMode(this);
							serviceSelected( services->getCurrent() );
						}
					}
				}
			}
			else if ( event.action == &i_serviceSelectorActions->deletePressed )
				/*emit*/ deletePressed( this );
			else if ( event.action == &i_serviceSelectorActions->renamePressed )
			{
				hide();
				if ( selected.type == eServicePlaylistHandler::ID )
					/*emit*/ renameBouquet( this );
				else
					/*emit*/ renameService( this );
				show();
			}
			else if ( event.action == &i_serviceSelectorActions->insertPressed )
			{
				if ( path.current().type == eServicePlaylistHandler::ID )
					/*emit*/ newMarkerPressed( this );
				else if ( selected.flags & eServiceReference::flagDirectory )
				{
					if ( selected.type == eServiceReference::idDVB
						&& (selected.data[0] == -2 || selected.data[0] == -3) )
						/*emit*/ copyToBouquetList(this);
				}
				else if ( selected.type == eServiceReference::idDVB
#ifndef DISABLE_FILE
					|| ( selected.type == eServiceReference::idUser
						&& ( (selected.data[0] == eMP3Decoder::codecMPG)
							|| (selected.data[0] == eMP3Decoder::codecMP3)
							|| (selected.data[0] == eMP3Decoder::codecFLAC)
							|| (selected.data[0] == eMP3Decoder::codecOGG) ) )
#endif
					)
				{
					hide();
					/*emit*/ addServiceToUserBouquet(&selected, 0);
					show();
				}
			}
			else
				break;
			if (enterPath.size())
			{
				// dont change path in radio and tv mode when recording is running
				if ( eZap::getInstance()->getServiceSelector() == this
					&& eDVB::getInstance()->recorder  && !eZapMain::getInstance()->isRecordingPermanentTimeshift()
					&& eZapMain::getInstance()->getMode() != eZapMain::modeFile )
					return 1;
				ci->clear();
				if ( path.bottom() == enterPath.bottom() )
					pathUp();
				else
					setPath(enterPath);
			}
			else if (num)
			{
				hide();
				eServiceNumberWidget s(num);
				s.show();
				num = s.exec();
				s.hide();
				if (num != -1)
				{
					if (selectService( num ))
					{
						result=&services->getCurrent()->service;
						close(0);
					}
					else
						show();
				}
				else
					show();
			}
			return 1;
		default:
			break;
	}
	return eWindow::eventHandler(event);
}

struct _selectService
{
	eServiceReference service;

	_selectService(const eServiceReference& e): service(e)
	{
	}

	bool operator()(const eListBoxEntryService& s)
	{
		if (service == s.service)
		{
			((eListBox<eListBoxEntryService>*) s.listbox)->setCurrent(&s);
			return 1;
		}
		return 0;
	}
};

#if 0
struct copyEntry
{
	std::list<eServiceReference> &dest;

	copyEntry(std::list<eServiceReference> &dest)
		:dest(dest)
	{
	}

	bool operator()(const eListBoxEntryService& s)
	{
		dest.push_back(s.service);
		return 0;
	}
};

bool eServiceSelector::selectServiceRecursive( eServiceReference &ref )
{
	services->beginAtomic();
	bool b = selServiceRec( ref );
	services->endAtomic();
	return b;
}

bool eServiceSelector::selServiceRec( eServiceReference &ref )
{
	std::list<eServiceReference> tmp;

	// copy all entrys to temp list
	services->forEachEntry( copyEntry( tmp ) );

	for ( std::list<eServiceReference>::iterator it( tmp.begin() ); it != tmp.end(); it++ )
	{
		if ( it->flags & eServiceReference::isDirectory )
		{
			path.down(*it);
			actualize();
			if ( selServiceRec( ref ) )
				return true;
			else
			{
				path.up();
				actualize();
			}
		}
		else if ( selectService(ref) )
			return true;
	}
	return false;
}
#endif

bool eServiceSelector::selectService(const eServiceReference &ref)
{
	if ( services->forEachEntry( _selectService(ref) ) )
	{
//		services->moveSelection( eListBox<eListBoxEntryService>::dirFirst );
		// ersten service NICHT selecten (warum auch - evtl. ist ja der aktuelle sinnvoller,
		// und bei einem entsprechenden returncode kann ja jeder sehen was er will)
		return false;
	}
	else
		return true;
}

void eServiceSelector::setStyle(int newStyle, bool force)
{
	eServicePath p = path;
	eServiceReference currentService;
	if (style != newStyle || force )
	{
		ci->hide();
		if ( services )
		{
			// save currentSelected Service
			if ( services->getCount() )
				currentService = services->getCurrent()->service;

			services->hide();
			delete services;
		}
		if ( bouquets )
		{
			bouquets->hide();
			delete bouquets;
		}

		if (key[0])
			delete key[0];
		if (key[1])
			delete key[1];
		if (key[2])
			delete key[2];
		if (key[3])
			delete key[3];

		if (newStyle == styleSingleColumn)
		{
			eListBoxEntryService::folder = eSkin::getActive()->queryImage("sselect_folder");
			eListBoxEntryService::marker = eSkin::getActive()->queryImage("sselect_marker");
			eListBoxEntryService::locked = eSkin::getActive()->queryImage("sselect_locked");
			eListBoxEntryService::newfound = eSkin::getActive()->queryImage("sselect_newfound");
			eListBoxEntryService::numberFont = eSkin::getActive()->queryFont("eServiceSelector.singleColumn.Entry.Number");
			eListBoxEntryService::serviceFont = eSkin::getActive()->queryFont("eServiceSelector.singleColumn.Entry.Name");
			i_serviceSelectorActions->toggleFocus.setDescription(_("toggle between now and next epg entries in single column list"));
		}
		else if (newStyle == styleMultiColumn)
		{
			eListBoxEntryService::folder = 0;
			eListBoxEntryService::marker = eSkin::getActive()->queryImage("sselect_marker_small");
			eListBoxEntryService::locked = eSkin::getActive()->queryImage("sselect_locked_small");
			eListBoxEntryService::newfound = eSkin::getActive()->queryImage("sselect_newfound_small");
			eListBoxEntryService::numberFont = eSkin::getActive()->queryFont("eServiceSelector.multiColumn.Entry.Number");
			eListBoxEntryService::serviceFont = eSkin::getActive()->queryFont("eServiceSelector.multiColumn.Entry.Name");
			i_serviceSelectorActions->toggleFocus.setDescription(_("toggle focus between service and bouquet list (in combi column style)"));
		}
		else
		{
			eListBoxEntryService::folder = 0;
			eListBoxEntryService::marker = eSkin::getActive()->queryImage("sselect_marker_small");
			eListBoxEntryService::locked = eSkin::getActive()->queryImage("sselect_locked_small");
			eListBoxEntryService::newfound = eSkin::getActive()->queryImage("sselect_newfound_small");
			eListBoxEntryService::numberFont = eSkin::getActive()->queryFont("eServiceSelector.combiColumn.Entry.Number");
			eListBoxEntryService::serviceFont = eSkin::getActive()->queryFont("eServiceSelector.combiColumn.Entry.Name");
			i_serviceSelectorActions->toggleFocus.setDescription(_("toggle focus between service and bouquet list (in combi column style)"));
		}
		services = new eListBoxExt<eListBoxEntryService>(this);
		services->setName("services");
		services->setActiveColor(eSkin::getActive()->queryScheme("eServiceSelector.highlight.background"), eSkin::getActive()->queryScheme("eServiceSelector.highlight.foreground"));
		services->hide();

		bouquets = new eListBoxExt<eListBoxEntryService>(this);
		bouquets->setName("bouquets");
		bouquets->setActiveColor(eSkin::getActive()->queryScheme("eServiceSelector.highlight.background"), eSkin::getActive()->queryScheme("eServiceSelector.highlight.foreground"));
		bouquets->hide();

		eString styleName="eServiceSelector_";
		if ( newStyle == styleSingleColumn )
			styleName+="singleColumn";
		else if ( newStyle == styleMultiColumn )
			styleName+="multiColumn";
		else
		{
			styleName+="combiColumn";
			CONNECT( bouquets->selchanged, eServiceSelector::bouquetSelChanged );
			CONNECT( bouquets->selected, eServiceSelector::bouquetSelected );
			bouquets->show();
		}
		int showButtons=0;
		eConfig::getInstance()->getKey("/ezap/serviceselector/showButtons", showButtons );
		if ( showButtons )
		{
			styleName+="_buttons";
			key[0] = new eLabel(this);
			key[0]->setName("key_red");
			key[1] = new eLabel(this);
			key[1]->setName("key_green");
			key[2] = new eLabel(this);
			key[2]->setName("key_yellow");
			key[3] = new eLabel(this);
			key[3]->setName("key_blue");
			for (int i=0; i < 4; i++)
				key[i]->show();
		}
		else
			key[0] = key[1] = key[2] = key[3] = 0;

		if (eSkin::getActive()->build(this, styleName.c_str()))
			eFatal("Service selector widget \"%s\" build failed!",styleName.c_str());

		style = newStyle;
		CONNECT(services->selected, eServiceSelector::serviceSelected);
		CONNECT(services->selchanged, eServiceSelector::serviceSelChanged);
		actualize();
		selectService( currentService );  // select the old service
		services->show();
		if ( services->isVisible() )
			setFocus(services);
		setKeyDescriptions();
	}
	ci->show();
}

void eServiceSelector::bouquetSelChanged( eListBoxEntryService *entry)
{
	if ( entry && entry->service )
	{
		ci->clear();
		services->beginAtomic();
		path.up();
		path.down(entry->service);
		fillServiceList( entry->service );
		if (!selectService( eServiceInterface::getInstance()->service ))
			services->moveSelection( eListBox<eListBoxEntryService>::dirFirst );
		services->endAtomic();
	}
}

void eServiceSelector::actualize()
{
	if ( movemode & 2 )
		return;
	if (style == styleCombiColumn)
	{
		bouquets->beginAtomic();
		if ( path.size() > 1 )
		{
			eServiceReference currentBouquet = path.current();
			path.up();
			eServiceReference allBouquets = path.current();
			path.down( currentBouquet );
			fillBouquetList( allBouquets );

			if ( bouquets->forEachEntry( _selectService( currentBouquet ) ) )
				bouquets->moveSelection( eListBox<eListBoxEntryService>::dirFirst );
		}
		else
		{
			bouquets->clearList();
			fillServiceList(path.current());
		}
		bouquets->endAtomic();
	}
	else
		fillServiceList(path.current());
}

eServiceSelector::eServiceSelector()
	:eWindow(0), result(0), services(0), bouquets(0)
	,style(styleInvalid), lastSelectedStyle(styleSingleColumn)
	,BrowseChar(0), BrowseTimer(eApp), ciDelay(eApp), movemode(0)
	,editMode(0), plockmode(0),isFileSelector(false)
{
	init_eServiceSelector();
}

void eServiceSelector::init_eServiceSelector()
{
	ci = new eChannelInfo(this);
	ci->setName("channelinfo");

	CONNECT(eDVB::getInstance()->bouquetListChanged, eServiceSelector::actualize);
	CONNECT(BrowseTimer.timeout, eServiceSelector::ResetBrowseChar);
	CONNECT(ciDelay.timeout, eServiceSelector::updateCi );

	addActionMap(&i_serviceSelectorActions->map);
	addActionMap(&i_numberActions->map);

	/* help text for service selector screen */
	setHelpText(_("Service selector\n\n" \
		"The Service selector allows you to select/view channels by groups (Satellite, Bouquet, " \
		"All, Provider, Radio/TV, File), and can show these in one or more columns."));

	if ( !eZap::getInstance()->getServiceSelector() )
	{
		addActionToHelpList(&i_serviceSelectorActions->deletePressed);
		addActionToHelpList(&i_serviceSelectorActions->markPressed);
		addActionToHelpList(&i_serviceSelectorActions->renamePressed);
		addActionToHelpList(&i_serviceSelectorActions->insertPressed);
	}
	addActionToHelpList(&i_serviceSelectorActions->showAll);
	if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )
		addActionToHelpList(&i_serviceSelectorActions->showSatellites);
	addActionToHelpList(&i_serviceSelectorActions->showProvider);
	addActionToHelpList(&i_serviceSelectorActions->showBouquets);
	addActionToHelpList(&i_serviceSelectorActions->nextBouquet);
	addActionToHelpList(&i_serviceSelectorActions->prevBouquet);
	if ( !eZap::getInstance()->getServiceSelector() )
		addActionToHelpList(&i_serviceSelectorActions->showMenu);
	addActionToHelpList(&i_serviceSelectorActions->toggleStyle);
	addActionToHelpList(&i_serviceSelectorActions->toggleFocus);
	addActionToHelpList(&i_serviceSelectorActions->gotoPrevMarker);
	addActionToHelpList(&i_serviceSelectorActions->gotoNextMarker);
	addActionToHelpList(&i_serviceSelectorActions->pathUp);
	if ( !eZap::getInstance()->getServiceSelector() )
		addActionToHelpList(&i_serviceSelectorActions->showInfo);
	if ( !eZap::getInstance()->getServiceSelector() )
		addActionToHelpList(&i_serviceSelectorActions->showEPGSelector);
	addActionToHelpList(&i_serviceSelectorActions->modeTV);
	addActionToHelpList(&i_serviceSelectorActions->modeRadio);
#ifndef DISABLE_FILE
	if ( !eZap::getInstance()->getServiceSelector() )
		addActionToHelpList(&i_serviceSelectorActions->modeFile);
#endif
	
	key[0] = key[1] = key[2] = key[3] = 0;

	CONNECT(eDVB::getInstance()->serviceListChanged, eServiceSelector::actualize );
	eActionMapList::getInstance()->activateStyle("sselect_default");
}

eServiceSelector::~eServiceSelector()
{
}

void eServiceSelector::enterDirectory(const eServiceReference &ref)
{
	path.down(ref);
	setPath(path);
}

void eServiceSelector::showMultiEPG()
{
	eZapEPG epg;

	epg.show();
	int epgresult = epg.exec();
	epg.hide();
	if ( !epgresult ) // switch to service requested...
	{
		selectService( epg.getCurSelected()->service );
		result=&selected;
		close(0);
	}
	else
		show();
}

void eServiceSelector::EPGSearchEvent(eServiceReference ref)  // EPG search
{
	eString EPGSearchName = "";
	eEPGSearch dd(ref, ci->GetDescription());
	dd.show();
	int back = 2;
	do
	{
		back = dd.exec();
		EPGSearchName = dd.getSearchName();
		if (back == 2)
		{
			dd.hide();
			eMessageBox::ShowBox(EPGSearchName + eString(_(" was not found!")) , _("EPG Search"), eMessageBox::iconInfo|eMessageBox::btOK);
			dd.show();
		}
	}
	while (back == 2);
	dd.hide();
	if (!back)
	{
		// Zeige EPG Ergebnis an
#ifndef DISABLE_LCD
		eZapLCD* pLCD = eZapLCD::getInstance();
		bool bMain = pLCD->lcdMain->isVisible();
		bool bMenu = pLCD->lcdMenu->isVisible();
		pLCD->lcdMain->hide();
		pLCD->lcdMenu->show();
#endif
			
		eEPGSelector eEPGSelectorSearch(EPGSearchName);
#ifndef DISABLE_LCD
		eEPGSelectorSearch.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
#endif
		eEPGSelectorSearch.show(); eEPGSelectorSearch.exec(); eEPGSelectorSearch.hide();
	
#ifndef DISABLE_LCD
		if (!bMenu)
			pLCD->lcdMenu->hide();
		if ( bMain )
			pLCD->lcdMain->show();
#endif
		}
	show();
}

void eServiceSelector::ResetBrowseChar()
{
	BrowseChar=0;
}

const eServiceReference *eServiceSelector::choose(int irc)
{
	ASSERT(this);
	services->beginAtomic();
	result=0;

	switch (irc)
	{
	case dirUp:
		services->moveSelection(eListBox<eListBoxEntryService>::dirUp);
		break;
	case dirDown:
		services->moveSelection(eListBox<eListBoxEntryService>::dirDown);
		break;
	case dirFirst:
		services->moveSelection(eListBox<eListBoxEntryService>::dirFirst);
		break;
	case dirLast:
		services->moveSelection(eListBox<eListBoxEntryService>::dirLast);
		break;
	default:
		break;
	}
	services->endAtomic();

	if ( !services->getCount() )
		ci->clear();

	show();

	if (exec())
		result=0;

	hide();
	return result;
}

const eServiceReference *eServiceSelector::next()
{
	services->beginAtomic();
	selectService(eServiceInterface::getInstance()->service);

	eListBoxEntryService *s=0, *cur=services->getCurrent();
	do
		s=services->goNext();
	while ( s != cur && s &&
			( s->flags & eListBoxEntryService::flagIsReturn ||
				s->service.flags == eServiceReference::isMarker ||
				s->service.flags & eServiceReference::isNotPlayable ) );

	services->endAtomic();
	if (s)
		return &s->service;
	else
		return 0;
}

const eServiceReference *eServiceSelector::prev()
{
	services->beginAtomic();
	selectService(eServiceInterface::getInstance()->service);
	eListBoxEntryService *s=0, *cur=services->getCurrent();
	do
		s=services->goPrev();
	while ( s != cur && s &&
			( s->flags & eListBoxEntryService::flagIsReturn ||
				s->service.flags & eServiceReference::isMarker ||
				s->service.flags & eServiceReference::isNotPlayable ) );

	services->endAtomic();
	if (s)
		return &s->service;
	else
		return 0;
}

void eServiceSelector::setPath(const eServicePath &newpath, const eServiceReference &select)
{
	path=newpath;
	if (services)
	{
		bool ret=false;
		services->beginAtomic();
		actualize();
		if (select)
			ret=selectService(select);
		else
		{
			eServiceReference &sref = eServiceInterface::getInstance()->service;
			if (!(ret=selectService(sref)))
			{
				if ( sref.type == eServiceReference::idDVB && !sref.path )
				{
					eServiceReferenceDVB &dvb_ref = (eServiceReferenceDVB&) sref;
					eString tmp;
					tmp.sprintf("1:15:fffffffe:12:%x:0:0:0:0:0:", dvb_ref.getDVBNamespace().get());
					eServiceReference sat_ref(tmp);
					if (!(ret=selectService(sat_ref)))
					{
						eServiceDVB *service = eTransponderList::getInstance()->searchService(sref);
						if ( service )
						{
							eBouquet *b = eDVB::getInstance()->settings->getBouquet(eString(service->service_provider).upper());
							if (b)
							{
								tmp.sprintf("1:15:fffffffd:12:%x:%x:0:0:0:0:", b->bouquet_id, dvb_ref.getDVBNamespace().get() );
								eServiceReference bref(tmp);
								if (!(ret=selectService(bref)))
								{
									tmp.sprintf("1:15:fffffffd:12:%x:ffffffff:0:0:0:0:", b->bouquet_id, dvb_ref.getDVBNamespace().get() );
									eServiceReference bref(tmp);
									ret=selectService(bref);
								}
							}
						}
					}
				}
			}
		}

		if (!ret)
			services->moveSelection( eListBox<eListBoxEntryService>::dirFirst );

		// we have a problem when selection not changes..
		// the listbox don't emit "selected"... then our current
		// selected entry is not valid.. to prevent this we set
		// it manual...
		eListBoxEntryService *cur = services->getCurrent();
		if ( cur )
			selected = cur->service;
		else
			selected = eServiceReference();

		services->endAtomic();
	}
}

void eServiceSelector::removeCurrent(bool selNext)
{
	eListBoxEntryService *cur = services->getCurrent();
	if ( !cur )
		return;
	services->beginAtomic();
	if ( selNext )
		services->goNext();
	else
		services->goPrev();
	services->remove( cur, true );
	services->endAtomic();
}

void eServiceSelector::invalidateCurrent( eServiceReference ref )
{
	eListBoxEntryService *cur = services->getCurrent();
	if ( !cur )
		return;
	if ( ref )
		cur->service = ref;
	cur->invalidate();
	services->invalidateCurrent();
}

int eServiceSelector::toggleMoveMode()
{
	if ( editMode )  // don't editmode and movemode at the same time..
		eZapMain::getInstance()->toggleEditMode(this);

	services->beginAtomic();
	movemode^=1;
	if (!movemode)
	{
		eListBoxEntryService::selectedToMove=0;
		services->setMoveMode(0);
		if ( goUpEntry )
			services->append( goUpEntry, true, true );
	}
	else if ( goUpEntry )
		services->take(goUpEntry, true);

	services->endAtomic();
	return movemode;
}

int eServiceSelector::toggleEditMode()
{
	if ( movemode )  // don't editmode and movemode at the same time..
		eZapMain::getInstance()->toggleMoveMode(this);
	editMode^=1;
	return editMode;
}
#ifndef DISABLE_FILE
eFileSelector::eFileSelector(eString startPath) : eServiceSelector()
{
	init_eFileSelector(startPath);
}
void eFileSelector::init_eFileSelector(eString startPath)
{
	isFileSelector = true;
	if (startPath.empty() || startPath[startPath.length() -1] != '/')
		startPath+= "/";
	eString tmp = startPath;
	eString tmp2 = "";
	while (!tmp.empty())
	{
		int pos =tmp.find_first_of('/');
		tmp2 = tmp2+tmp.substr(0,pos) + "/";
		eServiceReference startDirRef =eServiceReference(eServiceReference::idFile, eServiceReference::isDirectory|eServiceReference::canDescent|eServiceReference::mustDescent|eServiceReference::shouldSort|eServiceReference::sort1, tmp2);
		enterDirectory(startDirRef);
		tmp = tmp.substr(pos+1);
	}
	getRoot.connect( slot( *this, &eFileSelector::getDirRoot) );
	setStyle(eServiceSelector::styleSingleColumn);
}

void eFileSelector::setKeyDescriptions( bool editMode )
{
	if (!(key[0] && key[1] && key[2] && key[3]))
		return;

	if ( editMode )
	{
		eServiceSelector::setKeyDescriptions(editMode);
		return;
	}
	
	key[0]->setText(_("Root"));
	key[1]->setText(_("Select"));
	key[2]->setText(_("New Directory"));
	key[3]->setText(_("Delete"));
}
eServicePath eFileSelector::getDirRoot(int list, int _mode)
{
	eServicePath b;
	switch (list)
	{
	case listAll:
		b.down(eServiceReference(eServiceReference::idFile, eServiceReference::isDirectory|eServiceReference::canDescent|eServiceReference::mustDescent|eServiceReference::shouldSort|eServiceReference::sort1, "/"));
		break;
	case listSatellites:
		result = &selected;
		close(0);
		break;
	case listProvider:
		{
			TextEditWindow wnd(_("Enter name of new directory:"));
			wnd.setText(_("Create Directory"));
			wnd.show();
			int ret = wnd.exec();
			wnd.hide();
			if ( !ret )
			{
				eString cmd = eString().sprintf("mkdir \"%s/%s\"",getPath().current().path.c_str(),wnd.getEditText().c_str());
				if ( system(cmd.c_str()) )
				{
					eMessageBox::ShowBox(strerror(errno),_("Error creating directory"),eMessageBox::btOK|eMessageBox::iconError);
				}
				else
					actualize();
			}
		}
		break;
	case listBouquets:
		{
			if  (selected.type != eServiceReference::idFile)
				break;
			eString s;
			s.sprintf(_("You are trying to delete '%s'.\nReally do this?"),selected.path.c_str() );
			int r = eMessageBox::ShowBox(s, _("Delete"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btNo);
			if (r == eMessageBox::btYes)
			{
				eString cmd = eString().sprintf("rm -f -r \"%s\"",selected.path.c_str());
				if ( system(cmd.c_str()) )
				{
					eMessageBox::ShowBox(strerror(errno),_("Error") ,eMessageBox::btOK|eMessageBox::iconError);
				}
				else
					actualize();
			}
		}
		break;
	}
	return b;
}
#endif
