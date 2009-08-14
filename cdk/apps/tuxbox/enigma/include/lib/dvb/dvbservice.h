#ifndef __src_lib_dvb_dvbservice_h
#define __src_lib_dvb_dvbservice_h

#include <lib/dvb/edvb.h>

#ifndef DISABLE_CI
class eDVBCI;
#endif

class eDVBServiceEvent: public eDVBEvent
{
public:
	enum
	{
		eventServiceSwitch=eDVBEvent::eventUser,		// -> eventServiceSwitched or eventServiceFailed
		eventServiceTuneOK,
		eventServiceTuneFailed,	
		eventServiceGotPAT,
		eventServiceGotPMT,
		eventServiceNewPIDs,
		
		eventServiceGotSDT,

		eventServiceSwitched,
		eventServiceFailed,
	};
	eDVBServiceEvent(int event): eDVBEvent(event) { }
	eDVBServiceEvent(int event, int err, eTransponder *transponder): eDVBEvent(event, err, transponder) { }
};

class eDVBServiceState: public eDVBState
{
public:
	enum serviceEvent
	{
		stateServiceTune=eDVBState::stateUser,
		stateServiceGetPAT,
		stateServiceGetPMT,
		stateServiceGetSDT,
	};
	eDVBServiceState(int state): eDVBState(state) { }
};

class eDVBCaPMTClient
{
	int lastPMTVersion;
public:
	virtual void handlePMT(const eServiceReferenceDVB &, PMT *pmt) { }
	virtual void enterService(const eServiceReferenceDVB &) { }
	virtual void leaveService(const eServiceReferenceDVB &) { }
	virtual ~eDVBCaPMTClient() { }
};

class eDVBCaPMTClientHandler
{
	static std::set<eDVBCaPMTClient*> capmtclients;
public:
	static void registerCaPMTClient( eDVBCaPMTClient* cl )
	{
		capmtclients.insert( cl );
	}

	static void unregisterCaPMTClient( eDVBCaPMTClient* cl )
	{
		capmtclients.erase( cl );
	}

	static void distribute_enterService( const eServiceReferenceDVB &service )
	{
		eServiceReferenceDVB ref = service;
		switch(ref.getServiceType())
		{
			case 4:
			case 7:
				ref.data[0]=1; // TV
				break;
		}
		for ( std::set<eDVBCaPMTClient*>::iterator it(capmtclients.begin());
			it != capmtclients.end(); ++it )
			(*it)->enterService(ref);
	}

	static void distribute_leaveService( const eServiceReferenceDVB &service )
	{
		eServiceReferenceDVB ref = service;
		switch(ref.getServiceType())
		{
			case 4:
			case 7:
				ref.data[0]=1; // TV
				break;
		}
		for ( std::set<eDVBCaPMTClient*>::iterator it(capmtclients.begin());
			it != capmtclients.end(); ++it )
			(*it)->leaveService(ref);
	}

	static void distribute_gotPMT(const eServiceReferenceDVB &service, PMT *pmt)
	{
		eServiceReferenceDVB ref = service;
		switch(ref.getServiceType())
		{
			case 4:
			case 7:
				ref.data[0]=1; // TV
				break;
		}
		for ( std::set<eDVBCaPMTClient*>::iterator it(capmtclients.begin());
			it != capmtclients.end(); ++it )
			(*it)->handlePMT(ref,pmt);
	}
};

class eDVBServiceController
	:public eDVBController, public eDVBCaPMTClient, public Object
{
	Signal0<void> freeCheckFinishedCallback;
	void freeCheckFinished();
// for linkage handling
	eServiceReferenceDVB parentservice,prevservice;
	eTimer updateTDTTimer;
	eTimer disableFrontendTimer;
#ifndef DISABLE_FILE
	void FillPIDsFromFile(eService *sp);
#endif
	void init_eDVBServiceController(eDVB &dvb);
public:
	int lastTpTimeDifference;
	int service_state;
		/* current service */
	eServiceReferenceDVB service;  // meta-service
	int spSID;
	eTransponder *transponder;
	TDT *tdt;
	bool timeSet;

	static pthread_mutex_t availCALock;

	struct CA
	{
		int casysid, ecmpid, emmpid;
	};

	std::set<int> availableCASystems, usedCASystems;
	ePtrList<CA> calist;		/** currently used ca-systems */
	int checkCA(ePtrList<CA> &list, const ePtrList<Descriptor> &descriptors, int sid);

	struct audioStream
	{
		audioStream(PMTEntry *p)
			:component_tag(-1), pmtentry(p), isAC3(0), isDTS(0)
		{}
		audioStream(const audioStream &str)
			:component_tag(str.component_tag), pmtentry(str.pmtentry)
			,text(str.text), isAC3(str.isAC3), isDTS(str.isDTS)
		{}
		int component_tag;
		PMTEntry *pmtentry;
		eString text;
		int isAC3;
		int isDTS;
	};
	std::list<audioStream> audioStreams;

	ePtrList<PMTEntry> videoStreams;
	ePtrList<PMTEntry> subtitleStreams;

#ifndef DISABLE_CI
	eDVBCI *DVBCI;
	eDVBCI *DVBCI2;
	std::map< eServiceReferenceDVB, int > CIServices;
	// for CI handling
	void handlePMT(const eServiceReferenceDVB &, PMT*);
	void enterService( const eServiceReferenceDVB &service);
	void leaveService( const eServiceReferenceDVB &service);
#else
	int *dummy[2];
	std::map< eServiceReferenceDVB, int > dummy_map;
#endif

	// set pids... detect used ca systems
	void scanPMT( PMT *pmt );
	PMTEntry *priorityAudio(PMTEntry *);

	void PATready(int error);
	void SDTready(int error);
	void PMTready(int error);
	void NITready(int error);
	void ONITready(int error);
	void EITready(int error);
	void TDTready(int error);
	void BATready(int error);

	void setPID(const PMTEntry *entry);
	void setDecoder();

	eDVBServiceController(eDVB &dvb);
	~eDVBServiceController();
	void handleEvent(const eDVBEvent &event);

	int switchService(const eServiceReferenceDVB &service); /** -> eventServiceSwitched */

	void initCAlist();
	void clearCAlist();
	void startTDT();
	void disableFrontend();
};

#endif
