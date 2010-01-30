#include <lib/dvb/dvbfastscan.h>


#define HILO(x) (x##_hi << 8 | x##_lo)

//
// SCAN CONTROLLER
//
eDVBFastscanController::eDVBFastscanController(eDVB &dvb)
	: eDVBController(dvb), transponder(0)
{
	init_eDVBFastscanController(dvb);
}


void eDVBFastscanController::init_eDVBFastscanController(eDVB &dvb)
{

	CONNECT(dvb.tFastscanService.tableReady, eDVBFastscanController::FastscanServicesready);
	CONNECT(dvb.tFastscanNetwork.tableReady, eDVBFastscanController::FastscanNetworksready);
	dvb.setState(eDVBState(eDVBState::stateIdle));
}


eDVBFastscanController::~eDVBFastscanController()
{
	dvb.setState(eDVBState(eDVBState::stateIdle));
}

	
void eDVBFastscanController::handleEvent(const eDVBEvent &event)
{
	switch (event.type)
	{
		case eDVBFastscanEvent::eventFastscanBegin:
		{
			transponder = &knownTransponder.back();
			if (!transponder) {
				eDebug("[FASTSCANController] no tp to use!!!");
				dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanError));
			}
			else
			{
				if (transponder->tune())
				{
					eDebug("[FASTSCANController] tune failed because of missing infos");
					dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanError));
				} else
				{
					// eDebug("[FASTSCANController] emit eventFastscanTuneBegin");
					dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanTuneBegin, 0, transponder));
					// eDebug("[FASTSCANController] set stateFastscanTune");
					dvb.setState(eDVBFastscanState(eDVBFastscanState::stateFastscanTune));
				}
			}
			break;
		}
		case eDVBEvent::eventTunedIn:
			eDebug("[FASTSCANController] eventTunedIn");
			if (knownTransponder.size())
			{
				if (transponder && transponder == event.transponder) {
					if (event.err) {
						// eDebug("[FASTSCANController] emit eventFastscanError");
						dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanTuneError));
					}
					else {
						// eDebug("[FASTSCANController] emit eventFastscanTuneOK");
						dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanTuneOK));
					}
				}
			}
			break;
		case eDVBFastscanEvent::eventFastscanTuneError:
			eDebug("[FASTSCANController] tune failed");
			dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanError));
			break;
		case eDVBFastscanEvent::eventFastscanTuneOK:
			eDebug("[FASTSCANController] tuneOK");
			// found valid transponder
			dvb.setState(eDVBFastscanState(eDVBFastscanState::stateFastscanGetServices));
			eDebug("[FASTSCANController] start tFastscanService");
			dvb.tFastscanService.start(new FastscanService(fastscan_pid));
			break;
		case eDVBFastscanEvent::eventFastscanGotServices:
			eDebug("[FASTSCANController] eventScanGotServices");
			if (dvb.getState() != eDVBFastscanState::stateFastscanGetServices)
				eFatal("[FASTSCANController] unexpected gotServices");
			if (!dvb.tFastscanService.ready())
				eFatal("[FASTSCANController] mm trouble -> no fastscanservice");
			dvb.setState(eDVBFastscanState(eDVBFastscanState::stateFastscanGetNetworks));
			eDebug("[FASTSCANController] start tFastscanNetwork");
			dvb.tFastscanNetwork.start(new FastscanNetwork(fastscan_pid));
			break;
		case eDVBFastscanEvent::eventFastscanGotNetworks:
			eDebug("[FASTSCANController] eventScanGotNetworks");
			if (dvb.getState() != eDVBFastscanState::stateFastscanGetNetworks)
				eFatal("[FASTSCANController] unexpected gotNetworks");
			if (!dvb.tFastscanNetwork.ready())
				eFatal("[FASTSCANController] mm trouble -> no fastscannetwork");
			dvb.setState(eDVBFastscanState(eDVBFastscanState::stateFastscanComplete));
			dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanComplete));
			break;
		case eDVBFastscanEvent::eventFastscanError:
			eDebug("FASTSCANController with error");		// fall through
		case eDVBFastscanEvent::eventFastscanComplete:
			eDebug("FASTSCANController completed");
			transponder = 0;
#if 0
			// must be done affter parsing results...
			dvb.settings->saveServices();
			dvb.settings->sortInChannels();
			dvb.settings->saveBouquets();
			/*emit*/ dvb.serviceListChanged();
#endif
			dvb.setState(eDVBState(eDVBState::stateIdle));
			break;
	}
}


void eDVBFastscanController::start()
{
	dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanBegin));
}


