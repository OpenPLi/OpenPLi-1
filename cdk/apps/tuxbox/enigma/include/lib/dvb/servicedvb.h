#ifndef __lib_dvb_servicedvb_h
#define __lib_dvb_servicedvb_h

#include <lib/dvb/service.h>
#include <lib/dvb/servicecache.h>
#include <lib/base/thread.h>
#include <lib/base/buffer.h>
#include <lib/base/message.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/timestampts.h>

class eServiceHandlerDVB;

#ifndef DISABLE_FILE

//#define MOVIEDIR "/hdd/movie"
#define MAX_PERMANENT_TIMESHIFT_MINUTES 240

class ePermanentTimeshift
{
	std::list<std::pair<int,off64_t> > slicelist;
	std::list<std::pair<int,off64_t> >::iterator current_slice_playing;
	timeval last_split;
public:
	int IsTimeshifting;
	eLock lock;
	ePermanentTimeshift()
	: IsTimeshifting(0)
	{
	}
	void Start();
	void Stop();
	bool CheckSlice(unsigned int minutes);
	int getRecordedMinutes();
	void renameAllSlices(const char* filename);
	off64_t getCurrentLength(int slice);
	int getCurrentRecordingSlice() { return slicelist.back().first;	}
	int setNextPlayingSlice();
	int getCurrentPlayingSlice(); 
	off64_t seekTo(off64_t offset);
	eString getTimeshiftPath();
};

class eDVRPlayerThread: public eThread, public eMainloop, public Object
{
	eServiceHandlerDVB *handler;
	eIOBuffer buffer;
	enum
	{
		stateInit, stateError, stateBuffering, stateBufferFull, statePlaying, statePause, stateFileEnd
	};
	int state;
	int dvrfd;
	int sourcefd;
	int speed;
	int slice;
	int audiotracks;    // num of audiotracks in file

	int livemode; 		// used in timeshift where end-of-file is only temporary
	int playingPermanentTimeshift;
	eTimer liveupdatetimer;
	void updatePosition();

	int filelength, // in 1880 packets
		curBufferFullness; // bytes in enigma buffer...
	int pauseBufferFullness; /* buffer fullness at start of pause */
	off64_t position;
	bool needasyncworkaround;

	off64_t slicesize;
	eString filename;
	eSocketNotifier *inputsn, *outputsn;
	void readMore(int what);
	void outputReady(int what);
	int maxBufferFullness;
	int seekbusy, seeking;
	std::vector<off64_t> slicesizes;

	void dvrFlush();
	int openFile(int slice=0);
	void seekTo(off64_t offset);
	int getDriverBufferFullness();
	int FillSliceSizes();
	off64_t getCurrentSliceLength();

  	eTimeStampParserTS *timestampParser;
	void init_eDVRPlayerThread(const char *_filename);
public:
	struct eDVRPlayerThreadMessage
	{
		enum
		{
			start, startPaused, exit,
			skip,
			setSpeed, // 0..
			seek,	// 0..65536
			seekreal,
			seekmode,
			updateAudioTracks,
			addPermanentTimeshiftToRecording
		};
		int type;
		int parm;
		eDVRPlayerThreadMessage() { }
		eDVRPlayerThreadMessage(int type): type(type) { }
		eDVRPlayerThreadMessage(int type, int parm): type(type), parm(parm) { }
	};
	eFixedMessagePump<eDVRPlayerThreadMessage> messages;
	
	void gotMessage(const eDVRPlayerThreadMessage &message);
	
	eDVRPlayerThread(const char *filename, eServiceHandlerDVB *handler, int livemode, int playingPermanentTimeshift);
	~eDVRPlayerThread();

	int getPosition(int);
	int getLength(int);

	void thread();
};

#endif //DISABLE_FILE

class eServiceHandlerDVB: public eServiceHandler
{
#ifndef DISABLE_FILE
	friend class eDVRPlayerThread;
	int recording;

	struct eDVRPlayerThreadMessage
	{
		enum
		{
			done,
			liveeof,
			status
		};
		int type;
		int parm;
		eDVRPlayerThreadMessage() { }
		eDVRPlayerThreadMessage(int type): type(type) { }
		eDVRPlayerThreadMessage(int type, int status): type(type), parm(parm) { }
	};
	eFixedMessagePump<eDVRPlayerThreadMessage> messages;
	eDVRPlayerThread *decoder;
	eString current_filename;
	int playingPermanentTimeshift;

			// (u.a.) timeshift:
	void startPlayback(const eString &file, int livemode, bool startpaused = false);
	void stopPlayback(int waslivemode=0);

	void gotMessage(const eDVRPlayerThreadMessage &message);
	void handleDVBEvent( const eDVBEvent& );
#endif //DISABLE_FILE

	void scrambledStatusChanged(bool);
	void switchedService(const eServiceReferenceDVB &, int);
	void gotEIT(EIT *eit, int);
	void gotSDT(SDT *sdt);
	void gotPMT(PMT *pmt);
	void leaveService(const eServiceReferenceDVB &);
	void aspectRatioChanged(int ratio);
	int state, aspect, error;
	
	int pcrpid;

	eServiceCache<eServiceHandlerDVB> cache;
	void init_eServiceHandlerDVB();
public:
	int getID() const;
	eServiceHandlerDVB();
	~eServiceHandlerDVB();
	eService *lookupService(const eServiceReference &service);

	int play(const eServiceReference &service, int workaround=0 );

		// record	
	int serviceCommand(const eServiceCommand &cmd);

		// for DVB audio channels:
	PMT *getPMT();
	void setPID(const PMTEntry *);
	
		// for DVB nvod channels:
	SDT *getSDT();

		// for DVB events, nvod, audio....
	EIT *getEIT();
	
	int getAspectRatio();
	int getState();
	int getErrorInfo();

	int stop( int workaround = 0 );

	void loadNode(eServiceCache<eServiceHandlerDVB>::eNode &node, const eServiceReference &ref);
	eService *createService(const eServiceReference &node);
#ifndef DISABLE_FILE
	void addFile(void *node, const eString &filename);
#endif

	void enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback);
	void leaveDirectory(const eServiceReference &dir);

	eService *addRef(const eServiceReference &service);
	void removeRef(const eServiceReference &service);

	int getPosition(int what);
};

#endif
