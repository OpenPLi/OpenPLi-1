#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <timer.h>
#include <engrab.h>
#include <elirc.h>
#include <enigma_main.h>
#include <parentallock.h>
#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/servicestructure.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/servicedvb.h>
#include <lib/dvb/record.h>
#include <lib/gui/emessage.h>
#include <lib/gdi/font.h>
#include <lib/gui/textinput.h>
#include <lib/gui/enumber.h>
#include <lib/gui/combobox.h>
#include <lib/gui/echeckbox.h>
#include <lib/system/econfig.h>

#include <parentallock.h>

#ifdef WRITE_LOGFILE
#define TIMER_LOGFILE CONFIGDIR "/enigma/timer.log"
static int logfilesize;
#else
#define writeToLogfile(x) 
#endif

static const unsigned char monthdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
/* bug fix - at localization, 
   macro the type _ ("xxxxx") for a constant does not work, 
   if it is declared outside of the function
   
static const char *monthStr[12] = { _("January"), _("February"), _("March"),
													_("April"), _("May"), _("June"),	_("July"),
													_("August"), _("September"), _("October"),
													_("November"), _("December") };
static const char *dayStr[7] = { _("Sunday"), _("Monday"), _("Tuesday"), _("Wednesday"),
											 _("Thursday"), _("Friday"), _("Saturday") };
const char *dayStrShort[7] = { _("Sun"), _("Mon"), _("Tue"), _("Wed"),
											 _("Thu"), _("Fri"), _("Sat") };
*/
eTimerManager* eTimerManager::instance=0;

void normalize( struct tm & );

bool Overlapping( const ePlaylistEntry &e1, const ePlaylistEntry &e2 );

bool onSameTP( const eServiceReferenceDVB& ref1, const eServiceReferenceDVB &ref2 )
{
	return (ref1.getTransportStreamID() == ref2.getTransportStreamID() &&
				ref1.getOriginalNetworkID() == ref2.getOriginalNetworkID() &&
				ref1.getDVBNamespace() == ref2.getDVBNamespace() );
}

bool canPlayService( const eServiceReference & ref )
{
#ifndef DISABLE_FILE
	if ( !eDVB::getInstance()->recorder )
		return true;
	eServiceReferenceDVB &recRef = eDVB::getInstance()->recorder->recRef;
	if ( ref == recRef )
		return true;
	// On a DM500 (C/T/S) , 5600 and 5620 switching is never possible when recording!
	switch (eSystemInfo::getInstance()->getHwType())
	{
		case eSystemInfo::DM500:
		case eSystemInfo::DM5600:
		case eSystemInfo::DM5620:
			return false;

	} 
	if ( ref.type == eServiceReference::idDVB )
	{
		if ( ref.path )
			return eSystemInfo::getInstance()->canTimeshift();
		int canHandleTwoScrambledServices=0;
		eConfig::getInstance()->getKey("/ezap/ci/handleTwoServices", canHandleTwoScrambledServices);
		if ( !canHandleTwoScrambledServices && eDVB::getInstance()->recorder->scrambled )
			return false;
		return onSameTP( (eServiceReferenceDVB&)ref, recRef );
	}
#endif
	return true;
}

bool checkTimeshift( const eServiceReference &ref )
{
	if ( ref.path )
	{
		if ( ref.type == eServiceReference::idDVB )
			return eSystemInfo::getInstance()->canTimeshift();
		return true;
	}
	return false;
}

static time_t getNextEventStartTime( time_t t, int duration, int type, int last_activation=-1 )
{
	if ( type < ePlaylistEntry::isRepeating )
		return 0;

	time_t now = time(0)+eDVB::getInstance()->time_difference;
	tm tmp = *localtime( &now ),  // now
		tmp2 = *localtime( &t );    // activation hour:min

	tmp.tm_isdst=-1;
	tmp2.tm_isdst=-1;

// calc mask for today weekday
	int i = ePlaylistEntry::Su;
	for ( int x = 0; x < tmp.tm_wday; x++ )
		i*=2;

	bool found=false;

	int checkDays=8;
	for ( int d=0; d<=checkDays; d++,
											i*=2,
											tmp.tm_mday++ )
	{
//		eDebug("d = %d, tmp.tm_mday = %d", d, tmp.tm_mday);
		if ( i > ePlaylistEntry::Sa )  // wrap around..
			i = ePlaylistEntry::Su;

		if ( type & i )
		{
//			eDebug("match");
			if ( !d )
			{
				// in normal case we must check only 7 days ...
				// but when the event do begin at next day... 
				// and the timer is aborted in old day (in startCountDown state)
				// we must check on day more
				--checkDays;  
//				eDebug("today");
				int begTimeSec = tmp2.tm_hour*3600+tmp2.tm_min*60+tmp2.tm_sec;
				int nowTimeSec = tmp.tm_hour*3600+tmp.tm_min*60+tmp.tm_sec;
				if ( nowTimeSec > begTimeSec+duration )
				{
//					eDebug("%d > %d ... continue", nowTimeSec, begTimeSec+duration);
					continue;
				}
			}

			int year = last_activation % 1000,
					mon = (last_activation % 100000 ) / 1000,
					day = (last_activation % 100000000) / 1000000;
			time_t last_act = (year*1000000)+((100+mon)*1000)+day+100;
			int activation=(tmp.tm_year*1000000)+((100+tmp.tm_mon+1)*1000)+tmp.tm_mday+100;
//			eDebug("%d <= %d ?", activation, last_act );
			if ( activation <= last_act )
			{
//				eDebug("yes.. continue");
				continue;
			}
//			else
//				eDebug("no.. found");
			found = true;
			break;
		}
	}
	if ( !found )  // No day(s) selected
		return 0;
	tmp.tm_hour = tmp2.tm_hour;
	tmp.tm_min = tmp2.tm_min;
	tmp.tm_sec = tmp2.tm_sec;
	normalize( tmp );
	return mktime(&tmp);
}

eString getRight(const eString& str, char c )
{
	unsigned int found = str.find(c);
	unsigned int beg = ( found != eString::npos ? found : 0 );
	unsigned int len = str.length();
	if ( found != eString::npos )
		beg++;
	return str.mid( beg, len-beg );
}

static int getDate()
{
	time_t tmp = time(0)+eDVB::getInstance()->time_difference;
	tm now = *localtime(&tmp);
	return ((100+now.tm_mday)*1000000)+((100+now.tm_mon+1)*1000)+now.tm_year;
}

eString getLeft( const eString& str, char c )
{
	unsigned int found = str.find(c);
	return found != eString::npos ? str.left(found):str;
}

static const eString& getEventDescrFromEPGCache( const eServiceReference &_ref, time_t time, int event_id, eString &title, eString &descr)
{
	title = "";
	descr = "";
	const eServiceReferenceDVB &ref = (eServiceReferenceDVB&)_ref;
	// parse EPGCache to get event informations
	EITEvent *tmp = event_id != -1 ? eEPGCache::getInstance()->lookupEvent( ref, event_id ) : 0;
	if ( !tmp )
		tmp = eEPGCache::getInstance()->lookupEvent( ref, time );
	if (tmp)
	{
		LocalEventData led;
		led.getLocalData(tmp, &title, &descr);
		delete tmp;
	}
	return descr;
}

static eString buildDayString(int type)
{
	eString tmp;
	if ( type & ePlaylistEntry::isRepeating )
	{
		const char *dayStrShort[7] = { _("Sun"), _("Mon"), _("Tue"), _("Wed"), _("Thu"), _("Fri"), _("Sat") };
		int mask = ePlaylistEntry::Su;
		for ( int i = 0; i < 7; i++ )
		{
			// tmp += (type & mask) ? toupper(dayStrShort[i][0]) : tolower(dayStrShort[i][0]);
			if ( type & mask )
			{
				tmp += dayStrShort[i][0];
				// tmp+=dayStrShort[i];
				// tmp+=' ';
			}
			else
				//tmp += i%2 ? "_" : "-";
				tmp += '*';
			mask*=2;
		}
		// if ( tmp )
			// tmp.erase(tmp.length()-1);
	}
	return tmp;
}

#ifdef WRITE_LOGFILE
void eTimerManager::writeToLogfile( const char *str )
{
	if ( logfile && str )
	{
		time_t tmp = time(0)+eDVB::getInstance()->time_difference;
		tm now = *localtime( &tmp );
		eString str2;
		str2.sprintf("%02d.%02d, %02d:%02d - %s\n", now.tm_mday, now.tm_mon+1, now.tm_hour, now.tm_min, str );
		logfilesize += (fwrite(str2.c_str(), str2.size(), 1, logfile) * str2.size());
	}
}

void eTimerManager::writeToLogfile( eString str )
{
	if ( str.length() )
		writeToLogfile( str.c_str() );
}
#endif

// DBOX2 DEEPSTANDBY DEFINES
#ifndef FP_IOCTL_SET_WAKEUP_TIMER
#define FP_IOCTL_SET_WAKEUP_TIMER 6
#endif

#ifndef FP_IOCTL_IS_WAKEUP
#define FP_IOCTL_IS_WAKEUP 9
#endif

#ifndef FP_IOCTL_CLEAR_WAKEUP_TIMER
#define FP_IOCTL_CLEAR_WAKEUP_TIMER 10
#endif

eTimerManager::eTimerManager()
	:actionTimer(eApp), timer(eApp), setdeepstandbywakeup(true)
{
	init_eTimerManager();
}
void eTimerManager::init_eTimerManager()
{
	if (!instance)
		instance = this;

	eServicePlaylistHandler::getInstance()->addNum( 5 );
	timerlistref=eServiceReference( eServicePlaylistHandler::ID, eServiceReference::flagDirectory, 0, 5);
	timerlist=(ePlaylist*)eServiceInterface::getInstance()->addRef(timerlistref);
	ASSERT(timerlist);
	timerlist->service_name=_("Timerlist");
	timerlist->load(CONFIGDIR "/enigma/timer.epl");
	nextStartingEvent=timerlist->getList().end();
	CONNECT( actionTimer.timeout, eTimerManager::actionHandler );
	CONNECT( eDVB::getInstance()->timeUpdated, eTimerManager::timeChanged );

#ifdef WRITE_LOGFILE
	struct stat tmp;
	if ( stat( TIMER_LOGFILE, &tmp ) != -1 )
		logfilesize=tmp.st_size;
	else
	{
		eDebug("[eTimerManager] stat logfile failed (%m)");
		unlink(TIMER_LOGFILE);
		logfilesize=0;
	}
	logfile = fopen(TIMER_LOGFILE, "a" );
	writeToLogfile("Timer is comming up");
#else
	logfile=0;
#endif

	int deepstandbywakeup=0;
	eConfig::getInstance()->getKey("/ezap/timer/deepstandbywakeupset", deepstandbywakeup);
	eDebug("[eTimerManager] deepstandbywakeup is %d", deepstandbywakeup);
	writeToLogfile(eString().sprintf("deepstandbywakeup is %d", deepstandbywakeup));
	if ( deepstandbywakeup )
	{
		if (eSystemInfo::getInstance()->hasStandbyWakeupTimer())
		{
			__u8 isWakeup=0;
			int fd = open("/dev/dbox/fp0", O_RDWR);
			if ( fd >= 0 )
			{
				if ( ::ioctl(fd, FP_IOCTL_IS_WAKEUP, &isWakeup) < 0 )
				{
					eDebug("FP_IOCTL_IS_WAKEUP failed(%m)");
					writeToLogfile("FP_IOCTL_IS_WAKEUP failed");
				}
				else
					writeToLogfile(eString().sprintf("FP_IOCTL_IS_WAKEUP returned %d", isWakeup));
				eDebug("[eTimerManager] isWakeup is %d", isWakeup);
				writeToLogfile(eString().sprintf("isWakeup is %d", isWakeup));
				close(fd);
			}
			else
			{
				eDebug("couldn't open FP !!");
				writeToLogfile("couldn't open FP !!");
			}
			if ( !isWakeup )
				deepstandbywakeup=0;
		}
		else
			deepstandbywakeup=0;
	}

	eDebug("[eTimerManager] now deepstandbywakeup is %d", deepstandbywakeup);
	writeToLogfile(eString().sprintf("now deepstandbywakeup is %d", deepstandbywakeup));

	if (eSystemInfo::getInstance()->hasStandbyWakeupTimer())
	{
		int fd = open("/dev/dbox/fp0", O_RDWR);
		if ( fd >= 0 )
		{
			if ( ::ioctl(fd, FP_IOCTL_CLEAR_WAKEUP_TIMER) < 0 )
			{
				eDebug("FP_IOCTL_CLEAR_WAKEUP failed(%m)");
				writeToLogfile("FP_IOCTL_CLEAR_WAKEUP failed");
			}
			else
			{
				eDebug("FP_IOCTL_CLEAR_WAKEUP_TIMER okay");
				writeToLogfile("FP_IOCTL_CLEAR_WAKEUP okay");
			}
			close(fd);
		}
		else
		{
			eDebug("couldn't open FP to clear wakeup timer !!");
			writeToLogfile("couldn't open FP to clear wakeup timer !!");
		}
	}

	if (!deepstandbywakeup)
	{
		eDebug("[eTimerManager] delKey deepstandbywakeup");
		writeToLogfile("delKey deepstandbywakeup");
		eConfig::getInstance()->delKey("/ezap/timer/deepstandbywakeupset");
		eConfig::getInstance()->flush();
	}

	if ( eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000 )
	{
		time_t now = time(0);
		if ( now < 1072224000 ) // 01.01.2004
			eDebug("RTC not ready... wait for transponder time");
		else // inform all who's waiting for valid system time..
		{
			eDebug("Use valid Linux Time :) (RTC?)");
			eDVB::getInstance()->time_difference=1;
			/*emit*/ eDVB::getInstance()->timeUpdated();
		}
	}
}

