#include "enigma_epg.h"
#include "enigma.h"
#include "sselect.h"
#include "enigma_lcd.h"
#include "epgactions.h"
#include "timer.h"
#include "enigma_event.h"
#include "enigma_main.h"

#if HAVE_DVB_API_VERSION < 3
#include <dbox/avia_gt_pig.h>
#define PIG "/dev/dbox/pig0"
#else
#include <linux/input.h>
#include <linux/videodev.h>
#define PIG "/dev/v4l/video0"
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
/* devices */

#include <lib/dvb/epgcache.h>
#include <lib/dvb/service.h>
#include <lib/dvb/si.h>
#include <lib/dvb/serviceplaylist.h>
#include <lib/gdi/epng.h>
#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/guiactions.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/epicon.h>
#include <lib/gdi/font.h>
#include <lib/driver/eavswitch.h>
#include <lib/system/init_num.h>

int drawTlines = 1;
gPixmap *eZapEPG::entry::inTimer=0;
gPixmap *eZapEPG::entry::inTimerRec=0;

static eString buildShortName( const eString &str )
{
	eString tmp;
	static char stropen[3] = { 0xc2, 0x86, 0x00 };
	static char strclose[3] = { 0xc2, 0x87, 0x00 };
	unsigned int open=eString::npos-1;

	while ( (open = str.find(stropen, open+2)) != eString::npos )
	{
		unsigned int close = str.find(strclose, open);
		if ( close != eString::npos )
			tmp+=str.mid( open+2, close-(open+2) );
	}
	return tmp;
}

struct multiEpgActions
{
	eActionMap map;
	eAction key1, key2, key3, key4, key5, key6, keyBack, keyForward, showUserBouquets, showChannelEPG, ok, exit, searchEPG;
	multiEpgActions():
		map("multiEpg", _("multi epg actions")),
		key1(map, "1", _("show 1 hour"), eAction::prioDialog),
		key2(map, "2", _("show 2 hours"), eAction::prioDialog),
		key3(map, "3", _("show 3 hours"), eAction::prioDialog),
		key4(map, "4", _("show 4 hours"), eAction::prioDialog),
		key5(map, "5", _("show 5 hours"), eAction::prioDialog),
		key6(map, "6", _("show 6 hours"), eAction::prioDialog),
		keyBack(map, "scrollback", _("scroll one page left"), eAction::prioDialog),
		keyForward(map, "scrollforward", _("scroll one page right"), eAction::prioDialog),
		showUserBouquets(map, "showUserBouquets", _("open the serviceselector and show bouquets"), eAction::prioDialog ),
		showChannelEPG(map, "showChannelEPG", _("show channel EPG"), eAction::prioDialog),
		ok(map, "ok", _("switch to selected channel"), eAction::prioDialog),
		exit(map, "exit", _("leave Multi EPG"), eAction::prioDialog),
		searchEPG(map, "searchEPG", _("search in EPG"), eAction::prioDialog)
	{
	}
};

eAutoInitP0<multiEpgActions> i_multiEpgActions(eAutoInitNumbers::actions, "multi epg actions");

