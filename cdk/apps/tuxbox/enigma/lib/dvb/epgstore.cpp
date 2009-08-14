#include <dirent.h>
#include <libmd5sum.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/vfs.h>
#include <lib/dvb/epgstore.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/lowlevel/eit.h>


const eventData* eEPGStore::lookupEventData( const eServiceReferenceDVB &service, time_t t )
{
	return 0;
}


eEPGStore* eEPGStore::createEPGStore()
{
	int storeType;
	
	if ( eConfig::getInstance()->getKey("/enigma/epgStore", storeType ) ) 
	{
		// If key doesn't exist create a MEM_STORE key by default
		storeType = MEM_STORE ;
		eConfig::getInstance()->setKey("/enigma/epgStore", storeType );
	}

	if ( storeType == SQLITE_STORE )
	{
		return new eEPGSqlStore;
	}
	else
	{
		return new eEPGMemStore;
	}
}


void eEPGStore::getEITData( const __u8 *eitData, __u16* ptrEventId, int* ptrStartTime, int* ptrStopTime, int* ptrEitDataLength )
{
	eit_event_t* eitEvent = (eit_event_t*)eitData;
	if ( ptrEventId )
		*ptrEventId = HILO( eitEvent->event_id );
	
	if ( ptrStartTime )
		*ptrStartTime = parseDVBtime(
				eitEvent->start_time_1,
				eitEvent->start_time_2,
				eitEvent->start_time_3,
				eitEvent->start_time_4,
				eitEvent->start_time_5 );
	
	if ( ptrStopTime )
		*ptrStopTime = *ptrStartTime + fromBCD( eitEvent->duration_1 ) * 3600 +
				fromBCD( eitEvent->duration_2 ) * 60 +
				fromBCD( eitEvent->duration_3 );
	
	if ( ptrEitDataLength )
		*ptrEitDataLength = HILO( eitEvent->descriptors_loop_length ) + EIT_LOOP_SIZE;
}


void eEPGStore::createEventId( __u8 *eitData )
{
	eit_event_t* eitEvent = (eit_event_t*)eitData;
	time_t startTime = parseDVBtime(
				eitEvent->start_time_1,
				eitEvent->start_time_2,
				eitEvent->start_time_3,
				eitEvent->start_time_4,
				eitEvent->start_time_5 );

	int eventId = ( startTime/60 ) & 0xffff;

	eitEvent->event_id_hi = (eventId >> 8) & 0xff;
	eitEvent->event_id_lo = eventId & 0xff;
}


eEPGMemStore::eEPGMemStore()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
	pthread_mutex_init( &cacheLock, &attr );
	pthread_mutexattr_destroy( &attr );
	
	dbDir = getDefaultStorageDir();
	eConfig::getInstance()->getKey("/enigma/epgMemStoreDir", dbDir);
	eDebug("[EPGM] Using EPG directory %s", dbDir.c_str());
	
	load();
}


eEPGMemStore::~eEPGMemStore()
{
	save();
	flushEPG();
}


eEPGStore::StorageCheckEnum eEPGMemStore::checkStorage(eString directory)
{
	unsigned int dbSize = 0;
	struct stat s;
	struct statfs sfs;
	
	eDebug("eEPGMemStore::checkStorage directory=%s", directory.c_str());

	if(!stat((directory+"/"+DBNAME).c_str(), &s))
	{
		dbSize = s.st_size;
	}
	
	if(statfs(directory.c_str(), &sfs))
	{
		return STORAGE_NO_DIR;
	}
	
	if(((unsigned long long)sfs.f_bsize * (unsigned long long)sfs.f_bfree 
		+ (unsigned long long)dbSize) < (unsigned long long)(5*(1<<20)))
	{
		return STORAGE_NO_SPACE;
	}
	
	return STORAGE_OK;
}


void eEPGMemStore::processEpgRecord( uniqueEPGKey epgKey, int source, __u8 *eitData )
{
	singleLock l( cacheLock );
	eit_event_struct* eit_event = (eit_event_struct*) eitData;
	__u16 event_id;
	int TM, TM2, eit_event_size;
	getEITData( eitData, &event_id, &TM, &TM2, &eit_event_size );
	std::pair<eventMap,timeMap> &servicemap = eventDB[epgKey];
	eventMap::iterator prevEventIt = servicemap.first.end();
	timeMap::iterator prevTimeIt = servicemap.second.end();

	eventData *evt = 0;
	int ev_erase_count = 0;
	int tm_erase_count = 0;

	if ( event_id == 0 )
	{
		createEventId( eitData );
		getEITData( eitData, &event_id );
	}

// search in eventmap
	eventMap::iterator ev_it =
		servicemap.first.find(event_id);

	// entry with this event_id is already exist ?
	if ( ev_it != servicemap.first.end() )
	{
		if ( source > ev_it->second->type )  // update needed ?
			return; // when not.. the skip this entry
// search this event in timemap
		timeMap::iterator tm_it_tmp =
			servicemap.second.find(ev_it->second->getStartTime());
		if ( tm_it_tmp != servicemap.second.end() )
		{
			if ( tm_it_tmp->first == TM ) // correct eventData
			{
							// exempt memory
				delete ev_it->second;
				evt = new eventData(eit_event, eit_event_size, source);
				ev_it->second=evt;
				tm_it_tmp->second=evt;
				return;
			}
			else
			{
				tm_erase_count++;
				// delete the found record from timemap
				servicemap.second.erase(tm_it_tmp);
				prevTimeIt=servicemap.second.end();
			}
		}
	}

// search in timemap, for check of a case if new time has coincided with time of other event
// or event was is not found in eventmap
	timeMap::iterator tm_it =
		servicemap.second.find(TM);

	if ( tm_it != servicemap.second.end() )
	{
		if ( source > tm_it->second->type && tm_erase_count == 0 ) // update needed ?
			return; // when not.. the skip this entry

// search this time in eventmap
		eventMap::iterator ev_it_tmp =
			servicemap.first.find(tm_it->second->getEventID());

		if ( ev_it_tmp != servicemap.first.end() )
		{
			ev_erase_count++;
			// delete the found record from eventmap
			servicemap.first.erase(ev_it_tmp);
			prevEventIt=servicemap.first.end();
		}
	}

	evt = new eventData(eit_event, eit_event_size, source);
	if (ev_erase_count > 0 && tm_erase_count > 0) // 2 different pairs have been removed
	{
		// exempt memory
		delete ev_it->second;
		delete tm_it->second;
		ev_it->second=evt;
		tm_it->second=evt;
	}
	else if (ev_erase_count == 0 && tm_erase_count > 0)
	{
		// exempt memory
		delete ev_it->second;
		tm_it=prevTimeIt=servicemap.second.insert( prevTimeIt, std::pair<const time_t, eventData*>( TM, evt ) );
		ev_it->second=evt;
	}
	else if (ev_erase_count > 0 && tm_erase_count == 0)
	{
		// exempt memory
		delete tm_it->second;
		ev_it=prevEventIt=servicemap.first.insert( prevEventIt, std::pair<const __u16, eventData*>( event_id, evt) );
		tm_it->second=evt;
	}
	else // added new eventData
	{
		prevEventIt=servicemap.first.insert( prevEventIt, std::pair<const __u16, eventData*>( event_id, evt) );
		prevTimeIt=servicemap.second.insert( prevTimeIt, std::pair<const time_t, eventData*>( TM, evt ) );
	}
}