void eTimerManager::loadTimerList()
{
	writeToLogfile("--> loadTimerList()");
	if ( nextStartingEvent == timerlist->getList().end() 
		|| (!(nextStartingEvent->type & ePlaylistEntry::stateRunning)) )
	{
		timerlist->load(CONFIGDIR "/enigma/timer.epl");
		nextAction = setNextEvent;
		actionTimer.start(0,true);
	}
	else
	{
		ePlaylistEntry current = *nextStartingEvent;
		timerlist->load(CONFIGDIR "/enigma/timer.epl");
		for (nextStartingEvent = timerlist->getList().begin();
			nextStartingEvent != timerlist->getList().end();
			++nextStartingEvent )
		{
			if ( *nextStartingEvent == current )
			{
				*nextStartingEvent = current;  // for running state..
				eDebug("found nextStartingEvent");
				break;
			}
		}
		if ( nextStartingEvent == timerlist->getList().end() )
		{
			eDebug("nextStartingEvent removed from external app... stop running timer");
			timerlist->getList().push_back(current);
			nextStartingEvent=timerlist->getList().end();
			--nextStartingEvent;
			abortEvent( ePlaylistEntry::errorUserAborted );
		}
	}
	writeToLogfile("<-- loadTimerList()");
}

void eTimerManager::saveTimerList()
{
	writeToLogfile("saveTimerlist()");
	timerlist->save();
}

#if 0
static eString typeToString(int type)
{
	std::stringstream s;
	if ( type & ePlaylistEntry::SwitchTimerEntry )
		s << "SwitchTimerEntry";
	if ( type & ePlaylistEntry::RecTimerEntry )
		s << std::endl << "RecTimerEntry";
	if ( type & ePlaylistEntry::recDVR )
		s << std::endl << "recDVR";
	if ( type & ePlaylistEntry::recVCR )
		s << std::endl << "recVCR";
	if ( type & ePlaylistEntry::recNgrab )
		s << std::endl << "recNgrab";
	if ( type & ePlaylistEntry::stateWaiting )
		s << std::endl << "stateWaiting";
	if ( type & ePlaylistEntry::stateRunning )
		s << std::endl << "stateRunning";
	if ( type & ePlaylistEntry::statePaused )
		s << std::endl << "statePaused";
	if ( type & ePlaylistEntry::stateFinished )
		s << std::endl << "stateFinished";
	if ( type & ePlaylistEntry::stateError )
		s << std::endl << "stateError";
	if ( type & ePlaylistEntry::errorNoSpaceLeft )
		s << std::endl << "errorNoSpaceLeft";
	if ( type & ePlaylistEntry::errorUserAborted )
		s << std::endl << "errorUserAborted";
	if ( type & ePlaylistEntry::errorZapFailed )
		s << std::endl << "errorZapFailed";
	if ( type & ePlaylistEntry::errorOutdated )
		s << std::endl << "errorOutdated";
	if ( type & ePlaylistEntry::isSmartTimer )
		s << std::endl << "isSmartTimer";
	if ( type & ePlaylistEntry::isRepeating )
	{
		s << std::endl << "isRepeating(";
		if ( type & ePlaylistEntry::Su )
			s << "Su";
		if ( type & ePlaylistEntry::Mo )
			s << " Mo";
		if ( type & ePlaylistEntry::Tue )
			s << " Tue";
		if ( type & ePlaylistEntry::Wed )
			s << " Wed";
		if ( type & ePlaylistEntry::Thu )
			s << " Thu";
		if ( type & ePlaylistEntry::Fr )
			s << " Fr";
		if ( type & ePlaylistEntry::Sa )
			s << " Sa";
		s << ')';
	}
	if ( type & ePlaylistEntry::doFinishOnly )
		s << std::endl << "doFinishOnly";
	if ( type & ePlaylistEntry::doShutdown )
		s << std::endl << "doShutdown";
	if ( type & ePlaylistEntry::doGoSleep )
		s << std::endl << "doGoSleep";
	return s.str();
}
#endif

void eTimerManager::timeChanged()
{
	writeToLogfile("--> timeChanged()");
	if ( nextStartingEvent == timerlist->getConstList().end() // no event as next event set
		|| !(nextStartingEvent->type & ePlaylistEntry::stateRunning))  // event is set.. but not running
	{
		nextAction=setNextEvent;
		actionTimer.start(0, true);
	}
	writeToLogfile("<-- timeChanged()");
}

#define WOL_PREPARE_TIME 240
#define ZAP_BEFORE_TIME 60
#define HDD_PREPARE_TIME 10