eZapEPG::eZapEPG() 
	:eWidget(0,1), offs(0), focusColumn(0), hours(3)
	,numservices(6), eventWidget(0), NowTimeLineXPos(-1)
{
	init_eZapEPG();
}
void eZapEPG::init_eZapEPG()
{
	ViewSx=20, ViewSy=20, ViewEx=699, ViewEy=555, PigW=210, PigH=188, SNWidth=180;
	int fontIncrement = 0;
	eConfig::getInstance()->getKey("/elitedvb/multiepg/hours", hours);
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/left", ViewSx);
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/top", ViewSy);
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/right", ViewEx);
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/bottom", ViewEy);
	eConfig::getInstance()->getKey("/elitedvb/multiepg/pigwidth", PigW);
	eConfig::getInstance()->getKey("/elitedvb/multiepg/pigheight", PigH);
	eConfig::getInstance()->getKey("/elitedvb/multiepg/channellistwidth", SNWidth);
	eConfig::getInstance()->getKey("/elitedvb/multiepg/numservices", numservices);
	eConfig::getInstance()->getKey("/elitedvb/multiepg/fontIncrement", fontIncrement);
	if (eConfig::getInstance()->getKey("/elitedvb/multiepg/plicons", dopicon))
		dopicon = 0;
	if (eConfig::getInstance()->getKey("/elitedvb/multiepg/catcolor", doEPGCat))
		doEPGCat = 0;
	move(ePoint(ViewSx, ViewSy));
	resize(eSize(ViewEx - ViewSx, ViewEy - ViewSy));
	timeLine.setAutoDelete(true);
	timeFont = eSkin::getActive()->queryFont("epg.time");
	titleFont = eSkin::getActive()->queryFont("epg.title");
	descrFont = eSkin::getActive()->queryFont("epg.description");
	titleFont.pointSize += fontIncrement;
	// Do not increment description and time font size
	// timeFont.pointSize += fontIncrement;
	// descrFont.pointSize += fontIncrement;

	baseColor = eSkin::getActive()->queryColor("epg.base.background");
	if (baseColor == gColor(0))
		baseColor = eSkin::getActive()->queryColor("eStatusBar.background");
	entryColor = eSkin::getActive()->queryColor("epg.entry.background");
	entryFgColor = eSkin::getActive()->queryColor("epg.entry.foreground");
	if (entryFgColor == gColor(0))
		entryFgColor = getForegroundColor();
	entryColorSelected = eSkin::getActive()->queryColor("epg.entry.background.selected");

	timeBgCol = eSkin::getActive()->queryColor("epg.time.background");
	if (timeBgCol == gColor(0))
		timeBgCol = entryColor;
	timeFgCol = eSkin::getActive()->queryColor("epg.time.foreground");
	if (timeFgCol == gColor(0))
		timeFgCol = getForegroundColor();
	serviceBgCol = eSkin::getActive()->queryColor("epg.service.background");
	if (serviceBgCol == gColor(0))
		serviceBgCol = baseColor;
	serviceFgCol = eSkin::getActive()->queryColor("epg.service.foreground");
	if (serviceFgCol == gColor(0))
		serviceFgCol = getForegroundColor();
	descriptionBgCol = eSkin::getActive()->queryColor("epg.description.background");
	if (descriptionBgCol == gColor(0))
		descriptionBgCol = baseColor;
	descriptionFgCol = eSkin::getActive()->queryColor("epg.description.foreground");
	if (descriptionFgCol == gColor(0))
		descriptionFgCol = getForegroundColor();
	timelineColor = eSkin::getActive()->queryColor("epg.timeline");
	timelineNowColor = eSkin::getActive()->queryColor("epg.timeline.now");
	entry::inTimer = eSkin::getActive()->queryImage("timer_symbol");
	entry::inTimerRec = eSkin::getActive()->queryImage("timer_rec_symbol");

	addActionMap( &i_epgSelectorActions->map );
	addActionMap( &i_focusActions->map );
	addActionMap( &i_multiEpgActions->map );
	setBackgroundColor(gColor(0xff000000)); // transparrent so pig is visible

#ifndef DISABLE_FILE
	addActionToHelpList( &i_epgSelectorActions->addDVRTimerEvent );
#endif
#ifndef DISABLE_NETWORK
	addActionToHelpList( &i_epgSelectorActions->addNGRABTimerEvent );
#endif
	addActionToHelpList( &i_epgSelectorActions->addSwitchTimerEvent );
	addActionToHelpList( &i_epgSelectorActions->removeTimerEvent );
	addActionToHelpList( &i_epgSelectorActions->showExtendedInfo );
	addActionToHelpList( &i_multiEpgActions->showUserBouquets );
	addActionToHelpList( &i_multiEpgActions->showChannelEPG );
	addActionToHelpList( &i_multiEpgActions->searchEPG );
	addActionToHelpList( &i_multiEpgActions->ok );
	addActionToHelpList( &i_multiEpgActions->key1 );
	addActionToHelpList( &i_multiEpgActions->key2 );
	addActionToHelpList( &i_multiEpgActions->key3 );
	addActionToHelpList( &i_multiEpgActions->key4 );
	addActionToHelpList( &i_multiEpgActions->key5 );
	addActionToHelpList( &i_multiEpgActions->key6 );
	addActionToHelpList( &i_multiEpgActions->keyBack );
	addActionToHelpList( &i_multiEpgActions->keyForward );
	addActionToHelpList( &i_multiEpgActions->exit );

	buildServices();

	pig = -1;

	/* TODO: find a way to determine whether the pig device actually works, instead of checking hwtype */
	if (eSystemInfo::getInstance()->getHwType() != eSystemInfo::DM500
			&& eSystemInfo::getInstance()->getHwType() != eSystemInfo::DM500PLUS
			&& eSystemInfo::getInstance()->getHwType() != eSystemInfo::DM600PVR
			&& eSystemInfo::getInstance()->getHwType() != eSystemInfo::DM5600
			&& eSystemInfo::getInstance()->getHwType() != eSystemInfo::DM5620)
	{
		/* we don't want to open a PiG when we're in radiomode, or when we have no services (filemode outside a playlist) */
		if (eZapMain::getInstance() && eZapMain::getInstance()->getMode() != eZapMain::modeRadio && services.size())
		{
			/* open pig */
			pig = open(PIG, O_RDWR);
		}
	}

	if (pig >= 0)
	{
		int height = 0;
		if (eAVSwitch::getInstance()->getVSystem() == vsNTSC)
		{
			/*
			* Pig seems to be based on PAL dimensions.
			* This causes a garbage bar when using pig in NTSC.
			* To 'fix' this increase the pig height for NTSC by the relative
			* difference in NTSC/PAL lines.
			*/
			height = (PigH * 2) / 10 ; // (576-480)/480;
			if (height < 0)
				height = 0;
			// make more space for epg stuff...
			PigH -= height;
#if 0
			// Black bar to hide garbage on NTSC pig
			eLabel *black = new eLabel(this);
			black->move(ePoint(0, PigH - height) );
			black->resize( eSize(PigW, height) );
			black->setBackgroundColor(entryColor);
			black->setText("NTSC BUG FIX BAR");
			eDebug("Pig bar height = %d (PigH = %d)", height, PigH);

#endif
		}
#if HAVE_DVB_API_VERSION < 3
		avia_pig_hide(pig);
		avia_pig_set_pos(pig, ViewSx, ViewSy);
		avia_pig_set_size(pig, PigW, PigH + height);
		avia_pig_set_stack(pig, 2);
		avia_pig_show(pig);
#else
		struct v4l2_format format;
//struct v4l2_clip myclip
//{
//        struct v4l2_rect        c;
//        struct v4l2_clip        *next;
//};
		int sm = 0;
		ioctl(pig, VIDIOC_OVERLAY, &sm);
		sm = 1;
		myclip.c.left = 0;
		myclip.c.top = 0;
		myclip.c.width = PigW;
		myclip.c.height = PigH;

		myclip.next = NULL;
		ioctl(pig, VIDIOC_G_FMT, &format);
		format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
		format.fmt.win.w.left   = ViewSx;
		format.fmt.win.w.top    = ViewSy;
		format.fmt.win.w.width  = PigW;
		format.fmt.win.w.height = PigH;
//		format.fmt.win.clips    = &myclip;
		ioctl(pig, VIDIOC_S_FMT, &format);
		ioctl(pig, VIDIOC_OVERLAY, &sm);
#endif
	}
	else
	{ // no pig
		PigW = 0;
	}

	// detailed epg entry descriptiuon
	sbar = new eStatusBar(this);
	sbar->move( ePoint(PigW, 30) );
	sbar->resize( eSize( clientrect.width() - PigW, PigH - 30) );
	sbar->text_position = ePoint(5, 0);
	sbar->setFont(descrFont);
	sbar->loadDeco();
	sbar->setFlags(eStatusBar::flagOwnerDraw);
	sbar->removeFlags( eStatusBar::flagVCenter);

	// Help bar at the top next to pig
	eLabel *l = new eLabel(this);
	l->move(ePoint(PigW, 0) );
	l->setFont( eSkin::getActive()->queryFont("eStatusBar") );
	l->setBackgroundColor(entryColor);
	//l->resize( eSize( clientrect.width() - PigW - 170, 30) );
	l->resize( eSize( clientrect.width() - PigW, 30) );
	l->loadDeco();
	l->setText(_("Press HELP to get.... help"));
	l->setFlags( eLabel::flagVCenter );
	l->text_position = ePoint(5, 0);

#if 0
	eButton *add_tim=new eButton(this);
	add_tim->setText(_("Add"));
	add_tim->setShortcut("green");
	add_tim->setShortcutPixmap("green");
	add_tim->move(ePoint(clientrect.width() - 180, 0));
	add_tim->resize(eSize(70, 30));
	add_tim->setBackgroundColor(entryColor);
	add_tim->loadDeco();

	eButton *del_tim=new eButton(this);
	del_tim->setText(_("Del Timer"));
	del_tim->setShortcut("red");
	del_tim->setShortcutPixmap("red");
	del_tim->move(ePoint(clientrect.width() - 110, 0));
	del_tim->resize(eSize(110, 30));
	del_tim->setBackgroundColor(entryColor);
	del_tim->loadDeco();
#endif

	/* help text for multi epg screen */
	setHelpText(_("Multi EPG\n\n"
			"The Multi Channel EPG shows information about the programs on"
			" all channels in the current Bouqet.\n\n"
			"Please be aware that not all stations provide EPG data!\n\n"
			"The number of services shown per page and the fontsize of"
			" the short epg descriptions can be changed in the EPG Setup menu"));
}