void eEPGMemStore::cleanup( uniqueKeyVector *vec )
{
	time_t now = time(0)+eDVB::getInstance()->time_difference;

	for (eventCache::iterator DBIt = eventDB.begin(); DBIt != eventDB.end(); DBIt++)
	{
		bool updated=false;
		for (timeMap::iterator It = DBIt->second.second.begin(); It != DBIt->second.second.end() && It->first < now;)
		{
			if ( now > (It->first+It->second->getDuration()) )  // outdated normal entry (nvod references to)
			{
				// remove entry from eventMap
				eventMap::iterator b(DBIt->second.first.find(It->second->getEventID()));
				if ( b != DBIt->second.first.end() )
				{
					// release Heap Memory for this entry   (new ....)
					DBIt->second.first.erase(b);
				}

				// remove entry from timeMap
				delete It->second;
				updated = true;
				DBIt->second.second.erase(It++);
			}
			else
				++It;
		}
		if ( updated && vec )
			vec->push_back( DBIt->first );
	}
	//eDebug("[EPGM] %i bytes for cache used", eventData::CacheSize);
}


void eEPGMemStore::flushEPG( const uniqueEPGKey& s, const int event_id )
{
	singleLock l( cacheLock );
	if ( s )
	{
		// Cache of one specific service is deleted
		eventCache::iterator serviceIt = eventDB.find( s );
		if ( serviceIt != eventDB.end() )
		{
			if ( event_id )
			// Clear one event
			{
				eventMap::iterator evIt = serviceIt->second.first.find( event_id );
				if ( evIt != serviceIt->second.first.end() )
				{
					for ( timeMap::iterator tmIt = serviceIt->second.second.begin(); tmIt != serviceIt->second.second.end(); tmIt++ )
					{
						if ( tmIt->second == evIt->second )
						{
							serviceIt->second.second.erase( tmIt );
							break;
						}
					}
					delete evIt->second;
					serviceIt->second.first.erase( evIt );
				}
			}
			else
			// Clear one service
			{
				for (eventMap::iterator evIt = serviceIt->second.first.begin(); evIt != serviceIt->second.first.end(); evIt++)
					delete evIt->second;
			
				serviceIt->second.first.clear();
				serviceIt->second.second.clear();
				eventDB.erase( serviceIt );
			}
		}
	}
	else
	{
		// Clear whole cache
		for (eventCache::iterator serviceIt = eventDB.begin(); serviceIt != eventDB.end(); serviceIt++)
		{
			for (eventMap::iterator evIt = serviceIt->second.first.begin(); evIt != serviceIt->second.first.end(); evIt++)
				delete evIt->second;

			serviceIt->second.first.clear();
			serviceIt->second.second.clear();
		}
		eventDB.clear();
	}
}


timeMapPtr eEPGMemStore::getTimeMapPtr( const eServiceReferenceDVB& service, time_t from, time_t to, int limit )
{
	lock();

	eventCache::iterator it = eventDB.find( uniqueEPGKey( service ) );
	if ( it != eventDB.end() && it->second.second.size() )
	{
		return timeMapPtr( this, &(it->second.second) );
	}
	else
	{
		/* even though we return an empty timeMap, we use the 'this' pointer to ensure freeTimeMap is called */
		return timeMapPtr(this, NULL);
	}
}


void eEPGMemStore::freeTimeMap( timeMap* ptr )
{
	unlock();
}


EITEvent *eEPGMemStore::lookupEvent( const eServiceReferenceDVB &service, int event_id )
{
	singleLock s(cacheLock);
	uniqueEPGKey key( service );

	eventCache::iterator It = eventDB.find( key );
	if ( It != eventDB.end() && !It->second.first.empty() ) // entrys cached?
	{
		eventMap::iterator i( It->second.first.find( event_id ));
		if ( i != It->second.first.end() )
		{
			return new EITEvent( *i->second, (It->first.tsid<<16)|It->first.onid, i->second->type );
		}
		else
			eDebug("[EPGM] event %04x not found in epgcache", event_id);
	}
	return 0;
}


const eventData *eEPGMemStore::searchByTime( const eServiceReferenceDVB &service, time_t t )
// if t == 0 we search the current event...
{
	uniqueEPGKey key(service);

	// check if EPG for this service is ready...
	eventCache::iterator It = eventDB.find( key );
	if ( It != eventDB.end() && !It->second.first.empty() ) // entrys cached ?
	{
		if (!t)
			t = time(0)+eDVB::getInstance()->time_difference;

		timeMap::iterator i = It->second.second.lower_bound(t);
		if ( i != It->second.second.end() )
		{
			i--;
			if ( i != It->second.second.end() )
			{
				if ( t <= i->first+i->second->getDuration() )
				{
					return i->second;
				}
			}
		}

		for ( eventMap::iterator i( It->second.first.begin() ); i != It->second.first.end(); i++)
		{
			time_t begTime = i->second->getStartTime();
			if ( t >= begTime && t <= begTime+i->second->getDuration()) // then we have found
			{
				return i->second;
			}
		}
	}
	return 0;
}


EITEvent *eEPGMemStore::lookupEvent( const eServiceReferenceDVB &service, time_t t )
{
	singleLock s(cacheLock);
	const eventData *evt = searchByTime( service, t );
	if ( evt )
	{
		int tsidonid = ( service.getTransportStreamID().get()<<16 ) | service.getOriginalNetworkID().get();
		return new EITEvent( evt->get(), tsidonid, evt->type );
	}
	return 0;
}


const eventData* eEPGMemStore::lookupEventData( const eServiceReferenceDVB &service, time_t t )
{
	singleLock s(cacheLock);
	return searchByTime( service, t );
}


eventDataPtr eEPGMemStore::getEventDataPtr( const eServiceReferenceDVB& service, time_t t )
{
	singleLock s(cacheLock);
	const eventData *evt = searchByTime( service, t );
	if ( evt )
	{
		const eit_event_struct *eit_event = evt->get();
		int dataLength = HILO( eit_event->descriptors_loop_length ) + EIT_LOOP_SIZE;
		return eventDataPtr( new eventData( eit_event, dataLength, evt->type ) );
	}
	return eventDataPtr();
}


epgAtTime eEPGMemStore::getEpgAtTime( time_t atTime )
{
	if (atTime)
		return epgAtTime( this, atTime );
	else
		return epgAtTime();
}


bool eEPGMemStore::hasEPGData( const uniqueEPGKey& key )
{
	bool ret = false;
	
	eventCache::iterator it = eventDB.find( key );
	if ( it !=  eventDB.end() )
		ret = !it->second.second.empty() && !it->second.first.empty();
	
	return ret;
}