void eTimerManager::actionHandler()
{
	static int calldepth=0;
	switch( nextAction )
	{
		case zap:
		{
#ifndef DISABLE_FILE
			eServiceReferenceDVB &Ref = (eServiceReferenceDVB&) nextStartingEvent->service;
			eServiceReferenceDVB &rec = (eServiceReferenceDVB&) eServiceInterface::getInstance()->service;

			int canHandleTwoServices=0;
			eServiceHandler *handler=eServiceInterface::getInstance()->getService();
			if ( (handler && handler->getFlags()&eServiceHandlerDVB::flagIsScrambled)
				|| (eDVB::getInstance()->recorder && eDVB::getInstance()->recorder->scrambled) )
			{
				eConfig::getInstance()->getKey("/ezap/ci/handleTwoServices",
					canHandleTwoServices);
			}
			else
				canHandleTwoServices=1;

			long t = getSecondsToBegin();
			if ( (nextStartingEvent->type & ePlaylistEntry::recDVR)
				&& t > HDD_PREPARE_TIME
				&& rec && ( checkTimeshift(rec) || ( !rec.path && canHandleTwoServices && onSameTP(rec, Ref))))
			{
		// we dont zap now.. playback is running.. will zap immediate before eventbegin
				t -= HDD_PREPARE_TIME;
				eDebug("[eTimerManager] can Zap later... in %d sec", t);
				nextAction=zap;
				actionTimer.start( t*1000, true );
				break;
			}
#endif
			eDebug("[eTimerManager] zapToChannel");
/*
			time_t tmp=0;
			if ( !(tmp = getNextEventStartTime( nextStartingEvent->time_begin, nextStartingEvent->duration, nextStartingEvent->type, nextStartingEvent->last_activation ) ) )
					tmp=nextStartingEvent->time_begin;
			tm bla = *localtime(&tmp);
			eString descr = getRight( nextStartingEvent->service.descr, '/');

			eString tmpStr;
			tmpStr.sprintf(_("'%d.%d, %02d:%02d %s'\nPerform this event?"), 
				bla.tm_mday, bla.tm_mon+1, bla.tm_hour, bla.tm_min, descr.c_str() );
          
			eMessageBox mb(tmpStr, _("Upcomming Timer Event"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btYes, 10*1000 );
			mb.show();
			int ret = mb.exec();
			mb.hide();                   
			if (ret == eMessageBox::btYes)*/

			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d zap", ++calldepth));
			if ( !(nextStartingEvent->type&ePlaylistEntry::doFinishOnly) )
			{
				writeToLogfile("call eZapMain::getInstance()->handleStandby()");
				if (nextStartingEvent->type & ePlaylistEntry::SwitchTimerEntry)
					eZapMain::getInstance()->handleStandby();  // real wakeup needed
				else
					eZapMain::getInstance()->handleStandby(-1); 
			}

			if ( nextStartingEvent->service &&
					eServiceInterface::getInstance()->service != nextStartingEvent->service )
			{
				eDebug("[eTimerManager] change to the correct service");
				writeToLogfile("must zap to new service");
				conn = CONNECT( eDVB::getInstance()->switchedService, eTimerManager::switchedService );
				eString save = nextStartingEvent->service.descr;
				nextStartingEvent->service.descr = getLeft( nextStartingEvent->service.descr, '/' );

		// get Parentallocking state
				int pLockActive = pinCheck::getInstance()->pLockActive() && nextStartingEvent->service.isLocked();

				if ( pLockActive ) // P Locking is active ?
				{
					writeToLogfile("Parentallocking is active.. disable");
					pinCheck::getInstance()->setLocked(false);  // then disable for zap
				}
#ifndef DISABLE_FILE
				if ( eDVB::getInstance()->recorder
						&& !(nextStartingEvent->type&ePlaylistEntry::doFinishOnly)
						&& ( !onSameTP(eDVB::getInstance()->recorder->recRef, Ref)
							|| nextStartingEvent->type & ePlaylistEntry::recDVR ) )
				{
					writeToLogfile("must stop running recording :(");
					eZapMain::getInstance()->recordDVR(0,0);
				}
				eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		// workaround for start recording in background when a playback
		// is running or the service is on the same transponder and satellite
				if ( nextStartingEvent->type & ePlaylistEntry::recDVR
					&& handler && handler->getState() != eServiceHandler::stateStopped
					&& rec && ( checkTimeshift(rec) || (!rec.path && canHandleTwoServices && onSameTP(rec,Ref))))
				{
					eDebug("[eTimerManager] change to service in background :)");
					writeToLogfile("zap to correct service in background :)");
					playbackRef = eServiceInterface::getInstance()->service;
					Decoder::locked=2;
					eServiceInterface::getInstance()->play( nextStartingEvent->service, 1 );
				}
				else
#endif
				{
					eDebug("[eTimerManager] must zap in foreground :(");
					writeToLogfile("zap to correct service in foreground :(");
					playbackRef=eServiceReference();
		// switch to service
					eZapMain::getInstance()->playService( nextStartingEvent->service, eZapMain::psNoUser|eZapMain::psSetMode );
				}

				nextStartingEvent->service.descr=save;

				if ( pLockActive )  // reenable Parental locking
				{
					writeToLogfile("reenable parentallocking");
					pinCheck::getInstance()->setLocked(true);
				}
			}
			else
			{
				writeToLogfile("we are alraedy on the correct service");
				eDebug("[eTimerManager] we are already on the correct service... do not change service");
				nextAction=startCountdown;
				actionTimer.start(0, true);
			}
			writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d", calldepth--));
		}
		break;

		case prepareEvent:
		{
			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d prepareEvent", ++calldepth));
			eDebug("[eTimerManager] prepareEvent");
#ifndef DISABLE_NETWORK
			if ( nextStartingEvent->type & ePlaylistEntry::recNgrab )
			{
				writeToLogfile("recNgrab");
				char *serverMAC=0;
				if ( !eConfig::getInstance()->getKey("/elitedvb/network/hwaddress", serverMAC ) )
				{
					if ( system( eString().sprintf("etherwake -i eth0 %s", serverMAC ).c_str() ) )
					{
						writeToLogfile("could not execute etherwake");
						eDebug("[eTimerManager] could not execute etherwake");
					}
					else
					{
						writeToLogfile("Wake On Lan sent...");
						eDebug("[eTimerManager] Wake On Lan sent to %s", serverMAC );
					}
					free( serverMAC );
				}
#endif
				long t = getSecondsToBegin();
				t -= ZAP_BEFORE_TIME;
				if (t < 0) t = 0;
				eDebug("[eTimerManager] zap in %d seconds", t );
				writeToLogfile(eString().sprintf("zap in %d seconds",t));
				nextAction = zap;
				actionTimer.start( t*1000, true );
			}
//			else
//				abortEvent(ePlaylistEntry::errorUserAborted);
			writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d prepareEvent", calldepth--));
			break;
		}

		case startCountdown:
			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d startCountdown", ++calldepth));
			if ( nextStartingEvent->type & ePlaylistEntry::isRepeating )
			{
				writeToLogfile("reset state flags for repeating event");
				nextStartingEvent->type &= ~(
					ePlaylistEntry::stateError |
					ePlaylistEntry::stateFinished |
					ePlaylistEntry::statePaused |
					ePlaylistEntry::errorNoSpaceLeft |
					ePlaylistEntry::errorUserAborted |
					ePlaylistEntry::errorZapFailed|
					ePlaylistEntry::errorOutdated );

				nextStartingEvent->type |= ePlaylistEntry::stateWaiting;
			}
			if ( conn.connected() )
				conn.disconnect();
			if ( nextStartingEvent->type & ePlaylistEntry::doFinishOnly &&
				!nextStartingEvent->service )
				writeToLogfile("this is Sleeptimer... don't toggle TimerMode");
			else
			{
				writeToLogfile("call eZapMain::getInstance()->toggleTimerMode()");
				eZapMain::getInstance()->toggleTimerMode(1);
				// now in eZapMain the RemoteControl should be handled for TimerMode...
				// any service change stops now the Running Event and set it to userAborted
//				conn = CONNECT( eDVB::getInstance()->leaveService, eTimerManager::leaveService );
			}
			{
				long t = getSecondsToBegin();
				nextAction = startEvent;
				if ( nextStartingEvent->type & ePlaylistEntry::RecTimerEntry )
				{
					writeToLogfile("attend HDD_PREPARE_TIME");
					t -= HDD_PREPARE_TIME;  // for HDD Wakeup or whatever
					if (t < 0) t = 0;
					// this wakes up the hdd
					freeRecordSpace();
				}
				writeToLogfile(eString().sprintf("startEvent in %d seconds", t));
				actionTimer.start( t*1000 , true );
			}
			writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d startCountdown", calldepth--));
			break;

		case updateDuration:
			writeToLogfile("--> actionHandler() updateEndTime");
			nextAction = stopEvent;
			actionTimer.start( getSecondsToEnd() * 1000, true );
			writeToLogfile(eString().sprintf("new left duration is %d seconds", getSecondsToEnd()) );
			writeToLogfile("<-- actionHandler() updateEndTime");
			break;

		case startEvent:
			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d startEvent", ++calldepth));
			eDebug("[eTimerManager] startEvent");
			if ( nextStartingEvent->type & ePlaylistEntry::isRepeating )
			{
				writeToLogfile("set last_activation for repeated event");
				time_t tmp = getNextEventStartTime(
					nextStartingEvent->time_begin,
					nextStartingEvent->duration,
					nextStartingEvent->type);
				tm bla = *localtime(&tmp);
				nextStartingEvent->last_activation =
					((100+bla.tm_mday)*1000000)+((100+bla.tm_mon+1)*1000)+bla.tm_year;
			}
			nextStartingEvent->type &= ~ePlaylistEntry::stateWaiting;

			nextStartingEvent->type |= ePlaylistEntry::stateRunning;

			if (nextStartingEvent->type & ePlaylistEntry::doFinishOnly )
			{
				writeToLogfile("only Finish an running event");
				eDebug("[eTimerManager] only Finish an running event");
			}
			else if (nextStartingEvent->type & ePlaylistEntry::RecTimerEntry)
			{
				nextAction = startRecording;
				actionHandler();
			}
			else if (nextStartingEvent->type & ePlaylistEntry::SwitchTimerEntry)
			{
				writeToLogfile("SwitchTimerEvent... do nothing");
			}

			if ( playbackRef && playbackRef.type == eServiceReference::idDVB )
				writeToLogfile("set stopEventTime after zap back to playbackRef");
			else
			{
				nextAction = stopEvent;
				actionTimer.start( getSecondsToEnd() * 1000, true );
				writeToLogfile(eString().sprintf("stopEvent in %d seconds", getSecondsToEnd()) );
				writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d startEvent", calldepth--));
			}
			break;

		case pauseEvent:
			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d pauseEvent", ++calldepth));
			eDebug("[eTimerManager] pauseEvent");
			if ( nextStartingEvent->type & ePlaylistEntry::RecTimerEntry )
			{
				nextStartingEvent->type &= ~ePlaylistEntry::stateRunning;
				nextStartingEvent->type |= ePlaylistEntry::statePaused;
				nextAction = pauseRecording;
				actionHandler();
			}
			writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d pauseEvent", calldepth--));
			break;

		case restartEvent:
			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d restartEvent", ++calldepth));
			eDebug("[eTimerManager] restartEvent");
			if ( nextStartingEvent->type & ePlaylistEntry::RecTimerEntry )
			{
				nextStartingEvent->type &= ~ePlaylistEntry::statePaused; // we delete current state...
				nextStartingEvent->type |= ePlaylistEntry::stateRunning;
				nextAction=restartRecording;
				actionHandler();
			}
			writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d restartEvent", calldepth--));
			break;

		case stopEvent:
			if( nextStartingEvent->type & ePlaylistEntry::stateRunning )
			{
				writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d stopEvent", ++calldepth));
				eDebug("[eTimerManager] stopEvent");

				nextStartingEvent->type &= ~ePlaylistEntry::stateRunning;
	   // when no ErrorCode is set the we set the state to finished
				if ( !(nextStartingEvent->type & ePlaylistEntry::stateError) )
				{
					writeToLogfile("set to stateFinished");
					nextStartingEvent->type |= ePlaylistEntry::stateFinished;
				}
				if ( nextStartingEvent->type & ePlaylistEntry::RecTimerEntry )
				{
					nextAction=stopRecording;
					actionHandler();
				}
				else // SwitchTimer
				{
					writeToLogfile("SwitchTimer... do nothing");
				}

				if ( nextStartingEvent->type & ePlaylistEntry::doFinishOnly
					&& !nextStartingEvent->service )
					writeToLogfile("this is a Sleeptimer... dont toggle TimerMode");
				else
				{
					writeToLogfile("call eZapMain::getInstance()->toggleTimerMode()");
					eZapMain::getInstance()->toggleTimerMode(0);
				}
				if ( pinCheck::getInstance()->pLockActive() && eServiceInterface::getInstance()->service.isLocked() )
				{
					writeToLogfile("this service is parentallocked... stop service now");
					eServiceInterface::getInstance()->stop();
				}
				writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d stopEvent", calldepth--));
			}
			nextAction=setNextEvent;	// we set the Next Event... a new beginning *g*
			actionTimer.start(0, true);
			break;

		case setNextEvent:
		{
#ifdef WRITE_LOGFILE
//////////// SHRINK LOGFILE WHEN TO BIG //////////////////
			if ( logfilesize > 100*1024 )
			{
				eDebug("timer logfile is bigger than 100Kbyte.. shrink to 32kByte");
				rename(TIMER_LOGFILE, "/var/timer.old");
				logfile = fopen(TIMER_LOGFILE, "a" );
				FILE *old = fopen("/var/timer.old", "r");
				if ( old )
				{
					fseek(old, logfilesize-32768, SEEK_SET );
					char buf[32768];
					int rbytes=0;
					if( ( rbytes = fread( buf, 1, 32768, old ) ) )
						fwrite( buf, rbytes, 1, logfile );
					fclose(old);
					unlink("/var/timer.old");
					logfilesize=32768;
				}
			}
///////////////////////////////////////////
#endif
			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d setNextEvent", ++calldepth));
			eDebug("[eTimerManager] setNextEvent");
			if (conn.connected() )
				conn.disconnect();

			std::list< ePlaylistEntry >::iterator prevEvent = nextStartingEvent;

			nextStartingEvent=timerlist->getList().end();
			int timeToNextEvent=INT_MAX, count=0;
			// parse events... invalidate old, set nextEvent Timer
			for ( std::list< ePlaylistEntry >::iterator i(timerlist->getList().begin()); i != timerlist->getList().end(); )
			{
				writeToLogfile(eString().sprintf("(%d), descr = %s",count, i->service.descr.c_str()));
				time_t nowTime=time(0)+eDVB::getInstance()->time_difference;
				if ( i->type & ePlaylistEntry::isRepeating )
				{
					writeToLogfile(eString().sprintf(" - time_begin = %d, getDate()=%d, lastActivation=%d", i->time_begin,
						getDate(),
						i->last_activation ));
					time_t tmp = getNextEventStartTime( i->time_begin, i->duration, i->type, i->last_activation );
					tm evtTime = *localtime( &tmp );
					writeToLogfile(eString().sprintf(" - isRepeating event (days %s)", buildDayString(i->type).c_str() ));
					writeToLogfile(eString().sprintf(" - starts at %d (%02d.%02d, %02d:%02d)", tmp, evtTime.tm_mday, evtTime.tm_mon+1, evtTime.tm_hour, evtTime.tm_min));
					if ( tmp-nowTime < timeToNextEvent && nowTime < tmp+i->duration )
					{
						nextStartingEvent=i;
						timeToNextEvent = tmp-nowTime;
						writeToLogfile(eString().sprintf(" - is our new nextEvent... timeToNextEvent is %d", timeToNextEvent));
					}
					count++;
				}
				else if ( i->type & ePlaylistEntry::stateWaiting )
				{
					tm tmp = *localtime(&i->time_begin);
					writeToLogfile(" - is Single Event");
					writeToLogfile(eString().sprintf(" - starts at %d (%02d.%02d, %02d:%02d)", i->time_begin, tmp.tm_mday, tmp.tm_mon+1, tmp.tm_hour, tmp.tm_min));
					writeToLogfile(" - have stateWaiting");
					if ( i->type & ePlaylistEntry::stateError )
					{
						writeToLogfile(" - have stateError... unset stateWaiting");
						i->type &= ~ePlaylistEntry::stateWaiting;
					}
					else if ( i->time_begin+i->duration < nowTime ) // old event found
					{
						writeToLogfile(" - is an outdated event... set to stateError");
						i->type &= ~ePlaylistEntry::stateWaiting;
						i->type |= (ePlaylistEntry::stateError|ePlaylistEntry::errorOutdated);
					}
					else if ( i->type & ePlaylistEntry::doFinishOnly )
					{
						writeToLogfile(" - is doFinishOnly.. use this... dont check other events");
						nextStartingEvent=i;
						timeToNextEvent = i->time_begin - nowTime;
						// dont check the other events...
						break;
					}
					else if( (i->time_begin - nowTime) < timeToNextEvent )
					{
						nextStartingEvent=i;
						timeToNextEvent = i->time_begin - nowTime;
						writeToLogfile(eString().sprintf(" - is our new nextEvent... timeToNextEvent is %d", timeToNextEvent));
						count++;
					}
					else
					{
						count++;
						writeToLogfile(" - is not the nextEvent... ignore");
					}
				}
				else if ( i->type & ePlaylistEntry::doFinishOnly )
				{
					writeToLogfile(" - is an old doFinishOnly Event... remove this");
					eDebug("old doFinishOnly");
					// all timers they only finish a running action/event
					// are remove from the list after they ended
					i = timerlist->getList().erase(i);  // alten ShutOffTimer aus liste entfernen...
					continue;
				}
				i++;
			}
			eDebug("[eTimerManager] updated ( %d waiting events in list )", count );
			if ( nextStartingEvent != timerlist->getList().end() )
			{
				tm evtTime;
				writeToLogfile("our new nextStartingEvent:");
				if ( nextStartingEvent->type & ePlaylistEntry::isRepeating )
				{
					writeToLogfile(" - is a repeating event");

					if ( nextStartingEvent->last_activation != -1 )
						writeToLogfile(eString().sprintf(" - lastActivation was at %02d.%02d.%04d", 
								(nextStartingEvent->last_activation / 1000000)-100,
								(nextStartingEvent->last_activation % 100000)/1000,
								(nextStartingEvent->last_activation % 100) + 2000 ));

					time_t tmp = getNextEventStartTime( nextStartingEvent->time_begin, nextStartingEvent->duration, nextStartingEvent->type, nextStartingEvent->last_activation );
					if ( tmp )
						evtTime=*localtime( &tmp );
				}
				else
					evtTime=*localtime( &nextStartingEvent->time_begin );
				eDebug("[eTimerManager] next event starts at %02d.%02d, %02d:%02d", evtTime.tm_mday, evtTime.tm_mon+1, evtTime.tm_hour, evtTime.tm_min );
				writeToLogfile(eString().sprintf(" - descr/service is %s",nextStartingEvent->service.descr.length()?nextStartingEvent->service.descr.c_str():"" ));
				writeToLogfile(eString().sprintf(" - starts at %02d.%02d, %02d:%02d", evtTime.tm_mday, evtTime.tm_mon+1, evtTime.tm_hour, evtTime.tm_min));
				writeToLogfile(eString().sprintf(" - event type is %d, %08X", nextStartingEvent->type, nextStartingEvent->type ));
				long t = getSecondsToBegin();

				nextAction=zap;
				if ( nextStartingEvent->type & ePlaylistEntry::RecTimerEntry )
				{
					writeToLogfile(" - is a RecTimerEnty");
#ifndef DISABLE_NETWORK
					if ( nextStartingEvent->type & ePlaylistEntry::recNgrab )
					{
						writeToLogfile(" - is a recNgrab Event.. attend WOL_PREPARE_TIME");
						t -= WOL_PREPARE_TIME; // for send WOL Signal
						if (t < 0) t = 0;
						nextAction=prepareEvent;
						writeToLogfile("nextAction = prepareEvent");
					}
					else
#endif
					{
						t -= ZAP_BEFORE_TIME;
						if (t < 0) t = 0;
						writeToLogfile("nextAction = zap");
					}
				}
				else
					writeToLogfile("nextAction = zap");
				// set the Timer to eventBegin
				writeToLogfile(eString().sprintf("   starts in %d seconds", t ));
				actionTimer.startLongTimer(t);
			}
			else
			{
				writeToLogfile("no more waiting events...");
				actionTimer.stop();
			}
			if ( prevEvent != timerlist->getConstList().end() )
			{
//				eDebug("prevEvent is %s", typeToString(prevEvent->type).c_str() );
				if ( prevEvent != nextStartingEvent || 
					( prevEvent->type & ePlaylistEntry::isRepeating && 
						prevEvent->type & ePlaylistEntry::stateFinished &&
						!(prevEvent->type & ePlaylistEntry::stateWaiting) ) )
				{
					if ( prevEvent->type&ePlaylistEntry::stateError && 
							 prevEvent->type&ePlaylistEntry::errorUserAborted )
					{
						// must clear wasSleeping flag in enigma_main.cpp
						eDebug("user has aborted the previous event handleStandby returns %d",
							eZapMain::getInstance()->handleStandby(1) );
						writeToLogfile("don't handleStandby.. user has aborted the previous event");
					}
					else if ( prevEvent->type&ePlaylistEntry::stateWaiting ||
						!(prevEvent->type&ePlaylistEntry::stateFinished) )
					{
						writeToLogfile("don't handleStandby.. prev event is not finished.. or was waiting");
						eDebug("don't handleStandby.. prev event is not finished.. or was waiting");
					}
					else if ( nextStartingEvent == timerlist->getConstList().end() || getSecondsToBegin() > 10*60 )
					{
						int i=-1;
						if ( prevEvent->type & ePlaylistEntry::doShutdown )
							i=2;
						else if ( prevEvent->type & ePlaylistEntry::doGoSleep )
							i=3;

						// is sleeptimer?
						if ( prevEvent->type & ePlaylistEntry::doFinishOnly &&
							!prevEvent->service )
							i*=2; // force.. look in eZapMain::handleStandby

						// this gets wasSleeping from enigma..
						// 2 -> enigma was Wakedup from timer
						// 3 -> enigma was coming up from deepstandby.. initiated by timer
						int enigmaState = eZapMain::getInstance()->handleStandby(1);
						eDebug("call eZapMain::handleStandby(1) .. returned is %d, i was %d", enigmaState, i );
						writeToLogfile(eString().sprintf("call eZapMain::handleStandby(1) .. returned is %d, i was %d", enigmaState, i ) );

						if ( i == -1 )
							i = enigmaState;

						if ( prevEvent != timerlist->getConstList().end() 
							&& prevEvent->type & ePlaylistEntry::stateFinished )
						{
							eDebug("call eZapMain::handleStandby(%d)", i);
							writeToLogfile(eString().sprintf("call eZapMain::handleStandby(%d)", i));
							eZapMain::getInstance()->handleStandby(i);
						}
						else
						{
							eDebug("dont call handleStandby");
							writeToLogfile("dont call handleStandby");
						}
					}
					if ( prevEvent->type & ePlaylistEntry::isRepeating 
						&& prevEvent->type & ePlaylistEntry::stateFinished
						&& !(prevEvent->type & ePlaylistEntry::stateWaiting) )
					{
						writeToLogfile("[eTimerManager]reset stateWaiting to repeating timer");
						eDebug("[eTimerManager]reset stateWaiting to repeating timer");
						prevEvent->type |= ePlaylistEntry::stateWaiting;
					}
				}
			}
			if ( prevEvent != nextStartingEvent )
				timerlist->save();
			writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d setNextEvent", calldepth--));
		}
		break;

		case startRecording:
			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d startRecording", ++calldepth));
			eDebug("[eTimerManager] start recording");
#if ( defined(DISABLE_FILE) + defined(DISABLE_NETWORK) ) < 2
			if ( nextStartingEvent->type & (ePlaylistEntry::recDVR|ePlaylistEntry::recNgrab) )
			{
				writeToLogfile("recDVR or Ngrab");
//				eDebug("nextStartingEvent->service.data[0] = %d", nextStartingEvent->service.data[0] );
//				eDebug("nextStartingEvent->service.descr = %s", nextStartingEvent->service.descr.c_str() );
				eString recordDescr;
				time_t tmp=0;
				if ( nextStartingEvent->type & ePlaylistEntry::isRepeating )
				{
					writeToLogfile("repeating");
					tmp = getNextEventStartTime( nextStartingEvent->time_begin, nextStartingEvent->duration, nextStartingEvent->type );
					// build date instead of epoisode information
					tm evtTime = *localtime(&tmp);
					recordDescr.sprintf(" - %02d.%02d.%02d,", evtTime.tm_mday, evtTime.tm_mon+1, evtTime.tm_year%100 );
				}
				eString descr = getRight( nextStartingEvent->service.descr, '/');
				if ( recordDescr )
					descr += recordDescr;
#ifndef DISABLE_FILE
				if ( nextStartingEvent->type & ePlaylistEntry::recDVR )
				{
					writeToLogfile(eString().sprintf("call eZapMain::getInstance()->recordDVR(1,0,%s)",descr.length()?descr.c_str():""));
					eDebug("[eTimerManager] start DVR");

					if ( !tmp )
						tmp = nextStartingEvent->time_begin + (nextStartingEvent->duration/2);
					else
						tmp += nextStartingEvent->duration/2;

					int result = eZapMain::getInstance()->recordDVR(1, 0, tmp, descr.length()?descr.c_str():0 );
					writeToLogfile(eString().sprintf("result is %d", result));
					if (result < 0)
					{
							/* recording did not start due an error. an error message will already be displayed on the screen. */
							abortEvent(ePlaylistEntry::errorNoSpaceLeft);
					}

					if ( playbackRef )
					{
						writeToLogfile("we have playbackRef...zap back to old service");
						if ( playbackRef.type == eServiceReference::idDVB )
							conn2 = CONNECT( eDVB::getInstance()->switchedService, eTimerManager::switchedService );
						eServiceInterface::getInstance()->play( playbackRef, -1 );
						if ( playbackRef.type != eServiceReference::idDVB )
						{
							eDebug("old service is running again (no DVB) :)");
							writeToLogfile("old service is running again (no DVB) :)");
							Decoder::locked=0;
							playbackRef = eServiceReference();
						}
					}
				}
				else
#endif
#ifndef DISABLE_NETWORK
				if (nextStartingEvent->type & ePlaylistEntry::recNgrab)
				{
					eDebug("[eTimerManager] start Ngrab");
					writeToLogfile(eString().sprintf("call ENgrab::getNew()->sendStart(%s)",descr.length()?descr.c_str():""));
					if (eSystemInfo::getInstance()->getDefaultTimerType() == ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recDVR)
						eZapMain::getInstance()->startNGrabRecord();
					else
						ENgrab::getNew()->sendstart(descr.length()?descr.c_str():0);
				}
#else
				{
				}
#endif
			}
			else
#endif // defined(DISABLE_FILE) + defined(DISABLE_NETWORK) < 2
			{
#ifndef DISABLE_LIRC
				if (nextStartingEvent->type & ePlaylistEntry::recVCR)
				{
					writeToLogfile("call eLirc::getNew()->sendStart()");
					eDebug("[eTimerManager] start VCR-Lirc-Recording");
					ELirc::getNew()->sendstart();
				}
#endif
			}
			writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d startRecording", calldepth--));
			break;

		case stopRecording:
			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d stopRecording", ++calldepth));
			eDebug("[eTimerManager] stop recording");
#ifndef DISABLE_FILE
			if (nextStartingEvent->type & ePlaylistEntry::recDVR)
			{
				writeToLogfile("call eZapMain::getInstance()->recordDVR(0,0)");
				eDebug("[eTimerManager] stop DVR");
				int ret = eZapMain::getInstance()->recordDVR(0, 0);
				writeToLogfile(eString().sprintf("result is %d", ret));
			}
			else
#endif
#ifndef DISABLE_NETWORK
			if (nextStartingEvent->type & ePlaylistEntry::recNgrab)
			{
				writeToLogfile("call ENgrab::getNew()->sendstop()");
				eDebug("[eTimerManager] stop Ngrab");
//				ENgrab::getNew()->sendstop();
				eZapMain::getInstance()->stopNGrabRecord();
			}
			else  // insert lirc ( VCR stop ) here
#endif
			if (nextStartingEvent->type & ePlaylistEntry::recVCR)
			{
#ifndef DISABLE_LIRC
				writeToLogfile("call ELirc::getNew()->sendstop()");
				eDebug("[eTimerManager] stop VCR-Lirc-Recording");
				ELirc::getNew()->sendstop();
#endif
			}
			writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d stopRecording", calldepth--));
			break;
/*
		case restartRecording:
			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d restartRecording", ++calldepth));
			eDebug("[eTimerManager] restart recording");
#ifndef DISABLE_FILE
			if (nextStartingEvent->type & ePlaylistEntry::recDVR)
			{
				writeToLogfile("recDVR");
				eServiceHandler *handler=eServiceInterface::getInstance()->getService();
				if (!handler)
					eFatal("no service Handler");
				handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordStart));
			}
			else // insert lirc ( VCR START )
#endif
			{
#ifndef DISABLE_LIRC
				writeToLogfile("recVCR... not Implemented !!");
#endif
			}
			writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d restartRecording", calldepth--));
			break;

		case pauseRecording:
			writeToLogfile(eString().sprintf("--> actionHandler() calldepth=%d pauseRecording", ++calldepth));
#ifndef DISABLE_FILE
			if (nextStartingEvent->type & ePlaylistEntry::recDVR)
			{
				writeToLogfile("recDVR");
				eServiceHandler *handler=eServiceInterface::getInstance()->getService();
				if (!handler)
					eFatal("no service Handler");
				handler->serviceCommand(eServiceCommand(eServiceCommand::cmdRecordStop));
			}
			else // insert lirc ( VCR PAUSE )
#endif
			{
#ifndef DISABLE_LIRC
				writeToLogfile("recvcr not implemented...");
#endif
			}
			writeToLogfile(eString().sprintf("<-- actionHandler() calldepth=%d pauseRecording", calldepth--));
			break;
*/
		case oldService:
			Decoder::locked=0;
			playbackRef = eServiceReference();
			conn2.disconnect();
			eDebug("actionHandler() back on old service.. set stoptimer..");
			writeToLogfile("actionHandler() back on old service.. set stoptimer..");
			nextAction = stopEvent;
			actionTimer.start( getSecondsToEnd() * 1000, true );
			writeToLogfile(eString().sprintf("stopEvent in %d seconds", getSecondsToEnd()) );
			break;
		default:
		{
			writeToLogfile(eString().sprintf("unhandled timer action %d !!!", nextAction));
			eDebug("unhandled timer action %d", nextAction);
		}
	}
#ifdef WRITE_LOGFILE
	fflush(logfile);
#endif
}

