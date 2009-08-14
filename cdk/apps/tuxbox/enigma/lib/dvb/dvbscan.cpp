#include <lib/dvb/dvbscan.h>

inline int isValidONIDTSID(eOriginalNetworkID onid, eTransportStreamID tsid, int orbital_position)
{
	switch( onid.get() )
	{
		case 0:
		case 0x1111:
			return 0;
		case 1:
			return orbital_position == 192;
		case 0x00B1:
			return tsid != 0x00B0;
		case 0x0002:
			return abs(orbital_position-282) < 6;
		default:
			return onid.get() < 0xFF00;
	}
}

		// work around for buggy transponders on hotbird (and maybe others)
eDVBNamespace eTransponder::buildNamespace(eOriginalNetworkID onid, eTransportStreamID tsid, int orbital_position, int freq, int pol)
{
	int dvb_namespace=orbital_position<<16;
		// on invalid ONIDs, build hash from frequency and polarisation
	if (!isValidONIDTSID(onid, tsid, orbital_position))
		dvb_namespace|=((freq/1000)&0xFFFF)|((pol&1)<<15);
	return eDVBNamespace(dvb_namespace);
}

bool eDVBScanController::abort()
{
	if ( dvb.getState() == eDVBScanState::stateScanWait )
	{
		dvb.tSDT.abort();
		dvb.tNIT.abort();
		dvb.tONIT.abort();
		dvb.tBAT.abort();
	}
	if ( dvb.getState() == eDVBScanState::stateScanGetPAT )
		dvb.tPAT.abort();
	if ( dvb.getState() != eDVBState::stateIdle )
	{
		dvb.setState(eDVBState(eDVBState::stateIdle));
		dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanCompleted));
	}
	return true;
}


eDVBScanController::eDVBScanController(eDVB &dvb)
	: eDVBController(dvb), transponder(0)
{
	init_eDVBScanController(dvb);
}
void eDVBScanController::init_eDVBScanController(eDVB &dvb)
{
	CONNECT(dvb.tPAT.tableReady, eDVBScanController::PATready);
	CONNECT(dvb.tSDT.tableReady, eDVBScanController::SDTready);
	CONNECT(dvb.tNIT.tableReady, eDVBScanController::NITready);
	CONNECT(dvb.tONIT.tableReady, eDVBScanController::ONITready);
	CONNECT(dvb.tBAT.tableReady, eDVBScanController::BATready);
	CONNECT(freeCheckFinishedCallback, eDVBScanController::freeCheckFinished );

	flags=0;
#if DEBUG_TO_FILE
	FILE *out = fopen("bla.out", "a");
	if ( !out )
		eFatal("could not open bla.out");
	fprintf( out, "Begin Transponderscan\n");
	fclose(out);
#endif
	dvb.setState(eDVBState(eDVBState::stateIdle));
}

eDVBScanController::~eDVBScanController()
{
	dvb.setState(eDVBState(eDVBState::stateIdle));
}