void eEPGMemStore::load()
{	
	struct stat epgStat;
	int epgStoreLimit = 1;
	
	eConfig::getInstance()->getKey("/enigma/epgStoreLimit", epgStoreLimit);
	
	// Check if file is available
	if (!stat( (dbDir + "/epg.dat").c_str(), &epgStat ))
	{
		// Only open epg file if it is smaller than 5 MB (or if limit disabled)
		if(!epgStoreLimit || (epgStat.st_size <= 5*(1<<20)))
		{
			FILE *f = fopen((dbDir + "/epg.dat").c_str(), "r");
			if (f)
			{
				unsigned char md5_saved[16];
				unsigned char md5[16];
				int size=0;
				int cnt=0;
				bool md5ok=false;
				if (!md5_file((dbDir + "/epg.dat").c_str(), 1, md5))
				{
					FILE *f = fopen((dbDir + "/epg.dat.md5").c_str(), "r");
					if (f)
					{
						fread( md5_saved, 16, 1, f);
						fclose(f);
						if ( !memcmp(md5_saved, md5, 16) )
							md5ok=true;
					}
				}
				if ( md5ok )
				{
					unsigned int magic=0;
					fread( &magic, sizeof(int), 1, f);
					if (magic != 0x98765432)
					{
						eDebug("[EPGM] epg file has incorrect byte order.. dont read it");
						fclose(f);
						return;
					}
					char text1[13];
					fread( text1, 13, 1, f);
					if ( !strncmp( text1, "ENIGMA_PLI_V5", 13) )
					{
						fread( &size, sizeof(int), 1, f);
						while(size--)
						{
							uniqueEPGKey key;
							eventMap evMap;
							timeMap tmMap;
							int size=0;
							fread( &key, sizeof(uniqueEPGKey), 1, f);
							fread( &size, sizeof(int), 1, f);
							while(size--)
							{
								__u16 len=0;
								__u8 type=0;
								eventData *event=0;
								fread( &type, sizeof(__u8), 1, f);
								fread( &len, sizeof(__u16), 1, f);
								event = new eventData(0, len, type);
								event->EITdata = new __u8[len];
								eventData::CacheSize+=len;
								fread( event->EITdata, len, 1, f);
								evMap[ event->getEventID() ]=event;
								tmMap[ event->getStartTime() ]=event;
								++cnt;
							}
							eventDB[key]=std::pair<eventMap,timeMap>(evMap,tmMap);
						}
						eventData::load(f);
						eDebug("[EPGM] %d events read from %s", cnt, (dbDir + "/epg.dat").c_str());
					}
					else
					{
						eDebug("[EPGM] don't read old epg database");
					}
					fclose(f);
				}
			}
		}
		else
		{
			eDebug("[EPGM] epg data: file too big");
		}
	}
	else
	{
		eDebug("[EPGM] epg data: problem reading file");
	}
}


void eEPGMemStore::save()
{
	struct statfs s;
	off64_t tmp;

	if (statfs(dbDir.c_str(), &s)<0)
	{
		tmp=0;
	}
	else
	{
		tmp=s.f_blocks;
		tmp*=s.f_bsize;
	}

	// We won't write anyway if there's less than 5MB available
	if ( tmp < 1024*1024*5 ) // storage size < 5MB
		return;

	// check for enough free space on storage
	tmp=s.f_bfree;
	tmp*=s.f_bsize;
	if ( tmp < (eventData::CacheSize*12)/10 ) // 20% overhead
		return;

	FILE *f = fopen((dbDir + "/epg.dat").c_str(), "w");
	int cnt=0;
	if ( f )
	{
		unsigned int magic = 0x98765432;
		fwrite( &magic, sizeof(int), 1, f);
		const char *text = "ENIGMA_PLI_V5";
		fwrite( text, 13, 1, f );
		int size = eventDB.size();
		fwrite( &size, sizeof(int), 1, f );
		for (eventCache::iterator service_it(eventDB.begin()); service_it != eventDB.end(); ++service_it)
		{
			timeMap &timemap = service_it->second.second;
			fwrite( &service_it->first, sizeof(uniqueEPGKey), 1, f);
			size = timemap.size();
			fwrite( &size, sizeof(int), 1, f);
			for (timeMap::iterator time_it(timemap.begin()); time_it != timemap.end(); ++time_it)
			{
				__u16 len = time_it->second->getSize();
				fwrite( &time_it->second->type, sizeof(__u8), 1, f );
				fwrite( &len, sizeof(__u16), 1, f);
				fwrite( time_it->second->EITdata, len, 1, f);
				++cnt;
			}
		}
		eDebug("[EPGM] %d events written to %s", cnt, (dbDir + "/epg.dat").c_str());
		eventData::save(f);
		fclose(f);
		unsigned char md5[16];
		if (!md5_file((dbDir + "/epg.dat").c_str(), 1, md5))
		{
			FILE *f = fopen((dbDir + "/epg.dat.md5").c_str(), "w");
			if (f)
			{
				fwrite( md5, 16, 1, f);
				fclose(f);
			}
		}
	}
}


//*******  eEPGSqlBase  *************

eEPGSqlBase::eEPGSqlBase() : dbHandle(0)
{
	pthread_mutex_init( &dbLock, 0 );
	
	if(eConfig::getInstance()->getKey("/enigma/epgSQLiteDir", dbDir))
	{
		// No directory key, assume and set default SQLite directory
		dbDir = getDefaultStorageDir();
		eConfig::getInstance()->setKey("/enigma/epgSQLiteDir", dbDir);
	}
	
	// Create database directory if it does not exist.
	DIR *dr = opendir( dbDir.c_str() );
	if ( !dr )
	{
		mkdir( dbDir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );
	}
	else
		closedir( dr );
	
	if ( openDatabase( DBNAME ) )
	{
		// First check version
		bool createSettingsTable = false;
		if ( isTable( "settings" ) )
		{
			queriedValue result;
			if ( runSqlCmd( "SELECT value FROM settings WHERE property='dbVersion'", &result ) && result.string != DBVERSION )
			// Other database version, recreate database
			{
				closeDatabase();
				openDatabase( DBNAME, 1 );
				createSettingsTable = true;
			}
		}
		else
			createSettingsTable = true;
		
		// Prevent SQLite from syncing after each change.
		runSqlCmd( "PRAGMA synchronous=OFF" );
		
		if ( createSettingsTable )
		{
			bool ok = runSqlCmd( "CREATE table settings (property TEXT, value TEXT)" );
			ok = ok && runSqlCmd( "CREATE UNIQUE INDEX property ON settings ( property )" );
			eString sqlCmd = "INSERT INTO settings ( property, value ) VALUES ( 'dbVersion', '";
			sqlCmd += DBVERSION;
			sqlCmd += "')";
			ok = ok && runSqlCmd( sqlCmd );
			if ( !ok )
				closeDatabase();
		}
		if ( isConnected() && !isTable( "epgcache" ) )
		{
			bool ok = runSqlCmd( "CREATE table epgcache (serviceKey TEXT, eventId INTEGER, startTime INTEGER, stopTime INTEGER, source INTEGER, eitData BLOB)" );
			ok = ok && runSqlCmd( "CREATE UNIQUE INDEX unTime on epgcache ( serviceKey, startTime )" );
			ok = ok && runSqlCmd( "CREATE UNIQUE INDEX unEvent on epgcache ( serviceKey, eventId )" );
			ok = ok && runSqlCmd( "CREATE INDEX source ON epgcache ( source )" );
			ok = ok && runSqlCmd( "CREATE INDEX stopTime ON epgcache ( stopTime )" );
			ok = ok && runSqlCmd( "CREATE INDEX startTime ON epgcache ( startTime )" );
			
			if ( !ok )
				closeDatabase();
			
			// Let SQLite analyze the indexes (don't ask me why this is necessary :-( )
			runSqlCmd( "ANALYZE" );
		}
	}
}


