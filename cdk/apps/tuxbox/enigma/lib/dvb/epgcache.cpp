#include <lib/dvb/epgcache.h>

#undef NVOD
#undef EPG_DEBUG  

#include <time.h>
#include <unistd.h>  // for usleep
#include <sys/vfs.h> // for statfs
#include <libmd5sum.h>
#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/dvb/lowlevel/dvb.h>
#include <lib/dvb/si.h>
#ifdef ENABLE_MHW_EPG
#include <lib/dvb/lowlevel/mhw.h>
#endif
#include <lib/dvb/service.h>
#include <lib/dvb/dvbservice.h>

const int UPDATE_INTERVAL = 3600000;	// 60 minutes
const int ZAP_DELAY = 2000;	        // 2 sek


eEPGCache *eEPGCache::instance;
pthread_mutex_t eEPGCache::cache_lock=
	PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


extern unsigned int crc32_table[256];

#if 0
eventData::eventData(const eit_event_struct* e, int size, int type)
	:ByteSize(size&0xFF), type(type&0xFF)
{
	init_eventData(e,size,type);
}
void eventData::init_eventData(const eit_event_struct* e, int size, int type)
{
	if (!e)
		return;

	__u32 descr[65];
	__u32 *pdescr=descr;

	__u8 *data = (__u8*)e;
	int ptr=12;
	size -= 12;

	while(size > 1)
	{
		__u8 *descr = data+ptr;
		int descr_len = descr[1];
		descr_len += 2;
		if (size >= descr_len)
		{
			switch (descr[0])
			{
				case DESCR_EXTENDED_EVENT:
				case DESCR_SHORT_EVENT:
				case DESCR_LINKAGE:
				case DESCR_COMPONENT:
				case DESCR_CONTENT:
				case DESCR_TIME_SHIFTED_EVENT:
				{
					__u32 crc = 0;
					int cnt=0;
					while(cnt++ < descr_len)
						crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ data[ptr++]) & 0xFF];

					descriptorMap::iterator it =
						descriptors.find(crc);
					if ( it == descriptors.end() )
					{
						CacheSize+=descr_len;
						__u8 *d = new __u8[descr_len];
						memcpy(d, descr, descr_len);
						descriptors[crc] = descriptorPair(1, d);
					}
					else
						++it->second.first;
					*pdescr++=crc;
					break;
				}
				default: // do not cache all other descriptors
					ptr += descr_len;
					break;
			}
			size -= descr_len;
		}
		else
			break;
	}
	ASSERT(pdescr <= &descr[65]);
	ByteSize = 10+((pdescr-descr)*4);
	EITdata = new __u8[ByteSize];
	CacheSize+=ByteSize;
	memcpy(EITdata, (__u8*) e, 10);
	memcpy(EITdata+10, descr, ByteSize-10);
}

const eit_event_struct* eventData::get() const
{
	int pos = 12;
	int tmp = ByteSize-10;
	memcpy(data, EITdata, 10);
	int descriptors_length=0;
	__u32 *p = (__u32*)(EITdata+10);
	while(tmp>3)
	{
		descriptorMap::iterator it =
			descriptors.find(*p++);
		if ( it != descriptors.end() )
		{
			int b = it->second.second[1]+2;
			memcpy(data+pos, it->second.second, b );
			pos += b;
			descriptors_length += b;
		}
		else
			eFatal("LINE %d descriptor not found in descriptor cache %08x!!!!!!", __LINE__, *(p-1));
		tmp-=4;
	}
	ASSERT(pos <= 4108);
	data[10] = (descriptors_length >> 8) & 0x0F;
	data[11] = descriptors_length & 0xFF;
	return (eit_event_struct*)data;
}

bool eventData::search(int tsidonid, const eString &search, int intExactMatch, int intCaseSensitive, int genre, int Range)
{
	int tmp = ByteSize-10;
	__u32 *p = (__u32*)(EITdata+10);
	char language_code[3];
	__u8* data ;
	eString event_name;
	bool bGenreFound = (genre == 0);
	bool bTextFound = (search == "");
	while(tmp>3)
	{
		descriptorMap::iterator it =
			descriptors.find(*p++);
		if ( it != descriptors.end() )
		{
			switch (it->second.second[0])
			{
				case DESCR_SHORT_EVENT:
					{
						data = it->second.second;
						memcpy(language_code, data+2, 3);
						int ptr=5;
						int len=data[ptr++];
						int table = 0;
						std::map<eString, int>::iterator it =
							eString::CountryCodeDefaultMapping.find(eString(language_code,3));
						if ( it != eString::CountryCodeDefaultMapping.end() )
							table = it->second;
						event_name=convertDVBUTF8((unsigned char*)data+ptr, len,table,tsidonid);
						event_name.strReplace("\xc2\x8a",": ");
						if (!(intExactMatch || intCaseSensitive))
							event_name = event_name.upper();
						bTextFound = (intExactMatch ? !strcmp(search.c_str(),event_name.c_str()) : (event_name.find(search) != eString::npos));
						if (bGenreFound)
							return bTextFound;
						break;
					}
				case DESCR_CONTENT:
					if (genre)
					{
						data = (it->second.second+1);
						int len = *data;
						__u8 *work=data+1;
					
						while( work < (data+len) )
						{
							descr_content_entry_struct *tmp = (descr_content_entry_struct*)work;
							if ( genre < 32 )
							{
								bGenreFound = (genre  == tmp->content_nibble_level_1*16+tmp->content_nibble_level_2);
							}
							else
							{
								int genreID = tmp->content_nibble_level_1*16+tmp->content_nibble_level_2;
								bGenreFound =  (genreID >= genre) && (genreID <= Range);
							}
							if (bGenreFound)
								break;
							work+=2;
						}
						if (bTextFound)
							return bGenreFound;
					}
					break;
			}
		}
		tmp-=4;
	}
	return false;
}

eventData::~eventData()
{
	if ( ByteSize )
	{
		CacheSize -= ByteSize;
		__u32 *d = (__u32*)(EITdata+10);
		ByteSize -= 10;
		while(ByteSize>3)
		{
			descriptorMap::iterator it =
				descriptors.find(*d++);
			if ( it != descriptors.end() )
			{
				descriptorPair &p = it->second;
				if (!--p.first) // no more used descriptor
				{
					CacheSize -= it->second.second[1];
					delete [] it->second.second;  	// free descriptor memory
					descriptors.erase(it);	// remove entry from descriptor map
				}
			}
			else
				eFatal("LINE %d descriptor not found in descriptor cache %08x!!!!!!", __LINE__, *(d-1));
			ByteSize -= 4;
		}
		delete [] EITdata;
	}
}

void eventData::load(FILE *f)
{
	int size=0;
	int id=0;
	__u8 header[2];
	descriptorPair p;
	fread(&size, sizeof(int), 1, f);
	while(size)
	{
		fread(&id, sizeof(__u32), 1, f);
		fread(&p.first, sizeof(int), 1, f);
		fread(header, 2, 1, f);
		int bytes = header[1]+2;
		p.second = new __u8[bytes];
		p.second[0] = header[0];
		p.second[1] = header[1];
		fread(p.second+2, bytes-2, 1, f);
		descriptors[id]=p;
		--size;
		CacheSize+=bytes;
	}
}

void eventData::save(FILE *f)
{
	int size=descriptors.size();
	descriptorMap::iterator it(descriptors.begin());
	fwrite(&size, sizeof(int), 1, f);
	while(size)
	{
		fwrite(&it->first, sizeof(__u32), 1, f);
		fwrite(&it->second.first, sizeof(int), 1, f);
		fwrite(it->second.second, it->second.second[1]+2, 1, f);
		++it;
		--size;
	}
}
#endif

