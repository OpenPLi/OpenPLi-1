#ifndef __eventdata_h_
#define __eventdata_h_

#include <ext/hash_map>
#include "si.h"
#include "dvb.h"
#include "edvb.h"

class eventData;

struct uniqueEPGKey
{
	int sid, onid, tsid;
	uniqueEPGKey( const eServiceReferenceDVB &ref )
		:sid( ref.type != eServiceReference::idInvalid ? ref.getServiceID().get() : -1 )
		,onid( ref.type != eServiceReference::idInvalid ? ref.getOriginalNetworkID().get() : -1 )
		,tsid( ref.type != eServiceReference::idInvalid ? ref.getTransportStreamID().get() : -1 )
	{
	}
	uniqueEPGKey()
		:sid(-1), onid(-1), tsid(-1)
	{
	}
	uniqueEPGKey( int sid, int onid, int tsid )
		:sid(sid), onid(onid), tsid(tsid)
	{
	}
	bool operator <(const uniqueEPGKey &a) const
	{
		return memcmp( &sid, &a.sid, sizeof(int)*3)<0;
	}
	operator bool() const
	{ 
		return !(sid == -1 && onid == -1 && tsid == -1); 
	}
	bool operator==(const uniqueEPGKey &a) const
	{
		return !memcmp( &sid, &a.sid, sizeof(int)*3);
	}
	struct equal
	{
		bool operator()(const uniqueEPGKey &a, const uniqueEPGKey &b) const
		{
			return !memcmp( &a.sid, &b.sid, sizeof(int)*3);
		}
	};
};

//eventMap is sorted by event_id
#define eventMap std::map<__u16, eventData*>
//timeMap is sorted by beginTime
#define timeMap std::map<time_t, eventData*>
#define tmpMap std::map<uniqueEPGKey, std::pair<time_t, int> >
#define nvodMap std::map<uniqueEPGKey, std::list<NVODReferenceEntry> >

class serviceEpgInfo
{
	typedef std::map<uniqueEPGKey, eventData*> serviceMap;
	serviceMap *localMap;
public:
	serviceEpgInfo();
	~serviceEpgInfo();
	bool insert( uniqueEPGKey& key, eventData* data );
	const eventData* getInfo( uniqueEPGKey& key );
	operator bool() const {	return ( localMap != 0 ); }
};

struct hash_uniqueEPGKey
{
	inline size_t operator()( const uniqueEPGKey &x) const
	{
		return (x.tsid << 16) | x.sid;
	}
};

#define tidMap std::set<__u32>
#define descriptorPair std::pair<__u16,__u8*>

#if defined(__GNUC__) && ((__GNUC__ == 3 && __GNUC_MINOR__ >= 1) || __GNUC__ == 4 )  // check if gcc version >= 3.1
	#define eventCache __gnu_cxx::hash_map<uniqueEPGKey, std::pair<eventMap, timeMap>, hash_uniqueEPGKey, uniqueEPGKey::equal>
	#define updateMap __gnu_cxx::hash_map<uniqueEPGKey, time_t, hash_uniqueEPGKey, uniqueEPGKey::equal >
	#define descriptorMap __gnu_cxx::hash_map<__u32, descriptorPair >
	#ifdef ENABLE_PRIVATE_EPG
		#define contentTimeMap __gnu_cxx::hash_map<time_t, std::pair<time_t, __u16> >
		#define contentMap __gnu_cxx::hash_map<int, contentTimeMap >	    
		#define contentMaps __gnu_cxx::hash_map<uniqueEPGKey, contentMap, hash_uniqueEPGKey, uniqueEPGKey::equal >
	#endif
#else // for older gcc use following
	#define eventCache std::hash_map<uniqueEPGKey, std::pair<eventMap, timeMap>, hash_uniqueEPGKey, uniqueEPGKey::equal >
	#define updateMap std::hash_map<uniqueEPGKey, time_t, hash_uniqueEPGKey, uniqueEPGKey::equal >
	#define descriptorMap std::hash_map<__u32, descriptorPair, hash_descriptor >
	#ifdef ENABLE_PRIVATE_EPG
		#define contentTimeMap std::hash_map<time_t, std::pair<time_t, __u16> >
		#define contentMap std::hash_map<int, contentTimeMap >
		#define contentMaps std::hash_map<uniqueEPGKey, contentMap, hash_uniqueEPGKey, uniqueEPGKey::equal>
	#endif
#endif

class eventData
{
 	friend class eEPGMemStore;
private:
	__u8* EITdata;
public:
	__u8 type;
	static int CacheSize;
	static void load(FILE *);
	static void save(FILE *);
	eventData(const eit_event_struct* e, int size, int type);
	~eventData();
	const eit_event_struct* get() const;
	operator const eit_event_struct*() const
	{
		return get();
	}
	int getEventID()
	{
		return (EITdata[0] << 8) | EITdata[1];
	}
	time_t getStartTime()
	{
		return parseDVBtime(EITdata[2], EITdata[3], EITdata[4], EITdata[5], EITdata[6]);
	}
	int getDuration()
	{
		return fromBCD(EITdata[7])*3600+fromBCD(EITdata[8])*60+fromBCD(EITdata[9]);
	}
	int getSize();
};

#endif