void eTimerManager::switchedService( const eServiceReferenceDVB &ref, int err)
{
	if ( ref == (eServiceReference&)playbackRef )
	{
		writeToLogfile("switchedService back on playbackref :)");
		// back on old service.. reconnect enigma to dvb kram..
		nextAction=oldService;
		actionTimer.start(0,true);
	}
	else if ( err == -ENOSTREAM )
	{
		if ( Decoder::locked )
			Decoder::locked = 0;
		writeToLogfile("call abortEvent");
		abortEvent( ePlaylistEntry::errorZapFailed );
	}
	else
	{
		writeToLogfile("--> switchedService()");
		if ( nextStartingEvent->service != (eServiceReference&)ref )
		{
			if ( Decoder::locked )
				Decoder::locked = 0;
			writeToLogfile("call abortEvent");
			abortEvent( ePlaylistEntry::errorZapFailed );
		}
		else  // zap okay
		{
			writeToLogfile("call startCountdown");
			nextAction=startCountdown;
			actionTimer.start(0,true);
		}
		writeToLogfile("<-- switchedService()");
	}
}

void eTimerManager::abortEvent( int err )
{
	if ( nextStartingEvent->type & ePlaylistEntry::stateWaiting )
	{
		nextStartingEvent->type &= ~ePlaylistEntry::stateWaiting;
		nextStartingEvent->type |= ePlaylistEntry::stateRunning;
		if (nextStartingEvent->type & ePlaylistEntry::isRepeating)
		{
				writeToLogfile("set last_activation for aborted repeated event");
				time_t tmp = getNextEventStartTime(
					nextStartingEvent->time_begin,
					nextStartingEvent->duration,
					nextStartingEvent->type);
				tm bla = *localtime(&tmp);
				nextStartingEvent->last_activation =
					((100+bla.tm_mday)*1000000)+((100+bla.tm_mon+1)*1000)+bla.tm_year;
		}
	}
	writeToLogfile(eString().sprintf("--> abortEvent(err %d)",err));
	eDebug("[eTimerManager] abortEvent");
	nextAction=stopEvent;

	nextStartingEvent->type |= (ePlaylistEntry::stateError|err);
	actionHandler();
	writeToLogfile("<-- abortEvent");
}

void eTimerManager::leaveService( const eServiceReferenceDVB& ref )
{
	if ( ref == playbackRef )
	{
		eDebug("leaveplaybackRef");
		return;
	}
	writeToLogfile(eString().sprintf("--> leaveService %s", ref.toString().c_str() ));
	eDebug("[eTimerManager] leaveService");
	abortEvent( ePlaylistEntry::errorUserAborted );
	writeToLogfile("<-- leaveService");
}

long eTimerManager::getSecondsToBegin()
{
	time_t nowTime = time(0) + eDVB::getInstance()->time_difference;
	time_t tmp = 0;
	if ( (tmp = getNextEventStartTime( nextStartingEvent->time_begin, nextStartingEvent->duration, nextStartingEvent->type, nextStartingEvent->last_activation ) ) )
		tmp -= nowTime;
	else
		tmp = nextStartingEvent->time_begin - nowTime;
	eDebug("[eTimerManager] Seconds to begin = %ld, timediff = %ld", tmp, eDVB::getInstance()->time_difference);
	return tmp;
}

long eTimerManager::getSecondsToEnd()
{
	time_t nowTime = time(0)+eDVB::getInstance()->time_difference;
	time_t tmp = 0;
	if ( (tmp = getNextEventStartTime( nextStartingEvent->time_begin, nextStartingEvent->duration, nextStartingEvent->type ) ) )
		return (tmp + nextStartingEvent->duration) - nowTime;
	return (nextStartingEvent->time_begin + nextStartingEvent->duration) - nowTime;
}

eTimerManager::~eTimerManager()
{
	int setWakeupKey=0;
	if ( setdeepstandbywakeup && eSystemInfo::getInstance()->hasStandbyWakeupTimer() )
	{
		int fd = open("/dev/dbox/fp0", O_RDWR);
		if ( nextStartingEvent != timerlist->getList().end() )
		{
			switch ( eSystemInfo::getInstance()->getHwType() )
			{
				case eSystemInfo::dbox2Nokia:
				case eSystemInfo::dbox2Philips:
				case eSystemInfo::dbox2Sagem:
				{
					int min = getSecondsToBegin() / 60;
					min -= 5;   // we start the box 5 minutes before event begin

					if ( min < 1 )
						min = 1;

					int erg;
					if((erg=ioctl(fd, FP_IOCTL_SET_WAKEUP_TIMER, &min))<0)
					{
						if(erg==-1) // Wakeup not supported
						{
							eDebug("[eTimerManager] deepstandby wakeup not supported");
							writeToLogfile("deepstandby wakeup not supported");
						}
						else
						{
							eDebug("[eTimerManager] error setting wakeup");
							writeToLogfile("error setting wakeup");
						}
					}
					else
					{
						eDebug("[eTimerManager] deepStandby wakeup in %d minutes programmed", min );
						writeToLogfile(eString().sprintf("deepStandby wakeup in %d minutes programmed", min));
						setWakeupKey=1;
					}
					break;
				}
				case eSystemInfo::DM7000:
				case eSystemInfo::DM7020:
				{
					time_t tmp=0;
					if ( !(tmp = getNextEventStartTime( nextStartingEvent->time_begin, nextStartingEvent->duration, nextStartingEvent->type, nextStartingEvent->last_activation ) ) )
						tmp=nextStartingEvent->time_begin;
					tmp -= 5*60;
					if(::ioctl(fd, FP_IOCTL_SET_WAKEUP_TIMER, &tmp)<0)
					{
						eDebug("FP_IOCTL_SET_WAKEUP_TIMER failed (%m)");
						writeToLogfile("FP_IOCTL_SET_WAKEUP_TIMER failed");
					}
					else
					{
						tm bla = *localtime(&tmp);
						eDebug("Deepstandby wakeup at %02d.%02d, %02d:%02d", 
							bla.tm_mday, bla.tm_mon+1, bla.tm_hour, bla.tm_min );
						writeToLogfile(eString().sprintf("Deepstandby wakeup at %02d.%02d, %02d:%02d",
							bla.tm_mday, bla.tm_mon+1, bla.tm_hour, bla.tm_min));
						setWakeupKey=1;
					}
					break;
				}
				default:
					eDebug("no support for wakeup on this platform... ");
			}
		}
	}
	if ( setWakeupKey )
	{
		eConfig::getInstance()->setKey("/ezap/timer/deepstandbywakeupset", 1);
		writeToLogfile("write setWakeupKey to config");
	}
	else
	{
		eConfig::getInstance()->delKey("/ezap/timer/deepstandbywakeupset");
		writeToLogfile("remove setWakeupKey from config");
	}

	writeToLogfile("~eTimerManager()");
#ifdef WRITE_LOGFILE
	fclose(logfile);
#endif
	eDebug("[eTimerManager] down ( %d events in list )", timerlist->getList().size() );
	if (this == instance)
		instance = 0;
	timerlist->save();
	eServiceInterface::getInstance()->removeRef(timerlistref);
}

ePlaylistEntry* eTimerManager::findEvent( eServiceReference *service, EITEvent *evt )
{
// is the middle of the event overlapping with this event in timerlist?
// with this we prevent that the event before and after the "main" event
// are marked as "in timer" in the epg list
	ePlaylistEntry tmp(*service,
		evt->start_time + (evt->duration/2) -1,
		3, evt->event_id);

	for ( std::list<ePlaylistEntry>::iterator i( timerlist->getList().begin() ); i != timerlist->getList().end(); i++)
		if ( *service == i->service && Overlapping(*i, tmp ) )
			return &*i;

	return 0;
}