void eDVBScanController::handleEvent(const eDVBEvent &event)
{
	switch (event.type)
	{
	case eDVBEvent::eventTunedIn:
		eDebug("[SCAN] eventTunedIn");
		if (knownTransponder.size())
		{
			if (transponder && transponder==event.transponder)
				dvb.event(eDVBScanEvent(event.err?eDVBScanEvent::eventScanTuneError:eDVBScanEvent::eventScanTuneOK));
		}
		break;
	case eDVBScanEvent::eventScanBegin:
		eDebug("[SCAN] eventScanBegin");
		if (flags & flagClearList)
		{
			if (knownTransponder.front().satellite.isValid())
				dvb.settings->removeOrbitalPosition(knownTransponder.front().satellite.orbital_position);
			else
				dvb.settings->clearList();
		}
		current=knownTransponder.begin();
	case eDVBScanEvent::eventScanNext:
	{
		eDebug("[SCAN] eventScanNext");

		eTransponder* next = 0;

		while ( current != knownTransponder.end() && !next )
		{
			if ( current->state == eTransponder::stateToScan )
				next = &(*current);
			else
				current++;
		}

		if (!next)
			dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanCompleted));
		else
		{
			transponder=next;
			if (next->tune())
			{
				eDebug("[SCAN] tune failed because of missing infos");
				dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanError));
			} else
			{
				dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanTuneBegin, 0, next));
				dvb.setState(eDVBScanState(eDVBScanState::stateScanTune));
			}
		}
		break;
	}
	case eDVBScanEvent::eventScanTuneError:
		eDebug("[SCAN] tuned failed");
		dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanError));
		break;
	case eDVBScanEvent::eventScanTuneOK:
		eDebug("[SCAN] tuned in");
			// found valid transponder
		dvb.setState(eDVBScanState(eDVBScanState::stateScanGetPAT));
		dvb.tPAT.start(new PAT());
		break;
	case eDVBScanEvent::eventScanGotPAT:
	{
		eDebug("[SCAN] eventScanGotPAT");
		if (dvb.getState()!=eDVBScanState::stateScanGetPAT)
			eFatal("[SCAN] unexpected gotPAT");

		if (!dvb.tPAT.ready())
			eFatal("[SCAN] tmb suckt -> no pat");

		PAT *pat=dvb.tPAT.getCurrent();
		int nitpid;
		PATEntry *pe=pat->searchService(0);
		if (!pe)
		{
			eDebug("[SCAN] no NIT-PMTentry, assuming 0x10");
			nitpid=0x10;
		}	else
			nitpid=pe->program_map_PID;
		pat->unlock();
		scanOK=0;

		if (flags & flagNetworkSearch)
			dvb.tNIT.start(new NIT(nitpid));
		else
			scanOK|=2;
			
		if (flags & flagUseONIT)
			dvb.tONIT.start(new NIT(nitpid, NIT::typeOther));
		else
			scanOK|=8;

		dvb.tSDT.start(new SDT(SDT::typeBoth, pat->transport_stream_id));

		if (flags & flagUseBAT)
			;
		else
			scanOK|=4;

		dvb.setState(eDVBScanState(eDVBScanState::stateScanWait));
		break;
	}
	case eDVBScanEvent::eventScanGotSDT:
	{
		eDebug("[SCAN] eventScanGotSDT");
		dvb.setState(eDVBScanState(eDVBScanState::stateScanWait));
		SDT *sdt=dvb.tSDT.ready()?dvb.tSDT.getCurrent():0;
		if (sdt)
		{
			handleSDT(sdt);
			sdt->unlock();
		}
		else
			handleSDT(0);

		if (flags & flagUseBAT)
			dvb.tBAT.start(new BAT());

		break;
	}
	case eDVBScanEvent::eventScanGotNIT:
	case eDVBScanEvent::eventScanGotONIT:
	{
		eDebug("[SCAN] eventScanGotNIT/ONIT");
		NIT *nit=(event.type==eDVBScanEvent::eventScanGotNIT)?(dvb.tNIT.ready()?dvb.tNIT.getCurrent():0):(dvb.tONIT.ready()?dvb.tONIT.getCurrent():0);
		if (nit)
		{
			if (flags & flagNetworkSearch)
			{
#if 0
				for (ePtrList<Descriptor> i(nit->network_descriptor; i != nit->network_descriptor.end(); ++i)
				{
					if (i->Tag()==DESCR_LINKAGE)
					{
						LinkageDescriptor *l=(LinkageDescriptor*)*i;
						if ((l->linkage_type==0x01) && 		// information service
								(original_network_id==transponder->original_network_id) &&
								(transport_stream_id==transponder->transport_stream_id))
						{
							dvb.tSDT.abort();
						}
					}
				}
#endif
				for (ePtrList<NITEntry>::iterator i(nit->entries); i != nit->entries.end(); ++i)
				{
					eOriginalNetworkID onid=i->original_network_id;
					eTransportStreamID tsid=i->transport_stream_id;

					eTransponder tp(*dvb.settings->getTransponders(), -1, tsid, onid);

					for (ePtrList<Descriptor>::iterator d(i->transport_descriptor); d != i->transport_descriptor.end(); ++d)
					{
						switch (d->Tag())
						{
						case DESCR_SAT_DEL_SYS:
							tp.setSatellite((SatelliteDeliverySystemDescriptor*)*d);
							if ( abs(tp.satellite.orbital_position - knownTransponder.front().satellite.orbital_position) < 6 )
								tp.satellite.orbital_position = knownTransponder.front().satellite.orbital_position;
							break;
						case DESCR_CABLE_DEL_SYS:
							tp.setCable((CableDeliverySystemDescriptor*)*d);
							break;
						case DESCR_TERR_DEL_SYS:
							tp.setTerrestrial((TerrestrialDeliverySystemDescriptor*)*d);
							break;
						}
					}

					eDVBNamespace dvb_namespace =
						tp.satellite.isValid()
						?eTransponder::buildNamespace(onid,tsid,tp.satellite.orbital_position,tp.satellite.frequency, tp.satellite.polarisation)
						:tp.cable.isValid()
						?eTransponder::buildNamespace(onid,tsid, 0xFFFF, tp.cable.frequency, 0)
						:tp.terrestrial.isValid()
						?eTransponder::buildNamespace(onid,tsid, 0xEEEE, tp.terrestrial.centre_frequency/1000, 0)
						:-1; // should not happen

					tp.dvb_namespace=dvb_namespace;

					if (flags&flagNoCircularPolarization)
						tp.satellite.polarisation&=1;

					if ( tp.isValid() && addTransponder(tp) )
						dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanTPadded));
				}
			}
			nit->unlock();
		}

		scanOK|=(event.type==eDVBScanEvent::eventScanGotNIT)?2:8;
		eDebug("scanOK %d", scanOK);
		if (scanOK==15)
			dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanComplete));
		break;
	}
	case eDVBScanEvent::eventScanGotBAT:
	{
		eDebug("[SCAN] eventScanGotBAT");
		BAT *bat=dvb.tBAT.ready()?dvb.tBAT.getCurrent():0;
		if (bat)
		{
			dvb.settings->addDVBBouquet(transponder->dvb_namespace, bat);
			bat->unlock();
		}
		scanOK|=4;
		eDebug("scanOK %d", scanOK);
		if (scanOK==15)
			dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanComplete));
		break;
	}
	case eDVBScanEvent::eventScanError:
		eDebug("with error");		// fall through
	case eDVBScanEvent::eventScanComplete:
	{
		eDebug("completed");

//		eDebug("STATE ERROR -> freq = %i, srate = %i, pol = %i, fec = %i, svalid = %i, ycvalid = %i, onid = %i, tsid = %i, inv = %i, op = %i",transponder->satellite.frequency, transponder->satellite.symbol_rate, transponder->satellite.polarisation, transponder->satellite.fec,  transponder->satellite.valid, transponder->cable.valid, transponder->original_network_id.get(), transponder->transport_stream_id.get(), transponder->satellite.inversion, transponder->satellite.orbital_position);

		if (transponder)
			current->state = eTransponder::stateError;

		current++;
		dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanNext));
	}
		break;
	case eDVBScanEvent::eventScanCompleted:
		transponder=0;
		eDebug("scan has finally completed.");

		dvb.settings->saveServices();
		dvb.settings->sortInChannels();
		dvb.settings->saveBouquets();
		/*emit*/ dvb.serviceListChanged();

		dvb.setState(eDVBState(eDVBState::stateIdle));
		break;
	}
}

