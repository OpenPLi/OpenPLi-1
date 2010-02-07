#ifndef __src_lib_dvb_dvbfastscan_h
#define __src_lib_dvb_dvbfastscan_h

#include <lib/dvb/edvb.h>

#include <lib/dvb/dvbfastscanlow.h>
#include <lib/dvb/logicalchanneldescriptor.h>
#define PID_Fastscan 900
#define TID_Services 0xBD
#define TID_Networks 0xBC

class FastscanService;
class FastscanNetwork;

class eDVBFastscanEvent: public eDVBEvent
{
public:
	enum
	{
		eventFastscanBegin=eDVBEvent::eventUser,	// -> next
		eventFastscanTuneBegin,
		eventFastscanTuneOK,	
		eventFastscanTuneError,
		eventFastscanGotServices,
		eventFastscanGotNetworks,
		eventFastscanComplete,
		eventFastscanError
	};
	eDVBFastscanEvent(int event): eDVBEvent(event) { }
	eDVBFastscanEvent(int event, int err, eTransponder *transponder): eDVBEvent(event, err, transponder) { }
};


class eDVBFastscanState: public eDVBState
{
public:
	enum serviceEvent
	{
		stateFastscanTune=eDVBState::stateUser,
		stateFastscanGetServices,
		stateFastscanGetNetworks,
		stateFastscanComplete
	};
	eDVBFastscanState(int state): eDVBState(state) { }
};


class eDVBFastscanController: public eDVBController, public Object
{
	uint16_t fastscan_pid;
	bool fastscan_usenum;
	bool fastscan_usename;
	eString fastscan_provider;
	eTransponder *transponder; // the transponder to use for scan

	std::list<eTransponder> knownTransponder;
	std::list<eTransponder>::iterator current;

	void FastscanServicesready(int error);
	void FastscanNetworksready(int error);

	void init_eDVBFastscanController(eDVB &dvb);
public:
	enum
	{
		flagNetworkSearch=1,
		flagUseBAT=2,
		flagUseONIT=4,
		flagClearList=16,
		flagSkipOtherOrbitalPositions=32,
		flagNoCircularPolarization=64,
		flagOnlyFree=128
	};

	bool addTransponder(const eTransponder &transponder);
	void setPID(uint16_t pid) { fastscan_pid = pid; }
	void setUseNum(bool num) { fastscan_usenum = num; }
	void setUseName(bool name) { fastscan_usename = name; }
	void setProviderName(eString name) { fastscan_provider = name; }
	eString getProviderName() { return fastscan_provider; }
	bool getUseNum() { return fastscan_usenum; }
	bool getUseName() { return fastscan_usename; }

	eAUTable<FastscanService> tFastscanService;
	eAUTable<FastscanNetwork> tFastscanNetwork;

	Signal2<void, int, int> tProgress;
	Signal1<void, int> tService;

	eDVBFastscanController(eDVB &dvb);
	~eDVBFastscanController();

	void handleEvent(const eDVBEvent &event);

	void start();
	bool abort();
};


class FastscanServiceEntry // : public Descriptor
{
//protected:
public:
	unsigned originalNetworkId : 16;
	unsigned transportStreamId : 16;
	unsigned serviceId : 16;
	unsigned defaultVideoPid : 16;
	unsigned defaultAudioPid : 16;
	unsigned defaultVideoEcmPid : 16;
	unsigned defaultAudioEcmPid : 16;
	unsigned defaultPcrPid : 16;
	unsigned descriptorLoopLength : 16;

	unsigned desc_tag : 8;
	unsigned desc_len : 8;
	unsigned service_type : 8;
	unsigned serviceProviderNameLength : 8;
	unsigned serviceNameLength : 8;

	eString serviceProviderName;
	eString serviceName;

public:
	FastscanServiceEntry(const uint8_t *const buffer);
	~FastscanServiceEntry() {};
};


class FastscanService: public eTable
{
	uint16_t m_pid;
protected:
	int data(__u8 *data);
public:
	FastscanService(int pid);
	~FastscanService() {};
	Signal2<void, int, int> tableProgress;

	ePtrList<FastscanServiceEntry> entries;
};


class FastscanNetworkEntry // : public Descriptor
{
protected:
	unsigned transportStreamId : 16;
	unsigned originalNetworkId : 16;
	unsigned descriptorLoopLength : 16;

	SatelliteDeliverySystemDescriptor *deliverySystem;
	ServiceListDescriptor *serviceList;
	LogicalChannelDescriptor *logicalChannels;

public:
	FastscanNetworkEntry(const uint8_t *const buffer);
	~FastscanNetworkEntry() {};

	uint16_t getOriginalNetworkId(void);
	uint16_t getTransportStreamId(void);
	uint16_t getOrbitalPosition(void);
	SatelliteDeliverySystemDescriptor * getDeliverySystem(void);
	ServiceListDescriptor * getServiceList(void);
	const LogicalChannelList * getLogicalChannelList(void);
};


class FastscanNetwork: public eTable
{
	uint16_t m_pid;
protected:
	int data(__u8 *data);
	eString networkName;
public:
	FastscanNetwork(int pid);
	~FastscanNetwork() {};
	Signal2<void, int, int> tableProgress;

	ePtrList<FastscanNetworkEntry> entries;
};
#endif