eZapEPG::~eZapEPG()
{
	if (pig >= 0)
	{
#if HAVE_DVB_API_VERSION < 3
		avia_pig_hide(pig);
#else
		int screenmode = 0;
		ioctl(pig, VIDIOC_OVERLAY, &screenmode);
#endif
		close(pig);
	}

	for (std::list<serviceentry>::iterator it = serviceentries.begin(); it != serviceentries.end(); ++it)
	{
		/* setting the picon pixmap to NULL avoids the serviceentries calling 'compress' in their destructor */
		(*it).header->setPixmap(NULL);
	}

	/* delete old picons */
	for (unsigned int i = 0; i < picons.size(); i++)
	{
		delete picons[i];
	}

	eConfig::getInstance()->setKey("/elitedvb/multiepg/hours", hours);
}

void eZapEPG::show()
{
	eWidget::show();
	buildPage();
}

void eZapEPG::buildServices()
{
	services.clear();

	Signal1<void,const eServiceReference& > callback;
	CONNECT( callback, eZapEPG::addToList );
	eZap::getInstance()->getServiceSelector()->forEachServiceRef( callback, false );
	curS = curE = services.begin();

	current_service = serviceentries.end();
}

void eZapEPG::addToList( const eServiceReference& ref )
{
	if ( ref.type == eServiceReference::idDVB )
		services.push_back( (const eServiceReferenceDVB&) ref );
}

eZapEPG::entry::entry(eWidget *parent, gFont &timeFont, gFont &titleFont, 
	gFont &descrFont, gColor entryColor, gColor entryFgColor, gColor entryColorSelected, eWidget *sbar)
	: eWidget(parent), timeFont(timeFont),
	titleFont(titleFont), descrFont(descrFont), entryColor(entryColor), entryFgColor(entryFgColor), 
	entryColorSelected(entryColorSelected),
	sbar(sbar), para(0), xOffs(0), yOffs(0)
{
	setBackgroundColor(entryColor);
	setForegroundColor(entryFgColor);
	setRound(2);
};