eEPGSqlBase::~eEPGSqlBase()
{
	if ( isConnected() )
		closeDatabase();
}


const bool eEPGSqlBase::openDatabase( const char *dbfile, bool deleteExistingDb )
{
	int ret = -1;
	
	closeDatabase();

	// Create database if the database does not exist, otherwise open database.
	singleLock l( dbLock );
	if ( !dbHandle )
	{
		eString fileName( dbDir );
		struct stat fStatus;
		
		fileName += "/";
		fileName += dbfile;
		if ( !stat( fileName.c_str(), &fStatus ) && deleteExistingDb )
		{
			eDebug("[EPGSQL] Deleting existing database file");
			unlink( fileName.c_str() );
		}
		
		eDebug("[EPGSQL] Using EPG directory %s", dbDir.c_str());
		ret = sqlite3_open( fileName.c_str(), &dbHandle );
	}
	return ( ret == 0 );
}

void eEPGSqlBase::closeDatabase()
{
	singleLock l( dbLock );
	if ( dbHandle )
	{
		sqlite3_close( dbHandle );
		dbHandle = 0;
	}
}

const bool eEPGSqlBase::isTable( const eString tableName )
{
	eString cmd;
	
	cmd = "SELECT * FROM ";
	cmd += tableName;
	cmd += " LIMIT 1";
	return runSqlCmd( cmd );
}

const bool eEPGSqlBase::runSqlCmd( const eString command, queriedValue* queryResult )
{
// queryResult is an option parameter, its type should match the first field in the query result.
// For now this can only be int or text.

	sqlite3_stmt *byteCode;
	const char *nextCmd;
	bool ret = false;
	int finalizeResult;

	if ( queryResult )
	{
		queryResult->integer = 0;
		queryResult->string = "";
	}	
	
	int epgDebug = 0;
	int epgRows = 0;
	eConfig::getInstance()->getKey("/enigma/epgStoreDebug", epgDebug);
	if (epgDebug)
		eDebug( "[EPGSQL] runSqlCmd %s", command.c_str() );
	singleLock l( dbLock );
	if ( sqlite3_prepare( dbHandle, command.c_str(), command.length(), &byteCode, &nextCmd ) == SQLITE_OK )
	{
		int resultCode;
		do
		{
			resultCode = sqlite3_step( byteCode );
			if ( resultCode == SQLITE_ROW )
				epgRows++;
			if ( queryResult && resultCode == SQLITE_ROW && sqlite3_column_count( byteCode ) > 0 )
			{
				if ( sqlite3_column_type( byteCode, 0 ) == SQLITE_INTEGER )
					queryResult->integer = sqlite3_column_int( byteCode, 0 );
				else if ( sqlite3_column_type( byteCode, 0 ) == SQLITE_TEXT )
					queryResult->string = (const char*) sqlite3_column_text( byteCode, 0 );
			}
		} while ( resultCode == SQLITE_BUSY );
		ret = ( resultCode == SQLITE_DONE || resultCode == SQLITE_ROW );
	}
	else
	{
		eDebug( "[EPGSQL] could not prepare %s", command.c_str() );
	}
	finalizeResult = sqlite3_finalize( byteCode );
	if (epgDebug)
		eDebug( "[EPGSQL] runSqlCmd got %d rows", epgRows);
	return ( ret && finalizeResult == SQLITE_OK );
}


const eString eEPGSqlBase::key2String( const uniqueEPGKey& key )
{
	char txtServiceKey[32];
	sprintf( txtServiceKey, "%x:%x:%x", key.sid, key.onid, key.tsid );
	return eString( txtServiceKey );
}


const uniqueEPGKey eEPGSqlBase::string2Key( eString strKey )
{
	int sid, onid, tsid;
	sscanf( strKey.c_str(), "%x:%x:%x", &sid, &onid, &tsid );
	return uniqueEPGKey( sid, onid, tsid );
}


int eEPGSqlBase::addEpg( uniqueEPGKey epgKey, int eventId, int source, int startTime, int stopTime, __u8 *eitData, int eitDataLength )
{
	int ret = 0;
	eString sqlCmd( "INSERT INTO epgcache (serviceKey, eventId, startTime, stopTime, source, eitData) VALUES (?, ?, ?, ?, ?, ?)" );
	sqlite3_stmt *byteCode;
	const char *next;
		
	int epgDebug = 0;
	eConfig::getInstance()->getKey("/enigma/epgStoreDebug", epgDebug);
	if (epgDebug)
		eDebug( "[EPGSQL] addEpg %s", sqlCmd.c_str() );
	singleLock l( dbLock );
	int resultCode = sqlite3_prepare( dbHandle, sqlCmd.c_str(), sqlCmd.length(), &byteCode, &next );
	if ( resultCode == SQLITE_OK )
	{
		bool abort = false;
		resultCode = sqlite3_bind_text( byteCode, 1, key2String( epgKey ).c_str(), -1, SQLITE_TRANSIENT );
		if (resultCode != SQLITE_OK)
			abort = true;
		if ( !abort )
		{
			resultCode = sqlite3_bind_double( byteCode, 2, eventId );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			resultCode = sqlite3_bind_double( byteCode, 3, startTime );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			resultCode = sqlite3_bind_double( byteCode, 4, stopTime );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			resultCode = sqlite3_bind_int( byteCode, 5, source );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			resultCode = sqlite3_bind_blob( byteCode, 6, eitData, eitDataLength, SQLITE_STATIC );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			do
				resultCode = sqlite3_step( byteCode );
			while ( resultCode == SQLITE_BUSY );
			ret = ( resultCode == SQLITE_DONE || resultCode == SQLITE_ROW );
		}
		else
			eDebug("[EPGSQL] error binding field for insert into epgcache: %d", resultCode);
		
	}
	else
		eDebug("[EPGSQL] error preparing insert statement for epgcache: %d", resultCode);
	
	sqlite3_finalize( byteCode );
	if (epgDebug)
		eDebug( "[EPGSQL] addEpg finished INSERT");
	return ret;
}


int eEPGSqlBase::updateEpg( uniqueEPGKey epgKey, int eventId, int source, int startTime, int stopTime, __u8 *eitData, int eitDataLength, const eString& clause, queriedValue* queryResult )
{
	if ( queryResult )
		queryResult->integer = 0;

	int ret = 0;
	eString sqlCmd( "UPDATE epgcache SET eventId = ?, startTime = ?, stopTime = ?, source = ?, eitData = ?" );
	sqlCmd += clause;
	
	sqlite3_stmt *byteCode;
	const char *next;
		
	int epgDebug = 0;
	eConfig::getInstance()->getKey("/enigma/epgStoreDebug", epgDebug);
	if (epgDebug)
		eDebug( "[EPGSQL] updateEpg %s", sqlCmd.c_str() );
	singleLock l( dbLock );
	int resultCode = sqlite3_prepare( dbHandle, sqlCmd.c_str(), sqlCmd.length(), &byteCode, &next );
	if ( resultCode == SQLITE_OK )
	{
		bool abort = false;
		resultCode = sqlite3_bind_double( byteCode, 1, eventId );
		if (resultCode != SQLITE_OK)
			abort = true;
		if ( !abort )
		{
			resultCode = sqlite3_bind_double( byteCode, 2, startTime );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			resultCode = sqlite3_bind_double( byteCode, 3, stopTime );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			resultCode = sqlite3_bind_int( byteCode, 4, source );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			resultCode = sqlite3_bind_blob( byteCode, 5, eitData, eitDataLength, SQLITE_STATIC );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			do
				resultCode = sqlite3_step( byteCode );
			while ( resultCode == SQLITE_BUSY );
			ret = ( resultCode == SQLITE_DONE || resultCode == SQLITE_ROW );
			if ( queryResult && resultCode == SQLITE_ROW )
				queryResult->integer = sqlite3_column_int( byteCode, 0 );
		}
		else
			eDebug("[EPGSQL] error binding field for update: %d", resultCode);
		
	}
	else
		eDebug("[EPGSQL] error preparing update statement: %d", resultCode);
	
	sqlite3_finalize( byteCode );
	if (epgDebug)
		eDebug( "[EPGSQL] updateEpg finished UPDATE" );
	return ret;
}