eEPGCache::eEPGCache()
	:messages(this,1), paused(0), firstStart(1) 
	,CleanTimer(this), zapTimer(this), abortTimer(this), organiseTimer(this), epgStore(0)
{
	init_eEPGCache();
}
void eEPGCache::init_eEPGCache()
{
	eDebug("[EPGC] Initialized EPGCache");
	isRunning=0;
	isLoading=0;
	scheduleReader.setContext(this);
	scheduleOtherReader.setContext(this);
#ifdef ENABLE_MHW_EPG
	scheduleMhwReader.setContext(this);
#endif
#ifdef ENABLE_DISH_EPG
	dishEEPGReader.setContext(this);
	bevEEPGReader.setContext(this);
#endif
	nownextReader.setContext(this);
#ifdef ENABLE_PRIVATE_EPG
	contentReader.setContext(this);
	CONNECT(eDVB::getInstance()->gotContentPid, eEPGCache::setContentPid);
#endif
	CONNECT(messages.recv_msg, eEPGCache::gotMessage);
	CONNECT(eDVB::getInstance()->switchedService, eEPGCache::enterService);
	CONNECT(eDVB::getInstance()->leaveService, eEPGCache::leaveService);
	CONNECT(eDVB::getInstance()->timeUpdated, eEPGCache::timeUpdated);
	CONNECT(zapTimer.timeout, eEPGCache::startEPG);
	CONNECT(CleanTimer.timeout, eEPGCache::cleanLoop);
	CONNECT(abortTimer.timeout, eEPGCache::abortNonAvail);
	CONNECT(organiseTimer.timeout, eEPGCache::organiseEvent);
	instance=this;
	epgStore = eEPGStore::createEPGStore();
	setOrganiseTimer();
	epgHours = 4 * 24;
}

void eEPGCache::timeUpdated()
{
	if ( !thread_running() )
	{
		eDebug("[EPGC] time updated.. start EPG Mainloop");
		run();
		if ( cached_service )
			enterService(cached_service, cached_err);
	}
	else
		messages.send(Message(Message::timeChanged));
}

#ifdef ENABLE_PRIVATE_EPG

struct date_time
{
	__u8 data[5];
	time_t tm;
	date_time( const date_time &a )
	{
		memcpy(data, a.data, 5);
		tm = a.tm;
	}
	date_time( const __u8 data[5])
	{
		memcpy(this->data, data, 5);
		tm = parseDVBtime(data[0], data[1], data[2], data[3], data[4]);
	}
	date_time()
	{
	}
	const __u8& operator[](int pos) const
	{
		return data[pos];
	}
};

struct less_datetime
{
	bool operator()( const date_time &a, const date_time &b ) const
	{
		return abs(a.tm-b.tm) < 360 ? false : a.tm < b.tm;
	}
};

int ePrivateContent::sectionRead(__u8 *data)
{
	eEPGCache &instance = *eEPGCache::getInstance();
	uniqueEPGKey &current_service = instance.current_service;
//	contentMap &content_time_table = instance.content_time_tables[current_service];
	if ( instance.paused )
		return 0;
	if ( seenSections.find( data[6] ) == seenSections.end() )
	{
		std::map< date_time, std::list<uniqueEPGKey>, less_datetime > start_times;
		int ptr=8;
		int content_id = data[ptr++] << 24;
		content_id |= data[ptr++] << 16;
		content_id |= data[ptr++] << 8;
		content_id |= data[ptr++];

//		contentTimeMap &time_event_map = content_time_table[content_id];
//		for ( contentTimeMap::iterator it( time_event_map.begin() ); it != time_event_map.end(); ++it )
//			instance.epgStore->flushEPG( current_service, it->second.second );
		
//		time_event_map.clear();

		__u8 duration[3];
		memcpy(duration, data+ptr, 3);
		ptr+=3;
		int duration_sec =
			fromBCD(duration[0])*3600+fromBCD(duration[1])*60+fromBCD(duration[2]);

		__u8 *descriptors[65];
		__u8 **pdescr = descriptors;

		int descriptors_length = (data[ptr++]&0x0F) << 8;
		descriptors_length |= data[ptr++];
		while ( descriptors_length > 0 )
		{
			int descr_type = data[ptr];
			int descr_len = data[ptr+1];
			descriptors_length -= (descr_len+2);
			if ( descr_type == 0xf2 )
			{
				ptr+=2;
				int tsid = data[ptr++] << 8;
				tsid |= data[ptr++];
				int onid = data[ptr++] << 8;
				onid |= data[ptr++];
				int sid = data[ptr++] << 8;
				sid |= data[ptr++];

// WORKAROUND for wrong transmitted epg data (01.10.2007)
					if ( onid == 0x85 )
					{
						switch( (tsid << 16) | sid )
						{
							case 0x01030b: sid = 0x1b; tsid = 4; break;  // Premiere Win
							case 0x0300f0: sid = 0xe0; tsid = 2; break;
							case 0x0300f1: sid = 0xe1; tsid = 2; break;
							case 0x0300f5: sid = 0xdc; break;
							case 0x0400d2: sid = 0xe2; tsid = 0x11; break;
							case 0x1100d3: sid = 0xe3; break;
							case 0x0100d4: sid = 0xe4; tsid = 4; break;
						}
					}
////////////////////////////////////////////

				uniqueEPGKey service( sid, onid, tsid );
				descr_len -= 6;
				while( descr_len > 0 )
				{
					__u8 datetime[5];
					datetime[0] = data[ptr++];
					datetime[1] = data[ptr++];
					int tmp_len = data[ptr++];
					descr_len -= 3;
					while( tmp_len > 0 )
					{
						memcpy(datetime+2, data+ptr, 3);
						ptr+=3;
						descr_len -= 3;
						tmp_len -= 3;
						start_times[datetime].push_back(service);
					}
				}
			}
			else
			{
				*pdescr++=data+ptr;
				ptr += 2;
				ptr += descr_len;
			}
		}
		__u8 event[4098];
		eit_event_struct *ev_struct = (eit_event_struct*) event;
		ev_struct->running_status = 0;
		ev_struct->free_CA_mode = 1;
		memcpy(event+7, duration, 3);
		ptr = 12;
		__u8 **d=descriptors;
		while ( d < pdescr )
		{
			memcpy(event+ptr, *d, ((*d)[1])+2);
			ptr+=(*d++)[1];
			ptr+=2;
		}
		for ( std::map< date_time, std::list<uniqueEPGKey> >::iterator it(start_times.begin()); it != start_times.end(); ++it )
		{
			time_t now = time(0)+eDVB::getInstance()->time_difference;

			if ( (it->first.tm + duration_sec) < now )
				continue;

			memcpy(event+2, it->first.data, 5);
			int bptr = ptr;
			int cnt=0;
			for (std::list<uniqueEPGKey>::iterator i(it->second.begin()); i != it->second.end(); ++i)
			{
				event[bptr++] = 0x4A;
				__u8 *len = event+(bptr++);
				event[bptr++] = (i->tsid & 0xFF00) >> 8;
				event[bptr++] = (i->tsid & 0xFF);
				event[bptr++] = (i->onid & 0xFF00) >> 8;
				event[bptr++] = (i->onid & 0xFF);
				event[bptr++] = (i->sid & 0xFF00) >> 8;
				event[bptr++] = (i->sid & 0xFF);
				event[bptr++] = 0xB0;
				bptr += sprintf((char*)(event+bptr), "Option %d", ++cnt);
				*len = ((event+bptr) - len)-1;
			}
			int llen = bptr - 12;
			ev_struct->descriptors_loop_length_hi = (llen & 0xF00) >> 8;
			ev_struct->descriptors_loop_length_lo = (llen & 0xFF);

//			time_t stime = it->first.tm;
//			while( tmMap.find(stime) != tmMap.end() )
//				++stime;
//			event[6] += (stime - it->first.tm);
//			__u16 event_id = 0;
//			while( evMap.find(event_id) != evMap.end() )
//				++event_id;
			event[0] = 0;
			event[1] = 0;
//			time_event_map[it->first.tm]=std::pair<time_t, __u16>(stime, event_id);
//			eventData *d = new eventData( ev_struct, bptr, eEPGCache::SCHEDULE );
			if (instance.epgStore) instance.epgStore->processEpgRecord( current_service, eEPGCache::SCHEDULE, (__u8*) ev_struct );
		}
		seenSections.insert(data[6]);
	}
	if ( seenSections.size() == (unsigned int)(data[7] + 1) )
	{
		if (!instance.epgStore || !instance.epgStore->hasEPGData( current_service ) )
		{
			/*emit*/ instance.EPGAvail(1);
			instance.Lock();
			instance.temp[current_service] =
				std::pair<time_t, int>(time(0)+eDVB::getInstance()->time_difference, eEPGCache::NOWNEXT);
			instance.Unlock();
			/*emit*/ instance.EPGUpdated();
		}
		int version = data[5];
		if ( eSystemInfo::getInstance()->hasNegFilter() )
			version = ((version & 0x3E) >> 1);
		else
			version = (version&0xC1)|((version+2)&0x3E);
		start_filter(pid,version);
	}
	return 0;
}
#endif // ENABLE_PRIVATE_EPG