eZapEPG::entry::~entry()
{
	if(para)
	{
		para->destroy();
		para=0;
	}
	delete event;
}

void eZapEPG::entry::setColor(gColor bgcolor, gColor fgcolor)
{
	entryColor = bgcolor;
	entryFgColor = fgcolor;
	setBackgroundColor(entryColor);
	setForegroundColor(entryFgColor);
}

void eZapEPG::entry::redrawWidget(gPainter *target, const eRect &area)
{
	if ( !para )
	{
		para=new eTextPara( eRect(0, 0, size.width(), size.height() ) );
		para->setFont(titleFont);
		para->renderString(title, RS_WRAP);
		int bboxHeight = para->getBoundBox().height();
		int bboxWidth = para->getBoundBox().width();
		yOffs = (bboxHeight < size.height()) ? (( size.height() - bboxHeight ) / 2) - para->getBoundBox().top() : 0;
		xOffs = (bboxWidth < size.width()) ? (( size.width() - bboxWidth ) / 2) - para->getBoundBox().left() : 0;
	}

	target->renderPara(*para, ePoint( area.left()+xOffs, area.top()+yOffs) );

	ePlaylistEntry* p=0;
	if ( (p = eTimerManager::getInstance()->findEvent( &service->service, (EITEvent*)event )) )
	{
		if ( p->type & ePlaylistEntry::SwitchTimerEntry )
			target->blit( *inTimer, ePoint( size.width()-inTimer->x-1, size.height()-inTimerRec->y-1 ), eRect(), gPixmap::blitAlphaTest);
		else if ( p->type & ePlaylistEntry::RecTimerEntry )
			target->blit( *inTimerRec, ePoint(size.width()-inTimerRec->x-1, size.height()-inTimerRec->y-1), eRect(), gPixmap::blitAlphaTest);
	}

	//target->setForegroundColor(entryColorSelected);
	//target->fill(eRect(0, size.height()-1, size.width(), 1));
	//target->fill(eRect(size.width()-1, 0, 1, size.height()));
	if (backgroundColor==entryColorSelected)
		redrawed();
}

void eZapEPG::entry::gotFocus()
{
#ifndef DISABLE_LCD
	eZapLCD* pLCD = eZapLCD::getInstance();
	unsigned int pos = 0;
	for (int i=0; i < 4; ++i)
		pos = helptext.find(' ', pos+1);
	if ( pos != eString::npos && (pos+1) < helptext.length() )
	{
		eString title =
			helptext.left(pos);
		title.removeChars(' ');
		pLCD->lcdMenu->Title->setText(title);
		pLCD->lcdMenu->Element->setText(helptext.mid(pos+1));
	}
#endif
	sbar->setText( helptext );
	setBackgroundColor(entryColorSelected);
}

void eZapEPG::entry::lostFocus()
{
//	setForegroundColor(normalF,false);
	setBackgroundColor(entryColor);
}