//*******  eEPGSqlStore  *************

eEPGSqlStore::eEPGSqlStore() : bufferReaderActive( false ), serviceLoaderActive( false )
{
	int created_services = 0;
	pthread_mutex_init( &bufferLock, 0 );
	
	if ( isConnected() && !isTable( "services" ) )
	{
		bool ok = runSqlCmd( "CREATE TABLE services (serviceKey TEXT, name TEXT, provider TEXT)" );
		ok = ok && runSqlCmd( "CREATE UNIQUE INDEX servicesKey ON services ( serviceKey )" );
		created_services = 1;

		if ( !ok )
			closeDatabase();
	}
	if ( isConnected() )
	{
		cleanup();
		if ( created_services )
			loadServices();

		CONNECT( eDVB::getInstance()->settings->getTransponders()->service_found, eEPGSqlStore::updateService );
		CONNECT( eDVB::getInstance()->settings->getTransponders()->service_removed, eEPGSqlStore::removeService );
		CONNECT( eEPGCache::getInstance()->EPGReaderFinished, eEPGSqlStore::EPGReaderFinished );
		CONNECT( eDVB::getInstance()->switchedService, eEPGSqlStore::switchedService );
	}
}


eEPGSqlStore::~eEPGSqlStore()
{
	eDebug( "[EPGSQL] entered eEPGSqlStore destructor" );
	if ( bufferReaderActive )
	// Shutdown bufferReader thread if still running, flushing the buffer will end it.
	{
		flushBuffer( waitingBuffer );
		flushBuffer( activeBuffer );
		
		while( bufferReaderActive )
			usleep( 100000 );
	}
}


eEPGStore::StorageCheckEnum eEPGSqlStore::checkStorage(eString directory)
{
	unsigned int dbSize = 0;
	struct stat s;
	struct statfs sfs;

	if(!stat((directory+"/"+DBNAME).c_str(), &s))
	{
		dbSize = s.st_size;
	}
	
	if(statfs(directory.c_str(), &sfs))
	{
		return STORAGE_NO_DIR;
	}
	
	if((sfs.f_bsize*sfs.f_bfree + dbSize) < 20*(1<<20))
	{
		return STORAGE_NO_SPACE;
	}
	
	return STORAGE_OK;
}


void eEPGSqlStore::updateService( const eServiceReferenceDVB& s, bool added )
{
//	eDebug( "[EPGSQL] updateService" );
	if ( bufferReaderActive )
		return;

	sqlite3_stmt *byteCode;
	const char *nextCmd;
	
	eString sqlCmd;
	if ( added )
		sqlCmd = "INSERT INTO services ( name, provider, serviceKey ) VALUES ( ?, ?, ? )";
	else
		sqlCmd = "UPDATE services SET name = ?, provider = ? WHERE serviceKey = ?";
	
	int epgDebug = 0;
	eConfig::getInstance()->getKey("/enigma/epgStoreDebug", epgDebug);
	if (epgDebug)
		eDebug( "[EPGSQL] updateService %s", sqlCmd.c_str() );
	singleLock l( dbLock );
	int resultCode = sqlite3_prepare( dbHandle, sqlCmd.c_str(), sqlCmd.length(), &byteCode, &nextCmd );
	if ( resultCode == SQLITE_OK )
	{
		eServiceDVB *service( eDVB::getInstance()->settings->getTransponders()->searchService( s ) );
		bool abort = false;
		
		eString name = service->service_name;
		unsigned int pos;
		// Get rid of the strange character sequences, for example seen with Premiere channels
		while ( ( pos = name.find( "\xc2" ) ) != eString::npos )
		{
			if ( pos == name.length() )
				name.erase( pos );
			else
				name.erase( pos, 2 );
		}
		resultCode = sqlite3_bind_text( byteCode, 1, name.c_str(), -1, SQLITE_TRANSIENT );
		
		if (resultCode != SQLITE_OK)
			abort = true;
		if ( !abort )
		{
			resultCode = sqlite3_bind_text( byteCode, 2, service->service_provider.c_str(), -1, SQLITE_TRANSIENT );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			uniqueEPGKey epgKey( s );
			resultCode = sqlite3_bind_text( byteCode, 3, key2String( epgKey ).c_str(), -1, SQLITE_TRANSIENT );
			if (resultCode != SQLITE_OK)
				abort = true;
		}
		if ( !abort )
		{
			do
				resultCode = sqlite3_step( byteCode );
			while ( resultCode == SQLITE_BUSY );
			if (resultCode != SQLITE_DONE && resultCode != SQLITE_ROW)
			{
				eDebug("[EPGSQL] update services table: expected DONE but got: %d, %s", resultCode, sqlite3_errmsg( dbHandle ));
			}
		}
		else
			eDebug("[EPGSQL] error binding field for insert/update into services: %d", resultCode);

	}
	else
		eDebug("[EPGSQL] error preparing insert/update statement for services: %d", resultCode);
	
	sqlite3_finalize( byteCode );
	if (epgDebug)
		eDebug( "[EPGSQL] updateService finished UPDATE/INSERT" );
}


void eEPGSqlStore::removeService( const eServiceReferenceDVB& s )
{
//	eDebug( "[EPGSQL] removeService" );
	if ( bufferReaderActive )
		return;

	sqlite3_stmt *byteCode;
	const char *nextCmd;
	
	eString sqlCmd = "DELETE FROM services WHERE serviceKey = ?";
	int epgDebug = 0;
	eConfig::getInstance()->getKey("/enigma/epgStoreDebug", epgDebug);
	if (epgDebug)
		eDebug( "[EPGSQL] removeService %s", sqlCmd.c_str() );
	singleLock l( dbLock );
	int resultCode = sqlite3_prepare( dbHandle, sqlCmd.c_str(), sqlCmd.length(), &byteCode, &nextCmd );
	if ( resultCode == SQLITE_OK )
	{
		uniqueEPGKey epgKey( s );
		bool abort = false;
		resultCode = sqlite3_bind_text( byteCode, 1, key2String( epgKey ).c_str(), -1, SQLITE_TRANSIENT );
		if (resultCode != SQLITE_OK)
			abort = true;
		if ( !abort )
		{
			do
				resultCode = sqlite3_step( byteCode );
			while ( resultCode == SQLITE_BUSY );
			if (resultCode != SQLITE_DONE && resultCode != SQLITE_ROW)
				eDebug("[EPGSQL] expected DONE but got: %d, %s", resultCode, sqlite3_errmsg( dbHandle ));
		}
		else
			eDebug("[EPGSQL] error binding field for delete from services: %d", resultCode);

	}
	else
		eDebug("[EPGSQL] error preparing delete statement for services: %d", resultCode);
	
	sqlite3_finalize( byteCode );
	if (epgDebug)
		eDebug( "[EPGSQL] removeService finished DELETE" );
}