int eEPGCache::sectionRead(__u8 *data, int source)
{
	if ( !data || state > 1 )
		return -ECANCELED;

	eit_t *eit = (eit_t*) data;
	bool seen=false;
	tidMap &seenSections = this->seenSections[source];
	tidMap &calcedSections = this->calcedSections[source];

#ifdef ENABLE_MHW_EPG
	if ( source != SCHEDULE_MHW )
	// In case of SCHEDULE_MHW the procedure that is feeding the data will send each section once.
	{
#endif
#ifdef ENABLE_DISH_EPG
		if (source != DISH_EEPG && source != BEV_EEPG)
		{
#endif
			__u32 sectionNo = data[0] << 24;
			sectionNo |= data[3] << 16;
			sectionNo |= data[4] << 8;
			sectionNo |= eit->section_number;
	
			tidMap::iterator it =
				seenSections.find(sectionNo);
	
			if ( it != seenSections.end() )
				seen=true;
			else
			{
				seenSections.insert(sectionNo);
				calcedSections.insert(sectionNo);
				__u32 tmpval = sectionNo & 0xFFFFFF00;
				__u8 incr = source == NOWNEXT ? 1 : 8;
				for ( int i = 0; i <= eit->last_section_number; i+=incr )
				{
					if ( i == eit->section_number )
					{
						for (int x=i; x <= eit->segment_last_section_number; ++x)
							calcedSections.insert(tmpval|(x&0xFF));
					}
					else
						calcedSections.insert(tmpval|(i&0xFF));
				}
			}
#ifdef ENABLE_DISH_EPG
		}
#endif
#ifdef ENABLE_MHW_EPG
	}
#endif

	int len=HILO(eit->section_length)-1;//+3-4;
	int ptr=EIT_SIZE;
	if ( ptr < len && !seen )
	{
		// This fixed the EPG on the Multichoice irdeto systems
		// the EIT packet is non-compliant.. their EIT packet stinks
		if ( data[ptr-1] < 0x40 )
			--ptr;

		uniqueEPGKey service( HILO(eit->service_id), HILO(eit->original_network_id), HILO(eit->transport_stream_id) );

		eit_event_struct* eit_event = (eit_event_struct*) (data+ptr);
		int eit_event_size;
		int duration;

		time_t TM = parseDVBtime( eit_event->start_time_1, eit_event->start_time_2,	eit_event->start_time_3, eit_event->start_time_4, eit_event->start_time_5);
		time_t now = time(0)+eDVB::getInstance()->time_difference;

		if ( TM != 3599 && TM > -1)
		{
			switch(source)
			{
			case NOWNEXT:
				haveData |= (1 << NOWNEXT);
				break;
			case SCHEDULE:
				haveData |= (1 << SCHEDULE);
				break;
			case SCHEDULE_OTHER:
				haveData |= (1 << SCHEDULE_OTHER);
				break;
#ifdef ENABLE_DISH_EPG
			case DISH_EEPG:
				haveData |= (1 << DISH_EEPG);
				break;
			case BEV_EEPG:
				haveData |= (1 << BEV_EEPG);
				break;
#endif
			}
		}

#ifdef ENABLE_MHW_EPG
		if ( (haveData & ((1 << SCHEDULE) | (1 << SCHEDULE_OTHER))) && (isRunning & (1 << SCHEDULE_MHW)) )
		{
			eDebug("[EPGC] si schedule data avail.. abort mhw reader");
			isRunning &= ~(1 << SCHEDULE_MHW);
			scheduleMhwReader.abort();
		}
#endif

		// hier wird immer eine eventMap zurück gegeben.. entweder eine vorhandene..
		// oder eine durch [] erzeugte
	
		while (ptr<len)
		{
			eit_event_size = HILO(eit_event->descriptors_loop_length)+EIT_LOOP_SIZE;

			if ( eit_event_size < EIT_LOOP_SIZE+	2 ) // skip events without descriptors
				goto next;
	
			duration = fromBCD(eit_event->duration_1)*3600+fromBCD(eit_event->duration_2)*60+fromBCD(eit_event->duration_3);
			TM = parseDVBtime(
				eit_event->start_time_1,
				eit_event->start_time_2,
				eit_event->start_time_3,
				eit_event->start_time_4,
				eit_event->start_time_5);
	
#ifndef NVOD
			if ( TM == 3599 )
				goto next;
#endif

			if ( TM != 3599 && (TM+duration < now || TM > now + epgHours * 3600) )
				goto next;

#ifdef NVOD
			// look in 1st descriptor tag.. i hope all time shifted events have the
			// time shifted event descriptor at the begin of the descriptors..
			if ( ((unsigned char*)eit_event)[12] == 0x4F ) // TIME_SHIFTED_EVENT_DESCR
			{
				// get Service ID of NVOD Service ( parent )
				int sid = ((unsigned char*)eit_event)[14] << 8 | ((unsigned char*)eit_event)[15];
				uniqueEPGKey parent( sid, HILO(eit->original_network_id), HILO(eit->transport_stream_id) );
				// check that this nvod reference currently is not in the NVOD List
				std::list<NVODReferenceEntry>::iterator it( NVOD[parent].begin() );
				for ( ; it != NVOD[parent].end(); it++ )
					if ( it->service_id == HILO(eit->service_id) && it->original_network_id == HILO( eit->original_network_id ) )
						break;
				if ( it == NVOD[parent].end() )  // not in list ?
					NVOD[parent].push_back( NVODReferenceEntry( HILO(eit->transport_stream_id), HILO(eit->original_network_id), HILO(eit->service_id) ) );
			}
#endif
			if (epgStore) epgStore->processEpgRecord( service, source, (__u8*) eit_event );

next:

			ptr += eit_event_size;
			eit_event=(eit_event_struct*)(((__u8*)eit_event)+eit_event_size);
		}

		Lock();
		tmpMap::iterator it = temp.find( service );
		if ( it != temp.end() )
		{
			if ( source > it->second.second )
			{
				it->second.first=now;
				it->second.second=source;
			}
		}
		else
			temp[service] = std::pair< time_t, int> (now, source);
		Unlock();
	}

#ifdef ENABLE_DISH_EPG
	if (source == DISH_EEPG || source == BEV_EEPG)
	{
		/* dish eepg doesn't use seenSections */
		return 0;
	}
#endif

#ifdef ENABLE_MHW_EPG
	if (source == SCHEDULE_MHW)
	{
		/* MHW doesn't use seenSections */
		return 0;
	}
#endif

	if ( state == 1 && seenSections == calcedSections )
	{
		return -ECANCELED;
	}

	return 0;
}

bool eEPGCache::finishEPG(int source)
{
	if (haveData & (1 << source))
	{
		/* a reader which has delivered us data, has just finished */
		/*emit*/ EPGReaderFinished(source);
	}

	if (!isRunning)  // epg ready
	{
		eDebug("[EPGC] stop caching events(%d)", time(0)+eDVB::getInstance()->time_difference );

		for (int i=0; i < 3; ++i)
		{
			seenSections[i].clear();
			calcedSections[i].clear();
		}
		zapTimer.start(UPDATE_INTERVAL, 1);
		eDebug("[EPGC] next update in %i min", UPDATE_INTERVAL / 60000);

		Lock();
		tmpMap::iterator It = temp.begin();
		abortTimer.stop();

		while (It != temp.end())
		{
//			eDebug("sid = %02x, onid = %02x, type %d", It->first.sid, It->first.onid, It->second.second );
			// Only add services to LastUpdated if they are part of the current stream.
			if ( It->first.tsid == current_service.tsid || It->second.second == SCHEDULE )
			{
//				eDebug("ADD to last updated Map");
				serviceLastUpdated[It->first]=It->second.first;
			}
			if ( !epgStore || !epgStore->hasEPGData( It->first ) )
			{
//				eDebug("REMOVE from update Map");
				temp.erase(It++);
			}
			else
				It++;
		}
		Unlock();
		if ( epgStore && epgStore->hasEPGData( current_service ) )
			/*emit*/ EPGAvail(1);

		/*emit*/ EPGUpdated();

		return true;
	}
	return false;
}

