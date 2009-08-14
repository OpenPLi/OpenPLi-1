#ifndef DISABLE_FILE

#ifndef __codec_h
#define __codec_h

#include <libsig_comp.h>
#include <lib/base/estring.h>

#include <set>

class eIOBuffer;

class eAudioDecoder
{
protected:
	int speed;
	off64_t filelength;
	int sec_duration;
	int sec_currentpos;
public:
	eAudioDecoder();
	virtual ~eAudioDecoder();

	virtual int decodeMore(int last, int maxsamples, Signal1<void,unsigned int>*cb=0)=0; // returns number of samples(!) written to IOBuffer (out)
	virtual void resync()=0; // clear (secondary) decoder buffers

	struct pcmSettings
	{
		unsigned int samplerate;
		unsigned int channels;
		unsigned int format;
		int reconfigure;
	} pcmsettings;
	virtual int getMinimumFramelength()=0;
	void setSpeed(int _speed) { speed=_speed; }	
	virtual int getAverageBitrate()=0;
	virtual eString getAudioType()=0;
	int getSecondsCurrent() {return sec_currentpos;}
	int getSecondsDuration() {return sec_duration;}
};

#endif

#endif //DISABLE_FILE