void eDVBScanController::PATready(int error)
{
	if ( dvb.getState() == eDVBScanState::stateScanGetPAT )
	{
		eDebug("[SCAN] PATready %d", error);
		dvb.event(eDVBScanEvent(error?eDVBScanEvent::eventScanError:eDVBScanEvent::eventScanGotPAT));
	}
	else
		eDebug("[SCAN] PATready but state not stateScanGetPAT... ignore");
}

void eDVBScanController::SDTready(int error)
{
	if ( dvb.getState() == eDVBScanState::stateScanWait )
	{
		eDebug("[SCAN] SDTready %d", error);
		dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanGotSDT));
	}
	else
		eDebug("[SCAN] SDTready but not stateScanWait... ignore");
}

void eDVBScanController::NITready(int error)
{
	if ( dvb.getState() == eDVBScanState::stateScanWait )
	{
		eDebug("[SCAN] NITready %d", error);
		dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanGotNIT));
	}
	else
		eDebug("[SCAN] NITready but not stateScanWait... ignore");
}

void eDVBScanController::ONITready(int error)
{
	if ( dvb.getState() == eDVBScanState::stateScanWait )
	{
		eDebug("[SCAN] ONITready %d", error);
		dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanGotONIT));
	}
	else
		eDebug("[SCAN] ONITready but not stateScanWait... ignore");
}

void eDVBScanController::BATready(int error)
{
	if ( dvb.getState() == eDVBScanState::stateScanWait )
	{
		eDebug("[SCAN] BATready %d", error);
		dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanGotBAT));
	}
	else
		eDebug("[SCAN] BATready but not stateScanWait... ignore");
}