void eEPGCache::flushEPG(const uniqueEPGKey & s)
{
	eDebug("[EPGC] flushEPG %d", (int)(bool)s);
	if (epgStore) epgStore->flushEPG( s );
	singleLock l(cache_lock);
	if (s)  // clear only this service
	{
		updateMap::iterator u = serviceLastUpdated.find(s);
		if ( u != serviceLastUpdated.end() )
			serviceLastUpdated.erase(u);
#ifdef ENABLE_PRIVATE_EPG
		contentMaps::iterator it =
			content_time_tables.find(s);
		if ( it != content_time_tables.end() )
		{
			it->second.clear();
			content_time_tables.erase(it);
		}
#endif
	}
	else // clear complete EPG Cache
	{
		serviceLastUpdated.clear();
#ifdef ENABLE_PRIVATE_EPG
		content_time_tables.clear();
#endif
	}
	eDebug("[EPGC] %i bytes for cache used", eventData::CacheSize);
}

void eEPGCache::cleanLoop()
{
	if ( isRunning || (temp.size() && haveData) || !epgStore )
	{
		CleanTimer.startLongTimer(5);
		eDebug("[EPGC] schedule cleanloop");
		return;
	}
	
	// Cleanup eEPGStore object
	if ( !paused )
	{
		// eDebug("[EPGC] start cleanloop");
		uniqueKeyVector changedServices;
		epgStore->cleanup( &changedServices );
		
		// temp needs to be filled here
		Lock();
		time_t now = time(0)+eDVB::getInstance()->time_difference;
		for ( unsigned int i = 0; i < changedServices.size(); i++ )
		{
			// add this (changed) service to temp map...
			if ( temp.find( changedServices[i] ) == temp.end() )
				temp[ changedServices[i] ]=std::pair<time_t, int>(now, NOWNEXT);
		}
		Unlock();
				
		if ( !temp.empty() )
			/*emit*/ EPGUpdated();
		// eDebug("[EPGC] stop cleanloop");
	}
	
	CleanTimer.startLongTimer(epgStore->getCleanloopInterval());
	return;
	
	// Code below has been switched off
/*	if (!eventDB.empty() && !paused )
	{
//		time_t now = time(0)+eDVB::getInstance()->time_difference;

		for (eventCache::iterator DBIt = eventDB.begin(); DBIt != eventDB.end(); DBIt++)
		{
			bool updated=false;


#ifdef ENABLE_PRIVATE_EPG
			if ( updated )
			{
				contentMaps::iterator x =
					content_time_tables.find( DBIt->first );
				if ( x != content_time_tables.end() )
				{
					timeMap &tmMap = eventDB[DBIt->first].second;
					for ( contentMap::iterator i = x->second.begin(); i != x->second.end(); )
					{
						for ( contentTimeMap::iterator it(i->second.begin());
							it != i->second.end(); )
						{
							if ( tmMap.find(it->second.first) == tmMap.end() )
								i->second.erase(it++);
							else
								++it;
						}
						if ( i->second.size() )
							++i;
						else
							x->second.erase(i++);
					}
				}
			}
#endif

		}

#ifdef NVOD
		for (nvodMap::iterator it(NVOD.begin()); it != NVOD.end(); ++it )
		{
//			eDebug("check NVOD Service");
			eventCache::iterator evIt(eventDB.find(it->first));
			if ( evIt != eventDB.end() && evIt->second.first.size() )
			{
				for ( eventMap::iterator i(evIt->second.first.begin());
					i != evIt->second.first.end(); )
				{
#if EPG_DEBUG
					ASSERT(i->second->getStartTime() == 3599);
#endif
					int cnt=0;
					for ( std::list<NVODReferenceEntry>::iterator ni(it->second.begin());
						ni != it->second.end(); ++ni )
					{
						eventCache::iterator nie(eventDB.find(uniqueEPGKey(ni->service_id, ni->original_network_id, ni->transport_stream_id) ) );
						if (nie != eventDB.end() && nie->second.first.find( i->first ) != nie->second.first.end() )
						{
							++cnt;
							break;
						}
					}
					if ( !cnt ) // no more referenced
					{
						delete i->second;  // free memory
						evIt->second.first.erase(i++);  // remove from eventmap
					}
					else
						++i;
				}
			}
		}
#endif


	} */
}

eEPGCache::~eEPGCache()
{
	messages.send(Message::quit);
	kill(); // waiting for thread shutdown
	delete epgStore;
}

EITEvent *eEPGCache::lookupEvent(const eServiceReferenceDVB &service, int event_id)
{
	if (isLoading)
		return 0;

	if ( epgStore )
		return epgStore->lookupEvent( service, event_id );
	else
		return 0;
}

EITEvent *eEPGCache::lookupEvent(const eServiceReferenceDVB &service, time_t t )
{
	if (isLoading)
		return 0;

	if ( epgStore )
		return epgStore->lookupEvent( service, t );
	else
		return 0;
}

void eEPGCache::pauseEPG()
{
	if (!paused)
	{
		abortEPG();
		eDebug("[EPGC] paused]");
		paused=1;
	}
}

void eEPGCache::restartEPG()
{
	if (paused)
	{
		isRunning=0;
		eDebug("[EPGC] restarted");
		paused--;
		if (paused)
		{
			paused = 0;
			startEPG();   // updateEPG
		}
		cleanLoop();
	}
}


void eEPGCache::setOrganiseTimer()
{
	// The timer will be set for 4:00 am

#define EVENT_HOUR	4
#define EVENT_MIN	0
	
	time_t timenow = time(0)+eDVB::getInstance()->time_difference;
	tm *localtm = localtime( &timenow );

	if (localtm->tm_hour > EVENT_HOUR || (localtm->tm_hour == EVENT_HOUR && localtm->tm_min >= EVENT_MIN))
	// Skip one day when past the event time
	{
		time_t timecorr = timenow + 24*3600;
		localtm = localtime( &timecorr );
	}
	
	localtm->tm_hour = EVENT_HOUR;
	localtm->tm_min = EVENT_MIN;
	localtm->tm_sec = 0;
	time_t timeevent = mktime( localtm );	

	organiseTimer.startLongTimer( timeevent - timenow + 1 );
}


void eEPGCache::organiseEvent()
{
	/* emit */  organiseRequest();
	setOrganiseTimer();
}


void eEPGCache::organise()
{
	if (epgStore)
		epgStore->organise();
}


void eEPGCache::reloadStore()	// Can be used to switch between stores
{
	pauseEPG();
	
	// Avoid race
	eEPGStore *tempStore = epgStore;
	epgStore = 0;
	delete tempStore;
	serviceLastUpdated.clear();
	epgStore = eEPGStore::createEPGStore();
	
	restartEPG();
	if ( !isRunning )
		startEPG();
}


void eEPGCache::forceEpgScan()
{
	if ( !isRunning && epgStore )
		startEPG();
}