int eZapEPG::eventHandler(const eWidgetEvent &event)
{
	if (event.type == eWidgetEvent::evtAction)
	{
		int addtype = i_epgSelectorActions->checkTimerActions( event.action );
		int servicevalid = current_service != serviceentries.end();
		int eventvalid = (servicevalid && current_service->current_entry != current_service->entries.end());

		if (addtype != -1 && eventvalid)
		{
			if ( !eTimerManager::getInstance()->eventAlreadyInList( this, *(EITEvent*)current_service->current_entry->event, current_service->service) )
			{
				hide();
				eTimerEditView v( *(EITEvent*)current_service->current_entry->event, addtype, current_service->service);
				v.show();
				v.exec();
				v.hide();
				show();
			}
		}
		else if (event.action == &i_epgSelectorActions->removeTimerEvent)
		{
			if (eventvalid)
				if ( eTimerManager::getInstance()->removeEventFromTimerList( this, &current_service->service, current_service->current_entry->event ) )
						current_service->current_entry->invalidate();
		}
		else if (event.action == &i_focusActions->left)
			selEntry(-1);
		else if (event.action == &i_focusActions->right)
			selEntry(+1);
		else if (event.action == &i_focusActions->up)
			selService(-1);
		else if (event.action == &i_focusActions->down)
			selService(+1);
		else if (event.action == &i_multiEpgActions->ok)
			close(eventvalid?0:-1);
		else if (event.action == &i_multiEpgActions->exit)
			close(-1);
		else if (event.action == &i_epgSelectorActions->showExtendedInfo)
		{
			if (eventvalid)
			{
				eService *service=eDVB::getInstance()->settings->getTransponders()->searchService(current_service->service);
				eEventDisplay ei(service ? service->service_name.c_str() : "", current_service->service, 0, (EITEvent*)current_service->current_entry->event);

#ifndef DISABLE_LCD
				eZapLCD* pLCD = eZapLCD::getInstance();
				pLCD->lcdMain->hide();
				pLCD->lcdMenu->show();
				ei.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
#endif
				hide();
				ei.show();
				int ret;
				while((ret = ei.exec()))
				{
#if 0 /* PG: for now, disable scrolling. When buildPage is called (from inside setEntry), we segfault */
					if (ret == 1)
						selEntry(-1);
					else if (ret == 2)
						selEntry(+1);
					else
#endif
						break; // close EventDisplay
	
					ei.setEvent((EITEvent*)current_service->current_entry->event);
				}
				ei.hide();
				show();
				drawTimeLines();
			}
		}
		else if (event.action == &i_multiEpgActions->showChannelEPG)
			eZapMain::getInstance()->showEPGList(getCurSelected()->service);
		else if (event.action == &i_multiEpgActions->searchEPG)  // EPG search begin - added Searching from mEPG
		{
			if (eventvalid)
			{
				eService *service=eDVB::getInstance()->settings->getTransponders()->searchService(current_service->service);
				eEventDisplay ei(service ? service->service_name.c_str() : "", current_service->service, 0, (EITEvent*)current_service->current_entry->event);
				if (getCurSelected()->service.type != eServiceReference::idDVB)
					return 0;

				eString EPGSearchName = "";
				drawTlines = 0;
				eEPGSearch *dd = new eEPGSearch(getCurSelected()->service,current_service->current_entry->title);
				drawTlines = 1;
				dd->show(); 
				int back = 2;
				do
				{
					back = dd->exec();
					EPGSearchName = dd->getSearchName();
					if (back == 2)
					{
						dd->hide();
						eMessageBox rsl(EPGSearchName + eString(_(" was not found!")) , _("EPG Search"), eMessageBox::iconInfo|eMessageBox::btOK);
						rsl.show(); rsl.exec(); rsl.hide();
						dd->show();
					}
				}
				while (back == 2);
				dd->hide();
				delete dd;
				if (!back)
				{
#ifndef DISABLE_LCD
					eZapLCD* pLCD = eZapLCD::getInstance();
					pLCD->lcdMain->hide();
					pLCD->lcdMenu->show();
#endif
					eEPGSelector eEPGSelectorSearch(EPGSearchName);
#ifndef DISABLE_LCD
					eEPGSelectorSearch.setLCD(pLCD->lcdMenu->Title, pLCD->lcdMenu->Element);
#endif
					eEPGSelectorSearch.show(); eEPGSelectorSearch.exec(); eEPGSelectorSearch.hide();
				}
			}
			drawTimeLines();	
		} // EPG search end ims								
		else 
		{
			if (event.action == &i_multiEpgActions->key1)
				hours=1;
			else if (event.action == &i_multiEpgActions->key2)
				hours=2;
			else if (event.action == &i_multiEpgActions->key3)
				hours=3;
			else if (event.action == &i_multiEpgActions->key4)
				hours=4;
			else if (event.action == &i_multiEpgActions->key5)
				hours=5;
			else if (event.action == &i_multiEpgActions->key6)
				hours=6;
			else if (event.action == &i_multiEpgActions->keyBack)
			{
				if (offs < hours * 3600)
					return 1;
				offs -= hours * 3600;
			}
			else if (event.action == &i_multiEpgActions->keyForward)
				offs += hours * 3600;
			else if (event.action == &i_multiEpgActions->showUserBouquets)
			{
				eServiceSelector *e = eZap::getInstance()->getServiceSelector();
				/* avoid double execution, it can be that we were started by the service selector */
				if (e && e->isExecuting()) return 1;
				eZapMain::getInstance()->showServiceSelector( -1, eZapMain::pathBouquets );
				/* we need to rebuild our servicelist */
				buildServices();
			}
			else
				goto other_ev;

			buildPage();
		}

		return 1;
	}

other_ev:
	return eWidget::eventHandler(event);
}

