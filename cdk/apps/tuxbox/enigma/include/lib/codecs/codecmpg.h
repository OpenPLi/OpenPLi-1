#ifndef DISABLE_FILE

#ifndef __lib_codecs_codecmpg_h
#define __lib_codecs_codecmpg_h

#include <map>
#include <set>
#include <lib/base/buffer.h>
#include <lib/codecs/codec.h>

struct syncAudioPacket
{
	unsigned int pts;
	int len;
	__u8 *data;
};

// mpeg-2 ps demuxer.
class eDemux: public eAudioDecoder
{
	void init_eDemux(int sourcefd);
protected:
	eIOBuffer &input, &video, &audio;
	int minFrameLength;
	int mpegtype;
	unsigned int curAudioStreamID;

	unsigned long last, remaining;
	void refill();
	unsigned long getBits(unsigned int num);
	void syncBits();
	std::map<int,int> audiostreams;
	std::set<unsigned char> videostreams;
	int synced;
	int fd;
	std::list<syncAudioPacket> syncbuffer;
	unsigned char *sheader;
	unsigned int sheader_len;
	int movie_begin;
	int movie_end;
	int readTimestamp(unsigned char* data, int len, int* seconds);
	int parseData(unsigned char *data, int len, int* seconds);
	void setCurrentTime(unsigned char* data, int len);
public:
	void extractSequenceHeader( unsigned char *buf, unsigned int len );
	eDemux(eIOBuffer &input, eIOBuffer &video, eIOBuffer &audio, int fd, int sourcefd);
	~eDemux();
	virtual int decodeMore(int last, int maxsamples, Signal1<void, unsigned int>*newastreamid=0 )=0; // returns number of samples(!) written to IOBuffer (out)
	void resync(); // clear (secondary) decoder buffers
	int getMinimumFramelength();
	int getAverageBitrate();
	eString getAudioType() { return ""; };

	void setAudioStream( unsigned int id );
};

// mpeg-2 ps demuxer.
class eMPEGDemux: public eDemux
{
public:
	eMPEGDemux(eIOBuffer &input, eIOBuffer &video, eIOBuffer &audio, int fd, int sourcefd)
		:eDemux(input, video, audio, fd, sourcefd)
	{}
	int decodeMore(int last, int maxsamples, Signal1<void, unsigned int>*newastreamid=0 ); // returns number of samples(!) written to IOBuffer (out)
};

// PVA demuxer.
class ePVADemux: public eDemux
{
public:
	ePVADemux(eIOBuffer &input, eIOBuffer &video, eIOBuffer &audio, int fd, int sourcefd)
		:eDemux(input, video, audio, fd, sourcefd)
	{}
	int decodeMore(int last, int maxsamples, Signal1<void, unsigned int>*newastreamid=0 ); // returns number of samples(!) written to IOBuffer (out)
};

#endif

#endif //DISABLE_FILE
