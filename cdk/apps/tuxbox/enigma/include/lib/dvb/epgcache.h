#ifndef __epgcache_h_
#define __epgcache_h_

#include <ext/hash_set>

#include <errno.h>

#ifdef ENABLE_MHW_EPG
#include "lowlevel/mhw.h"
#endif
#include "epgstore.h"
#include <lib/base/ebase.h>
#include <lib/base/thread.h>
#include <lib/base/message.h>

#define HILO(x) (x##_hi << 8 | x##_lo)


class eEPGCache;

class eSchedule: public eSection
{
	friend class eEPGCache;
	inline int sectionRead(__u8 *data);
	inline void sectionFinish(int);
	eSchedule()
	{}
	inline void start()
	{ // 0x50 .. 0x5F
		setFilter(0x12, 0x50, -1, -1, SECREAD_CRC|SECREAD_NOTIMEOUT, 0xF0);
	}
};

class eScheduleOther: public eSection
{
	friend class eEPGCache;
	inline int sectionRead(__u8 *data);
	inline void sectionFinish(int);
	eScheduleOther()
	{}
	inline void start()
	{ // 0x60 .. 0x6F
		setFilter(0x12, 0x60, -1, -1, SECREAD_CRC|SECREAD_NOTIMEOUT, 0xF0);
	}
};

#ifdef ENABLE_MHW_EPG
class eScheduleMhw: public eSection
{
	friend class eEPGCache;
	std::vector<mhw_channel_name_t> channels;
	std::map<__u8, mhw_theme_name_t> themes;
	std::map<__u32, mhw_title_t> titles;
	std::map<__u32, __u32> program_ids;
	time_t tnew_summary_read;
	time_t tnew_title_read;
	
	void cleanup();
	__u8 *delimitName( __u8 *in, __u8 *out, int len_in );
	void eScheduleMhw::timeMHW2DVB( u_char hours, u_char minutes, u_char *return_time);
	void eScheduleMhw::timeMHW2DVB( int minutes, u_char *return_time);
	void eScheduleMhw::timeMHW2DVB( u_char day, u_char hours, u_char minutes, u_char *return_time);
	void eScheduleMhw::storeTitle(std::map<__u32, mhw_title_t>::iterator itTitle, 
		eString sumText, __u8 *data);
	int sectionRead(__u8 *data);
	void sectionFinish(int);
	int start()
	{
		return setFilter( 0xD3, 0x91, -1, -1, SECREAD_NOTIMEOUT, 0xFF );
	}
	eScheduleMhw()	{}
};
#endif

#ifdef ENABLE_DISH_EPG
class eDishEEPG: public eSection
{
	friend class eEPGCache;
	int sectionRead(__u8 *data);
	void sectionFinish(int);
	void storeTitle(eit_t *disheit, eit_event_struct *dishevent, eString &name, eString &description);
	void start(const char *dmxdev = DEMUX0);
public:
	eDishEEPG();
	~eDishEEPG() {}
};

class eBevEEPG: public eSection
{
	friend class eEPGCache;
	int sectionRead(__u8 *data);
	void sectionFinish(int);
	void storeTitle(eit_t *disheit, eit_event_struct *dishevent, eString &name, eString &description);
	void start(const char *dmxdev = DEMUX0);
public:
	eBevEEPG();
	~eBevEEPG() {}
};
#endif

class eNowNext: public eSection
{
	friend class eEPGCache;
	inline int sectionRead(__u8 *data);
	inline void sectionFinish(int);
	eNowNext()
	{}
	inline void start()
	{  // 0x4E, 0x4F
		setFilter(0x12, 0x4E, -1, -1, SECREAD_CRC|SECREAD_NOTIMEOUT, 0xFE);
	}
};

