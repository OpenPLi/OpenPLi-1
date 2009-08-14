#ifndef __lib_dvb_serviceplaylist_h
#define __lib_dvb_serviceplaylist_h

#include <lib/dvb/service.h>
#include <list>

struct ePlaylistEntry
{
	enum
	{
// PlaylistEntry types
		PlaylistEntry=1,       // normal PlaylistEntry (no Timerlist entry)
		SwitchTimerEntry=2,    // simple service switch timer
		RecTimerEntry=4,       // timer do recording
// Recording subtypes
		recDVR=8,              // timer do DVR recording
		recVCR=16,             // timer do VCR recording (LIRC) not used yet
		recNgrab=131072,       // timer do record via Ngrab Server
// Timer States
		stateWaiting=32,       // timer is waiting
		stateRunning=64,       // timer is running
		statePaused=128,       // timer is paused
		stateFinished=256,     // timer is finished
		stateError=512,        // timer has error state(s)
// Timer Error states
		errorNoSpaceLeft=1024, // HDD no space Left ( recDVR )
		errorUserAborted=2048, // User Action aborts this event
		errorZapFailed=4096,   // Zap to service failed
		errorOutdated=8192,    // Outdated event
													 // box was switched off during the event
//  advanced entry propertys
		boundFile=16384,        // Playlistentry have an bounded file
		isSmartTimer=32768,     // this is a smart timer (EIT related) not uses Yet
		isRepeating=262144,     // this timer is repeating
		doFinishOnly=65536,     // Finish an running event/action
														// this events are automatically removed
														// from the timerlist after finish
		doShutdown=67108864,    // timer shutdown the box
		doGoSleep=134217728,    // timer set box to standby
//  Repeated Timer Days
		Su=524288, Mo=1048576, Tue=2097152,
		Wed=4194304, Thu=8388608, Fr=16777216, Sa=33554432
	};
	eServicePath services;
	eServiceReference service;
	union
	{
		int current_position;
		int event_id;
		int last_activation;
	};
	time_t time_begin;
	int duration,
			type;  // event type and current state of timer events...
	ePlaylistEntry(const eServiceReference &ref)
		:services(ref), service(services.path.back()), current_position(-1), time_begin(-1), duration(-1), type(PlaylistEntry)
	{ }
	ePlaylistEntry(const eServiceReference &ref, int current_position)
		:services(ref), service(services.path.back()), current_position(current_position), time_begin(-1), duration(-1), type(PlaylistEntry)
	{ }
	ePlaylistEntry(const eServiceReference &ref, int time_begin, int duration, int event_id=-1, int type=SwitchTimerEntry )
		:services(ref), service(services.path.back()), event_id(event_id), time_begin(time_begin), duration(duration), type(type)
	{ }
	ePlaylistEntry(const eServicePath &p)
		:services(p), service(services.path.back()), current_position(-1), time_begin(-1), duration(-1), type(PlaylistEntry)
	{ }
	const eServicePath &getPath() const { return services; }
	operator eServiceReference &() { return service; }
	operator const eServiceReference &() const { return service; }
	operator const eServicePath &() const { return services; }

	bool operator == (const eServiceReference &r) const
	{
		return r == service;
	}
	bool operator == (const ePlaylistEntry &e) const
	{
		if ( type == PlaylistEntry )
			return e.service == service;
		else if ( type & isRepeating )
			return (e.service == service) 
				&& (e.time_begin == time_begin) 
				&& ((e.type & (Su|Mo|Tue|Wed|Thu|Fr|Sa)) == (type & (Su|Mo|Tue|Wed|Thu|Fr|Sa)));
		else
			return e.service == service && e.time_begin == time_begin;
	}
	bool operator < (const ePlaylistEntry &e) const
	{
		return ( service.descr.compare( e.service.descr ) < 0 ) ;
	}
};

class ePlaylist: public eService
{
	eString filename;
	std::list<ePlaylistEntry> list;
	__u8 changed;
	
	eString filepath;
	bool lockActive;
	int lockCount;
public:
	int load(const char *filename);
	int save(const char *filename=0);

	std::list<ePlaylistEntry>::iterator current;

	void clear();
	const std::list<ePlaylistEntry>& getConstList() const { return list; }
	std::list<ePlaylistEntry>& getList() { changed=1;return list; }

	int deleteService(std::list<ePlaylistEntry>::iterator it);
	int moveService(std::list<ePlaylistEntry>::iterator it, std::list<ePlaylistEntry>::iterator before);

	ePlaylist();
	~ePlaylist();

	void lockPlaylist();
	void unlockPlaylist();
};

class eServicePlaylistHandler: public eServiceHandler
{
	static eServicePlaylistHandler *instance; 
	void addFile(void *node, const eString &filename);

	std::multimap<eServiceReference,eServiceReference> playlists;
	std::set<int> usedUniqueIDs;
public:
	enum { ID = 0x1001 } ;
	static eServicePlaylistHandler *getInstance() { return instance; }

	eService *createService(const eServiceReference &node);
	
	eServicePlaylistHandler();
	~eServicePlaylistHandler();

		// service list functions
	void enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback);
	void leaveDirectory(const eServiceReference &dir);

	eService *addRef(const eServiceReference &service);
	void removeRef(const eServiceReference &service);

		// playlist functions
	eServiceReference newPlaylist(const eServiceReference &parent=eServiceReference(), const eServiceReference &serviceref=eServiceReference());
	int addNum( int );
	void removePlaylist(const eServiceReference &service);
};

#endif
