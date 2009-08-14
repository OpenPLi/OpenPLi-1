#ifndef __epgstore_h_
#define __epgstore_h_

#include <linux/types.h>
#include <vector>
#include <list>
#include <sqlite3.h>
#include "eventdata.h"

#define HILO(x) (x##_hi << 8 | x##_lo)

#define uniqueKeyVector std::vector<uniqueEPGKey>

class timeMapPtr;
class eventDataPtr;
class epgAtTime;


class eEPGStore
{
public:
	enum { MEM_STORE = 0, SQLITE_STORE, NR_OF_STORES };
	enum StorageCheckEnum { STORAGE_OK, STORAGE_NO_SPACE, STORAGE_NO_DIR };
	
	virtual ~eEPGStore() {};
	virtual void processEpgRecord( uniqueEPGKey epgKey, int source, __u8 *eitData ) = 0;
	virtual void cleanup( uniqueKeyVector *vec=0 ) = 0;
	virtual void flushEPG(const uniqueEPGKey& s=uniqueEPGKey(), const int event_id=0 ) = 0;
	virtual timeMapPtr getTimeMapPtr( const eServiceReferenceDVB&, time_t from=0, time_t to=0, int limit=0 ) = 0;
	virtual void freeTimeMap( timeMap* ) = 0;
	virtual EITEvent *lookupEvent( const eServiceReferenceDVB &service, int event_id ) = 0;
	virtual EITEvent *lookupEvent( const eServiceReferenceDVB &service, time_t t=0 ) = 0;
	virtual const eventData *lookupEventData( const eServiceReferenceDVB &service, time_t t=0 );
	virtual eventDataPtr getEventDataPtr( const eServiceReferenceDVB&, time_t t=0 ) = 0;
	virtual epgAtTime getEpgAtTime( time_t atTime=0 ) = 0;
	virtual bool hasEPGData( const uniqueEPGKey& ) = 0;
	virtual void organise() {}
	virtual int getCleanloopInterval() { return 60; }
	static bool available() { return 1; }
	
	static eEPGStore* createEPGStore();
	static void getEITData( const __u8 *eitData, __u16* eventId, int* startTime = 0, int* stopTime = 0, int* eitDataLength = 0 );
	static void createEventId( __u8 *eitData );
};

class eEPGMemStore : public eEPGStore
{
	eventCache eventDB;
	pthread_mutex_t cacheLock;
	eString dbDir;

	inline void lock()
	{
		pthread_mutex_lock( &cacheLock );
	}

	inline void unlock()
	{
		pthread_mutex_unlock( &cacheLock );
	}

	const eventData *searchByTime( const eServiceReferenceDVB &service, time_t t=0 );
	void load();
	void save();
public:
	eEPGMemStore();
	~eEPGMemStore();

	static eEPGStore::StorageCheckEnum checkStorage(eString directory);
	void processEpgRecord( uniqueEPGKey epgKey, int source, __u8 *eitData );
	void cleanup( uniqueKeyVector *vec=0 );
	void flushEPG( const uniqueEPGKey& s=uniqueEPGKey(), const int event_id=0 );
	timeMapPtr getTimeMapPtr( const eServiceReferenceDVB&, time_t from=0, time_t to=0, int limit=0 );
	void freeTimeMap( timeMap* );
	EITEvent *lookupEvent( const eServiceReferenceDVB &service, int event_id );
	EITEvent *lookupEvent( const eServiceReferenceDVB &service, time_t t=0 );
	const eventData *lookupEventData( const eServiceReferenceDVB &service, time_t t=0 );
	eventDataPtr getEventDataPtr( const eServiceReferenceDVB&, time_t t=0 );
	epgAtTime getEpgAtTime( time_t atTime=0 );
	bool hasEPGData( const uniqueEPGKey& );
	static inline eString getDefaultStorageDir() {return (eString)"/media/hdd";};
};


// For the moment the database name and version is over here:
#define DBNAME "epg.db"
#define DBVERSION "2.1g"

// Every instance of eEPGSqlBase is associated with one database connection.
class eEPGSqlBase
{
protected:
	pthread_mutex_t dbLock;
	sqlite3* dbHandle;
	eString dbDir;

	inline bool isConnected() { return ( dbHandle != 0 ); }
	const bool openDatabase( const char*, bool deleteExistingDb=0 );
	void closeDatabase();
	const bool isTable( const eString );
	static const eString key2String( const uniqueEPGKey& );
	static const uniqueEPGKey string2Key( eString );
	
	inline void lock()
	{
		pthread_mutex_lock( &dbLock );
	}

	inline void unlock()
	{
		pthread_mutex_unlock( &dbLock );
	}
public:
	eEPGSqlBase();
	~eEPGSqlBase();