void eZapEPG::buildService(serviceentry &service)
{
	int width = service.pos.width();
	service.entries.setAutoDelete(1);
	eEPGCache *epgcache=eEPGCache::getInstance();
	timeMapPtr evmap = epgcache->getTimeMapPtr(service.service, start, end); // add start/end optimizes sqlStore 
	if (!evmap)
	{
		return;
	}

	timeMap::const_iterator ibegin = evmap->lower_bound(start);
	if ((ibegin != evmap->end()) && (ibegin != evmap->begin()) )
	{
		if ( ibegin->first != start )
			--ibegin;
	}
	else
		ibegin=evmap->begin();

	timeMap::const_iterator iend = evmap->lower_bound(end);

	int tsidonid =
		(service.service.getTransportStreamID().get()<<16) | service.service.getOriginalNetworkID().get();

	for (timeMap::const_iterator event(ibegin); event != iend; ++event)
	{
		EITEvent *ev = new EITEvent(*event->second,tsidonid,event->second->type);
		if (((ev->start_time+ev->duration) >= start) && (ev->start_time <= end))
		{
			eString description;
			eString genre;
			int genreCategory;
			entry *e = new entry(eventWidget, timeFont, titleFont, descrFont, entryColor, entryFgColor, entryColorSelected, sbar);
			e->service = &service;
			int xpos = (ev->start_time - start) * width / (end - start);
			int ewidth = (ev->start_time + ev->duration - start) * width / (end - start);
			ewidth -= (xpos + 1) /* border */;

			if (xpos < 0)
			{
				ewidth += xpos;
				xpos = 0;
			}

			if ((xpos+ewidth) > width)
				ewidth = width - xpos;

			e->move(ePoint(service.pos.x() + xpos, service.pos.y()));
		        if (doEPGCat)
                        	e->resize(eSize(ewidth, service.pos.height() - 2 /* border plus genrebar */));	
			else
				e->resize(eSize(ewidth, service.pos.height() - 1 /* border */));	
			CONNECT( e->redrawed, eZapEPG::drawTimeLines );
			service.entries.push_back(e);

			LocalEventData led;
			led.getLocalData(ev, &e->title, &description);
			tm *begin=localtime(&ev->start_time);

			genre = "";
			genreCategory = 0; // none
			for (ePtrList<Descriptor>::iterator d(ev->descriptor); d != ev->descriptor.end(); ++d)
			{
				Descriptor *descriptor = *d;
				if (descriptor->Tag() == DESCR_CONTENT)
				{
					ContentDescriptor *cod = (ContentDescriptor *)descriptor;
					for (ePtrList<descr_content_entry_struct>::iterator ce(cod->contentList.begin()); ce != cod->contentList.end(); ++ce)
					{
						if (genreCategory == 0)
							genreCategory = ce->content_nibble_level_1;
						if (!genre && eChannelInfo::getGenre(genreCategory << 4 | ce->content_nibble_level_2))
							genre = gettext(eChannelInfo::getGenre(genreCategory << 4 | ce->content_nibble_level_2).c_str());
					}
				}
			}
			if (genre)
				genre = " - " + genre;
			if (doEPGCat && genreCategory)
			{
				int bgcolor = eSkin::getActive()->queryColor(eString().sprintf("genre%d", genreCategory));
				eLabel *d = new eLabel(eventWidget);
				d->move(ePoint(service.pos.x()+xpos,service.pos.y()+service.pos.height()-2));
				d->resize(eSize(ewidth, 2));
				d->setBackgroundColor(bgcolor);
			}

			eString tmp;
			tmp.sprintf("%02d.%02d. %s - ",
				begin->tm_mday,
				begin->tm_mon+1,
				getTimeStr(begin, 0).c_str());

			time_t endTime = ev->start_time + ev->duration;
			tm *end=localtime(&endTime);
			tmp+=eString().sprintf("%s %s%s\n%s",
				getTimeStr(end, 0).c_str(),
				e->title.c_str(), genre.c_str(), description.c_str());

			e->setHelpText(tmp);
			e->event = ev;
		}
		else
			delete ev;
	}
}

void eZapEPG::selService(int dir)
{
	if (serviceentries.begin() == serviceentries.end())
		return;
	int isok;
	ePtrList<entry>::iterator l = current_service->current_entry;
	isok = l != current_service->entries.end();
	if (dir == +1)
	{
		do
		{
			++current_service;
			if (current_service == serviceentries.end())
			{
				focusColumn=0;
				buildPage(ScrollDown);
				return;
			}
			else
				++focusColumn;
		}
		while(current_service->entries.empty());
	} else if (dir == -1)
	{
		do
		{
			if (current_service != serviceentries.begin())
			{
				--focusColumn;
				--current_service;
			}
			else
			{
				focusColumn=numservices-1;
				buildPage(ScrollUp);
				return;
			}
		}
		while(current_service->entries.empty());
	}
	time_t last_time=0;

	if (isok)
	{
		l->lostFocus();
		last_time = l->event->start_time;
	}
	
	if (current_service->current_entry != current_service->entries.end())
	{
		if (last_time)
		{
			int best_diff=0;
			ePtrList<entry>::iterator best=current_service->entries.end();
			for (ePtrList<entry>::iterator i(current_service->entries.begin()); 
					i != current_service->entries.end(); ++i)
			{
				if ((best == current_service->entries.end()) || abs(i->event->start_time-last_time) < best_diff)
				{
					best = i;
					best_diff = abs(i->event->start_time-last_time);
				}
			}
			
			if (best != current_service->entries.end())
				current_service->current_entry = best;
		}
		current_service->current_entry->gotFocus();
	}
}

void eZapEPG::selEntry(int dir)
{
	if (current_service == serviceentries.end() || current_service->entries.empty())
	{
		if ( dir == -1 && offs >= hours*3600 )
		{
			offs -= hours*3600;
			buildPage(ScrollLeft);
		}
/*		else
			eDebug("invalid service");*/
		return;
	}
	ePtrList<entry>::iterator l = current_service->current_entry;
	if ( dir == +1)
	{
		++current_service->current_entry;
		if (current_service->current_entry == current_service->entries.end())
		{
			if ( eventWidget->isVisible() )
			{
				offs += hours*3600;
				buildPage(ScrollRight);
			}
			else
				--current_service->current_entry;
			return;
		}
	}
	else
	{
		if (current_service->current_entry == current_service->entries.begin())
		{
			if ( offs >= hours*3600 )
			{
				offs -= hours*3600;
				buildPage(ScrollLeft);
			}
			return;
		}
		--current_service->current_entry;
	}
	if (l != current_service->entries.end())
		l->lostFocus();
	current_service->current_entry->gotFocus();
}