eEPGSqlStore::loadService::loadService(eEPGSqlStore *par) : parent(par)
{
	while ( parent->bufferReaderActive )
		usleep( 100000 );

	eString sqlCmd = "DELETE FROM services";
	parent->runSqlCmd( sqlCmd );
}


void eEPGSqlStore::loadService::operator()( const eServiceReferenceDVB& s )
{
	while ( parent->bufferReaderActive )
		usleep( 100000 );

	parent->updateService( s, true );
}


void eEPGSqlStore::loadServices()
{
	pthread_t threadId;
	pthread_attr_t attr;
	pthread_attr_init( &attr );
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
	pthread_create( &threadId, &attr, loadServicesThread, (void*) this );
	serviceLoaderActive = true;
}


void *eEPGSqlStore::loadServicesThread( void *arg )
{
	eEPGSqlStore* user = (eEPGSqlStore*) arg;
	eDVB::getInstance()->settings->getTransponders()->forEachServiceReference( loadService(user) );
	user->serviceLoaderActive = false;
	return 0;
}


void eEPGSqlStore::processEpgRecord( uniqueEPGKey epgKey, int source, __u8 *eitData )
{
// Put data in buffer and return asap
	bufferEntry_t bufferEntry;
	bufferEntry.epgKey = epgKey;
	bufferEntry.source = source;

	getEITData( eitData, &bufferEntry.eventId, &bufferEntry.startTime, &bufferEntry.stopTime, &bufferEntry.eitDataLength );
	if ( bufferEntry.eventId == 0 )
	{
		createEventId( eitData );
		getEITData( eitData, &bufferEntry.eventId );
	}

	bufferEntry.eitData = new __u8[ bufferEntry.eitDataLength ];
	memcpy( bufferEntry.eitData, eitData, bufferEntry.eitDataLength );
	
	bufLock();
	if ( source == eEPGCache::NOWNEXT )
		activeBuffer.push_front( bufferEntry );
	else
		waitingBuffer.push_back( bufferEntry );
	
	bufUnlock();
	
	if ( !bufferReaderActive )  // Start bufferreader if either one of the buffers contains data
		startBufferReader();
}


void eEPGSqlStore::startBufferReader()
{
	bufLock();

	if ( !bufferReaderActive )
	// Start bufferReader thread
	{
		pthread_t threadId;
		pthread_attr_t attr;
		pthread_attr_init( &attr );
		pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
		
		pthread_create( &threadId, &attr, bufferReader, (void*) this );
		bufferReaderActive = true;
	}
	
	bufUnlock();
}


void *eEPGSqlStore::bufferReader( void *arg )
{
	eDebug( "[EPGSQL] thread for bufferReader started" );
	eEPGSqlStore* user = (eEPGSqlStore*) arg;
	setpriority( PRIO_PROCESS, 0, 2 );
	int inserts=0, updates=0, conflicts=0;
	int transactionCounter = 0;
	bool transaction = false;
//	eEPGSqlBase db;	// Use a seperate database connection in this thread
	// For testing purposes, use same database connection as the calling object:
	eEPGSqlStore *db = user;
	queriedValue result;
	
	db->runSqlCmd( "PRAGMA count_changes=1" );
	user->bufLock();
	while ( !user->activeBuffer.empty() || !user->waitingBuffer.empty() )
	{
		if ( !user->activeBuffer.empty() )
		{
			bufferEntry_t bufferEntry( user->activeBuffer.front() );
			user->activeBuffer.pop_front();
			user->bufUnlock();

			if (!transaction)
			{
				db->runSqlCmd( "BEGIN TRANSACTION" );
				transaction = true;
				transactionCounter = 0;
			}

			char toChar[12];
			eString sqlClause = " WHERE serviceKey = '";
			sqlClause += key2String( bufferEntry.epgKey );
			sqlClause += "' AND startTime = ";
			sprintf( toChar, "%d", bufferEntry.startTime );
			sqlClause += toChar;
			sqlClause += " AND source >= ";
			sprintf( toChar, "%d", bufferEntry.source );
			sqlClause += toChar;
			if ( db->updateEpg( bufferEntry.epgKey, bufferEntry.eventId, bufferEntry.source, bufferEntry.startTime, bufferEntry.stopTime, bufferEntry.eitData, bufferEntry.eitDataLength, sqlClause, &result ) && result.integer == 0 )
			// No error, though no records updated, try to insert 
			{
				if ( db->addEpg( bufferEntry.epgKey, bufferEntry.eventId, bufferEntry.source, bufferEntry.startTime, bufferEntry.stopTime, bufferEntry.eitData, bufferEntry.eitDataLength) )
					inserts++;
			}
			else if ( result.integer == 0 )
			// Update failed, probably a constraint error.
			// This is inconsistent. Delete these records and add a new one.
			{
				eString sqlCmd = "DELETE FROM epgcache";
				sqlCmd += " WHERE serviceKey = '";
				sqlCmd += key2String( bufferEntry.epgKey );
				sqlCmd += "' AND ( eventId = ";
				sprintf( toChar, "%d", bufferEntry.eventId );
				sqlCmd += toChar;
				sqlCmd += " OR startTime = ";
				sprintf( toChar, "%d", bufferEntry.startTime );
				sqlCmd += toChar;
				sqlCmd += " )";
				db->runSqlCmd( sqlCmd );
				db->addEpg( bufferEntry.epgKey, bufferEntry.eventId, bufferEntry.source, bufferEntry.startTime, bufferEntry.stopTime, bufferEntry.eitData, bufferEntry.eitDataLength );
				conflicts++;
			}
			else
				updates++;
		
			// Commit transaction every 1000 processed records
			if ( transaction && ++transactionCounter == 1000 )
			{
				db->runSqlCmd( "COMMIT TRANSACTION" );
				transaction = false;
				transactionCounter = 0;
			}
			
			delete [] bufferEntry.eitData;
			user->bufLock();
		}
		
		
		int loopCount = 0;
		while ( user->activeBuffer.empty() && !user->waitingBuffer.empty() && ++loopCount < 60 )
		// Do not shutdown this thread if there are records waiting in waitBuffer
		{
			user->bufUnlock();
			if (transaction)
			{
				db->runSqlCmd( "COMMIT TRANSACTION" );
				transaction = false;
			}
			usleep( 500000 ); // Sleep for 500 ms.
			user->bufLock();
		}
		if ( loopCount == 60 )
		{
			eDebug( "[EPGSQL] suspicious: activeBuffer empty while records in waitBuffer for 30 s." );
			user->activeBuffer.splice( user->activeBuffer.end(), user->waitingBuffer );
		}
		else if ( user->activeBuffer.empty() && user->waitingBuffer.empty() )
		// Prevent excessive thread restarts, wait a moment for new data to arrive
		{
			user->bufUnlock();
			if (transaction)
			{
				db->runSqlCmd( "COMMIT TRANSACTION" );
				transaction = false;
			}
			usleep( 500000 ); // Sleep for 500 ms.
			user->bufLock();		
		}
	}
	user->bufferReaderActive = false;
	user->bufUnlock();
	if (transaction)
	{
		db->runSqlCmd( "COMMIT TRANSACTION" );
	}
	eDebug( "[EPGSQL] thread for bufferReader stopped (%d)", time(0)+eDVB::getInstance()->time_difference );
	eDebug( "[EPGSQL] inserts/updates/conflicts  %d/%d/%d", inserts, updates, conflicts );
	return 0;
}


