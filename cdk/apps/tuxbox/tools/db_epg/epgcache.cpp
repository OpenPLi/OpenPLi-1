#include "epgcache.h"

#include <time.h>
#include "si.h"

/* Part of enigma's epgcache.cpp. Included here only to make testing a bit easier */


descriptorMap eventData::descriptors;

extern unsigned int crc32_table[256];

eventData::eventData(const eit_event_struct* e, int size, int type)
	:ByteSize(size), type(type)
{
	if (!e)
		return;
	std::list<__u32> d;
	__u8 *data = (__u8*)e;
	int ptr=10;
	int descriptors_length = (data[ptr++]&0x0F) << 8;
	descriptors_length |= data[ptr++];
	while ( descriptors_length > 0 )
	{
		int descr_len = data[ptr+1] + 2;
		__u8 *descr = new __u8[descr_len];
		unsigned int crc=0;
		int cnt=0;
		while(cnt < descr_len && descriptors_length > 0)
		{
			crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ data[ptr]) & 0xff];
			descr[cnt++] = data[ptr++];
			--descriptors_length;
		}
		descriptorMap::iterator it =
			descriptors.find(crc);
		if ( it == descriptors.end() )
		{
			//CacheSize+=descr_len;
			descriptors[crc] = descriptorPair(1, descr);
		}
		else
		{
			++it->second.first;
			delete [] descr;
		}
		d.push_back(crc);
	}
	ByteSize = 12+d.size()*4;
	EITdata = new __u8[ByteSize];
	//CacheSize+=ByteSize;
	memcpy(EITdata, (__u8*) e, 12);
	__u32 *p = (__u32*)(EITdata+12);
	for (std::list<__u32>::iterator it(d.begin()); it != d.end(); ++it)
		*p++ = *it;
}
//std::map<__u32, std::pair<int,__u8*>> descriptors;
const eit_event_struct* eventData::get() const
{
	static __u8 *data=NULL;
	if ( data )
		delete [] data;

	int bytes = 12;
	std::list<__u8*> d;

// cnt needed bytes
	int tmp = ByteSize-12;
	__u32 *p = (__u32*)(EITdata+12);
	while(tmp>0)
	{
		descriptorMap::iterator it =
			descriptors.find(*p++);
		if ( it != descriptors.end() )
		{
            d.push_back(it->second.second); //second.second  is a descriptor
            bytes += it->second.second[1]; //descriptor length
		}
		bytes += 2; // descr_type, descr_len
		tmp-=4;
	}

// copy eit_event header to buffer
	data = new __u8[bytes];
	memcpy(data, EITdata, 12);

	tmp=12;
// copy all descriptors to buffer
	for(std::list<__u8*>::iterator it(d.begin()); it != d.end(); ++it)
	{
		int b=(*it)[1]+2;
		memcpy(data+tmp, *it, b);
		tmp+=b;
	}

	return (const eit_event_struct*)data;
}

eventData::~eventData()
{
	if ( ByteSize )
	{
		//CacheSize-=ByteSize;
		ByteSize-=12;
		__u32 *d = (__u32*)(EITdata+12);
		while(ByteSize)
		{
			descriptorMap::iterator it =
				descriptors.find(*d++);
			if ( it != descriptors.end() )
			{
				descriptorPair &p = it->second;
				if (!--p.first) // no more used descriptor
				{
					//CacheSize -= p.second[1]+2;
					delete [] p.second;  	// free descriptor memory
					descriptors.erase(it);	// remove entry from descriptor map
				}
			}
			ByteSize-=4;
		}
		delete [] EITdata;
	}
}