void eEPGCache::startEPG()
{
	if (paused)  // called from the updateTimer during pause...
	{
		paused++;
		return;
	}
	if ( !epgStore )
	{
		eDebug("[EPGC] wait for epgStore to be available");
		zapTimer.start(1000, 1); // restart Timer
	
	}
	else if (eDVB::getInstance()->time_difference)
	{
		if ( firstStart )
		{
			eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
			if ( !sapi || !sapi->transponder )
				return;
			firstStart=0;
		}
		Lock();
		temp.clear();
		for (int i=0; i < 3; ++i)
		{
			seenSections[i].clear();
			calcedSections[i].clear();
		}
		Unlock();
		eConfig::getInstance()->getKey("/extras/epghours", epgHours);
		eDebug("[EPGC] start caching events(%d)", time(0)+eDVB::getInstance()->time_difference);
		state=0;
		haveData=0;
		scheduleReader.start();
		isRunning |= 1 << SCHEDULE;
		nownextReader.start();
		isRunning |= 1 << NOWNEXT;
		scheduleOtherReader.start();
		isRunning |= 1 << SCHEDULE_OTHER;
#ifdef ENABLE_MHW_EPG
		int mhwepg = 1;
		eConfig::getInstance()->getKey("/extras/mhwepg", mhwepg);
		if (mhwepg)
		{
			scheduleMhwReader.start();
			isRunning |= 1 << SCHEDULE_MHW;
		}
#endif
#ifdef ENABLE_DISH_EPG
		int disheepg = 0;
		eConfig::getInstance()->getKey("/extras/disheepg", disheepg);
		if (disheepg)
		{
			eDebug("[EPGC] start dishEEPGReader");
			dishEEPGReader.start();
			isRunning |= 1 << DISH_EEPG;
			eDebug("[EPGC] start bevEEPGReader");
			bevEEPGReader.start();
			isRunning |= 1 << BEV_EEPG;
		}
#endif
		abortTimer.start(7000,true);
	}
	else
	{
		eDebug("[EPGC] wait for clock update");
		zapTimer.start(1000, 1); // restart Timer
	}
}

void eEPGCache::abortNonAvail()
{
	if (!state)
	{
		if ( !(haveData&(1 << NOWNEXT)) && (isRunning&(1 << NOWNEXT)) )
		{
			eDebug("[EPGC] abort non avail nownext reading");
			isRunning &= ~(1 << NOWNEXT);
			nownextReader.abort();
		}
		if ( !(haveData&(1 << SCHEDULE)) && (isRunning&(1 << SCHEDULE)) )
		{
			eDebug("[EPGC] abort non avail schedule reading");
			isRunning &= ~(1 << SCHEDULE);
			scheduleReader.abort();
		}
		if ( !(haveData&(1 << SCHEDULE_OTHER)) && (isRunning&(1 << SCHEDULE_OTHER)) )
		{
			eDebug("[EPGC] abort non avail schedule_other reading");
			isRunning &= ~(1 << SCHEDULE_OTHER);
			scheduleOtherReader.abort();
		}
#ifdef ENABLE_MHW_EPG
		if ( !(haveData&(1 << SCHEDULE_MHW)) && (isRunning&(1 << SCHEDULE_MHW)) )
		{
			eDebug("[EPGC] abort non avail mhw schedule reading");
			isRunning &= ~(1 << SCHEDULE_MHW);
			scheduleMhwReader.abort();
		}
#endif
#ifdef ENABLE_DISH_EPG
		if ( !(haveData&(1 << DISH_EEPG)) && (isRunning&(1 << DISH_EEPG)) )
		{
			eDebug("[EPGC] abort non avail Dish reading");
			isRunning &= ~(1 << DISH_EEPG);
			dishEEPGReader.abort();
		}
		if ( !(haveData&(1 << BEV_EEPG)) && (isRunning&(1 << BEV_EEPG)) )
		{
			eDebug("[EPGC] abort non avail BEV reading");
			isRunning &= ~(1 << BEV_EEPG);
			bevEEPGReader.abort();
		}
#endif
		if ( isRunning )
			abortTimer.start(90000, true);
		else
		{
			eDebug("[EPGC] no data .. abort");
			++state;
		}
	}
	else
		eDebug("[EPGC] timeout");
	++state;
}

void eEPGCache::enterService(const eServiceReferenceDVB& ref, int err)
{
	if ( ref.path )
		err = 2222;
	else if ( ref.getServiceType() == 7 )
		switch ( eSystemInfo::getInstance()->getHwType() )
		{
			case eSystemInfo::dbox2Nokia:
			case eSystemInfo::dbox2Sagem:
			case eSystemInfo::dbox2Philips:
				err = 1111; //faked
			default:
				break;
		}

	if ( thread_running() )
	// -> gotMessage -> changedService
		messages.send(Message(Message::enterService, ref, err));
	else
	{
		cached_service = ref;
		cached_err = err;
	}
}

void eEPGCache::leaveService(const eServiceReferenceDVB& ref)
{
	if ( thread_running() )
		messages.send(Message(Message::leaveService, ref));
	else
		cached_service=eServiceReferenceDVB();
	// -> gotMessage -> abortEPG
}

#ifdef ENABLE_PRIVATE_EPG
void eEPGCache::setContentPid( int pid )
{
	messages.send(Message(Message::content_pid, pid));
	// -> gotMessage -> content_pid
}
#endif

void eEPGCache::changedService(const uniqueEPGKey &service, int err)
{
	if ( err == 2222 )
	{
		eDebug("[EPGC] dont start ... its a replay");
		/*emit*/ EPGAvail(0);
		return;
	}

	current_service = service;
	updateMap::iterator It = serviceLastUpdated.find( current_service );

	int update;

// check if this is a subservice and this is only a dbox2
// then we dont start epgcache on subservice change..
// ever and ever..

	if ( !err || err == -ENOCASYS || err == -ENVOD )
	{
		update = ( It != serviceLastUpdated.end() ? ( UPDATE_INTERVAL - ( (time(0)+eDVB::getInstance()->time_difference-It->second) * 1000 ) ) : ZAP_DELAY );

		if (update < ZAP_DELAY)
			update = ZAP_DELAY;

		zapTimer.start(update, 1);
		if (update >= 60000)
			eDebug("[EPGC] next update in %i min", update/60000);
		else if (update >= 1000)
			eDebug("[EPGC] next update in %i sec", update/1000);
	}

	bool empty = (!epgStore || !epgStore->hasEPGData( current_service ));

	if (!empty)
	{
		eDebug("[EPGC] yet cached");
		/*emit*/ EPGAvail(1);
	}
	else
	{
		eDebug("[EPGC] not cached yet");
		/*emit*/ EPGAvail(0);
	}
}

void eEPGCache::abortEPG()
{
	abortTimer.stop();
	zapTimer.stop();
	if (isRunning)
	{
		if (isRunning & (1 << SCHEDULE))
		{
			isRunning &= ~(1 << SCHEDULE);
			scheduleReader.abort();
		}
		if (isRunning & (1 << NOWNEXT))
		{
			isRunning &= ~(1 << NOWNEXT);
			nownextReader.abort();
		}
		if (isRunning & (1 << SCHEDULE_OTHER))
		{
			isRunning &= ~(1 << SCHEDULE_OTHER);
			scheduleOtherReader.abort();
		}
#ifdef ENABLE_MHW_EPG
		if (isRunning & (1 << SCHEDULE_MHW))
		{
			isRunning &= ~(1 << SCHEDULE_MHW);
			scheduleMhwReader.abort();
		}
#endif
#ifdef ENABLE_DISH_EPG
		if (isRunning & (1 << DISH_EEPG))
		{
			isRunning &= ~(1 << DISH_EEPG);
			dishEEPGReader.abort();
		}
		if (isRunning & (1 << BEV_EEPG))
		{
			isRunning &= ~(1 << BEV_EEPG);
			bevEEPGReader.abort();
		}
#endif
		eDebug("[EPGC] abort caching events !!");
		Lock();
		temp.clear();
		Unlock();
	}
}

void eEPGCache::gotMessage( const Message &msg )
{
	switch (msg.type)
	{
		case Message::flush:
			flushEPG(msg.service);
			break;
		case Message::enterService:
			changedService(msg.service, msg.err);
			break;
		case Message::leaveService:
			// msg.service is valid.. but not needed
			abortEPG();
#ifdef ENABLE_PRIVATE_EPG
			contentReader.stop();
#endif
			break;
		case Message::pause:
			pauseEPG();
			break;
		case Message::restart:
			restartEPG();
			break;
		case Message::quit:
			quit(0);
			break;
		case Message::timeChanged:
			cleanLoop();
			setOrganiseTimer();
			break;
#ifdef ENABLE_PRIVATE_EPG
		case Message::content_pid:
			contentReader.start(msg.pid);
			break;
#endif
/*		case Message::save:
			save();
			break;
		case Message::load:
			flushEPG();
			load();
			break;	*/
		case Message::reloadStore:
			reloadStore();
			break;
		case Message::forceEpgScan:
			forceEpgScan();
			break;
		case Message::organise:
			organise();
			break;
		default:
			eDebug("[EPGC] unhandled EPGCache Message!!");
			break;
	}
}

