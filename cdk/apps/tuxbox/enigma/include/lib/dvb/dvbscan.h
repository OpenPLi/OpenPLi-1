#ifndef __src_lib_dvb_dvbscan_h
#define __src_lib_dvb_dvbscan_h

#include <lib/dvb/edvb.h>

#include <set>
	
class eDVBScanEvent: public eDVBEvent
{
public:
	enum
	{
		eventScanBegin=eDVBEvent::eventUser,		// -> next
		eventScanTPadded,
		eventScanNext,		// -> tune
		eventScanTuneBegin,
		eventScanTuneOK,	// tuneOK führt zu "getPAT"
		eventScanTuneError,	// tuneError führt zu ScanError
		eventScanGotPAT,	// -> Wait
		eventScanGotSDT,	// scanOK |= SDT
		eventScanGotNIT,	// scanOK |= NIT
		eventScanGotONIT,	// scanOK |= ONIT
		eventScanGotBAT,	// scanOK |= BAT
		eventScanComplete,
		eventScanError,
		eventScanCompleted
	};
	eDVBScanEvent(int event): eDVBEvent(event) { }
	eDVBScanEvent(int event, int err, eTransponder *transponder): eDVBEvent(event, err, transponder) { }
};

class eDVBScanState: public eDVBState
{
public:
	enum serviceEvent
	{
		stateScanTune=eDVBState::stateUser,		// tune ended mit "tuned" in switchedTransponder
		stateScanGetPAT,	// -> gotPAT:scanError (PATready)
		stateScanWait,
		stateScanComplete,
	};
	eDVBScanState(int state): eDVBState(state) { }
};

class eDVBScanController: public eDVBController, public Object
{
	int flags;
	
	int scanOK;	// 1 SDT, 2 NIT, 4 BAT, 8 oNIT
	int scanflags;
			// der aktuelle gescannte transponder
	eTransponder *transponder;

	void PATready(int error);
	void SDTready(int error);
	void NITready(int error);
	void ONITready(int error);
	void BATready(int error);
	void FreeCheckFinished();

	std::list<eTransponder> knownTransponder;
	std::list<eTransponder> changedTransponder;
	std::list<eTransponder>::iterator current;

	void handleSDT(const SDT *sdt);

	void freeCheckFinished();
	Signal0<void> freeCheckFinishedCallback;
	void init_eDVBScanController(eDVB &dvb);
public:
	bool abort();
	enum
	{
			// not compatible to xml-flags!
		flagNetworkSearch=1,
		flagUseBAT=2,
		flagUseONIT=4,
		flagClearList=16,
		flagSkipOtherOrbitalPositions=32,
		flagNoCircularPolarization=64,
		flagOnlyFree=128
	};

	eDVBScanController(eDVB &dvb);
	~eDVBScanController();

	void handleEvent(const eDVBEvent &event);
	unsigned int getOrbitalPosition() { return knownTransponder.front().satellite.orbital_position; }

	bool addTransponder(const eTransponder &transponder);
	int getknownTransponderSize()	{ return knownTransponder.size(); }

	void setUseONIT(int useonit);
	void setUseBAT(int usebat);
	void setNetworkSearch(int networksearch);
	void setClearList(int clearlist);
	void setSkipOtherOrbitalPositions(int skipOtherOP);
	void setNoCircularPolarization(int nocircular);
	void setOnlyFree(int onlyFree);

	void start();
};

#endif