void eZapEPG::buildPage(int direction)
{
	// Clear timeline and event space so we have room to build a new one
	if ( eventWidget )
		eventWidget->hide();
	for (std::list<serviceentry>::iterator it = serviceentries.begin(); it != serviceentries.end(); )
	{
		/* setting the picon pixmap to NULL avoids the serviceentries calling 'compress' in their destructor */
		(*it).header->setPixmap(NULL);
		it = serviceentries.erase(it);
	}
	current_service = serviceentries.end();
	NowTimeLineXPos = -1;
	timeLine.clear();
	delete eventWidget;

	/* delete old picons */
	for (unsigned int i = 0; i < picons.size(); i++)
	{
		delete picons[i];
	}
	picons.clear();

	eventWidget = new eWidget( this );
	eventWidget->move(ePoint(0, PigH));
	eventWidget->resize( eSize( clientrect.width(), clientrect.height() - PigH) );
	eventWidget->setBackgroundColor(baseColor);
#ifndef DISABLE_LCD
	eventWidget->setLCD( LCDTitle, LCDElement );
#endif

	start = time(0) + eDVB::getInstance()->time_difference + offs;
	unsigned int tmp = start % 1800;  // align to 30 min
	start -= tmp;
	end = start + hours * 3600;

	int width = clientrect.width();
	int serviceheight = (eventWidget->height()-21) / numservices;

	time_t tmpTime=start;
	tm *bla = localtime(&tmpTime);

	// Show date of first timestamp
        const char *dayStrShort[7] = { _("Sun"), _("Mon"), _("Tue"), _("Wed"), _("Thu"), _("Fri"), _("Sat") };
        const char *monStrShort[12] = { _("Jan"), _("Feb"), _("Mar"), _("Apr"), _("May"), _("Jun"),
					_("Jul"), _("Aug"), _("Sep"), _("Oct"), _("Nov"), _("Dec") };
	eLabel *d = new eLabel(eventWidget);
	d->move(ePoint(0, 0));
	d->resize(eSize(SNWidth-3, 20)); // timeline height
	d->setFlags(eLabel::flagVCenter);
	d->text_position = ePoint(5, 0);
	d->setText(eString().sprintf("%3s %02d %3s", dayStrShort[bla->tm_wday], bla->tm_mday, monStrShort[bla->tm_mon]));
	d->setFont(timeFont);
	d->setBackgroundColor(timeBgCol);
	d->setForegroundColor(timeFgCol);

	// show time labels
	int timeWidth = (width - SNWidth) / (hours > 3 ? hours : hours*2);
	gColor col;
	for (unsigned int i=0; i < (hours > 3 ? hours : hours*2); ++i)
	{
		eLabel *l = new eLabel(eventWidget);
		//l->move(ePoint( i*timeWidth-(timeWidth/2)+100, 0));
		l->move(ePoint( i*timeWidth+SNWidth+1, 0));
		l->resize(eSize(timeWidth-1,20)); // timeline height
		l->setFlags(eLabel::flagVCenter);
		//l->setAlign(eTextPara::dirCenter);
		l->setAlign(eTextPara::dirLeft);
		tm *bla = localtime(&tmpTime);
		l->setText(getTimeStr(bla, 0));
		l->setFont(timeFont);
		l->setBackgroundColor(timeBgCol);
		l->setForegroundColor(timeFgCol);
		timeLine.push_back(l);
		tmpTime += hours>3?3600:1800;
	}

	std::set<int> nonEmptyServices;

	int p = 0;
	if ( direction == ScrollUp ) // we must count "numservices" back
	{
		std::list<eServiceReferenceDVB>::iterator s(curS);
		unsigned int cnt=0;
		do
		{
			if ( s == services.end() )
				break;
			if ( ++cnt > numservices )
					break;
			if ( s != services.begin() )
				--s;
			else
			{
				s = services.end();
				if (s != services.begin())
					--s;
				else
					break;
			}
		}
		while( s != curS );
		curS = curE = s;
	}
	else if (direction == ScrollDown)
		curS = curE;
	else if (direction == ScrollLeft || direction == ScrollRight || direction == ScrollNone)
		curE = curS;

	do
	{
		if ( curE == services.end() )
			break;

//		const eventData *e = (const eventData*) eEPGCache::getInstance()->lookupEvent( *curE, (time_t)(start+tmp), true );
//		if ( e )
		{
			serviceentries.push_back(serviceentry());
			serviceentry &service = serviceentries.back();
			service.header = new eLabel(eventWidget); // service name
			service.header->move(ePoint(0, p * serviceheight + 21));
			service.header->resize(eSize(SNWidth-3, serviceheight - 1));
			// service.header->text_position = ePoint(5, 0);
			service.header->text_position = ePoint(42, 0);
			service.pos = eRect(SNWidth, p * serviceheight + 21, width - SNWidth, serviceheight ); // epg bar for service
			service.header->setFont(titleFont); // use the EPG title font for service name too
			service.header->setBackgroundColor(serviceBgCol);
			service.header->setForegroundColor(serviceFgCol);

			eString stext;
			if ( curE->descr )   // have changed service name?
				stext=curE->descr;  // use this...
			else
			{
				eService *sv=eServiceInterface::getInstance()->addRef(*curE);
				if ( sv )
				{
					eString shortname = buildShortName( sv->service_name );
					stext = shortname ? shortname : sv->service_name;
					// eDebug("multiepg: service name=%s, shortname=%s. serviceprovider=%s, namespace=%d, transport_stream_id=%d, original_network_id=%d, service_id=%d, service_type=%d, service_number=%d", sv->service_name.c_str(), shortname.c_str(), sv->dvb->service_provider.c_str(), sv->dvb->dvb_namespace.get(), sv->dvb->transport_stream_id.get(), sv->dvb->original_network_id.get(), sv->dvb->service_id.get(), sv->dvb->service_type, sv->dvb->service_number);
					eServiceInterface::getInstance()->removeRef(*curE);
				}
			}
			service.service = *curE;

			// set column service name
			service.header->setText(stext);
			if (dopicon)
			{
				/* TODO: dynamic size? */
				gPixmap *picon = ePicon::loadPicon(*curE, eSize(40, 30));
				if (picon)
				{
					picons.push_back(picon);
				}
				// eDebug("multiepg: loading PLi icon %s", picon ? "ok" : "failed");
				service.header->setPixmap(picon);
				if (picon && picon->getSize().height() < serviceheight - 1) // Vcenter icon
					service.header->setPixmapPosition(ePoint(0, (serviceheight - 1 - picon->getSize().height()) / 2));
				service.header->text_position = ePoint(42, 0);
			}
			else
				service.header->text_position = ePoint(5, 0);
			service.header->setFlags( eLabel::flagVCenter );

			buildService(service);

			if ( service.entries.empty() ) // Should we just skip emtpy services?
			{
#ifdef SKIP_EMPTY_EPG
				// skip channels without epg
				// only works well for scrolling down. 
				// when scrolling up you again see part of the last screen as we just go maxentries channels up
				serviceentries.pop_back();
				// eDebug("multiepg: no epg data for %s skipping channel in view", stext.c_str());
#else
				service.current_entry = service.entries.end();
				++p;
#endif
			}
			else
			{
				// set focus line
				if ( direction == ScrollLeft )  // left pressed
				// set focus to last event on the row
					service.current_entry = --service.entries.end();
				else  // set focus to first event on the row
					service.current_entry = service.entries.begin();
				nonEmptyServices.insert(p);
				// eDebug("multiepg: added epg data for %s", stext.c_str());
				++p;
			}
		}
		if ( ++curE == services.end() )  // need wrap ?
			curE = services.begin();
	}
	while( serviceentries.size() < numservices && curE != curS );

	if (!p)
	{
		eDebug("multiepg: no services with epg data");
		sbar->setText("");
		return;
	}

	eventWidget->show();

	if ( serviceentries.empty() )
	{
		drawTimeLines();
		return;
	}

	std::set<int>::iterator it =
		nonEmptyServices.lower_bound(focusColumn);
	/* scroll back to the bottom non-empty service */
	while ( it == nonEmptyServices.end() && it != nonEmptyServices.begin() ) --it;
	if ( it != nonEmptyServices.end() )
		focusColumn = *it;
	else
		focusColumn = 0;

	// set column focus
	current_service = serviceentries.begin();
	for (unsigned int i=0; i < focusColumn; i++ )
	{
		if (current_service == --serviceentries.end())
			break;
		current_service++;
	}

	if (current_service->current_entry != current_service->entries.end())
		current_service->current_entry->gotFocus();

	if (nonEmptyServices.empty())
		drawTimeLines();
}