// void eventData::load(FILE *f)
// {
// 	int size=0;
// 	int id=0;
// 	__u8 header[2];
// 	descriptorPair p;
// 	fread(&size, sizeof(int), 1, f);
// 	while(size)
// 	{
// 		fread(&id, sizeof(__u32), 1, f);
// 		fread(&p.first, sizeof(int), 1, f);
// 		fread(header, 2, 1, f);
// 		int bytes = header[1]+2;
// 		p.second = new __u8[bytes];
// 		p.second[0] = header[0];
// 		p.second[1] = header[1];
// 		fread(p.second+2, bytes-2, 1, f);
// 		descriptors[id]=p;
// 		--size;
// 		CacheSize+=bytes;
// 	}
// }
// 
// void eventData::save(FILE *f)
// {
// 	int size=descriptors.size();
// 	descriptorMap::iterator it(descriptors.begin());
// 	fwrite(&size, sizeof(int), 1, f);
// 	while(size)
// 	{
// 		fwrite(&it->first, sizeof(__u32), 1, f);
// 		fwrite(&it->second.first, sizeof(int), 1, f);
// 		fwrite(it->second.second, it->second.second[1]+2, 1, f);
// 		++it;
// 		--size;
// 	}
// }
// 
// 
// /*
// void eEPGCache::load()
// {
// 	FILE *f = fopen("/media/hdd/epg.dat", "r");
// 	if (f)
// 	{
// 		unsigned char md5_saved[16];
// 		unsigned char md5[16];
// 		int size=0;
// 		int cnt=0;
// 		bool md5ok=false;
// 		if (!md5_file("/media/hdd/epg.dat", 1, md5))
// 		{
// 			FILE *f = fopen("/media/hdd/epg.dat.md5", "r");
// 			if (f)
// 			{
// 				fread( md5_saved, 16, 1, f);
// 				fclose(f);
// 				if ( !memcmp(md5_saved, md5, 16) )
// 					md5ok=true;
// 			}
// 		}
// 		if ( md5ok )
// 		{
// 			char text1[13];
// 			fread( text1, 13, 1, f);
// 			if ( !strncmp( text1, "ENIGMA_EPG_V2", 13) )
// 			{
// 				fread( &size, sizeof(int), 1, f);
// 				while(size--)
// 				{
// 					uniqueEPGKey key;
// 					eventMap evMap;
// 					timeMap tmMap;
// 					int size=0;
// 					fread( &key, sizeof(uniqueEPGKey), 1, f);
// 					fread( &size, sizeof(int), 1, f);
// 					while(size--)
// 					{
// 						int len=0;
// 						int type=0;
// 						eventData *event=0;
// 						fread( &type, sizeof(int), 1, f);
// 						fread( &len, sizeof(int), 1, f);
// 						event = new eventData(0, len, type);
// 						event->EITdata = new __u8[len];
// 						eventData::CacheSize+=len;
// 						fread( event->EITdata, len, 1, f);
// 						evMap[ event->getEventID() ]=event;
// 						tmMap[ event->getStartTime() ]=event;
// 						++cnt;
// 					}
// 					eventDB[key]=std::pair<eventMap,timeMap>(evMap,tmMap);
// 				}
// 				eventData::load(f);
// 				eDebug("%d events read from /media/hdd/epg.dat", cnt);
// #ifdef ENABLE_PRIVATE_EPG
// 				char text2[11];
// 				fread( text2, 11, 1, f);
// 				if ( !strncmp( text2, "PRIVATE_EPG", 11) )
// 				{
// 					size=0;
// 					fread( &size, sizeof(int), 1, f);
// 					while(size--)
// 					{
// 						int size=0;
// 						uniqueEPGKey key;
// 						fread( &key, sizeof(uniqueEPGKey), 1, f);
// 						fread( &size, sizeof(int), 1, f);
// 						while(size--)
// 						{
// 							int size;
// 							int content_id;
// 							fread( &content_id, sizeof(int), 1, f);
// 							fread( &size, sizeof(int), 1, f);
// 							while(size--)
// 							{
// 								time_t time1, time2;
// 								__u16 event_id;
// 								fread( &time1, sizeof(time_t), 1, f);
// 								fread( &time2, sizeof(time_t), 1, f);
// 								fread( &event_id, sizeof(__u16), 1, f);
// 								content_time_tables[key][content_id][time1]=std::pair<time_t, __u16>(time2, event_id);
// 							}
// 						}
// 					}
// 				}
// #endif // ENABLE_PRIVATE_EPG
// 			}
// 			else
// 				eDebug("[EPGC] don't read old epg database");
// 			fclose(f);
// 		}
// 	}
// }
// 
// void eEPGCache::save()
// {
// 	struct statfs s;
// 	off64_t tmp;
// 	if (statfs("/media/hdd", &s)<0)
// 		tmp=0;
// 	else
// 	{
// 		tmp=s.f_blocks;
// 		tmp*=s.f_bsize;
// 	}
// 
// 	// prevent writes to builtin flash
// 	if ( tmp < 1024*1024*50 ) // storage size < 50MB
// 		return;
// 
// 	// check for enough free space on storage
// 	tmp=s.f_bfree;
// 	tmp*=s.f_bsize;
// 	if ( tmp < (eventData::CacheSize*12)/10 ) // 20% overhead
// 		return;
// 
// 	FILE *f = fopen("/media/hdd/epg.dat", "w");
// 	int cnt=0;
// 	if ( f )
// 	{
// 		const char *text = "ENIGMA_EPG_V2";
// 		fwrite( text, 13, 1, f );
// 		int size = eventDB.size();
// 		fwrite( &size, sizeof(int), 1, f );
// 		for (eventCache::iterator service_it(eventDB.begin()); service_it != eventDB.end(); ++service_it)
// 		{
// 			timeMap &timemap = service_it->second.second;
// 			fwrite( &service_it->first, sizeof(uniqueEPGKey), 1, f);
// 			size = timemap.size();
// 			fwrite( &size, sizeof(int), 1, f);
// 			for (timeMap::iterator time_it(timemap.begin()); time_it != timemap.end(); ++time_it)
// 			{
// 				int len = time_it->second->ByteSize;
// 				fwrite( &time_it->second->type, sizeof(int), 1, f );
// 				fwrite( &len, sizeof(int), 1, f);
// 				fwrite( time_it->second->EITdata, len, 1, f);
// 				++cnt;
// 			}
// 		}
// 		eDebug("%d events written to /media/hdd/epg.dat", cnt);
// 		eventData::save(f);
// #ifdef ENABLE_PRIVATE_EPG
// 		const char* text2 = "PRIVATE_EPG";
// 		fwrite( text2, 11, 1, f );
// 		size = content_time_tables.size();
// 		fwrite( &size, sizeof(int), 1, f);
// 		for (contentMaps::iterator a = content_time_tables.begin(); a != content_time_tables.end(); ++a)
// 		{
// 			contentMap &content_time_table = a->second;
// 			fwrite( &a->first, sizeof(uniqueEPGKey), 1, f);
// 			int size = content_time_table.size();
// 			fwrite( &size, sizeof(int), 1, f);
// 			for (contentMap::iterator i = content_time_table.begin(); i != content_time_table.end(); ++i )
// 			{
// 				int size = i->second.size();
// 				fwrite( &i->first, sizeof(int), 1, f);
// 				fwrite( &size, sizeof(int), 1, f);
// 				for ( std::map<time_t, std::pair<time_t, __u16> >::iterator it(i->second.begin());
// 					it != i->second.end(); ++it )
// 				{
// 					fwrite( &it->first, sizeof(time_t), 1, f);
// 					fwrite( &it->second.first, sizeof(time_t), 1, f);
// 					fwrite( &it->second.second, sizeof(__u16), 1, f);
// 				}
// 			}
// 		}
// #endif
// 		fclose(f);
// 		unsigned char md5[16];
// 		if (!md5_file("/media/hdd/epg.dat", 1, md5))
// 		{
// 			FILE *f = fopen("/media/hdd/epg.dat.md5", "w");
// 			if (f)
// 			{
// 				fwrite( md5, 16, 1, f);
// 				fclose(f);
// 			}
// 		}
// 	}
// }*/
// 