bool Overlap( time_t beginTime1, int duration1, time_t beginTime2, int duration2 )
{
	eRect movie1( ePoint(beginTime1, 0), eSize( duration1, 10) );
	eRect movie2( ePoint(beginTime2, 0), eSize( duration2, 10) );

	return movie1.intersects(movie2);
}

bool msOverlap( const ePlaylistEntry &m, const ePlaylistEntry &s )
{
	struct tm multiple = *localtime( &m.time_begin ),
				 Entry = *localtime( &s.time_begin );

	if ( s.type & (ePlaylistEntry::stateError|ePlaylistEntry::stateFinished) )
		return false;

	int todayDate = getDate();
	if ( m.last_activation == todayDate && m.type & ePlaylistEntry::stateError )
	{
		tm evtTime = *localtime(&s.time_begin);
		int evtDate = ((100+evtTime.tm_mday)*1000000)+((100+evtTime.tm_mon+1)*1000)+evtTime.tm_year;
		if ( evtDate == todayDate )
			return false;
	}

	int mask = ePlaylistEntry::Su;
	for ( int x=0; x < 7; ++x )
	{
		if ( m.type & mask )
		{
			int tmp1 = Entry.tm_wday*24*3600+Entry.tm_hour*3600+Entry.tm_min*60;
			int tmp2 = x*24*3600+multiple.tm_hour*3600+multiple.tm_min*60;
			if ( Overlap( tmp1, s.duration, tmp2, m.duration ) )
				return true;
			else if ( tmp1+s.duration > 24*7*3600 && Overlap( 0, tmp1+s.duration-24*7*3600, tmp2, m.duration ) )// event over end of weak
				return true;
			else if ( tmp2+m.duration > 24*7*3600 && Overlap( 0, tmp2+m.duration-24*7*3600, tmp1, s.duration ) )
				return true;
		}
		mask <<= 1;
	}
	return false;
}

bool mmOverlap( const ePlaylistEntry &m1, const ePlaylistEntry &m2 )
{
	struct tm multiple1 = *localtime( &m1.time_begin ),
		     multiple2 = *localtime( &m2.time_begin );

	int mask1 = ePlaylistEntry::Su;
	for ( int x=0; x < 7; ++x )
	{
		if ( m1.type & mask1 )
		{
			int mask2 = ePlaylistEntry::Su;
			for ( int y=0; y < 7; ++y )
			{
				if ( m2.type & mask2 )
				{
					int tmp1 = x*24*3600+multiple1.tm_hour*3600+multiple1.tm_min*60;
					int tmp2 = y*24*3600+multiple2.tm_hour*3600+multiple2.tm_min*60;
					if ( Overlap( tmp1, m1.duration, tmp2, m2.duration ) )
						return true;
					else if ( tmp1+m1.duration > 24*7*3600 && Overlap( 0, tmp1+m1.duration-24*7*3600, tmp2, m2.duration ) )
						return true;
					else if ( tmp2+m2.duration > 24*7*3600 && Overlap( 0, tmp2+m2.duration-24*7*3600, tmp1, m1.duration ) )
						return true;
				}
				mask2 <<= 1;
			}
		}
		mask1 <<= 1;
	}
	return false;
}

bool Overlapping( const ePlaylistEntry &e1, const ePlaylistEntry &e2 )
{
	bool overlap=false;
	if ( e1.type & ePlaylistEntry::isRepeating && e2.type & ePlaylistEntry::isRepeating )
		overlap = mmOverlap( e1, e2 );
	else if ( e1.type & ePlaylistEntry::isRepeating )
		overlap = msOverlap( e1, e2 );
	else if ( e2.type & ePlaylistEntry::isRepeating )
		overlap = msOverlap( e2, e1 );
	else overlap =
		(  !( e1.type & (ePlaylistEntry::stateError|ePlaylistEntry::stateFinished) )
		&& !( e2.type & (ePlaylistEntry::stateError|ePlaylistEntry::stateFinished) )
		&& Overlap( e2.time_begin, e2.duration, e1.time_begin, e1.duration) );
	return overlap;
}

bool eTimerManager::removeEventFromTimerList( eWidget *sel, const ePlaylistEntry& entry, int type )
{
	for ( std::list<ePlaylistEntry>::iterator i( timerlist->getList().begin() ); i != timerlist->getList().end(); i++)
		if ( *i == entry )
		{
			sel->hide();
			eString str1, str2, str3;
			if (type == erase)
			{
				str1 = _("You would delete the running event..\nthis stops the timer mode (recording)!");
				str2 = _("Delete the event from the timer list");
				str3 = _("Really delete this event?");
			}
			else if (type == update)
			{
				str1 = _("You would update the running event.. \nthis stops the timer mode (recording)!");
				str2 = _("Update event in timer list");
				str3 = _("Really update this event?");
			}

			int ret = eMessageBox::btNo;
			if ( &(*nextStartingEvent) == &entry && entry.type & ePlaylistEntry::stateRunning  )
			{
				ret = eMessageBox::ShowBox(str1+'\n'+str3, str2, eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btNo);
			}
			else
			{
				ret = eMessageBox::ShowBox(str3, str2, eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btNo);
			}
			if (ret == eMessageBox::btYes)
			{
				eDebug("remove Event");
				timerlist->getList().erase(i);
				if ( &(*nextStartingEvent) == &entry )
				{
					if ( nextStartingEvent->type & ePlaylistEntry::stateRunning )
					{
						nextAction=stopEvent;
						nextStartingEvent->type |= (ePlaylistEntry::stateError | ePlaylistEntry::errorUserAborted);
						actionHandler();
					}
					else if ( !(nextStartingEvent->type & ePlaylistEntry::stateRunning) )
					{
						nextAction=setNextEvent;
						actionHandler();
					}
				}
				sel->show();
				return true;
			}
			sel->show();
			break;
		}
	return false;
}

void eTimerManager::cleanupEvents()
{
	for ( std::list<ePlaylistEntry>::iterator i( timerlist->getList().begin() ); i != timerlist->getList().end();)
		if ((i->type & (ePlaylistEntry::stateFinished|ePlaylistEntry::stateError)) && !(i->type & ePlaylistEntry::isRepeating))
			i = timerlist->getList().erase(i);
		else
	 		++i;
}

void eTimerManager::clearEvents()
{
	for ( std::list<ePlaylistEntry>::iterator i( timerlist->getList().begin() ); i != timerlist->getList().end();)
		i = timerlist->getList().erase(i);
	nextStartingEvent = timerlist->getList().end();
	actionTimer.stop();
}

int eTimerManager::deleteEventFromTimerList(const ePlaylistEntry &entry, bool force)
{
	if (nextStartingEvent != timerlist->getConstList().end()
		&& *nextStartingEvent == entry && nextStartingEvent->type & ePlaylistEntry::stateRunning)
	{
		if (force)
			abortEvent( ePlaylistEntry::errorUserAborted);
		else
			return -1;
	}
	for (std::list<ePlaylistEntry>::iterator i(timerlist->getList().begin()); i != timerlist->getList().end(); i++)
	{
		if (*i == entry)
		{
			timerlist->getList().erase(i);
			if ( nextStartingEvent == timerlist->getList().end()
				|| (!(nextStartingEvent->type & ePlaylistEntry::stateRunning)))
			{
				nextAction=setNextEvent;
				actionTimer.start(0,true);
			}
			break;
		}
	}
	return 0;
}

int eTimerManager::modifyEventInTimerList(const ePlaylistEntry &old_entry, const ePlaylistEntry &new_entry, bool force)
{
	if ( nextStartingEvent != timerlist->getConstList().end()
		&& *nextStartingEvent == old_entry && nextStartingEvent->type & ePlaylistEntry::stateRunning )
	{
		if (force) // change only duration and 'after_event' action
		{
			updateRunningEvent(new_entry.duration, new_entry.type & (ePlaylistEntry::doGoSleep|ePlaylistEntry::doShutdown) );
			return 0;
		}
		return -1;  
		// no changes on running event..  
		// you can ask the user if he wants to change duration and 'after_event' action only..
	}
	for (std::list<ePlaylistEntry>::iterator i(timerlist->getList().begin()); i != timerlist->getList().end(); i++)
	{
		if (old_entry == *i)
		{
			*i = new_entry;
			if ( nextStartingEvent == timerlist->getList().end() 
				|| (!(nextStartingEvent->type & ePlaylistEntry::stateRunning)) )
			{
				nextAction=setNextEvent;
				actionTimer.start(0,true);
			}
			break;
		}
	}
	return 0;
}

bool eTimerManager::updateRunningEvent( int duration, int after_event )
{
	if ( nextStartingEvent->type & ePlaylistEntry::stateRunning )
	{
		nextStartingEvent->duration = duration;
		nextStartingEvent->type &= ~(ePlaylistEntry::doGoSleep|ePlaylistEntry::doShutdown);
		nextStartingEvent->type |= after_event;
		nextAction = updateDuration;
		actionHandler();
		return true;
	}
	return false;
}

bool eTimerManager::removeEventFromTimerList( eWidget *sel, const eServiceReference *ref, const EITEvent *evt )
{
	ePlaylistEntry tmp(*ref, evt->start_time, evt->duration, evt->event_id);
	for ( std::list<ePlaylistEntry>::iterator i( timerlist->getList().begin() ); i != timerlist->getList().end(); i++)
		if ( *ref == i->service && Overlapping(*i, tmp ) )
			return removeEventFromTimerList( sel, *i );
	return false;
}

bool eTimerManager::eventAlreadyInList( eWidget *w, EITEvent &e, eServiceReference &ref )
{
	ePlaylistEntry tmp(ref, e.start_time, e.duration, e.event_id);
	for ( std::list<ePlaylistEntry>::iterator i( timerlist->getList().begin() ); i != timerlist->getList().end(); i++)
		if ( ref == i->service && Overlapping(*i, tmp ) )
		{
			w->hide();
			eMessageBox::ShowBox(
				_("This event is already in the timerlist."),
				_("Add event to timerlist"),
				eMessageBox::iconWarning|eMessageBox::btOK);
			w->show();
			return true;
		}
	return false;
}

int eTimerManager::addEventToTimerList(const ePlaylistEntry& entry)
{
	for ( std::list<ePlaylistEntry>::iterator i(timerlist->getList().begin()); i != timerlist->getList().end(); i++)
	{
		if (Overlapping(*i, entry))
			return -1; // event overlaps with an event that is already in the timer list
	}

	timerlist->getList().push_back(entry);
	if (nextStartingEvent == timerlist->getList().end() ||
			!(nextStartingEvent->type & ePlaylistEntry::stateRunning))
	{
		nextAction = setNextEvent;
		actionHandler();
	}
	return 1;
}

bool eTimerManager::addEventToTimerList( eWidget *sel, const ePlaylistEntry& entry, const ePlaylistEntry *exclude )
{
	for ( std::list<ePlaylistEntry>::iterator i( timerlist->getList().begin() ); i != timerlist->getList().end(); i++)
	{
		if ( exclude && *exclude == *i )
			continue;
		if ( Overlapping(*i, entry) )
		{
			if ( entry.type & (ePlaylistEntry::doFinishOnly|ePlaylistEntry::stateRunning) )
			{
				sel->hide();
				eMessageBox::ShowBox(_("The Endtime overlaps with another event in the timerlist"), _("Set Stop Time"), eMessageBox::iconWarning|eMessageBox::btOK);
				sel->show();
			}
			else
			{
				sel->hide();
				eMessageBox::ShowBox(_("This event cannot added to the timerlist.\n"
					"The event overlaps with another event in the timerlist\n"
					"Please check the timerlist manually."), _("Add event to timerlist"), eMessageBox::iconWarning|eMessageBox::btOK);
				sel->show();
			}
			return false;
		}
	}
	if (!exclude)
	{
		timerlist->getList().push_back( entry );
		if ( nextStartingEvent == timerlist->getList().end() ||
				!(nextStartingEvent->type & ePlaylistEntry::stateRunning) )
		{
			nextAction = setNextEvent;
			actionHandler();
		}
	}
	return true;
}

bool eTimerManager::addEventToTimerList( eWidget *sel, const eServiceReference *ref, const EITEvent *evt, int type, const ePlaylistEntry *exclude )
{
	if ( !exclude )
	{
		eSubServiceSelector subservicesel(false);
		eServiceReference *subref=0;
		for (ePtrList<Descriptor>::const_iterator d(evt->descriptor); d != evt->descriptor.end(); ++d)
		{
			if (d->Tag()==DESCR_LINKAGE)
			{
				LinkageDescriptor *ld=(LinkageDescriptor*)*d;
				if (ld->linkage_type==0xB0)
					subservicesel.add(((eServiceReferenceDVB*)ref)->getDVBNamespace(), ld);
			}
		}
		if ( subservicesel.getSelected() )  // channel have subservices?
		{
			sel->hide();
			subservicesel.show();
			eWindow::globalCancel(eWindow::OFF);
			if ( !subservicesel.exec() )
				subref=subservicesel.getSelected();
			eWindow::globalCancel(eWindow::ON);
			subservicesel.hide();
			sel->show();
		}
		if ( subref )
		{
			subref->descr+=ref->descr;
			ePlaylistEntry e( *subref, evt->start_time, evt->duration, evt->event_id, type );
			return addEventToTimerList( sel, e, exclude );
		}
	}
	ePlaylistEntry e( *ref, evt->start_time, evt->duration, evt->event_id, type );
//	eDebug("e.service.descr = %s", e.service.descr.c_str() );
//	eDebug("descr = %s", descr.c_str() );
//	eString tmp = getLeft(e.service.descr, '/');
//	eDebug("tmp = %s", tmp.c_str() );
//	e.service.descr = tmp + '/' + descr;
	return addEventToTimerList( sel, e, exclude );
}

eAutoInitP0<eTimerManager> init_eTimerManager(eAutoInitNumbers::osd-1, "Timer Manager");

gFont eListBoxEntryTimer::TimeFont;
gFont eListBoxEntryTimer::DescrFont;
gPixmap *eListBoxEntryTimer::ok=0;
gPixmap *eListBoxEntryTimer::failed=0;
int eListBoxEntryTimer::timeXSize=0;
int eListBoxEntryTimer::dateXSize=0;
int eListBoxEntryTimer::dateYSize=0;