#ifdef ENABLE_PRIVATE_EPG
class ePrivateContent: public eSection
{
	friend class eEPGCache;
	int sectionRead(__u8 *data);
	std::set<__u8> seenSections;
	ePrivateContent()
	{}
	void stop()
	{
		if ( pid )
		{
			abort();
			pid = 0;
		}
	}
	void start( int pid )
	{
		if ( pid != this->pid )
			start_filter(pid,-1);
	}
	void restart()
	{
		if ( pid )
			start_filter(pid);
	}
	void start_filter(int pid, int version=-1)
	{
		eDebug("[EPGC] start private content filter pid %04x, version %d", pid, version);
		seenSections.clear();
		setFilter(pid, 0xA0, -1, version, SECREAD_CRC|SECREAD_NOTIMEOUT, 0xFF);
	}
};
#endif

class eEPGCache: public eMainloop, private eThread, public Object
{
public:
	enum 	{
				NOWNEXT,
				SCHEDULE,
				SCHEDULE_OTHER,
				DISH_EEPG,
				SCHEDULE_MHW,
				BEV_EEPG,
				};
#ifdef ENABLE_MHW_EPG
	friend class eScheduleMhw;
#endif
#ifdef ENABLE_DISH_EPG
	friend class eDishEEPG;
	friend class eBevEEPG;
#endif
	friend class eSchedule;
	friend class eScheduleOther;
	friend class eNowNext;
#ifdef ENABLE_PRIVATE_EPG
	friend class ePrivateContent;
#endif
	struct Message
	{
		enum
		{
			flush,
			enterService,
			leaveService,
			pause,
			restart,
			updated,
			isavail,
			quit,
			timeChanged,
			content_pid,
			save,
			load,
			reloadStore,
			forceEpgScan,
			organise
		};
		int type;
		uniqueEPGKey service;
		union {
			int err;
			int pid;
			time_t time;
			bool avail;
		};
		Message()
			:type(0), time(0) {}
		Message(int type)
			:type(type) {}
		Message(int type, int pid)
			:type(type), pid(pid) {}
		Message(int type, bool b)
			:type(type), avail(b) {}
		Message(int type, const eServiceReferenceDVB& service, int err=0)
			:type(type), service(service), err(err) {}
		Message(int type, time_t time)
			:type(type), time(time) {}
	};
	eFixedMessagePump<Message> messages;
private:
	// needed for caching current service until the thread has started
	// (valid transponder time received)
	eServiceReferenceDVB cached_service;
	int cached_err;

	static pthread_mutex_t cache_lock;	// Is used to lock updated map (temp)
	uniqueEPGKey current_service;
	int paused;
	int isLoading;

	int state;
	__u8 isRunning, firstStart, haveData;
	int sectionRead(__u8 *data, int source);
	static eEPGCache *instance;

	updateMap serviceLastUpdated;
	tmpMap temp;
	nvodMap NVOD;
#ifdef ENABLE_DISH_EPG
	tidMap seenSections[4], calcedSections[4];
#else
	tidMap seenSections[3], calcedSections[3];
#endif
	eSchedule scheduleReader;
	eScheduleOther scheduleOtherReader;
#ifdef ENABLE_MHW_EPG
	eScheduleMhw scheduleMhwReader;
#endif
#ifdef ENABLE_DISH_EPG
	eDishEEPG dishEEPGReader;
	eBevEEPG bevEEPGReader;
#endif
	eNowNext nownextReader;
#ifdef ENABLE_PRIVATE_EPG
	eString dbDir;
	contentMaps content_time_tables;
	ePrivateContent contentReader;
	void setContentPid(int pid);
#endif
	eTimer CleanTimer;
	eTimer zapTimer;
	eTimer abortTimer;
	eTimer organiseTimer;
	void setOrganiseTimer();
	void organiseEvent();
	void organise();
	eEPGStore *epgStore;
	int epgHours;
	bool finishEPG(int source);
	void abortNonAvail();
	void flushEPG(const uniqueEPGKey & s=uniqueEPGKey());
	void startEPG();

	void changedService(const uniqueEPGKey &, int);
	void abortEPG();

	// called from other thread context !!
	void enterService(const eServiceReferenceDVB &, int);
	void leaveService(const eServiceReferenceDVB &);