void eEPGSqlStore::flushBuffer( buffer_t& buffer )
{
	bufLock();
	while ( !buffer.empty() )
	{
		delete [] buffer.front().eitData;
		buffer.pop_front();
	}
	bufUnlock();
}


void eEPGSqlStore::EPGReaderFinished(int source)
// an EPG reader has finished
{
	if (source != eEPGCache::NOWNEXT)
	{
		/*
		 * The epg reader which has just finished, has delivered data into the waiting buffer.
		 * We should now start processing this data.
		 */
		bufLock();
		eDebug( "[EPGSQL] size waitingBuffer: %d", waitingBuffer.size() );
		eDebug( "[EPGSQL] size activeBuffer: %d", activeBuffer.size() );
		activeBuffer.splice( activeBuffer.end(), waitingBuffer );
		eDebug( "[EPGSQL] waitingBuffer activated, %d records left", waitingBuffer.size() );
		eDebug( "[EPGSQL] size activeBuffer: %d", activeBuffer.size() );
		bufUnlock();
	}
}


void eEPGSqlStore::switchedService( const eServiceReferenceDVB& ref, int err )
{
	flushBuffer( waitingBuffer );
}


void eEPGSqlStore::cleanup( uniqueKeyVector *vec )
{
	if ( bufferReaderActive )
	{
		eDebug( "[EPGSQL] Bufferreader is running, cancel cleanloop" );
		return;
	}

	time_t now = time(0) + eDVB::getInstance()->time_difference;
	char tm[12];

	sprintf( tm, "%d", (int) now );
	eString sqlCmd;

	if ( vec )
	{
		sqlCmd = "SELECT DISTINCT serviceKey FROM epgcache WHERE stopTime < ";
		sqlCmd += tm;
		sqlite3_stmt *byteCode;
		const char *next;

		int epgDebug = 0;
		int epgRows = 0;
		eConfig::getInstance()->getKey("/enigma/epgStoreDebug", epgDebug);
		if (epgDebug)
			eDebug( "[EPGSQL] cleanup %s", sqlCmd.c_str() );
		lock();
		int resultCode = sqlite3_prepare( dbHandle, sqlCmd.c_str(), sqlCmd.length(), &byteCode, &next );
		if ( resultCode == SQLITE_OK )
		{
			do
			{
				resultCode = sqlite3_step( byteCode );
				if ( resultCode == SQLITE_ROW ) {
					epgRows++;
					vec->push_back( string2Key( (const char *) sqlite3_column_text( byteCode, 0 ) ) );
				}
			} while ( resultCode == SQLITE_ROW || resultCode == SQLITE_BUSY );
		}
		else
		{
			eDebug( "[EPGSQL] error preparing %s", sqlCmd.c_str() );
		}
		sqlite3_finalize( byteCode );
		if (epgDebug)
			eDebug( "[EPGSQL] cleanup got %d rows", epgRows);
		unlock();
	}

	sqlCmd = "DELETE FROM epgcache WHERE stopTime < ";
	sqlCmd += tm;
	runSqlCmd( sqlCmd.c_str() );
}


void eEPGSqlStore::flushEPG( const uniqueEPGKey& s, const int event_id )
{
	eString sqlCmd( "DELETE FROM epgcache" );
	if ( s )
	{
		sqlCmd += " WHERE serviceKey = '";
		sqlCmd += key2String( s );
		sqlCmd += "'";
		if ( event_id )
		{
			char evt[17];
			sqlCmd += " AND event_id = ";
			sprintf( evt, "%d", event_id );
			sqlCmd += evt;
		}
	}
	
	runSqlCmd( sqlCmd );
}


timeMapPtr eEPGSqlStore::getTimeMapPtr( const eServiceReferenceDVB& service, time_t from, time_t to, int limit)
{
//eDebug( "[EPGSQL] start/stop %d/%d", from, to );
	timeMap* newMap = new timeMap;
	eString sqlCmd( "SELECT startTime, eitData, source FROM epgcache WHERE serviceKey='" );
	sqlCmd = sqlCmd + key2String( uniqueEPGKey( service ) ) + "'";

	if (from == 0 && to == 0)
	{
		/*
		 * user didn't specify time, so we get all events starting from 'now'
		 * (compatible with memstore behaviour)
		 */
		from = time(0) + eDVB::getInstance()->time_difference;
	}

	if ( from > 0 || to > 0 )
	{
		char tm[12];
		// serviceKey and startTime are in the same index, so these terms should
		// be next to each other to profit from this index.
		if ( to > 0 )
		{
			sqlCmd += " AND startTime < ";
			sprintf( tm, "%d", (int) to );
			sqlCmd += tm;
		}
		if ( from > 0 )
		{
			sqlCmd += " AND stopTime > ";
			sprintf( tm, "%d", (int) from );
			sqlCmd += tm;
		}
	}
	sqlCmd += " ORDER BY startTime";

	if (limit > 0)
	{
		eString strLimit;
		strLimit.sprintf(" LIMIT %d", limit);
		sqlCmd += strLimit;
	}

	sqlite3_stmt *byteCode;
	const char *next;
		
	int epgDebug = 0;
	int epgRows = 0;
	eConfig::getInstance()->getKey("/enigma/epgStoreDebug", epgDebug);
	if (epgDebug)
		eDebug( "[EPGSQL] getTimeMapPtr %s", sqlCmd.c_str() );
	singleLock l( dbLock );
	int resultCode = sqlite3_prepare( dbHandle, sqlCmd.c_str(), sqlCmd.length(), &byteCode, &next );
	if ( resultCode == SQLITE_OK )
	{
		do
		{
			resultCode = sqlite3_step( byteCode );
			if ( resultCode == SQLITE_ROW )
			{
				epgRows++;
				int blobsize = sqlite3_column_bytes( byteCode, 1 );
				eventData* data = new eventData( (eit_event_struct*) sqlite3_column_blob( byteCode, 1 ), blobsize, sqlite3_column_int( byteCode, 2 ) );
				std::pair<timeMap::iterator,bool> insertResult = newMap->insert(std::pair<time_t, eventData*>( sqlite3_column_int(byteCode, 0), data) );
				if ( !insertResult.second )
					delete data;  // Insert in map failed, delete allocated memory
			}
		} while ( resultCode == SQLITE_ROW || resultCode == SQLITE_BUSY);
	}
	else
	{
		eDebug( "[EPGSQL] error preparing %s", sqlCmd.c_str() );
	}
	sqlite3_finalize( byteCode );
	if (epgDebug)
		eDebug( "[EPGSQL] getTimeMapPtr got %d rows", epgRows);
	return timeMapPtr( this, newMap );
}


