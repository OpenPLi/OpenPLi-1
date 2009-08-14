#ifndef __include_lib_dvb_timestampts_h
#define __include_lib_dvb_timestampts_h

#include <map>
#include <set>

#include <time.h>
#include <lib/dvb/service.h>

class eTimeStampParserTS
{
	typedef unsigned long long Timestamp;
	unsigned char pkt[188];
	int pktptr;
	int processPacket(const unsigned char *pkt, int type);
	inline int wantPacket(const unsigned char *hdr) const;
	int pid;
	int needNextPacket;
	tm movie_begin;
	tm movie_end;	
	tm movie_current;
	int MovieCurrentTime;
	int MovieBeginTime;
	int MovieEndTime;
	int MovieDuration;
	off64_t filelength;
	int sec_duration;
	int sec_currentpos;
	eString basefilename;
	void init_eTimeStampParserTS(const char* filename);
public:
	eTimeStampParserTS(const char* filename);
	void parseData(const void *data, unsigned int len, int type = 0);
	int getSecondsDuration() { return sec_duration;}
	int getSecondsCurrent() { return sec_currentpos; }
	int getAverageBitrate() { return (sec_duration > 0 && filelength > 0 ? filelength*8/sec_duration : -1 ); }
	void RefreshEndTime();
};
#endif
