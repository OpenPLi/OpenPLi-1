#ifndef __lib_dvb_service_h
#define __lib_dvb_service_h

#include <map>
#include <set>
#include <libsig_comp.h>
#include <lib/dvb/dvb.h>

class eServiceReference;

class eServiceEvent
{
public:
	eServiceEvent(int type, int param): type(type), param(param)
	{
	}
	eServiceEvent(int type): type(type), param(0)
	{
	}
	enum
	{
		evtStop, 
		evtStart, 
		evtPause, 
		
		evtGotPMT,
		evtGotSDT,
		evtGotEIT,
		
		evtStatus, 		// have (new) status
		evtInfoUpdated, // updated info..
		evtAddNewAudioStreamId,

		evtStateChanged,
		evtFlagsChanged,

		evtAspectChanged,

		evtEnd,

		evtRecordFailed
	};
	int type;
	int param;
};

class eServiceCommand
{
public:
	eServiceCommand(int type): type(type) { }
	eServiceCommand(int type, int parm): type(type), parm(parm) { }
	enum
	{
		cmdRecordOpen,
		cmdRecordStart,
		cmdRecordStop,
		cmdRecordClose,

		cmdSeekBegin,
		cmdSeekEnd,
		cmdSetSpeed,		// parm : ratio.. 1 normal, 0 pause, >1 fast forward, <0 reverse (if supported)
		cmdSkip,				// parm : in ms (~)
		cmdSeekAbsolute,	// parm : percentage ~
		cmdSeekReal,			// parm : service specific, as given by queryRealPosition

		cmdRecordOpenPermanentTimeshift,
		cmdAddPermanentTimeshiftToRecording
	};
	int type;
	int parm;
};

class PMTEntry;
class eService;
class EIT;
class PMT;
class SDT;
class PMTEntry;

class eServiceHandler: public Object
{
protected:
	int id;
	static int flags;
public:
	enum
	{
		flagIsScrambled=1, 
		flagHaveSubservices=2, 
		flagHaveMultipleAudioStreams=4,
		flagIsSeekable=8,                // supports seek commands
		flagSupportPosition=16,          // supports position read
		flagIsTrack=32,                  // behaves like an audio track
		flagStartTimeshift=64
	};
	enum
	{
		statePlaying, stateError, stateScrambled, stateStopped, statePause, stateSkipping
	};
	eServiceHandler(int id);
	int getID()
	{
		return id;
	}
	virtual ~eServiceHandler();
	virtual eService *createService(const eServiceReference &node);

	virtual int play(const eServiceReference &service, int workaround=0 );

		// current service
	virtual int serviceCommand(const eServiceCommand &cmd);

		// for DVB audio channels:
	virtual PMT *getPMT();
	virtual void setPID(const PMTEntry *);

		// for DVB nvod channels:
	virtual SDT *getSDT();

		// for DVB events, nvod, audio....
	virtual EIT *getEIT();

		// for PS MPEG Files
	virtual void setAudioStream(unsigned int stream_id);

	static int getFlags();
	virtual int getState();

		// get visual flags
	virtual int getAspectRatio();

	virtual int getErrorInfo();

	virtual int stop(int workaround=0);

		// position query
	enum {
		posQueryLength,	// query length (in seconds)
		posQueryCurrent, // query current position
		posQueryRealLength, // service specific length, e.g. file length in bytes
		posQueryRealCurrent // service specific current position, e.g. file position in bytes
	};
	virtual int getPosition(int what);	// -1 means: not available
	
			// simple "info" functions..
	virtual eString getInfo(int id); // 0: status, 1+2 upper/lower line :)

	Signal1<void, const eServiceEvent &> serviceEvent;

		// service list functions
	virtual void enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback);
	virtual void leaveDirectory(const eServiceReference &dir);

	virtual eService *addRef(const eServiceReference &service);
	virtual void removeRef(const eServiceReference &service);
};

class eService;

class eServiceInterface: public Object
{
	eServiceHandler *currentServiceHandler;
	std::map<int,eServiceHandler*> handlers;
	int switchServiceHandler(int id, int workaround=0);
	
	SigC::Connection conn;
	void handleServiceEvent(const eServiceEvent &event);
	
	static eServiceInterface *instance;
public:
	eServiceInterface();
	~eServiceInterface();
	static eServiceInterface *getInstance();

	int registerHandler(int id, eServiceHandler *handler);
	int unregisterHandler(int id);
	eServiceHandler *getServiceHandler(int id);

	int play(const eServiceReference &service);
	int play(const eServiceReference &service, int workaround );

		// service related functions
	Signal1<void,const eServiceEvent &> serviceEvent;

	eServiceHandler *getService()
	{
		return currentServiceHandler;
	}

	int stop(int workaround=0);

	eServiceReference service;
		
		// service list functions
	void enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback);
	void leaveDirectory(const eServiceReference &dir);
	
		// stuff for modifiying ...

	eService *addRef(const eServiceReference &service);
	void removeRef(const eServiceReference &service);
};

#endif
