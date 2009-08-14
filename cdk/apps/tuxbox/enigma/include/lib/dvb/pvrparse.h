#ifndef __include_lib_dvb_pvrparse_h
#define __include_lib_dvb_pvrparse_h

#include <map>
#include <set>

	/* This module parses TS data and collects valuable information  */
	/* about it, like PTS<->offset correlations and sequence starts. */

	/* At first, we define the collector class: */
class eMPEGStreamInformation
{
public:
	typedef unsigned long long Timestamp;
	typedef unsigned long long off_t;
		/* we order by off_t here, since the timestamp may */
		/* wrap around. */
		/* we only record sequence start's pts values here. */
	std::map<off_t, Timestamp> accessPoints;
		/* timestampDelta is in fact the difference between */
		/* the PTS in the stream and a real PTS from 0..max */
	std::map<off_t, Timestamp> timestampDeltas;
	
	int save(const char *filename);
	int load(const char *filename);
	
		/* recalculates timestampDeltas */
	void fixupDiscontinuties();
	
		/* get delta at specific offset */
	Timestamp getDelta(off_t offset);
	
		/* inter/extrapolate timestamp from offset */
	Timestamp getInterpolated(off_t offset);
	
	off_t getAccessPoint(Timestamp ts);
	
	operator bool();
};

	/* Now we define the parser's state: */
class eMPEGStreamParserTS
{
	eMPEGStreamInformation &streaminfo;
	unsigned char pkt[188];
	int pktptr;
	int processPacket(const unsigned char *pkt, eMPEGStreamInformation::off_t offset);
	inline int wantPacket(const unsigned char *hdr) const;
	int pid;
	int needNextPacket;
	int skip;
public:
	eMPEGStreamParserTS(eMPEGStreamInformation &streaminfo);
	void parseData(eMPEGStreamInformation::off_t offset, const void *data, unsigned int len);
	void setPid(int pid);
};

#endif