	class queriedValue
	{
	public:
		int integer;
		eString string;
		queriedValue() { };
		queriedValue( int i ) : integer( i ) { };
		queriedValue( eString s ) : string( s ) { };
	};
	
	int addEpg( uniqueEPGKey epgKey, int eventId, int source, int startTime, int stopTime, __u8 *eitData, int eitDataLength);
	int updateEpg( uniqueEPGKey epgKey, int eventId, int source, int startTime, int stopTime, __u8 *eitData, int eitDataLength, const eString& clause, queriedValue* queryResult = 0 );
	const bool runSqlCmd( const eString, queriedValue* queryResult = 0 );
	static inline eString getDefaultStorageDir() {return (eString)"/media/hdd";};
};


class eEPGSqlStore : public eEPGStore, public eEPGSqlBase, public Object
{
	pthread_mutex_t bufferLock;
	bool bufferReaderActive, serviceLoaderActive;

	void updateService( const eServiceReferenceDVB &, bool );
	void removeService( const eServiceReferenceDVB & );
	void loadServices();
	static void *loadServicesThread( void* );
	eventData *queryEvent( const eServiceReferenceDVB&, const eString& sqlCmd );

	inline void bufLock()
	{
		pthread_mutex_lock( &bufferLock );
	}

	inline void bufUnlock()
	{
		pthread_mutex_unlock( &bufferLock );
	}

	struct loadService
	{
		eEPGSqlStore *parent;

		loadService( eEPGSqlStore* );
		~loadService() { };
		void operator()( const eServiceReferenceDVB& );
	};
	
	struct bufferEntry_t
	{
		uniqueEPGKey epgKey;
		__u16 eventId;
		__u8 source;
		int startTime;
		int stopTime;
		int eitDataLength;
		__u8* eitData;
	};
	
	typedef std::list<bufferEntry_t> buffer_t;
	buffer_t activeBuffer, waitingBuffer;
	void startBufferReader();
	static void *bufferReader( void* arg );
	void flushBuffer( buffer_t& );
	void EPGReaderFinished(int source);
	void switchedService( const eServiceReferenceDVB& ref, int err );

public:
	eEPGSqlStore();
	~eEPGSqlStore();
	
	static eEPGStore::StorageCheckEnum checkStorage(eString directory);
	void processEpgRecord( uniqueEPGKey epgKey, int source, __u8 *eitData );
	void cleanup( uniqueKeyVector *vec=0 );
	void flushEPG( const uniqueEPGKey& s=uniqueEPGKey(), const int event_id=0 );
	timeMapPtr getTimeMapPtr( const eServiceReferenceDVB&, time_t from=0, time_t to=0, int limit=0 );
	void freeTimeMap( timeMap* );
	EITEvent *lookupEvent(const eServiceReferenceDVB &service, int event_id );
	EITEvent *lookupEvent(const eServiceReferenceDVB &service, time_t t=0 );
	eventDataPtr getEventDataPtr( const eServiceReferenceDVB&, time_t t=0 );
	epgAtTime getEpgAtTime( time_t atTime=0 );
	bool hasEPGData( const uniqueEPGKey& );
	
	// From eEPGStore
	void organise();
	int getCleanloopInterval() { return 3600; }
};


class timeMapPtr
{
	eEPGStore* store;
	timeMap* localMap;
public:
	timeMapPtr()  : store(0), localMap(0) { }
	timeMapPtr( eEPGStore* store, timeMap* localMap )  : store( store ), localMap( localMap ) { }
	~timeMapPtr() {
		if ( store )
			store->freeTimeMap( localMap );
	}

	timeMap* operator -> () { return localMap; }
	operator bool() const {	return ( localMap != 0 ); }
};


class eventDataPtr
{
	eventData* localData;
public:
	eventDataPtr()  : localData(0) { }
	eventDataPtr( eventData* localData ) : localData( localData ) { }
	~eventDataPtr() { delete localData; }

	eventData* operator -> () { return localData; }
	operator bool() const {	return ( localData != 0 ); }
};


class epgAtTime
{
	eEPGStore* store;
	serviceEpgInfo* epgInfo;
	time_t atTime;
public:
	epgAtTime() : store(0), epgInfo(0), atTime(0) { }
	epgAtTime( serviceEpgInfo* epgInfo ) : store(0), epgInfo(epgInfo), atTime(0) { }
	epgAtTime( eEPGStore* store, time_t atTime ) : store( store ), epgInfo(0), atTime( atTime ) { }
	~epgAtTime()
	{
		if (epgInfo)
			delete epgInfo;
	}
	const eventData* channelEpg( eServiceReferenceDVB& ref );
};

#endif