void eEPGCache::thread()
{
	nice(4);
#ifdef ENABLE_PRIVATE_EPG
	dbDir = eEPGMemStore::getDefaultStorageDir();
	eConfig::getInstance()->getKey("/enigma/epgMemStoreDir", dbDir);
	eDebug("[EPGC] Using EPG file %s", dbDir.c_str());

	loadPrivateEPG();
#endif // ENABLE_PRIVATE_EPG
	exec();
#ifdef ENABLE_PRIVATE_EPG
	savePrivateEPG();
#endif // ENABLE_PRIVATE_EPG
}


#ifdef ENABLE_PRIVATE_EPG
void eEPGCache::loadPrivateEPG()
{
	FILE *f = fopen((dbDir + "/private_epg.dat").c_str(), "r");
	if (f)
	{
		unsigned char md5_saved[16];
		unsigned char md5[16];
		int size=0;
//		int cnt=0;
		bool md5ok=false;
		if (!md5_file((dbDir + "/private_epg.dat").c_str(), 1, md5))
		{
			FILE *f = fopen((dbDir + "/private_epg.dat.md5").c_str(), "r");
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
				eDebug("[EPGC] epg file has incorrect byte order.. dont read it");
				fclose(f);
				return;
			}
			char text2[11];
			fread( text2, 11, 1, f);
			if ( !strncmp( text2, "PRIVATE_EPG", 11) )
			{
				size=0;
				isLoading = 1;
				singleLock l(cache_lock);
				fread( &size, sizeof(int), 1, f);
				while(size--)
				{
					int size=0;
					uniqueEPGKey key;
					fread( &key, sizeof(uniqueEPGKey), 1, f);
					fread( &size, sizeof(int), 1, f);
					while(size--)
					{
						int size;
						int content_id;
						fread( &content_id, sizeof(int), 1, f);
						fread( &size, sizeof(int), 1, f);
						while(size--)
						{
							time_t time1, time2;
							__u16 event_id;
							fread( &time1, sizeof(time_t), 1, f);
							fread( &time2, sizeof(time_t), 1, f);
							fread( &event_id, sizeof(__u16), 1, f);
							content_time_tables[key][content_id][time1]=std::pair<time_t, __u16>(time2, event_id);
						}
					}
				}
				isLoading=0;
			}
			else
				eDebug("[EPGC] don't read incompatible epg database");
			fclose(f);
		}
	}
}


void eEPGCache::savePrivateEPG()
{
	struct statfs s;
	off64_t tmp;
	if (statfs(dbDir.c_str(), &s)<0)
		tmp=0;
	else
	{
		tmp=s.f_blocks;
		tmp*=s.f_bsize;
	}

	// prevent writes to builtin flash
	if ( tmp < 1024*1024*50 ) // storage size < 50MB
		return;

	FILE *f = fopen((dbDir + "/private_epg.dat").c_str(), "w");
//	int cnt=0;
	if ( f )
	{
		unsigned int magic = 0x98765432;
		fwrite( &magic, sizeof(int), 1, f);
		const char* text3 = "PRIVATE_EPG";
		fwrite( text3, 11, 1, f );
		int size = content_time_tables.size();
		fwrite( &size, sizeof(int), 1, f);
		for (contentMaps::iterator a = content_time_tables.begin(); a != content_time_tables.end(); ++a)
		{
			contentMap &content_time_table = a->second;
			fwrite( &a->first, sizeof(uniqueEPGKey), 1, f);
			int size = content_time_table.size();
			fwrite( &size, sizeof(int), 1, f);
			for (contentMap::iterator i = content_time_table.begin(); i != content_time_table.end(); ++i )
			{
				int size = i->second.size();
				fwrite( &i->first, sizeof(int), 1, f);
				fwrite( &size, sizeof(int), 1, f);
				for ( contentTimeMap::iterator it(i->second.begin());
					it != i->second.end(); ++it )
				{
					fwrite( &it->first, sizeof(time_t), 1, f);
					fwrite( &it->second.first, sizeof(time_t), 1, f);
					fwrite( &it->second.second, sizeof(__u16), 1, f);
				}
			}
		}
		fclose(f);
		unsigned char md5[16];
		if (!md5_file((dbDir + "/private_epg.dat").c_str(), 1, md5))
		{
			FILE *f = fopen((dbDir + "/private_epg.dat.md5").c_str(), "w");
			if (f)
			{
				fwrite( md5, 16, 1, f);
				fclose(f);
			}
		}
	}
}
#endif // ENABLE_PRIVATE_EPG


eAutoInitP0<eEPGCache> init_eEPGCacheInit(eAutoInitNumbers::service+3, "EPG cache");

#ifdef ENABLE_MHW_EPG
void eScheduleMhw::cleanup()
{
	channels.clear();
	themes.clear();
	titles.clear();
	program_ids.clear();
}

__u8 *eScheduleMhw::delimitName( __u8 *in, __u8 *out, int len_in )
{
	// Names in mhw structs are not strings as they are not '\0' terminated.
	// This function converts the mhw name into a string.
	// Constraint: "length of out" = "length of in" + 1.
	int i;
	for ( i=0; i < len_in; i++ )
		out[i] = in[i];
	
	i = len_in - 1;
	while ( ( i >=0 ) && ( out[i] == 0x20 ) )
		i--;
	
	out[i+1] = 0;
	return out;
}

void eScheduleMhw::timeMHW2DVB( u_char hours, u_char minutes, u_char *return_time)
// For time of day
{
	return_time[0] = toBCD( hours );
	return_time[1] = toBCD( minutes );
	return_time[2] = 0;
}

void eScheduleMhw::timeMHW2DVB( int minutes, u_char *return_time)
{
	timeMHW2DVB( int(minutes/60), minutes%60, return_time );
}

void eScheduleMhw::timeMHW2DVB( u_char day, u_char hours, u_char minutes, u_char *return_time)
// For date plus time of day
{
	// Remove offset in mhw time.
	__u8 local_hours = hours;
	if ( hours >= 16 )
		local_hours -= 4;
	else if ( hours >= 8 )
		local_hours -= 2;

	// As far as we know all mhw time data is sent in central Europe time zone.
	// So, temporarily set timezone to western europe
	time_t dt = ::time(0);

	char *old_tz = getenv( "TZ" );
	putenv("TZ=CET-1CEST,M3.5.0/2,M10.5.0/3");
	tzset();

	tm localnow;
	localtime_r(&dt, &localnow);

	if (day == 7)
		day = 0;
	if ( day + 1 < localnow.tm_wday )		// day + 1 to prevent old events to show for next week.
		day += 7;
	if (local_hours <= 5)
		day++;

	dt += 3600*24*(day - localnow.tm_wday);	// Shift dt to the recording date (local time zone).
	dt += 3600*(local_hours - localnow.tm_hour);  // Shift dt to the recording hour.

	tm recdate;
	gmtime_r( &dt, &recdate );   // This will also take care of DST.

	if ( old_tz == NULL )
		unsetenv( "TZ" );
	else
		putenv( old_tz );
	tzset();

	// Calculate MJD according to annex in ETSI EN 300 468
	int l=0;
	if ( recdate.tm_mon <= 1 )	// Jan or Feb
		l=1;
	int mjd = 14956 + recdate.tm_mday + int( (recdate.tm_year - l) * 365.25) +
		int( (recdate.tm_mon + 2 + l * 12) * 30.6001);

	return_time[0] = (mjd & 0xFF00)>>8;
	return_time[1] = mjd & 0xFF;

	timeMHW2DVB( recdate.tm_hour, minutes, return_time+2 );
}