struct eTimerViewActions
{
	eActionMap map;
	eAction addTimerEntry, removeTimerEntry;
	eTimerViewActions():
		map("timerView", _("timerView")),
		addTimerEntry(map, "addTimerEntry", _("add new event to Timerlist"), eAction::prioDialog ),
		removeTimerEntry(map, "removeTimerEntry", _("remove this entry from timer list"), eAction::prioDialog )
	{
	}
};
eAutoInitP0<eTimerViewActions> i_TimerViewActions(eAutoInitNumbers::actions, "timer view actions");

eListBoxEntryTimer::~eListBoxEntryTimer()
{
	if (paraTime)
		paraTime->destroy();

	if (paraDate)
		paraDate->destroy();

	if (paraDescr)
		paraDescr->destroy();

	if (paraService)
		paraService->destroy();
}

int eListBoxEntryTimer::getEntryHeight()
{
	if (!DescrFont.pointSize && !TimeFont.pointSize)
	{
		int clktype = 0;

		DescrFont = eSkin::getActive()->queryFont("eEPGSelector.Entry.Description");
		TimeFont = eSkin::getActive()->queryFont("eEPGSelector.Entry.DateTime");
		ok = eSkin::getActive()->queryImage("ok_symbol");
		failed = eSkin::getActive()->queryImage("failed_symbol");
		eTextPara* tmp = new eTextPara( eRect(0, 0, 250, 30) );
		tmp->setFont( TimeFont );
		eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);
		if (clktype)
			tmp->renderString( "55:55PM X 55:55PM" );
		else
			tmp->renderString( "55:55 X 55:55" );
		timeXSize = tmp->getBoundBox().width();
		tmp->destroy();
		tmp = new eTextPara( eRect(0, 0, 200, 30) );
		tmp->setFont( TimeFont );
		// tmp->renderString( "55.55x" );
		tmp->renderString( "SMTWDFS " );
		dateXSize = tmp->getBoundBox().width();
		dateYSize = tmp->getBoundBox().height();
		// eDebug("getEntryHeight: timeXsize %d dateXsize %d dateYsize %d", timeXSize, dateXSize, dateYSize);
		tmp->destroy();
	}
	return (calcFontHeight(DescrFont)+4)*2;
}

eListBoxEntryTimer::eListBoxEntryTimer( eListBox<eListBoxEntryTimer> *listbox, ePlaylistEntry* entry )
		:eListBoxEntry((eListBox<eListBoxEntry>*)listbox),
		paraDate(0), paraTime(0), paraDescr(0), paraService(0), entry(entry)
{
}

const eString &eListBoxEntryTimer::redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int hilited)
{
	drawEntryRect(rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, hilited);

	int xpos=rect.left()+10;

	if ( entry->type & ePlaylistEntry::stateFinished )
	{
		int ypos = ( rect.height() - ok->y ) / 2;
		rc->blit( *ok, ePoint( xpos, rect.top()+ypos ), eRect(), gPixmap::blitAlphaTest);
	}
	else if ( entry->type & ePlaylistEntry::stateError )
	{
		int ypos = (rect.height() - failed->y) / 2;
		rc->blit( *failed, ePoint( xpos, rect.top()+ypos ), eRect(), gPixmap::blitAlphaTest);
	}
	xpos+=24+10; // i think no people want to change the ok and false pixmaps....

	tm start_time = *localtime(&entry->time_begin);
	time_t t = entry->time_begin+entry->duration;
	tm stop_time = *localtime(&t);
	if (!paraDescr)
	{
		hlp = getRight( entry->service.descr, '/' );
		paraDescr = new eTextPara( eRect( 0 ,0, rect.width(), rect.height()/2) );
		paraDescr->setFont( DescrFont );
		paraDescr->renderString( hlp );
		DescrYOffs = ((rect.height()/2 - paraDescr->getBoundBox().height()) / 2 ) - paraDescr->getBoundBox().top();
	}
	rc->renderPara(*paraDescr, ePoint( xpos, rect.top() + DescrYOffs + rect.height()/2 ) );

	if (!paraDate)
	{
		eString tmp = buildDayString( entry->type );
		if (tmp )
		{
			int newdateXSize = (dateXSize * tmp.size()) / strlen("00:00");
			paraDate = new eTextPara( eRect( 0, 0, newdateXSize, rect.height()/2) );
			eDebug("daystring: newdateXsize %d", newdateXSize);
		}
		else
		{
			tmp.sprintf("%02d.%02d,", start_time.tm_mday, start_time.tm_mon+1);
			paraDate = new eTextPara( eRect( 0, 0, dateXSize, rect.height()/2) );
		}
		paraDate->setFont( TimeFont );
		paraDate->renderString( tmp );
		TimeYOffs = ((rect.height()/2 - paraDate->getBoundBox().height()) / 2 ) - paraDate->getBoundBox().top();
		hlp=tmp+' '+hlp;
	}
	// dateXSize is a global, so we need to check for paraDate size every time.
	// eDebug("paraDateSize %d, dateXsize %d xpos %d", paraDate->getBoundBox().width(), dateXSize, xpos);
	rc->renderPara(*paraDate, ePoint( xpos, rect.top() + TimeYOffs ) );
	xpos += dateYSize + ((paraDate->getBoundBox().width() > dateXSize) ? paraDate->getBoundBox().width() : dateXSize);

	if (!paraTime)
	{
		paraTime = new eTextPara( eRect( 0, 0, timeXSize, rect.height()/2) );
		paraTime->setFont( TimeFont );
		eString tmp;
		tmp = getTimeStr(&start_time, 0) + " - " + getTimeStr(&stop_time, 0);
		paraTime->renderString( tmp );
		hlp=hlp+' '+tmp;
	}
	// eDebug("paraTimeSize %d, timeXsize %d xpos %d", paraTime->getBoundBox().width(), timeXSize, xpos);
	rc->renderPara(*paraTime, ePoint( xpos, rect.top() + TimeYOffs ) );
	xpos+=timeXSize+dateYSize;

	if (!paraService)
	{
		eString sname = getLeft(entry->service.descr,'/');
		if (!sname)
		{
			eService *service = eServiceInterface::getInstance()->addRef( entry->service );
			if ( service )
			{
				sname = service->service_name;
				eServiceInterface::getInstance()->removeRef( entry->service );
			}
		}
		if (sname)
		{
			paraService = new eTextPara( eRect( 0, 0, rect.width()-xpos, rect.height()/2) );
			paraService->setFont( TimeFont );
			paraService->renderString( sname );
		}
	}
	if ( paraService )
		rc->renderPara(*paraService, ePoint( xpos, rect.top() + TimeYOffs ) );

	return hlp;
}

static int weekday (int d, int m, int y)
{
	static char table[13] = {0,0,3,2,5,0,3,5,1,4,6,2,4};
	if (m<3)
		--y;
	return (y+y/4-y/100+y/400+table[m]+d)%7;
}

void normalize( struct tm &t )
{
	while ( t.tm_sec > 59 )
	{
		t.tm_sec -= 60;
		t.tm_min++;
	}
	while ( t.tm_min > 59 )
	{
		t.tm_min -= 60;
		t.tm_hour++;
	}
	while ( t.tm_hour > 23 )
	{
		t.tm_hour-=24;
		t.tm_mday++;
	}
	int days = monthdays[t.tm_mon];
	if ( days==28 && __isleap(t.tm_year) )
		days++;
	if ( t.tm_mday > days )
	{
		t.tm_mday -= days;
		t.tm_mon++;
	}
	while ( t.tm_mon > 11 )
	{
		t.tm_year++;
		t.tm_mon-=11;
	}
	t.tm_wday=-1;
	t.tm_yday=-1;
}

eTimerListView::eTimerListView()
	:eWindow(0)
{
	init_eTimerListView();
}
void eTimerListView::init_eTimerListView()
{
	events = new eListBox<eListBoxEntryTimer>(this);
	events->setName("events");
	events->setActiveColor(eSkin::getActive()->queryScheme("eServiceSelector.highlight.background"), eSkin::getActive()->queryScheme("eServiceSelector.highlight.foreground"));
	CONNECT(events->selected, eTimerListView::entrySelected );
	events->setFlags(eListBoxBase::flagLostFocusOnFirst|eListBoxBase::flagLostFocusOnLast);

	erase = new eButton( this );
	erase->setName("remove");
	CONNECT( erase->selected, eTimerListView::erasePressed );

	add = new eButton( this );
	add->setName("add");
	CONNECT( add->selected, eTimerListView::addPressed );

	cleanup = new eButton( this );
	cleanup->setName("cleanup");
	CONNECT( cleanup->selected, eTimerListView::cleanupPressed );

	if (eSkin::getActive()->build(this, "eTimerListView"))
		eWarning("Timer view widget build failed!");

	setText(_("Timer list"));

	fillTimerList();

	addActionMap( &i_TimerViewActions->map );

	/* help text for timer setup */
	setHelpText(_("\tTimer\n\n>>> [MENU] >>> [8] Timer\n. . . . . . . . . .\n\n" \
		"Here you can add/remove/review timer settings for DVR (recording), Switching, and NGrab\n" \
		". . . . . . . . . .\n\n" \
		"Usage:\n\n[UP]/[DOWN]\tSelect Inputfield or Button\n\nTimer list:\n" \
		"[UP]/[DOWN]\tScroll through the list\n[OK]\tSelect item\n\n" \
		"[GREEN]\tAdd new event\n\n[RED]\tRemove timer event\n\n[BLUE]\tCleanup list (remove old events)\n\n" \
		"[EXIT]\tClose window without saving changes"));
}

void eTimerListView::erasePressed()
{
	if ( events->getCount() && eTimerManager::getInstance()->removeEventFromTimerList( this, *events->getCurrent()->entry ) )
		fillTimerList();
}

void eTimerListView::addPressed()
{
	hide();
	eTimerEditView e(0);
	e.show();
	if ( !e.exec() )
		fillTimerList();
	e.hide();
	show();
}

void eTimerListView::cleanupPressed()
{
	eTimerManager::getInstance()->cleanupEvents();
	fillTimerList();
}

void eTimerListView::entrySelected(eListBoxEntryTimer *entry)
{
	// FinishOnly Timers can not edited
	if ( entry && entry->entry->service &&
		!(entry->entry->type&ePlaylistEntry::doFinishOnly) )
	{
		hide();
		eTimerEditView e( entry->entry );
#ifndef DISABLE_LCD
		e.setLCD( LCDTitle, LCDElement );
#endif
		e.show();
		if ( !e.exec() )
			fillTimerList();
		e.hide();
		show();
	}
}

#if 0
int ExpectedMoviesDuration;
#endif

struct addToView
{
	eListBox<eListBoxEntryTimer> *listbox;

	addToView(eListBox<eListBoxEntryTimer> *lb): listbox(lb)
	{
	}

	void operator()(ePlaylistEntry* se)
	{
		new eListBoxEntryTimer(listbox, se);
#if 0
		if ( (se->type & se->recDVR) && ((se->type & se->stateWaiting) || (se->type & se->stateRunning) || (se->type & se->isRepeating)))
		{
			ExpectedMoviesDuration += (se->duration / 60); // in minutes
		}
#endif
	}
};

void eTimerListView::fillTimerList()
{
	events->beginAtomic();
	events->clearList();
#if 0
	ExpectedMoviesDuration = 0;
#endif
	eTimerManager::getInstance()->forEachEntry( addToView(events) );
	events->sort();
	events->endAtomic();
#if 0
#ifndef DISABLE_FILE
	int fds = freeRecordSpace();
	if (fds != -1)
	{
		eString Caption = "";
		eString Caption2 = "";
		int min = (fds / 33); 
		if (min < 60)
			Caption = eString().sprintf(", %s %d min",("free space: about"), min);
		else
			Caption = eString().sprintf(", %s %d h, %02d min",("free space: about"), min/60, min%60);
		if (ExpectedMoviesDuration < 60)
			Caption2 = eString().sprintf(": %d min", ExpectedMoviesDuration);
		else
			Caption2 = eString().sprintf(": %d h, %02d min", ExpectedMoviesDuration/60, ExpectedMoviesDuration%60);
		eString sTitle = (_("Timerlist"));
		setText(sTitle + Caption2 + Caption);
	}
#endif	
#endif
}

struct TimerEditActions
{
	eActionMap map;
	eAction incBegTime, decBegTime, incEndTime, decEndTime;
	TimerEditActions():
		map("timerEdit", _("Timer Edit View")),
		incBegTime(map, "incBegTime", _("increase the event begin time in 1 minute steps"), eAction::prioDialog ),
		decBegTime(map, "decBegTime", _("decrease the event begin time in 1 minute steps"), eAction::prioDialog ),
		incEndTime(map, "incEndTime", _("increase the event end time in 1 minute steps"), eAction::prioDialog ),
		decEndTime(map, "decEndTime", _("decrease the event end time in 1 minute steps"), eAction::prioDialog )
	{
	}
};

eAutoInitP0<TimerEditActions> i_TimerEditActions(eAutoInitNumbers::actions, "timer edit actions");