bool eDVBFastscanController::abort()
{
	if ( dvb.getState() != eDVBState::stateIdle )
	{
		dvb.setState(eDVBState(eDVBState::stateIdle));
		dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanComplete));
	}
	return true;
}


bool eDVBFastscanController::addTransponder(const eTransponder &transponder)
{
	//eDebug("[FASTSCANcontroller] adding tp...");
	knownTransponder.push_back(transponder);
	knownTransponder.back().state = eTransponder::stateToScan;
	return true;
}


void eDVBFastscanController::FastscanServicesready(int error)
{
	if ( dvb.getState() == eDVBFastscanState::stateFastscanGetServices )
	{
		eDebug("[FASTSCANcontroller] Services scan ready rc=%d", error);
		dvb.event(eDVBFastscanEvent(error ?	eDVBFastscanEvent::eventFastscanError :
							eDVBFastscanEvent::eventFastscanGotServices));
	}
	else {
		eDebug("[FASTSCANcontroller] Servicesready but state not stateScanGetServices... ignore");
		dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanError));
	}
}


void eDVBFastscanController::FastscanNetworksready(int error)
{
	if ( dvb.getState() == eDVBFastscanState::stateFastscanGetNetworks )
	{
		eDebug("[FASTSCANcontroller] Network scan ready rc=%d", error);
		dvb.event(eDVBFastscanEvent(error?eDVBFastscanEvent::eventFastscanError:eDVBFastscanEvent::eventFastscanGotNetworks));
	}
	else
	{
		eDebug("[FASTSCANcontroller] Networksready but state not stateScanGetNetworks... ignore");
		dvb.event(eDVBFastscanEvent(eDVBFastscanEvent::eventFastscanError));
	}
}


//
// SERVICE ENTRY
//
FastscanServiceEntry::FastscanServiceEntry(const uint8_t * const buffer)
{
	originalNetworkId = r16 (&buffer[0]);
	transportStreamId = r16 (&buffer[2]);
	serviceId = r16 (&buffer[4]);
	defaultVideoPid = DVB_PID(&buffer[6]);
	defaultAudioPid = DVB_PID(&buffer[8]);
	defaultVideoEcmPid = DVB_PID(&buffer[10]);
	defaultAudioEcmPid = DVB_PID(&buffer[12]);
	defaultPcrPid = DVB_PID(&buffer[14]);
	descriptorLoopLength = DVB_LENGTH(&buffer[16]);

	uint16_t pos = 18;

	desc_tag = buffer[pos];
	desc_len = buffer[pos + 1];
	service_type = buffer[pos + 2];
	serviceProviderNameLength = buffer[pos + 3];
	serviceNameLength = buffer[pos + serviceProviderNameLength + 4];

	serviceProviderName.assign((char *)&buffer[pos + 4], serviceProviderNameLength);

	serviceName.assign((char *)&buffer[pos + serviceProviderNameLength + 5], serviceNameLength);

#if DEBUGFASTSCAN
	FILE *out = fopen("/hdd/fs.out", "a");
	if (out) {
		fprintf( out, "onid=%04x tsid=%04x sid=%04x vpid=%04x apid=%04x vecmpid=%04x aexmpid=%04x pcrpid=%04x looplen=%04x desctag=%02x desclen=%02x servtype=%02x provlen=%02x namlen=%02x prov=%s nam=%s\n",
			originalNetworkId,
			transportStreamId,
			serviceId,
			defaultVideoPid,
			defaultAudioPid,
			defaultVideoEcmPid,
			defaultAudioEcmPid,
			defaultPcrPid,
			descriptorLoopLength,
			desc_tag, 
			desc_len,
			service_type,
			serviceProviderNameLength,
			serviceNameLength,
			serviceProviderName.c_str(),
			serviceName.c_str()
		);
		fclose(out);
	}
#endif
	// inform upper about new service
	eDVB::getInstance()->getFastscanAPI()->tService(service_type);
}


FastscanService::FastscanService(int pid)
	:eTable(pid, TID_Services)
{
	m_pid = pid;
	entries.setAutoDelete(true);
#if DEBUGFASTSCAN
	FILE *out = fopen("/hdd/fs.out", "w");
	fclose(out);
	out = fopen("/hdd/fshead.out", "w");
	fprintf( out, "pid = %d\n", m_pid);
	fclose(out);
#endif
}