void eZapEPG::drawTimeLines()
{
	if ( eventWidget && eventWidget->isVisible() && timeLine.size() && drawTlines )
	{
		gPainter *p = getPainter(eRect(eventWidget->getPosition(),eventWidget->getSize()));
		int incWidth=((eventWidget->width()-SNWidth)/(hours>3?hours:hours*2));
		int hpos=SNWidth;
		int lineheight = eventWidget->height()-21;
		int serviceheight = lineheight / numservices;

		// Remove old now-timeline
		if ((timelineNowColor.color >> 24) != 0xFF)
		{
			if ( NowTimeLineXPos != -1 )
			{
				int tmp=NowTimeLineXPos;
				NowTimeLineXPos=-1;
				invalidate( eRect( tmp, PigH + 21, 2, lineheight) );
			}
		}
		// Draw periodic time lines, only if color is nontransparent
		if ((timelineColor.color >> 24) != 0xFF)
		{
			p->setForegroundColor( timelineColor );
			for (ePtrList<eLabel>::iterator it(timeLine); it != timeLine.end(); ++it)
			{
				//p->fill(eRect(hpos,20,1,lineheight));
				//p->fill(eRect(hpos, PigH, 1, eventWidget->height()));
				p->fill(eRect(hpos, PigH, 1, 20+2));
				int vpos = PigH + 21 - 3 + serviceheight;
				for (unsigned int k = 1; k < numservices; k++)
				{
					p->fill(eRect(hpos, vpos, 1, 5));
					vpos += serviceheight;
				}
				p->fill(eRect(hpos, vpos, 1, eventWidget->height() - vpos));
				hpos += incWidth;
			}
		}

		// Draw current time line
		if ((timelineNowColor.color >> 24) != 0xFF)
		{
			time_t now=time(0)+eDVB::getInstance()->time_difference;
			if ( now >= start && now < end )
			{
				int bla = ((eventWidget->width()-SNWidth)*1000) / (hours*60);
				NowTimeLineXPos = ((now/60) - (start/60)) * bla / 1000;
				NowTimeLineXPos += SNWidth;
				p->setForegroundColor( timelineNowColor );
				p->fill(eRect(NowTimeLineXPos, PigH + 21, 2, lineheight));
			}
		}

		// Draw seperator between timeline and event area
		// p->setForegroundColor(entryColorSelected);
		// p->fill(eRect(SNWidth, PigH + 20, eventWidget->width()-hpos, 1));
		delete p;
	}
}