void eTimerEditView::createWidgets()
{
	const char *monthStr[12] = { _("January"), _("February"), _("March"),
								_("April"), _("May"), _("June"), _("July"),
								_("August"), _("September"), _("October"),
								_("November"), _("December") };
	event_name = new eTextInputField(this);
	event_name->setName("event_name");

	int takefocus = !(curEntry && curEntry->type&ePlaylistEntry::stateRunning);
	event_name->setActive(takefocus);
	CONNECT(event_name->selected, eTimerEditView::eventNameSelected);

	multiple = new eCheckbox(this, 0, takefocus);
	multiple->setName("multiple");
	CONNECT(multiple->checked, eTimerEditView::multipleChanged);

	cMo = new eCheckbox(this, 0, takefocus);
	cMo->setName("Mo");

	after_event = new eComboBox( this, 3, 0 );
	after_event->setName("after_event");
	new eListBoxEntryText( *after_event, _("Nothing"), (void*) 0, 0, _("do nothing") );
	new eListBoxEntryText( *after_event, _("Standby"), (void*) ePlaylistEntry::doGoSleep, 0, _("put box into standby") );
	if ( eSystemInfo::getInstance()->canShutdown() )
	{
		if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM600PVR ||
			eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM500PLUS)
			new eListBoxEntryText( *after_event, _("Shutdown"), (void*) ePlaylistEntry::doShutdown, 0, _("shutdown the box") );
		else
			new eListBoxEntryText( *after_event, _("Shutdown"), (void*) ePlaylistEntry::doShutdown, 0, _("put box into deep standby") );
	}

	cTue = new eCheckbox(this, 0, takefocus);
	cTue->setName("Tue");

	cWed = new eCheckbox(this, 0, takefocus);
	cWed->setName("Wed");

	cThu = new eCheckbox(this, 0, takefocus);
	cThu->setName("Thu");

	byear = new eComboBox(this, 5, 0, takefocus);
	byear->setName("b_year");

	bmonth = new eComboBox(this, 5, 0, takefocus);
	bmonth->setName("b_month");

	bday = new eComboBox(this, 5, 0, takefocus);
	bday->setName("b_day");

	lBegin = new eLabel(this);
	lBegin->setName("lBegin");

	btime = new eNumber( this, 2, 0, 59, 2, 0, 0, lBegin, takefocus );
	btime->setName("b_time");
	btime->setFlags( eNumber::flagDrawPoints|eNumber::flagFillWithZeros|eNumber::flagTime );
	CONNECT( btime->selected, eTimerEditView::focusNext );

	bAmpm = new eComboBox( this, 2, 0 );
	bAmpm->setName("b_ampm");
	new eListBoxEntryText( *bAmpm, _("AM"), (void*) 0);
	new eListBoxEntryText( *bAmpm, _("PM"), (void*) 1);

	cFr = new eCheckbox(this, 0, takefocus);
	cFr->setName("Fr");

	cSa = new eCheckbox(this, 0, takefocus);
	cSa->setName("Sa");

	cSu = new eCheckbox(this, 0, takefocus);
	cSu->setName("Su");

	eyear = new eComboBox(this, 5, 0, takefocus);
	eyear->setName("e_year");

	emonth = new eComboBox(this, 5, 0, takefocus);
	emonth->setName("e_month");

	eday = new eComboBox(this, 5, 0, takefocus);
	eday->setName("e_day");

	lEnd = new eLabel(this);
	lEnd->setName("lEnd");

	etime = new eNumber( this, 2, 0, 59, 2, 0, 0, lEnd );
	etime->setName("e_time");
	etime->setFlags( eNumber::flagDrawPoints|eNumber::flagFillWithZeros|eNumber::flagTime );
	CONNECT( etime->selected, eTimerEditView::focusNext );

	eAmpm = new eComboBox( this, 2, 0 );
	eAmpm->setName("e_ampm");
	new eListBoxEntryText( *eAmpm, _("AM"), (void*) 0);
	new eListBoxEntryText( *eAmpm, _("PM"), (void*) 1);

	int clktype = 0;
	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);

	if (!clktype)
	{ // hide am/pm widgets in 24-hour mode
		bAmpm->hide();
		eAmpm->hide();
	}

	type = new eComboBox( this, 5, 0, takefocus );
	type->setName("type");

	bSelectService = new eButton( this, 0, takefocus );
	bSelectService->setName("select_service");
	CONNECT( bSelectService->selected, eTimerEditView::showServiceSelector );

	bApply = new eButton( this );
	bApply->setName("apply");
	CONNECT( bApply->selected, eTimerEditView::applyPressed );

	bScanEPG = new eButton(this, 0, takefocus );
	bScanEPG->setName("scanEPG");
	CONNECT( bScanEPG->selected, eTimerEditView::scanEPGPressed);

	CONNECT(byear->selchanged_id, eTimerEditView::comboBoxClosed);
	CONNECT(bmonth->selchanged_id, eTimerEditView::comboBoxClosed);
	CONNECT(bday->selchanged_id, eTimerEditView::comboBoxClosed);
	CONNECT(eyear->selchanged_id, eTimerEditView::comboBoxClosed);
	CONNECT(emonth->selchanged_id, eTimerEditView::comboBoxClosed);
	CONNECT(eday->selchanged_id, eTimerEditView::comboBoxClosed);

	if (eSkin::getActive()->build(this, "eTimerEditView"))
		eWarning("Timer view widget build failed!");

	if (clktype)
	{  
		// 12-hour mode
		// The text "start time" and "stop time" should always be left from resp. "Thu" and "Sun"
		lBegin->move(ePoint(cThu->getPosition().x() + cThu->getSize().width() + 10, lBegin->getPosition().y()));
		lEnd->move(ePoint(cSu->getPosition().x() + cSu->getSize().width() + 10, lEnd->getPosition().y()));
		
		// move around resize a view widgets to get space for the checkboxes.
		// note: this assumes a specific layout order...
		btime->move(ePoint(btime->getPosition().x() - bAmpm->getSize().width() - 10, btime->getPosition().y()));
		etime->move(ePoint(etime->getPosition().x() - eAmpm->getSize().width() - 10, etime->getPosition().y()));
		bday->resize(eSize(bday->getSize().width() - bAmpm->getSize().width() - 10, bday->getSize().height()));
		eday->resize(eSize(eday->getSize().width() - eAmpm->getSize().width() - 10, eday->getSize().height()));
	}
	time_t tmp = time(0)+eDVB::getInstance()->time_difference;
	tm now = *localtime( &tmp );

	new eListBoxEntryText( *byear, "1970", (void*)70 ); // needed when change a multiple event to single event
	for ( int i=0; i<10; i++ )
		new eListBoxEntryText( *byear, eString().sprintf("20%02d", now.tm_year+(i-100)), (void*) (now.tm_year+i) );

	for ( int i=0; i<=11; i++ )
		new eListBoxEntryText( *bmonth, monthStr[i], (void*) i );

	new eListBoxEntryText( *eyear, "1970", (void*)70 ); // needed when change a multiple event to single event
	for ( int i=0; i<10; i++ )
		new eListBoxEntryText( *eyear, eString().sprintf("20%02d", now.tm_year+(i-100)), (void*) (now.tm_year+i) );

	for ( int i=0; i<=11; i++ )
		new eListBoxEntryText( *emonth, monthStr[i], (void*) i );

	new eListBoxEntryText( *type, _("switch"), (void*) ePlaylistEntry::SwitchTimerEntry );
#ifndef DISABLE_FILE
	if ( eSystemInfo::getInstance()->canRecordTS() )
		new eListBoxEntryText( *type, _("record DVR"), (void*) (ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recDVR) );
#endif
#ifndef DISABLE_NETWORK
	if ( eSystemInfo::getInstance()->hasNetwork() )
		new eListBoxEntryText( *type, _("Ngrab"), (void*) (ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recNgrab) );
#endif
#ifndef DISABLE_LIRC
	switch ( eSystemInfo::getInstance()->getHwType() )
	{
		case eSystemInfo::dbox2Nokia ... eSystemInfo::dbox2Philips:
			new eListBoxEntryText( *type, _("Record VCR"), (void*) (ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recVCR) );
		default:
			break;
	}
#endif
	addActionMap( &i_TimerEditActions->map );
}

eTimerEditView::eTimerEditView( ePlaylistEntry* e)
	:curEntry(e), event_id(-1)
{
	init_eTimerEditView(e);
}
void eTimerEditView::init_eTimerEditView( ePlaylistEntry* e)
{
	createWidgets();

	if ( e )
	{
		fillInData( e->time_begin, e->duration, e->type, e->service );
		multipleChanged( e->type&ePlaylistEntry::isRepeating );
	}
	else
	{
		// If this timer is added it will start directly, it makes no sense to subtract the start offset
		time_t now = time(0)+eDVB::getInstance()->time_difference;

		int timeroffsetstop = 0;
		eConfig::getInstance()->getKey("/enigma/timeroffsetstop", timeroffsetstop);

		int type = eSystemInfo::getInstance()->getDefaultTimerType();
		fillInData( now, (timeroffsetstop * 60), type, eServiceInterface::getInstance()->service );
		multipleChanged( 0 );

		// load user defined default action; use nothing when no value has been stored
		int defaultaction = 0;

		eConfig::getInstance()->getKey("/enigma/timerenddefaultaction", defaultaction);

		// set user defined action in combobox
		after_event->setCurrent((void*)defaultaction);
	}

	/* help text for timer details */
	setHelpText(_("\tTimer\n\n>>> [MENU] >>> [8] Timer >>> Select Event\n. . . . . . . . . .\n\n" \
					"Here you can set the details of an event for DVR (recording), Switching, and NGrab\n. . . . . . . . . .\n\n" \
					"Usage:\n\nRepeated Timer:\nUse [OK] to toggle repetition on/off - if in repeat mode, toggle the day(s) on/off by using [OK]\n\n" \
					"Start time and Stop time:\nFirst line (year, month, day, time) represents the START time of an event, the second line the STOP time.\n" \
					"Use [UP]/[DOWN]/[LEFT]/[RIGHT] to select the desired fields. Use [OK]/[UP]/[DOWN] to select from a dropdown box, or use [NUMBERS] to enter time\n\n" \
					"Don't forget to select the event type (record DVR, switch, NGrab), service (channel etc), and what to do after the event (Nothing, Shutdown, Standby).\n\n" \
					"[BLUE]\tSee if there is matching EPG data\n\n[EXIT]\tClose window without saving changes"));
}

eTimerEditView::eTimerEditView( const EITEvent &e, int type, eServiceReference ref )
	:curEntry(0), event_id(e.event_id)
{
	createWidgets();
	int timeroffsetstart = 0;
	eConfig::getInstance()->getKey("/enigma/timeroffsetstart", timeroffsetstart);
	int timeroffsetstop = 0;
	eConfig::getInstance()->getKey("/enigma/timeroffsetstop", timeroffsetstop);

	fillInData( e.start_time - (timeroffsetstart * 60), e.duration + (timeroffsetstart * 60) + (timeroffsetstop * 60), type, ref );
	multipleChanged(0);

	// load user defined default action; use nothing when no value has been stored
	int defaultaction = 0;

	eConfig::getInstance()->getKey("/enigma/timerenddefaultaction", defaultaction);

	// set user defined action in combobox
	after_event->setCurrent((void*)defaultaction);

	scanEPGPressed();
}

void eTimerEditView::fillInData( time_t begin_time, int duration, int ttype, eServiceReference& ref )
{
	eString descr = getRight( ref.descr, '/' );
	if ( descr.length() )
		event_name->setText(descr);
	else
		event_name->setText(_("No description available"));
	
	beginTime = *localtime( &begin_time );
	time_t tmp = begin_time + duration;
	endTime = *localtime( &tmp );
	updateDateTime( beginTime, endTime, 3 );
	type->setCurrent( (void*) ( ttype & (ePlaylistEntry::RecTimerEntry|ePlaylistEntry::SwitchTimerEntry|ePlaylistEntry::recDVR|ePlaylistEntry::recVCR|ePlaylistEntry::recNgrab ) ) );
	tmpService = ref;

	if (eSystemInfo::getInstance()->canShutdown() && ttype & ePlaylistEntry::doShutdown)
		after_event->setCurrent((void*)ePlaylistEntry::doShutdown);
	else if (ttype & ePlaylistEntry::doGoSleep)
		after_event->setCurrent((void*)ePlaylistEntry::doGoSleep);
	else
		after_event->setCurrent((void*)0);

	eString sname = getLeft( tmpService.descr, '/' );
	if ( sname )
		bSelectService->setText(sname);
	else
	{
		eService *service = eServiceInterface::getInstance()->addRef( tmpService );
		if (service)
		{
			bSelectService->setText( service->service_name );
			eServiceInterface::getInstance()->removeRef( tmpService );
		}
		else
			bSelectService->setText(_("unknown service"));
	}
	setMultipleCheckboxes( ttype );
}

void eTimerEditView::setMultipleCheckboxes( int type )
{
	if ( type & ePlaylistEntry::isRepeating )
	{
		multiple->setCheck(1);
		cMo->setCheck(type&ePlaylistEntry::Mo);
		cTue->setCheck(type&ePlaylistEntry::Tue);
		cWed->setCheck(type&ePlaylistEntry::Wed);
		cThu->setCheck(type&ePlaylistEntry::Thu);
		cFr->setCheck(type&ePlaylistEntry::Fr);
		cSa->setCheck(type&ePlaylistEntry::Sa);
		cSu->setCheck(type&ePlaylistEntry::Su);
	}
	else
	{
		multiple->setCheck(0);
		cMo->setCheck(0);
		cTue->setCheck(0);
		cWed->setCheck(0);
		cThu->setCheck(0);
		cFr->setCheck(0);
		cSa->setCheck(0);
		cSu->setCheck(0);
	}
}