int FastscanService::data(__u8 *buffer)
{
	FST_t &fst=*(FST_t*)buffer;
	uint16_t sectionLength = HILO(fst.section_length);

	uint16_t pos = sizeof(FST_t);
	uint16_t bytesLeft = sectionLength > 8 ? sectionLength - 8 : 0;
	uint16_t loopLength = 0;

#if DEBUGFASTSCAN
	FILE *out = fopen("/hdd/fshead.out", "a");
	if (out) {
		fprintf( out, "tid=%d syntax=%d seclen=%d onid=%d version=%d curnex=%d secnum=%d lastsecnum=%d\n",
			fst.table_id,
			fst.section_syntax_indicator,
			HILO(fst.section_length),
			HILO(fst.operator_network_id),
			fst.version_number,
			fst.current_next_indicator,
			fst.section_number,
			fst.last_section_number
		);
		fclose(out);
	}
#endif
	while (bytesLeft > 17 && bytesLeft >= (loopLength = 18 + DVB_LENGTH(&buffer[pos+16])))
	{
		entries.push_back(new FastscanServiceEntry(&buffer[pos]));
		bytesLeft -= loopLength;
		pos += loopLength;
	}
	eDVB::getInstance()->getFastscanAPI()->tProgress(fst.section_number, fst.last_section_number);

	return 0;
}


//
// NETWORK ENTRY
//
FastscanNetworkEntry::FastscanNetworkEntry(const uint8_t * const buffer)
{
	transportStreamId = r16 (&buffer[0]);
	originalNetworkId = r16 (&buffer[2]);
	descriptorLoopLength = DVB_LENGTH(&buffer[4]);

	uint16_t pos = 6;
	uint16_t bytesLeft = descriptorLoopLength;
	uint16_t loopLength = 0;

#if DEBUGFASTSCAN
	FILE *out = fopen("/hdd/fn.out", "a");
	fprintf(out, "New network entry tsid=%d onid=%d\n", transportStreamId, originalNetworkId);
#endif
	while (bytesLeft > 1 && bytesLeft >= (loopLength = 2 + buffer[pos + 1]))
	{
		uint8_t desc_tag = buffer[pos];
		uint8_t desc_len = buffer[pos + 1];
		switch (desc_tag)
		{
		case 0x83: /* logical channel descriptor */
		{
			logicalChannels = new LogicalChannelDescriptor(&buffer[pos]);
#if DEBUGFASTSCAN
			if (out) fprintf(out, "  New logical channels\n");
			int i = 0;
			while (i < desc_len) {
				uint16_t serviceId = r16 (&buffer[pos + 2 + i]);
				uint16_t logical_channel_number = (r16 (&buffer[pos + 4 + i])) & 0x3fff;
				i += 4;
				if (out)
					fprintf( out, "    logical_channel   sid=%04x logical_channel_number=%04x\n",
						serviceId, logical_channel_number);
			}
#endif
			break;
		}
		case 0x43: //SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR:
		{
			deliverySystem = new SatelliteDeliverySystemDescriptor((descr_satellite_delivery_system_struct *) &buffer[pos]);
#if DEBUGFASTSCAN
			uint32_t frequency;
			uint16_t orbitalPosition;
			uint8_t westEastFlag;
			uint8_t polarization;
			uint8_t rollOff;
			uint8_t modulationSystem;
			uint8_t modulationType;
			uint32_t symbolRate;
			uint8_t fecInner;
			int i = 0;

			if (out) fprintf(out, "  New sat delivery systems\n");
			while (i < desc_len) {
				frequency =
					(buffer[pos + i + 2] >> 4)       * 10000000 +
					(buffer[pos + i + 2] & 0x0F)     * 1000000 +
					(buffer[pos + i + 3] >> 4)       * 100000 +
					(buffer[pos + i + 3] & 0x0F)     * 10000 +
					(buffer[pos + i + 4] >> 4)       * 1000 +
					(buffer[pos + i + 4] & 0x0F)     * 100 +
					(buffer[pos + i + 5] >> 4)       * 10 +
					(buffer[pos + i + 5] & 0x0F)     * 1
					;

				//orbitalPosition = (buffer[pos + i + 6] << 8) | buffer[pos + i + 7];
				orbitalPosition = r16 (&buffer[pos + i + 6]);

				westEastFlag     = (buffer[pos + i + 8] >> 7) & 0x01;
				polarization     = (buffer[pos + i + 8] >> 5) & 0x03;
				rollOff	  = (buffer[pos + i + 8] >> 3) & 0x03;
				modulationSystem = (buffer[pos + i + 8] >> 2) & 0x01;
				modulationType   = (buffer[pos + i + 8]) & 0x03;

				symbolRate =
					(buffer[pos + i + 9] >> 4)       * 1000000 +
					(buffer[pos + i + 9] & 0x0F)     * 100000 +
					(buffer[pos + i + 10] >> 4)      * 10000 +
					(buffer[pos + i + 10] & 0x0F)    * 1000 +
					(buffer[pos + i + 11] >> 4)      * 100 +
					(buffer[pos + i + 11] & 0x0F)    * 10 +
					(buffer[pos + i + 12] >> 4)      * 1
					;

				fecInner = buffer[pos + i + 12] & 0x0F;

				i += 11;
				if (out)
					fprintf( out, "    satellite_deliver freq=%04d orbpos=%04x WE=%d pol=%d roll=%d modsys=%d modtype=%d symrate=%08d fec=%d\n",
						frequency,
						orbitalPosition,
						westEastFlag,
						polarization,
						rollOff,
						modulationSystem,
						modulationType,
						symbolRate,
						fecInner
						);
			}
#endif
			break;
		}
		case 0x41: // SERVICE_LIST_DESCRIPTOR:
		{
			serviceList = new ServiceListDescriptor((descr_gen_t *) &buffer[pos]);
#if DEBUGFASTSCAN
			int i = 0;
			if (out) fprintf(out, "  New service lists\n");
			while (i < desc_len) {
				uint16_t serviceId = r16 (&buffer[pos + 2 + i]);
				uint8_t serviceType = buffer[pos + 4 + i];
				i += 3;
				if (out)
					fprintf( out, "    service_list_desc sid=%04x servicetype=%02x\n",
						serviceId,
						serviceType
						);
			}
#endif
			break;
		}
		}
		bytesLeft -= loopLength;
		pos += loopLength;
	}

#if DEBUGFASTSCAN
	if (out)
		fclose(out);
#endif

}