	void cleanLoop();
	void pauseEPG();
	void restartEPG();
	void reloadStore();
	void forceEpgScan();
	void thread();
	void gotMessage(const Message &message);
	void timeUpdated();
#ifdef ENABLE_PRIVATE_EPG
	void loadPrivateEPG();
	void savePrivateEPG();
#endif
	void init_eEPGCache();
public:
	eEPGCache();
	~eEPGCache();
	static eEPGCache *getInstance() { return instance; }

	inline void Lock();
	inline void Unlock();

	tmpMap* getUpdatedMap() { return &temp; }
	timeMapPtr getTimeMapPtr(const eServiceReferenceDVB &service, time_t from=0, time_t to=0, int limit=0)
	{
		if ( epgStore )
			return epgStore->getTimeMapPtr( service, from, to, limit );
		else
			return timeMapPtr();
	}

	eventDataPtr getEventDataPtr( const eServiceReferenceDVB& ref, time_t t )
	{
		if ( epgStore )
			return epgStore->getEventDataPtr( ref, t );
		else
			return eventDataPtr();
	}

	epgAtTime getEpgAtTime(time_t atTime=0)
	{
		if ( epgStore )
			return epgStore->getEpgAtTime( atTime );
		else
			return epgAtTime();
	}

	const std::list<NVODReferenceEntry>* getNVODRefList(const eServiceReferenceDVB &service);

	EITEvent *lookupEvent(const eServiceReferenceDVB &service, int event_id );
	EITEvent *lookupEvent(const eServiceReferenceDVB &service, time_t=0 );

	Signal1<void, bool> EPGAvail;
	Signal0<void> EPGUpdated;
	Signal1<void, int> EPGReaderFinished;
	Signal0<void> organiseRequest;
};

inline const std::list<NVODReferenceEntry>* eEPGCache::getNVODRefList(const eServiceReferenceDVB &service)
{
	nvodMap::iterator It = NVOD.find( service );
	if ( It != NVOD.end() && It->second.size() )
		return &(It->second);
	else
		return 0;
}

inline int eNowNext::sectionRead(__u8 *data)
{
	return eEPGCache::getInstance()->sectionRead(data, eEPGCache::NOWNEXT);
}

inline int eSchedule::sectionRead(__u8 *data)
{
	return eEPGCache::getInstance()->sectionRead(data, eEPGCache::SCHEDULE);
}

inline int eScheduleOther::sectionRead(__u8 *data)
{
	return eEPGCache::getInstance()->sectionRead(data, eEPGCache::SCHEDULE_OTHER);
}

inline void eSchedule::sectionFinish(int err)
{
	eEPGCache *e = eEPGCache::getInstance();
	if ( (e->isRunning & (1 << eEPGCache::SCHEDULE)) && (err == -ETIMEDOUT || err == -ECANCELED ) )
	{
		eDebug("[EPGC] stop schedule");
		e->isRunning &= ~(1 << eEPGCache::SCHEDULE);
		if (e->haveData)
			e->finishEPG(eEPGCache::SCHEDULE);
	}
}

inline void eScheduleOther::sectionFinish(int err)
{
	eEPGCache *e = eEPGCache::getInstance();
	if ( (e->isRunning & (1 << eEPGCache::SCHEDULE_OTHER)) && (err == -ETIMEDOUT || err == -ECANCELED ) )
	{
		eDebug("[EPGC] stop schedule other");
		e->isRunning &= ~(1 << eEPGCache::SCHEDULE_OTHER);
		if (e->haveData)
			e->finishEPG(eEPGCache::SCHEDULE_OTHER);
	}
}

inline void eNowNext::sectionFinish(int err)
{
	eEPGCache *e = eEPGCache::getInstance();
	if ( (e->isRunning & (1 << eEPGCache::NOWNEXT)) && (err == -ETIMEDOUT || err == -ECANCELED ) )
	{
		eDebug("[EPGC] stop nownext");
		e->isRunning &= ~(1 << eEPGCache::NOWNEXT);
		if (e->haveData)
			e->finishEPG(eEPGCache::NOWNEXT);
	}
}

inline void eEPGCache::Lock()
{
	pthread_mutex_lock(&cache_lock);
}

inline void eEPGCache::Unlock()
{
	pthread_mutex_unlock(&cache_lock);
}

#endif