void eScheduleMhw::storeTitle(std::map<__u32, mhw_title_t>::iterator itTitle, eString sumText, __u8 *data)
// data is borrowed from calling proc to save memory space.
{
	__u8 name[34];

	// For each title a separate EIT packet will be sent to eEPGCache::sectionRead()
	bool isMHW2 = itTitle->second.mhw2_mjd_hi || itTitle->second.mhw2_mjd_lo ||
		itTitle->second.mhw2_duration_hi || itTitle->second.mhw2_duration_lo;

	eit_t *packet = (eit_t *) data;
	packet->table_id = 0x50;
	packet->section_syntax_indicator = 1;

	packet->service_id_hi = channels[ itTitle->second.channel_id - 1 ].channel_id_hi;
	packet->service_id_lo = channels[ itTitle->second.channel_id - 1 ].channel_id_lo;
	packet->version_number = 0;	// eEPGCache::sectionRead() will dig this for the moment
	packet->current_next_indicator = 0;
	packet->section_number = 0;	// eEPGCache::sectionRead() will dig this for the moment
	packet->last_section_number = 0;	// eEPGCache::sectionRead() will dig this for the moment
	packet->transport_stream_id_hi = channels[ itTitle->second.channel_id - 1 ].transport_stream_id_hi;
	packet->transport_stream_id_lo = channels[ itTitle->second.channel_id - 1 ].transport_stream_id_lo;
	packet->original_network_id_hi = channels[ itTitle->second.channel_id - 1 ].network_id_hi;
	packet->original_network_id_lo = channels[ itTitle->second.channel_id - 1 ].network_id_lo;
	packet->segment_last_section_number = 0; // eEPGCache::sectionRead() will dig this for the moment
	packet->segment_last_table_id = 0x50;

	__u8 *title = isMHW2 ? ((__u8*)(itTitle->second.title))-4 : (__u8*)itTitle->second.title;
	std::string prog_title = (char *) delimitName( title, name, isMHW2 ? 33 : 23 );
	int prog_title_length = prog_title.length();

	int packet_length = EIT_SIZE + EIT_LOOP_SIZE + EIT_SHORT_EVENT_DESCRIPTOR_SIZE +
		prog_title_length + 1;

	eit_event_t *event_data = (eit_event_t *) (data + EIT_SIZE);
	event_data->event_id_hi = (( itTitle->first ) >> 8 ) & 0xFF;
	event_data->event_id_lo = ( itTitle->first ) & 0xFF;

	if (isMHW2)
	{
		u_char *data = (u_char*) event_data;
		data[2] = itTitle->second.mhw2_mjd_hi;
		data[3] = itTitle->second.mhw2_mjd_lo;
		data[4] = itTitle->second.mhw2_hours;
		data[5] = itTitle->second.mhw2_minutes;
		data[6] = itTitle->second.mhw2_seconds;
		timeMHW2DVB( HILO(itTitle->second.mhw2_duration), data+7 );
	}
	else
	{
		timeMHW2DVB( itTitle->second.dh.day, itTitle->second.dh.hours, itTitle->second.ms.minutes,
		(u_char *) event_data + 2 );
		timeMHW2DVB( HILO(itTitle->second.duration), (u_char *) event_data+7 );
	}

	event_data->running_status = 0;
	event_data->free_CA_mode = 0;
	int descr_ll = EIT_SHORT_EVENT_DESCRIPTOR_SIZE + 1 + prog_title_length;

	eit_short_event_descriptor_struct *short_event_descriptor =
		(eit_short_event_descriptor_struct *) ( (u_char *) event_data + EIT_LOOP_SIZE);
	short_event_descriptor->descriptor_tag = EIT_SHORT_EVENT_DESCRIPTOR;
	short_event_descriptor->descriptor_length = EIT_SHORT_EVENT_DESCRIPTOR_SIZE +
		prog_title_length - 1;
	short_event_descriptor->language_code_1 = 'e';
	short_event_descriptor->language_code_2 = 'n';
	short_event_descriptor->language_code_3 = 'g';
	short_event_descriptor->event_name_length = prog_title_length;
	u_char *event_name = (u_char *) short_event_descriptor + EIT_SHORT_EVENT_DESCRIPTOR_SIZE;
	memcpy(event_name, prog_title.c_str(), prog_title_length);

	// Set text length
	event_name[prog_title_length] = 0;

	if ( sumText.length() > 0 )
	// There is summary info
	{
		unsigned int sum_length = sumText.length();
		if ( sum_length + short_event_descriptor->descriptor_length <= 0xff )
		// Store summary in short event descriptor
		{
			// Increase all relevant lengths
			event_name[prog_title_length] = sum_length;
			short_event_descriptor->descriptor_length += sum_length;
			packet_length += sum_length;
			descr_ll += sum_length;
			sumText.copy( (char *) event_name+prog_title_length+1, sum_length );
		}
		else
		// Store summary in extended event descriptors
		{
			int remaining_sum_length = sumText.length();
			int nbr_descr = int(remaining_sum_length/247) + 1;
			for ( int i=0; i < nbr_descr; i++)
			// Loop once per extended event descriptor
			{
				eit_extended_descriptor_struct *ext_event_descriptor = (eit_extended_descriptor_struct *) (data + packet_length);
				sum_length = remaining_sum_length > 247 ? 247 : remaining_sum_length;
				remaining_sum_length -= sum_length;
				packet_length += 8 + sum_length;
				descr_ll += 8 + sum_length;

				ext_event_descriptor->descriptor_tag = EIT_EXTENDED_EVENT_DESCRIPOR;
				ext_event_descriptor->descriptor_length = sum_length + 6;
				ext_event_descriptor->descriptor_number = i;
				ext_event_descriptor->last_descriptor_number = nbr_descr - 1;
				ext_event_descriptor->iso_639_2_language_code_1 = 'e';
				ext_event_descriptor->iso_639_2_language_code_2 = 'n';
				ext_event_descriptor->iso_639_2_language_code_3 = 'g';
				u_char *the_text = (u_char *) ext_event_descriptor + 8;
				the_text[-2] = 0;
				the_text[-1] = sum_length;
				sumText.copy( (char *) the_text, sum_length, sumText.length() - sum_length - remaining_sum_length );
			}
		}
	}

	if (!isMHW2)
	{
		// Add content descriptor
		u_char *descriptor = (u_char *) data + packet_length;
		packet_length += 4;
		descr_ll += 4;

		int content_id = 0;
		std::string content_descr = (char *) delimitName( themes[itTitle->second.theme_id].name, name, 15 );
		if ( content_descr.find( "FILM" ) != std::string::npos )
			content_id = 0x10;
		else if ( content_descr.find( "SPORT" ) != std::string::npos )
			content_id = 0x40;

		descriptor[0] = 0x54;
		descriptor[1] = 2;
		descriptor[2] = content_id;
		descriptor[3] = 0;
	}

	event_data->descriptors_loop_length_hi = (descr_ll & 0xf00)>>8;
	event_data->descriptors_loop_length_lo = (descr_ll & 0xff);

	packet->section_length_hi =  ((packet_length - 3)&0xf00)>>8;
	packet->section_length_lo =  (packet_length - 3)&0xff;

	eEPGCache *e = eEPGCache::getInstance();
	e->sectionRead( data, eEPGCache::SCHEDULE_MHW );  // Feed the data to eEPGCache::sectionRead()
}