void eDVBScanController::handleSDT(const SDT *sdt)
{
	eTransponder *old=0;
	// update tsid / onid
	eTransportStreamID tsid=sdt?sdt->transport_stream_id:transponder->transport_stream_id;
	eOriginalNetworkID onid=sdt?sdt->original_network_id:transponder->original_network_id;

	if (transponder->transport_stream_id != tsid.get() ||
			transponder->original_network_id != onid.get())
	{
		changedTransponder.push_back(*transponder);

		transponder->transport_stream_id=tsid.get();
		transponder->original_network_id=onid.get();

		old = transponder;
	}

	eDVBNamespace dvb_namespace;

		// build "namespace" to work around buggy satellites
	if (transponder->satellite.valid)
		dvb_namespace=eTransponder::buildNamespace(onid, tsid, transponder->satellite.orbital_position, transponder->satellite.frequency, transponder->satellite.polarisation);
	else if (transponder->cable.valid)
		dvb_namespace=eTransponder::buildNamespace(onid, tsid, 0xFFFF, transponder->cable.frequency, 0);
	else if (transponder->terrestrial.valid)
		dvb_namespace=eTransponder::buildNamespace(onid, tsid, 0xEEEE, transponder->terrestrial.centre_frequency/1000, 0);
	else
		dvb_namespace=0; // should not happen!

	transponder->dvb_namespace=dvb_namespace;

	eTransponder *tmp = 0;
	if ( dvb_namespace.get() & 0xFFFF )  // feeds.. scpc.. or muxxers with default values
	{
		eDebug("[SCAN] SCPC detected... compare complete transponder");
		// we must search transponder via freq pol usw..
		transponder->transport_stream_id = -1;
		transponder->original_network_id = -1;
		tmp = dvb.settings->getTransponders()->searchTransponder(*transponder);
		transponder->transport_stream_id = tsid.get();
		transponder->original_network_id = onid.get();
	}

	// ok we found the transponder, it seems to be valid
	// get Reference to the new Transponder
	eTransponder &real = tmp?*tmp:dvb.settings->getTransponders()->createTransponder(dvb_namespace, tsid, onid);
	// replace referenced transponder with new transponderdata

	if ( tmp )
	{
		dvb_namespace=tmp->dvb_namespace;
		tsid=tmp->transport_stream_id;
		onid=tmp->original_network_id;
	}
	else
		real=*transponder;

	// set transponder pointer to the adress in the TransponderList
	transponder = &real;
	// set transponder to stateOK
	transponder->state=eTransponder::stateOK;
	// set flagOnlyFree in Transponder...
	if ( flags & flagOnlyFree )
		transponder->state |= eTransponder::stateOnlyFree;

	if (old) // scan for duplicate TPs after TSID / ONID Update
		for (std::list<eTransponder>::iterator it(current); it != knownTransponder.end(); it++)
			if ( *transponder == *it && old != &(*it) )
			{
//				eDebug("set TP in handleSDT to stateError");
				it->state=eTransponder::stateError;
			}

	dvb.settings->getTransponders()->startHandleSDT(sdt,dvb_namespace,onid,tsid,&freeCheckFinishedCallback, flags&flagOnlyFree?eTransponderList::SDT_SCAN_FREE:eTransponderList::SDT_SCAN );
}

#if DEBUG_TO_FILE
static char *FEC[] = { "Auto", "1/2", "2/3", "3/4", "5/6", "7/8" };
#endif

