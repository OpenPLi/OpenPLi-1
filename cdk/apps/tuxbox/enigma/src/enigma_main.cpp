#include <enigma_main.h>

#include <errno.h>
#include <iomanip>
//#include <stdio.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <streaminfo.h>
#include <parentallock.h>
#include <enigma_mainmenu.h>
#include <enigma_event.h>
#include <enigma.h>
#include <enigma_vcr.h>
#include <enigma_standby.h>
#include <enigma_lcd.h>
#include <enigma_plugins.h>
#include <enigma_ci.h>
#include <enigma_streamer.h>
#include <helpwindow.h>
#include <timer.h>
#include <download.h>
#include <enigma_epg.h>
#include <epgwindow.h>
#include <lib/base/i18n.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/econfig.h>
#include <lib/system/file_eraser.h>
#include <lib/dvb/servicedvb.h>
#include <lib/dvb/dvbci.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/esection.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/servicemp3.h>
#include <lib/dvb/servicestructure.h>
#include <lib/dvb/serviceplaylist.h>
#include <lib/dvb/frontend.h>
#include <lib/dvb/dvbwidgets.h>
#include <lib/dvb/record.h>
#include <lib/gdi/font.h>
#include <lib/gui/elabel.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/egauge.h>
#include <lib/gui/enumber.h>
#include <lib/gui/eskin.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/actions.h>
#include <lib/gui/echeckbox.h>
#include <lib/driver/rc.h>
#include <lib/driver/streamwd.h>
#include <lib/driver/eavswitch.h>
#include <lib/dvb/dvbservice.h>
#include <lib/gdi/lcd.h>
#include <lib/gdi/glcddc.h>
#include <lib/system/info.h>
#include <src/time_correction.h>
#include <lib/driver/audiodynamic.h>
#include <lib/dvb/cahandler.h>
#include <media_mapping.h>
#include <callablemenu.h>
#include <epgsearch.h> // EPG search
#include <key_mapping.h>
#include <lib/movieplayer/movieplayer.h>
#include <src/enigma_tuxtxt.h>
#ifndef TUXTXT_CFG_STANDALONE
#include <tuxtxt/tuxtxt_def.h>
extern "C" tuxtxt_cache_struct tuxtxt_cache;
#endif

struct enigmaMainActions
{
	eActionMap map;
	eAction showMainMenu, standby_press, standby_nomenu_press, standby_repeat, standby_release,
		showInfobar, hideInfobar, myshowInfobar, showInfobarEPG, showServiceSelector,
		showSubservices, yellowButton, showAudio, pluginVTXT, blueButton, showEPGList, showEPG,
		nextSubService, prevSubService, nextService, prevService, greenButton,
		playlistNextService, playlistPrevService, serviceListDown,
		serviceListUp, volumeUp, volumeDown, toggleMute,
		stop, pause, play, record,
		startSkipForward, repeatSkipForward, stopSkipForward,
		startSkipReverse, repeatSkipReverse, stopSkipReverse,
		discrete_stop, discrete_pause, discrete_play, discrete_record,
		discrete_startSkipForward, discrete_repeatSkipForward, discrete_stopSkipForward,
		discrete_startSkipReverse, discrete_repeatSkipReverse, discrete_stopSkipReverse,
		showUserBouquets, showDVBBouquets, showRecMovies, showPlaylist,
		modeTV, modeRadio, modeFile,
		toggleDVRFunctions, toggleIndexmark, indexSeekNext, indexSeekPrev, stepForward, stepBack;
	enigmaMainActions():
		map("enigmaMain", _("enigma Zapp")),
		showMainMenu(map, "showMainMenu", _("show main menu"), eAction::prioDialog),
		standby_press(map, "standby_press", _("go to standby (press)"), eAction::prioDialog),
		standby_nomenu_press(map, "standby_nomenu_press", _("go to standby without menu (press)"), eAction::prioDialog),
		standby_repeat(map, "standby_repeat", _("go to standby (repeat)"), eAction::prioDialog),
		standby_release(map, "standby_release", _("go to standby (release)"), eAction::prioDialog),

		showInfobar(map, "showInfobar", _("show infobar"), eAction::prioDialog),
		hideInfobar(map, "hideInfobar", _("hide infobar"), eAction::prioDialog),
		myshowInfobar(map, "myshowInfobar", _("show infobar"), eAction::prioDialog),
		showInfobarEPG(map, "showInfobarEPG", _("show infobar or EPG"), eAction::prioDialog),
		showServiceSelector(map, "showServiceSelector", _("show service selector"), eAction::prioDialog),
		showSubservices(map, "showSubservices", _("show subservices/NVOD"), eAction::prioDialog),
		yellowButton(map, "yellowButton", _("execute programmable yellow button"), eAction::prioDialog),
		showAudio(map, "showAudio", _("show audio selector"), eAction::prioDialog),
		pluginVTXT(map, "pluginVTXT", _("show Videotext"), eAction::prioDialog),
		blueButton(map, "blueButton", _("execute programmable blue button"), eAction::prioDialog),
		showEPGList(map, "showEPGList", _("show epg schedule list"), eAction::prioDialog),
		showEPG(map, "showEPG", _("show extended info"), eAction::prioDialog),
		nextSubService(map, "nextSubService", _("zap to next subService"), eAction::prioDialog),
		prevSubService(map, "prevSubService", _("zap to prev subService"), eAction::prioDialog),
		nextService(map, "nextService", _("quickzap next"), eAction::prioDialog),
		prevService(map, "prevService", _("quickzap prev"), eAction::prioDialog),
		greenButton(map, "greenButton", _("execute programmable green button"), eAction::prioDialog),

		playlistNextService(map, "playlistNextService", _("playlist/history next"), eAction::prioDialog),
		playlistPrevService(map, "playlistPrevService", _("playlist/history prev"), eAction::prioDialog),

		serviceListDown(map, "serviceListDown", _("service list and down"), eAction::prioDialog),
		serviceListUp(map, "serviceListUp", _("service list and up"), eAction::prioDialog),

		volumeUp(map, "volumeUp", _("volume up"), eAction::prioDialog),
		volumeDown(map, "volumeDown", _("volume down"), eAction::prioDialog),
		toggleMute(map, "toggleMute", _("toggle mute flag"), eAction::prioDialog),

		stop(map, "stop", _("stop playback"), eAction::prioWidget),
		pause(map, "pause", _("pause playback"), eAction::prioWidget),
		play(map, "play", _("resume playback"), eAction::prioWidget),
		record(map, "record", _("record"), eAction::prioWidget),

		startSkipForward(map, "startSkipF", _("start skipping forward"), eAction::prioWidget),
		repeatSkipForward(map, "repeatSkipF", _("repeat skipping forward"), eAction::prioWidget),
		stopSkipForward(map, "stopSkipF", _("stop skipping forward"), eAction::prioWidget),

		startSkipReverse(map, "startSkipR", _("start skipping reverse"), eAction::prioWidget),
		repeatSkipReverse(map, "repeatSkipR", _("repeat skipping reverse"), eAction::prioWidget),
		stopSkipReverse(map, "stopSkipR", _("stop skipping reverse"), eAction::prioWidget),

		discrete_stop(map, "discrete_stop", _("stop playback"), eAction::prioWidget),
		discrete_pause(map, "discrete_pause", _("pause playback"), eAction::prioWidget),
		discrete_play(map, "discrete_play", _("resume playback"), eAction::prioWidget),
		discrete_record(map, "discrete_record", _("record"), eAction::prioWidget),

		discrete_startSkipForward(map, "discrete_startSkipF", _("start skipping forward"), eAction::prioWidget),
		discrete_repeatSkipForward(map, "discrete_repeatSkipF", _("repeat skipping forward"), eAction::prioWidget),
		discrete_stopSkipForward(map, "discrete_stopSkipF", _("stop skipping forward"), eAction::prioWidget),

		discrete_startSkipReverse(map, "discrete_startSkipR", _("start skipping reverse"), eAction::prioWidget),
		discrete_repeatSkipReverse(map, "discrete_repeatSkipR", _("repeat skipping reverse"), eAction::prioWidget),
		discrete_stopSkipReverse(map, "discrete_stopSkipR", _("stop skipping reverse"), eAction::prioWidget),

		showUserBouquets(map, "showUserBouquets", _("open the serviceselector and show bouquets"), eAction::prioWidget),
		showDVBBouquets(map, "showDVBBouquets", _("open the serviceselector and show provider"), eAction::prioWidget),
		showRecMovies(map, "showRecMovies", _("open the serviceselector and show recorded movies"), eAction::prioWidget),
		showPlaylist(map, "showPlaylist", _("open the serviceselector and shows the playlist"), eAction::prioWidget),

		modeTV(map, "modeTV", _("switch to TV mode"), eAction::prioDialog),
		modeRadio(map, "modeRadio", _("switch to Radio mode"), eAction::prioDialog),
		modeFile(map, "modeFile", _("switch to File mode"), eAction::prioDialog),

		toggleDVRFunctions(map, "toggleDVRFunctions", _("toggle DVR panel"), eAction::prioDialog),
		toggleIndexmark(map, "toggleIndexmark", _("toggle index mark"), eAction::prioDialog),

		indexSeekNext(map, "indexSeekNext", _("seek to next index mark"), eAction::prioDialog),
		indexSeekPrev(map, "indexSeekPrev", _("seek to previous index mark"), eAction::prioDialog),
 	 
		stepForward(map, "stepForward", _("step forward"), eAction::prioDialog), 	 
		stepBack(map, "stepBack", _("step back"), eAction::prioDialog) 	 
	{
	}
};

eAutoInitP0<enigmaMainActions> i_enigmaMainActions(eAutoInitNumbers::actions, "enigma main actions");

struct enigmaGlobalActions
{
	eActionMap map;
	eAction volumeUp, volumeDown, toggleMute;
	enigmaGlobalActions():
		map("enigmaGlobal", "enigma global"),

		volumeUp(map, "volumeUp", _("volume up"), eAction::prioGlobal),
		volumeDown(map, "volumeDown", _("volume down"), eAction::prioGlobal),
		toggleMute(map, "toggleMute", _("toggle mute flag"), eAction::prioGlobal)
	{
		eWidget::addGlobalActionMap(&map);
	}
	~enigmaGlobalActions()
	{
		eWidget::removeGlobalActionMap(&map);
	}
};

eAutoInitP0<enigmaGlobalActions> i_enigmaGlobalActions(eAutoInitNumbers::actions, "enigma global actions");

eString getTimeStr(tm *timem, int flags)
{
	int clktype = 0;
	eString clock;
	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);
	char str[12];

	if (clktype == 1)  // 12 hour clock
	{
		if (flags & gTS_NOAPM) // no AM/PM in 12-hour clock
		{
			strftime(str, 12, (flags & gTS_SECS) ? "%l:%M:%S" : "%l:%M", timem);
		}
		else
		{
			strftime(str, 12, (flags & gTS_SECS) ? "%l:%M:%S%P" : "%l:%M%P", timem);
			if (flags & gTS_SHORT) // short strings, remove the M
				str[strlen(str) - 1] = '\0';
		}
	}
	else  // 24 hour clock
	{
		strftime(str, 12, (flags & gTS_SECS) ? "%H:%M:%S" : "%H:%M", timem);
	}

	clock.sprintf("%s", str);
	return clock;
}

eString getDateStr(tm *t, int flags)
{
   eString clock;
   char charBeforeYear=' '; 
  
   int clktype = 0;
   eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);
  
   const char *dayStrShort[7] = { _("Su"), _("Mo"), _("Tu"), _("We"), _("Th"), _("Fr"), _("Sa") };
   const char *dayStrLong[7] = { _("Sunday"), _("Monday"), _("Tuesday"), _("Wednesday"), _("Thursday"), _("Friday"), _("Saturday") };
   const char *monStrShort[12] = { _("Jan"), _("Feb"), _("Mar"), _("Apr"), _("May"), _("Jun"),
                                        _("Jul"), _("Aug"), _("Sep"), _("Oct"), _("Nov"), _("Dec") };
   const char *monStrLong[12] = { _("January"), _("February"), _("March"), _("April"), _("May"), _("June"),
                                        _("July"), _("August"), _("September"), _("October"), _("November"), _("December") };

   if (clktype == 0) //12 hour NA clock
   {
      switch (flags&7)
      {
         case 0:
            clock=eString().sprintf("%s, %2d %s",dayStrShort[t->tm_wday], t->tm_mday, monStrShort[t->tm_mon]);
            charBeforeYear=' ';
	    break;
         case 1:
            clock=eString().sprintf("%s, %2d %s",dayStrLong[t->tm_wday], t->tm_mday, monStrLong[t->tm_mon]);
            charBeforeYear=' ';
            break;
         case 2:
            clock=eString().sprintf("%s %02d-%02d",dayStrLong[t->tm_wday], t->tm_mday, t->tm_mon+1);
            charBeforeYear='-';
            break;
         case 3:
            clock=eString().sprintf("%s %02d.%02d",dayStrLong[t->tm_wday], t->tm_mday, t->tm_mon+1);
            charBeforeYear='.';
            break;
         case 4:
            clock=eString().sprintf("%02d-%02d",t->tm_mday, t->tm_mon+1);
            charBeforeYear='-';
            break;
         case 5:
            clock=eString().sprintf("%02d.%02d",t->tm_mday, t->tm_mon+1);
            charBeforeYear='.';
            break;
      }
   }
   else
   {
      switch (flags&7)
      {
         case 0:
            clock=eString().sprintf("%s, %s %2d",dayStrShort[t->tm_wday], monStrShort[t->tm_mon], t->tm_mday);
            charBeforeYear=' ';
	    break;
         case 1:
            clock=eString().sprintf("%s, %s %2d",dayStrLong[t->tm_wday], monStrLong[t->tm_mon], t->tm_mday);
            charBeforeYear=' ';
            break;
         case 2:
            clock=eString().sprintf("%s %02d-%02d",dayStrLong[t->tm_wday], t->tm_mon+1, t->tm_mday);
            charBeforeYear='-';
            break;
         case 3:
            clock=eString().sprintf("%s %02d.%02d",dayStrLong[t->tm_wday], t->tm_mon+1, t->tm_mday);
            charBeforeYear='.';
            break;
         case 4:
            clock=eString().sprintf("%02d-%02d", t->tm_mon+1, t->tm_mday);
            charBeforeYear='-';
            break;
         case 5:
            clock=eString().sprintf("%02d.%02d", t->tm_mon+1, t->tm_mday);
            charBeforeYear='.';
            break;
      }
   }
   switch ((flags&24)>>3)
   {
      case 1:
           clock+=eString().sprintf("%c%02d",charBeforeYear,t->tm_year % 100);
           break;
      case 2:
           clock+=eString().sprintf("%c20%02d",charBeforeYear,t->tm_year % 100);
           break;
   }
   return clock;
}

#ifndef DISABLE_FILE

eZapSeekIndices::eZapSeekIndices()
{
	length = -1;
}

void eZapSeekIndices::load(const eString &filename)
{
	this->filename=filename;
	FILE *f=fopen(filename.c_str(), "rt");
	if (!f)
		return;

	char line[64];
	while (fgets(line, sizeof(line), f))
	{
		int real;
		Index marker;
		marker.type = 0;
		if (sscanf(line, "%d %d %d", &real, &marker.time, &marker.type) < 2)
			break;
		index.insert(std::pair<int,Index>(real, marker));
	}
	fclose(f);
	changed=0;
}

void eZapSeekIndices::save()
{
	if (!changed)
		return;

	if (!index.size())
	{
		unlink(filename.c_str());
		return;
	}

	FILE *f=fopen(filename.c_str(), "wt");
	if (!f)
		return;

	for (std::map<int, Index>::const_iterator i(index.begin()); i != index.end(); ++i)
	{
		if (i->second.type > 0)
		{
			fprintf(f, "%d %d %d\n", i->first, i->second.time, i->second.type);
		}
		else
		{
			fprintf(f, "%d %d\n", i->first, i->second.time);
		}
	}
	fclose(f);
	changed=0;
}

void eZapSeekIndices::add(int real, int time, int type)
{
	Index marker;
	marker.time = time;
	marker.type = type;
	if (type > 0)
	{
		/* nonzero index types can orrur only once */
		removeType(type);
	}
	index.insert(std::pair<int,Index>(real, marker));
	changed = 1;
}

void eZapSeekIndices::remove(int real)
{
	index.erase(real);
	changed = 1;
}

void eZapSeekIndices::removeType(int type)
{
	/* removes the first occurrance of 'type' (nonzero types occur once at most) */
	for (std::map<int, Index>::iterator i(index.begin()); i != index.end(); ++i)
	{
		if (i->second.type == type)
		{
			index.erase(i);
			break;
		}
	}
}

void eZapSeekIndices::clear()
{
	index.clear();
	changed = 1;
}

int eZapSeekIndices::getNext(int real, int dir)
{
	int diff=-1, r=-1;
	for (std::map<int, Index>::const_iterator i(index.begin()); i != index.end(); ++i)
	{
		if ((dir > 0) && (i->first <= real))
			continue;
		if ((dir < 0) && (i->first >= real))
			break;
		int d=abs(i->first-real);
		if ((d < diff) || (diff == -1))
		{
			diff=d;
			r=i->first;
		} else
			break;
	}
	return r;
}

int eZapSeekIndices::getTime(int real)
{
	std::map<int, Index>::const_iterator i=index.find(real);
	if (i != index.end())
		return i->second.time;
	return -1;
}

void eZapSeekIndices::setTotalLength(int l)
{
	length = l;
}

int  eZapSeekIndices::getTotalLength()
{
	return length;
}

eProgressWithIndices::eProgressWithIndices(eWidget *parent): eProgress(parent)
{
	indexmarkcolor = eSkin::getActive()->queryColor("indexmark");
}

void eProgressWithIndices::redrawWidget(gPainter *target, const eRect &area)
{
	eProgress::redrawWidget(target, area);
	if (!indices)
		return;
	int  len = indices->getTotalLength();
	if (len <= 0)
		return;
	int xlen = size.width();
	target->setForegroundColor(indexmarkcolor);
	for (int i=indices->getNext(-1, 1); i != -1; i=indices->getNext(i, 1))
	{
		int time = indices->getTime(i);
		int pos  = time * xlen / len;
		target->fill(eRect(pos - 2, 0, 4, size.height()));
	}
}

void eProgressWithIndices::setIndices(eZapSeekIndices *i)
{
	indices = i;
	invalidate();
}

static eWidget *create_eProgressWithIndices(eWidget *parent)
{
	return new eProgressWithIndices(parent);
}

class eProgressWithIndicesSkinInit
{
public:
        eProgressWithIndicesSkinInit()
        {
                eSkin::addWidgetCreator("eProgressIndex", create_eProgressWithIndices);
        }
        ~eProgressWithIndicesSkinInit()
        {
                eSkin::removeWidgetCreator("eProgressIndex", create_eProgressWithIndices);
        }
};

eAutoInitP0<eProgressWithIndicesSkinInit> init_eProgressWithIndicesSkinInit(eAutoInitNumbers::guiobject, "eProgressIndex");

#else

class eProgressWithIndicesSkinInit
{
public:
        eProgressWithIndicesSkinInit()
        {
                eSkin::addWidgetCreator("eProgressIndex", create_eProgress);
        }
        ~eProgressWithIndicesSkinInit()
        {
                eSkin::removeWidgetCreator("eProgressIndex", create_eProgress);
        }
};

eAutoInitP0<eProgressWithIndicesSkinInit> init_eProgressWithIndicesSkinInit(eAutoInitNumbers::guiobject, "eProgressIndex");

#endif

int NVODStream::validate()
{
	text.str(eString());
	for (ePtrList<EITEvent>::const_iterator event(eit.events); event != eit.events.end(); ++event)		// always take the first one
	{
		tm *begin=event->start_time!=-1?localtime(&event->start_time):0;

		if (begin)
			text << getTimeStr(begin, 0);

		time_t endtime=event->start_time+event->duration;
		tm *end=event->start_time!=-1?localtime(&endtime):0;

		if (end)
			text << _(" to ") << getTimeStr(end, 0);

		time_t now=time(0)+eDVB::getInstance()->time_difference;

		if ( now > endtime )
			return 0;

		if ((event->start_time <= now) && (now < endtime))
		{
			int perc=(now-event->start_time)*100/event->duration;
			text << " (" << perc << "%, " << perc*3/100 << '.' << std::setw(2) << (perc*3)%100 << _(" Euro lost)");
		}
		return event->start_time;
	}
	return 0;
}

void NVODStream::EITready(int error)
{
	eDebug("NVOD eit ready: %d", error);

	if ( error )
		delete this;
	else if ( eit.ready && !error && !begTime && (begTime = validate()) )
	{
		listbox->append( this );
		((eListBox<NVODStream>*)listbox)->sort(); // <<< without explicit cast the compiler nervs ;)
		/*emit*/ ready();
	}
}

NVODStream::NVODStream(eListBox<NVODStream> *listbox, eDVBNamespace dvb_namespace, const NVODReferenceEntry *ref, int type)
	: eListBoxEntryTextStream((eListBox<eListBoxEntryTextStream>*)listbox),
		service(dvb_namespace, eTransportStreamID(ref->transport_stream_id), eOriginalNetworkID(ref->original_network_id),
			eServiceID(ref->service_id), 5), eit(EIT::typeNowNext, ref->service_id, type)
{
	CONNECT(eit.tableReady, NVODStream::EITready);
	listbox->take(this);
	begTime=0;
	eit.start();
}

const eString &NVODStream::redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state )
{
	if (begTime && (begTime = validate()) )
		return eListBoxEntryTextStream::redraw(rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state);

	listbox->take( this );
	eit.start();

	static eString ret;
	return ret = _("not valid!");
}

void NVODStream::selfDestroy()
{
	if (!begTime)
		delete this;
}

void eNVODSelector::selected(NVODStream* nv)
{
	if (nv)
		eServiceInterface::getInstance()->play(nv->service);

	close(0);
}

eNVODSelector::eNVODSelector()
	:eListBoxWindow<NVODStream>(_("NVOD"), 10, 440), count(0)
{
	valign();
	list.setActiveColor(eSkin::getActive()->queryScheme("eServiceSelector.highlight.background"), eSkin::getActive()->queryScheme("eServiceSelector.highlight.foreground"));
	CONNECT(list.selected, eNVODSelector::selected);
}

void eNVODSelector::clear()
{
	count=0;
	list.clearList();
	/*emit*/ clearEntrys();
}

struct findNVOD
{
	NVODStream **stream;
	time_t nowTime;

	findNVOD( NVODStream** str )
		:stream( str ), nowTime( eDVB::getInstance()->time_difference + time(0) )
	{
	}

	bool operator()(NVODStream& str)
	{
		if ( *stream )
		{
			if ( abs( (*stream)->getBegTime() - nowTime ) > abs( str.getBegTime() - nowTime ) )
				*stream = &str;
		}
		else
			*stream = &str;
		return false;
	}
};

void eNVODSelector::readyCallBack( )
{
	if ( count )
	{
		count--;
		if ( !count )
		{
			NVODStream *select=0;
			list.forEachEntry( findNVOD( &select ) );
			if ( select )
				eServiceInterface::getInstance()->play( select->service );
		}
	}
}

void eNVODSelector::add(eDVBNamespace dvb_namespace, NVODReferenceEntry *ref)
{
	eServiceReference &s=eServiceInterface::getInstance()->service;
	if (s.type != eServiceReference::idDVB)
		return ;
	eServiceReferenceDVB &service=(eServiceReferenceDVB&)s;

	int type= ((service.getTransportStreamID()==eTransportStreamID(ref->transport_stream_id))
			&&	(service.getOriginalNetworkID()==eOriginalNetworkID(ref->original_network_id))) ? EIT::tsActual:EIT::tsOther;
	count++;
	NVODStream *nvod = new NVODStream(&list, dvb_namespace, ref, type);
	CONNECT( nvod->ready, eNVODSelector::readyCallBack );
	clearEntrys.connect( slot( *nvod, &NVODStream::selfDestroy) );
}

struct selectCurVideoStream
{
	int pid;
	eListBox<eListBoxEntryText> &lb;
	selectCurVideoStream(int pid, eListBox<eListBoxEntryText> &lb )
		:pid(pid), lb(lb)
	{
	}

	bool operator()(eListBoxEntryText& stream)
	{
		if ( ((PMTEntry*)stream.getKey())->elementary_PID == pid )
		{
			lb.setCurrent( &stream );
			return true;
		}
		return false;
	}
};

int eVideoSelector::eventHandler(const eWidgetEvent &e)
{
	switch (e.type)
	{
		case eWidgetEvent::execBegin:
			list.forEachEntry(selectCurVideoStream(Decoder::current.vpid, list));
			return 1;
		default:
			break;
	}
	return eWindow::eventHandler(e);
}

void eVideoSelector::selected(eListBoxEntryText *l)
{
	eServiceHandler *service=eServiceInterface::getInstance()->getService();

	if (l && service)
		service->setPID((PMTEntry*)l->getKey());

	close(0);
}

eVideoSelector::eVideoSelector()
	:eListBoxWindow<eListBoxEntryText>(_("Video"), 10, 330)
{
	valign();
	CONNECT(list.selected, eVideoSelector::selected);
}

void eVideoSelector::clear()
{
	list.clearList();
}

void eVideoSelector::add(PMTEntry *stream)
{
	list.beginAtomic();
	new eListBoxEntryText(&list,
		eString().sprintf("PID %04x", stream->elementary_PID),
		(void*)stream );
	list.endAtomic();
}

struct updateAudioStream
{
	std::list<eDVBServiceController::audioStream> &astreams;
	updateAudioStream(std::list<eDVBServiceController::audioStream> &astreams)
		:astreams(astreams)
	{
	}

	bool operator()(AudioStream& stream)
	{
		for (std::list<eDVBServiceController::audioStream>::iterator it(astreams.begin()); it != astreams.end(); ++it )
		{
			if (it->pmtentry->elementary_PID == stream.stream.pmtentry->elementary_PID )
			{
				stream.stream.text = it->text;
				stream.update();
				break;
			}
		}
		return false;
	}
};

struct selectCurAudioStream
{
	int pid;
	eListBox<AudioStream> &lb;
	selectCurAudioStream( int pid, eListBox<AudioStream> &lb )
		:pid(pid), lb(lb)
	{
	}

	bool operator()(AudioStream& stream)
	{
		if ( stream.stream.pmtentry->elementary_PID == pid )
		{
			lb.setCurrent( &stream );
			return true;
		}
		return false;
	}
};

struct selectCurSubtitleStream
{
	int pid;
	eListBox<eListBoxEntryText> &lb;
	selectCurSubtitleStream( int pid, eListBox<eListBoxEntryText> &lb )
		:pid(pid), lb(lb)
	{
	}

	bool operator()(eListBoxEntryText& stream)
	{
		//PMTEntry *e = (PMTEntry*)stream.getKey();
		//if ( (e && e->elementary_PID == pid) || (!e && pid == -1) )
		int k = (int)stream.getKey();
		if ( (k == pid) || (!k && pid == -1) )
		{
			lb.setCurrent( &stream );
			return true;
		}
		return false;
	}
};

AudioStream::AudioStream(eListBox<AudioStream> *listbox, eDVBServiceController::audioStream &stream)
	:eListBoxEntryText((eListBox<eListBoxEntryText>*)listbox, stream.text)
	,stream(stream.pmtentry)
{
}

void AudioStream::update()
{
	text=stream.text;
	if ( para )
	{
		para->destroy();
		para=0;
		listbox->invalidateContent();
	}
}

void ePSAudioSelector::selected(eListBoxEntryText*l)
{
	if (l)
	{
		if ( l->isSelectable() == 3 )
		{
			eServiceHandler *service=eServiceInterface::getInstance()->getService();
			if (l && service)
			{
				eDebug("SET %02x", (int)l->getKey() );
				service->setAudioStream((int)l->getKey());
			}
			if ( in_loop )
				close(0);
		}
	}
	else
		close(0);
}

void AudioChannelSelectionChanged( eListBoxEntryText *e )
{
	if ( e )
	{
		eService *sp=eServiceInterface::getInstance()->addRef(eServiceInterface::getInstance()->service);
		if (sp)
		{
			if (sp->dvb)
			{
				int val = (int)e->getKey();
				if (val < 0 /* left */ || val > 2 /* right */ || val == 1 /* stereo is default */)
					val = -1; // remove from cache
				sp->dvb->set(eServiceDVB::cStereoMono, val);
			}
			eServiceInterface::getInstance()->removeRef(eServiceInterface::getInstance()->service);
		}
		eAVSwitch::getInstance()->selectAudioChannel((int)e->getKey());
	}
}

ePSAudioSelector::ePSAudioSelector()
	:eListBoxWindow<eListBoxEntryText>(_("Audio"), 10, 330)
{
	init_ePSAudioSelector();
}
void ePSAudioSelector::init_ePSAudioSelector()
{
	valign();
	CONNECT(list.selected, ePSAudioSelector::selected);

	list.setFlags( eListBoxBase::flagLostFocusOnFirst );
	m_stereo_mono = new eListBox<eListBoxEntryText>(this);
	m_stereo_mono->loadDeco();
	m_stereo_mono->move(list.getPosition());
	m_stereo_mono->resize(eSize(getClientSize().width()-20, 35));
	m_stereo_mono->setFlags( eListBoxBase::flagNoUpDownMovement );

	ePtrList<eWidget> *focuslist = getTLW()->focusList();
	focuslist->remove(m_stereo_mono);
	focuslist->push_front(m_stereo_mono);

	new eListBoxEntryText(m_stereo_mono, _("   Left Mono  >"), (void*) 0, (int)eTextPara::dirCenter );
	new eListBoxEntryText(m_stereo_mono, _("<  Stereo  >"), (void*) 1, (int)eTextPara::dirCenter );
	new eListBoxEntryText(m_stereo_mono, _("<  Right Mono  "), (void*) 2, (int)eTextPara::dirCenter );
	m_stereo_mono->selchanged.connect( slot( AudioChannelSelectionChanged ) );
	ePoint p(0,40);
	list.move(m_stereo_mono->getPosition()+p);

#if 0
	if ( 0 && eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7000 )
	{
		list.resize(eSize(getClientSize().width()-20, getClientSize().height()-105));
		m_dyncfg = new eAudioDynamicConfig(this);
		m_dyncfg->move(ePoint(10, getClientSize().height()-60));
		m_dyncfg->resize(eSize(getClientSize().width()-20, 50));
	}
	else
#endif
		list.resize(eSize(getClientSize().width()-20, getClientSize().height()-40));
}

void ePSAudioSelector::clear()
{
	list.clearList();
//	m_stereo_mono->moveSelection(eListBoxBase::dirFirst,false);
//	m_stereo_mono->goNext();
}

void ePSAudioSelector::add(unsigned int id)
{
	eDebug("add AUDIO %02x",id);
	list.beginAtomic();
	new eListBoxEntryText(&list, (id&0xFF)==0xBD ?
		_("Audiotrack(AC3)") : _("Audiotrack"), (void*)id );
	list.endAtomic();
}


int ePSAudioSelector::eventHandler(const eWidgetEvent &e)
{
	switch (e.type)
	{
		case eWidgetEvent::execBegin:
		{
			setFocus(&list);
			int curSel = (int)m_stereo_mono->getCurrent()->getKey();
			int cur = eAVSwitch::getInstance()->getAudioChannel();
			while ( cur != curSel )
			{
				m_stereo_mono->moveSelection(
					curSel > cur ? eListBoxBase::dirUp : eListBoxBase::dirDown
					,false );
				curSel = (int)m_stereo_mono->getCurrent()->getKey();
			}
			return 1;
		}
		default:
			break;
	}
	return eWindow::eventHandler(e);
}

void eAudioSelector::subtitleSelected(eListBoxEntryText *entry)
{
	if (!entry)
		return;
	eServiceHandler *service=eServiceInterface::getInstance()->getService();
	if (service)
	{
		//PMTEntry *pe = (PMTEntry*)entry->getKey();
		int k = (int)entry->getKey();
		eSubtitleWidget *i = eSubtitleWidget::getInstance();
		if (!i)
			return;
		eService *sp=eServiceInterface::getInstance()->addRef(eServiceInterface::getInstance()->service);
		if (sp)
		{
			if (sp->dvb)
			{
				//int val = (pe ? pe->elementary_PID : -1);
				int val1 = (k > 0 ? k : -1);
				int val2 = (k < 0 ? -k : -1);
				sp->dvb->set(eServiceDVB::cSubtitle, val1);
				sp->dvb->set(eServiceDVB::cTTXSubtitle, val2);
			}
			eServiceInterface::getInstance()->removeRef(eServiceInterface::getInstance()->service);
		}
		//if (pe)
		if (k> 0)
		{
			std::set<int> pages; pages.insert(-1);
			//i->start(pe->elementary_PID, pages);
			i->start(k, pages);

		} else if (k < 0)
		{
			i->startttx(-k);
		} else
			i->stop();
	}
	close(0);
}

void eAudioSelector::update(std::list<eDVBServiceController::audioStream>& lst)
{
	list.forEachEntry(updateAudioStream(lst));
}

extern eString getISO639Description(char *iso);

void eAudioSelector::addSubtitle(const PMTEntry *entry)
{
	m_subtitles->show();
	list.setFlags( eListBoxBase::flagLostFocusOnLast );

	eString description;
	description.sprintf("PID %04x", entry->elementary_PID);

	// TODO : Support PIDs with more than on composition page_id..
	// the Subtitling Descriptor is ready for this..
	// at moment we use only the iso639 descriptor from the first entry
	// in the entries list..

	for (ePtrList<Descriptor>::const_iterator ii(entry->ES_info); ii != entry->ES_info.end(); ++ii)
	{
		switch (ii->Tag())
		{
/*
		case DESCR_ISO639_LANGUAGE:
			description=getISO639Description(((ISO639LanguageDescriptor*)*ii)->language_code);
			break;*/
		case DESCR_SUBTITLING:
			SubtitlingDescriptor *descr = (SubtitlingDescriptor*)*ii;
			if (descr->entries.size())
			{
				description=getISO639Description(descr->entries.front()->language_code);
				goto end;
			}
			break;
		}

	}
end:
	//new eListBoxEntryText(m_subtitles, description, (void*)entry, eTextPara::dirCenter );
	new eListBoxEntryText(m_subtitles, description, (void*)entry->elementary_PID, eTextPara::dirCenter );
}
struct RemoveTTXSubtitle
{
	eListBox<eListBoxEntryText> &lb;
	RemoveTTXSubtitle(eListBox<eListBoxEntryText> &lb )
		:lb(lb)
	{
	}

	bool operator()(eListBoxEntryText& stream)
	{
		int k = (int)stream.getKey();
		if (k<0)
		{
			lb.take( &stream );
			return true;
		}
		return false;
	}
};
void eAudioSelector::addTTXSubtitles()
{
#ifndef TUXTXT_CFG_STANDALONE
	const eString countries[] ={
		"",   /*  0 no subset specified */
		"CS/SK",   /*  1 czech, slovak */
		"EN",   /*  2 english */
		"ET",   /*  3 estonian */
		"FR",   /*  4 french */
		"DE",   /*  5 german */
		"IT",   /*  6 italian */
		"LV/LT",   /*  7 latvian, lithuanian */
		"PL",   /*  8 polish */
		"PT/ES",   /*  9 portuguese, spanish */
		"RO",   /* 10 romanian */
		"SR/HR/SL",   /* 11 serbian, croatian, slovenian */
		"SV/FI/HU",   /* 12 swedish, finnish, hungarian */
		"TR",   /* 13 turkish */
		"RU/BUL/SER/CRO/UKR",   /* 14 cyrillic */
		"EK"   /* 15 greek */
	};
	while (m_subtitles->forEachEntry(RemoveTTXSubtitle(*m_subtitles)) == eListBoxBase::OK);
	for (int i = 0; i < 8; i++)
	{
		int page = tuxtxt_cache.subtitlepages[i].page;
		if (page)
		{
			m_subtitles->show();
			list.setFlags( eListBoxBase::flagLostFocusOnLast );
			eString description;
			description.sprintf(_("Teletext Page %03x (%s)"), page,countries[tuxtxt_cache.subtitlepages[i].language].c_str());

			new eListBoxEntryText(m_subtitles, description, (void*)-page, eTextPara::dirCenter );
		}
	}
#endif
}

int eAudioSelector::eventHandler(const eWidgetEvent &e)
{
	switch (e.type)
	{
		case eWidgetEvent::execBegin:
		{
			if (Decoder::current.tpid != -1)
				addTTXSubtitles();
			list.forEachEntry(selectCurAudioStream(Decoder::current.apid, list));
			m_subtitles->forEachEntry(selectCurSubtitleStream(eSubtitleWidget::getInstance()->getCurPid(), *m_subtitles ));
			setFocus(&list);
			int curSel = (int)m_stereo_mono->getCurrent()->getKey();
			int cur = eAVSwitch::getInstance()->getAudioChannel();
			while ( cur != curSel )
			{
				m_stereo_mono->moveSelection(
					curSel > cur ? eListBoxBase::dirUp : eListBoxBase::dirDown
					,false );
				curSel = (int)m_stereo_mono->getCurrent()->getKey();
			}
			return 1;
		}
		default:
			break;
	}
	return eWindow::eventHandler(e);
}

void eAudioSelector::selected(AudioStream *l)
{
	if (l)
	{
		if ( l->isSelectable() == 3 )
		{
			eServiceHandler *service=eServiceInterface::getInstance()->getService();
			if (l && service)
				service->setPID(l->stream.pmtentry);
			close(0);
		}
	}
	else
		close(0);
}

eAudioSelector::eAudioSelector()
	:eListBoxWindow<AudioStream>(_("Audio"), 10, 330)
{
	init_eAudioSelector();
}
void eAudioSelector::init_eAudioSelector()
{
	valign();
	CONNECT(list.selected, eAudioSelector::selected);

	list.setFlags( eListBoxBase::flagLostFocusOnFirst );
	m_stereo_mono = new eListBox<eListBoxEntryText>(this);
	m_stereo_mono->loadDeco();
	m_stereo_mono->move(list.getPosition());
	m_stereo_mono->resize(eSize(getClientSize().width()-20, 35));
	m_stereo_mono->setFlags( eListBoxBase::flagNoUpDownMovement );

	ePtrList<eWidget> *focuslist = getTLW()->focusList();
	focuslist->remove(m_stereo_mono);
	focuslist->push_front(m_stereo_mono);

	new eListBoxEntryText(m_stereo_mono, _("   Left Mono  >"), (void*) 0, (int)eTextPara::dirCenter );
	new eListBoxEntryText(m_stereo_mono, _("<  Stereo  >"), (void*) 1, (int)eTextPara::dirCenter );
	new eListBoxEntryText(m_stereo_mono, _("<  Right Mono  "), (void*) 2, (int)eTextPara::dirCenter );
	m_stereo_mono->selchanged.connect( slot( AudioChannelSelectionChanged ) );
	ePoint p(0,40);
	list.move(m_stereo_mono->getPosition()+p);

	m_subtitles = new eListBox<eListBoxEntryText>(this);
	m_subtitles->loadDeco();
	CONNECT(m_subtitles->selected, eAudioSelector::subtitleSelected);

#if 0
	if ( 0 && eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7000 )
	{
		list.resize(eSize(getClientSize().width()-20, getClientSize().height()-140));
		m_dyncfg = new eAudioDynamicConfig(this);
		m_dyncfg->move(ePoint(10, getClientSize().height()-60));
		m_dyncfg->resize(eSize(getClientSize().width()-20, 50));
		m_subtitles->move(ePoint(10, getClientSize().height()-100));
	}
	else
#endif
	{
		list.resize(eSize(getClientSize().width()-20, getClientSize().height()-80));
		m_subtitles->move(ePoint(10, getClientSize().height()-40));
	}

	m_subtitles->resize(eSize(getClientSize().width()-20, 35));
	m_subtitles->setShortcut("green");
	m_subtitles->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
}

void eAudioSelector::clear()
{
	list.clearList();
//	m_stereo_mono->moveSelection(eListBoxBase::dirFirst,false);
//	m_stereo_mono->goNext();
	m_subtitles->clearList();
	m_subtitles->hide();
	list.removeFlags( eListBoxBase::flagLostFocusOnLast );
	new eListBoxEntryText(m_subtitles, _("no subtitles"), 0, eTextPara::dirCenter );
}

void eAudioSelector::add(eDVBServiceController::audioStream &pmt)
{
	list.beginAtomic();
	new AudioStream(&list, pmt);
	list.endAtomic();
}

SubService::SubService(eListBox<SubService> *listbox, eDVBNamespace dvb_namespace, const LinkageDescriptor *descr)
	:eListBoxEntryText((eListBox<eListBoxEntryText>*) listbox),
		service(dvb_namespace,
			eTransportStreamID(descr->transport_stream_id),
			eOriginalNetworkID(descr->original_network_id),
			eServiceID(descr->service_id), 7)
{
	service.descr=text=convertDVBUTF8(descr->private_data, descr->priv_len);
	eTransponderList::getInstance()->createSubService(service);
}

struct selectCurSubService
{
	eServiceReferenceDVB& cur;
	eListBox<SubService> &lb;
	selectCurSubService( eServiceReferenceDVB& cur, eListBox<SubService> &lb )
		:cur(cur), lb(lb)
	{
	}

	bool operator()(SubService& service)
	{
		if ( service.service == cur )
		{
			lb.setCurrent( &service );
			return true;
		}
		return false;
	}
};

eSubServiceSelector::eSubServiceSelector( bool showbuttons )
	:eListBoxWindow<SubService>(_("multiple Services"), 10, 530),
	quickzap(0)
{
	init_eSubServiceSelector(showbuttons);
}
void eSubServiceSelector::init_eSubServiceSelector(bool showbuttons)
{
	cresize( eSize( getClientSize().width(), getClientSize().height()+80 ) );
	valign();

	if ( showbuttons )
	{
		bToggleQuickZap = new eButton( this );
		bToggleQuickZap->resize( eSize( getClientSize().width()-20, 30 ) );
		bToggleQuickZap->move( ePoint( 10, getClientSize().height()-70 ) );
		bToggleQuickZap->setText(_("enable quickzap"));
		bToggleQuickZap->setShortcut("green");
		bToggleQuickZap->setShortcutPixmap("green");
		bToggleQuickZap->show();

		bAddToUserBouquet = new eButton( this );
		bAddToUserBouquet->resize( eSize( getClientSize().width()-20, 30 ) );
		bAddToUserBouquet->move( ePoint( 10, getClientSize().height()-40 ) );
		bAddToUserBouquet->setText(_("add to bouquet"));
		bAddToUserBouquet->setShortcut("yellow");
		bAddToUserBouquet->setShortcutPixmap("yellow");
		bAddToUserBouquet->show();

		CONNECT(bAddToUserBouquet->selected, eSubServiceSelector::addPressed );
		CONNECT(bToggleQuickZap->selected, eSubServiceSelector::quickZapPressed );
	}
	CONNECT(list.selected, eSubServiceSelector::selected);
}

void eSubServiceSelector::addPressed()
{
	if ( list.getCount() )
	{
		hide();
		list.getCurrent()->service.setServiceType(1);
		/* emit */ addToUserBouquet( &list.getCurrent()->service, 0 );
		list.getCurrent()->service.setServiceType(7);
		show();
	}
}

bool eSubServiceSelector::quickzapmode()
{
	if ( eActionMapList::getInstance()->getCurrentStyles().find("classic")
		!= eActionMapList::getInstance()->getCurrentStyles().end() )
		return true;
	else if ( quickzap )
		return true;
	else
		return false;
}

void eSubServiceSelector::quickZapPressed()
{
	quickzap ^= 1;
	if ( quickzap )
	{
		bToggleQuickZap->setText(_("disable quickzap"));
	}
	else
	{
		bToggleQuickZap->setText(_("enable quickzap"));
	}
	close(-1);
}

void eSubServiceSelector::selected(SubService *ss)
{
	if (ss)
		close(0);
}

void eSubServiceSelector::disableQuickZap()
{
	quickzap=0;
	bToggleQuickZap->setText(_("enable quickzap"));
}

void eSubServiceSelector::clear()
{
	list.clearList();
}

void eSubServiceSelector::selectCurrent()
{
	list.forEachEntry( selectCurSubService( (eServiceReferenceDVB&)eServiceInterface::getInstance()->service, list ) );
}

void eSubServiceSelector::add(eDVBNamespace dvb_namespace, const LinkageDescriptor *ref)
{
	list.beginAtomic();
	new SubService(&list, dvb_namespace, ref);
	list.endAtomic();
}

void eSubServiceSelector::willShow()
{
	selectCurrent();
	eWindow::willShow();
}

void eSubServiceSelector::next()
{
	selectCurrent();
	if ( !list.goNext() )
		list.moveSelection( eListBox<SubService>::dirFirst );
	play();
}

void eSubServiceSelector::prev()
{
	selectCurrent();
	if ( !list.goPrev() )
		list.moveSelection( eListBox<SubService>::dirLast );
	play();
}

extern bool onSameTP( const eServiceReferenceDVB& ref1, const eServiceReferenceDVB &ref2 ); // implemented in timer.cpp
extern bool canPlayService( const eServiceReference & ref ); // implemented in timer.cpp

void eSubServiceSelector::play()
{
	SubService* ss = list.getCurrent();
	if (ss)
	{
		eServiceReferenceDVB &ref=ss->service;
#ifndef DISABLE_FILE
		if ( !canPlayService(ref) && !eZapMain::getInstance()->handleState() )
			return;
#else
		if ( !eZapMain::getInstance()->handleState() )
#endif
			eServiceInterface::getInstance()->play(ref);
	}
}

void eServiceNumberWidget::selected(int *res)
{
	timer->stop();
	if (!res)
	{
		close(-1);
		return;
	}
	close(number->getNumber());
}

void eServiceNumberWidget::timeout()
{
	close(number->getNumber());
}

eServiceNumberWidget::eServiceNumberWidget(int initial)
										:eWindow(0)
{
	init_eServiceNumberWidget(initial);
}
void eServiceNumberWidget::init_eServiceNumberWidget(int initial)
{
	setText(_("Channel"));
	resize(eSize(280, 120));
	valign();
	eLabel *label;
	label=new eLabel(this);
	label->setText(_("Channel:"));
	label->move(ePoint(50, 15));
	label->resize(eSize(110, eSkin::getActive()->queryValue("fontsize", 20)+4));

	number=new eNumber(this, 1, 1, 9999, 4, 0, 1, label);
	number->move(ePoint(160, 15));
	number->resize(eSize(60, eSkin::getActive()->queryValue("fontsize", 20)+4));
	number->setNumber(initial);
	CONNECT(number->selected, eServiceNumberWidget::selected);
	CONNECT(number->numberChanged, eServiceNumberWidget::numberChanged );

	timer=new eTimer(eApp);
	timer->start(2000,true);
	CONNECT(timer->timeout, eServiceNumberWidget::timeout);
}

eServiceNumberWidget::~eServiceNumberWidget()
{
	if (timer)
		delete timer;
}

void eServiceNumberWidget::numberChanged()
{
	timer->start(2000,true);
}

eZapMain *eZapMain::instance;

bool eZapMain::CheckService( const eServiceReference& ref )
{
	if ( mode == modeFile )
		return ref.path.length();
	else if ( ref.type == eServiceReference::idDVB )
	{
		switch( mode )
		{
			case modeTV:
				return ref.data[0] == 1 || ref.data[0] == 4 || ref.data[0] == 6;
			case modeRadio:
				return ref.data[0] == 2;
		}
	}
	return false;
}

static bool ModeTypeEqual( const eServiceReference& ref1, const eServiceReference& ref2 )
{
	if ( (ref1.path.length()>0) == (ref2.path.length()>0) )
	{
		if ( ref1.path.length() )
			return true;   // booth are mp3 or rec ts
		else if ( ref1.type == eServiceReference::idDVB && ref2.type == eServiceReference::idDVB )
		{
			if ( ref1.data[0] == ref2.data[0] )
				return true;  // have self types..
			else if ( ref1.data[0] & 1 && ref2.data[0] & 1 )
				return true;  // nvod, linkage, tv, nvodref
			else if ( ref1.data[0] == 4 && ref2.data[0] & 1 )
				return true;  // nvod, linkage, tv, nvodref
			else if ( ref1.data[0] & 1 && ref2.data[0] == 4 )
				return true;  // nvod, linkage, tv, nvodref
			else if ( ref1.data[0] == 6 && ref2.data[0] & 1 )
				return true;  // mosaic
			else if ( ref1.data[0] & 1 && ref2.data[0] == 6 )
				return true;  //  mosaic
		}
	}
	return false;
}

void eZapMain::onRotorStart( int newPos )
{
	if (!pRotorMsg)
	{
		pRotorMsg = new eMessageBox( eString().sprintf(_("Please wait while the motor is turning to %d.%d\xC2\xB0%c ...."),abs(newPos)/10,abs(newPos)%10,newPos<0?'W':'E'), _("Message"), 0);
		pRotorMsg->zOrderRaise();
		pRotorMsg->show();
	}
}

void eZapMain::onRotorStop()
{
	if (pRotorMsg)
	{
		pRotorMsg->hide();
		delete pRotorMsg;
		pRotorMsg=0;
	}
}

void eZapMain::onRotorTimeout()
{
	if (pRotorMsg)
	{
		pRotorMsg->hide();
		delete pRotorMsg;
		pRotorMsg=0;
	}
/*	pRotorMsg = new eMessageBox( _("Rotor has timeouted... check your LNB Cable, or if you use rotor drive speed for running detection then decrease the \xC2\xB0/sec value")
									, _("Message"),
									eMessageBox::btOK|eMessageBox::iconInfo);
	pRotorMsg->show();
	timeout.start(10000, true);*/
}

void eZapMain::eraseBackground(gPainter *painter, const eRect &where)
{
	(void)painter;
	(void)where;
}

#ifndef DISABLE_FILE
void eZapMain::saveRecordings( bool destroy )
{
	// save and destroy recordingslist

	/*
	 * If the harddrive is asleep, it will wake up now.
	 * That might take some time, and we don't want to receive
	 * RC repeat events during that time.
	 */
	eRCInput::getInstance()->lock();
	recordings->save();
	::sync();
	eRCInput::getInstance()->unlock();
	if (destroy)
	{
		eServiceInterface::getInstance()->removeRef(recordingsref);
		eServicePlaylistHandler::getInstance()->removePlaylist(recordingsref);
		recordings=0;
		recordingsref=eServiceReference();
	}
}
#endif

void eZapMain::savePlaylist(bool destroy)
{
	// save and destroy playlist
	playlist->save();
	if (destroy)
	{
		eServiceInterface::getInstance()->removeRef(playlistref);
		eServicePlaylistHandler::getInstance()->removePlaylist(playlistref);
		playlist=0;
		playlistref=eServiceReference();
	}
}

void eZapMain::loadPlaylist( bool create )
{
	// create Playlist
	if (create)
	{
		eServicePlaylistHandler::getInstance()->addNum( 0 );
		playlistref=eServiceReference( eServicePlaylistHandler::ID,
			eServiceReference::isDirectory, 0, 0 );
		playlist=(ePlaylist*)eServiceInterface::getInstance()->addRef(playlistref);
		ASSERT(playlist);
		eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeRoot), playlistref);
	}
	playlist->load((eplPath+"/playlist.epl").c_str());
	if ( !create && eZap::getInstance()->getServiceSelector()->getPath().current() == playlistref )
		eZap::getInstance()->getServiceSelector()->actualize();
}

#ifndef DISABLE_FILE
void eZapMain::loadRecordings( bool create )
{
	if ( create )
	{
		// create recordingslist..
		eServicePlaylistHandler::getInstance()->addNum( 1 );
		recordingsref=eServiceReference( eServicePlaylistHandler::ID, eServiceReference::flagDirectory, 0, 1);
		recordings=(ePlaylist*)eServiceInterface::getInstance()->addRef(recordingsref);
		ASSERT(recordings);
		recordings->lockPlaylist();
		eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeFile), recordingsref);
		recordings->service_name=_("Recorded movies");
		recordings->unlockPlaylist();
	}
	if (!create)
	{
		eString movieLocation;
	
		eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaMovies, movieLocation);
		recordings->load((movieLocation + "/recordings.epl").c_str());
	}
	if ( !create && eZap::getInstance()->getServiceSelector()->getPath().current() == recordingsref )
		eZap::getInstance()->getServiceSelector()->actualize();
}

void eZapMain::clearRecordings()
{
	recordings->clear();
	saveRecordings();
}
#endif

void eZapMain::addServiceToLastScannedUserBouquet (const eServiceReference &service, int service_type, int services_scanned, bool newService) 
{ 
	eServiceReference parentRef;
	ePlaylist* parentList=0;
	
	switch(service_type)
	{
		case 1: //TV
			parentList = userTVBouquets;
			parentRef = userTVBouquetsRef;
			break;
		case 2: //Radio
			parentList = userRadioBouquets;
			parentRef = userRadioBouquetsRef;
			break;
		default://Other stored as file
			parentList = userFileBouquets;
			parentRef = userFileBouquetsRef;
			break;
	}
	
	if ( services_scanned == 1 )
	{
		for (std::list<ePlaylistEntry>::iterator it(userTVBouquets->getList().begin()); it != userTVBouquets->getList().end(); it++ )
		{
			ePlaylist *p = (ePlaylist*) eServiceInterface::getInstance()->addRef(it->service);
			if (p)
			{
				if ( p->service_name == "Last Scanned" )
				{
					p->getList().clear();
				}
			}
		}
		for (std::list<ePlaylistEntry>::iterator it(userRadioBouquets->getList().begin()); it != userRadioBouquets->getList().end(); it++ )
		{
			ePlaylist *p = (ePlaylist*) eServiceInterface::getInstance()->addRef(it->service);
			if (p)
			{
				if ( p->service_name == "Last Scanned" )
				{
					p->getList().clear();
				}
			}
		}
		for (std::list<ePlaylistEntry>::iterator it(userFileBouquets->getList().begin()); it != userFileBouquets->getList().end(); it++ )
		{
			ePlaylist *p = (ePlaylist*) eServiceInterface::getInstance()->addRef(it->service);
			if (p)
			{
				if ( p->service_name == "Last Scanned" )
				{
					p->getList().clear();
				}
			}
		}
	}
	for (std::list<ePlaylistEntry>::iterator it(parentList->getList().begin()); it != parentList->getList().end(); it++ )
	{
		ePlaylist *p = (ePlaylist*) eServiceInterface::getInstance()->addRef(it->service);
		if ( p )
		{
			if ( p->service_name == "Last Scanned" )
			{
				if (newService)
				{
					p->getList().push_front(service);
				}
				else
				{
					p->getList().push_back(service);
				}
				return;
			}
		}
	}

	eString basePath=eplPath;
	if ( eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000 )
	{
		if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feCable )
			basePath+="/cable";
		if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feTerrestrial )
			basePath+="/terrestrial";
	}
	eServiceReference newList = eServicePlaylistHandler::getInstance()->newPlaylist(); 
	switch(service_type) {
		case 1: //TV
			addUserBouquet( parentRef, parentList, basePath+'/'+eString().sprintf("userbouquet.%x.tv",newList.data[1]), "Last Scanned", newList, true ); 
			break;
		case 2: //Radio
			addUserBouquet( parentRef, parentList, basePath+'/'+eString().sprintf("userbouquet.%x.radio",newList.data[1]), "Last Scanned", newList, true );
		        break;
		default://Other stored as file		
			addUserBouquet( parentRef, parentList, basePath+'/'+eString().sprintf("userbouquet.%x.file",newList.data[1]), "Last Scanned", newList, true );
			break;
	}		
	ePlaylist *p = (ePlaylist*)eServiceInterface::getInstance()->addRef( newList ); 
	p->getList().push_back(service); 
}

void eZapMain::saveUserBouquets()
{
	userTVBouquets->save();
	for (std::list<ePlaylistEntry>::iterator it(userTVBouquets->getList().begin()); it != userTVBouquets->getList().end(); it++ )
	{
		ePlaylist *p = (ePlaylist*) eServiceInterface::getInstance()->addRef( it->service );
		if ( p )
		{
			p->save();
			eServiceInterface::getInstance()->removeRef( it->service );
		}
	}

	userRadioBouquets->save();
	for (std::list<ePlaylistEntry>::iterator it(userRadioBouquets->getList().begin()); it != userRadioBouquets->getList().end(); it++ )
	{
		ePlaylist *p = (ePlaylist*) eServiceInterface::getInstance()->addRef( it->service );
		if ( p )
		{
			p->save();
			eServiceInterface::getInstance()->removeRef( it->service );
		}
	}

	userFileBouquets->save();
	for (std::list<ePlaylistEntry>::iterator it(userFileBouquets->getList().begin()); it != userFileBouquets->getList().end(); it++ )
	{
		ePlaylist *p = (ePlaylist*) eServiceInterface::getInstance()->addRef( it->service );
		if ( p )
		{
			p->save();
			eServiceInterface::getInstance()->removeRef( it->service );
		}
	}
}

void eZapMain::destroyUserBouquets( bool save )
{
	// destroy all userBouquets
	if (save)
	{
		userTVBouquets->save();
		userRadioBouquets->save();
		userFileBouquets->save();
		saveUserBouquets();
	}

	// save and destroy userTVBouquetList
	for (std::list<ePlaylistEntry>::iterator it(userTVBouquets->getList().begin()); it != userTVBouquets->getList().end(); it++ )
	{
		eServicePlaylistHandler::getInstance()->removePlaylist(it->service);
		eServiceInterface::getInstance()->removeRef(it->service);
	}

	eServiceInterface::getInstance()->removeRef(userTVBouquetsRef);
	eServicePlaylistHandler::getInstance()->removePlaylist(userTVBouquetsRef);
	userTVBouquets=0;
	userTVBouquetsRef=eServiceReference();

	// save and destroy userRadioBouquetList
	for (std::list<ePlaylistEntry>::iterator it(userRadioBouquets->getList().begin()); it != userRadioBouquets->getList().end(); it++ )
	{
		eServicePlaylistHandler::getInstance()->removePlaylist(it->service);
		eServiceInterface::getInstance()->removeRef(it->service);
	}

	eServiceInterface::getInstance()->removeRef(userRadioBouquetsRef);
	eServicePlaylistHandler::getInstance()->removePlaylist(userRadioBouquetsRef);
	userRadioBouquets=0;
	userRadioBouquetsRef=eServiceReference();

	// save and destroy userFileBouquetList
	for (std::list<ePlaylistEntry>::iterator it(userFileBouquets->getList().begin()); it != userFileBouquets->getList().end(); it++ )
	{
		eServicePlaylistHandler::getInstance()->removePlaylist(it->service);
		eServiceInterface::getInstance()->removeRef(it->service);
	}

	eServiceInterface::getInstance()->removeRef(userFileBouquetsRef);
	eServicePlaylistHandler::getInstance()->removePlaylist(userFileBouquetsRef);
	userFileBouquets=0;
	userFileBouquetsRef=eServiceReference();
}

void eZapMain::loadUserBouquets( bool destroy )
{
	if ( destroy )
		destroyUserBouquets();

	eString basePath=eplPath;
	if ( eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000 )
	{
		if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feCable )
			basePath+="/cable";
		if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feTerrestrial )
			basePath+="/terrestrial";
	}

	// create user bouquet tv list
	eServicePlaylistHandler::getInstance()->addNum( 6 );
	userTVBouquetsRef=eServiceReference( eServicePlaylistHandler::ID, eServiceReference::flagDirectory, 0, 6);
	userTVBouquets=(ePlaylist*)eServiceInterface::getInstance()->addRef(userTVBouquetsRef);
	ASSERT(userTVBouquets);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeRoot), userTVBouquetsRef);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeTV), userTVBouquetsRef);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeTvRadio), userTVBouquetsRef);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeBouquets), userTVBouquetsRef);
	userTVBouquets->service_name=_("Bouquets (TV)");
	userTVBouquets->load((basePath+"/userbouquets.tv.epl").c_str());

	// create user bouquet file list
	eServicePlaylistHandler::getInstance()->addNum( 3 );
	userFileBouquetsRef=eServiceReference( eServicePlaylistHandler::ID, eServiceReference::flagDirectory, 0, 3);
	userFileBouquets=(ePlaylist*)eServiceInterface::getInstance()->addRef(userFileBouquetsRef);
	ASSERT(userFileBouquets);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeRoot), userFileBouquetsRef);
//	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeFile), userFileBouquetsRef);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeBouquets), userFileBouquetsRef);
	userFileBouquets->service_name=_("Bouquets (File)");
	userFileBouquets->load((eplPath+"/userbouquets.file.epl").c_str());

	// create user bouquet radio list
	eServicePlaylistHandler::getInstance()->addNum( 4 );
	userRadioBouquetsRef=eServiceReference( eServicePlaylistHandler::ID, eServiceReference::flagDirectory, 0, 4);
	userRadioBouquets=(ePlaylist*)eServiceInterface::getInstance()->addRef(userRadioBouquetsRef);
	ASSERT(userRadioBouquets);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeRoot), userRadioBouquetsRef);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeRadio), userRadioBouquetsRef);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeTvRadio), userRadioBouquetsRef);
	eServicePlaylistHandler::getInstance()->newPlaylist(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeBouquets), userRadioBouquetsRef);
	userRadioBouquets->service_name=_("Bouquets (Radio)");
	userRadioBouquets->load((basePath+"/userbouquets.radio.epl").c_str());

	int i=0;
	for (int d = modeTV; d <= modeFile; d++)
	{
		eServiceReference parentRef;
		ePlaylist* parentList=0;
		switch(d)
		{
			case modeTV:
				parentList = userTVBouquets;
				parentRef = userTVBouquetsRef;
				break;
			case modeRadio:
				parentList = userRadioBouquets;
				parentRef = userRadioBouquetsRef;
				break;
			case modeFile:
				parentList = userFileBouquets;
				parentRef = userFileBouquetsRef;
				break;
		}
		for ( std::list<ePlaylistEntry>::iterator it(parentList->getList().begin()); it != parentList->getList().end(); it++, i++)
		{
			eServicePlaylistHandler::getInstance()->addNum( it->service.data[1] );
			eServiceReference ref = eServiceReference( eServicePlaylistHandler::ID, eServiceReference::flagDirectory, 0, it->service.data[1] );
			ref.path = it->service.path;
			(ePlaylist*)eServiceInterface::getInstance()->addRef(ref);
			eServicePlaylistHandler::getInstance()->newPlaylist(parentRef,ref);
		}
	}

	if(!i)  // no favlist loaded...
	{
		// create default user bouquet lists (favourite lists)
		for (int i = modeTV; i <= modeFile; i++)
		{
			eServiceReference ref = eServicePlaylistHandler::getInstance()->newPlaylist();
			ePlaylist* parentList=0;
			eServiceReference parentRef;
			eString path;
			eString name;
			switch(i)
			{
				case modeTV:
					parentList = userTVBouquets;
					parentRef = userTVBouquetsRef;
					path = basePath+'/'+eString().sprintf("userbouquet.%x.tv",ref.data[1]);
					name = _("Favourites (TV)");
					break;
				case modeRadio:
					parentList = userRadioBouquets;
					parentRef = userRadioBouquetsRef;
					path = basePath+'/'+eString().sprintf("userbouquet.%x.radio",ref.data[1]);
					name = _("Favourites (Radio)");
					break;
				case modeFile:
					parentList = userFileBouquets;
					parentRef = userFileBouquetsRef;
					path = eplPath+'/'+eString().sprintf("userbouquet.%x.file",ref.data[1]);
					name = _("Favourites (File)");
					break;
			}
			addUserBouquet( parentRef, parentList, path, name, ref, true );
		}
	}
	if ( destroy )
		eZap::getInstance()->getServiceSelector()->actualize();
}

void eZapMain::reloadPaths(int reset)
{
	// read for all modes last servicePaths from registry
	for (int m=modeTV; m < modeEnd; m++)
	{
		char* str;
		if ( !reset && !eConfig::getInstance()->getKey( eString().sprintf("/ezap/ui/modes/%d/path0", m).c_str(), str ) )
		{
			modeLast[m].setString(str);
//			eDebug(str);
			free(str);
		}
		else  // no path in registry... create default..
		{
			modeLast[m]=eServiceStructureHandler::getRoot(m+1);
			modeLast[m].down( eServiceReference() );
		}
	}

	// set serviceSelector style
	int style;
	if (reset || eConfig::getInstance()->getKey("/ezap/ui/serviceSelectorStyle", style ) )
		style=eServiceSelector::styleSingleColumn;  // default we use single Column Style

	eZap::getInstance()->getServiceSelector()->setStyle(style);

	if (reset)
	{
		int oldm=mode;
		mode=-1;
		setMode(oldm);
	}
}

int eZapMain::doHideInfobar()
{
	int iAutoHide = 1;
	eConfig::getInstance()->getKey("/ezap/osd/hideinradiomode", iAutoHide) ;
	if (iAutoHide)
		 return 1;
	eServiceReference &ref = eServiceInterface::getInstance()->service;
	if ( (ref.type == eServiceReference::idDVB /*&& ref.data[0] != 2*/ )
#ifndef DISABLE_FILE
		||
			(ref.type == eServiceReference::idUser &&
			ref.data[0] == eMP3Decoder::codecMPG )
#endif
		 )
		return 1;
	return 0;
}

eZapMain::eZapMain()
	:eWidget(0, 1)
	,mute( eZap::getInstance()->getDesktop( eZap::desktopFB ) )
	,volume( eZap::getInstance()->getDesktop( eZap::desktopFB ) )
	,validEITReceived(false)
	,pMsg(0), pRotorMsg(0)
	,message_notifier(eApp, 0)
//#ifndef DISABLE_CI
	,mmi_messages(eApp, 1)
//#endif
	,epg_messages(eApp, 1)
	,timeout(eApp)
	,clocktimer(eApp), messagetimeout(eApp), progresstimer(eApp)
	,volumeTimer(eApp), recStatusBlink(eApp), doubleklickTimer(eApp)
	,unusedTimer(eApp), permanentTimeshiftTimer(eApp), epgNowNextTimer(eApp), currentSelectedUserBouquet(0), timeshift(0)
	,standby_nomenu(0)
	,skipping(0)
	,state(0)
	,wasSleeping(0)
	,led_timer(0)
	,ledStatusBack(eApp)
	,AnalogNoSec(false)
{
	init_main();
}

void eZapMain::init_main()
{
	if (!instance)
		instance=this;

	int wasDeepstandby=0;
	eConfig::getInstance()->getKey("/ezap/timer/deepstandbywakeupset", wasDeepstandby );
	eConfig::getInstance()->delKey("/ezap/timer/deepstandbywakeupset");
	eConfig::getInstance()->flush();
	wasSleeping = wasDeepstandby ? 2 : 0;
	eDebug("[eZapMain]wasSleeping is %d, wasDeepStandby is %d", wasSleeping, wasDeepstandby);
	
// set tuner in outer FEC mode
	if (eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite) 
	{
		int berinfec = 0;
        	eConfig::getInstance()->getKey("/pli/ImprovedBER", berinfec);
		eFrontend::getInstance()->setBERMode(berinfec);
	}	

// get Infobar timeout
	if ((eConfig::getInstance()->getKey("/enigma/timeoutInfobar", timeoutInfobar)) != 0)
	{
		timeoutInfobar = 6;
		eConfig::getInstance()->setKey("/enigma/timeoutInfobar", timeoutInfobar);
	}

	if ((timeoutInfobar < 2) || (timeoutInfobar > 12))
	{
		if (timeoutInfobar < 2) timeoutInfobar = 2;
		if (timeoutInfobar > 12) timeoutInfobar = 12;
		eConfig::getInstance()->setKey("/enigma/timeoutInfobar", timeoutInfobar);
	}


	isVT=0;
	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "ezap_main"))
		eFatal("skin load of \"ezap_main\" failed");

// Mute Symbol
	gPixmap *pm = NULL;
	eString x, y;
	pm = skin->queryImage("mute.pixmap");
	if (!pm) 
	{
		/* still using old naming */
		pm = skin->queryImage("mute_symbol");
	}
	x = skin->queryValue("mute.pos.x", "0");
	y = skin->queryValue("mute.pos.y", "0");

	if (pm)
	{
		mute.setPixmap(pm);
		mute.setProperty("position", x + ":" + y);
		mute.resize( eSize( pm->x, pm->y ) );
		mute.setPixmapPosition(ePoint(0,0));
		mute.hide();
		mute.setBlitFlags( BF_ALPHATEST );
	}

// Volume Pixmap
	
	if ((pm = skin->queryImage("volume.background.pixmap")))
	{
		x = skin->queryValue("volume.background.pos.x", "0");
		y = skin->queryValue("volume.background.pos.y", "0");
	}
	else if ((pm = skin->queryImage("volume_grafik")))
	{
		/* still using old naming */
		x = skin->queryValue("volume.grafik.pos.x", "0");
		y = skin->queryValue("volume.grafik.pos.y", "0");
	}

	if (pm)
	{
		volume.setPixmap(pm);
		volume.setProperty("position", x + ":" + y);
		volume.resize( eSize( pm->x, pm->y ) );
		volume.setPixmapPosition(ePoint(0,0));
		volume.hide();
		volume.setBlitFlags( BF_ALPHATEST );
	}

// Volume Slider
	if (skin->queryValue("volume.slider.gauge", 0)) {
		// eDebug("VOLUMEBAR IS GAUGE");
		VolumeBar = new eGauge(&volume);
	}
	else {
		// eDebug("VOLUMEBAR IS SLIDER");
		VolumeBar = new eProgress(&volume);
	}

	VolumeBar->setSliderPixmap(skin->queryImage("volume.slider.pixmap"));

	if (skin->queryValue("volume.slider.alphatest", 0))
	{
		VolumeBar->setProperty("alphatest", "on");
	}

	x = skin->queryValue("volume.slider.pos.x", "0"),
	y = skin->queryValue("volume.slider.pos.y", "0");
	VolumeBar->setProperty("position", x + ":" + y);

	x = skin->queryValue("volume.slider.width", "0"),
	y = skin->queryValue("volume.slider.height", "0");
	VolumeBar->setProperty("size", x + ":" + y);

	VolumeBar->setLeftColor( skin->queryColor("volume_left") );
	VolumeBar->setRightColor( skin->queryColor("volume_right") );
	VolumeBar->setBorder(0);

	VolumeBar->setDirection(skin->queryValue("volume.slider.direction", 0));
	VolumeBar->setStart(skin->queryValue("volume.slider.start", 0));

// Other widgets

	subtitle = new eSubtitleWidget();
	subtitle->show();

	ASSIGN_MULTIPLE(dvrInfoBar, eMultiWidget, "dvrInfoBar");
	dvrInfoBar->hide();

	ASSIGN(DVRSpaceLeft, eLabel, "TimeLeft");
	DVRSpaceLeft->hide();

	ASSIGN_MULTIPLE(OSDExtra, eMultiWidget, "osdExtra");
	OSDExtra->hide();
	
	ASSIGN_MULTIPLE(OSDVerbose, eMultiWidget, "osdVerbose");
	OSDVerbose->hide();

	ASSIGN_MULTIPLE(miniZap, eMultiWidget, "miniZap");
	miniZap->hide();

	ASSIGN_MULTIPLE(maxiZap, eMultiWidget, "maxiZap");
	maxiZap->hide();

	ASSIGN_MULTIPLE(dvbInfoBar, eMultiWidget, "dvbInfoBar");
	dvbInfoBar->hide();

	ASSIGN_MULTIPLE(fileInfoBar, eMultiWidget, "fileInfoBar");
	fileInfoBar->hide();

	ASSIGN_MULTIPLE(fileinfos, eMultiLabel, "fileinfos");

	dvrfunctions=0;
	stateOSD=0;

	ASSIGN(recstatus, eLabel, "recStatus");
	recstatus->hide();

	ASSIGN(recchannel, eLabel, "recChannel");
	recchannel->hide();

#ifndef DISABLE_FILE
	ASSIGN_MULTIPLE(Progress, eMultiProgressWithIndices, "progress_bar");
	Progress->setIndices(&indices);
	indices_enabled = 0;
#else
	ASSIGN_MULTIPLE(Progress, eMultiProgressWithIndices, "progress_bar");
	//Progress=new eProgress(this);
	//Progress->setName("progress_bar");
#endif
#ifndef DISABLE_LCD
	lcdmain.show();
#endif
// SNR Patch
	ASSIGN(p_snr, eProgress, "snr_progress");
	ASSIGN(p_agc, eProgress, "agc_progress");
	ASSIGN(p_ber, eProgress, "ber_progress");
	ASSIGN(lsnr_num, eLabel, "snr_num");
	ASSIGN(lsync_num, eLabel, "agc_num");
	ASSIGN(lber_num, eLabel, "ber_num");
	ASSIGN(lsnr_text, eLabel, "snr_text");
	ASSIGN(lagc_text, eLabel, "agc_text");
	ASSIGN(lber_text, eLabel, "ber_text");
	lsnr_text->setText("SNR:");
	lagc_text->setText("AGC:");
	lber_text->setText("BER:");
	ASSIGN_MULTIPLE(lfreq_val, eMultiLabel, "fq_val");
	ASSIGN_MULTIPLE(lsymrate_val, eMultiLabel, "sr_val");
	ASSIGN_MULTIPLE(lpolar_val, eMultiLabel, "pl_val");
	ASSIGN_MULTIPLE(lfec_val, eMultiLabel, "fc_val");

	ASSIGN_MULTIPLE(SoftCam, eMultiLabel, "softcam");
	ASSIGN_MULTIPLE(SoftcamInfo, eMultiLabel, "softcam_info");	
	ASSIGN_MULTIPLE(VidFormat, eMultiLabel, "vidformat");
	ASSIGN(IrdetoEcm, eLabel, "irdeto_ecm");
	ASSIGN(SecaEcm, eLabel, "seca_ecm");
	ASSIGN(ViaEcm, eLabel, "via_ecm");
	ASSIGN(NagraEcm, eLabel, "nagra_ecm");
	ASSIGN(CWEcm, eLabel, "cw_ecm");
	ASSIGN(NDSEcm, eLabel, "nds_ecm");
	ASSIGN(ConaxEcm, eLabel, "conax_ecm");
	ASSIGN(BetaEcm, eLabel, "beta_ecm");
	ASSIGN(PowerVuEcm, eLabel, "powervu_ecm");
	ASSIGN(DreamCrEcm, eLabel, "dreamcr_ecm");
	ASSIGN(RusCrEcm, eLabel, "ruscr_ecm");
	ASSIGN(IceCrEcm, eLabel, "icecr_ecm");
	ASSIGN(CodiCrEcm, eLabel, "codicr_ecm");
	ASSIGN(IrdetoNo, eLabel, "irdeto_no");
	ASSIGN(SecaNo, eLabel, "seca_no");
	ASSIGN(ViaNo, eLabel, "via_no");
	ASSIGN(NagraNo, eLabel, "nagra_no");
	ASSIGN(CWNo, eLabel, "cw_no");
	ASSIGN(NDSNo, eLabel, "nds_no");
	ASSIGN(ConaxNo, eLabel, "conax_no");
	ASSIGN(BetaNo, eLabel, "beta_no");
	ASSIGN(PowerVuNo, eLabel, "powervu_no");
	ASSIGN(DreamCrNo, eLabel, "dreamcr_no");
	ASSIGN(RusCrNo, eLabel, "ruscr_no");
	ASSIGN(IceCrNo, eLabel, "icecr_no");
	ASSIGN(CodiCrNo, eLabel, "codicr_no");
	ASSIGN(IrdetoUca, eLabel, "irdeto_uca");
	ASSIGN(SecaUca, eLabel, "seca_uca");
	ASSIGN(ViaUca, eLabel, "via_uca");
	ASSIGN(NagraUca, eLabel, "nagra_uca");
	ASSIGN(CWUca, eLabel, "cw_uca");
	ASSIGN(NDSUca, eLabel, "nds_uca");
	ASSIGN(ConaxUca, eLabel, "conax_uca");
	ASSIGN(BetaUca, eLabel, "beta_uca");
	ASSIGN(PowerVuUca, eLabel, "powervu_uca");
	ASSIGN(DreamCrUca, eLabel, "dreamcr_uca");
	ASSIGN(RusCrUca, eLabel, "ruscr_uca");
	ASSIGN(IceCrUca, eLabel, "icecr_uca");
	ASSIGN(CodiCrUca, eLabel, "codicr_uca");
	IrdetoEcm->hide();
	SecaEcm->hide();
	ViaEcm->hide();
	NagraEcm->hide();
	CWEcm->hide();
	NDSEcm->hide();
	ConaxEcm->hide();
	BetaEcm->hide();
	PowerVuEcm->hide();
	DreamCrEcm->hide();
	RusCrEcm->hide();
	IceCrEcm->hide();
	CodiCrEcm->hide();
	IrdetoNo->show();
	SecaNo->show();
	ViaNo->show();
	NagraNo->show();
	CWNo->show();
	NDSNo->show();
	ConaxNo->show();
	BetaNo->show();
	PowerVuNo->show();
	DreamCrNo->show();
	RusCrNo->show();
	IceCrNo->show();
	CodiCrNo->show();
	IrdetoUca->hide();
	SecaUca->hide();
	ViaUca->hide();
	NagraUca->hide();
	CWUca->hide();
	NDSUca->hide();
	ConaxUca->hide();
	BetaUca->hide();
	PowerVuUca->hide();
	DreamCrUca->hide();
	RusCrUca->hide();
	IceCrUca->hide();
	CodiCrUca->hide();
// SNR Patch

	ASSIGN_MULTIPLE(ChannelNumber, eMultiLabel, "ch_number");
	ASSIGN_MULTIPLE(ChannelName, eMultiLabel, "ch_name");
	ASSIGN_MULTIPLE(ProviderName, eMultiLabel,"prov_name");

	ASSIGN_MULTIPLE(EINow, eMultiLabel, "e_now_title");
	ASSIGN_MULTIPLE(EINext, eMultiLabel, "e_next_title");

	ASSIGN_MULTIPLE(EINowDuration, eMultiLabel, "e_now_duration");
	ASSIGN_MULTIPLE(EINextDuration, eMultiLabel, "e_next_duration");

	ASSIGN_MULTIPLE(EINowTime, eMultiLabel, "e_now_time");
	ASSIGN_MULTIPLE(EINextTime, eMultiLabel, "e_next_time");
	ASSIGN_MULTIPLE(EINextETime, eMultiLabel, "e_next_etime");

	ASSIGN(Description, eLabel, "description");
	ASSIGN_MULTIPLE(IBVolumeBar, eMultiProgress, "volume_bar");

	ASSIGN(ButtonRedEn, eLabel, "button_red_enabled");
	ASSIGN(ButtonGreenEn, eLabel, "button_green_enabled");
	ASSIGN(ButtonYellowEn, eLabel, "button_yellow_enabled");
	ASSIGN(ButtonBlueEn, eLabel, "button_blue_enabled");
	ASSIGN(ButtonRedDis, eLabel, "button_red_disabled");
	ASSIGN(ButtonGreenDis, eLabel, "button_green_disabled");
	ASSIGN(ButtonYellowDis, eLabel, "button_yellow_disabled");
	ASSIGN(ButtonBlueDis, eLabel, "button_blue_disabled");
	//ASSIGN(AudioOrPause,eLabel, "AudioOrPause");
	ASSIGN(YellowButtonDesc, eLabel, "button_yellow_desc");
	ASSIGN(BlueButtonDesc, eLabel, "button_blue_desc");
	ASSIGN(GreenButtonDesc, eLabel, "button_green_desc");

	ASSIGN(DolbyOn, eLabel, "osd_dolby_on");
	ASSIGN(CryptOn, eLabel, "osd_crypt_on");
	ASSIGN(WideOn, eLabel, "osd_format_on");
	ASSIGN(VtxtOn, eLabel, "osd_txt_on");
	ASSIGN(AudioOn, eLabel, "osd_audio_on");
	ASSIGN(DolbyOff, eLabel, "osd_dolby_off");
	ASSIGN(CryptOff, eLabel, "osd_crypt_off");
	ASSIGN(WideOff, eLabel, "osd_format_off");
	ASSIGN(VtxtOff, eLabel, "osd_txt_off");
	ASSIGN(AudioOff, eLabel, "osd_audio_off");
	DolbyOn->hide();
	CryptOn->hide();
	WideOn->hide();
	VtxtOn->hide();
	AudioOn->hide();
	DolbyOff->show();
	CryptOff->show();
	WideOff->show();
	VtxtOff->show();
	AudioOff->show();

	ButtonRedEn->hide();
	ButtonRedDis->show();
	ButtonGreenEn->hide();
	ButtonGreenDis->show();
	// ButtonYellowEn->hide();
	// ButtonYellowDis->show();
	// Always enable Yellow button
	ButtonYellowEn->show();
	ButtonYellowDis->hide();
	ButtonBlueEn->show();
	ButtonBlueDis->hide();

	ASSIGN_MULTIPLE(Clock, eMultiLabel, "time");
	ASSIGN_MULTIPLE(Date, eMultiLabel, "date");
	ASSIGN(aHour, eGauge, "analog.hour");
	ASSIGN(aMins, eGauge, "analog.min");
	ASSIGN(aSecs, eGauge, "analog.sec");

	cur_start=cur_duration=-1;
	cur_event_text="";
	cur_event_id=-1;

	CONNECT(eServiceInterface::getInstance()->serviceEvent, eZapMain::handleServiceEvent);

	CONNECT(eEPGCache::getInstance()->EPGAvail, eZapMain::EPGAvail);
	CONNECT(eEPGCache::getInstance()->EPGUpdated, eZapMain::EPGUpdated);
	CONNECT(eEPGCache::getInstance()->organiseRequest, eZapMain::EPGOrganiseRequest);

	CONNECT(timeout.timeout, eZapMain::timeOut);

	CONNECT(clocktimer.timeout, eZapMain::clockUpdate);
	CONNECT(messagetimeout.timeout, eZapMain::nextMessage);
	CONNECT(epgNowNextTimer.timeout, eZapMain::epgNowNextRefresh);

	CONNECT(progresstimer.timeout, eZapMain::updateProgress);

	CONNECT(eDVB::getInstance()->timeUpdated, eZapMain::clockUpdate);
	CONNECT(eAVSwitch::getInstance()->volumeChanged, eZapMain::updateVolume);

	CONNECT(message_notifier.recv_msg, eZapMain::gotMessage);
	CONNECT(epg_messages.recv_msg, eZapMain::gotEPGMessage);

	CONNECT(volumeTimer.timeout, eZapMain::hideVolumeSlider );
#ifndef DISABLE_FILE
	CONNECT(recStatusBlink.timeout, eZapMain::blinkRecord);
	CONNECT(permanentTimeshiftTimer.timeout, eZapMain::startPermanentTimeshift);
	CONNECT(ledStatusBack.timeout, eZapMain::ledBack);
#endif

	CONNECT( eFrontend::getInstance()->s_RotorRunning, eZapMain::onRotorStart );
	CONNECT( eFrontend::getInstance()->s_RotorStopped, eZapMain::onRotorStop );
	CONNECT( eFrontend::getInstance()->s_RotorTimeout, eZapMain::onRotorTimeout );

	CONNECT( eWidget::showHelp, eZapMain::showHelp );

	CONNECT( i_enigmaGlobalActions->volumeUp.handler, eZapMain::volumeUp);
	CONNECT( i_enigmaGlobalActions->volumeDown.handler, eZapMain::volumeDown);
	CONNECT( i_enigmaGlobalActions->toggleMute.handler, eZapMain::toggleMute);

	CONNECT( eZapStandby::RCWakeUp, eZapMain::clearWasSleeping );

	CONNECT( eDVBCAHandler::getInstance()->clientname, eZapMain::SoftcamNameChanged);
	CONNECT( eDVBCAHandler::getInstance()->usedcaid, eZapMain::usedCaidChanged);
	CONNECT( eDVBCAHandler::getInstance()->verboseinfo, eZapMain::SoftcamInfoChanged);
	softcamName[0] = 0;
	softcamInfo[0] = 0;

	actual_eventDisplay=0;

	clockUpdate();
	standbyTime.tv_sec=-1;

#ifndef DISABLE_FILE
	skipcounter=0;
	skipping=0;
#endif

	addActionMap(&i_enigmaMainActions->map);
	addActionMap(&i_numberActions->map);

	gotPMT();
	gotSDT();
	gotEIT();

	eplPath = CONFIGDIR "/enigma";

#ifndef DISABLE_FILE
	loadRecordings(true);
#endif
	loadPlaylist(true);
	loadUserBouquets( false );

	eServiceSelector *sel = eZap::getInstance()->getServiceSelector();
	CONNECT(sel->addServiceToPlaylist, eZapMain::doPlaylistAdd);
	CONNECT(sel->addServiceToUserBouquet, eZapMain::addServiceToUserBouquet);
	CONNECT(subservicesel.addToUserBouquet, eZapMain::addServiceToUserBouquet );
	CONNECT(sel->removeServiceFromUserBouquet, eZapMain::removeServiceFromUserBouquet );
	CONNECT(sel->showMenu, eZapMain::showServiceMenu);
	CONNECT_2_1(sel->setMode, eZapMain::setMode, 0);
	CONNECT(sel->moveEntry, eZapMain::moveService);
	CONNECT(sel->showEPGList, eZapMain::showEPGList);
	CONNECT(sel->getRoot, eZapMain::getRoot);
	CONNECT(sel->getFirstBouquetServiceNum, eZapMain::getFirstBouquetServiceNum);
	CONNECT(sel->deletePressed, eZapMain::deleteService );
	CONNECT(sel->renameService, eZapMain::renameService );
	CONNECT(sel->renameBouquet, eZapMain::renameBouquet );
	CONNECT(sel->newMarkerPressed, eZapMain::createMarker );
	CONNECT(sel->copyToBouquetList, eZapMain::copyProviderToBouquets );
	reloadPaths();

	eServiceReference::loadLockedList( (eplPath+"/services.locked").c_str() );
	eTransponderList::getInstance()->readTimeOffsetData( (eplPath+"/timeOffsetMap").c_str() );

	mode=-1;  // fake mode for first call of setMode

	// get last mode form registry ( TV Radio File )
	int curMode;
	if ( eConfig::getInstance()->getKey("/ezap/ui/lastmode", curMode ) )
		curMode = 0;  // defaut TV Mode

	if (curMode < 0)
		curMode=0;

	setMode(curMode);  // do it..

	if ( eConfig::getInstance()->getKey("/ezap/ui/playlistmode", playlistmode ) )
		playlistmode = 0;  // default TV Mode

	int fastzap=0;
	if ( eConfig::getInstance()->getKey("/elitedvb/extra/fastzapping", fastzap ) )
	{
		fastzap = 1;
		eConfig::getInstance()->setKey("/elitedvb/extra/fastzapping", fastzap );
	}

	Decoder::setFastZap(fastzap);

	startMessages();

	// dvrInfoBar->zOrderRaise();
	// dvbInfoBar->zOrderRaise();
	CONNECT( eStreamWatchdog::getInstance()->VCRActivityChanged, eZapMain::toggleScart );
#ifndef DISABLE_CI
	CONNECT( mmi_messages.recv_msg, eZapMain::handleMMIMessage );
	if ( eDVB::getInstance()->DVBCI )
		CONNECT( eDVB::getInstance()->DVBCI->ci_mmi_progress, eZapMain::receiveMMIMessageCI1 );
	if ( eDVB::getInstance()->DVBCI2 )
		CONNECT( eDVB::getInstance()->DVBCI2->ci_mmi_progress, eZapMain::receiveMMIMessageCI2 );
#endif
	CONNECT( rdstext_decoder.textReady, eZapMain::gotRDSText );
	int bootcount=1;
	eConfig::getInstance()->getKey("/elitedvb/system/bootCount", bootcount);
	if ( bootcount > 1 )
	{
		int startup=0;
		eConfig::getInstance()->getKey(eString().sprintf("/ezap/ui/startup/%d",mode).c_str(), startup );
		if ( startup == 0 && playlist->current != playlist->getConstList().end() )
			playService( playlist->current->service, psDontAdd|psSeekPos);  // then play the last service
		else if ( modeLast[mode].current() )
			playService( modeLast[mode].current() ,0 );  // then play the last service
	}
	message_notifier.send(eZapMain::messageCheckVCR);
}

#ifndef DISABLE_CI
void eZapMain::receiveMMIMessageCI1( const char* data, int len )
{
	if ( !enigmaCIMMI::getInstance( eDVB::getInstance()->DVBCI)->connected() )
	{
		char *dest = new char[len];
		memcpy( dest, data, len );
		mmi_messages.send( eMMIMessage( eDVB::getInstance()->DVBCI, dest, len ) );
	}
	else
		eDebug("ignore MMI");
}

void eZapMain::receiveMMIMessageCI2( const char* data, int len )
{
	if ( !enigmaCIMMI::getInstance( eDVB::getInstance()->DVBCI2)->connected() )
	{
		char *dest = new char[len];
		memcpy( dest, data, len );
		mmi_messages.send( eMMIMessage( eDVB::getInstance()->DVBCI2, dest, len ) );
	}
	else
		eDebug("ignore MMI");
}

void eZapMain::handleMMIMessage( const eMMIMessage &msg )
{
//	eDebug("eZapMain::handleMMIMessage");
	enigmaMMI *p = enigmaCIMMI::getInstance(msg.from);

	if ( !memcmp( msg.data, "\x9f\x88\x00", 3 ) )
		postMessage(eZapMessage(0), 1);

	if ( !strncmp( msg.data, "INIT", 4 ) )
	{
		if ( eApp->looplevel() == 1 && ( !currentFocus || currentFocus == this ) )
		{
			int hideErrorWindows = 0;
			eConfig::getInstance()->getKey("/elitedvb/extra/hideerror", hideErrorWindows);
			if(!hideErrorWindows)
			{
				postMessage(eZapMessage(0,"Common Interface",_("please wait while initializing Common Interface ..."),8),0);
			}
		}
	}
	else
	{
		eServiceHandler *handler = eServiceInterface::getInstance()->getService();
		if ( handler && handler->getFlags() & eServiceHandler::flagIsScrambled )
		{
			if ( !p->handleMMIMessage( msg.data ) )
				return;
		}
	}
	delete [] msg.data;
}
#endif

eZapMain::~eZapMain()
{
	exit_main();
}

void eZapMain::exit_main()
{
#ifndef DISABLE_FILE
	if ( state & stateRecording )
	{
		if ( state & stateInTimerMode )
			eTimerManager::getInstance()->abortEvent( ePlaylistEntry::errorUserAborted );
		else
			recordDVR(0,1);
	}
#endif
	getPlaylistPosition();
	savePlaylist(true);
	destroyUserBouquets(true);
	// get current selected serviceselector path
	if ( mode != -1 ) // valid mode?
		getServiceSelectorPath(modeLast[mode]);
  // save last mode to registry
	eConfig::getInstance()->setKey("/ezap/ui/lastmode", mode );
	// save for all modes the servicePath to registry
	for (mode=modeTV; mode < modeEnd; mode++ )
	{
		int startup = 0;
		eConfig::getInstance()->getKey( eString().sprintf("/ezap/ui/startup/%d",mode).c_str(), startup );
		if (!startup)
		{
			eString str = modeLast[mode].toString();
			eConfig::getInstance()->setKey( eString().sprintf("/ezap/ui/modes/%d/path0", mode).c_str(), str.c_str() );
		}
	}
	if (instance == this)
		instance = 0;
#ifndef DISABLE_LCD
	lcdmain.lcdMain->hide();
	lcdmain.lcdShutdown->show();
	gLCDDC::getInstance()->setUpdate(0);
	if ( eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7000
		|| eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7020 )
		eDBoxLCD::getInstance()->switchLCD(0);
#endif
	delete subtitle;
	eConfig::getInstance()->setKey("/ezap/ui/serviceSelectorStyle", eZap::getInstance()->getServiceSelector()->getStyle() );
	eConfig::getInstance()->setKey("/ezap/ui/playlistmode", playlistmode);
	eServiceReference::saveLockedList( (eplPath+"/services.locked").c_str() );
	eTransponderList::getInstance()->writeTimeOffsetData( (eplPath+"/timeOffsetMap").c_str() );
}

void eZapMain::prepareDVRHelp()
{
	addActionToHelpList(&i_enigmaMainActions->startSkipReverse);
	addActionToHelpList(&i_enigmaMainActions->play);
	addActionToHelpList(&i_enigmaMainActions->pause);
	addActionToHelpList(&i_enigmaMainActions->startSkipForward);

	addActionToHelpList(&i_enigmaMainActions->stop);
	addActionToHelpList(&i_enigmaMainActions->record);

	addActionToHelpList(&i_enigmaMainActions->showEPG);
	addActionToHelpList(&i_enigmaMainActions->showInfobarEPG);

	addActionToHelpList(&i_enigmaMainActions->stepForward);  	 
	addActionToHelpList(&i_enigmaMainActions->stepBack); 	 

	addActionToHelpList(i_numberActions->key1.setDescription(_("rewind 15 seconds")));
	addActionToHelpList(i_numberActions->key4.setDescription(_("rewind 1 minute")));
	addActionToHelpList(i_numberActions->key7.setDescription(_("rewind 5 minutes")));
	addActionToHelpList(i_numberActions->key2.setDescription(_("enter number of minutes to rewind")));
	addActionToHelpList(i_numberActions->key3.setDescription(_("fast forward 15 seconds")));
	addActionToHelpList(i_numberActions->key6.setDescription(_("fast forward 1 minute")));
	addActionToHelpList(i_numberActions->key9.setDescription(_("fast forward 5 minutes")));
	addActionToHelpList(i_numberActions->key8.setDescription(_("enter number of minutes to fast forward")));


/*	addActionToHelpList(&i_enigmaMainActions->standby_press);
	addActionToHelpList(&i_enigmaMainActions->showInfobar);
	addActionToHelpList(&i_enigmaMainActions->hideInfobar);

	addActionToHelpList(&i_enigmaMainActions->showServiceSelector);
	addActionToHelpList(&i_enigmaMainActions->serviceListDown);
	addActionToHelpList(&i_enigmaMainActions->serviceListUp);*/
	addActionToHelpList(&i_enigmaMainActions->toggleIndexmark);
	addActionToHelpList(&i_enigmaMainActions->indexSeekNext);
	addActionToHelpList(&i_enigmaMainActions->indexSeekPrev);

}

void eZapMain::prepareNonDVRHelp()
{
	addActionToHelpList(&i_enigmaMainActions->showMainMenu);
	addActionToHelpList(&i_enigmaMainActions->showEPG);
	addActionToHelpList(&i_enigmaMainActions->pluginVTXT);
	addActionToHelpList(&i_enigmaMainActions->toggleDVRFunctions);
	addActionToHelpList(&i_enigmaMainActions->modeTV);
	addActionToHelpList(&i_enigmaMainActions->modeRadio);
#ifndef DISABLE_FILE
	addActionToHelpList(&i_enigmaMainActions->modeFile);
#endif
	addActionToHelpList(&i_enigmaMainActions->playlistNextService);
	addActionToHelpList(&i_enigmaMainActions->playlistPrevService);

	addActionToHelpList(&i_enigmaMainActions->showAudio);
	addActionToHelpList(&i_enigmaMainActions->showEPGList);
//	addActionToHelpList(&i_enigmaMainActions->showSubservices);
	addActionToHelpList(&i_enigmaMainActions->greenButton);
	addActionToHelpList(&i_enigmaMainActions->yellowButton);
	addActionToHelpList(&i_enigmaMainActions->blueButton);
}

void eZapMain::set16_9Logo(int aspect)
{
	if (aspect)
	{
		WideOff->hide();
		WideOn->show();
		is16_9 = 1;
	} else
	{
		WideOn->hide();
		WideOff->show();
		is16_9 = 0;
	}
}

void eZapMain::setEPGButton(bool b)
{
	if (b)
	{
		ButtonRedDis->hide();
		ButtonRedEn->show();
		isEPG = 1;

		/* we have epg available for our channel, immediately fill now/next with our epg data, if we didn't get valid now/next EIT already */
		if (!validEITReceived)
		{
			setEPGNowNext();
		}
		
	}
	else
	{
		ButtonRedEn->hide();
		ButtonRedDis->show();
		isEPG = 0;
	}
}

void eZapMain::setVTButton(bool b)
{
	if (b)
	{
		VtxtOff->hide();
		VtxtOn->show();
		isVT = 1;
	}
	else
	{
		VtxtOn->hide();
		VtxtOff->show();
		isVT = 0;
	}
}

void eZapMain::setAC3Logo(bool b)
{
	if (b)
	{
		DolbyOff->hide();
		DolbyOn->show();
		isAC3 = 1;
	} else
	{
		DolbyOn->hide();
		DolbyOff->show();
		isAC3 = 0;
	}
}

void eZapMain::setSmartcardLogo(bool b)
{
	if (b)
	{
		CryptOff->hide();
		CryptOn->show();
		isCrypted = 1;
	} else
	{
		CryptOn->hide();
		CryptOff->show();
		isCrypted = 0;
	}
}

void eZapMain::setNow(EITEvent *event)
{
	eString start, duration, descr;
	cur_event_text = "";

	if (event)
	{
		LocalEventData led;
		led.getLocalData(event, &cur_event_text, &descr);

		if (event->duration > 0)
		{
			tm *t = event->start_time != -1 ? localtime(&event->start_time) : 0;
			if (t)
			{
				start = getTimeStr(t, gTS_SHORT);
			}
			int show_current_remaining = 1;
			eConfig::getInstance()->getKey("/ezap/osd/showCurrentRemaining", show_current_remaining);
			if (!show_current_remaining || !eDVB::getInstance()->time_difference)
			{
				duration.sprintf("%d min", event->duration / 60);
				EINowDuration->setText(duration);
			}
		}
	}

	fileinfos->setText(cur_event_text);
	EINow->setText(cur_event_text ? cur_event_text : _("no EPG available"));
	EINowTime->setText(start);
	Description->setText(descr);
}

void eZapMain::setNext(EITEvent *event)
{
	eString start, duration, endtime, text;

	if (event)
	{
		LocalEventData led;
		led.getLocalData(event, &text);

		if (event->duration > 0)
		{
			tm *t = event->start_time != -1 ? localtime(&event->start_time) : 0;

			if (t)
			{
				start = getTimeStr(t, gTS_SHORT);
			}
			duration.sprintf("%d min", event->duration / 60);

			time_t etime = event->start_time + event->duration;
			t = event->start_time != -1 ? localtime(&etime) : 0;
			if (t) endtime = getTimeStr(t, gTS_SHORT);
		}
	}

	EINextDuration->setText(duration);
	EINext->setText(text);
	EINextTime->setText(start);
	EINextETime->setText(endtime);
}

void eZapMain::setEIT(EIT *eit)
{
	if (eit)
	{
		int p = 0;
		eServiceReferenceDVB &ref=(eServiceReferenceDVB&)eServiceInterface::getInstance()->service;

		for (ePtrList<EITEvent>::iterator i(eit->events); i != eit->events.end(); ++i, p++)
		{
			EITEvent *event=*i;

//			eDebug("event->running_status=%d, p=%d", event->running_status, p );
			if ((event->running_status>=2) || ((!p) && (!event->running_status)))
			{
//				eDebug("set cur_event_id to %d", event->event_id);
				cur_event_id=event->event_id;
				cur_start=event->start_time;
				cur_duration=event->duration;
				clockUpdate();

				int cnt=0;
				for (ePtrList<Descriptor>::iterator d(event->descriptor); d != event->descriptor.end(); ++d)
				{
					if (d->Tag()==DESCR_LINKAGE)
					{
						LinkageDescriptor *ld=(LinkageDescriptor*)*d;
						if (ld->linkage_type==0xB0)
						{
							if (!cnt)
							{
								subservicesel.clear();
							}
							subservicesel.add(ref.getDVBNamespace(), ld);
							cnt++;
						}
					}
					else if ( d->Tag()==DESCR_CAROUSEL_ID )
					{
						CarouselIdentifierDescriptor *carousel = (CarouselIdentifierDescriptor*)*d;
						eDebug("DESCR_CAROUSEL_ID descriptor found!!!!!!!!! %X\n", carousel->carousel_id);
					}
					else if (d->Tag() == DESCR_DATA_BROADCAST_ID)
					{
						eDebug("DESCR_DATA_BROADCAST_ID descriptor found!!!!!!!!!\n");
					}
				}

				if ( cnt )  // subservices added ?
				{
					flags|=ENIGMA_SUBSERVICES;
					subservicesel.selectCurrent();
				}
			}
			switch (p)
			{
			case 0:
				setNow(event);
				break;
			case 1:
				setNext(event);
				break;
			}
		}

		/* now that we've got EIT now/next, stop refreshing now/next with EPG */
		validEITReceived = true;
		epgNowNextTimer.stop();

		eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
		if ( sapi )
			audiosel.update(sapi->audioStreams);
	}
	else
	{
		/* we have no EIT, try to use EPG instead */
		validEITReceived = false;
		if (setEPGNowNext() < 0)
		{
			/* no EPG either, clear 'now' info */
			setNow(NULL);
		}
	}

	if (flags&(ENIGMA_NVOD|ENIGMA_SUBSERVICES))
	{
		ButtonGreenEn->show();
		ButtonGreenDis->hide();
	}
	ePtrList<EITEvent> dummy;
	if (actual_eventDisplay)
		actual_eventDisplay->setList(eit?eit->events:dummy);
}

void eZapMain::epgNowNextRefresh()
{
	setEPGNowNext();
}

int eZapMain::setEPGNowNext()
{
	int ret = -1;

	eServiceReferenceDVB &ref = (eServiceReferenceDVB&)eServiceInterface::getInstance()->service;

	timeMapPtr pMap = eEPGCache::getInstance()->getTimeMapPtr(ref, 0, 0, 2);

	if (pMap)
	{
		int p = 0;
		int tsidonid = (ref.getTransportStreamID().get() << 16) | ref.getOriginalNetworkID().get();

		for (timeMap::const_iterator It = pMap->begin(); It != pMap->end() && p < 2; It++, p++)
		{
			EITEvent event(*It->second, tsidonid, It->second->type);
			switch (p)
			{
			case 0:
				cur_event_id = event.event_id;
				cur_start = event.start_time;
				cur_duration = event.duration;
				clockUpdate();
				setNow(&event);
				ret = 0;
				break;
			case 1:
				setNext(&event);
				break;
			}
		}
	}

	epgNowNextTimer.startLongTimer(30);
	return ret;
}

void eZapMain::updateServiceNum( const eServiceReference &_serviceref )
{
	eService *rservice = eServiceInterface::getInstance()->addRef( refservice );
	eService *service = eServiceInterface::getInstance()->addRef( _serviceref );
	int num=-1;

	if (rservice)
		num=eZap::getInstance()->getServiceSelector()->getServiceNum(_serviceref);
	if ((num == -1) && service)
		num=eZap::getInstance()->getServiceSelector()->getServiceNum(_serviceref);
	if ((num == -1) && service && service->dvb)
		num=service->dvb->service_number;

	if ( num != -1)
	{
		ChannelNumber->setText(eString().sprintf("%d", num));
		if( !eSystemInfo::getInstance()->hasLCD() )
		{
			//eDebug("write servicenum to led-display");
			int fd=::open("/dev/dbox/fp0",O_RDWR);
			::ioctl(fd,4,&num);
			::close(fd);
		}
	}

	if ( service )
		eServiceInterface::getInstance()->removeRef( _serviceref );
	if ( rservice )
		eServiceInterface::getInstance()->removeRef( refservice );
}

void eZapMain::updateProgress()
{
#ifndef DISABLE_FILE
	if (serviceFlags & eServiceHandler::flagSupportPosition)
	{
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (!handler)
			return;
		int total=handler->getPosition(eServiceHandler::posQueryLength);
		int current=handler->getPosition(eServiceHandler::posQueryCurrent);

		if (total != indices.getTotalLength())
			indices.setTotalLength(total);

		if ((total > 0) && (current != -1))
		{
			Progress->setPerc(current*100/total);
			Progress->show();
#ifndef DISABLE_LCD
			lcdmain.lcdMain->Progress->setPerc(current*100/total);
			lcdmain.lcdMain->Progress->show();
#endif
			int min= total-current;
			int sec=min%60;
			min/=60;
			int sign=-1;
			ChannelNumber->setText(eString().sprintf("%s%d:%02d", (sign==-1)?"-":"", min, sec));

		} else
		{
#ifndef DISABLE_LCD
			lcdmain.lcdMain->Progress->hide();
#endif
			Progress->hide();
			ChannelNumber->setText("");
		}
	}
	else
#endif
	{
		updateServiceNum( eServiceInterface::getInstance()->service );

		time_t c=time(0)+eDVB::getInstance()->time_difference;
		tm *t=localtime(&c);
		if (t && eDVB::getInstance()->time_difference)
		{
			if ((cur_start <= c) && (c < cur_start+cur_duration) && cur_start != -1)
			{
				Progress->setPerc((c-cur_start)*100/cur_duration);
				int show_current_remaining=1;
				eConfig::getInstance()->getKey("/ezap/osd/showCurrentRemaining", show_current_remaining);
				if (show_current_remaining)
					EINowDuration->setText(eString().sprintf("+%d min", ((cur_start+cur_duration) - c) / 60 + (c%60 ? 1 : 0)));
				else
					EINowDuration->setText(eString().sprintf("%d min", cur_duration / 60));
				Progress->show();
#ifndef DISABLE_LCD
				lcdmain.lcdMain->Progress->setPerc((c-cur_start)*100/cur_duration);
				lcdmain.lcdMain->Progress->show();
#endif
			} else
			{
				if ( cur_duration != -1 )
					EINowDuration->setText(eString().sprintf("%d min", cur_duration / 60));
				Progress->hide();
#ifndef DISABLE_LCD
				lcdmain.lcdMain->Progress->hide();
#endif
			}
		} else
		{
			Progress->hide();
#ifndef DISABLE_LCD
			lcdmain.lcdMain->Progress->hide();
#endif
		}
	}
}

void eZapMain::getPlaylistPosition()
{
	int time=-1;
	if (serviceFlags & eServiceHandler::flagIsSeekable)
	{
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (!handler)
			return;
		time=handler->getPosition(eServiceHandler::posQueryRealCurrent);

		if ( playlist->current != playlist->getConstList().end() && playlist->current->service == eServiceInterface::getInstance()->service )
			playlist->current->current_position=time;
	}
}


void eZapMain::handleNVODService(SDTEntry *sdtentry)
{
	nvodsel.clear();
	for (ePtrList<Descriptor>::iterator i(sdtentry->descriptors); i != sdtentry->descriptors.end(); ++i)
		if (i->Tag()==DESCR_NVOD_REF)
		{
//			eDebug("nvod ref descr");
			for (ePtrList<NVODReferenceEntry>::iterator e(((NVODReferenceDescriptor*)*i)->entries); e != ((NVODReferenceDescriptor*)*i)->entries.end(); ++e)
				nvodsel.add(((eServiceReferenceDVB&)eServiceInterface::getInstance()->service).getDVBNamespace(), *e);
		}

	eService *service=eServiceInterface::getInstance()->addRef(eServiceInterface::getInstance()->service);
	if (service)
		nvodsel.setText(service->service_name.c_str());
	eServiceInterface::getInstance()->removeRef(eServiceInterface::getInstance()->service);
}

void eZapMain::showServiceSelector(int dir, int newTarget )
{
	hide();

	entered_playlistmode=0;
	eServiceSelector *e = eZap::getInstance()->getServiceSelector();

#ifndef DISABLE_LCD
	lcdmain.lcdMain->hide();
	lcdmain.lcdMenu->show();
	e->setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif

	getServiceSelectorPath(modeLast[mode]);

	eServicePath savedmode[modeEnd];
	for ( int m=modeTV; m < modeEnd; m++ )
			savedmode[m]=modeLast[m];
	int oldmode=mode;

	if ( newTarget )
	{
		eActionMap *amap = eActionMapList::getInstance()->findActionMap("serviceSelector");
		eAction *action=0;
		int newMode=-1;
		switch (newTarget)
		{
			case pathRoot:
				newMode=modeFile;
			case pathAll:
				action = amap->findAction("showAll");
				break;
			case pathPlaylist:
				newMode=modeFile;
			case pathProvider:
				action = amap->findAction("showProvider");
				break;
			case pathBouquets:
				action = amap->findAction("showBouquets");
				break;
			case pathRecordings:
				newMode = modeFile;
			case pathSatellites:
				action = amap->findAction("showSatellites");
				break;
		}
		if ( action )
		{
			if ( mode != newMode )
				setMode( newMode );
			e->eventHandler( eWidgetEvent(eWidgetEvent::evtAction, action));
		}
	}

	e->selectService(eServiceInterface::getInstance()->service);
	const eServiceReference *service = e->choose(dir); // reset path only when NOT showing specific list

#ifndef DISABLE_LCD
	lcdmain.lcdMain->show();
	lcdmain.lcdMenu->hide();
#endif

	int extZap=0;
	eConfig::getInstance()->getKey("/elitedvb/extra/extzapping", extZap);
	eServiceReference ref = eServiceInterface::getInstance()->service;
	if ( !service ||    // cancel pressed
			( !extZap && ref &&
				!(service->path && mode == modeFile) &&
				!CheckService(*service) ) )
	{
		if ( !entered_playlistmode )
		{
			for ( int m=modeTV; m < modeEnd; m++ )
					modeLast[m]=savedmode[m];

			if ( mode != oldmode ) // restore mode..
				setMode(oldmode);
			else // restore old path
				setServiceSelectorPath(modeLast[mode]);
		}
		else // push playlistref to top of current path
		{
			eServicePath p = eZap::getInstance()->getServiceSelector()->getPath();
			if ( p.current() != playlistref )
			{
				p.down( playlistref );
				p.down( eZap::getInstance()->getServiceSelector()->getSelected() );
				setServiceSelectorPath(p);
			}
		}
		if (!doHideInfobar())
			showInfobar();
		return;
	}

	if (*service == eServiceInterface::getInstance()->service)
	{
		if (!doHideInfobar())
			showInfobar();
		return;
	}

#ifndef DISABLE_FILE
	if ( eDVB::getInstance()->recorder || handleState() )
#else
	if ( handleState() )
#endif
	{
		getServiceSelectorPath(modeLast[mode]);

		if (eZap::getInstance()->getServiceSelector()->getPath().current() != playlistref)
		{
			playlistmode=0;
			playService(*service, 0);
		}
		else
			playService(*service, playlistmode?psDontAdd:psDontAdd|psSeekPos);
	}
}

void eZapMain::nextService(int add)
{
	if (mode == modeFile && indexSeek(1))
	{
		showInfobar();
		return;
	}
	
	eServicePath p = eZap::getInstance()->getServiceSelector()->getPath();
	(void)add;

	if ( p.size() && p.current() == playlistref )
		playlistNextService();
	else
	{
		int autoBouquetChange=0;
		eConfig::getInstance()->getKey("/elitedvb/extra/autobouquetchange", autoBouquetChange );
		eServiceSelector *ssel = eZap::getInstance()->getServiceSelector();
		bool ok=false;
		if ( !eDVB::getInstance()->recorder && autoBouquetChange
			&& ssel->selectService( eServiceInterface::getInstance()->service ) )
		{
			eServicePath p = modeLast[mode];
			if ( p.size() == 3 )
			{
				p.up();
				p.up();
				if ( p.current() == userTVBouquetsRef
						|| p.current() == userFileBouquetsRef
						|| p.current() == userRadioBouquetsRef )
				{
					int num=ssel->getServiceNum(eServiceInterface::getInstance()->service)+1;
					int ret = 0;
					do
					{
						ret = switchToNum( num, true );
						if ( ret == -1 )
							++num;
						else if ( ret )  // last service in bouquet(s)... wrap around
							num=1;
						else
							ok=true;
					}
					while ( ret ); //parental locked
				}
			}
		}

		if ( !ok )
		{
			const eServiceReference *service=eZap::getInstance()->getServiceSelector()->next();
			if (!service)
				return;
			else
				getServiceSelectorPath( modeLast[mode] );

			if (service->flags & eServiceReference::isDirectory)
				return;

			playService(*service, 0 );
		}
	}
}

void eZapMain::prevService()
{
	if (mode == modeFile && indexSeek(-1))
	{
		showInfobar();
		return;
	}
	
	eServicePath p = eZap::getInstance()->getServiceSelector()->getPath();

	if ( p.size() && p.current() == playlistref )
		playlistPrevService();
	else
	{
		int autoBouquetChange=0;
		eConfig::getInstance()->getKey("/elitedvb/extra/autobouquetchange", autoBouquetChange );
		eServiceSelector *ssel = eZap::getInstance()->getServiceSelector();
		bool ok=false;
		if ( !eDVB::getInstance()->recorder && autoBouquetChange
			&& ssel->selectService( eServiceInterface::getInstance()->service ) )
		{
			eServicePath p = modeLast[mode];
			if ( p.size() == 3 )
			{
				p.up();
				p.up();
				if ( p.current() == userTVBouquetsRef
					|| p.current() == userFileBouquetsRef
					|| p.current() == userRadioBouquetsRef )
				{
					int newNum = ssel->getServiceNum(eServiceInterface::getInstance()->service);
					int ret=0;
					do
					{
						newNum-=1;
						if ( !newNum ) // end of bouquet
							newNum=INT_MAX - switchToNum(INT_MAX, true);  // fake call for get last service in last bouquet
						ret = switchToNum(newNum);
						if ( ret == -1 )  // parental locked.. try next..
							continue;
						else if ( !ret )
							ok=true;
					}
					while ( ret == -1 );  // parental locked
				}
			}
		}

		if ( !ok )
		{
			const eServiceReference *service=eZap::getInstance()->getServiceSelector()->prev();

			if (!service)
				return;
			else
				getServiceSelectorPath( modeLast[mode] );

			if (service->flags & eServiceReference::isDirectory)
				return;

			playService(*service, 0 );
		}
	}
}



void eZapMain::playlistPrevService()
{
	int extZap=0;
	eConfig::getInstance()->getKey("/elitedvb/extra/extzapping", extZap);
	eServiceReference ref = eServiceInterface::getInstance()->service;
	if ( playlist->current != playlist->getConstList().end() && ref == playlist->current->service )
		getPlaylistPosition();
	while ( playlist->current != playlist->getConstList().begin())
	{
		playlist->current--;
		if ( playlist->current->service != ref && (extZap || ModeTypeEqual(ref, playlist->current->service) ) )
		{
			const eServicePath &p = playlist->current->getPath();
			playService((eServiceReference&)(*playlist->current), (playlistmode?psDontAdd:psDontAdd|psSeekPos)|(extZap?psSetMode:0));
			if (p.size() > 1)
				setServiceSelectorPath(p);
			return;
		}
	}
}

void eZapMain::playlistNextService()
{
	int extZap=0;
	eConfig::getInstance()->getKey("/elitedvb/extra/extzapping", extZap);
	eServiceReference ref = eServiceInterface::getInstance()->service;
	if ( playlist->current != playlist->getConstList().end() && ref == playlist->current->service )
		getPlaylistPosition();
	while (playlist->current != playlist->getConstList().end())
	{
		playlist->current++;
		if (playlist->current == playlist->getConstList().end())
		{
			playlist->current--;
			return;
		}
		if ( playlist->current->service != ref && (extZap || ModeTypeEqual(ref,playlist->current->service)) )
		{
			const eServicePath &p = playlist->current->getPath();
			playService((eServiceReference&)(*playlist->current), (playlistmode?psDontAdd:psDontAdd|psSeekPos)|(extZap?psSetMode:0));
			if (p.size() > 1)
				setServiceSelectorPath(p);
			return;
		}
	}
}

void eZapMain::volumeUp()
{
	if ( eZapStandby::getInstance() || enigmaVCR::getInstance() )
		switch ( eSystemInfo::getInstance()->getHwType() )
		{
			case eSystemInfo::dbox2Nokia:
			case eSystemInfo::dbox2Philips:
			case eSystemInfo::dbox2Sagem:
				break;
			default:
				return;
		}

#if 0
	if (eAudioDynamicCompression::getInstance() &&
			eAudioDynamicCompression::getInstance()->getEnable())
		{
			int oldval = eAudioDynamicCompression::getInstance()->getMax();
			oldval += 1000;
			if (oldval > 100000)
				oldval = 100000;
			eAudioDynamicCompression::getInstance()->setMax(oldval);
		}
	else
#endif
	if ( eZapStandby::getInstance() )
		eAVSwitch::getInstance()->changeVCRVolume(0, -2);
	else
		eAVSwitch::getInstance()->changeVolume(0, -2);

	if ( eMoviePlayer::getInstance()->status.STAT == eMoviePlayer::PLAY || !currentFocus || (currentFocus == this && !IBVolumeBar->isVisible()) )
	{
		volume.show();
		volumeTimer.start(2000, true);
	}
}

void eZapMain::volumeDown()
{
	if ( eZapStandby::getInstance() || enigmaVCR::getInstance() )
		switch ( eSystemInfo::getInstance()->getHwType() )
		{
			case eSystemInfo::dbox2Nokia:
			case eSystemInfo::dbox2Philips:
			case eSystemInfo::dbox2Sagem:
				break;
			default:
				return;
		}

#if 0
	if (eAudioDynamicCompression::getInstance() &&
			eAudioDynamicCompression::getInstance()->getEnable())
		{
			int oldval = eAudioDynamicCompression::getInstance()->getMax();
			oldval -= 1000;
			if (oldval < 0)
				oldval = 0;
			eAudioDynamicCompression::getInstance()->setMax(oldval);
		}
	else
#endif
	if ( eZapStandby::getInstance() )
		eAVSwitch::getInstance()->changeVCRVolume(0, +2);
	else
		eAVSwitch::getInstance()->changeVolume(0, +2);

	if ( eMoviePlayer::getInstance()->status.STAT == eMoviePlayer::PLAY || !currentFocus || (currentFocus == this && !IBVolumeBar->isVisible()) )
	{
		volume.show();
		volumeTimer.start(2000, true);
	}
}

void eZapMain::hideVolumeSlider()
{
	volume.hide();
}

void eZapMain::toggleMute()
{
	if ( eZapStandby::getInstance() || enigmaVCR::getInstance() )
		switch ( eSystemInfo::getInstance()->getHwType() )
		{
			case eSystemInfo::dbox2Nokia:
			case eSystemInfo::dbox2Philips:
			case eSystemInfo::dbox2Sagem:
				break;
			default:
				return;
		}
#ifndef DISABLE_FILE
	eServiceReference &ref = eServiceInterface::getInstance()->service;
// sorry.. disable Mute when playback TS or MPG File..
// better do pause
	if ( ( (ref.type == eServiceReference::idDVB && ref.path)
				|| (ref.type == eServiceReference::idUser
				&& ref.data[0] == eMP3Decoder::codecMPG )
				|| timeshift )
			&& (!eAVSwitch::getInstance()->getMute()) )
		pause();
	else
#endif
		eAVSwitch::getInstance()->toggleMute();
}

void eZapMain::showMainMenu()
{
	hide();

	eMainMenu mm;

#ifndef DISABLE_LCD
	lcdmain.lcdMain->hide();
	lcdmain.lcdMenu->show();
	mm.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif

	mm.show();
	int res = mm.exec();
	mm.hide();

	if (!doHideInfobar())
		showInfobar();

	if (res == 1 && handleState())
	{
		//standby Show Menu
		gettimeofday(&standbyTime, 0);
		standbyTime-=1;
		// reset nomenu flag, otherwise no shutdown menu next time
		standby_nomenu = 0;
		standbyRelease();
	}
#ifndef DISABLE_LCD
	lcdmain.lcdMenu->hide();
	lcdmain.lcdMain->show();
#endif
}

void eZapMain::toggleTimerMode(int newstate)
{
	eDebug("toggleTimerMode (%d)", newstate);
	if ( !newstate )
	{
		eDebug("remove stateInTimerMode");
		state &= ~stateInTimerMode;
	}
	else
	{
		eDebug("add stateInTimerMode");
		state |= stateInTimerMode;
	}
}

void eZapMain::standbyPress(int n)
{
	standby_nomenu = n;
	int fastshutdown = 0;
	eConfig::getInstance()->getKey("/extras/fastshutdown", fastshutdown);
	if (fastshutdown == 1)
		eZap::getInstance()->quit();
	gettimeofday(&standbyTime,0);
	standbyTime+=1000;
}

void eZapMain::standbyRepeat()
{
	if (state&stateInTimerMode && state&stateRecording)
		standbyRelease();
	else
	{
		timeval now;
		gettimeofday(&now,0);
		if ( standbyTime < now )
			standbyRelease();
	}
}

void eZapMain::standbyRelease()
{
	if ( standbyTime.tv_sec == -1) // we come from standby ?
		return;

	timeval now;
	gettimeofday(&now,0);
	bool b = standbyTime < now;
	if (b)
	{
		hide();
#ifndef DISABLE_LCD
		eSleepTimerContextMenu m(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#else
		eSleepTimerContextMenu m;
#endif
		int ret;
		if (standby_nomenu)
			ret = 1; // always shutdown if shutdown_nomenu-key is pressed
		else
		{
			m.show();
			ret = m.exec();
			m.hide();
		}
		switch (ret)
		{
			case 2:
				goto standby;
			case 3:
			{
				eSleepTimer t;
				t.show();
				EITEvent *evt = (EITEvent*) t.exec();
				int i = t.getCheckboxState();
				if (evt != (EITEvent*)-1)
				{
					eServiceReference ref;
					ref.descr = ( i==2 ? _("ShutdownTimer") : _("SleepTimer") );
					eTimerManager::getInstance()->addEventToTimerList( &t,
						&ref, evt,
						ePlaylistEntry::stateWaiting|
						ePlaylistEntry::doFinishOnly|
						(i==2?ePlaylistEntry::doShutdown:0)|
						(i==3?ePlaylistEntry::doGoSleep:0)
					);
					delete evt;
				}
				t.hide();
				break;
			}
			case 4: // reboot
					eZap::getInstance()->quit(4);
					break;
			case 5: // restart enigma
					eZap::getInstance()->quit(2);
					break;
			case 1: // shutdown
/*				if (handleState())*/
					eZap::getInstance()->quit();
				break;
			default:
			case 0:
				break;
		}
	}
	else
	{
standby:
		if ( !enigmaVCR::getInstance() )
		{
			eZapStandby standby;
			hide();
			standby.show();
			state |= stateSleeping;
			standbyTime.tv_sec=-1;
			if (state&stateInTimerMode && state&stateRecording)
				wasSleeping=3;
			// After standby we always want to check the pin when entering a pin protected service
			pinCheck::getInstance()->resetParentalPinOk();
			pinCheck::getInstance()->resetSetupPinOk();
#ifndef DISABLE_LCD
			lcdmain.lcdMenu->hide();
			lcdmain.lcdMain->show();
#endif
			standby.exec();   // this blocks all main actions...

/*
	  ...... sleeeeeeeep
*/
			standby.hide();   // here we are after wakeup
			state &= ~stateSleeping;
#ifndef DISABLE_FILE
			beginPermanentTimeshift();
#endif
		}
	}
}

/* SNR,AGC,BER DISPLAY begin */
void eZapMain::showSNR()
{
 	int status;	
	int snrrel = 0;
	int agcrel = 0;
	int berrel = 0;
	eString snrstring="";
	eString agcstring="";
	eString berstring="";

	if ( eZapMain::getInstance()->getMode() != eZapMain::modeFile )
	{
		eFrontend::getInstance()->getStatus(status, snrrel, snrstring, agcrel, agcstring, berrel, berstring);	

		// Vidformat should not be here, but cannot find a better place
		// as /proc/buts/bitstream is not completely updated at
		// eServiceEvent::evtStart event.
		VidFormat->setText(getVidFormat());
	}

	p_snr->setPerc(snrrel);
	p_agc->setPerc(agcrel);
	lsnr_num->setText(snrstring);
	lsync_num->setText(agcstring);
	lber_num->setText(berstring);
	p_ber->setPerc(berrel);	
}
/* SNR,AGC,BER DISPLAY end */

void eZapMain::showFreq()
{
	if ( eZapMain::getInstance()->getMode() != eZapMain::modeFile )
	{
		eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
		eTransponder *tp = sapi->transponder;
		if (tp)
		{
			lfreq_val->setText(eString().sprintf("%d", tp->satellite.frequency / 1000));
			lsymrate_val->setText(eString().sprintf("%d", tp->satellite.symbol_rate / 1000));
			lpolar_val->setText(tp->satellite.polarisation ? "V" : "H");
			lfec_val->setText(
				tp->satellite.fec == 0 ? "Auto" :
				tp->satellite.fec == 1 ? "1/2" :
				tp->satellite.fec == 2 ? "2/3" :
				tp->satellite.fec == 3 ? "3/4" :
				tp->satellite.fec == 4 ? "5/6" :
				tp->satellite.fec == 5 ? "7/8" :
				tp->satellite.fec == 6 ? "8/9" : "n/a/");
		}
	}
}


/* CA ECM DISPLAY patch */
void eZapMain::showECM(int usedcaid)
{
	bool irdeto = false;
	bool irdetoUsed = false;
	bool seca = false;
	bool secaUsed = false;
	bool via = false;
	bool viaUsed = false;
	bool nagra = false;
	bool nagraUsed = false;
	bool cw = false;
	bool cwUsed = false;
	bool nds = false;
	bool ndsUsed = false;
	bool conax = false;
	bool conaxUsed = false;
	bool beta = false;
	bool betaUsed = false;
	bool powervu = false;
	bool powervuUsed = false;
	bool dreamcr = false;
	bool dreamcrUsed = false;
	bool ruscr = false;
	bool ruscrUsed = false;
	bool icecr = false;
	bool icecrUsed = false;
	bool codicr = false;
	bool codicrUsed = false;

	if ( eZapMain::getInstance()->getMode() != eZapMain::modeFile )
	{
		eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();

		if (sapi) {
			std::set<int>& calist = sapi->usedCASystems;

			for (std::set<int>::iterator i(calist.begin()); i != calist.end(); ++i) {
				int cavalue = eStreaminfo::getInstance()->getCAValue(*i);
				switch (cavalue) {
					case 0x0100: seca = true; break; // "Seca/Mediaguard (Canal Plus)"
					// case 0x0200: break; // "CCETT"
					// case 0x0300: break; // "MSG MediaServices GmbH"
					// case 0x0400: break; // "Eurodec"
					case 0x0500: via = true; break; // "Viaccess (France Telecom)"
					case 0x0600: irdeto = true; break;  // "Irdeto"
					// case 0x0700: break; // "Jerrold/GI/Motorola"
					// case 0x0800: break; // "Matra Communication"
					case 0x0900: nds = true; break;// "Videoguard (News Datacom)"
					// case 0x0A00: break; // "Nokia"
					case 0x0B00: conax = true; break; // "Conax (Norwegian Telekom)"
					// case 0x0C00: break; // "NTL"
					case 0x0D00: cw = true; break; // "Cryptoworks (Philips)"
					case 0x0E00: powervu = true; break; // "Power VU (Scientific Atlanta)"
					// case 0x0F00: break; // "Sony"
					// case 0x1000: break; // "Tandberg Television"
					// case 0x1100: break; // "Thomson"
					// case 0x1200: break; // "TV/Com"
					// case 0x1300: break; // "HPT - Croatian Post and Telecommunications"
					// case 0x1400: break; // "HRT - Croatian Radio and Television"
					// case 0x1500: break; // "IBM"
					// case 0x1600: break; // "Nera"
					case 0x1702:  // "Betacrypt (BetaTechnik) (C)"
					case 0x1708:  // "Betacrypt 2 (BetaTechnik)"
					case 0x1722:  // "Betacrypt (BetaTechnik) (D)"
					case 0x1762:  // "Betacrypt (BetaTechnik) (F)"
					case 0x1700: beta = true; break; // "Betacrypt (BetaTechnik)"
					case 0x1800: nagra = true; break; // "NagraVisionKudelski SA"
					// case 0x1900: break;  // "Titan Information Systems"
					// case 0x2000: break;  // "Telefonica Servicios Audiovisuales"
					// case 0x2100: break;  // "STENTOR (France Telecom, CNES and DGA)"
					// case 0x2200: break;  // "Scopus Network Technologies"
					// case 0x2300: break;  // "BARCO AS"
					// case 0x2400: break;  // "Starguide Digital Networks"
					// case 0x2500: break;  // "Mentor Data Systems, Inc."
					// case 0x2600: break;  // "European Broadcasting Union"
					// case 0x4700: break;  // "General Instrument"
					// case 0x4800: break;  // "Telemann"
					// case 0x4900: break;  // "Digital TV Industry Alliance of China"
					// case 0x4A00: break;  // "Tsinghua TongFang"
					// case 0x4A10: break;  // "Easycas"
					// case 0x4A20: break;  // "AlphaCrypt"
					// case 0x4A30: break;  // "DVN Holdings"
					// case 0x4A40: break;  // "Shanghai Advanced Digital Technology Co. Ltd."
					// case 0x4A50: break;  // "Shenzhen Kingsky Company (China) Ltd."
					// case 0x4A60: break;  // "@Sky"
					case 0x4A61: icecr = true; break;  // "@Sky ??"
					case 0x4A70:   // "Dream Multimedia TV (DreamCrypt)"
					case 0x4A71:   // "Dream Multimedia TV (High security, 4096bit RSA)"
					case 0x4A72:   // "Dream Multimedia TV (Consumer, 48bit)"
					case 0x4A73:   // "Dream Multimedia TV (non-DVB)"
					case 0x4A74: dreamcr = true; break;  // "Dream Multimedia TV (export)"
					// case 0x4A80: break;  // "THALESCrypt"
					// case 0x4A90: break;  // "Runcom Technologies"
					// case 0x4AA0: break;  // "SIDSA"
					// case 0x4AB0: break;  // "Beijing Compunicate Technology Inc."
					// case 0x4AC0: break;  // "Latens Systems Ltd"
					// case 0x4AD0: break;  // "XCrypt Inc."
					// case 0x4AD2: break;  // "Beijing Digital Video Technology Co., Ltd."
					// case 0x0000: break;  // "other/unknown"
				}
			}
			int cavalue = eStreaminfo::getInstance()->getCAValue(usedcaid);
			switch (cavalue) {
				case 0x0100: secaUsed = true; break; // "Seca/Mediaguard (Canal Plus)"
				// case 0x0200: break; // "CCETT"
				// case 0x0300: break; // "MSG MediaServices GmbH"
				// case 0x0400: break; // "Eurodec"
				case 0x0500: viaUsed = true; break; // "Viaccess (France Telecom)"
				case 0x0600: irdetoUsed = true; break;  // "Irdeto"
				// case 0x0700: break; // "Jerrold/GI/Motorola"
				// case 0x0800: break; // "Matra Communication"
				case 0x0900: ndsUsed = true; break;// "Videoguard (News Datacom)"
				// case 0x0A00: break; // "Nokia"
				case 0x0B00: conaxUsed = true; break; // "Conax (Norwegian Telekom)"
				// case 0x0C00: break; // "NTL"
				case 0x0D00: cwUsed = true; break; // "Cryptoworks (Philips)"
				case 0x0E00: powervuUsed = true; break; // "Power VU (Scientific Atlanta)"
				// case 0x0F00: break; // "Sony"
				// case 0x1000: break; // "Tandberg Television"
				// case 0x1100: break; // "Thomson"
				// case 0x1200: break; // "TV/Com"
				// case 0x1300: break; // "HPT - Croatian Post and Telecommunications"
				// case 0x1400: break; // "HRT - Croatian Radio and Television"
				// case 0x1500: break; // "IBM"
				// case 0x1600: break; // "Nera"
				case 0x1702:  // "Betacrypt (BetaTechnik) (C)"
				case 0x1708:  // "Betacrypt 2 (BetaTechnik)"
				case 0x1722:  // "Betacrypt (BetaTechnik) (D)"
				case 0x1762:  // "Betacrypt (BetaTechnik) (F)"
				case 0x1700: betaUsed = true; break; // "Betacrypt (BetaTechnik)"
				case 0x1800: nagraUsed = true; break; // "NagraVisionKudelski SA"
				// case 0x1900: break;  // "Titan Information Systems"
				// case 0x2000: break;  // "Telefonica Servicios Audiovisuales"
				// case 0x2100: break;  // "STENTOR (France Telecom, CNES and DGA)"
				// case 0x2200: break;  // "Scopus Network Technologies"
				// case 0x2300: break;  // "BARCO AS"
				// case 0x2400: break;  // "Starguide Digital Networks"
				// case 0x2500: break;  // "Mentor Data Systems, Inc."
				// case 0x2600: break;  // "European Broadcasting Union"
				// case 0x4700: break;  // "General Instrument"
				// case 0x4800: break;  // "Telemann"
				// case 0x4900: break;  // "Digital TV Industry Alliance of China"
				// case 0x4A00: break;  // "Tsinghua TongFang"
				// case 0x4A10: break;  // "Easycas"
				// case 0x4A20: break;  // "AlphaCrypt"
				// case 0x4A30: break;  // "DVN Holdings"
				// case 0x4A40: break;  // "Shanghai Advanced Digital Technology Co. Ltd."
				// case 0x4A50: break;  // "Shenzhen Kingsky Company (China) Ltd."
				// case 0x4A60: break;  // "@Sky"
				case 0x4A61: icecrUsed = true; break;  // "@Sky ??"
				case 0x4A70:   // "Dream Multimedia TV (DreamCrypt)"
				case 0x4A71:   // "Dream Multimedia TV (High security, 4096bit RSA)"
				case 0x4A72:   // "Dream Multimedia TV (Consumer, 48bit)"
				case 0x4A73:   // "Dream Multimedia TV (non-DVB)"
				case 0x4A74: dreamcrUsed = true; break;  // "Dream Multimedia TV (export)"
				// case 0x4A80: break;  // "THALESCrypt"
				// case 0x4A90: break;  // "Runcom Technologies"
				// case 0x4AA0: break;  // "SIDSA"
				// case 0x4AB0: break;  // "Beijing Compunicate Technology Inc."
				// case 0x4AC0: break;  // "Latens Systems Ltd"
				// case 0x4AD0: break;  // "XCrypt Inc."
				// case 0x4AD2: break;  // "Beijing Digital Video Technology Co., Ltd."
				// case 0x0000: break;  // "other/unknown"
			}
		}
	}
	
	// printf("showECM: irdeto=%d\n", irdeto ? 1 : 0);
	// printf("showECM: seca=%d\n", seca ? 1 : 0);
	// printf("showECM: via=%d\n", via ? 1 : 0);
	// printf("showECM: nagra=%d\n", nagra ? 1 : 0);
	// printf("showECM: cw=%d\n", cw ? 1 : 0);
	// printf("showECM: nds=%d\n", nds ? 1 : 0);
	// printf("showECM: conax=%d\n", conax ? 1 : 0);
	// printf("showECM: beta=%d\n", beta ? 1 : 0);
	if (irdeto) {
		if (irdetoUsed) { IrdetoUca->show() ; IrdetoEcm->hide(); }
		else            { IrdetoUca->hide() ; IrdetoEcm->show(); }
		IrdetoNo->hide();
	}
	else { IrdetoUca->hide(); IrdetoEcm->hide(); IrdetoNo->show(); }
	if (seca) {
		if (secaUsed) { SecaUca->show() ; SecaEcm->hide(); }
		else          { SecaUca->hide() ; SecaEcm->show(); }
		SecaNo->hide();
	}
	else { SecaUca->hide(); SecaEcm->hide(); SecaNo->show(); }
	if (via) {
		if (viaUsed) { ViaUca->show() ; ViaEcm->hide(); }
		else         { ViaUca->hide() ; ViaEcm->show(); }
		ViaNo->hide();
	}
	else { ViaUca->hide(); ViaEcm->hide(); ViaNo->show(); }

	if (nagra) {
		if (nagraUsed) { NagraUca->show() ; NagraEcm->hide(); }
		else           { NagraUca->hide() ; NagraEcm->show(); }
		NagraNo->hide();
	}
	else { NagraUca->hide(); NagraEcm->hide(); NagraNo->show(); }
	if (cw) {
		if (cwUsed) { CWUca->show() ; CWEcm->hide(); }
		else        { CWUca->hide() ; CWEcm->show(); }
		CWNo->hide();
	}
	else { CWUca->hide(); CWEcm->hide(); CWNo->show(); }
	if (nds) {
		if (ndsUsed) { NDSUca->show() ; NDSEcm->hide(); }
		else         { NDSUca->hide() ; NDSEcm->show(); }
		NDSNo->hide();
	}
	else { NDSUca->hide(); NDSEcm->hide(); NDSNo->show(); }
	if (conax) {
		if (conaxUsed) { ConaxUca->show() ; ConaxEcm->hide(); }
		else           { ConaxUca->hide() ; ConaxEcm->show(); }
		ConaxNo->hide();
	}
	else { ConaxUca->hide(); ConaxEcm->hide(); ConaxNo->show(); }
	if (beta) {
		if (betaUsed) { BetaUca->show() ; BetaEcm->hide(); }
		else          { BetaUca->hide() ; BetaEcm->show(); }
		BetaNo->hide();
	}
	else { BetaUca->hide(); BetaEcm->hide(); BetaNo->show(); }
	if (powervu) {
		if (powervuUsed) { PowerVuUca->show() ; PowerVuEcm->hide(); }
		else             { PowerVuUca->hide() ; PowerVuEcm->show(); }
		PowerVuNo->hide();
	}
	else { PowerVuUca->hide(); PowerVuEcm->hide(); PowerVuNo->show(); }
	if (dreamcr) {
		if (dreamcrUsed) { DreamCrUca->show() ; DreamCrEcm->hide(); }
		else             { DreamCrUca->hide() ; DreamCrEcm->show(); }
		DreamCrNo->hide();
	}
	else { DreamCrUca->hide(); DreamCrEcm->hide(); DreamCrNo->show(); }
	if (ruscr) {
		if (ruscrUsed) { RusCrUca->show() ; RusCrEcm->hide(); }
		else           { RusCrUca->hide() ; RusCrEcm->show(); }
		RusCrNo->hide();
	}
	else { RusCrUca->hide(); RusCrEcm->hide(); RusCrNo->show(); }
	if (icecr) {
		if (icecrUsed) { IceCrUca->show() ; IceCrEcm->hide(); }
		else           { IceCrUca->hide() ; IceCrEcm->show(); }
		IceCrNo->hide();
	}
	else { IceCrUca->hide(); IceCrEcm->hide(); IceCrNo->show(); }
	if (codicr) {
		if (codicrUsed) { CodiCrUca->show() ; CodiCrEcm->hide(); }
		else            { CodiCrUca->hide() ; CodiCrEcm->show(); }
		CodiCrNo->hide();
	}
	else { CodiCrUca->hide(); CodiCrEcm->hide(); CodiCrNo->show(); }

}
/* CA ECM DISPLAY end */

void eZapMain::SoftcamNameChanged(const char *newname)
{
	if (newname)
		strncpy(softcamName, newname, sizeof(softcamName));
	softcamName[sizeof(softcamName) - 1] = 0;
	SoftCam->setText(softcamName);
}

void eZapMain::usedCaidChanged(int newcaid)
{
	currentcaid = newcaid;
	showECM(currentcaid);
}

void eZapMain::SoftcamInfoChanged(const char *newSoftcamInfo)
{
        if (newSoftcamInfo)
                strncpy(softcamInfo, newSoftcamInfo, sizeof(softcamInfo));
        softcamInfo[sizeof(softcamInfo) - 1] = 0;
        SoftcamInfo->setText(softcamInfo);
}

void eZapMain::StartInfoBarTimer()
{
	if ( doubleklickTimer.isActive() )
	{
		doubleklickTimer.stop();
		doubleklickTimerConnection.disconnect();
		showServiceSelector( -1, state&stateRecording ? 0 : pathBouquets );
	} else {
		doubleklickTimer.start(500,true);
		doubleklickTimerConnection = CONNECT( doubleklickTimer.timeout, eZapMain::myshowInfobar );
	}
}

void eZapMain::myshowInfobar()
{
	showInfobar();
}

void eZapMain::showInfobar(bool startTimeout)
{
#if 0
	int tmp = 0;
	eConfig::getInstance()->getKey("/enigma/timeshift/activatepausebutton", tmp );
	unsigned char pause = (unsigned char) tmp;
	AudioOrPause->setText(pause ? _("Pause") : _("audio track"));
#endif

	YellowButtonDesc->setText(KeyMapping::getShortButtonDescription("/pli/keyMapping/yellowButton"));
	BlueButtonDesc->setText(KeyMapping::getShortButtonDescription("/pli/keyMapping/blueButton"));
	GreenButtonDesc->setText(KeyMapping::getShortButtonDescription("/pli/keyMapping/greenButton"));

	int v;
	int minizap_mode = 0;

	/*
	 * A note about the stateOSD values:
	 * 0 = disabled, or non-permanent OSD (activated automatically, disappears after timeout)
	 * 1 = permanent OSD, activated manually, and normally doesn't disappear after timeout (unless enableAutohideOSDOn is set)
	 * 2 = same as 0, but used for minizap to allow the showInfobar event to be captured, even when isVisible() is true
	 */

	if (stateOSD != 1)
	{
		if (miniZap->size())
		{
			minizap_mode = 1;
			eConfig::getInstance()->getKey("/ezap/osd/miniZap", minizap_mode);
			if (minizap_mode)
			{
				stateOSD = 2;
			}
		}
	}
	
	if (minizap_mode)
	{
		OSDExtra->hide();
		OSDVerbose->hide();

		maxiZap->hide();
		miniZap->show();
	}
	else
	{
		miniZap->hide();
		maxiZap->show();

		v = 1;
		eConfig::getInstance()->getKey("/ezap/osd/OSDExtraInfo", v);
		if (stateOSD != 1 && !dvrfunctions)
		{
			eConfig::getInstance()->getKey("/ezap/osd/OSDExtraInfoOnZap", v);
		}
	
		if (v)
		{
			OSDExtra->show();
			// eDebug("[showInfoBar] Extra info on\n");
		}
		else
		{
			OSDExtra->hide();
			// eDebug("[showInfoBar] Extra info off\n");
		}
	
		v = 1;
		eConfig::getInstance()->getKey("/ezap/osd/OSDVerboseInfo", v);
		if (stateOSD != 1 && !dvrfunctions)
		{
			eConfig::getInstance()->getKey("/ezap/osd/OSDVerboseInfoOnZap", v);
		}
	
		if (v)
		{
			OSDVerbose->show();
			// eDebug("[showInfoBar] Verbose info on\n");
		}
		else
		{
			OSDVerbose->hide();
			// eDebug("[showInfoBar] Verbose info off\n");
		}
	}

	if ( !isVisible() && eApp->looplevel() == 1 &&
		( !currentFocus || currentFocus == this ) )
	{
		show();
	}

	if (mute.isVisible())
	{
		/*
		 * The mute pixmap might be an integral part of the osd, in which case the z-order could be wrong now.
		 * Redraw the mute pixmap to ensure it appears on top.
		 */
		mute.invalidate();
	}

	if (startTimeout && doHideInfobar())
	{
		eConfig::getInstance()->getKey("/enigma/timeoutInfobar", timeoutInfobar);
		timeout.start((timeoutInfobar * 1000), 1);
	}

/* SNR,AGC DISPLAY begin */
/* SNR,AGC Display function call */
	if (eZapMain::getInstance()->getMode() != eZapMain::modeFile)
	{
		if(!snrTimer)
		{
			snrTimer = new eTimer(eApp);
			snrTimer->start(1000,false);
			CONNECT(snrTimer->timeout,eZapMain::showSNR);
		}
	}
/* SNR,AGC DISPLAY end */
}

void eZapMain::hideInfobar()
{
	if (doHideInfobar())
	{
		timeout.stop();
		hide();
	}
/* SNR,AGC DISPLAY begin */
	if (snrTimer)
	{
		delete snrTimer;
		snrTimer = 0;
	}
/* SNR,AGC DISPLAY end */
}

#ifndef DISABLE_FILE
void eZapMain::play()
{
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler)
		return;

	// disable skipping
	if(skipping)
		endSkip();

	switch (handler->getState())
	{
		case eServiceHandler::statePause:
			pause();
			break;
		case eServiceHandler::stateStopped:
			if ( mode == modeFile )
				playService( eServiceInterface::getInstance()->service, psDontAdd );
		default:
			break;
	}
	updateProgress();
}

void eZapMain::stop()
{
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler)
		return;

	// disable skipping
	if(skipping)
		endSkip();

	if (indices_enabled)
	{
		int real = handler->getPosition(eServiceHandler::posQueryRealCurrent);
		int time = handler->getPosition(eServiceHandler::posQueryCurrent);
		/* add a 'stop/pause' marker, to continue on the same point next time */
		indices.add(real, time, 1);
	}

	handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 0));
	handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekAbsolute, 0));
	updateProgress();
}

void eZapMain::pause()
{
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler)
		return;

	// disable skipping
	if(skipping)
		endSkip();

	eServiceReference &ref = eServiceInterface::getInstance()->service;
	if (handler->getState() == eServiceHandler::statePause)
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 1));
	else
	{
		if (indices_enabled)
		{
			int real = handler->getPosition(eServiceHandler::posQueryRealCurrent);
			int time = handler->getPosition(eServiceHandler::posQueryCurrent);
			/* add a 'stop/pause' marker, to continue on the same point next time */
			indices.add(real, time, 1);
		}

		if ( ref.type == eServiceReference::idDVB && !ref.path && !timeshift )
		{
			if (state & recPermanentTimeshift) // permanent timeshift
			{
				Decoder::Pause(2);  // freeze frame
				Decoder::setAutoFlushScreen(0);
				handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 0));
				Decoder::setAutoFlushScreen(1);
			}
			else if (eSystemInfo::getInstance()->canTimeshift())
			{
				if (!eDVB::getInstance()->recorder)
				{
					switch(eSystemInfo::getInstance()->getHwType())
					{
					case eSystemInfo::DM600PVR:
					case eSystemInfo::DM500PLUS:
						Decoder::setAutoFlushScreen(0);
						Decoder::Pause(2);  // freeze frame
						record();
						break;
					default:
						record();
						Decoder::setAutoFlushScreen(0);
						Decoder::Pause(2);  // freeze frame
						break;
					}
					timeshift=1;
					handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, -1));
					Decoder::setAutoFlushScreen(1);
				}
				else
				{
					Decoder::Pause(2);  // freeze frame
					Decoder::setAutoFlushScreen(0);
					handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 0));
					Decoder::setAutoFlushScreen(1);
				}
			}
		}
		else
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 0));
	}
}

void eZapMain::record()
{
	// disable skipping
	if(skipping)
		endSkip();

	if ( state & stateRecording )
	{
		if ( state & stateInTimerMode )
			eTimerManager::getInstance()->abortEvent( ePlaylistEntry::errorUserAborted );
		else
			recordDVR(0,1);
	}
	else
		recordDVR(1, 1);
}

int freeRecordSpace(bool ofParent)
{
	struct statfs s;
	int free;
	eString movieLocation;
	
	eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaMovies, movieLocation);
	
	if(ofParent)
	{
		// Strip "/movie" to get the parent directory
		int postfix = movieLocation.rfind("/movie");
		movieLocation = movieLocation.left(postfix);
	}
	
	if (statfs(movieLocation.c_str(), &s)<0)
		free=-1;
	else
		free=s.f_bfree/1000*s.f_bsize/1000;
	return free;
}

int eZapMain::recordDVR(int onoff, int user, time_t evtime, const char *timer_descr )
{
	// disable skipping
	if(skipping)
		endSkip();

	if (onoff) //  start recording
	{
		int tryCount = 1;
		
		stopPermanentTimeshift();
		
		while(tryCount <= 2)
		{
			// If movie directory not found, but parent directory has minimal 32MB (to prevent writing in flash)
			if((freeRecordSpace() < 0) && (freeRecordSpace(true) >= 32))
			{
				eString movieDirectory;
				eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaMovies, movieDirectory);
				movieDirectory = "mkdir " + movieDirectory;
				eDebug("eZapMain::recordDVR executed '%s' to create movie directory", movieDirectory.c_str());
				system(movieDirectory.c_str());
			}
			
			// If enough space (32MB or more), stop the tries
			if(freeRecordSpace() >= 32)
			{
				break;
			}
			
			// If less than 32MB in movie directory (or could not be created)
			if(tryCount == 1)
			{
				// Fall back to internal harddisk if available
				int HDDAvailable = 0;
				eConfig::getInstance()->getKey("/pli/HDDAvailable", HDDAvailable);

				// If there's a real physical harddisk, choose this one for recordings
				if(HDDAvailable)
				{
					eMediaMapping::getInstance()->setStorageLocation(eMediaMapping::mediaMovies, "/media/hdd/movie");
				}
			}
			
			// Already second try and still no success. Recording is impossible
			if(tryCount == 2)
			{
				handleServiceEvent(eServiceEvent(eServiceEvent::evtRecordFailed));
				return -3;
			}
			
			++tryCount;
		}
		
		if (state & stateRecording)
		{
			eDebug("hmm already recording in progress... try to stop.. but this should not happen\n");
			if ( state & stateInTimerMode )
			{
				eDebug("abort timer recording....:(\n");
				eTimerManager::getInstance()->abortEvent( ePlaylistEntry::errorUserAborted );
			}
			else
			{
				eDebug("abort recording....:(\n");
				recordDVR(0, 0); // try to stop recording.. should not happen
			}
		}
		if (state & stateRecording)
		{
			eDebug("after stop recording always record is running???\n");
			return -2; // already recording
		}

		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (!handler)
			return -1;
		else if ( handler->getID() != eServiceReference::idDVB )
			return -1;

		eServiceReference &ref_= eServiceInterface::getInstance()->service;

		if ( (ref_.type != eServiceReference::idDVB) ||
				(ref_.data[0] < 0) || ref_.path.size())
		{
			if (user)
			{
				eMessageBox::ShowBox(_("Sorry, you cannot record this service."), _("record"), eMessageBox::iconWarning|eMessageBox::btOK );
			}
			return -1; // cannot record this service
		}

		eServiceReferenceDVB &ref=(eServiceReferenceDVB&)ref_;
		// build filename
		eString servicename, descr/* = _("no description available")*/;

		eService *service=0;

		if ( ref.getServiceType() > 4 && !timer_descr )  // nvod or linkage
			service = eServiceInterface::getInstance()->addRef(refservice);
		else
			service = eServiceInterface::getInstance()->addRef(ref);

		if (service)
		{
			servicename = service->service_name;
			static char strfilter[4] = { 0xC2, 0x87, 0x86, 0x00 };
			// filter short name brakets...
			for (eString::iterator it(servicename.begin()); it != servicename.end();)
				strchr( strfilter, *it ) ? it = servicename.erase(it) : it++;

			if ( ref.getServiceType() > 4 && !timer_descr )  // nvod or linkage
				eServiceInterface::getInstance()->removeRef(refservice);
			else
				eServiceInterface::getInstance()->removeRef(ref);
		}

		if ( timer_descr )
		{
//			eDebug("timer_descr = %s", timer_descr );
			descr = timer_descr;
		}
		else if (cur_event_text.length())
		{
//			eDebug("no timer_descr");
//			eDebug("use cur_event_text %s", cur_event_text.c_str() );
			descr = cur_event_text;
		}
//		else
//			eDebug("no timer_descr");

		eString filename;
		if ( servicename )
		{
//			eDebug("we have servicename... sname + \" - \" + descr(%s)",descr.c_str());
			// append date..
			filename = servicename;

// temporary gone, test in PLi image
//			if ( descr )
//				filename += " - " + descr;
//			filename = servicename + " - " + descr;

			time_t now = time(0)+eDVB::getInstance()->time_difference;
			tm nowTime = *localtime(&now);
// temporary gone, test in PLi image
//			filename += eString().sprintf(" - %02d.%02d.%02d", nowTime.tm_mday, nowTime.tm_mon+1, nowTime.tm_year%100 );
			filename += eString().sprintf("-%02d.%02d.%04d-%02d:%02d", nowTime.tm_mday, nowTime.tm_mon+1, nowTime.tm_year+1900, nowTime.tm_hour, nowTime.tm_min );
			if ( descr )
				filename += "-" + descr;
		}
		else
		{
//			eDebug("filename = descr = %s", descr.c_str() );
			filename = descr.left(100);
		}
		filename = filename.left(100);

		eString cname="";

		for (unsigned int i=0; i<filename.length(); ++i)
		{
// temporary gone, test in PLi image
//			if (strchr("abcdefghijklkmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_- ()", filename[i]) || (filename[i] & 0x80)) 	// allow UTF-8
			if (strchr("abcdefghijklkmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890-()", filename[i]) || (filename[i] & 0x80)) 	// allow UTF-8
				cname+=filename[i];
			else
				cname+='_';
		}

		int suffix=0;
		struct stat64 s;
		do
		{
			eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaMovies, filename);
			filename+="/";
			filename+=cname;
			if (suffix)
				filename+=eString().sprintf(" [%d]", suffix);
			suffix++;
			filename+=".ts";
		} while (!stat64(filename.c_str(), &s));

		if (handler->serviceCommand(
			eServiceCommand(
				eServiceCommand::cmdRecordOpen,
				reinterpret_cast<int>(
						strcpy(new char[strlen(filename.c_str())+1], filename.c_str())
					)
				)
			))
		{
			eDebug("couldn't record... :/");
			return -4; // couldn't record... warum auch immer.
		} else
		{
			eServiceReference ref2=ref;

  // no faked service types in recordings.epl
			if ( ref2.data[0] & 1 )  // tv or faked service type(nvod,linkage)
				ref2.data[0] = 1;  // this saved as tv service...

			ref2.path=filename;
			ref2.descr=descr;
			ref2.descr.strReplace("\n", " ");
			ePlaylistEntry en(ref2);
			en.type=ePlaylistEntry::PlaylistEntry|ePlaylistEntry::boundFile;
			eDebug("First re-read recordings file");
			recordings->lockPlaylist();
			loadRecordings();
			eDebug("Add recording to the list");
			recordings->getList().push_back(en); // add to playlist
			eDebug("Saving recordings");
			saveRecordings();
			recordings->unlockPlaylist();
			state |= (stateRecording|recDVR);
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordStart));
			if (servicename)
				recchannel->setText(servicename.c_str());
			else
				recchannel->setText("Unknown");
			recstatus->show();
			recchannel->show();

			DVRSpaceLeft->show();

			recStatusBlink.start(500, 1);
			eZap::getInstance()->getServiceSelector()->actualize();

			{
				eventDataPtr d = eEPGCache::getInstance()->getEventDataPtr( (eServiceReferenceDVB&)eServiceInterface::getInstance()->service, evtime );;
				const eit_event_struct *event=0;
				EIT *eit=0;
				if ( evtime && d )
				{
					event = d->get();
					/* NOTE: event is NOT valid outside the scope of 'd'! */
				}

				if ( !event )
				{
					eServiceHandler *service=eServiceInterface::getInstance()->getService();
					if (service)
					{
						eit=service->getEIT();
						if ( eit && eit->ready && !eit->error )
						{
							int p=0;
							for (ePtrList<__u8>::iterator e(eit->eventsPlain); e != eit->eventsPlain.end(); ++e)
							{
								eit_event_struct *evt = (eit_event_struct*) *e;
								if ((evt->running_status>=2) || (!p && !evt->running_status))		// currently running service
									event = evt;
								p++;
							}
						}
					}
				}
				if ( event ) // then store
				{
					filename.erase(filename.length()-2, 2);
					filename+="eit";
					int fd = open(filename.c_str(), O_CREAT|O_WRONLY, 0777);
					if (fd>-1)
					{
						int evLen=HILO(event->descriptors_loop_length)+12/*EIT_LOOP_SIZE*/;
						int wr = ::write( fd, (unsigned char*)event, evLen );
						if (  wr != evLen )
							eDebug("eit write error (%m)");
						::close(fd);
					}
				}
				if (eit)
					eit->unlock();
			} /* end scope of 'd' */
		}
		return 0;
	}
	else   // stop recording
	{
		if(ENgrab::nGrabActive) //is running nGrab
		{
			stopNGrabRecord();
			return 0;
		}
		eServiceHandler *handler=eServiceInterface::getInstance()->getServiceHandler(eServiceReference::idDVB);
		if (!handler)
			return -1;
		state &= ~(stateRecording|recDVR);
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordStop));
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordClose));

		DVRSpaceLeft->hide();
		recStatusBlink.stop();

		recstatus->hide();
		recchannel->hide();
		recchannel->setText("");

		beginPermanentTimeshift();
#ifndef DISABLE_LCD
		// if standby disable lcdMain
		if(state & stateSleeping)
		{
			lcdmain.lcdMain->hide();
			lcdmain.lcdStandby->show();
		}

		// disable lcd-blink
		lcdmain.lcdMain->Clock->show();
#endif

		eZap::getInstance()->getServiceSelector()->actualize();

		int profimode=0;
		eConfig::getInstance()->getKey("/elitedvb/extra/profimode", profimode);

		if (user)
		{
			if (!profimode)
			{
				int ret = eMessageBox::ShowBox(_("Show recorded movies?"), _("recording finished"),  eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btNo);
				if ( ret == eMessageBox::btYes )
					showServiceSelector( eServiceSelector::dirLast, pathRecordings );
			}
		}
		return 0;
	}
}

void eZapMain::startSkip(int dir)
{
	eServiceHandler *handler=NULL;
	int firstskip = !skipping;

	if(!skipping) // first call?
	{
		skipcounter=0;
		skipspeed=0;
		handler=eServiceInterface::getInstance()->getService();
		if (handler)
		{
			switch (handler->getState())
			{
				case eServiceHandler::statePause:
					pause();// continue playing in preparation for skipping
					break;
				case eServiceHandler::stateStopped:
					return;
				default:
					break;
			}

			if(!eServiceInterface::getInstance()->service.path)
			{
				if ( !eSystemInfo::getInstance()->canTimeshift() )
					return;
				if (state & (stateRecording | recPermanentTimeshift) && eDVB::getInstance()->recorder
					&& eDVB::getInstance()->recorder->recRef == eServiceInterface::getInstance()->service )
				{
					if (!timeshift && dir!=skipForward)
					{ // necessary to enable skipmode, because file end
						handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip,-1000));
						usleep(100*1000);
						handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip,-1000));
						usleep(100*1000);
						handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip,-1000));
						timeshift=1;
					}
					else if (!timeshift)
						return;
				}
				else if (!timeshift)
					return;
			}
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekBegin, 0));
			seekstart=handler->getPosition(eServiceHandler::posQueryCurrent); // Startpunkt fr Zeitausgabe
			seekpos=seekstart;
		}
	}

	if (dir == skipForward)
		skipspeed++;
	else
		skipspeed--;

	if (skipspeed > 4)
		skipspeed=4;
	else if (skipspeed < -4)
		skipspeed=-4;

	if(!skipping && handler)
	{
		if(!skipTimer)
		{
			skipTimer=new eTimer(eApp);
			skipTimer->start(250,false);
			CONNECT(skipTimer->timeout,eZapMain::skipLoop); // enable timer
		}
		if(!skipWidget) // enable view
		{
			int fsize=eSkin::getActive()->queryValue("fontsize", 20)+4;
			skipWidget=new eWidget();
			skipWidget->move(ePoint(0,160));
			skipWidget->resize(eSize(150, (fsize<<1)+12));
			skipLabel1=new eLabel(skipWidget);
			skipLabel1->setAlign(eTextPara::dirRight);
			skipLabel1->setText((skipspeed<0)?"<<":">>");
			skipLabel1->move(ePoint(30,4));
			skipLabel1->resize(eSize(110,fsize));
			skipLabel2=new eLabel(skipWidget);
			skipLabel2->setAlign(eTextPara::dirRight);
			skipLabel2->setText((dir==skipForward)?"+00:00:00":"-00:00:00");
			skipLabel2->move(ePoint(30,fsize+8));
			skipLabel2->resize(eSize(110,fsize));
			skipWidget->show();
		}
		skipping=1;
	}

	if(skipWidget && skipLabel1)
	{
		eString s;
		s=(skipspeed<0)?"<<":">>";
		switch(abs(skipspeed))
		{
			case 1: s=s+"  8x"; break; // very estimate :-)
			case 2: s=s+" 16x"; break;
			case 3: s=s+" 32x"; break;
			case 4: s=s+" 64x"; break;
		}
		if(timeshift)
			s="(ts) "+s;
		skipLabel1->setText(s);
	}
	int showosd = 1;
	eConfig::getInstance()->getKey("/ezap/osd/showOSDOnSwitchService", showosd );
	if (firstskip && showosd)
		showInfobar();
	if ( timeout.isActive() )
		timeout.stop();
}

void eZapMain::skipLoop()
{
	// called from skipTimer (eTimer)

	int time,speed,faktor,pos,diff,ts;

	faktor = (skipspeed<0) ? -1 : 1;
	speed = skipspeed * faktor;

	switch(speed)
	{
		case  1:
			time = (skipspeed<0) ? 4 : 1;
			break; //Seconds
		case  2:
			time = (skipspeed<0) ? 6 : 2;
			break; //back must more!
		case  3:
			time = (skipspeed<0) ? 10 : 6;
			break;
		case  4:
			time = (skipspeed<0) ? 18 : 14;
			break;
		default:
			time = 0;
	}

	if(time)
	{
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (handler)
		{
			// view relative skip time
			pos=handler->getPosition(eServiceHandler::posQueryCurrent);
			diff=abs(pos-seekstart);
			int std=diff/3600;
			int min=(diff-(std*3600))/60;
			int sec=diff-(std*3600)-(min*60);

			if(skipLabel2)
				if(!(pos>seekstart && skipspeed<0))
					skipLabel2->setText(eString().sprintf("%c%02d:%02d:%02d",(pos>seekstart)?'+':'-',std,min,sec));

			if(skipspeed<0 && pos < (time<<2) ) //back and begin reached
			{
				endSkip();
				handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekReal, 0));
				updateProgress();
				return;
			}

			ts = 0;
			eServiceReference &ref = eServiceInterface::getInstance()->service;
			if(!(ref.type == eServiceReference::idUser &&
				((ref.data[0] ==  eMP3Decoder::codecMPG) ||
				 (ref.data[0] ==  eMP3Decoder::codecMP3) ||
				 (ref.data[0] ==  eMP3Decoder::codecFLAC) ||
				 (ref.data[0] ==  eMP3Decoder::codecOGG) )))
				ts = 1;

            if (skipspeed == 1 && ts)
            	return; // normal trickmode forward (ts only)
			
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip,time*(ts?1000:2000)*faktor));

			seekpos=pos;
		}
		else
			endSkip();
		updateProgress();
	}
	else
		endSkip();
}

void eZapMain::repeatSkip(int dir)
{
	static int aktiv;

	if (skipping)
		endSkip();	// break skipping, if active
	else
		return;

	if(aktiv)
		return;		// not more...

	eServiceReference &ref = eServiceInterface::getInstance()->service;

	if (ref.type == eServiceReference::idUser &&
		( (ref.data[0] != eMP3Decoder::codecMPG) &&
			(ref.data[0] != eMP3Decoder::codecMP3) &&
			(ref.data[0] != eMP3Decoder::codecFLAC) &&
			(ref.data[0] != eMP3Decoder::codecOGG) ) )
		return;

	aktiv=1;

	// Enter distance in minutes
	SkipEditWindow dlg( (dir == skipForward) ? ">> Min:" : "<< Min:" );
	if (ref.type == eServiceReference::idUser && 
		(ref.data[0] == eMP3Decoder::codecMP3 ||
		ref.data[0] == eMP3Decoder::codecFLAC ||
		ref.data[0] == eMP3Decoder::codecOGG) )
		dlg.setEditText("1"); // pre value (advertising) :-)
	else
		dlg.setEditText("5"); // pre value (advertising) :-)

	dlg.show();
	int ret=dlg.exec();
	dlg.hide();

	if(!ret)
	{
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (handler)
		{
			int time=atoi(dlg.getEditText().c_str())*60; // Seconds

			if(dir!=skipForward)
				time = -time;

			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekBegin));
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip,time*1000));
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekEnd));

			updateProgress();

			if (isVisible() && doHideInfobar())
				timeout.start(2000, 1);
		}
	}
	aktiv=0;
}

void eZapMain::stopSkip(int dir)
{
	// release key doesn't need...
	(void)dir;
}

void eZapMain::endSkip(void)
{
	// breaks the timed skipping (eTimer)

	if(skipTimer)
	{
		delete skipTimer;
		skipTimer=0;
	}

	if(skipWidget)
	{
		delete skipWidget;
		skipWidget=0;
	}
	skipspeed=0;
	skipping=0;

	eServiceHandler *handler=eServiceInterface::getInstance()->getService();

	if (handler)
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekEnd));

	if (isVisible() && doHideInfobar())
		timeout.start(2000, 1);
}
#endif //DISABLE_FILE

int eZapMain::handleStandby(int i)
{
	int force=0;

	if ( i <= 0 )
	{
		if ( state & stateSleeping )
		{
			if ( !i )
			{
				// this breakes the eZapStandby mainloop...
				// and enigma wakes up
				eZapStandby::getInstance()->wakeUp(1);
			}
			else  // no real wakeup.... dvr timer do wakeup only tuner and hdd...
			{
				struct stat s;
				if (!::stat("/var/etc/enigma_leave_idle.sh", &s))
					system("/var/etc/enigma_leave_idle.sh");
			}
			wasSleeping=3;
		}
		return 0;
	}
	else if ( i == 1 ) // get wasSleeping state
	{
		int tmp = wasSleeping;
		wasSleeping=0;
		return tmp;
	}
	else switch(i) // before record we was in sleep mode...
	{
		case 4: // force .. ( Sleeptimer )
			force=1;
		case 2: // complete Shutdown
		{
			if ( !force && (eServiceInterface::getInstance()->service.path || timeshift) )
				break;
			if ( ePluginThread::getInstance() ) // don't shutdown, when a plugin is running
				break;
			int ret = eMessageBox::ShowBox(_("Shutdown your Receiver now?"),_("Timer Message"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btYes, 30);
			if (ret == eMessageBox::btYes)
			{
				stopPermanentTimeshift();
		// we do hardly shutdown the box..
		// any pending timers are ignored
			}
				eZap::getInstance()->quit();
			break;
		}
		case 6: // force .. ( Sleeptimer )
			force=1;
		case 3: // immediate go to standby
		{
			if ( !force && (eServiceInterface::getInstance()->service.path || timeshift) )
				break;
			if ( ePluginThread::getInstance() ) // don't goto standby, when a plugin is running
				break;
			if ( !eZapStandby::getInstance() )
			{
				int ret = eMessageBox::ShowBox(_("Go to Standby now?"),_("Timer Message"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btYes, 30 );
				if (ret == eMessageBox::btYes)
				{
					stopPermanentTimeshift();
					// use message_notifier to goto sleep...
					// we will not block the mainloop...
					gotoStandby();
				}
			}
			else
				eZapStandby::getInstance()->renewSleep();
			break;
		}
		default:
			break;
	}
	return 0;
}

ePlaylist *eZapMain::addUserBouquet( eServiceReference &parent, ePlaylist *list, const eString &path, const eString &name, eServiceReference& ref, bool save )
{
	ePlaylist *pl = (ePlaylist*)eServiceInterface::getInstance()->addRef(ref);
	pl->service_name = name;
	pl->load( path.c_str() );
	pl->save();
	eServiceInterface::getInstance()->removeRef(ref);
	ref.path=path;
	eServicePlaylistHandler::getInstance()->newPlaylist(parent, ref); // add to playlists multimap
	pl = (ePlaylist*)eServiceInterface::getInstance()->addRef(ref);
	pl->load( path.c_str() );
	list->getList().push_back( ref );
	list->getList().back().type = ePlaylistEntry::PlaylistEntry|ePlaylistEntry::boundFile;
	list->save();
	if (save)
		pl->save();
	return pl;
}

void eZapMain::renameService( eServiceSelector *sel )
{
	eServiceReference ref=sel->getSelected();
	eServiceReference path=sel->getPath().current();
	ePlaylist *p=0;
	if ( path.type == eServicePlaylistHandler::ID )
		p=(ePlaylist*) eServicePlaylistHandler::getInstance()->addRef(path);
	if ( !p )
	{
		if ( ref.type == eServiceReference::idDVB && !ref.path )
		{
			eServiceDVB *service = eTransponderList::getInstance()->searchService(ref);
			if ( service )
			{
				eString old = service->service_name;
				if (!old)
					old="";
				TextEditWindow wnd(_("Enter new name:"));
				wnd.setText(_("Rename entry"));
				wnd.show();
				wnd.setEditText(old);
				int ret = wnd.exec();
				wnd.hide();
				if ( !ret )
				{
					eString New = wnd.getEditText();
					if ( old != New )
					{
						if ( New )
						{
							service->service_name=New;
							service->dxflags |= eServiceDVB::dxHoldName;
						}
						else
						{
							service->dxflags &= ~eServiceDVB::dxHoldName;
							service->service_name="unknown service";
  // get original name from SDT
							eServiceHandler *sapi=eServiceInterface::getInstance()->getService();
							if (sapi)
							{
								SDT *sdt=sapi->getSDT();
								if (sdt)
								{
									for (ePtrList<SDTEntry>::iterator i(sdt->entries); i != sdt->entries.end(); ++i)
									{
										if (eServiceID(i->service_id)==((eServiceReferenceDVB&)ref).getServiceID())
										{
											service->update(*i);
											break;
										}
									}
									sdt->unlock();
								}
							}
						}
						sel->invalidateCurrent(ref);
					}
				}
			}
		}
		return;
	}

	std::list<ePlaylistEntry>::iterator it=std::find(p->getList().begin(), p->getList().end(), ref);
	if (it == p->getList().end())
	{
		eServiceInterface::getInstance()->removeRef(path);
		return;
	}

	if ( (( ref.type == eServiceReference::idDVB )
#ifndef DISABLE_FILE
		|| 	( ref.type == eServiceReference::idUser &&
					( (ref.data[0] ==  eMP3Decoder::codecMPG) ||
						(ref.data[0] ==  eMP3Decoder::codecMP3) ||
						(ref.data[0] ==  eMP3Decoder::codecFLAC) ||
						(ref.data[0] ==  eMP3Decoder::codecOGG) ) )
#endif
					) )
	{
		TextEditWindow wnd(_("Enter new name:"));
		wnd.setText(_("Rename entry"));
		wnd.show();
		if ( it->service.descr.length() )
			wnd.setEditText(it->service.descr);
		else if ( it->type & ePlaylistEntry::boundFile )
		{
			int i = it->service.path.rfind('/');
			++i;
			wnd.setEditText(it->service.path.mid( i, it->service.path.length()-i ));
		}
		else
		{
			eService* s = eServiceInterface::getInstance()->addRef(it->service);
			if ( s )
			{
				wnd.setEditText(s->service_name);
				eServiceInterface::getInstance()->removeRef( it->service );
			}
		}
		int ret = wnd.exec();
		wnd.hide();
		if ( !ret )
		{
			if ( it->service.descr != wnd.getEditText() )
			{
				it->service.descr = wnd.getEditText();
				sel->invalidateCurrent(it->service);
				p->save();
			}
		}
	}
	eServiceInterface::getInstance()->removeRef(path);
}

void eZapMain::deleteService( eServiceSelector *sel )
{
	eServiceReference ref=sel->getSelected();
	eServiceReference path=sel->getPath().current();

	eDebug("path: '%s', ref: '%s', pathtype: %d, reftype: %d\n",path.toString().c_str(),ref.toString().c_str(),path.type,ref.type);

/*  i think is not so good to delete normal providers
		if (ref.data[0]==-3) // Provider
		{
			eDVB::getInstance()->settings->removeDVBBouquet(ref.data[2]);
			eDVB::getInstance()->settings->saveBouquets();
			sel->actualize();
			break;
		}*/
	ePlaylist *pl=0;
	if ( path.type == eServicePlaylistHandler::ID )
		pl =(ePlaylist*)eServicePlaylistHandler::getInstance()->addRef(path);
	if (!pl)
	{
		if ( ref.type == eServiceReference::idDVB && !ref.path )
		{
			sel->removeCurrent(true);
			sel->movemode |= 2;
			eTransponderList::getInstance()->removeService((eServiceReferenceDVB&)ref);
			sel->movemode &= ~2;
			return;
		}
		eMessageBox::ShowBox(_("Sorry, you cannot delete this service."), _("delete service"), eMessageBox::iconWarning|eMessageBox::btOK );
		return;
	}
	bool removeEntry=true;

	std::list<ePlaylistEntry>::iterator it=std::find(pl->getList().begin(), pl->getList().end(), ref);
	if (it == pl->getList().end())
		return;

	// remove parent playlist ref...
	eServiceInterface::getInstance()->removeRef(path);

	int profimode=0;
	eConfig::getInstance()->getKey("/elitedvb/extra/profimode", profimode);

	if ( it->service.type == eServiceReference::idDVB )
	{
		bool boundfile=false;
		if ( (it->type & (ePlaylistEntry::PlaylistEntry|ePlaylistEntry::boundFile))==(ePlaylistEntry::PlaylistEntry|ePlaylistEntry::boundFile) )
		{
			// recorded stream selected ( in recordings.epl )
			boundfile=true;
			if (!profimode)
			{
				eString str;
				str.sprintf(_("You are trying to delete '%s'.\nReally do this?"), it->service.descr.c_str() );
				int r = eMessageBox::ShowBox(str, _("Delete recorded stream"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btNo);
				if (r != eMessageBox::btYes)
					removeEntry=false;
			}
		}
		// delete running service ?
		if (removeEntry)
		{
			if (eServiceInterface::getInstance()->service == ref)
				eServiceInterface::getInstance()->stop();
			if ( boundfile )
			{
				eString fname=it->service.path;
				fname.erase(fname.length()-2,2);
				fname+="eit";
				::unlink(fname.c_str());
				::unlink((it->service.path+".indexmarks").c_str());
			}
		}
	} // bouquet (playlist) selected
	else if ( it->service.type == eServicePlaylistHandler::ID )
	{
		if (!profimode)
		{
			if ( (it->type & (ePlaylistEntry::PlaylistEntry|ePlaylistEntry::boundFile))==(ePlaylistEntry::PlaylistEntry|ePlaylistEntry::boundFile) )
			{
				int r = eMessageBox::ShowBox(
					_("This is a complete Bouquet!\nReally delete?"),
					_("Delete bouquet"),
					eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btNo);
				if (r != eMessageBox::btYes)
					removeEntry=false;
			}
		}
		if (removeEntry) // remove service.. and linked files..
		{
			eServicePlaylistHandler::getInstance()->removePlaylist( it->service );
			eServiceInterface::getInstance()->removeRef(ref);
		}
	}
	if (removeEntry) // remove service.. and linked files..
	{
		// move selection to prev entry... when exist...
		std::list<ePlaylistEntry>::iterator i = it;
		i++;
		if ( i == pl->getList().end() )
		{
			i=it;
			i--;
			sel->removeCurrent(false);
		}
		else
			sel->removeCurrent(true);

		// remove the entry in playlist
		pl->deleteService(it);

		sel->updateNumbers();

		// save playlist
		pl->save();
		// enter new into the current path
	}
}

#ifndef DISABLE_FILE
void eZapMain::deleteFile(eServiceReference ref)
{
	bool isTS = (ref.path.right(3).upper() == ".TS");
	int slice=0;
	eString filename=ref.path;
	if (eServiceInterface::getInstance()->service == ref)
		eServiceInterface::getInstance()->stop();
	if ( isTS )
	{
		// Reread recordings file (could be changed by other box)
		recordings->lockPlaylist();
		loadRecordings();
		for ( std::list<ePlaylistEntry>::iterator it(recordings->getList().begin());
		it != recordings->getList().end(); ++it )
		{
			if ( it->service.path == ref.path )
			{
				recordings->getList().erase(it);
				recordings->save();
				break;
			}
		}
		filename.erase(filename.length()-2,2);
		filename+="eit";
		::unlink(filename.c_str());
		::unlink((ref.path+".indexmarks").c_str());
		recordings->unlockPlaylist();
		while (1)
		{
			filename=ref.path;
			if (slice)
				filename+=eString().sprintf(".%03d", slice);
			slice++;
			struct stat64 s;
			if (::stat64(filename.c_str(), &s) < 0)
				break;
			eBackgroundFileEraser::getInstance()->erase(filename.c_str());
		}
	}
	else
	{
		::unlink((ref.path+".indexmarks").c_str());
		eBackgroundFileEraser::getInstance()->erase(ref.path.c_str());
	}
}

void eZapMain::deleteFile( eServiceSelector *sel )
{
	eServiceReference ref=sel->getSelected();

	int profimode=0;
	eConfig::getInstance()->getKey("/elitedvb/extra/profimode", profimode);

	bool removeEntry=true;

	if (!profimode)
	{
		eString s;
		s.sprintf(_("You are trying to delete '%s'.\nReally do this?"),ref.path.c_str() );
		int r = eMessageBox::ShowBox(s, _("Delete File"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btNo);
		if (r != eMessageBox::btYes)
			removeEntry=false;
	}

	if (removeEntry)
	{
		deleteFile(ref);
		sel->removeCurrent(false);
	}
}

int eZapMain::renameFile(const eString& oldFilePath, const eString& newFilePath, const eString& description)
{
	int rc = 0;

	eDebug("[ENIGMA_MAIN] renameFile %s to %s", oldFilePath.c_str(), newFilePath.c_str());

	recordings->lockPlaylist();
	// re-read recordings file, could be changed by other box
	loadRecordings();
	// First find out if the new filename allready exist, if it allready exist we cannot rename the file
	bool exist = false; 
	std::list<ePlaylistEntry>::iterator it;
	for (it = recordings->getList().begin(); it != recordings->getList().end(); ++it)
	{
		if ( it->service.path == newFilePath )
		{
			exist = true;
			break;
		}
	}
	// If the new filename does existy rename file within playlist.
	// if we do it after rename of real file it is not in recordings.epl anymore ;-)
	if (!exist)
	{
		if ( ::rename( oldFilePath.c_str(), newFilePath.c_str() ) < 0 )
			eDebug("rename File '%s' to '%s' failed (%m)",
				oldFilePath.c_str(), newFilePath.c_str() );
		else
		{
			if (oldFilePath.right(3).upper() == ".TS")
			{
				int ret = 0;
				int cnt = 1;
				do
				{
					eString tmpold, tmpnew;
					tmpold.sprintf("%s.%03d", oldFilePath.c_str(), cnt);
					tmpnew.sprintf("%s.%03d", newFilePath.c_str(), cnt++);
					ret = ::rename(tmpold.c_str(), tmpnew.c_str());
				}
				while( !ret );
	
				::rename((oldFilePath + ".indexmarks").c_str(), (newFilePath + ".indexmarks").c_str());
				eString fname = oldFilePath;
				fname.erase(fname.length()-2,2);
				fname += "eit";
				eString eitFile = newFilePath; 
				eitFile.erase(eitFile.length() - 2, 2);
				::rename(fname.c_str(), (eitFile + "eit").c_str());
			}
			rc = 1;

			for (it = recordings->getList().begin(); it != recordings->getList().end(); ++it)
			{
				if ( it->service.path == oldFilePath )
				{
					it->service.path=newFilePath;
					if (description)
						it->service.descr = description;
					recordings->save();
					break;
				}
			}
	
			saveRecordings();
		}	
	}
	else
	{
		eDebug("Cannot rename %s to %s This file does allready exist in recordings.epl",oldFilePath.c_str(), newFilePath.c_str()); 
	}
	// Release our lock on the playlist
	recordings->unlockPlaylist();
	return rc;
}

void eZapMain::renameFile( eServiceSelector *sel )
{
	eServiceReference ref=sel->getSelected();

	eString fname=ref.path.mid( ref.path.rfind('/')+1 );

	unsigned int b = fname.find('.');
	if ( b != eString::npos )
		fname.erase(b);

	TextEditWindow wnd(_("Enter new Filename:"),"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 -_äöüÄÖÜ");
	wnd.setText(_("Rename File"));
	wnd.show();
	wnd.setEditText(fname);
	int ret = wnd.exec();
	wnd.hide();
	if ( !ret )
	{
		// name changed?
		if ( fname != wnd.getEditText() )
		{
			eString oldFilePath = ref.path;
			eString newFilePath = ref.path.left( ref.path.rfind('/')+1 );
			newFilePath += wnd.getEditText();
			unsigned int c = ref.path.rfind('.');
			if ( c != eString::npos )
				newFilePath += ref.path.mid( c );

			if (renameFile(oldFilePath, newFilePath, ""))
			{
				ref.path=newFilePath;
				sel->invalidateCurrent(ref);
			}
		}
	}
}
#endif

void eZapMain::renameBouquet( eServiceSelector *sel)
{
	eServiceReference ref=sel->getSelected();
	eServiceReference path=sel->getPath().current();
	ePlaylist *p=(ePlaylist*)eServiceInterface::getInstance()->addRef(ref);
	TextEditWindow wnd(_("Enter new name for the bouquet:"));
	wnd.setText(_("Rename bouquet"));
	wnd.show();
	wnd.setEditText(p->service_name);
	int ret = wnd.exec();
	wnd.hide();
	if ( !ret )
	{
		// name changed?
		if ( p->service_name != wnd.getEditText() )
		{
			p->service_name=wnd.getEditText();
/*			if (ref.data[0]==-3) // dont rename provider
			{
				eDVB::getInstance()->settings->renameDVBBouquet(ref.data[2],p->service_name);
				eDVB::getInstance()->settings->saveBouquets();
			}*/
			p->save();
			sel->invalidateCurrent();
		}
	}
	eServiceInterface::getInstance()->removeRef(ref);
}

void eZapMain::createEmptyBouquet(int mode)
{
	TextEditWindow wnd(_("Enter name for the new bouquet:"));
	wnd.setText(_("Add new bouquet"));
	wnd.show();
	wnd.setEditText(eString(""));
	int ret = wnd.exec();
	wnd.hide();
	if ( !ret )
	{
		eServiceReference newList=
			eServicePlaylistHandler::getInstance()->newPlaylist();

		eString basePath=eplPath;
		if ( eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000 )
		{
			if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feCable )
				basePath+="/cable";
			if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feTerrestrial )
				basePath+="/terrestrial";
		}

		switch ( mode )
		{
			case modeTV:
				addUserBouquet( userTVBouquetsRef, userTVBouquets, basePath+'/'+eString().sprintf("userbouquet.%x.tv",newList.data[1]), wnd.getEditText(), newList, true );
				break;
			case modeRadio:
				addUserBouquet( userRadioBouquetsRef, userRadioBouquets, basePath+'/'+eString().sprintf("userbouquet.%x.radio",newList.data[1]), wnd.getEditText(), newList, true );
				break;
#ifndef DISABLE_FILE
			case modeFile:
				addUserBouquet( userFileBouquetsRef, userFileBouquets, eplPath+'/'+eString().sprintf("userbouquet.%x.file",newList.data[1]), wnd.getEditText(), newList, true );
				break;
#endif
		}
	}
}

void eZapMain::createMarker(eServiceSelector *sel)
{
	eServiceReference ref=sel->getSelected();
	eServiceReference path=sel->getPath().current();
	ePlaylist *pl=0;
	if ( path.type == eServicePlaylistHandler::ID )
		pl = (ePlaylist*)eServicePlaylistHandler::getInstance()->addRef( path );
	if ( pl )
	{
		std::set<int> markerNums;
		std::list<ePlaylistEntry>::iterator it=pl->getList().end();
		for (std::list<ePlaylistEntry>::iterator i(pl->getList().begin()); i != pl->getList().end(); ++i )
		{
			if ( i->service.flags & eServiceReference::isMarker )
				markerNums.insert( i->service.data[0] );
			if ( i->service == ref )
				it = i;
		}
		eServiceReference mark( eServiceReference::idDVB, eServiceReference::isMarker, markerNums.size() ? (*(--markerNums.end()))+1 : 1 );
		mark.descr=_("unnamed");
		TextEditWindow wnd(_("Enter marker name:"));
		wnd.setText(_("create marker"));
		sel->hide();
		wnd.show();
		int ret = wnd.exec();
		wnd.hide();
		if ( !ret )
		{
			if ( mark.descr != wnd.getEditText() )
				mark.descr = wnd.getEditText();
			pl->lockPlaylist();
			if ( it != pl->getList().end() )
				pl->getList().insert( it, mark );
			else
				pl->getList().push_back( mark );
			// If this is recordings.epl directly save it
			if (pl == recordings)
			{
				saveRecordings();
			}
			pl->unlockPlaylist();
			sel->actualize();
			sel->selectService(mark);
		}
		sel->show();
		eServicePlaylistHandler::getInstance()->removeRef( path );
	}
}

void eZapMain::copyProviderToBouquets(eServiceSelector *sel)
{
	eServiceReference ref=sel->getSelected();
	eServiceReference path=sel->getPath().current();

	eServiceReference oldref = currentSelectedUserBouquetRef;
	ePlaylist *oldBouquet = currentSelectedUserBouquet;

	// create new user bouquet
	currentSelectedUserBouquetRef = eServicePlaylistHandler::getInstance()->newPlaylist();

	// get name of the source bouquet
	eString name;

	const eService *pservice=eServiceInterface::getInstance()->addRef(ref);
	if (pservice)
	{
		if ( pservice->service_name.length() )
			name = pservice->service_name;
		else
			name = _("unnamed bouquet");
		eServiceInterface::getInstance()->removeRef(ref);
	}

	eString basePath=eplPath;
	if ( eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000 )
	{
		if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feCable )
			basePath+="/cable";
		if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feTerrestrial )
			basePath+="/terrestrial";
	}

	switch ( mode )
	{
		case modeTV:
			currentSelectedUserBouquet = addUserBouquet( userTVBouquetsRef, userTVBouquets, basePath+'/'+eString().sprintf("userbouquet.%x.tv",currentSelectedUserBouquetRef.data[1]), name, currentSelectedUserBouquetRef, false );
			break;
		case modeRadio:
			currentSelectedUserBouquet = addUserBouquet( userRadioBouquetsRef, userRadioBouquets, basePath+'/'+eString().sprintf("userbouquet.%x.radio",currentSelectedUserBouquetRef.data[1]), name, currentSelectedUserBouquetRef, false );
			break;
	}

	Signal1<void,const eServiceReference&> signal;
	CONNECT(signal, eZapMain::addServiceToCurUserBouquet);

	eServicePath safe = sel->getPath();
	sel->enterDirectory(ref);
	sel->forEachServiceRef(signal, true);
	sel->setPath( safe, ref );

	currentSelectedUserBouquet->save();

	currentSelectedUserBouquetRef=oldref;
	currentSelectedUserBouquet=oldBouquet;
}
struct eServiceHandlerDVB_removeNewFoundFlag
{
	int type;
	int DVBNamespace;
	eServiceHandlerDVB_removeNewFoundFlag(int type, int DVBNamespace)
	: type(type), DVBNamespace(DVBNamespace)
	{
	}
	void operator()(const eServiceReference &service)
	{
		eService *s = eTransponderList::getInstance()->searchService( service );
		if ( !s )  // dont show "removed services"
			return;
		else if ( s->dvb && s->dvb->dxflags & eServiceDVB::dxDontshow )
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
			s->dvb->dxflags &= ~eServiceDVB::dxNewFound;
			
	}
};

extern bool rec_movies();		// implemented in enigma_dyn.cpp (extern is dirty but done in several places and since it is too hot now it is legimate)

void eZapMain::showServiceMenu(eServiceSelector *sel)
{
	if(!pinCheck::getInstance()->checkPin(pinCheck::setup))
	{
		// Service menu only available with right pin code
		return;
	}
	
	eServiceReference ref=sel->getSelected();
	eServiceReference path=sel->getPath().current();
#ifndef DISABLE_LCD
	eServiceContextMenu m(ref, path, LCDTitle, LCDElement);
#else
	eServiceContextMenu m(ref, path);
#endif
	if ( !m.getCount() )
		return;
	m.show();
	int res=m.exec();
	m.hide();
	switch (res)
	{
	case 0: // cancel
		break;
	case 1: // delete service
		deleteService(sel);
		break;
	case 2: // enable/disable movemode
		toggleMoveMode(sel);
		break;
	case 3: // add service to playlist
		if (ref.flags & eServiceReference::mustDescent)
			playService(ref, playlistmode?psAdd:0);  // M3U File
		else
			doPlaylistAdd(ref);  // single service
		break;
	case 4: // add service to bouquet
		addServiceToUserBouquet(&ref);
		break;
	case 5: // toggle edit User Bouquet Mode
		toggleEditMode(sel);
		break;
	case 6: // add new bouquet
	{
		eServicePath path=sel->getPath();
		path.up();
		createEmptyBouquet(mode);
		if ( mode == modeTV && path.current() == userTVBouquetsRef )
			sel->actualize();
		else if ( mode == modeRadio && path.current() == userRadioBouquetsRef )
			sel->actualize();
		else if ( mode == modeFile && path.current() == userFileBouquetsRef )
			sel->actualize();
		break;
	}
	case 7: // rename user bouquet
		renameBouquet(sel);
		break;
	case 8: // duplicate normal bouquet as user bouquet
		copyProviderToBouquets(sel);
		break;
	case 9: // rename service
		renameService(sel);
		break;
	case 10: // lock service ( parental locking )
	{
		if ( handleState() )
		{
			int pLockActive = pinCheck::getInstance()->pLockActive();
			ref.lock();
			if ( pLockActive && ref == eServiceInterface::getInstance()->service )
				eServiceInterface::getInstance()->stop();
			sel->invalidateCurrent();
			break;
		}
	}
	case 11:  // unlock service ( parental locking )
		if ( pinCheck::getInstance()->pLockActive() )
		{
			if ( !pinCheck::getInstance()->checkPin(pinCheck::parental))
				break;
		}
		ref.unlock();
		sel->invalidateCurrent();
		break;
	case 12:  // show / hide locked service ( parental locking )
	{
		if ( handleState() )
		{
			if ( pinCheck::getInstance()->pLockActive() )
			{
				if ( !pinCheck::getInstance()->checkPin(pinCheck::parental))
					break;
			}
			pinCheck::getInstance()->switchLock();
			if ( pinCheck::getInstance()->pLockActive() )
			{
				// Lock is active again so we reset the pin timeout
				pinCheck::getInstance()->resetParentalPinOk();
			}

			sel->actualize();
			if ( eServiceInterface::getInstance()->service.isLocked() && pinCheck::getInstance()->isLocked() )
				eServiceInterface::getInstance()->stop();
		}
		break;
	}
	case 13:
		createMarker(sel);
		break;
#ifndef DISABLE_FILE
	case 14:
		deleteFile(sel);
		break;
	case 15:
		renameFile(sel);
		break;
#endif
	case 16:
	{
		eServiceDVB *s = eTransponderList::getInstance()->searchService(ref);
		if (s)
			s->dxflags &= ~eServiceDVB::dxNewFound;
		break;
	}
#ifndef DISABLE_FILE
	case 17:
	{
		int profimode=0;
		eConfig::getInstance()->getKey("/elitedvb/extra/profimode", profimode);
		
		if(!profimode)
		{
			eMessageBox box(
				_("Do you want to rebuild your movie list?"), 
				_("Recover movies"), 
				eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btNo);
			box.show();
			int r=box.exec();
			box.hide();
			
			if(r == eMessageBox::btYes)
			{
				rec_movies();
				sel->actualize();
			}
		}
		break;
	}
#endif
	case 18:
	{
		eServicePath p;
		switch(mode)
		{
			case modeTV:
				if ( userTVBouquets->getList().size() )
					p.down(userTVBouquetsRef);
				break;
			case modeRadio:
				if ( userRadioBouquets->getList().size() )
					p.down(userRadioBouquetsRef);
				break;
			case modeFile:
				if ( userFileBouquets->getList().size() )
					p.down(userFileBouquetsRef);
				break;
		}
		p.down(path);
		p.down(ref);
		int startup = 1;
		eConfig::getInstance()->setKey( eString().sprintf("/ezap/ui/startup/%d",mode).c_str(), startup );
		eString str = p.toString();
eDebug(str.c_str());
		eConfig::getInstance()->setKey( eString().sprintf("/ezap/ui/modes/%d/path0", mode).c_str(), str.c_str() );
		break;
	}
	case 19:
	{
		int startup = 0;
		eConfig::getInstance()->setKey( eString().sprintf("/ezap/ui/startup/%d",mode).c_str(), startup );
		break;
	}
	case 20:
	{
		eTransponderList::getInstance()->forEachServiceReference(eServiceHandlerDVB_removeNewFoundFlag(ref.data[1], ref.data[2]));
		sel->invalidateCurrent();
		break;
	}
	case 21:
	{
		sel->shuffle();
		break;
	}
	case 22:
		eTransponderList::getInstance()->removeNewFlags(-1);
		break;
	default:
		break;
	}
}

void eZapMain::toggleMoveMode( eServiceSelector* sel )
{
	if ( sel->toggleMoveMode() )
	{
		currentSelectedUserBouquetRef=sel->getPath().current();
		currentSelectedUserBouquet=(ePlaylist*)eServiceInterface::getInstance()->addRef( currentSelectedUserBouquetRef );
	}
	else
	{
		currentSelectedUserBouquet->save();
		currentSelectedUserBouquet=0;
		eServiceInterface::getInstance()->removeRef( currentSelectedUserBouquetRef );
		currentSelectedUserBouquetRef=eServiceReference();
	}
}

int eZapMain::toggleEditMode( eServiceSelector *sel, int defmode )
{
	if ( sel->toggleEditMode() ) // toggleMode.. and get new state !!!
	{
		std::list<ePlaylistEntry> &lst =
			defmode == modeTV ?
					userTVBouquets->getList() :
			defmode == modeRadio ?
					userRadioBouquets->getList() :
			defmode == modeFile ?
					userFileBouquets->getList() :
			mode == modeRadio ?
				userRadioBouquets->getList() :
#ifndef DISABLE_FILE
			mode == modeFile ?
				userFileBouquets->getList() :
#endif
			userTVBouquets->getList();
		UserBouquetSelector s( lst );
		s.show();
		s.exec();
		s.hide();
		if (s.curSel)
		{
			currentSelectedUserBouquetRef = s.curSel;
			currentSelectedUserBouquet = (ePlaylist*)eServiceInterface::getInstance()->addRef( s.curSel );
			for (std::list<ePlaylistEntry>::const_iterator i(currentSelectedUserBouquet->getConstList().begin()); i != currentSelectedUserBouquet->getConstList().end(); ++i)
				eListBoxEntryService::hilitedEntrys.insert(*i);
			return 0;
		}
		else  // no user bouquet is selected...
		{
			sel->toggleEditMode();  // disable edit mode
			return -1;
		}
	}
	else
	{
		eListBoxEntryService::hilitedEntrys.clear();
		currentSelectedUserBouquet->save();
		currentSelectedUserBouquet=0;
		eServiceInterface::getInstance()->removeRef( currentSelectedUserBouquetRef );
		currentSelectedUserBouquetRef=eServiceReference();
	}
	return 0;
}

#ifndef DISABLE_FILE
void eZapMain::startPermanentTimeshift()
{

	if ( state & stateRecording ) // no timeshift when recording
		return;
	if ( freeRecordSpace() < 10) // less than 10MB free
		return;
	eServiceReference streamingRef;
	if ( eStreamer::getInstance()->getServiceReference(streamingRef)) // no timeshift when streaming
	{
		eDebug("[PERM] no permanent timeshifting when streamts is running");
		return;
	}

	eDebug("starting permanent timeshift ...");

	state |= recPermanentTimeshift;
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler || handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordOpenPermanentTimeshift)))
	{
		state &= ~(recPermanentTimeshift);
		eDebug("couldn't start permanent timeshift ... :/");
	} else
	{
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordStart));
		//timeshift = 2;
	}
}
void eZapMain::stopPermanentTimeshift()
{
	eDebug("stopping permanent timeshift ...");
	permanentTimeshiftTimer.stop();
	eServiceHandler *handler=eServiceInterface::getInstance()->getServiceHandler(eServiceReference::idDVB);
	//timeshift = 0;
	if (state & recPermanentTimeshift)
	{
		if (!handler)
			return;
		eDebug("stopping permanent timeshift with handler...");
		state &= ~(recPermanentTimeshift);
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordStop));
		handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordClose));
	}
}

void eZapMain::beginPermanentTimeshift()
{
	if ( !(state & stateRecording) && !(state & recPermanentTimeshift) && (mode == modeTV || mode == modeRadio))
	{
		int permanentOn = 0;
		eConfig::getInstance()->getKey("/enigma/timeshift/permanent", permanentOn );
		if (permanentOn)
		{
			// start timer for permanent timeshift
			int permanentdelay = 0;
			eConfig::getInstance()->getKey("/enigma/timeshift/permanentdelay", permanentdelay );
			eDebug("starting timeout for permanent timeshift:%d",permanentdelay);
			permanentTimeshiftTimer.start(permanentdelay*1000, 1);
		}
	}
}
void eZapMain::addTimeshiftToRecording()
{
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (handler)
	{
		if(!eServiceInterface::getInstance()->service.path)
		{
			if ( !eSystemInfo::getInstance()->canTimeshift() )
				return;
			if (state & stateRecording && eDVB::getInstance()->recorder
				&& eDVB::getInstance()->recorder->recRef == eServiceInterface::getInstance()->service )
			{
				handler->serviceCommand(eServiceCommand(eServiceCommand::cmdAddPermanentTimeshiftToRecording));
			}
		}
	}
}
#endif

void eZapMain::addServiceToCurUserBouquet(const eServiceReference& service)
{
	currentSelectedUserBouquet->getList().push_back(service);
}

void eZapMain::playService(const eServiceReference &service, int flags)
{
	int first=0;

	if ( !service || ( service.path && service.path == "/" ) )
		return;

	if ( flags & psNoUser )
		goto zap;

#ifndef DISABLE_FILE
	stopPermanentTimeshift();
	if ( !canPlayService(service) )
	{
		if ( handleState() )
			goto zap;
		return;
	}
#endif
zap:
	if (flags&psSetMode)
	{
		if ( service.type == eServiceReference::idDVB && !service.path )
		{
			if ( service.data[0] == 2 )
				setMode( modeRadio );
			else
				setMode( modeTV );
		}
		else
			setMode( modeFile );
	}

	if (flags&psDontAdd)
	{
//		eDebug("psDontAdd");

		if ( (playlist->current != playlist->getConstList().end())
			&& (playlist->current->service != service) )
		{
//			eDebug("new service is played... getPlaylistPosition");
			getPlaylistPosition();
		}

//		eDebug("play");
		eServiceInterface::getInstance()->play(service);
#ifndef DISABLE_FILE
		beginPermanentTimeshift();
#endif
		if ( flags&psSeekPos )
		{
			std::list<ePlaylistEntry>::iterator i =
				std::find( playlist->getList().begin(),
					playlist->getList().end(), service);

			if (i != playlist->getList().end())
			{
				eServiceHandler *handler=eServiceInterface::getInstance()->getService();
				if (!handler)
					return;
				playlist->current=i;
				if (playlist->current->current_position != -1)
				{
//					eDebug("we have stored PlaylistPosition.. get this... and set..");
					handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekReal, playlist->current->current_position));
				}
			}
			else
				addService(service);
		}
		return;
	}
		// we assume that no "isDirectory" is set - we won't open the service selector again.
	if (!(flags&psAdd))
	{
		if (!playlistmode)		// dem user liebgewonnene playlists nicht einfach killen
		{
//			eDebug("not playlistmode.. shrink playlist");
			while (playlist->getConstList().size() > 25)
				playlist->getList().pop_front();
		}
/*		else
			eDebug("playlistmode.. do not shrink playlist");*/

		// a M3U File is selected...
		if ((!playlistmode) && (service.flags & eServiceReference::mustDescent))
		{
			playlist->getList().clear();
			first=1;
			playlistmode=1;
		}
		else
		{
			playlist->current=playlist->getList().end();
			if (playlist->current == playlist->getList().begin())
				first=1;
		}
	}

	addService(service);

	if (!(flags&psAdd))
	{
		if (first)
			playlist->current = playlist->getList().begin();
		if (playlist->current != playlist->getConstList().end())
		{
			eServiceReference ref=(eServiceReference&)(*playlist->current);
			eServiceInterface::getInstance()->play(ref);
#ifndef DISABLE_FILE
			beginPermanentTimeshift();
#endif
		}
		else
		{
			Description->setText(_("This directory doesn't contain anything playable!"));
			postMessage(eZapMessage(0, _("Play"), _("This directory doesn't contain anything playable!")), 1);
		}
	}

	if (eZap::getInstance()->getServiceSelector()->getPath().current() == playlistref)
		eZap::getInstance()->getServiceSelector()->actualize();
}

void eZapMain::addService(const eServiceReference &service)
{
	if (service.flags & eServiceReference::mustDescent) // recursive add services..
	{
//		eDebug("recursive add");
		entered_playlistmode=1;
		Signal1<void,const eServiceReference&> signal;
		CONNECT( signal, eZapMain::addService);
		eServiceInterface::getInstance()->enterDirectory(service, signal);
		eServiceInterface::getInstance()->leaveDirectory(service);
	}
	else
	{
//		eDebug("addService");
		int last=0;
		if (playlist->current != playlist->getConstList().end()
			&& *playlist->current == service)
		{
			++playlist->current;
			last=1;
		}
		playlist->getList().remove(service);
//		playlist->getList().push_back(ePlaylistEntry(service));
		eServicePath p = eZap::getInstance()->getServiceSelector()->getPath();
		p.down(service);
		playlist->getList().push_back(p);
		if ((playlist->current == playlist->getConstList().end()) || last)
			--playlist->current;
	}
}

void eZapMain::doPlaylistAdd(const eServiceReference &service)
{
//	eDebug("playlistmode=%d", playlistmode);
	entered_playlistmode=1;
	if (!playlistmode)
	{
		playlistmode=1;
		playlist->getList().clear();
		playlist->current=playlist->getList().begin();
		playService(service, 0);
	}
	else
	{
//		eDebug("call playService(service,psAdd)");
		playService(service, psAdd);
	}
}

void eZapMain::addServiceToUserBouquet(eServiceReference *service, int dontask)
{
	if (!service)
		return;

	eServiceReference oldRef = currentSelectedUserBouquetRef;
	ePlaylist *old = currentSelectedUserBouquet;

	if (!dontask)
	{
		if ( (mode > modeFile) || (mode < 0) )
			return;
		UserBouquetSelector s( mode == modeTV?userTVBouquets->getList():mode==modeRadio?userRadioBouquets->getList():userFileBouquets->getList() );
		s.show();
		s.exec();
		s.hide();

		if ( s.curSel )
		{
			currentSelectedUserBouquetRef = s.curSel;
			currentSelectedUserBouquet = (ePlaylist*)eServiceInterface::getInstance()->addRef( currentSelectedUserBouquetRef );
		}
		else
		{
			currentSelectedUserBouquet=0;
			currentSelectedUserBouquetRef=eServiceReference();
		}
	}

	if (currentSelectedUserBouquet)
	{
		std::list<ePlaylistEntry> &list =
			currentSelectedUserBouquet->getList();

		if ( std::find( list.begin(), list.end(), ePlaylistEntry(*service) ) == list.end() )
			list.push_back(*service);

		if (!dontask)
		{
			currentSelectedUserBouquet->save();
			currentSelectedUserBouquet = old;
			eServiceInterface::getInstance()->removeRef( currentSelectedUserBouquetRef );
			currentSelectedUserBouquetRef = oldRef;
		}
	}
}

void eZapMain::removeServiceFromUserBouquet(eServiceSelector *sel )
{
	const eServiceReference &service=sel->getSelected();
	if ((mode > modeRadio) || (mode < 0))
		return;

	if (currentSelectedUserBouquet)
	{
		currentSelectedUserBouquet->getList().remove(service);
		if ( currentSelectedUserBouquetRef == sel->getPath().top() )
			sel->removeCurrent(true);
	}
}

void eZapMain::showSubserviceMenu()
{
	if (!(flags & (ENIGMA_NVOD|ENIGMA_SUBSERVICES|ENIGMA_VIDEO)))
		return;

	if (isVisible())
	{
		timeout.stop();
		hide();
	}

#ifndef DISABLE_LCD
	lcdmain.lcdMain->hide();
	lcdmain.lcdMenu->show();
#endif

	if (flags&ENIGMA_NVOD)
	{
#ifndef DISABLE_LCD
		nvodsel.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif
		nvodsel.show();
		nvodsel.exec();
		nvodsel.hide();
	}
	else if (flags&ENIGMA_SUBSERVICES)
	{
#ifndef DISABLE_LCD
		subservicesel.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif
		subservicesel.show();
		int ret = subservicesel.exec();
		subservicesel.hide();
		if ( !ret )
			subservicesel.play();
	}
	else if (flags&ENIGMA_VIDEO)
	{
#ifndef DISABLE_LCD
		videosel.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif
		videosel.show();
		videosel.exec();
		videosel.hide();
	}
#ifndef DISABLE_LCD
	lcdmain.lcdMenu->hide();
	lcdmain.lcdMain->show();
#endif
	if (!doHideInfobar())
		showInfobar();
}

void eZapMain::showAudioMenu()
{
	if (flags&(ENIGMA_AUDIO|ENIGMA_AUDIO_PS))
	{
#ifndef DISABLE_LCD
		lcdmain.lcdMain->hide();
		lcdmain.lcdMenu->show();
#endif
		if (isVisible())
		{
			timeout.stop();
			hide();
		}
#ifndef DISABLE_LCD
		if (flags&ENIGMA_AUDIO_PS)
			audioselps.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
		else
			audiosel.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif
		if (flags&ENIGMA_AUDIO_PS)
		{
			audioselps.show();
			audioselps.exec();
			audioselps.hide();
		}
		else
		{
			audiosel.show();
			audiosel.exec();
			audiosel.hide();
		}
#ifndef DISABLE_LCD
		lcdmain.lcdMenu->hide();
		lcdmain.lcdMain->show();
#endif
		if (!doHideInfobar())
			showInfobar();
	}
}

void eZapMain::runVTXT()
{
	eDebug("runVTXT/RassInteractive");
	if (rdstext_decoder.interactive_avail == 1)
	{
		hideInfobar();
		rdstext_decoder.rass_interactive();
	} 
	else if (isVT)
	{
		// If there's an external tuxtxt plugin, use that one, otherwise the internal
		eZapPlugins plugins(eZapPlugins::StandardPlugin);

		if(plugins.execPluginByName("tuxtxt.cfg", true) == "OK")
		{
			plugins.execPluginByName("tuxtxt.cfg");
		}
		else
		{
			hide();
			eTuxtxtWidget w;
			w.show();
			w.exec();
			w.hide();
		}
	}
}

void eZapMain::showPluginScreen()
{
	hide();
	eZapPlugins plugins(eZapPlugins::StandardPlugin, lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#ifndef DISABLE_LCD
	bool bMain = lcdmain.lcdMain->isVisible();
	bool bMenu = lcdmain.lcdMenu->isVisible();
	lcdmain.lcdMain->hide();
	lcdmain.lcdMenu->show();
#endif
	plugins.execSelectPrevious(previousPlugin);

	if(access("/tmp/reloadUserBouquets", F_OK) == 0)
	{
		reloadSettings();
		unlink("/tmp/reloadUserBouquets");
	}
#ifndef DISABLE_LCD
	if (!bMenu)
		lcdmain.lcdMenu->hide();
	if ( bMain )
		lcdmain.lcdMain->show();
#endif
	if (!doHideInfobar())
		showInfobar();
}

void eZapMain::showSelectorStyleEPG()
{
	if ( doubleklickTimer.isActive() )
	{
		doubleklickTimer.stop();
		doubleklickTimerConnection.disconnect();
		hide();
		eEPGStyleSelector e(0);
#ifndef DISABLE_LCD
		bool bMain = lcdmain.lcdMain->isVisible();
		bool bMenu = lcdmain.lcdMenu->isVisible();
		lcdmain.lcdMain->hide();
		lcdmain.lcdMenu->show();
		e.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif
		e.show();
		int ret = e.exec();
		e.hide();
		if (!doHideInfobar())
			showInfobar();
		switch ( ret )
		{
			case 1:
				showEPGList((eServiceReferenceDVB&)eServiceInterface::getInstance()->service);
				break;
			case 2:
				showMultiEPG();
				break;
			case 3:
				runPluginEPG();
				break;
			case 4:		// EPG Search
				EPGSearchEvent((eServiceReferenceDVB&)eServiceInterface::getInstance()->service);// EPG Search
				break;	// EPG Search
			default:
				break;
		}
#ifndef DISABLE_LCD
		if (!bMenu)
			lcdmain.lcdMenu->hide();
		if ( bMain )
			lcdmain.lcdMain->show();
#endif
	}
	else
	{
		doubleklickTimer.start(400,true);
		doubleklickTimerConnection = CONNECT( doubleklickTimer.timeout, eZapMain::showCurrentStyleEPG );
	}
}


void eZapMain::showCurrentStyleEPG()
{
	if ( doubleklickTimerConnection.connected() )
		doubleklickTimerConnection.disconnect();

	int lastEPGStyle=1;
	eConfig::getInstance()->getKey("/ezap/lastEPGStyle", lastEPGStyle);

	switch ( lastEPGStyle )
	{
		case 1:
			showEPGList((eServiceReferenceDVB&)eServiceInterface::getInstance()->service);
			break;
		case 2:
			showMultiEPG();
			break;
		case 3:
			runPluginEPG();
			break;
		default:
			showEPGList((eServiceReferenceDVB&)eServiceInterface::getInstance()->service);
			break;
	}
}

void eZapMain::showMultiEPG()
{
	eZapEPG epg;
#ifndef DISABLE_LCD
	bool bMain = lcdmain.lcdMain->isVisible();
	bool bMenu = lcdmain.lcdMenu->isVisible();
	lcdmain.lcdMain->hide();
	lcdmain.lcdMenu->show();
	epg.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif
	hide();
	epg.show();
	int result = epg.exec();
	epg.hide();
#ifndef DISABLE_LCD
	if (!bMenu)
		lcdmain.lcdMenu->hide();
	if ( bMain )
		lcdmain.lcdMain->show();
#endif
	if ( !result && (epg.getCurSelected()->service != eServiceInterface::getInstance()->service)) // switch to service requested... and update service selector with our new choice
	{
		playService(epg.getCurSelected()->service, psSeekPos);
		eServiceSelector *e = eZap::getInstance()->getServiceSelector();
		e->selectService(epg.getCurSelected()->service);
	}
	if (!doHideInfobar())
		showInfobar();
}

void eZapMain::runPluginEPG()
{
	eZapPlugins plugins(eZapPlugins::StandardPlugin);
	hide();
	plugins.execPluginByName("extepg.cfg");
	if (!doHideInfobar())
		showInfobar();
}

void eZapMain::showEPGList(eServiceReferenceDVB service)
{
	if (service.type != eServiceReference::idDVB)
		return;
	bool empty = true;

	{
		/* we only want to check whether we have any data, so we limit our map to 1 event */
		timeMapPtr e = eEPGCache::getInstance()->getTimeMapPtr((eServiceReferenceDVB&)service, 0, 0, 1);
		empty = e ? e->empty() : true;
	}

	if (!empty)
	{
		eEPGSelector wnd(service);
#ifndef DISABLE_LCD
		bool bMain = lcdmain.lcdMain->isVisible();
		bool bMenu = lcdmain.lcdMenu->isVisible();
		lcdmain.lcdMain->hide();
		lcdmain.lcdMenu->show();
		wnd.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif
		int wasVisible=isVisible();
		if (wasVisible)
		{
			timeout.stop();
			hide();
		}
		wnd.show();
		int back = wnd.exec();  // EPG search - added "int back ="
		wnd.hide();
		if (back == 2)  	// EPG search begin
		{
			eString searchname = wnd.getEPGSearchName();
			eEPGSelector eEPGSelectorSearch(searchname);
#ifndef DISABLE_LCD
			eEPGSelectorSearch.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif
			eEPGSelectorSearch.show(); eEPGSelectorSearch.exec(); eEPGSelectorSearch.hide();
		}			// EPG search end
#ifndef DISABLE_LCD
		if (!bMenu)
			lcdmain.lcdMenu->hide();
		if ( bMain )
			lcdmain.lcdMain->show();
#endif
		if (wasVisible && !doHideInfobar())
			showInfobar();
	}
}

void eZapMain::EPGSearchEvent(eServiceReferenceDVB service)	// EPG search
{
	
	if (service.type != eServiceReference::idDVB)
		return;
	
	int wasVisible=isVisible();
	if (wasVisible)
	{
		timeout.stop();
		hide();
	}
	
	//if (EINow->getText() != eString(_("no EPG available")))
	//if (!strcmp(EINow->getText(),_("no EPG available"))
	//{	

		eString EPGSearchName = "";
		eEPGSearch *dd = new eEPGSearch(service, cur_event_text );
		dd->show();
		int back = 2;
		do
		{
			back = dd->exec();
			EPGSearchName = dd->getSearchName();
			if (back == 2)
			{
				dd->hide();
				eMessageBox rsl(
					eString().sprintf(_("%s was not found!"), EPGSearchName.c_str()),
					_("EPG Search"), 
					eMessageBox::iconInfo|eMessageBox::btOK);
				rsl.show(); rsl.exec(); rsl.hide();
				dd->show();
			}
		}
		while (back == 2);
		dd->hide();
		delete dd;
		if (!back)
		{
			// Zeige EPG Ergebnis an
#ifndef DISABLE_LCD
			bool bMain = lcdmain.lcdMain->isVisible();
			bool bMenu = lcdmain.lcdMenu->isVisible();
			lcdmain.lcdMain->hide();
			lcdmain.lcdMenu->show();
#endif
			
			eEPGSelector eEPGSelectorSearch(EPGSearchName);
#ifndef DISABLE_LCD
			eEPGSelectorSearch.setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif
			eEPGSelectorSearch.show(); eEPGSelectorSearch.exec(); eEPGSelectorSearch.hide();
		
#ifndef DISABLE_LCD
			if (!bMenu)
				lcdmain.lcdMenu->hide();
			if ( bMain )
				lcdmain.lcdMain->show();
#endif
		

		}
	//}
	if (wasVisible && !doHideInfobar())
		showInfobar();
	
}

void eZapMain::AnalogSkinClock(tm *timem, bool secOn)
{
	char str[12];
	int H,M,S;

	strftime(str,12,"%l %M %S %d",timem);
	sscanf(str,"%d %d %d",&H,&M,&S);

	aHour->setStart(30*H + M/2);
	aMins->setStart(6*M + (S/20)*2);	// minutes moving every 20 sec
    
	if(secOn)
	{
		if(AnalogNoSec)
		{
			aSecs->show();
			AnalogNoSec = false;
		}
		
		aSecs->setStart(6*S);
	}
	else
	{
		if(!AnalogNoSec)
		{
			aSecs->hide();
			AnalogNoSec = true;
		}    
	}

// eDebug("Analog Clock: %02d:%02d:%02d",H,M,S);
}

void eZapMain::showEPG()
{
	actual_eventDisplay=0;
	if ( doubleklickTimerConnection.connected() )
		doubleklickTimerConnection.disconnect();

	if ( eServiceInterface::getInstance()->service.type != eServiceReference::idDVB)
		return;

	int stype = ((eServiceReferenceDVB&)eServiceInterface::getInstance()->service).getServiceType();

	eServiceReferenceDVB& ref = ( stype > 4 ) ? refservice : (eServiceReferenceDVB&)eServiceInterface::getInstance()->service;
	const eService *service=0;

	service = eServiceInterface::getInstance()->addRef( ref );

	if (!service && !(ref.flags & eServiceReference::isDirectory) )
		return;

	int wasVisible=isVisible();
	if (wasVisible)
	{
		timeout.stop();
		hide();
	}

	ePtrList<EITEvent> events;
	if (isEPG)
	{
		timeMapPtr pMap = eEPGCache::getInstance()->getTimeMapPtr( ref );
		if (pMap)  // EPG vorhanden
		{
			timeMap::const_iterator It = pMap->begin();
			int tsidonid = (ref.getTransportStreamID().get()<<16)|ref.getOriginalNetworkID().get();

			while (It != pMap->end())
			{
				events.push_back( new EITEvent(*It->second, tsidonid, It->second->type) );
				It++;
			}
			actual_eventDisplay=new eEventDisplay( service->service_name.c_str(), ref, &events );
		}
	}
	else
	{
		EIT *eit=eDVB::getInstance()->getEIT();
		if (eit)
		{
			actual_eventDisplay=new eEventDisplay( service->service_name.c_str(), ref, eit?&eit->events:&events);
			eit->unlock(); // HIER liegt der hund begraben.
		}
	}
#ifndef DISABLE_LCD
	lcdmain.lcdMain->hide();
	lcdmain.lcdMenu->show();
#endif
	if (actual_eventDisplay)
	{
#ifndef DISABLE_LCD
		actual_eventDisplay->setLCD(lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#endif
		actual_eventDisplay->show();
		actual_eventDisplay->exec();
		actual_eventDisplay->hide();
	}
	events.setAutoDelete(true);
#ifndef DISABLE_LCD
	lcdmain.lcdMenu->hide();
	lcdmain.lcdMain->show();
#endif
	if (actual_eventDisplay)
	{
		delete actual_eventDisplay;
		actual_eventDisplay=0;
	}

	eServiceInterface::getInstance()->removeRef( ref );
	if (wasVisible && !doHideInfobar())
		showInfobar();
}

void eZapMain::showHelp( ePtrList<eAction>* actionHelpList, eString &helptext )
{
	hide();
	if ((actionHelpList && actionHelpList->size()) || helptext.size())
	{
		eHelpWindow helpwin(*actionHelpList, helptext);
		helpwin.show();
		helpwin.exec();
		helpwin.hide();
	}

	if (!doHideInfobar())
		showInfobar();
}

bool eZapMain::handleState(int justask)
{
	eString text, caption;
	bool b=false;

	int profimode=0;
	eConfig::getInstance()->getKey("/elitedvb/extra/profimode", profimode);

#ifndef DISABLE_FILE
	if ( state & stateRecording )
	{
		if ( state & stateInTimerMode )
		{
			if (state & recDVR )
				text=_("A timer based recording is currently in progress!\n"
							"This stops the timer, and recording!");
			else
				return true; // we wouldn't destroy the VCR Recording *g*
/*				text=_("Currently an timer based VCR recording is in progress!\n"
							"This stops the timer, and the VCR recording!");*/
		}
		else
		{
			if (state & recDVR )
				text=_("A recording is currently in progress!\n"
							"This stops the recording!");
			else
				return true; // we wouldn't destroy the VCR Recording *g*
/*				text=_("Currently an VCR recording is in progress!\n"
							"This stops the VCR recording!");*/
		}
	}
	else
#endif //DISABLE_FILE
	if ( state & stateInTimerMode )
	{
		text=_("A timer event is currently in progress!\n"
					"This stops the timer event!");
	}
	else		// not timer event or recording in progress
		return true;

	if (!profimode)
	{
		int ret = eMessageBox::ShowBox(text, _("Really do this?"), eMessageBox::iconQuestion|eMessageBox::btYes|eMessageBox::btNo, eMessageBox::btNo );
		b = (ret == eMessageBox::btYes);
	}
	else
		b=true;

	if (b && !justask)
	{
		if (state & stateInTimerMode)
		{
			eTimerManager::getInstance()->abortEvent( ePlaylistEntry::errorUserAborted );
		}
#ifndef DISABLE_FILE
		else if (state & stateRecording)
		{
			if (state & recDVR)
				recordDVR(0,0);
			else
				; // here send LIRC VCR Stop
		}
#endif
	}
	return b;
}

#ifndef DISABLE_FILE
void eZapMain::blinkRecord()
{
#ifndef DISABLE_LCD
	// if standby enable lcdMain
	if(state & stateSleeping && !lcdmain.lcdMain->isVisible() )
	{
		lcdmain.lcdStandby->hide();
		lcdmain.lcdMain->show();
	}
#endif

	if (state & stateRecording)
	{
#ifndef DISABLE_LCD
		// handle clock-blinking when record is active..
		if(lcdmain.lcdMain->Clock->isVisible())
			lcdmain.lcdMain->Clock->hide();
		else
			lcdmain.lcdMain->Clock->show();
#endif
		// LED flash for DM500,DM600PVR,DM500PLUS
		if(eSystemInfo::getInstance()->getHwType()==eSystemInfo::DM500 || 
           eSystemInfo::getInstance()->getHwType()==eSystemInfo::DM600PVR ||
		   eSystemInfo::getInstance()->getHwType()==eSystemInfo::DM500PLUS)
		{
			int green=0;int red=1;
			if(led_timer > 2) // interval
			{
				int fd=::open("/dev/dbox/fp0",O_RDWR);
  	        	::ioctl(fd, 11, (state & stateSleeping ? &green : &red));
				::close(fd);
				ledStatusBack.start(12, 1); // time of change - 12ms
				led_timer=0;
			}
			led_timer++;
		}

		if (isVisible())
		{
			if (recstatus->isVisible())
			{
				recstatus->hide();
				recchannel->hide();
			}
			else
			{
				recstatus->show();
				recchannel->show();
			}

			showHDDSpaceLeft();
		}
		recStatusBlink.start(500, 1);
	}
}

void eZapMain::ledBack()
{
	int green=0;int red=1;
	int fd=::open("/dev/dbox/fp0",O_RDWR);
	::ioctl(fd, 11, (state & stateSleeping ? &red : &green));
	::close(fd);
}
#endif // DISABLE_FILE

int eZapMain::eventHandler(const eWidgetEvent &event)
{
	// timer service change in progress...
	if ( event.type == eWidgetEvent::evtAction && Decoder::locked == 2 )
	{
		eMessageBox::ShowBox(
			_("please wait until the timer has started the recording (max 10 seconds)"),
			_("information"),
			eMessageBox::btOK|eMessageBox::iconInfo,
			eMessageBox::btOK,
			10);
		return 1;
	}
	switch (event.type)
	{
	case eWidgetEvent::evtAction:
	{
		int num=0;
		stopMessages();

#ifndef DISABLE_FILE
		if (	skipping )
		{
			// workaround because no action "hideInfobar" is sended
			// when pressing OK-Button
			if (strcmp(event.action->getIdentifier(),"ok") == 0)
			{
				// Toggle Infobar while skipping
				if (isVisible()) 
					hideInfobar(); 
				else 
					showInfobar();
				startMessages();
				return 1;
			}			
			else if(
				event.action != &i_enigmaMainActions->discrete_startSkipForward
				&& dvrfunctions && event.action != &i_enigmaMainActions->startSkipForward &&
				event.action != &i_enigmaMainActions->discrete_repeatSkipForward
				&& dvrfunctions && event.action != &i_enigmaMainActions->repeatSkipForward &&
				event.action != &i_enigmaMainActions->discrete_stopSkipForward
				&& dvrfunctions && event.action != &i_enigmaMainActions->stopSkipForward &&
				event.action != &i_enigmaMainActions->discrete_startSkipReverse
				&& dvrfunctions && event.action != &i_enigmaMainActions->startSkipReverse &&
				event.action != &i_enigmaMainActions->discrete_repeatSkipReverse
				&& dvrfunctions && event.action != &i_enigmaMainActions->repeatSkipReverse &&
				event.action != &i_enigmaMainActions->discrete_stopSkipReverse
				&& dvrfunctions && event.action != &i_enigmaMainActions->stopSkipReverse )
			{
				endSkip();
			}
		}
#endif

		if (event.action == &i_enigmaMainActions->showMainMenu)
		{
			int oldmode=mode;
			showMainMenu();
			if (mode != -1 && mode != oldmode && eServiceInterface::getInstance()->service != modeLast[mode].current() )
			{
#ifndef DISABLE_FILE
				if ( eDVB::getInstance()->recorder && eDVB::getInstance()->recorder->recRef == eServiceInterface::getInstance()->service )
					break;
#endif
				showServiceSelector(-1);
			}
		}
		else if (event.action == &i_enigmaMainActions->standby_press)
		{
			if ( (state&stateInTimerMode && state&stateRecording) || handleState() )
				standbyPress(0);
		}
		else if (event.action == &i_enigmaMainActions->standby_nomenu_press)
		{
			if ( (state&stateInTimerMode && state&stateRecording) || handleState() )
				standbyPress(1);
		}
		else if (event.action == &i_enigmaMainActions->standby_repeat)
			standbyRepeat();
		else if (event.action == &i_enigmaMainActions->standby_release)
			standbyRelease();
		else if ( ( !isVisible() && event.action == &i_enigmaMainActions->showInfobar) ||
			  (isVisible() && stateOSD == 2 && event.action == &i_enigmaMainActions->showInfobar) )
		{
			stateOSD=1;
			showInfobar(true);
		}
		else if ( !isVisible() && event.action == &i_enigmaMainActions->myshowInfobar)
		{
			StartInfoBarTimer();
		}
		else if (event.action == &i_enigmaMainActions->hideInfobar)
		{
			stateOSD=0;
			hideInfobar();
		}
		else if ( isVisible() && event.action == &i_enigmaMainActions->showInfobarEPG)
			showEPG();
		else if (event.action == &i_enigmaMainActions->showServiceSelector)
			showServiceSelector(-1);
		else if (event.action == &i_enigmaMainActions->greenButton)
			executeProgrammableButton("/pli/keyMapping/greenButton", "SubServices");
		else if (event.action == &i_enigmaMainActions->yellowButton)
			executeProgrammableButton("/pli/keyMapping/yellowButton", "PluginScreen");
		else if (event.action == &i_enigmaMainActions->showAudio)
			showAudioMenu();
		else if (event.action == &i_enigmaMainActions->pluginVTXT)
			runVTXT();
		else if (event.action == &i_enigmaMainActions->blueButton)
			executeProgrammableButton("/pli/keyMapping/blueButton", "Menu:eZapSetup");
		else if (event.action == &i_enigmaMainActions->showEPGList && mode != modeFile)
		{
			showSelectorStyleEPG();
		}
		else if ( subservicesel.quickzapmode() && event.action == &i_enigmaMainActions->nextSubService )
		{
			if ( flags&ENIGMA_SUBSERVICES )
				subservicesel.next();
		}
		else if (event.action == &i_enigmaMainActions->nextService)
		{
#ifndef DISABLE_FILE
			if ( eDVB::getInstance()->recorder || handleState() )
#else
			if ( handleState() )
#endif
				nextService();
		}
		else if ( subservicesel.quickzapmode() && event.action == &i_enigmaMainActions->prevSubService )
		{
			if ( flags&ENIGMA_SUBSERVICES )
				subservicesel.prev();
		}
		else if (event.action == &i_enigmaMainActions->prevService)
		{
#ifndef DISABLE_FILE
			if ( eDVB::getInstance()->recorder || handleState() )
#else
			if ( handleState() )
#endif
				prevService();
		}
		else if (!dvrfunctions && event.action == &i_enigmaMainActions->playlistNextService)
		{
#ifndef DISABLE_FILE
			if ( eDVB::getInstance()->recorder || handleState() )
#else
			if ( handleState() )
#endif
				playlistNextService();
		}
		else if (!dvrfunctions && event.action == &i_enigmaMainActions->playlistPrevService)
		{
#ifndef DISABLE_FILE
			if ( eDVB::getInstance()->recorder || handleState() )
#else
			if ( handleState() )
#endif
				playlistPrevService();
		}
		else if (event.action == &i_enigmaMainActions->serviceListDown)
			showServiceSelector(eServiceSelector::dirDown);
		else if (event.action == &i_enigmaMainActions->serviceListUp)
			showServiceSelector(eServiceSelector::dirUp);
		else if (event.action == &i_enigmaMainActions->volumeUp)
			volumeUp();
		else if (event.action == &i_enigmaMainActions->volumeDown)
			volumeDown();
		else if (event.action == &i_enigmaMainActions->toggleMute)
			toggleMute();
#ifndef DISABLE_FILE
		else if (event.action == &i_enigmaMainActions->discrete_play
			|| (dvrfunctions && event.action == &i_enigmaMainActions->play))
			play();
		else if (event.action == &i_enigmaMainActions->discrete_stop
			|| (dvrfunctions && event.action == &i_enigmaMainActions->stop))
		{
			if ( state & stateRecording )
			{
				if ( handleState(1) )
					record();
			}
			else
				stop();
		}
		else if (event.action == &i_enigmaMainActions->discrete_pause
			|| (dvrfunctions && event.action == &i_enigmaMainActions->pause))
			pause();
		else if (event.action == &i_enigmaMainActions->discrete_record
			|| (dvrfunctions && event.action == &i_enigmaMainActions->record))
		{
			if ( state & stateRecording && !(state & stateInTimerMode) )
			{
#ifndef DISABLE_LCD
				eRecordContextMenu menu(LCDTitle, LCDElement);
#else
				eRecordContextMenu menu;
#endif
				hide();
				menu.show();
				int ret = menu.exec();
				menu.hide();
				switch ( ret )
				{
					case 1: // stop record now
						record();
					break;

					case 2: // set Record Duration...
					{
						eTimerInput e;
						e.show();
						EITEvent *evt = (EITEvent*) e.exec();

						if (evt != (EITEvent*)-1)
						{
							int i = e.getCheckboxState();
							eTimerManager::getInstance()->addEventToTimerList( &e, &eServiceInterface::getInstance()->service, evt,
									ePlaylistEntry::stateWaiting|
									ePlaylistEntry::RecTimerEntry|
									ePlaylistEntry::recDVR|
									ePlaylistEntry::doFinishOnly|
									(i==2?ePlaylistEntry::doShutdown:0)|
									(i==3?ePlaylistEntry::doGoSleep:0)
									);
							delete evt;
						}

						e.hide();
					}
					break;

					case 3: // set Record Stop Time...
					{
						eRecTimeInput e;
						e.show();
						EITEvent *evt = (EITEvent*) e.exec();
						if (evt != (EITEvent*)-1)
						{
							int i = e.getCheckboxState();
							eTimerManager::getInstance()->addEventToTimerList( &e, &eServiceInterface::getInstance()->service, evt,
								ePlaylistEntry::stateWaiting|
								ePlaylistEntry::RecTimerEntry|
								ePlaylistEntry::recDVR|
								ePlaylistEntry::doFinishOnly|
								(i==2?ePlaylistEntry::doShutdown:0)|
								(i==3?ePlaylistEntry::doGoSleep:0)
 							);
							delete evt;
						}
						e.hide();
					}
					break;

					case 4: // add timeshifted minutes
						addTimeshiftToRecording();
					break;
					case 0:
					default:
						;
				}
			}
			else if ( handleState(1) )
			{
				record();
			}
		}
		else if (event.action == &i_enigmaMainActions->discrete_startSkipForward
			|| (dvrfunctions && event.action == &i_enigmaMainActions->startSkipForward))
			startSkip(skipForward);
		else if (event.action == &i_enigmaMainActions->discrete_repeatSkipForward
			|| (dvrfunctions && event.action == &i_enigmaMainActions->repeatSkipForward))
			repeatSkip(skipForward);
		else if (event.action == &i_enigmaMainActions->discrete_stopSkipForward
			|| (dvrfunctions && event.action == &i_enigmaMainActions->stopSkipForward))
			stopSkip(skipForward);
		else if (event.action == &i_enigmaMainActions->discrete_startSkipReverse
			|| (dvrfunctions && event.action == &i_enigmaMainActions->startSkipReverse))
			startSkip(skipReverse);
		else if (event.action == &i_enigmaMainActions->discrete_repeatSkipReverse
			|| (dvrfunctions && event.action == &i_enigmaMainActions->repeatSkipReverse))
			repeatSkip(skipReverse);
		else if (event.action == &i_enigmaMainActions->discrete_stopSkipReverse
			|| (dvrfunctions && event.action == &i_enigmaMainActions->stopSkipReverse))
			stopSkip(skipReverse);
#endif // DISABLE_FILE
		else if (event.action == &i_enigmaMainActions->showEPG)
			showEPG_Streaminfo();
		else if (event.action == &i_numberActions->key0)
		{
#ifndef DISABLE_FILE
			if ( (eDVB::getInstance()->recorder || handleState()) && !playlistmode && playlist->getConstList().size() > 1 )
#else
			if ( handleState() && !playlistmode && playlist->getConstList().size() > 1 )
#endif
			{
				std::list<ePlaylistEntry>::iterator swap =
					--playlist->getList().end();

				if ( playlist->current == playlist->getConstList().end() )
					playlist->current = swap;

				if ( playlist->current == swap ) // end of history? 
					--swap;  // swap with previous entry
				else // middle of history
				{
					swap = playlist->current;
					++swap;  // swap with next entry
				}
				eServiceReference ref = eServiceInterface::getInstance()->service;
				int extZap=0;
				eConfig::getInstance()->getKey("/elitedvb/extra/extzapping", extZap);
				if ( extZap || ModeTypeEqual( playlist->current->service, swap->service ) )
				{
					std::iter_swap( playlist->current, swap );
					const eServicePath &p = playlist->current->getPath();
					playService( playlist->current->service, psDontAdd|psSeekPos|(extZap?psSetMode:0) );
					if (p.size() > 1)
						setServiceSelectorPath(p);
				}
			}
		}
		else if ((event.action == &i_numberActions->key1) ||
			(event.action == &i_enigmaMainActions->stepBack)) // For '<' key
			num=1;
		else if (event.action == &i_numberActions->key2)
			num=2;
		else if (event.action == &i_numberActions->key3)
			num=3;
		else if (event.action == &i_numberActions->key4)
			num=4;
		else if (event.action == &i_numberActions->key5)
			num=5;
		else if ((event.action == &i_numberActions->key6) ||
			(event.action == &i_enigmaMainActions->stepForward)) // For '>' key
			num=6;
		else if (event.action == &i_numberActions->key7)
			num=7;
		else if (event.action == &i_numberActions->key8)
			num=8;
		else if (event.action == &i_numberActions->key9)
			num=9;
		else if (mode != modeFile && event.action == &i_enigmaMainActions->showUserBouquets)
			showServiceSelector( -1, state&stateRecording ? 0 : pathBouquets );
		else if (mode != modeFile && event.action == &i_enigmaMainActions->showDVBBouquets)
			showServiceSelector( -1, state&stateRecording ? 0 : pathProvider );
#ifndef DISABLE_FILE
		else if (event.action == &i_enigmaMainActions->showRecMovies)
			showServiceSelector( state&stateRecording ? -1 : eServiceSelector::dirLast, state&stateRecording ? 0 : pathRecordings );
		else if (event.action == &i_enigmaMainActions->showPlaylist)
			showServiceSelector( -1, state&stateRecording ? 0 : pathPlaylist );
#endif
		else if (event.action == &i_enigmaMainActions->modeRadio)
		{
#ifndef DISABLE_FILE
			if ( eDVB::getInstance()->recorder || handleState() )
#else
			if ( handleState() )
#endif
			{
				if ( mode != modeRadio )
				{
					setMode(modeRadio,
#ifndef DISABLE_FILE
					eDVB::getInstance()->recorder ? 0 :
#endif
					1);
				}
				else
				{
					// don't show irri serviceselector when switching beteen TV/radio
					showServiceSelector(-1);
				}
			}
		}
		else if (event.action == &i_enigmaMainActions->modeTV)
		{
#ifndef DISABLE_FILE
			if ( eDVB::getInstance()->recorder || handleState() )
#else
			if ( handleState() )
#endif
			{
				if ( mode != modeTV )
				{
					setMode(modeTV,
#ifndef DISABLE_FILE
					eDVB::getInstance()->recorder ? 0 :
#endif
					1);
				}
				else
				{
					showServiceSelector(-1);
				}
			}
		}
		else if (event.action == &i_enigmaMainActions->modeFile)
		{
#ifndef DISABLE_FILE
			if ( eDVB::getInstance()->recorder || handleState() )
#else
			if ( handleState() )
#endif
			{
				if ( mode != modeFile )
					setMode(modeFile, 2);
				showServiceSelector(-1);
			}
		}
#ifndef DISABLE_FILE
		else if (event.action == &i_enigmaMainActions->toggleDVRFunctions)
		{
			showServiceInfobar(!dvrfunctions);
			showInfobar(true);
		}
		else if (event.action == &i_enigmaMainActions->toggleIndexmark)
			toggleIndexmark();
		else if (event.action == &i_enigmaMainActions->indexSeekNext)
			indexSeek(1);
		else if (event.action == &i_enigmaMainActions->indexSeekPrev)
			indexSeek(-1);
#endif //DISABLE_FILE
		else
		{
			startMessages();
			break;
		}
		startMessages();
#ifndef DISABLE_FILE
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (handler)
		{
			eServiceReference &myref = eServiceInterface::getInstance()->service;

			if ( num && ( (myref.type == eServiceReference::idDVB && myref.path)
				|| (myref.type == eServiceReference::idUser
				&& myref.data[0] == eMP3Decoder::codecMPG ) || (myref.type == eServiceReference::idUser
				&& (myref.data[0] == eMP3Decoder::codecMP3 || myref.data[0] == eMP3Decoder::codecFLAC || myref.data[0] == eMP3Decoder::codecOGG)) || timeshift ) && (handler->getState() == eServiceHandler::statePlaying || handler->getState() == eServiceHandler::statePause)) // nur, wenn ts, mpg oder mp3 ausgewahlt ist und vor allem, wenn es abgespielt wird oder im Standbild ist! :-)
			{
				if (handler->getState() == eServiceHandler::statePause)
					pause();// continue playing in preparation for skipping
				int time=0;
				switch (num)
				{
					case 2:
					{
						SkipEditWindow dlg( "<< Min:" );
						if (myref.type == eServiceReference::idUser && (myref.data[0] == eMP3Decoder::codecMP3 || myref.data[0] == eMP3Decoder::codecFLAC || myref.data[0] == eMP3Decoder::codecOGG))
							dlg.setEditText("1");
						else
							dlg.setEditText("6");

						dlg.show();
						int ret=dlg.exec();
						dlg.hide();

						if(!ret)
						{
							time=atoi(dlg.getEditText().c_str())*60;
							time = -time;
						}
						else
							time = 0;
						break;
					}
					case 8:
					{
						SkipEditWindow dlg( ">> Min:" );
						if (myref.type == eServiceReference::idUser && (myref.data[0] == eMP3Decoder::codecMP3 || myref.data[0] == eMP3Decoder::codecFLAC || myref.data[0] == eMP3Decoder::codecOGG))
							dlg.setEditText("1");
						else
							dlg.setEditText("6");

						dlg.show();
						int ret=dlg.exec();
						dlg.hide();

						if(!ret)
							time=atoi(dlg.getEditText().c_str())*60;
						else
							time = 0;
						break;
					}
					case 1:
						time = -15;
						break;
					case 4:
						time = -60;
						break;
					case 7:
						time = -300;
						break;
					case 3:
						time = 15;
						break;
					case 6:
						time = 60;
						break;
					case 9:
						time = 300;
						break;
				}
				if (time)
				{
					if (skipping) 
						endSkip();
					handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekBegin));
					if (myref.type == eServiceReference::idUser)
					{
						switch(myref.data[0])
						{
							case eMP3Decoder::codecMPG:
								handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip,time*2000));
								break;
							case eMP3Decoder::codecMP3:
							case eMP3Decoder::codecFLAC:
							case eMP3Decoder::codecOGG:
								handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip,time*3800));
								break;
						}
					}
					#if 0
					else if (Decoder::current.vpid==-1) // Radio
						handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip,time*24));
					#endif
					else
						handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip,time*1000)); // ca in TS
					handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekEnd));
					updateProgress();
					int showosd = 1;
					eConfig::getInstance()->getKey("/ezap/osd/showOSDOnSwitchService", showosd );
					if (showosd)
						showInfobar(true);

				}
				return 1;
			}
		}
		if ( num && (!eDVB::getInstance()->recorder || handleState() ) )
#else
		if ( num && handleState() )
#endif
		{
			hide();
			eServiceNumberWidget s(num);
			s.show();
			int number = s.exec();
			s.hide();
			int num = number;
			while( switchToNum(num) == -1 ) // parental locked
				++num;
			num = 0;
  	}
		return 1;
	}
	default:
		break;
	}
	return eWidget::eventHandler(event);
}

void eZapMain::executeProgrammableButton(const eString& buttonKey, const eString& defaultAction)
{
	eString buttonType = defaultAction;
	eConfig::getInstance()->getKey(buttonKey.c_str(), buttonType);
			
	if(buttonType == "AudioSelection")
	{
		showAudioMenu();
	}
	else if(buttonType == "TimeshiftPause")
	{
		// TODO: Do we need to check if timeshifting is running?
		// Otherwise, normal recordings are started and immediately paused too
		pause();
	}
	else if(buttonType == "PluginScreen")
	{
		showPluginScreen();
	}
	else if(buttonType == "Teletext")
	{
		runVTXT();
	}
	else if(buttonType == "SubServices")
	{
#ifndef DISABLE_FILE
			if ( eDVB::getInstance()->recorder || handleState() )
#else
			if ( handleState() )
#endif
				showSubserviceMenu();
	}
	else if(buttonType.left(7) == "Plugin:")
	{
		hide();
		eZapPlugins plugins(eZapPlugins::StandardPlugin, lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#ifndef DISABLE_LCD
		bool bMain = lcdmain.lcdMain->isVisible();
		bool bMenu = lcdmain.lcdMenu->isVisible();
		lcdmain.lcdMain->hide();
		lcdmain.lcdMenu->show();
#endif
		plugins.execPluginByName(buttonType.mid(7).c_str());
#ifndef DISABLE_LCD
		if(!bMenu)
			lcdmain.lcdMenu->hide();
		if(bMain)
			lcdmain.lcdMain->show();
#endif
		if(!doHideInfobar())
			showInfobar();
	}
	else if(buttonType.left(5) == "Menu:")
	{
		hide();
#ifndef DISABLE_LCD
		bool bMain = lcdmain.lcdMain->isVisible();
		bool bMenu = lcdmain.lcdMenu->isVisible();
		lcdmain.lcdMain->hide();
		lcdmain.lcdMenu->show();
#endif
		eCallableMenuFactory::showMenu(buttonType.mid(5), lcdmain.lcdMenu->Title, lcdmain.lcdMenu->Element);
#ifndef DISABLE_LCD
		if(!bMenu)
			lcdmain.lcdMenu->hide();
		if(bMain)
			lcdmain.lcdMain->show();
#endif
		if(!doHideInfobar())
			showInfobar();
	}
}

int eZapMain::switchToNum( int num, bool onlyBouquets )
{
	int orgnum=num;
	if (num != -1)
	{
		eServicePath path;
		ePlaylist *p=0;
		switch(mode)
		{
			case modeTV:
				if ( userTVBouquets->getList().size() )
				{
					p=userTVBouquets;
					path.down(userTVBouquetsRef);
				}
				break;
			case modeRadio:
				if ( userRadioBouquets->getList().size() )
				{
					p=userRadioBouquets;
					path.down(userRadioBouquetsRef);
				}
				break;
			case modeFile:
				if ( userFileBouquets->getList().size() )
				{
					p=userFileBouquets;
					path.down(userFileBouquetsRef);
				}
				break;
		}
		if ( p )
		{
			for ( std::list<ePlaylistEntry>::const_iterator it ( p->getConstList().begin() );
				it != p->getConstList().end() && num; ++it)
			{
				ePlaylist *pl=0;
				if ( it->service.type == eServicePlaylistHandler::ID )
					pl = (ePlaylist*)eServiceInterface::getInstance()->addRef( it->service );
				if ( pl )
				{
					for (std::list<ePlaylistEntry>::const_iterator i(pl->getConstList().begin());
						i != pl->getConstList().end(); ++i )
					{
						if ( i->service.flags & eServiceReference::isMarker )
							continue;
						if ( !--num )
						{
							path.down(it->service);
							path.down(i->service);
							setServiceSelectorPath(path);
							modeLast[mode]=path;
							if ( i->service.isLocked() && (pinCheck::getInstance()->pLockActive()&2) )
								return -1;
							playService( i->service, 0);
							return 0;
						}
					}
				}
				eServiceInterface::getInstance()->removeRef( it->service );
			}
		}
	}
	if ( num )
	{
#ifndef DISABLE_FILE
		if ( mode == modeFile )
		{
			for (std::list<ePlaylistEntry>::const_iterator i(recordings->getConstList().begin());
				i != recordings->getConstList().end(); ++i )
			{
				if ( i->service.flags & eServiceReference::isMarker )
					continue;
				if ( !--num )
				{
					eServicePath path;
					path.down(recordingsref);
					path.down(i->service);
					setServiceSelectorPath(path);
					modeLast[mode]=path;
					if ( i->service.isLocked() && (pinCheck::getInstance()->pLockActive()&2) )
						return -1;
					playService( i->service, 0);
					return 0;
				}
			}
		}
		else
#endif
		{
			eServiceReferenceDVB s=eDVB::getInstance()->settings->getTransponders()->searchServiceByNumber(orgnum);
			if (s && !onlyBouquets)
			{
				eServicePath path(getRoot(listAll));  // All Services
				path.down(s); // current service
				setServiceSelectorPath(path);
				modeLast[mode]=path;
				if ( s.isLocked() )
					return -1;
				playService(s, 0);
				return 0;
			}
		}
	}
	return num;
}

void showRadioPic()
{
	if (access(CONFIGDIR "/enigma/pictures/radio.mvi", R_OK) == 0)
		Decoder::displayIFrameFromFile(CONFIGDIR "/enigma/pictures/radio.mvi" );
	else
		Decoder::displayIFrameFromFile(TUXBOXDATADIR "/enigma/pictures/radio.mvi" );
}

void showMP3Pic()
{
	if (access(CONFIGDIR "/enigma/pictures/mp3.mvi", R_OK) == 0)
		Decoder::displayIFrameFromFile(CONFIGDIR "/enigma/pictures/mp3.mvi" );
	else if (access(TUXBOXDATADIR "/enigma/pictures/mp3.mvi", R_OK) == 0)
		Decoder::displayIFrameFromFile(TUXBOXDATADIR "/enigma/pictures/mp3.mvi" );
	else
		showRadioPic();
}

void eZapMain::handleServiceEvent(const eServiceEvent &event)
{
	if ( Decoder::locked == 2 )  // timer zap in background
		return;

	switch (event.type)
	{
	case eServiceEvent::evtStateChanged:
		break;
	case eServiceEvent::evtFlagsChanged:
	{
		serviceFlags = eServiceInterface::getInstance()->getService()->getFlags();
#ifndef DISABLE_FILE
		if ( timeshift && !(serviceFlags & eServiceHandler::flagIsSeekable) )
		{
			if ( eDVB::getInstance()->recorder &&
				eServiceInterface::getInstance()->service != eDVB::getInstance()->recorder->recRef )
				playService(eDVB::getInstance()->recorder->recRef, psNoUser|psSetMode);
			timeshift=0;
		}
		if ( serviceFlags & eServiceHandler::flagStartTimeshift )
			timeshift=1;
#endif
		setSmartcardLogo( serviceFlags & eServiceHandler::flagIsScrambled );
		if (serviceFlags & eServiceHandler::flagSupportPosition)
			progresstimer.start(1000);
		else
		{
#ifndef DISABLE_FILE
			// disable skipping
			if(skipping)
				endSkip();
#endif
			progresstimer.stop();
		}
		updateProgress();
		break;
	}
	case eServiceEvent::evtAspectChanged:
	{
		int aspect = eServiceInterface::getInstance()->getService()->getAspectRatio();
		set16_9Logo(aspect);
        	VidFormat->setText(getVidFormat());
		break;
	}
	case eServiceEvent::evtStart:
	{
		int err = eServiceInterface::getInstance()->getService()->getErrorInfo();
		serviceFlags = eServiceInterface::getInstance()->getService()->getFlags();

		showServiceInfobar(serviceFlags & eServiceHandler::flagIsSeekable);

		eServiceReference &ref = eServiceInterface::getInstance()->service;
		startService(ref, err);

		rdstext_decoder.clear_service();

		validEITReceived = false;
		
// SHOW PICTURE
		int iHideBgInRadioMp3Mode = 0;
		eConfig::getInstance()->getKey("/ezap/osd/hidebginradiomode", iHideBgInRadioMp3Mode) ;

		switch(mode)
		{
#ifndef DISABLE_FILE
			case modeFile:
				if (ref.type == eServiceReference::idUser &&
					(ref.data[0] == eMP3Decoder::codecMP3 ||
					ref.data[0] == eMP3Decoder::codecFLAC ||
					ref.data[0] == eMP3Decoder::codecOGG) &&
					iHideBgInRadioMp3Mode == 0)				{
					eAVSwitch::getInstance()->setVSystem(vsPAL);
					showMP3Pic();
					break;
				}
#endif
			case modeRadio:
				if (ref.type == eServiceReference::idDVB &&
					Decoder::current.vpid == -1 &&
					iHideBgInRadioMp3Mode == 0)				{
					eAVSwitch::getInstance()->setVSystem(vsPAL);
					showRadioPic();
				}
				break;
			default:
				break;
		}
        	VidFormat->setText(getVidFormat());
		break;
	}
	case eServiceEvent::evtStop:
		lfreq_val->setText("---");
		lsymrate_val->setText("---");
		lpolar_val->setText("-");
		lfec_val->setText("---");
		leaveService();
		break;
	case eServiceEvent::evtGotEIT:
	{
//		eDebug("enigma_main::handleServiceEvent.. evtGotEIT");
		gotEIT();
		break;
	}
	case eServiceEvent::evtGotSDT:
		gotSDT();
		break;
	case eServiceEvent::evtGotPMT:
		gotPMT();
		break;
#ifndef DISABLE_FILE
	case eServiceEvent::evtRecordFailed:
	{
		int freespace = freeRecordSpace();

		if ( state&stateInTimerMode )
		{
			if (state & stateRecording)
				message_notifier.send(eZapMain::messageNoRecordSpaceLeft);
		} else
			recordDVR(0, 0);  // stop Recording

		const char *message;
		if (freespace < 0)
			message=_("Record failed due to inaccessable storage.");
		else if (freespace < 10)
			message=_("Record stopped due to full harddisk. Delete some recordings and try again.");
		else
			message=_("Record failed due to a write error during recording. Check for filesystem corruption.");

		postMessage(eZapMessage(1, _("record failed"), message, -1), 0);
		break;
	}
	case eServiceEvent::evtEnd:
	{
		// disable skipping
		if(skipping)
			endSkip();
		int serviceFlags = eServiceInterface::getInstance()->getService()->getFlags();
		if (playlist->current != playlist->getConstList().end())
		{
			playlist->current->current_position=-1;
			++playlist->current;
		}
		if (playlist->current != playlist->getConstList().end())
		{
			eServiceReference &ref = *playlist->current;
			if ( ref.path )  // is the current playlist entry a file?
			{
				playlist->current->current_position=-1;	// start from beginning
				eServiceInterface::getInstance()->play((eServiceReference&)(*playlist->current));
			}
			else  // when not hold prev service
				--playlist->current;
		}
		else if (!playlistmode)
		{
			if(! (serviceFlags & eServiceHandler::flagIsTrack)  )  // current service is a track (file)
				break;
			nextService(1);
		}
		else
			eDebug("end in the area.");
		break;
	}
	case eServiceEvent::evtStatus:
	{
		eServiceHandler *sapi=eServiceInterface::getInstance()->getService();
		if (!sapi)
			return;
		//showServiceInfobar(0);
		// eDebug("Status=%s", sapi->getInfo(0).c_str());
		eString str1 = sapi->getInfo(1);
		eString str2 = sapi->getInfo(2);
		fileinfos->setText(sapi->getInfo(0));
		fileinfos->setText(sapi->getInfo(0) +
					(str1.length() > 2 ? "\n" + str1 : "") +
					(str2.length() > 2 ? "\n" + str2 : ""));
		break;
	}
	case eServiceEvent::evtInfoUpdated:
	{
		// only seems to be generated by servicemp3!
		eServiceHandler *sapi=eServiceInterface::getInstance()->getService();
		if (!sapi)
			return;
		//eDebug("Info1=%s", sapi->getInfo(1).c_str());
		//eDebug("Info2=%s", sapi->getInfo(2).c_str());
		//showServiceInfobar(0);
		eString str0 = sapi->getInfo(1);
		eString str = sapi->getInfo(2);
		if (str0.length() || str.length())
			fileinfos->setText(str0 + (str.length() > 2 ? "\n" + str : ""));

		str = sapi->getInfo(3);
		if (str.length())
			VidFormat->setText(str);
		str = sapi->getInfo(4);
		if (str.length())
			lfreq_val->setText(str);
		str = sapi->getInfo(5);
		if (str.length())
			lsymrate_val->setText(str);
		break;
	}
	case eServiceEvent::evtAddNewAudioStreamId:
//		eDebug("case eServiceEvent::evtAddNewAudioStreamId:.. %02x", event.param );
		flags|=ENIGMA_AUDIO_PS;
		audioselps.add(event.param);
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (!handler)
			return;

		// get previous selected audio stream_id
		unsigned int audioStreamID=0;
		eConfig::getInstance()->getKey("/ezap/audio/prevAudioStreamID", audioStreamID);

		// check if playback begin in the middle of the file
		if ( playlist->current != playlist->getConstList().end()
			&& playlist->current->current_position != -1 )
			eDebug("dont seek to begin");
		else if ( audioselps.getCount() == 1 )
// first stream_id found.. seek to begin.. we will see the complete file :)
		{
			Decoder::Pause(0);
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekReal, 0));
			usleep(200);
			Decoder::flushBuffer();
			Decoder::Resume(false);
		}
// new audio stream_id found.. when this is our saved stream_id.. then change
		if ( audioselps.getCount() > 1 )
		{
			// ButtonYellowDis->hide();
			// ButtonYellowEn->show();
			if ( audioStreamID && audioStreamID == (unsigned int)event.param )
				handler->setAudioStream(event.param);
		}
		break;
#endif // DISABLE_FILE
	}
}

void eZapMain::showEPG_Streaminfo()
{
	if ( doubleklickTimer.isActive() )
	{
		doubleklickTimer.stop();
		doubleklickTimerConnection.disconnect();
		if ( isVisible() )
			hide();
		eStreaminfo si(0, eServiceInterface::getInstance()->service);
#ifndef DISABLE_LCD
		si.setLCD(LCDTitle, LCDElement);
#endif
		si.show();
		si.exec();
		si.hide();
		if (!doHideInfobar())
			showInfobar();
	}
	else
	{
		doubleklickTimer.start(400,true);
		doubleklickTimerConnection = CONNECT( doubleklickTimer.timeout, eZapMain::showEPG );
	}
}

void eZapMain::startService(const eServiceReference &_serviceref, int err)
{

	subtitle->stop();
	audioselps.clear();

	int tmp = -1;
	eService *sp=eServiceInterface::getInstance()->addRef(_serviceref);
	if (sp)
	{
		if (sp->dvb)
		{
			tmp = sp->dvb->get(eServiceDVB::cStereoMono);
			if ( tmp != -1)
				eAVSwitch::getInstance()->selectAudioChannel(tmp);
			int tmp2 = sp->dvb->get(eServiceDVB::cSubtitle);
			if ( tmp2 != -1)
			{
				eSubtitleWidget *i = eSubtitleWidget::getInstance();
				if (i)
				{
					std::set<int> pages; pages.insert(-1);
					i->start(tmp2, pages);
				}
			}
			int tmp3 = sp->dvb->get(eServiceDVB::cTTXSubtitle);
			if ( tmp3 != -1)
			{
				eSubtitleWidget *i = eSubtitleWidget::getInstance();
				if (i)
				{
					i->startttx(tmp3);
				}
			}
		}
		eServiceInterface::getInstance()->removeRef(_serviceref);
	}
	if (tmp == -1)
		eAVSwitch::getInstance()->selectAudioChannel(1);

#ifndef DISABLE_FILE
	skipcounter=0;
#endif
	eServiceHandler *sapi=eServiceInterface::getInstance()->getService();

	if (!sapi)
		return;

	eService *service=eServiceInterface::getInstance()->addRef(_serviceref);

#ifndef DISABLE_FILE
			/* enable indices when we have something to store them. */
	if (_serviceref.path.size())
	{
		indices.clear();
		indices.load(_serviceref.path + ".indexmarks");
		indices_enabled = 1;
	} else
		indices_enabled = 0;
#endif

	showFreq();
	VidFormat->setText("n/a");
	if (_serviceref.type == eServiceReference::idDVB )
	{
		isVT = Decoder::current.tpid != -1;

		const eServiceReferenceDVB &serviceref=(const eServiceReferenceDVB&)_serviceref;

		setVTButton(isVT);

		// es wird nur dann versucht einen service als referenz-service zu uebernehmen, wenns den ueberhaupt
		// gibt.

		switch (serviceref.getServiceType())
		{
			case 1: // TV
			case 2: // radio
			case 4: // nvod ref
				refservice=serviceref;
				break;
		}

		eService *rservice=0;

		  // linkage or nvod ?
		if ( ( refservice != serviceref ||
			// need this compare since the dvb service type is no more compared in eServiceReference ==, !=, <
			refservice.getServiceType() != serviceref.getServiceType() )
			&& !( refservice.flags & eServiceReference::flagDirectory )
			&& !serviceref.path.length() )
		{
			rservice=eServiceInterface::getInstance()->addRef(refservice);

			if (refservice.getServiceType()==4) // nvod ref service
				flags|=ENIGMA_NVOD;
		}

		switch ( serviceref.getServiceType() )
		{
			case 4:
				flags|=ENIGMA_NVOD;
			default:
				subservicesel.disableQuickZap();
			case 7:
				break;
		}

		eString name="";

		if (rservice)
		{
			if ( refservice.descr.length() )
				name = refservice.descr;
			else
				name = rservice->service_name;

			eServiceInterface::getInstance()->removeRef( refservice );
		}
		else if (serviceref.descr.length())
			name = serviceref.descr;
		else if (service)
			name=service->service_name;
		if (!name.length())
			name="unknown service";

		if (serviceref.getServiceType() == 7)  // linkage service..
			name+=" - " + serviceref.descr;

#ifndef DISABLE_LCD
		lcdmain.lcdMain->setServiceName(name);
#endif

		if ( !serviceref.path.length() &&   // no recorded movie
				eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )
		{
			int opos=0;
			if (rservice)
				opos = refservice.data[4]>>16;
			else
				opos = serviceref.data[4]>>16;

			int showSatPos = 1;
			eConfig::getInstance()->getKey("/extras/showSatPos", showSatPos);
			if (showSatPos == 1 && eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite)
				name+=eString().sprintf(" (%d.%d\xC2\xB0%c)", abs(opos / 10), abs(opos % 10), opos>0?'E':'W');
//			name+=eString().sprintf("(%04x)",((eServiceReferenceDVB&)_serviceref).getServiceID().get() );
		}

		ChannelName->setText(name);
		ProviderName->setText((service && service->dvb)?service->dvb->service_provider.c_str():"");
	
		SoftCam->setText(softcamName);
		SoftcamInfo->setText(softcamInfo);	
		showECM(currentcaid);

		switch (err)
		{
		case 0:
			Description->setText("");
			postMessage(eZapMessage(0), 1);
			break;
		case -EAGAIN:
			Description->setText(_("One moment please..."));
			postMessage(eZapMessage(0, _("switch"), _("One moment please...")), 1);
			break;
		case -ENOENT:
			{
				int hideErrorWindows = 0;
				eConfig::getInstance()->getKey("/elitedvb/extra/hideerror", hideErrorWindows);
				if(!hideErrorWindows)
				{
					postMessage(eZapMessage(0, _("switch"), _("Service could not be found !")), 1);
				}
				Description->setText(_("Service could not be found !"));
			}
			break;
		case -ENOCASYS:
			{
				int serviceFlags = eServiceInterface::getInstance()->getService()->getFlags();
				if( serviceFlags & eServiceHandler::flagIsScrambled )
				{
					int hideErrorWindows = 0;
					eConfig::getInstance()->getKey("/elitedvb/extra/hideerror", hideErrorWindows);
					if(!hideErrorWindows)
					{
						postMessage(eZapMessage(0, _("switch"), _("This service could not be descrambled"), 2), 1);
					}
					Description->setText(_("This service could not be descrambled"));
					eDebug("This service could not be descrambled");
				}
			}
			break;
		case -ENOSTREAM:
			{
				int hideErrorWindows = 0;
				eConfig::getInstance()->getKey("/elitedvb/extra/hideerror", hideErrorWindows);
				if(!hideErrorWindows)
				{
					postMessage(eZapMessage(0, _("switch"), _("This service doesn't currently send a signal"), 2), 1);
				}
				Description->setText(_("This service doesn't currently send a signal"));
				eDebug("This service doesn't currently send a signal");
			}
			break;
		case -ENOSYS:
			{
				int hideErrorWindows = 0;
				eConfig::getInstance()->getKey("/elitedvb/extra/hideerror", hideErrorWindows);
				if(!hideErrorWindows)
				{
					postMessage(eZapMessage(0, _("switch"), _("This content could not be displayed"), 2), 1);
				}
				Description->setText(_("This content could not be displayed"));
				eDebug("This content could not be displayed");
			}
			break;
		case -ENVOD:
			Description->setText(_("NVOD Service - please select a starttime"));
			eDebug("NVOD Service - please select a starttime");
			postMessage(eZapMessage(0, _("switch"), _("NVOD Service - please select a starttime"), 5), 1);
			break;
		default:
			Description->setText(_("Unknown error!!"));
			eDebug("Unknown error!!");
			postMessage(eZapMessage(0, _("switch"), _("Unknown error!!")), 1);
			break;
		}
		updateServiceNum( _serviceref );

		if (flags&(ENIGMA_NVOD|ENIGMA_SUBSERVICES|ENIGMA_VIDEO))
		{
			ButtonGreenDis->hide();
			ButtonGreenEn->show();
		}
		else
		{
			ButtonGreenEn->hide();
			ButtonGreenDis->show();
		}
	}
#ifndef DISABLE_FILE
	else
	{
		eString name;
		postMessage(eZapMessage(0), 1);
		if (service)
			name=service->service_name;
		else
			name="bla :(";

		ChannelName->setText(name);
#ifndef DISABLE_LCD
		lcdmain.lcdMain->setServiceName(name);
#endif
		if (service && service->id3)
		{
			std::map<eString,eString> &tags = service->id3->getID3Tags();
			eString artist="unknown artist", album="unknown album", title="", num="", mp3info="";
			if (tags.count("TALB"))
				album=tags.find("TALB")->second;
			if (tags.count("TIT2"))
				title=tags.find("TIT2")->second;
			if (tags.count("TPE1"))
				artist=tags.find("TPE1")->second;
			if (tags.count("TRCK"))
				num=tags.find("TRCK")->second;
			if (tags.count("MP3M"))
				mp3info=tags.find("MP3M")->second;
			eString text = artist + '\n' + album + '\n';
			if (num)
				text +='[' + num + ']' + ' ';
			text+=title;
			fileinfos->setText(text);
		}
	}
#endif // DISABLE_FILE

	cur_event_id = -1;

	eServiceInterface::getInstance()->removeRef(_serviceref);

	// Quick und Dirty ... damit die aktuelle Volume sofort angezeigt wird.
	eAVSwitch::getInstance()->sendVolumeChanged();


	showInfobarOnZap = 1;
	eConfig::getInstance()->getKey("/ezap/osd/showOSDOnSwitchService", showInfobarOnZap);
	if (showInfobarOnZap) {
		showInfobar(true);
		if (doHideInfobar())
		{
			eConfig::getInstance()->getKey("/enigma/timeoutInfobar", timeoutInfobar);
			timeout.start((sapi->getState() == eServiceHandler::statePlaying) ?
				(timeoutInfobar * 1000) - 1000 :
				2000, 1);
		}
	}
}

void eZapMain::gotEIT()
{
	eServiceHandler *sapi=eServiceInterface::getInstance()->getService();

	if (!sapi)
	{
		eDebug("no sapi");
		return;
	}

	EIT *eit=sapi->getEIT();
	int old_start=cur_start;

	setEIT(eit);

	if (eit)	
	{
		int state=0;
		if (old_start != cur_start)
		{
			eConfig::getInstance()->getKey("/ezap/osd/showOSDOnEITUpdate", state);

			if (old_start == -1 || state)
			{
				if (old_start != -1)
					showInfobar(true);

				if ( doHideInfobar() && isVisible() )
				{
					eConfig::getInstance()->getKey("/enigma/timeoutInfobar",
									timeoutInfobar);
					timeout.start((sapi->getState() == eServiceHandler::statePlaying) ?
						(timeoutInfobar * 1000) - 1000 :
						2000, 1);
				}
			}
		}
		if (eit) eit->unlock();
	}
	else
	{
		eDebug("no eit");
	}
}

void eZapMain::gotSDT()
{
	eServiceHandler *sapi=eServiceInterface::getInstance()->getService();
	if (!sapi)
		return;

	SDT *sdt=sapi->getSDT();
	if (!sdt)
		return;

	switch (((eServiceReferenceDVB&)eServiceInterface::getInstance()->service).getServiceType())
	{
	case 0x4:	// nvod reference
	{
		for (ePtrList<SDTEntry>::iterator i(sdt->entries); i != sdt->entries.end(); ++i)
		{
			if (eServiceID(i->service_id)==((eServiceReferenceDVB&)eServiceInterface::getInstance()->service).getServiceID())
			{
				handleNVODService(*i);
			}
		}
		break;
	}
	}
	sdt->unlock();
}

void eZapMain::gotPMT()
{
	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
	if (!sapi)
		return;

	audiosel.clear();
	videosel.clear();

	bool isAC3=false;
	for (std::list<eDVBServiceController::audioStream>::iterator it( sapi->audioStreams.begin() )
		;it != sapi->audioStreams.end(); ++it )
	{
		audiosel.add(*it);
		if ( it->isAC3 || it->isDTS )
			isAC3=true;
	}

	for (ePtrList<PMTEntry>::iterator it(sapi->videoStreams); it != sapi->videoStreams.end(); ++it)
		videosel.add(*it);

	for (ePtrList<PMTEntry>::iterator it(sapi->subtitleStreams); it != sapi->subtitleStreams.end(); ++it)
		audiosel.addSubtitle(*it);

	if (sapi->audioStreams.size()>1)
	{
		// ButtonYellowDis->hide();
		// ButtonYellowEn->show();
		AudioOn->show();
		AudioOff->hide();
	}
	else
	{
		AudioOn->hide();
		AudioOff->show();
	}

	if (sapi->audioStreams.size())
		flags|=ENIGMA_AUDIO;
	else
		flags&=~ENIGMA_AUDIO;

	if (sapi->videoStreams.size()>1)
		flags|=ENIGMA_VIDEO;
	else
		flags&=~ENIGMA_VIDEO;

	setAC3Logo(isAC3);
}

void eZapMain::timeOut()
{
	int state = 0;
	eConfig::getInstance()->getKey("/ezap/osd/enableAutohideOSDOn", state);
	/* only stateOSD 1 is 'permanent', unless enableAutohideOSDOn is set */
	if ( doHideInfobar() && (currentFocus==this) && (state || (stateOSD != 1)))
 	{
		stateOSD=0;
		hide();
	}	
}

void eZapMain::leaveService()
{
	timeshift=0;

#ifndef DISABLE_FILE
	// disable skipping
	if(skipping)
		endSkip();
	stopPermanentTimeshift();
#endif

	cur_start=cur_duration=cur_event_id=-1;

	ButtonGreenDis->show();
	ButtonGreenEn->hide();
	// Leave Yellow button enabled!
	//ButtonYellowDis->show();
	//ButtonYellowEn->hide();

#ifndef DISABLE_FILE
	if ( eDVB::getInstance()->recorder && eDVB::getInstance()->recorder->recRef.getServiceType() == 7 )
		flags&=~(ENIGMA_NVOD|ENIGMA_AUDIO|ENIGMA_AUDIO_PS|ENIGMA_VIDEO);
	else
#endif
		flags&=~(ENIGMA_NVOD|ENIGMA_SUBSERVICES|ENIGMA_AUDIO|ENIGMA_AUDIO_PS|ENIGMA_VIDEO);

	if (subtitle)
		subtitle->stop();

	ChannelName->setText("");
//	ChannelNumber->setText("");
 	ProviderName->setText("");	
	Description->setText("");

	fileinfos->setText("");
	EINow->setText("");
	EINowDuration->setText("");
	EINowTime->setText("");
	EINext->setText("");
	EINextDuration->setText("");
	EINextTime->setText("");
	EINextETime->setText("");

	Progress->hide();
#ifndef DISABLE_FILE
	if (indices_enabled)
	{
			/* this will delete the index file if no index is left: */
			/* the filename is the one from the load()-call */
		indices.save();
		indices.clear();
		indices_enabled = 0;
	}
#endif
}

void eZapMain::clockUpdate()
{
	struct timeval tim;

	gettimeofday(&tim, NULL);
	
	tim.tv_sec += eDVB::getInstance()->time_difference;
	tm *t=localtime(&tim.tv_sec);
	if (t && eDVB::getInstance()->time_difference)
	{
		eString s;
		int dosecs = 0;
		// should get a more global var on OSD show instead of doing getKey every time...
		eConfig::getInstance()->getKey("/ezap/osd/clockSeconds", dosecs) ;

		clocktimer.start(dosecs ? 1000 - tim.tv_usec/1000 : 60000 - (tim.tv_sec % 60)*1000 - tim.tv_usec/1000);
		s = getTimeStr(t, dosecs ? gTS_SECS : 0);
		// clocktimer.start((60-t->tm_sec)*1000);
		// assume we are always within one second of the minute
		Clock->setText(s);
		Date->setText(getDateStr(t, eSkin::getActive()->queryValue("date.format", 1 )));
		AnalogSkinClock(t, dosecs);

		if (t->tm_sec == 0)
		{
			if( !eSystemInfo::getInstance()->hasLCD()
				&& eZapStandby::getInstance() ) //  in standby
			{
			        int clktype = 0;
				int num;
        			eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);
				if (clktype == 1)
					num = ((t->tm_hour%12) ? t->tm_hour%12 : 12)*100 + t->tm_min;
				else
					num = t->tm_hour*100 + t->tm_min;
				//eDebug("write time to led-display");
				int fd=::open("/dev/dbox/fp0",O_RDWR);
				::ioctl(fd,4,&num);
				::close(fd);
			}
#ifndef DISABLE_LCD
			else
			{
				s = getTimeStr(t, gTS_NOAPM);
				lcdmain.lcdMain->Clock->setText(s);
				lcdmain.lcdStandby->Clock->setText(s);
			}
#endif
		}
	} 
	else
	{
		Clock->setText("--:--");
		Date->setText("unknown");
		clocktimer.start(60000 - (tim.tv_sec % 60)*1000 - tim.tv_usec/1000);
		if( !eSystemInfo::getInstance()->hasLCD()
			&& eZapStandby::getInstance() ) //  in standby
		{
			int num=9999;
			//eDebug("write time to led-display");
			int fd=::open("/dev/dbox/fp0",O_RDWR);
			::ioctl(fd,4,&num);
			::close(fd);
		}
#ifndef DISABLE_LCD
		else
		{
			lcdmain.lcdMain->Clock->setText("--:--");
			lcdmain.lcdStandby->Clock->setText("--:--");
		}
#endif
	}
	updateProgress();
}

void eZapMain::updateVolume(int mute_state, int vol)
{
	//int show=(!currentFocus) || (currentFocus == this);

	if (mute_state)
	{
		volume.hide();
		mute.show();
	}
	else
	{
		VolumeBar->setPerc((63-vol)*100/63);
		IBVolumeBar->setPerc((63-vol)*100/63);
		mute.hide();
	}
}

void eZapMain::postMessage(const eZapMessage &message, int clear)
{
	eLocker l(messagelock);

	int c=0;
	if (clear)
	{
		for (std::list<eZapMessage>::iterator i(messages.begin()); i != messages.end(); )
		{
			if (message.isSameType(*i))
			{
				if (i == messages.begin())
				{
					c=1;
					++i;
				} else
					i = messages.erase(i);
			} else
				++i;
		}
	}
	if (!message.isEmpty())
		messages.push_back(message);
	message_notifier.send(c);
}

void eZapMain::gotMessage(const int &c)
{
	switch (c)
	{
		case eZapMain::messageGoSleep:
			if (!eZapStandby::getInstance())
			{
	// close all open windows before goto standby
				if(eApp->looplevel() > 1)
				{
					eApp->exit_loop();
					message_notifier.send(c);
					return;
				}
				eDebug("goto Standby (sleep)");
				standbyPress(0);
				standbyRelease();
			}
			return;
		case eZapMain::messageWakeUp:
			if ( eZapStandby::getInstance() )
				eZapStandby::getInstance()->wakeUp(0);
			else if ( enigmaVCR::getInstance() )
				enigmaVCR::getInstance()->switchBack();
			return;
		case eZapMain::messageCheckVCR:
			eStreamWatchdog::getInstance()->reloadSettings();
			return;
#ifndef DISABLE_FILE
		case eZapMain::messageNoRecordSpaceLeft:
			if (state & stateInTimerMode)
			{
			 	if (state & stateRecording)
					eTimerManager::getInstance()->abortEvent( ePlaylistEntry::errorNoSpaceLeft );
				else
					eDebug("no state Recording!");
			}
			else
				eWarning("noSpaceLeft message.. but not in TimerMode");
			return;
#endif
		default:
			if ((!c) && pMsg) // noch eine gueltige message vorhanden
				return;
			if ((!isVisible()) && currentFocus)
			{
				pauseMessages();
				message_notifier.send(c);
				return;
			}
			pauseMessages();
			while (!messages.empty())
			{
				nextMessage();
				if (pMsg)
					break;
			}
			startMessages();
	}
}

void eZapMain::nextMessage()
{
	eZapMessage msg;
	messagelock.lock();

	messagetimeout.stop();

	if (pMsg)
	{
#if 0
		if (pMsg->in_loop)
			pMsg->close();
#endif
		pMsg->hide();
		delete pMsg;
		pMsg=0;
		messages.pop_front();
	}

	std::list<eZapMessage>::iterator i(messages.begin());
 	if (i != messages.end())
 	{
		msg=messages.front();
		messagelock.unlock();
		int showonly=msg.getTimeout()>=0;
		if (!showonly)
			hide();
		int flags = 0;
		switch (msg.getIcon())
		{
			case 1:
				flags = eMessageBox::iconInfo;
				break;
			case 2:
				flags = eMessageBox::iconWarning;
				break;
			case 3:
				flags = eMessageBox::iconQuestion;
				break;
			case 4:
				flags = eMessageBox::iconError;
				break;
		}
		if (!showonly)
			flags |= eMessageBox::btOK;

		pMsg = new eMessageBox(msg.getBody(), msg.getCaption(), flags);
		pMsg->show();
		if (!showonly)
		{
			pMsg->exec();
			pMsg->hide();
			delete pMsg;
			pMsg=0;
			messages.pop_front();
		} else if (msg.getTimeout())
			messagetimeout.start(msg.getTimeout()*1000, 1);
	} else
		messagelock.unlock();
}

void eZapMain::stopMessages()
{
	pauseMessages();
	if (pMsg)
	{
		delete pMsg;
		pMsg=0;
		messages.pop_front();
	}
}

void eZapMain::startMessages()
{
	message_notifier.start();
}

void eZapMain::pauseMessages()
{
	message_notifier.stop();
}

void eZapMain::setMode(int newmode, int user)
{
	if ( newmode != -1 )
	{
		eServiceReference tmp = modeLast[newmode].bottom();
		int i=listAll;
		for (; i <=listBouquets; ++i)
		{
			if ( tmp == getRoot(i, newmode).bottom() )
				break;
		}
		if ( i > listBouquets ) // invalid path stored.. restore..
			modeLast[newmode]=getRoot(listAll,newmode);
#ifndef DISABLE_FILE
		// disable skipping
		if(skipping)
			endSkip();

		eServiceReference cur = modeLast[newmode].current();
		int recmode = -1;
		if ( eDVB::getInstance()->recorder && !cur.path )
		{
			eServiceReferenceDVB &ref = (eServiceReferenceDVB&) cur;
			eServiceReferenceDVB &rec = eDVB::getInstance()->recorder->recRef;
			if ( !ref.path && !onSameTP(ref,rec) )
				user=0;
			recmode = rec.data[0] & 1 ? modeTV : modeRadio;
		}

		if ( recmode != -1 && recmode == newmode )
		{
			getServiceSelectorPath(modeLast[mode]);
			mode = newmode;
			setServiceSelectorPath(modeLast[mode]);
			eZap::getInstance()->getServiceSelector()->setKeyDescriptions();
			if ( user && eServiceInterface::getInstance()->service != eDVB::getInstance()->recorder->recRef )
				playService(eDVB::getInstance()->recorder->recRef, psDontAdd|psSeekPos);
			return;
		}
	}
#else
	}
	if ( handleState() )
#endif
	{
#ifndef DISABLE_FILE
		if ( newmode == modeFile )
		{
			playlist->service_name=_("Playlist");
			// Re-read recordings.epl, could be changed on other box
			recordings->lockPlaylist();
			loadRecordings();
			recordings->unlockPlaylist();
		}
		else
			playlist->service_name=_("History");

		if ( eSystemInfo::getInstance()->getHwType() < 3 )  // dbox2
		{
			if ( newmode == modeFile && mode != newmode )
				eEPGCache::getInstance()->messages.send(eEPGCache::Message(eEPGCache::Message::pause));
			else if ( mode == modeFile && mode != newmode && newmode != -1 )
				eEPGCache::getInstance()->messages.send(eEPGCache::Message(eEPGCache::Message::restart));
		}
#endif
//		eDebug("setting mode to %d", newmode);

//		// save oldMode
//		if (mode != -1)
//			getServiceSelectorPath(modeLast[mode]);

		if (mode == newmode)
			user=0;

		if ( newmode != -1 )
			mode=newmode;

		if (user)
		{
//			eDebug("playservice");
#ifndef DISABLE_FILE
			int play=1;
			if ( newmode == modeFile )
			{
				eConfig::getInstance()->getKey("/ezap/extra/autoplay", play);
			}
			if (play)
#endif
				playService(modeLast[mode].current(), psDontAdd|psSeekPos);
		}

		if (mode != -1)
		{
//			eDebug("setServiceSelectorPath");
			setServiceSelectorPath(modeLast[mode]);
		}
		eZap::getInstance()->getServiceSelector()->setKeyDescriptions();
	}
}

void eZapMain::setServiceSelectorPath(eServicePath path)
{
	eServiceSelector *sel = eZap::getInstance()->getServiceSelector();
	eServiceReference ref=path.current();
	path.up();
//	eServicePath p = path;
//	eDebug("Setting currentService to %s", ref.toString().c_str() );
//	eDebug("setting path to %s", p.toString().c_str());
	eServicePath current = sel->getPath();
	if ( path != current )
		sel->setPath(path, ref);
	else if ( sel->getSelected() != ref )
		sel->selectService(ref);
}

void eZapMain::getServiceSelectorPath(eServicePath &path)
{
//	eDebug("selected = %s",eZap::getInstance()->getServiceSelector()->getSelected().toString().c_str() );
	path=eZap::getInstance()->getServiceSelector()->getPath();
	path.down(eZap::getInstance()->getServiceSelector()->getSelected());
//	eDebug("stored path for mode %d: %s", mode, eServicePath(path).toString().c_str());
}

int eZapMain::getFirstBouquetServiceNum( eServiceReference ref, int _mode )
{
	int Mode;
	if (_mode == -1)
		Mode = mode;
	else
		Mode = _mode;

	ePlaylist *p=0;
	switch(Mode)
	{
		case modeTV:
			if ( userTVBouquets->getList().size() )
				p=userTVBouquets;
			break;
		case modeRadio:
			if ( userRadioBouquets->getList().size() )
				p=userRadioBouquets;
			break;
#ifndef DISABLE_FILE
		case modeFile:
			if ( userFileBouquets->getList().size() )
			{
				p=userFileBouquets;
				p->getList().push_back(recordingsref);
			}
			break;
#endif
	}
	int num=1;
	if ( p )
	{
		for ( std::list<ePlaylistEntry>::const_iterator it ( p->getConstList().begin() );
			it != p->getConstList().end(); ++it)
		{
			if ( *it == ref )
			{
#ifndef DISABLE_FILE
				if ( Mode == modeFile )
					p->getList().remove(recordingsref);
#endif
				return num;
			}
			ePlaylist *pl = (ePlaylist*)eServiceInterface::getInstance()->addRef( it->service );
			if ( pl )
			{
				for (std::list<ePlaylistEntry>::const_iterator i(pl->getConstList().begin());
					i != pl->getConstList().end(); ++i )
				{
					if ( i->service.flags & eServiceReference::isMarker )
						continue;
					++num;
				}
				eServiceInterface::getInstance()->removeRef( it->service );
			}
		}
#ifndef DISABLE_FILE
		if ( Mode == modeFile )
			p->getList().remove(recordingsref);
#endif
	}
	return 1;
}

eServicePath eZapMain::getRoot(int list, int _mode)
{
	int Mode;
	if ( _mode != -1 )
		Mode = _mode;
	else
		Mode = mode;
	eServicePath b;
	switch (Mode)
	{
	case modeTV:
		switch (list)
		{
		case listAll:
			b.down(eServiceReference(eServiceReference::idDVB,
				eServiceReference::flagDirectory|eServiceReference::shouldSort,
				-2, (1<<4)|(1<<1), 0xFFFFFFFF ));
			break;
		case listSatellites:
			b.down(eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, (1<<4)|(1<<1) ));
			break;
		case listProvider:
			b.down(eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, (1<<4)|(1<<1), 0xFFFFFFFF ));
			break;
		case listBouquets:
			b.down(userTVBouquetsRef);
			break;
		}
		break;
	case modeRadio:
		switch (list)
		{
		case listAll:
			b.down(eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, 1<<2, 0xFFFFFFFF ));
			break;
		case listSatellites:
			b.down(eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, 1<<2 ));
			break;
		case listProvider:
			b.down(eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, 1<<2, 0xFFFFFFFF ));
			break;
		case listBouquets:
			b.down(userRadioBouquetsRef);
			break;
		}
		break;
#ifndef DISABLE_FILE
	case modeFile:
		switch (list)
		{
		case listAll:
			b.down(eServiceStructureHandler::getRoot(eServiceStructureHandler::modeFile));
			break;
		case listSatellites:
			b.down(recordingsref);
			break;
		case listProvider:
			b.down(playlistref);
			break;
		case listBouquets:
			b.down(userFileBouquetsRef);
			break;
		}
		break;
#endif
	default:
		return eServicePath();
	}
	return b;
}

#ifndef DISABLE_FILE
void eZapMain::showHDDSpaceLeft()
{
	static int swp = 0;
	static int cnt = 0;

	int fds = freeRecordSpace();
	if (fds != -1)
	{
		if (!(cnt++ % 7))
			swp ^= 1;
		if (swp)
		{
			if (fds < 1024)
				DVRSpaceLeft->setText(eString().sprintf("%dMB\nfree", fds));
			else
				DVRSpaceLeft->setText(eString().sprintf("%d.%02d GB\nfree", fds/1024, (int)((fds%1024)/10.34) ));
		}
		else
		{
			int min;
			if (Decoder::current.vpid==-1) //Radiorecording
				min = fds/2; //One minute mp2-audio equals roughly 2MB
			else
				min = fds/33; //One minute mpeg-video equals roughly 33MB

			if (min<60)
				DVRSpaceLeft->setText(eString().sprintf("~%d min\nfree", min ));
			else if (min>5999)
				DVRSpaceLeft->setText(eString().sprintf("~%d h\nfree", min/60 ));//Radiorecording
			else
				DVRSpaceLeft->setText(eString().sprintf("~%dh%02dmin\nfree", min/60, min%60 ));
		}
		DVRSpaceLeft->show();
	}
}
#endif

void eZapMain::showServiceInfobar(int show)
{
	dvrfunctions=show;
	clearHelpList();

	if (dvrfunctions)
	{
		dvrInfoBar->show();
#ifndef DISABLE_FILE
// 	i don't like always running HDD !!
		showHDDSpaceLeft();
#endif
		prepareDVRHelp();
	} else
	{
		dvrInfoBar->hide();
		prepareNonDVRHelp();
	}

	int ID=eServiceReference::idDVB;
	if ( eServiceInterface::getInstance()->getService() )
		ID=eServiceInterface::getInstance()->getService()->getID();
	switch(ID)
	{
		case eServiceReference::idUser: // MP3
			dvbInfoBar->hide();
			fileInfoBar->show();
			break;
		case eServiceReference::idDVB: // DVB or ts playback
			if ( eServiceInterface::getInstance()->service.path )
			{
				dvbInfoBar->hide();
				fileInfoBar->show();
				//fileInfoBar->hide();
				//dvbInfoBar->show();
			}
			else
			{
				fileInfoBar->hide();
				dvbInfoBar->show();
			}
			break;
	}
}

void eZapMain::moveService(const eServiceReference &path, const eServiceReference &ref, const eServiceReference &afterref)
{
	std::list<ePlaylistEntry>::iterator it=std::find(currentSelectedUserBouquet->getList().begin(), currentSelectedUserBouquet->getList().end(), ref),
																			after;

	if (afterref)
		after=std::find(currentSelectedUserBouquet->getList().begin(), currentSelectedUserBouquet->getList().end(), afterref);
	else
		after=currentSelectedUserBouquet->getList().end();

	currentSelectedUserBouquet->moveService(it, after);
}

#ifndef DISABLE_FILE
void eZapMain::toggleIndexmark()
{
	if (!indices_enabled)
		return;

	if (!(serviceFlags & eServiceHandler::flagIsSeekable))
		return;

	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler)
		return;

	int real=handler->getPosition(eServiceHandler::posQueryRealCurrent),
			time=handler->getPosition(eServiceHandler::posQueryCurrent);

	int nearest=indices.getNext(real, 0);
	if ((nearest == -1) || abs(indices.getTime(nearest)-time) > 5)
		indices.add(real, time);
	else
		indices.remove(nearest);

	Progress->invalidate();
}

bool eZapMain::indexSeek(int dir)
{
	// dir should be 1 or -1. If not unexpected results may occur
	static bool lockSeek = false;

	if (!indices_enabled)
		return false;

	if (!(serviceFlags & eServiceHandler::flagIsSeekable))
		return false;

	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	if (!handler || lockSeek)
		return false;

	lockSeek = true;

	int real = handler->getPosition(eServiceHandler::posQueryRealCurrent);
	int time = handler->getPosition(eServiceHandler::posQueryCurrent);

	int nearest = indices.getNext(real, dir);

	if (nearest != -1)
	{
		int nearestt = indices.getTime(nearest);
		if (abs(time - nearestt) < 5) // ... less than 5 seconds, then seek to prev/next
			nearest = indices.getNext(nearest, dir);
	}
	if (nearest == -1)
	{
		if (dir == -1)
			nearest = 0; // seek to start of file
		else
		{
			lockSeek = false;
			return false; // go play next service
		}
	}
	handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekReal, nearest));

	lockSeek=false;

	if ( time < 5 && dir == -1 ) // play previous service if we are within the first 5 seconds
		return false;
	return true;
}

#endif // DISABLE_FILE

void eZapMain::toggleScart( int state )
{
	enigmaVCR *instance = enigmaVCR::getInstance();
	if ( state )
	{
		if ( !instance )
		{
			enigmaVCR mb("If you can read this, your scartswitch doesn't work", "VCR");
			mb.show();
#ifndef DISABLE_LCD
			eWidget *oldlcd=0;
			if ( lcdmain.lcdMenu->isVisible() )
				oldlcd = lcdmain.lcdMenu;
			else if ( lcdmain.lcdMain->isVisible() )
				oldlcd = lcdmain.lcdMain;
			else if ( lcdmain.lcdStandby->isVisible() )
				oldlcd = lcdmain.lcdStandby;
			if ( oldlcd )
				oldlcd->hide();
			lcdmain.lcdScart->show();
#endif
			mb.exec();
#ifndef DISABLE_LCD
			lcdmain.lcdScart->hide();
			if ( oldlcd )
				oldlcd->show();
#endif
			mb.hide();
		}
	}
	else if ( instance )
		instance->switchBack();
}

void eZapMain::ShowTimeCorrectionWindow( tsref ref )
{
	eTimeCorrectionEditWindow wnd( ref );
	wnd.show();
	wnd.exec();
	wnd.hide();
}

#ifndef DISABLE_NETWORK
void eZapMain::startNGrabRecord()
{
	if(state & (stateRecording|recDVR))return; //is any recording
	stopPermanentTimeshift();
	state |= (stateRecording|recDVR);
	ENgrab::getNew()->sendstart();
#ifndef DISABLE_FILE
	recStatusBlink.start(500, 1);
#endif
}

void eZapMain::stopNGrabRecord()
{
	if (!ENgrab::nGrabActive)return; // nGrab not recording
#ifndef DISABLE_FILE
	if ( !eDVB::getInstance()->recorder )
#endif
	state &= ~(stateRecording|recDVR);
	ENgrab::getNew()->sendstop();
#ifndef DISABLE_FILE
	recStatusBlink.stop();
	recstatus->hide();
	recchannel->hide();
	beginPermanentTimeshift();
#ifndef DISABLE_LCD
	if(state & stateSleeping)
	{
		lcdmain.lcdMain->hide();
		lcdmain.lcdStandby->show();
	}
	lcdmain.lcdMain->Clock->show();
#endif
#endif
}
#endif

void eZapMain::EPGAvail(bool avail)
{
	epg_messages.send( eEPGCache::Message(eEPGCache::Message::isavail,avail));
}

void eZapMain::EPGUpdated()
{
	epg_messages.send( eEPGCache::Message(eEPGCache::Message::updated));
}

void eZapMain::EPGOrganiseRequest()
{
	// Organise EPG when sleeping and not recording
	if ((state & stateSleeping) && !(state & stateRecording))
		eEPGCache::getInstance()->messages.send( eEPGCache::Message( eEPGCache::Message::organise ));
}

void eZapMain::gotEPGMessage( const eEPGCache::Message &msg )
{
	switch(msg.type)
	{
		case eEPGCache::Message::isavail:
			setEPGButton(msg.avail);
			break;
		case eEPGCache::Message::updated:
			eZap::getInstance()->getServiceSelector()->EPGUpdated();
			break;
	}
}

void eZapMain::gotoStandby()
{
	message_notifier.start();
	message_notifier.send( messageGoSleep );
}

void eZapMain::wakeUp()
{
	message_notifier.start();
	message_notifier.send( messageWakeUp );
}

void eZapMain::gotRDSText(eString text)
{
//	eDebug("gotRDSText(%s)", text.c_str() );
//	dvbInfoBar->hide();
//	fileInfoBar->show();
//	fileinfos->setText(convertLatin1UTF8(text));
#ifndef DISABLE_LCD
	lcdmain.lcdMain->gotRDSText(text);
#endif
}
void eZapMain::getAllBouquetServices(std::list<ePlaylistEntry> &servicelist)
{
	servicelist.clear();
	for ( std::list<ePlaylistEntry>::const_iterator it(userTVBouquets->getConstList().begin()); it != userTVBouquets->getConstList().end(); ++it)
	{
		ePlaylist *p = (ePlaylist*) eServiceInterface::getInstance()->addRef( it->service );
		if ( p )
		{
			for ( std::list<ePlaylistEntry>::const_iterator itp ( p->getConstList().begin() );
				itp != p->getConstList().end(); ++itp)
				servicelist.push_back(*itp);
			eServiceInterface::getInstance()->removeRef( it->service );
		}
	}
	for (std::list<ePlaylistEntry>::const_iterator it(userRadioBouquets->getConstList().begin()); it != userRadioBouquets->getConstList().end(); it++ )
	{
		ePlaylist *p = (ePlaylist*) eServiceInterface::getInstance()->addRef( it->service );
		if ( p )
		{
			for ( std::list<ePlaylistEntry>::const_iterator itp ( p->getConstList().begin() );
				itp != p->getConstList().end(); ++itp)
				servicelist.push_back(*itp);
			eServiceInterface::getInstance()->removeRef( it->service );
		}
	}
//eDebug("getAllBouquetServices:%d",servicelist.size());
}

void eZapMain::reloadSettings()
{
	// Reloads all settings for example after a new settings list or a restore of a backup
	eDebug("eZapMain::reloadSettings start");
	eMessageBox::ShowBox(
		_("Networks, services and user bouquets will be reloaded"), 
		_("Reloading..."), eMessageBox::btOK | eMessageBox::iconInfo);

	eMessageBox msg2(
		_("Please wait..."),
		_("Reloading..."), eMessageBox::iconInfo);
	msg2.show();

	eTransponderList::getInstance()->invalidateNetworks();
	eTransponderList::getInstance()->reloadNetworks();
	eTransponderList::getInstance()->writeLNBData();
	eDVB::getInstance()->settings->loadServices();
	eDVB::getInstance()->settings->loadBouquets();
	eZapMain::getInstance()->loadUserBouquets();
	eZap::getInstance()->getServiceSelector()->actualize();
	eServiceReference::loadLockedList((eZapMain::getInstance()->getEplPath()+"/services.locked").c_str());

	msg2.hide();

	eDebug("eZapMain::reloadSettings done");
}

eServiceContextMenu::eServiceContextMenu(const eServiceReference &ref, const eServiceReference &path, eWidget *lcdTitle, eWidget *lcdElement)
: eListBoxWindow<eListBoxEntryText>(_("Service Menu"), 13, 400, true), ref(ref)
{
	init_eServiceContextMenu(ref, path, lcdTitle, lcdElement);
}

void eServiceContextMenu::init_eServiceContextMenu(const eServiceReference &ref, const eServiceReference &path, eWidget *lcdTitle, eWidget *lcdElement)
{
#ifndef DISABLE_LCD
	setLCD(lcdTitle,lcdElement);
#endif
	valign();

	eListBoxEntry *prev=0;

	if ( (!(ref.flags & eServiceReference::isDirectory))
		&& (ref != eServiceReference()) && ( ref.type == 0x1000 // mp3
		|| ( ref.type == eServiceReference::idDVB && ref.path ) ) )
		
		prev = new eListBoxEntryText(&list, _("add service to playlist"), (void*)3, 0, _("add selected service to the playlist"));
	//shuffle
	if ( eZapMain::getInstance()->getMode() == eZapMain::modeFile &&
			( ( ref.type == eServiceReference::idUser
					&& ( (ref.data[0] == eMP3Decoder::codecMPG) || (ref.data[0] == eMP3Decoder::codecMP3)
								|| (ref.data[0] == eMP3Decoder::codecFLAC)
								|| (ref.data[0] == eMP3Decoder::codecOGG)
							)
				)
				|| (ref.type == 0x2000) // picture
			))
	{
		prev = new eListBoxEntryText(&list, _("shuffle"), (void*)21, 0, _("shuffle playlist"));
	}

	eListBoxEntryText *sel=0;
	/* i think it is not so good to rename normal providers
	if ( ref.data[0]==-3 ) // rename Provider
		new eListBoxEntryText(&list, _("rename"), (void*)7);*/
	
	// remove newflag from satellite
	if ( ref && (ref.flags & eServiceReference::isDirectory)
		&& (ref.type == eServiceReference::idDVB)
		&& (ref.data[0] == -5 ) )
	{
		prev = new eListBoxEntryText(&list, _("remove all 'new found' flags"), (void*)20, 0, _("select to remove all 'new found' flags from this satellite"));
	}
	// remove newflag from dvb service
	eServiceDVB *service = eTransponderList::getInstance()->searchService(ref);
	if ( service && service->dxflags & eServiceDVB::dxNewFound )
	{
		prev = new eListBoxEntryText(&list, _("remove 'new found flag'"), (void*)16, 0, _("select to remove the 'new found flag'"));
		prev = new eListBoxEntryText(&list, _("remove all 'new found' flags"), (void*)22, 0, _("select to remove all 'new found' flags"));
	}

	// create new bouquet
	prev = new eListBoxEntryText(&list, _("create new bouquet"), (void*)6, 0, _("select to create a new bouquet"));

	if (path.type == eServicePlaylistHandler::ID)
	{
		if (ref)  // valid entry? ( GO UP is not valid )
		{
			// copy complete provider to bouquets
			if ( ref.flags & eServiceReference::flagDirectory )
			{
				// not in file mode
				if ( eZapMain::getInstance()->getMode() != eZapMain::modeFile )
				{
					prev = new eListBoxEntryText(&list, _("duplicate bouquet"), (void*)8, 0, _("duplicate the complete bouquet with all content"));
					prev = new eListBoxEntryTextSeparator( &list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
				}
			}
			else // add dvb service to specific bouquet
			{
				prev = new eListBoxEntryText(&list, _("add to specific bouquet"), (void*)4, 0, _("add the selected service to another bouquet"));
				if ( path.type == eServicePlaylistHandler::ID )
					prev = new eListBoxEntryText(&list, _("add marker"), (void*)13, 0, _("create a new marker in the current bouquet"));
				prev = new eListBoxEntryTextSeparator( &list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
			}

			// rename bouquet
			if ( ref.type == eServicePlaylistHandler::ID )
			{
				if ( prev && prev->isSelectable() )
					prev = new eListBoxEntryTextSeparator( &list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
				prev = new eListBoxEntryText(&list, _("rename"), (void*)7, 0, _("rename the current selected bouquet"));
			}

			// rename dvb service
			if ( (( ref.type == eServiceReference::idDVB )
#ifndef DISABLE_FILE
				|| 	( ref.type == eServiceReference::idUser &&
							( (ref.data[0] ==  eMP3Decoder::codecMPG) ||
								(ref.data[0] ==  eMP3Decoder::codecMP3) ||
								(ref.data[0] ==  eMP3Decoder::codecFLAC) ||
								(ref.data[0] ==  eMP3Decoder::codecOGG) ) )
#endif
					) )
			{
				if ( prev && prev->isSelectable() )
					prev = new eListBoxEntryTextSeparator( &list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
				prev = new eListBoxEntryText(&list, _("rename"), (void*)9, 0, _("rename the current selected service/movie"));
			}

			// all what contain in a playlists is deleteable
			prev = new eListBoxEntryText(&list, _("delete"), (void*)1, 0, _("delete the current selected service/movie"));

#ifndef DISABLE_FILE
			if ( eZapMain::getInstance()->getMode() == eZapMain::modeFile )
			{
				prev = new eListBoxEntryText(&list, _("rebuild movie list"), (void*)17, 0, _("rebuild the current movie list"));
			}
#endif
			prev = new eListBoxEntryTextSeparator(&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
			
			if ( ref.type == eServiceReference::idDVB )
			{
				prev = new eListBoxEntryText(&list, _("set as startup service"), (void*)18, 0, _("currently selected service will be selected on startup"));
				prev = new eListBoxEntryText(&list, _("reset startup service"), (void*)19, 0, _("last selected service will be selected on startup"));
				prev = new eListBoxEntryTextSeparator(&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
			}
		}

		// move mode in playlists
		if ( eZap::getInstance()->getServiceSelector()->movemode )
			prev = sel = new eListBoxEntryText(&list, _("disable move mode"), (void*)2, 0, _("switch move mode off"));
		else
			prev = new eListBoxEntryText(&list, _("enable move mode"), (void*)2, 0, _("activate mode to simply change the entry order"));
	}
	else if (ref) // not in a playlist
	{
		bool b=true;
		if ( (ref.flags & eServiceReference::flagDirectory)
			&& (ref.type == eServiceReference::idDVB)
			&& (ref.data[0] == -2 || ref.data[0] == -3 ) )
		{
			prev = new eListBoxEntryText(&list, _("copy to bouquet list"), (void*)8, 0, _("copy the selected provider to the bouquet list"));
			b=false;
		}
		else if ( ref.flags & eServiceReference::flagDirectory )
		{
			if ( ref.data[0] != -1 )
				prev = new eListBoxEntryText(&list, _("add to specific bouquet"), (void*)4, 0, _("add the selected service to a selectable bouquet"));
			b=false;
		}
		else if ( ref.type == eServiceReference::idDVB && !ref.path )
		{
			prev = new eListBoxEntryText(&list, _("add to specific bouquet"), (void*)4, 0, _("add the selected service to a selectable bouquet"));
			prev = new eListBoxEntryTextSeparator( &list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
			prev = new eListBoxEntryText(&list, _("rename"), (void*)9, 0, _("rename the current selected service/movie"));
			prev = new eListBoxEntryText(&list, _("delete"), (void*)1, 0, _("delete the current selected service/movie"));
			prev = new eListBoxEntryTextSeparator(&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
			prev = new eListBoxEntryText(&list, _("set as startup"), (void*)18, 0, _("set the current selected service/movie as startup service"));
			prev = new eListBoxEntryText(&list, _("reset startup"), (void*)19, 0, _("reset startup service"));
			b=false;
		}
#ifndef DISABLE_FILE
		if ( b && (ref.type == eServiceReference::idDVB && ref.path)
			|| ( ref.type == eServiceReference::idUser
				&& ( (ref.data[0] == eMP3Decoder::codecMPG) || (ref.data[0] == eMP3Decoder::codecMP3) 
					|| (ref.data[0] == eMP3Decoder::codecFLAC) 
					|| (ref.data[0] == eMP3Decoder::codecOGG) )
			   )
			|| (ref.type == 0x2000) // picture
		   )
		{// deleteable file
			if ( ref.type != 0x2000 )
				prev = new eListBoxEntryText(&list, _("add to specific bouquet"), (void*)4, 0, _("add the selected file to a selectable bouquet"));
			prev = new eListBoxEntryTextSeparator( &list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
			prev = new eListBoxEntryText(&list, _("delete file"), (void*)14, 0, _("delete the selected file (and all corresponding ts files"));
			prev = new eListBoxEntryText(&list, _("rename file"), (void*)15, 0, _("rename the selected file (and all corresponding ts files"));
			if ( ref.type != 0x2000 )
				prev = new eListBoxEntryTextSeparator( &list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
		}
#endif
	}

	// not in File mode
	if ( eZapMain::getInstance()->getMode() != eZapMain::modeFile )
	{
/*		if ( prev && prev->isSelectable() )
			new eListBoxEntryTextSeparator( (eListBox<eListBoxEntry>*)&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );*/
		// edit Mode ( simple add services to any bouquet(playlist)
		if ( eZap::getInstance()->getServiceSelector()->editMode )
			prev = sel = new eListBoxEntryText(&list, _("disable edit mode"), (void*)5, 0, _("disable the edit mode"));
		else
			prev = new eListBoxEntryText(&list, _("enable edit mode"), (void*)5, 0, _("activate mode to simply add many services to a selectable bouquet"));
	}

	// options for activated parental locking
	if (pinCheck::getInstance()->getParentalEnabled())  
	{
		if (ref)
		{
			if ( prev && prev->isSelectable() )
				new eListBoxEntryTextSeparator( &list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
			if ( ref.isLocked() )
				new eListBoxEntryText(&list, _("unlock"), (void*)11, 0, _("do parental-unlock the selected service or file"));
			else
				new eListBoxEntryText(&list, _("lock"), (void*)10, 0, _("do parentallock the selected service or file"));
		}

		if ( pinCheck::getInstance()->pLockActive() )
			new eListBoxEntryText(&list, _("disable parental lock"), (void*)12, 0, _("temporary disable the parental locking") );
		else
			new eListBoxEntryText(&list, _("enable parental lock"), (void*)12, 0, _("re-enable the parental locking"));
	}

	if ( sel )
		list.setCurrent( sel );

	list.setFlags( eListBoxBase::flagHasShortcuts );
	CONNECT(list.selected, eServiceContextMenu::entrySelected);
}

void eServiceContextMenu::entrySelected(eListBoxEntryText *test)
{
	if (!test)
		close(0);
	else
		close((int)test->getKey());
}

eSleepTimerContextMenu::eSleepTimerContextMenu( eWidget* lcdTitle, eWidget *lcdElement )
	: eListBoxWindow<eListBoxEntryText>(_("Shutdown/Standby Menu"), 6, 400, true)
{
#ifndef DISABLE_LCD
	setLCD(lcdTitle, lcdElement);
#endif
	valign();
	switch( eSystemInfo::getInstance()->getHwType() )
	{
		case eSystemInfo::DM500:
		case eSystemInfo::DM5600:
		case eSystemInfo::DM5620:
			new eListBoxEntryText(&list, _("restart enigma"), (void*)5, 0, _("only restart the software, without rebooting the receiver"));
			new eListBoxEntryText(&list, _("reboot now"), (void*)4, 0, _("restart your dreambox"));
			break;
		case eSystemInfo::TR_DVB272S:
			new eListBoxEntryText(&list, _("restart enigma"), (void*)5, 0, _("only restart the software, without rebooting the receiver"));
			new eListBoxEntryText(&list, _("reboot now"), (void*)4, 0, _("restart your receiver"));
			break;
		case eSystemInfo::DM500PLUS:
		case eSystemInfo::DM600PVR:
		case eSystemInfo::DM7000:
		case eSystemInfo::DM7020:
			new eListBoxEntryText(&list, _("restart enigma"), (void*)5, 0, _("only restart the software, without rebooting the receiver"));
			new eListBoxEntryText(&list, _("shutdown now"), (void*)1, 0, _("shutdown your dreambox"));
			new eListBoxEntryText(&list, _("restart"), (void*)4, 0, _("restart your dreambox"));
			break;
		case eSystemInfo::dbox2Nokia ... eSystemInfo::dbox2Philips:
			new eListBoxEntryText(&list, _("shutdown now"), (void*)1, 0, _("shutdown your dbox-2"));
			new eListBoxEntryText(&list, _("restart"), (void*)4, 0, _("restart your dbox-2"));
			break;
	}
	new eListBoxEntryTextSeparator( &list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
	new eListBoxEntryText(&list, _("goto standby"), (void*)2, 0, _("goto standby mode") );
	new eListBoxEntryText(&list, _("set sleeptimer"), (void*)3, 0, _("set a sleep timer"));
	CONNECT(list.selected, eSleepTimerContextMenu::entrySelected);

	/* help text for shutdown menu */
	setHelpText(_("\tShutdown/Standby Menu\n\n>>> [MENU] >>> [5] Shutdown\n. . . . . . . . . .\n\n" \
								"This menu offers you ways to shutdown your receiver.\n. . . . . . . . . .\n\n" \
								"Usage:\n\nrestart enigma\tRestarts your Dreambox (warm restart)\n\n" \
								"Shutdown now\tShutdown your Dreambox\n\nRestart\tRestart you Dreambox (cold restart)\n\n" \
								"Goto standby\tSwitch to StandBy mode\n\nSet Sleeptimer\tSleep timer setup\n\n[EXIT]\tClose window"));
}

void eSleepTimerContextMenu::entrySelected( eListBoxEntryText *sel )
{
	if (!sel)
		close(0);
	else
		close((int)sel->getKey());
}

eShutdownStandbySelWindow::eShutdownStandbySelWindow(eWidget *parent, int len, int min, int max, int maxdigits, int *init, int isactive, eWidget* descr, int grabfocus, const char* deco )
{
	num = new eNumber( parent, len, min, max, maxdigits, init, isactive, descr, grabfocus, deco );
	Shutdown = new eCheckbox(this);
	Shutdown->setName("shutdown");
	Standby = new eCheckbox(this);
	Standby->setName("standby");
	set = new eButton(this);
	set->setName("set");
	CONNECT( num->selected, eShutdownStandbySelWindow::fieldSelected );
	CONNECT( Shutdown->checked, eShutdownStandbySelWindow::ShutdownChanged );
	CONNECT( Standby->checked, eShutdownStandbySelWindow::StandbyChanged );
}

void eShutdownStandbySelWindow::StandbyChanged( int checked )
{
	if ( checked )
		Shutdown->setCheck( 0 );
}

void eShutdownStandbySelWindow::ShutdownChanged( int checked )
{
	if ( checked )
		Standby->setCheck( 0 );
}

int eShutdownStandbySelWindow::getCheckboxState()
{
	return Standby->isChecked()?3:Shutdown->isChecked()?2:0;
}

eSleepTimer::eSleepTimer()
:eShutdownStandbySelWindow( this, 1, 1, 999, 3, 0, 0 )
{
	eLabel *l = new eLabel(this);
	l->setName("l_duration");
	num->setDescr(l);
	num->setName("duration");
	num->setNumber(30);
	if (eSkin::getActive()->build(this, "sleeptimer"))
		eFatal("skin load of \"sleeptimer\" failed");
	CONNECT( set->selected, eSleepTimer::setPressed );

	if ( !eSystemInfo::getInstance()->canShutdown() )
		Shutdown->hide();

	Standby->setCheck(1);

	/* help text for sleep timer screen */
	setHelpText(_("\tSet Sleeptimer\n\n>>> [MENU] >>> [5] Shutdown >>> Set sleeptimer\n. . . . . . . . . .\n\n" \
		"Sleeptimer allows you to set a countdown time for when the Dreambox should shutdown automatically.\n" \
		". . . . . . . . . .\n\nUsage:\n\n[LEFT]/[RIGHT]\tPrevious/Next Menu item\n[UP]/[DOWN]\n\n" \
		"Duration:\tCountdown time\n[NUMBERS]\n\nShutdown [OK]\tToggle Shutdown mode\n\n" \
		"Standby [OK]\tToggle standby mode\n\n[GREEN]\tSave changes and close window\n\n" \
		"[EXIT]\tClose window without saving changes"));
}

void eSleepTimer::setPressed()
{
	EITEvent *evt = new EITEvent();
	evt->start_time = time(0)+eDVB::getInstance()->time_difference;
	evt->duration = num->getNumber()*60;
	evt->event_id = -1;
	evt->free_CA_mode = -1;
	evt->running_status = -1;
	close((int)evt);
}

#ifndef DISABLE_FILE
eTimerInput::eTimerInput()
:eShutdownStandbySelWindow( this, 1, 1, 999, 3, 0, 0 )
{
	eLabel *l = new eLabel(this);
	l->setName("lrec_duration");
	num->setDescr(l);
	num->setName("rec_duration");

	int min=10;
	EIT *eit=eDVB::getInstance()->getEIT();
	int p=0;
	if (eit)
	{
		for (ePtrList<EITEvent>::iterator e(eit->events); e != eit->events.end(); ++e)
		{
			if ((e->running_status>=2)|| (!p && !e->running_status))		// currently running service
			{
				int timeroffsetstop = 0;
				eConfig::getInstance()->getKey("/enigma/timeroffsetstop", timeroffsetstop);
				time_t stime = e->start_time;
				time_t now = time(0)+eDVB::getInstance()->time_difference;
				min = ((e->duration - (now - stime)) / 60) + timeroffsetstop;
				break;
			}
			p++;
		}
		eit->unlock();
	}

	num->setNumber(min);
	if (eSkin::getActive()->build(this, "recording_duration"))
		eFatal("skin load of \"recording_duration\" failed");
	CONNECT( set->selected, eTimerInput::setPressed );
}

void eTimerInput::setPressed()
{
	EITEvent *evt = new EITEvent();
	evt->start_time = time(0)+eDVB::getInstance()->time_difference;
	evt->duration = num->getNumber()*60;
	evt->event_id = -1;
	evt->free_CA_mode = -1;
	evt->running_status = -1;
	close((int)evt);
}
extern ePermanentTimeshift permanentTimeshift;

eRecordContextMenu::eRecordContextMenu( eWidget *LCDTitle, eWidget *LCDElement )
	: eListBoxWindow<eListBoxEntryText>(_("Record Menu"), 5, 350, true)
{
	init_eRecordContextMenu();
}
void eRecordContextMenu::init_eRecordContextMenu()
{
#ifndef DISABLE_LCD
	setLCD(LCDTitle, LCDElement);
#endif
	valign();
	new eListBoxEntryText(&list, _("stop record now"), (void*)1, 0, _("immediate stop the recording"));
	new eListBoxEntryTextSeparator( &list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
	new eListBoxEntryText(&list, _("set record duration"), (void*)2, 0, _("set the recording time (in minutes)"));
	new eListBoxEntryText(&list, _("set record stop time"), (void*)3, 0, _("set the recording end time") );
	int permanentOn = 0;
	eConfig::getInstance()->getKey("/enigma/timeshift/permanent", permanentOn );
	if (permanentOn && permanentTimeshift.getRecordedMinutes() > 0)
	{
		eString str;
		str.sprintf(_("add timeshift to recording (%d min) "),permanentTimeshift.getRecordedMinutes());
		new eListBoxEntryText(&list, str.c_str(), (void*)4, 0, _("add recorded timeshift at beginning of this recording") );

	}
	CONNECT(list.selected, eRecordContextMenu::entrySelected);
}

void eRecordContextMenu::entrySelected( eListBoxEntryText *sel )
{
	if (!sel)
		close(0);
	else
		close((int)sel->getKey());
}

eRecTimeInput::eRecTimeInput()
:eShutdownStandbySelWindow( this, 2, 0, 59, 2, 0, 0 )
{
	init_eRecTimeInput();
}
void eRecTimeInput::init_eRecTimeInput()
{
	eLabel *l = new eLabel(this);
	l->setName("lrec_end_time");
	num->setDescr(l);
	num->setName("rec_end_time");
	num->setFlags( eNumber::flagFillWithZeros|eNumber::flagTime );
	ampm = new eCheckbox(this);
	ampm->setName("ampm");

	time_t tmp=0;

	int p=0;
	EIT *eit=eDVB::getInstance()->getEIT();
	if (eit)
	{
		for (ePtrList<EITEvent>::iterator e(eit->events); e != eit->events.end(); ++e)
		{
			if ((e->running_status>=2)|| (!p && !e->running_status))		// currently running service
			{
				int timeroffsetstop = 0;
				eConfig::getInstance()->getKey("/enigma/timeroffsetstop", timeroffsetstop);
				tmp = e->start_time + e->duration + (timeroffsetstop * 60);
				break;
			}
			p++;
		}
		eit->unlock();
	}
	else
		tmp = time(0)+eDVB::getInstance()->time_difference;

	int clktype = 0;
	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);

	struct tm *t = localtime( &tmp );
	int hour = t->tm_hour;
	if (clktype) {
		if (hour < 12)
			ampm->setCheck(1);
		hour %= 12;
		if (hour = 0) hour = 12;
	}
	else
		ampm->hide();

	num->setNumber(0, hour);
	num->setNumber(1, t->tm_min);

	if (eSkin::getActive()->build(this, "recording_end_time"))
		eFatal("skin load of \"recording_end_time\" failed");

	CONNECT( set->selected, eRecTimeInput::setPressed );
}

extern time_t normalize( struct tm & t );

void eRecTimeInput::setPressed()
{
	int hour = num->getNumber(0);
	int min = num->getNumber(1);

	int clktype = 0;
	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);
	
	if (clktype)
	{ // 12-hour clock
		if (hour == 12)
			hour = 0;
		if (!ampm->isChecked())
			hour += 12;
	}
	// eDebug("Recording end time: %d:%d", hour, min);

	time_t now = time(0)+eDVB::getInstance()->time_difference;
	tm t = *localtime( &now );
	t.tm_isdst=-1;
	if ( hour*60+min < t.tm_hour*60+t.tm_min )
	{
		t.tm_mday++;
		t.tm_hour = hour;
		t.tm_min = min;
		normalize(t);
	}
	else
	{
		t.tm_hour = hour;
		t.tm_min = min;
	}

	time_t tmp = mktime(&t);

	EITEvent *evt = new EITEvent();
	evt->start_time = time(0)+eDVB::getInstance()->time_difference;
	evt->duration = tmp - evt->start_time;
	evt->event_id = -1;
	evt->free_CA_mode = -1;
	evt->running_status = -1;
	close((int)evt);
}

#endif //DISABLE_FILE

TextEditWindow::TextEditWindow( const char *InputFieldDescr, const char* useableChars )
	:eWindow(0)
{
	init_TextEditWindow(InputFieldDescr, useableChars );
}
void TextEditWindow::init_TextEditWindow( const char *InputFieldDescr, const char* useableChars )
{
	eTextInputFieldHelpWidget *image=new eTextInputFieldHelpWidget(this);
	image->setName("image");

	input = new eTextInputField(this,0,image);
	input->setName("inputfield");
	input->setHelpText(_("press ok to start edit mode"));
	input->setFlags(eTextInputField::flagCloseParent);
	if (useableChars)
		input->setUseableChars( useableChars );
	CONNECT( input->selected, TextEditWindow::accept );

	descr = new eLabel(this);
	descr->setName("descr");
	descr->setText(InputFieldDescr);

	eStatusBar *n = new eStatusBar(this);
	n->setName("statusbar");

	if (eSkin::getActive()->build(this, "TextEditWindow"))
		eWarning("TextEditWindow build failed!");
}

int TextEditWindow::eventHandler( const eWidgetEvent &e )
{
	switch (e.type)
	{
		case eWidgetEvent::execBegin:
			input->setState(1, 0);
			return 1;
		case eWidgetEvent::evtAction:
			if ( e.action != &i_cursorActions->help )
				break;
			else
				return 1;
		default:
			break;
	}
	return eWindow::eventHandler(e);
}

UserBouquetSelector::UserBouquetSelector( std::list<ePlaylistEntry>&list )
	:eListBoxWindow<eListBoxEntryText>(_("Bouquets"), 9, 400),
	SourceList(list)
{
	valign();

	for (std::list<ePlaylistEntry>::iterator it( SourceList.begin() ); it != SourceList.end(); it++)
	{
		ePlaylist *pl = (ePlaylist*)eServiceInterface::getInstance()->addRef( it->service );
		new eListBoxEntryText( &this->list, pl->service_name, &it->service );
		eServiceInterface::getInstance()->removeRef( it->service );
	}
	CONNECT( this->list.selected, UserBouquetSelector::selected );
}

void UserBouquetSelector::selected( eListBoxEntryText *sel )
{
	if (sel && sel->getKey())
		curSel=*((eServiceReference*)sel->getKey());

	close(0);
}

#ifndef DISABLE_FILE

SkipEditWindow::SkipEditWindow( const char *InputFieldDescr)
{
	int fsize=eSkin::getActive()->queryValue("fontsize", 20)+4;

	cresize(eSize(160, fsize+8));
	valign();

	description=new eLabel(this);
	description->setText(InputFieldDescr);
	description->move(ePoint(20, 4));
	description->resize(eSize(80,fsize));

	input = new eTextInputField(this);
	input->move(ePoint(100, 4));
	input->resize(eSize(50,fsize));
	input->setMaxChars(3);
	input->setFlags(eTextInputField::flagCloseParent|eTextInputField::flagGoAlwaysNext);
	input->setUseableChars("0123456789");
	CONNECT( input->selected, TextEditWindow::accept );
}

int SkipEditWindow::eventHandler( const eWidgetEvent &e )
{
	switch (e.type)
	{
		case eWidgetEvent::execBegin:
			input->setState(1, 0);
			return 1;
		case eWidgetEvent::evtAction:
			if ( e.action != &i_cursorActions->help )
				break;
			else
				return 1;
		default:
			break;
	}
	return eWidget::eventHandler(e);
}
/*##################################################*/
#endif //DISABLE_FILE