void eTimerEditView::applyPressed()
{
	if ( pinCheck::getInstance()->pLockActive() && tmpService.isLocked() &&
			!pinCheck::getInstance()->checkPin(pinCheck::parental))
		return;

	EITEvent evt;
	time_t newEventBegin;
	int newEventDuration;
	int ttype = ( (int) type->getCurrent()->getKey() ) |
		ePlaylistEntry::stateWaiting;
	if ( multiple->isChecked() )
	{
		ttype |= ePlaylistEntry::isRepeating;
		if ( cMo->isChecked() )
			ttype |= ePlaylistEntry::Mo;
		if ( cTue->isChecked() )
			ttype |= ePlaylistEntry::Tue;
		if ( cWed->isChecked() )
			ttype |= ePlaylistEntry::Wed;
		if ( cThu->isChecked() )
			ttype |= ePlaylistEntry::Thu;
		if ( cFr->isChecked() )
			ttype |= ePlaylistEntry::Fr;
		if ( cSa->isChecked() )
			ttype |= ePlaylistEntry::Sa;
		if ( cSu->isChecked() )
			ttype |= ePlaylistEntry::Su;
	}
	else
	{
		ttype &= ~ePlaylistEntry::isRepeating;
		if ( cMo->isChecked() )
			ttype &= ~ePlaylistEntry::Mo;
		if ( cTue->isChecked() )
			ttype &= ~ePlaylistEntry::Tue;
		if ( cWed->isChecked() )
			ttype &= ~ePlaylistEntry::Wed;
		if ( cThu->isChecked() )
			ttype &= ~ePlaylistEntry::Thu;
		if ( cFr->isChecked() )
			ttype &= ~ePlaylistEntry::Fr;
		if ( cSa->isChecked() )
			ttype &= ~ePlaylistEntry::Sa;
		if ( cSu->isChecked() )
			ttype &= ~ePlaylistEntry::Su;
	}
	ttype |= (int)after_event->getCurrent()->getKey();
	if ( getData( newEventBegin, newEventDuration) )  // all is okay... we add the event..
	{
		// parse EPGCache to get event informations
		EITEvent *tmp = 0;
		if (!multiple->isChecked() && event_id)
		{
			tmp = eEPGCache::getInstance()->lookupEvent( (eServiceReferenceDVB&)tmpService, event_id );
			if (!tmp)
				tmp = eEPGCache::getInstance()->lookupEvent( (eServiceReferenceDVB&)tmpService, newEventBegin+newEventDuration / 2 );
		}
		if (tmp)
		{
			evt = *tmp;
			evt.descriptor.setAutoDelete(true);
			tmp->descriptor.setAutoDelete(false); // Problem Ptrlist....
			delete tmp;
		}
		else  // ohh.. not found...
		{
			evt.running_status = -1;
			evt.free_CA_mode = -1;
			evt.event_id = -1;
		}

		evt.start_time = newEventBegin;
		evt.duration = newEventDuration;

		eString sname = getLeft(tmpService.descr,'/');
		eString descr = event_name->getText();

		// On Multiple Events.. kill Episode Infos..
		unsigned int pos=0;
		if ( multiple->isChecked() && ( pos = descr.find(" - ") ) != eString::npos )
			descr.erase( pos, descr.length() - pos );

		if ( descr.length() )
			tmpService.descr='/'+descr;
		if ( sname.length() )
			tmpService.descr=sname+tmpService.descr;

		bool ret = !curEntry;
		if ( curEntry )  // remove old event from list...
		{
			// this is a fake call to addEventToTimerList..
			// this only checks if the new event can added ! (overlapp check only)
			if ( eTimerManager::getInstance()->addEventToTimerList( this, &tmpService, &evt, ttype, curEntry ) )
			{
				if ( curEntry->type & ePlaylistEntry::stateRunning )
				{
					time_t now = time(0)+eDVB::getInstance()->time_difference;

					time_t newEnd = (curEntry->type & ePlaylistEntry::isRepeating) ?
						getNextEventStartTime(curEntry->time_begin, curEntry->duration, curEntry->type) + evt.duration :
						curEntry->time_begin + evt.duration;

					int ret = eMessageBox::btYes;
					if ( newEnd < now )
					{  
						eString str=_("The new endtime is before now time!\n"
													"This stops the running timer(recording)\n");
						str+=_("Really update this event?");
						hide();
						ret = eMessageBox::ShowBox(
							str,
							_("Update event in timerlist"),
							eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion,
							eMessageBox::btNo);
						show();
					}
					if ( ret == eMessageBox::btYes )
					{
						if ( eTimerManager::getInstance()->updateRunningEvent(evt.duration,ttype&(ePlaylistEntry::doGoSleep|ePlaylistEntry::doShutdown)) )
						{
							hide();
							eMessageBox::ShowBox(
								_("New endtime are now adjusted"),
								_("Update event in timerlist"),
								eMessageBox::iconInfo|eMessageBox::btOK);
							show();
							close(0);
						}
						else
						{
							hide();
							eMessageBox::ShowBox(
								_("The event has already finished... you can not change endtime!"),
								_("Update event in timerlist"),
								eMessageBox::iconInfo|eMessageBox::btOK);
							show();
						}
					}
					return;
				}
				// now we can delete the old event without any problem :)
				ret = eTimerManager::getInstance()->removeEventFromTimerList( this, *curEntry, eTimerManager::update );
			}
		}
		// this now adds the event
		if ( ret && eTimerManager::getInstance()->addEventToTimerList( this, &tmpService, &evt, ttype ) )
			close(0);
	}
	else
	{
		hide();
		eMessageBox::ShowBox(_("Invalid begin or end time.!\nPlease check time and date"), _("Update event in timerlist"), eMessageBox::iconWarning|eMessageBox::btOK);
		show();
	}
}

void eTimerEditView::eventNameSelected()
{
	// If event name is "No description available", clear it here
	// so the user doesn't need to remove all characters in the textbox
	
	if(event_name->getText() == eString(_("No description available"))) // See eTimerEditView::fillInData
	{
		event_name->setText("");
	}
}

bool eTimerEditView::getData( time_t &bTime, int &duration )
{

	int clktype = 0;
	eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);

	beginTime.tm_isdst=-1;
	endTime.tm_isdst=-1;
	if ( multiple->isChecked() )
	{
		beginTime.tm_year = 70;  // 1.1.1970
		beginTime.tm_mon = 0;
		beginTime.tm_mday = 1;

		endTime.tm_year = 70;  // 1.1.1970
		endTime.tm_mon = 0;
		endTime.tm_mday = 1;
	}
	else
	{
		beginTime.tm_year = (int)byear->getCurrent()->getKey();
		beginTime.tm_mon = (int)bmonth->getCurrent()->getKey();
		beginTime.tm_mday = (int)bday->getCurrent()->getKey();

		endTime.tm_year = (int)eyear->getCurrent()->getKey();
		endTime.tm_mon = (int)emonth->getCurrent()->getKey();
		endTime.tm_mday = (int)eday->getCurrent()->getKey();
	}

	beginTime.tm_hour = btime->getNumber(0);
	beginTime.tm_min = btime->getNumber(1);
	beginTime.tm_sec = 0;
	if (clktype)
	{ // 12-hour clock
		if (beginTime.tm_hour == 12)
			beginTime.tm_hour = 0;
		if (bAmpm->getCurrent()->getKey())
			beginTime.tm_hour += 12;
	}

	endTime.tm_hour = etime->getNumber(0);
	endTime.tm_min = etime->getNumber(1);
	endTime.tm_sec = 0;
	if (clktype)
	{ // 12-hour clock
		if (endTime.tm_hour == 12)
			endTime.tm_hour = 0;
		if (eAmpm->getCurrent()->getKey())
			endTime.tm_hour += 12;
	}

	if ( multiple->isChecked() &&   // endTime after 0:00
			endTime.tm_hour*60+endTime.tm_min <
			beginTime.tm_hour*60+beginTime.tm_min )
	{
		endTime.tm_mday++;
	}

	bTime = mktime( &beginTime );
	time_t tmp = mktime( &endTime );
	duration = tmp - bTime;
	return duration > 0;
}

void eTimerEditView::updateDateTime( const tm& beginTime, const tm& endTime, int what )
{
        int clktype = 0;
        eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);

	if ( what & 1 )
	{
		updateDay( bday, beginTime.tm_year+1900, beginTime.tm_mon+1, beginTime.tm_mday );

		btime->setNumber( 1, beginTime.tm_min );
		if (clktype)
		{
			btime->setNumber(0, beginTime.tm_hour%12 ? beginTime.tm_hour%12 : 12 );
			bAmpm->setCurrent((void *) (beginTime.tm_hour < 12 ? 0 : 1));
		}
		else
			btime->setNumber( 0, beginTime.tm_hour );

		byear->setCurrent( (void*) beginTime.tm_year );
		bmonth->setCurrent( (void*) beginTime.tm_mon );
	}
	if ( what & 2 )
	{
		updateDay( eday, endTime.tm_year+1900, endTime.tm_mon+1, endTime.tm_mday );

		etime->setNumber( 1, endTime.tm_min );
		if (clktype)
		{
			etime->setNumber(0, endTime.tm_hour%12 ? endTime.tm_hour%12 : 12 );
			eAmpm->setCurrent((void *) (endTime.tm_hour < 12 ? 0 : 1));
		}
		else
			etime->setNumber( 0, endTime.tm_hour );

		eyear->setCurrent( (void*) endTime.tm_year );
		emonth->setCurrent( (void*) endTime.tm_mon );
	}
}

void eTimerEditView::multipleChanged( int i )
{
	if ( i )
	{
		byear->hide();
		eyear->hide();
		bmonth->hide();
		emonth->hide();
		bday->hide();
		eday->hide();
		lBegin->show();
		lEnd->show();
		cMo->show();
		cTue->show();
		cWed->show();
		cThu->show();
		cFr->show();
		cSa->show();
		cSu->show();
	}
	else
	{
		cMo->hide();
		cMo->setCheck(0);
		cTue->hide();
		cTue->setCheck(0);
		cWed->hide();
		cWed->setCheck(0);
		cThu->hide();
		cThu->setCheck(0);
		cFr->hide();
		cFr->setCheck(0);
		cSa->hide();
		cSa->setCheck(0);
		cSu->hide();
		cSu->setCheck(0);
		lBegin->hide();
		lEnd->hide();
		byear->show();
		eyear->show();
		bmonth->show();
		emonth->show();
		bday->show();
		eday->show();
		setFocus(multiple);
	}
}

void eTimerEditView::updateDay( eComboBox* dayCombo, int year, int month, int day )
{
	const char *dayStr[7] = { _("Sunday"), _("Monday"), _("Tuesday"), _("Wednesday"),
							 _("Thursday"), _("Friday"), _("Saturday") };
	dayCombo->clear();
	int wday = weekday( 1, month, year );
	int days = monthdays[ month-1 ];
	days += (days == 28 && __isleap( year ) );
	for ( int i = wday; i < wday+days; i++ )
		new eListBoxEntryText( *dayCombo, eString().sprintf("%s, %02d", dayStr[i%7], i-wday+1), (void*) (i-wday+1) );
	dayCombo->setCurrent( day>days ? 0 : (void*) day );
}

void eTimerEditView::comboBoxClosed( eComboBox *combo,  eListBoxEntryText* )
{
	if ( combo == bmonth || combo == byear )
		updateDay( bday, (int) byear->getCurrent()->getKey()+1900, (int) bmonth->getCurrent()->getKey()+1, (int) bday->getCurrent()->getKey() );
	else if ( combo == emonth || combo == eyear )
		updateDay( eday, (int) eyear->getCurrent()->getKey()+1900, (int) emonth->getCurrent()->getKey()+1, (int) eday->getCurrent()->getKey() );
}

void eTimerEditView::showServiceSelector()
{
	if ( !tmpService )  // Sleeptimer...
		return;
	eServiceSelector sel;
#ifndef DISABLE_LCD
	sel.setLCD(LCDTitle, LCDElement);
#endif
	hide();
	sel.getFirstBouquetServiceNum.connect( slot( *eZapMain::getInstance(), &eZapMain::getFirstBouquetServiceNum) );
	sel.getRoot.connect( slot( *eZapMain::getInstance(), &eZapMain::getRoot) );
	sel.setPath(eServiceReference(eServiceReference::idDVB,
				eServiceReference::flagDirectory|eServiceReference::shouldSort,
				-2, (1<<4)|(1<<1), 0xFFFFFFFF ),eServiceReference() );
	sel.setStyle(eServiceSelector::styleSingleColumn);

/*	if ( tmpService )
		sel.selectServiceRecursive( tmpService );*/

	const eServiceReference *ref = sel.choose(-1);

	if (ref)
	{
		if ( ref->data[0] == 4 ) // NVOD
		{
			hide();
			eMessageBox::ShowBox(_("Sorry, you can not add a time shifted service manually to the timer.\nPlease close the Timer and use the EPG of the service you wish to add!"), _("Information"), eMessageBox::iconInfo|eMessageBox::btOK);
			show();
		}
		else if (tmpService != *ref)
		{
			tmpService = *ref;
			if ( ref->descr.length() )
				bSelectService->setText(ref->descr);
			else
			{
				eService *service = eServiceInterface::getInstance()->addRef( tmpService );
				if ( service )
				{
					bSelectService->setText(service->service_name);
					eServiceInterface::getInstance()->removeRef( tmpService );
				}
				else
					bSelectService->setText(_("unknown"));
			}
		}
	}
	show();
	setFocus(bSelectService);
}

void eTimerEditView::scanEPGPressed()
{
	time_t newEventBegin;
	int newEventDuration;
	if ( getData( newEventBegin, newEventDuration ) )  // all is okay... we add the event..
	{
		if ( multiple->isChecked() )
		{
			int ttype = ePlaylistEntry::isRepeating;
			if ( cMo->isChecked() )
				ttype |= ePlaylistEntry::Mo;
			if ( cTue->isChecked() )
				ttype |= ePlaylistEntry::Tue;
			if ( cWed->isChecked() )
				ttype |= ePlaylistEntry::Wed;
			if ( cThu->isChecked() )
				ttype |= ePlaylistEntry::Thu;
			if ( cFr->isChecked() )
				ttype |= ePlaylistEntry::Fr;
			if ( cSa->isChecked() )
				ttype |= ePlaylistEntry::Sa;
			if ( cSu->isChecked() )
				ttype |= ePlaylistEntry::Su;
			newEventBegin = getNextEventStartTime( newEventBegin, newEventDuration, ttype );
		}
		eString title, descr;
		getEventDescrFromEPGCache( tmpService, newEventBegin+newEventDuration/2, event_id, title, descr);
		if ( title )
		{
			event_name->setText( title );
			return;
		}
	}
}

int eTimerEditView::eventHandler( const eWidgetEvent &event )
{
	switch ( event.type )
	{
		case eWidgetEvent::evtAction:
			if (event.action == &i_TimerEditActions->incBegTime && !event_name->inEditMode() )
				changeTime(-1);
			else if (event.action == &i_TimerEditActions->decBegTime && !event_name->inEditMode() )
				changeTime(-2);
			else if (event.action == &i_TimerEditActions->incEndTime && !event_name->inEditMode() )
				changeTime(+2);
			else if ( event.action == &i_TimerEditActions->decEndTime && !event_name->inEditMode() )
				changeTime(+1);
			else
				return eWindow::eventHandler( event );
			break;
		default:
			return eWindow::eventHandler( event );
	};
	return 1;
}

void eTimerEditView::changeTime( int dir )
{
	if ( dir < 0 && curEntry && curEntry->type&ePlaylistEntry::stateRunning )
  // don't let event start_time via volume +/- when the event ist currently running		
		return;
	time_t curBegin;
	int duration;
	getData( curBegin, duration );
	switch( dir )
	{
		case -2:  // dec begTime
			curBegin-=60;
		break;
		case -1:  // inc begTime
			curBegin+=60;
		break;
		case +1:  // dec duration
			duration-=60;
		break;
		case +2:  // inc duration
			duration+=60;
		break;
	}
	if ( dir > 0 )
	{
		curBegin+=duration;
		endTime = *localtime( &curBegin );
		updateDateTime( beginTime, endTime, 2 );
	}
	else
	{
		beginTime = *localtime( &curBegin );
		updateDateTime( beginTime, endTime, 1 );
	}
}
