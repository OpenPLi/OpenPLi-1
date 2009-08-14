#include <lib/dvb/eventdata.h>

#define HILO(x) (x##_hi << 8 | x##_lo)

int eventData::CacheSize=0;

eventData::eventData(const eit_event_struct* e, int size, int type)
	:EITdata(NULL), type(type&0xFF)
{
	if (!e)
		return;

	if (!size)
	{
		size = HILO(e->descriptors_loop_length) + EIT_LOOP_SIZE;
	}
	EITdata = new __u8[size];
	CacheSize+=size;
	memcpy(EITdata, (__u8*) e, size);
}

const eit_event_struct* eventData::get() const
{
	return (const eit_event_struct*)EITdata;
}

int eventData::getSize()
{
	if (!EITdata) return 0;
	return HILO(((const eit_event_struct*)EITdata)->descriptors_loop_length) + EIT_LOOP_SIZE;
}

eventData::~eventData()
{
	delete [] EITdata;
}

void eventData::load(FILE *f)
{
}

void eventData::save(FILE *f)
{
}

serviceEpgInfo::serviceEpgInfo() : localMap(0)
{
}

serviceEpgInfo::~serviceEpgInfo()
{
	if (localMap)
	{
		for ( serviceMap::iterator it( localMap->begin() ); it != localMap->end(); it++ )
			delete it->second;

		localMap->clear();
		delete localMap;
	}
}

bool serviceEpgInfo::insert( uniqueEPGKey& key, eventData* data )
{
	bool ret = true;
	
	if (!localMap)
		localMap = new serviceMap;
	
	std::pair<serviceMap::iterator,bool> insertResult = localMap->insert(std::pair<uniqueEPGKey, eventData*>( key, data) );
	if ( !insertResult.second )
	{
		delete data;  // Insert in map failed, delete allocated memory
		ret = false;
	}
	
	return ret;
}

const eventData* serviceEpgInfo::getInfo( uniqueEPGKey& key )
{
	const eventData* ret = 0;

	if ( localMap )
	{
		serviceMap::iterator it( localMap->find(key) );
		if ( it != localMap->end() )
			ret = it->second;
	}
	return ret;
}