uint16_t FastscanNetworkEntry::getOriginalNetworkId(void)
{
	return originalNetworkId;
}

uint16_t FastscanNetworkEntry::getTransportStreamId(void)
{
	return transportStreamId;
}

uint16_t FastscanNetworkEntry::getOrbitalPosition(void)
{
	if (deliverySystem) return deliverySystem->orbital_position;
	return 0;
}

SatelliteDeliverySystemDescriptor * FastscanNetworkEntry::getDeliverySystem(void)
{
	if (deliverySystem) return deliverySystem;
	return 0;
}

ServiceListDescriptor * FastscanNetworkEntry::getServiceList(void)
{
	if (serviceList) return serviceList;
	return NULL;
}

const LogicalChannelList * FastscanNetworkEntry::getLogicalChannelList(void)
{
	if (logicalChannels) return logicalChannels->getChannelList();
	return NULL;
}


FastscanNetwork::FastscanNetwork(int pid)
	:eTable(pid, TID_Networks)
{
	m_pid = pid;
	entries.setAutoDelete(true);
#if DEBUGFASTSCAN
	FILE *out = fopen("/hdd/fn.out", "w");
	fclose(out);
	out = fopen("/hdd/fnhead.out", "w");
	fprintf( out, "pid = %d\n", m_pid);
	fclose(out);
#endif
}


int FastscanNetwork::data(__u8 *buffer)
{
	FST_t &fnt=*(FST_t*)buffer;

	uint16_t pos = sizeof(FST_t);
	uint16_t loopLength = 0;

	uint16_t networkdescriptorLength = DVB_LENGTH(&buffer[pos]);
	pos += 2;

	if (networkdescriptorLength >= 2) 
		networkName.assign((char *)&buffer[pos+2], networkdescriptorLength-2);
	pos += networkdescriptorLength;

	uint16_t bytesLeft = DVB_LENGTH(&buffer[pos]);
	pos += 2;
	
#if DEBUGFASTSCAN
	FILE *out = fopen("/hdd/fnhead.out", "a");
	if (out) {
		fprintf( out, "tid=%d syntax=%d seclen=%d onid=%d version=%d curnex=%d secnum=%d lastsecnum=%d namlen=%d netname=%s len=%d\n",
			fnt.table_id,
			fnt.section_syntax_indicator,
			HILO(fnt.section_length),
			HILO(fnt.operator_network_id),
			fnt.version_number,
			fnt.current_next_indicator,
			fnt.section_number,
			fnt.last_section_number,
			networkdescriptorLength,
			networkName.c_str(),
			bytesLeft
		);
		fclose(out);
	}
#endif
	while (bytesLeft > 6 && bytesLeft >= (loopLength = 6 + DVB_LENGTH(&buffer[pos+4])))
	{
		entries.push_back(new FastscanNetworkEntry(&buffer[pos]));
		bytesLeft -= loopLength;
		pos += loopLength;
	}
	eDVB::getInstance()->getFastscanAPI()->tProgress(fnt.section_number, fnt.last_section_number);

	return 0;
}