void eEPGSqlStore::freeTimeMap( timeMap* ptr )
{
	if (ptr)
	{
		for ( timeMap::iterator it( ptr->begin() ); it != ptr->end(); it++ )
			delete it->second;
		
		ptr->clear();
		delete ptr;
	}
}


eventData *eEPGSqlStore::queryEvent( const eServiceReferenceDVB &service, const eString& sqlCmd )
{
	sqlite3_stmt *byteCode;
	const char *next;
	eventData* data = 0;
		
	int epgDebug = 0;
	int epgRows = 0;
	eConfig::getInstance()->getKey("/enigma/epgStoreDebug", epgDebug);
	if (epgDebug)
		eDebug( "[EPGSQL] queryEvent %s", sqlCmd.c_str() );
	singleLock l( dbLock );
	int resultCode = sqlite3_prepare( dbHandle, sqlCmd.c_str(), sqlCmd.length(), &byteCode, &next );
	if ( resultCode == SQLITE_OK )
	{
		do
			resultCode = sqlite3_step( byteCode );
		while ( resultCode == SQLITE_BUSY );
				
		if ( resultCode == SQLITE_ROW )
		{
			epgRows++;
			eit_event_struct* eit_event = (eit_event_struct*) sqlite3_column_blob( byteCode, 0 );
			int dataLength = HILO( eit_event->descriptors_loop_length ) + EIT_LOOP_SIZE;
			data = new eventData( eit_event, dataLength, sqlite3_column_int( byteCode, 1 ) );
		}
		
	}
	else
	{
		eDebug( "[EPGSQL] error preparing %s", sqlCmd.c_str() );
	}

	sqlite3_finalize( byteCode );
	if (epgDebug)
		eDebug( "[EPGSQL] queryEvent got %d rows", epgRows);
	return data;
}


EITEvent* eEPGSqlStore::lookupEvent( const eServiceReferenceDVB &service, int event_id )
{
	char evtid[12];
	sprintf( evtid, "%d", event_id );

	eString sqlCmd( "SELECT eitData, source FROM epgcache WHERE serviceKey='" );
	sqlCmd += key2String( uniqueEPGKey( service ) ) + "' AND eventId = ";
	sqlCmd += evtid;
	
	eventData *evt = queryEvent( service, sqlCmd );
	if ( evt )
	{
		EITEvent *eitevent = new EITEvent( evt->get(), ( service.getTransportStreamID().get()<<16 ) | service.getOriginalNetworkID().get(), evt->type );
		delete evt;
		return eitevent;
	}
	return NULL;
}


EITEvent* eEPGSqlStore::lookupEvent( const eServiceReferenceDVB &service, time_t t )
{
	if (!t)
		t = time(0)+eDVB::getInstance()->time_difference;

	char tm[12];
	sprintf( tm, "%d", (int) t );

	eString sqlCmd( "SELECT eitData, source FROM epgcache WHERE serviceKey='" );
	sqlCmd += key2String( uniqueEPGKey( service ) ) + "' AND startTime <= ";
	sqlCmd += tm;
	sqlCmd += " AND stopTime > ";
	sqlCmd += tm;
	
	eventData *evt = queryEvent( service, sqlCmd );
	if ( evt )
	{
		EITEvent *eitevent = new EITEvent( evt->get(), ( service.getTransportStreamID().get()<<16 ) | service.getOriginalNetworkID().get(), evt->type );
		delete evt;
		return eitevent;
	}
	return NULL;
}


eventDataPtr eEPGSqlStore::getEventDataPtr( const eServiceReferenceDVB &service, time_t t )
{
	if (!t)
		t = time(0)+eDVB::getInstance()->time_difference;

	char tm[12];
	sprintf( tm, "%d", (int) t );

	eString sqlCmd( "SELECT eitData, source FROM epgcache WHERE serviceKey='" );
	sqlCmd += key2String( uniqueEPGKey( service ) ) + "' AND startTime <= ";
	sqlCmd += tm;
	sqlCmd += " AND stopTime > ";
	sqlCmd += tm;

	eventData *evt = queryEvent( service, sqlCmd );
	return eventDataPtr( evt );
}


epgAtTime eEPGSqlStore::getEpgAtTime( time_t atTime )
{
	if (atTime)
	{
		serviceEpgInfo *newInfo = new serviceEpgInfo;
		bool dataAvailable = false;

		eString sqlCmd( "SELECT serviceKey, eitData FROM epgcache WHERE startTime <= " );
		char ctm[12];
	
		sprintf( ctm, "%d", (int) atTime );
		sqlCmd += ctm;
		sqlCmd += " AND stopTime > ";
		sqlCmd += ctm;
	
		sqlite3_stmt *byteCode;
		const char *next;
		
		int epgDebug = 0;
		int epgRows = 0;
		eConfig::getInstance()->getKey("/enigma/epgStoreDebug", epgDebug);
		if (epgDebug)
			eDebug( "[EPGSQL] getEpgAtTime %s", sqlCmd.c_str() );
		singleLock l( dbLock );
		int resultCode = sqlite3_prepare( dbHandle, sqlCmd.c_str(), sqlCmd.length(), &byteCode, &next );
		if ( resultCode == SQLITE_OK )
		{
			do
			{
				resultCode = sqlite3_step( byteCode );
				if ( resultCode == SQLITE_ROW )
				{
					epgRows++;
					int blobsize = sqlite3_column_bytes( byteCode, 1 );
					eventData* data = new eventData( (eit_event_struct*) sqlite3_column_blob( byteCode, 1 ), blobsize, sqlite3_column_int( byteCode, 2 ) );
					uniqueEPGKey key = string2Key((const char*) sqlite3_column_text(byteCode, 0));
					bool insertResult = newInfo->insert(key, data);
					if ( insertResult && !dataAvailable )
						dataAvailable = true;
				}
			} while ( resultCode == SQLITE_ROW || resultCode == SQLITE_BUSY );
		}
		else
		{
			eDebug( "[EPGSQL] error preparing %s", sqlCmd.c_str() );
		}
		sqlite3_finalize( byteCode );
		if (epgDebug)
			eDebug( "[EPGSQL] getEpgAtTime got %d rows", epgRows);
		if (dataAvailable)
			return epgAtTime( newInfo );
		else
			delete newInfo;
	}
	return epgAtTime();
}


bool eEPGSqlStore::hasEPGData( const uniqueEPGKey& key )
{
	eString sqlCmd( "SELECT startTime FROM epgcache WHERE serviceKey ='" );
	sqlCmd += key2String( key ) + "' LIMIT 1";
	
	queriedValue queryRes;
	queryRes.integer = -1;
	runSqlCmd( sqlCmd, &queryRes );
	
	return ( queryRes.integer >= 0 );
}


void eEPGSqlStore::organise()
{
	if (!bufferReaderActive)
		runSqlCmd( "VACUUM" );
}


const eventData* epgAtTime::channelEpg( eServiceReferenceDVB& ref )
{
	const eventData* ret = 0;

	if ( epgInfo )
	// EPG data is stored in object
	{
		uniqueEPGKey key = uniqueEPGKey(ref);
		ret = epgInfo->getInfo( key );
	}
	else if ( atTime && store )
	// EPG data will be retreived directly from store
		ret = store->lookupEventData( ref, atTime );
		
	return ret;
}