int eScheduleMhw::sectionRead(__u8 *data)
{
	eEPGCache *cache = eEPGCache::getInstance();

	if ( cache->state > 1 )
		return -ECANCELED;
	
	if ( ( pid == 0xD3 ) && ( tableid == 0x91 ) )
	// Channels table
	{
		int len = ((data[1]&0xf)<<8) + data[2] - 1;
		int record_size = sizeof( mhw_channel_name_t );
		int nbr_records = int (len/record_size);
		
		for ( int i = 0; i < nbr_records; i++ )
		{
			mhw_channel_name_t *channel = (mhw_channel_name_t*) &data[4 + i*record_size];
			channels.push_back( *channel );
		}

		cache->haveData |= (1 << eEPGCache::SCHEDULE_MHW);
	}
	else if ( ( pid == 0xD3 ) && ( tableid == 0x92 ) )
	// Themes table
	{
		int len = ((data[1]&0xf)<<8) + data[2] - 16;
		int record_size = sizeof( mhw_theme_name_t );
		int nbr_records = int (len/record_size);
		int idx_ptr = 0;
		__u8 next_idx = (__u8) *(data + 3 + idx_ptr);
		__u8 idx = 0;
		__u8 sub_idx = 0;
		for ( int i = 0; i < nbr_records; i++ )
		{
			mhw_theme_name_t *theme = (mhw_theme_name_t*) &data[19 + i*record_size];
			if ( i >= next_idx )
			{
				idx = (idx_ptr<<4);
				idx_ptr++;
				next_idx = (__u8) *(data + 3 + idx_ptr);
				sub_idx = 0;
			}
			else
				sub_idx++;
			
			themes[idx+sub_idx] = *theme;
		}
	}
	else if ( ( pid == 0xD2 ) && ( tableid == 0x90 ) )
	// Titles table
	{
		mhw_title_t *title = (mhw_title_t*) data;
		__u8 name[24];
		eString prog_title = (char *) delimitName( title->title, name, 23 );
		
		if ( title->channel_id == 0xFF || prog_title.substr(0,7) == "BIENTOT" )	// Separator or BIENTOT record
			return 0;	// Continue reading of the current table.
		else
		{
			// Create unique key per title
			__u32 title_id = ((title->channel_id)<<16)|((title->dh.day)<<13)|((title->dh.hours)<<8)|
				(title->ms.minutes);
			__u32 program_id = ((title->program_id_hi)<<24)|((title->program_id_mh)<<16)|
				((title->program_id_ml)<<8)|(title->program_id_lo);
			
			if ( titles.find( title_id ) == titles.end() )
			{
				tnew_title_read = time(0)+eDVB::getInstance()->time_difference;
				title->mhw2_mjd_hi = 0;
				title->mhw2_mjd_lo = 0;
				title->mhw2_duration_hi = 0;
				title->mhw2_duration_lo = 0;
				titles[ title_id ] = *title;
				if ( (title->ms.summary_available) && (program_ids.find(program_id) == program_ids.end()) )
					// program_ids will be used to gather summaries.
					program_ids[ program_id ] = title_id;
				return 0;	// Continue reading of the current table.
			}
			else
			{
				/* keep reading titles, if we got a new title less than 4 seconds ago */
				if (time(0) + eDVB::getInstance()->time_difference - tnew_title_read <= 4) return 0;
			}
		}
	}
	else if (( pid == 0xD3 ) && ( tableid == 0x90 ))
	// Summaries table
	{
		mhw_summary_t *summary = (mhw_summary_t*) data;
		
		// Create unique key per record
		__u32 program_id = ((summary->program_id_hi)<<24)|((summary->program_id_mh)<<16)|
			((summary->program_id_ml)<<8)|(summary->program_id_lo);
		int len = ((data[1]&0xf)<<8) + data[2];
		data[len+3] = 0;	// Terminate as a string.
		std::map<__u32, __u32>::iterator itProgid( program_ids.find( program_id ) );
		if ( itProgid == program_ids.end() )
		{ /*	This part is to prevent to looping forever if some summaries are not received yet.
			There is a timeout of 4 sec. after the last successfully read summary. */
			
			if ( !program_ids.empty() && 
				time(0)+eDVB::getInstance()->time_difference - tnew_summary_read <= 4 )
				return 0;	// Continue reading of the current table.
		}
		else
		{
			tnew_summary_read = time(0)+eDVB::getInstance()->time_difference;
			eString the_text = (char *) (data + 11 + summary->nb_replays * 7);
			the_text.strReplace( "\r\n", " " );
			
			// Find corresponding title, store title and summary in epgcache.
			std::map<__u32, mhw_title_t>::iterator itTitle( titles.find( itProgid->second ) );
			if ( itTitle != titles.end() )
			{
				storeTitle( itTitle, the_text, data );
				titles.erase( itTitle );
			}
			
			program_ids.erase( itProgid );
			if ( !program_ids.empty() )
				return 0;	// Continue reading of the current table.
		}
	}
	return -EAGAIN;
}

void eScheduleMhw::sectionFinish(int err)
{
	eEPGCache *e = eEPGCache::getInstance();
	if (e->isRunning & (1 << eEPGCache::SCHEDULE_MHW))
	{
		if ( ( pid == 0xD3 ) && ( tableid == 0x91 ) && ( err == -EAGAIN ) )
		{
			// Channels table has been read, start reading the themes table.
			setFilter( 0xD3, 0x92, -1, -1, SECREAD_NOTIMEOUT, 0xFF );
			return;
		}
		if ( ( pid == 0xD3 ) && ( tableid == 0x92 ) && ( err == -EAGAIN ) )
		{
			// Themes table has been read, start reading the titles table.
			setFilter( 0xD2, 0x90, -1, -1, SECREAD_NOTIMEOUT, 0xFF );
			return;
		}
		if ( ( pid == 0xD2 ) && ( tableid == 0x90 ) && ( err == -EAGAIN ) && ( !program_ids.empty() ) )
		{
			// Titles table has been read, there are summaries to read.
			// Start reading summaries, store corresponding titles on the fly.
			tnew_summary_read = time(0)+eDVB::getInstance()->time_difference;
			setFilter( 0xD3, 0x90, -1, -1, SECREAD_NOTIMEOUT, 0xFF );
			return;
		}
		if ( err == -EAGAIN )
		{
			// Summaries have been read, titles that have summaries have been stored.
			// Now store titles that do not have summaries.
			__u8 data[65];
			for (std::map<__u32, mhw_title_t>::iterator itTitle(titles.begin()); itTitle != titles.end(); itTitle++)
			{
				storeTitle( itTitle, "", data );
			}
		}
		eDebug("[EPGC] stop schedule mhw");
		e->isRunning &= ~(1 << eEPGCache::SCHEDULE_MHW);
		if (e->haveData)
			e->finishEPG(eEPGCache::SCHEDULE_MHW);
	}
	cleanup();
}
#endif

#ifdef ENABLE_DISH_EPG

eDishEEPG::eDishEEPG()
: eSection(0x300, 0x00, -1, -1, SECREAD_CRC | SECREAD_NOTIMEOUT, 0x00)
{
}

void eDishEEPG::start(const char *dmxdev)
{
	eDebug("[DISH] eDishEEPG: start");
	eSection::start(dmxdev);
}

int eDishEEPG::sectionRead(__u8 *data)
{
	eit_t *eit = (eit_t *)data;
	/* dish eepg pid doesn't use section numbering, but it uses table numbering */
	if (!eit->segment_last_table_id || eit->section_number || eit->last_section_number)
	{
		return 0;
	}
	return eEPGCache::getInstance()->sectionRead(data, eEPGCache::DISH_EEPG);
}

void eDishEEPG::sectionFinish(int err)
{
	eEPGCache *e = eEPGCache::getInstance();
	eDebug("[DISH] eDishEEPG: reader stopped (err %d) (running %d)", err, e->isRunning & (1 << eEPGCache::DISH_EEPG));
	if (e->isRunning & (1 << eEPGCache::DISH_EEPG))
	{
		e->isRunning &= ~(1 << eEPGCache::DISH_EEPG);
		if (e->haveData)
		{
			e->finishEPG(eEPGCache::DISH_EEPG);
		}
	}
}

eBevEEPG::eBevEEPG()
: eSection(0x441, 0x00, -1, -1, SECREAD_CRC | SECREAD_NOTIMEOUT, 0x00)
{
}

void eBevEEPG::start(const char *dmxdev)
{
	eDebug("[BEV] eBevEEPG: start");
	eSection::start(dmxdev);
}

int eBevEEPG::sectionRead(__u8 *data)
{
	eit_t *eit = (eit_t *)data;
	/* dish eepg pid doesn't use section numbering, but it uses table numbering */
	if (!eit->segment_last_table_id || eit->section_number || eit->last_section_number)
	{
		return 0;
	}
	return eEPGCache::getInstance()->sectionRead(data, eEPGCache::BEV_EEPG);
}

void eBevEEPG::sectionFinish(int err)
{
	eEPGCache *e = eEPGCache::getInstance();
	eDebug("[BEV] eBevEEPG: reader stopped (err %d) (running %d)", err, e->isRunning & (1 << eEPGCache::BEV_EEPG));
	if (e->isRunning & (1 << eEPGCache::BEV_EEPG))
	{
		e->isRunning &= ~(1 << eEPGCache::BEV_EEPG);
		if (e->haveData)
		{
			e->finishEPG(eEPGCache::BEV_EEPG);
		}
	}
}

#endif
