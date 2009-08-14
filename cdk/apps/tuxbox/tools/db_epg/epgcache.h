#ifndef __epgcache_h_
#define __epgcache_h_

#include <vector>
#include <list>
#include <map>

#include "si.h"
#include "eit.h"

#define HILO(x) (x##_hi << 8 | x##_lo)

class eventData;

struct uniqueEPGKey
{
	int sid, onid, tsid;

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

#define descriptorPair std::pair<int,__u8*>
#define descriptorMap std::map<__u32, descriptorPair >


//std::map<__u32, std::pair<int,__u8*>> descriptors;
class eventData
{
private:
	__u8* EITdata;
	int ByteSize;
	static descriptorMap descriptors;
public:
	int type;
	static int CacheSize;
//	static void load(FILE *);
//	static void save(FILE *);
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
};


#endif