bool eDVBScanController::addTransponder(const eTransponder &transponder)
{
#if DEBUG_TO_FILE
	FILE *out = fopen("bla.out", "a");
	if ( !out )
		eFatal("could not open bla.out");

	fprintf(out, "TOADD -> %d, %d, %c, %s, %s, %s(%d), %d onid = %d, tsid = %d\n",
		transponder.satellite.frequency, transponder.satellite.symbol_rate,
		transponder.satellite.polarisation?'H':'V', FEC[transponder.satellite.fec],
		transponder.satellite.valid?"SAT":transponder.cable.valid?"CAB":"UNK",
		!transponder.satellite.inversion?"NO":transponder.satellite.inversion==2?"AUTO":"INV",
		transponder.satellite.inversion,
		transponder.satellite.orbital_position, transponder.original_network_id.get(),
		transponder.transport_stream_id.get() );
#endif
	if ( transponder.satellite.valid &&
			 abs(transponder.satellite.orbital_position-knownTransponder.front().satellite.orbital_position) > 5
			 && flags & flagSkipOtherOrbitalPositions )
	{
#if DEBUG_TO_FILE
		fprintf(out,"Skip Transponder from other orbital position ( transponder.satellite.orbital_position = %i, knownTransponder.front().satellite.orbital_position = %i)\n", transponder.satellite.orbital_position, knownTransponder.front().satellite.orbital_position );
		fclose(out);
#endif
		return false;
	}
	for ( std::list<eTransponder>::iterator n(changedTransponder.begin()); n != changedTransponder.end(); ++n)
	{
		if (*n == transponder)
		{
#if DEBUG_TO_FILE
			fprintf(out, "This transponder has changed tsid/onid\n");
			fclose(out);
#endif
			return false;
		}
	}
	for (std::list<eTransponder>::iterator n(knownTransponder.begin()); n != knownTransponder.end(); ++n)
	{
#if DEBUG_TO_FILE
		fprintf(out,"COMPARE -> %d, %d, %c, %s, %s, %s, %d onid = %d, tsid = %d\n",
		n->satellite.frequency, n->satellite.symbol_rate,
		n->satellite.polarisation?'H':'V', FEC[n->satellite.fec],
		n->satellite.valid?"SAT":n->cable.valid?"CAB":"UNK",
		!n->satellite.inversion?"NO":n->satellite.inversion==2?"AUTO":"INV",
		n->satellite.orbital_position, n->original_network_id.get(),
		n->transport_stream_id.get() );                                      
#endif
		eTransponder tmp = transponder;
		tmp.transport_stream_id = -1;
		tmp.original_network_id = -1;
		if (*n == transponder || *n == tmp)  // no duplicate Transponders
		{
#if DEBUG_TO_FILE
			fprintf(out, "Don't add %d, %d, %c, %s, %s, %s, %d onid = %d, tsid = %d\n",
				transponder.satellite.frequency, transponder.satellite.symbol_rate,
				transponder.satellite.polarisation?'H':'V', FEC[transponder.satellite.fec],
				transponder.satellite.valid?"SAT":transponder.cable.valid?"CAB":"UNK",
				!transponder.satellite.inversion?"NO":transponder.satellite.inversion==2?"AUTO":"INV",
				transponder.satellite.orbital_position, transponder.original_network_id.get(),
				transponder.transport_stream_id.get() );
			fprintf(out, "Transponder is already in list\n");
			fclose(out);
#endif
			return false;
		}
	}
#if DEBUG_TO_FILE
	fprintf(out,"Transponder added\n");
	fprintf(out, "Add %d, %d, %c, %s, %s, %s, %d onid = %d, tsid = %d\n",
		transponder.satellite.frequency, transponder.satellite.symbol_rate,
		transponder.satellite.polarisation?'H':'V', FEC[transponder.satellite.fec],
		transponder.satellite.valid?"SAT":transponder.cable.valid?"CAB":"UNK",
		!transponder.satellite.inversion?"NO":transponder.satellite.inversion==2?"AUTO":"INV",
		transponder.satellite.orbital_position, transponder.original_network_id.get(),
		transponder.transport_stream_id.get() );
	fclose(out);
#endif

	knownTransponder.push_back(transponder);
	if ( transponder.state != eTransponder::stateError )
		knownTransponder.back().state = eTransponder::stateToScan;
	return true;
}

void eDVBScanController::setUseONIT(int useonit)
{
	if (useonit)
		flags|=flagUseONIT;
	else
		flags&=~flagUseONIT;
}

void eDVBScanController::setUseBAT(int usebat)
{
	if (usebat)
		flags|=flagUseBAT;
	else
		flags&=~flagUseBAT;
}

void eDVBScanController::setNetworkSearch(int networksearch)
{
	if (networksearch)
		flags|=flagNetworkSearch;
	else
		flags&=~flagNetworkSearch;
}

void eDVBScanController::setClearList(int clearlist)
{
	if (clearlist)
		flags|=flagClearList;
	else
		flags&=~flagClearList;
}

void eDVBScanController::setSkipOtherOrbitalPositions(int skipOtherOP)
{
	if (skipOtherOP)
		flags|=flagSkipOtherOrbitalPositions;
	else
		flags&=~flagSkipOtherOrbitalPositions;

}

void eDVBScanController::setNoCircularPolarization(int nocircular)
{
	if (nocircular)
		flags|=flagNoCircularPolarization;
	else
		flags&=~flagNoCircularPolarization;

}

void eDVBScanController::setOnlyFree(int onlyFree)
{
	if ( onlyFree )
		flags|=flagOnlyFree;
	else
		flags&=~flagOnlyFree;
}

void eDVBScanController::start()
{
	dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanBegin));
}

void eDVBScanController::freeCheckFinished()
{
	scanOK|=1;
	if (scanOK==15)
		dvb.event(eDVBScanEvent(eDVBScanEvent::eventScanComplete));
}
