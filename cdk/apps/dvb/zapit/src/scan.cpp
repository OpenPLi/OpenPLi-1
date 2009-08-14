/*
 * $Id: scan.cpp,v 1.81 2002/10/12 23:14:20 obi Exp $
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* libevent */
#include <eventserver.h>

#include <zapit/bat.h>
#include <zapit/bouquets.h>
#include <zapit/client/zapitclient.h>
#include <zapit/frontend.h>
#include <zapit/nit.h>
#include <zapit/pat.h>
#include <zapit/scan.h>
#include <zapit/sdt.h>
#include <zapit/settings.h>
#include <zapit/xmlinterface.h>

short scan_runs;
short curr_sat;
static int status = 0;



CBouquetManager* scanBouquetManager;

extern tallchans allchans;   //  defined in zapit.cpp
extern int found_transponders;
extern int found_channels;
extern std::map <t_channel_id, uint8_t> service_types;

/* zapit.cpp */
extern CFrontend *frontend;
extern XMLTreeParser *scanInputParser;
extern std::map <uint8_t, std::string> scanProviders;
extern CZapitClient::bouquetMode bouquetMode;

extern CEventServer *eventServer;

void stop_scan()
{
	/* notify client about end of scan */
	scan_runs = 0;
	eventServer->sendEvent(CZapitClient::EVT_SCAN_COMPLETE, CEventServer::INITID_ZAPIT);
	if (scanBouquetManager)
	{
		for (vector<CBouquet*>::iterator it = scanBouquetManager->Bouquets.begin(); it != scanBouquetManager->Bouquets.end(); it++)
		{
			for (vector<CZapitChannel*>::iterator jt = (*it)->tvChannels.begin(); jt != (*it)->tvChannels.end(); jt++)
				delete (*jt);
			for (vector<CZapitChannel*>::iterator jt = (*it)->radioChannels.begin(); jt != (*it)->radioChannels.end(); jt++)
				delete (*jt);
		}
		scanBouquetManager->clearAll();
		delete scanBouquetManager;
	}
}

/* build transponder for cable-users with sat-feed*/
int build_bf_transponder(uint32_t frequency, uint32_t symbol_rate, CodeRate FEC_inner, Modulation modulation)
{
	FrontendParameters feparams;
	feparams.Frequency = frequency;
	feparams.Inversion = INVERSION_AUTO;
	if (frontend->getInfo()->type == FE_QPSK)
	{
		feparams.u.qpsk.SymbolRate = symbol_rate;
		feparams.u.qpsk.FEC_inner = FEC_inner;
	}
	else
	{
		feparams.u.qam.SymbolRate = symbol_rate;
		feparams.u.qam.FEC_inner = FEC_inner;
		feparams.u.qam.QAM = modulation;
	}

	if (frontend->tuneFrequency(feparams, 0, 0) == true)
	{
		status = fake_pat(get_sdt_TsidOnid(), feparams,0,0);
	}
	else
	{
		printf("No signal found on transponder\n");
		status = -1;
	}
	return status;
}

int get_nits (uint32_t frequency, uint32_t symbol_rate, CodeRate FEC_inner, uint8_t polarization, uint8_t DiSEqC, Modulation modulation)
{
	FrontendParameters feparams;
	feparams.Frequency = frequency;
	feparams.Inversion = INVERSION_AUTO;

	if (frontend->getInfo()->type == FE_QPSK)
	{
		feparams.u.qpsk.SymbolRate = symbol_rate;
		feparams.u.qpsk.FEC_inner = FEC_inner;
	}
	else
	{
		feparams.u.qam.SymbolRate = symbol_rate;
		feparams.u.qam.FEC_inner = FEC_inner;
		feparams.u.qam.QAM = modulation;
	}

	if (frontend->tuneFrequency(feparams, polarization, DiSEqC) == true)
	{
		int tmp = found_transponders;

		if ((status = parse_nit(DiSEqC)) <= -2)
		{
			/* NIT war leer, leese TS-ID und ON-ID von der SDT aus */
			switch (status)
			{
				case -2:
					printf("[scan.cpp] NIT nicht frontendkonform, lese SDT aus\n");
					break;
				case -3:
					printf("[scan.cpp] NIT war leer, lese SDT aus\n");
					break;
				//todo weitere stati abfragen
			}

			status = fake_pat(get_sdt_TsidOnid(), feparams, polarization, DiSEqC);
			printf("[scan.cpp] TS-ON ID = %08x\n", get_sdt_TsidOnid());
		}

		if (found_transponders != tmp)
		{
			printf("found new transponder(s) on %u %u %hhu\n", frequency, symbol_rate, polarization);
		}
		else
		{
			printf("no new transponder(s) in current nit\n");
		}
	}
	else
	{
		printf("\n****************\nNo signal found on transponder (%u %u %hhu)\n****************\n\n", frequency, symbol_rate, polarization);
		status = -1;
	}

	return status;
}

int get_sdts()
{
	stiterator tI;
	int demux_fd = -1;

	//demux_fd = open(DEMUX_DEVICE, O_RDWR);
	//if (demux_fd == -1)
	//	perror(DEMUX_DEVICE);

	for (tI = scantransponders.begin(); tI != scantransponders.end(); tI++)
	{
		if (frontend->tuneFrequency(tI->second.feparams, tI->second.polarization, tI->second.DiSEqC) == true)
		{
			printf("[scan.cpp] parsing SDT (tsid:onid %04x:%04x)\n", tI->second.transport_stream_id, tI->second.original_network_id);
			status = parse_sdt(tI->second.DiSEqC);

			if (demux_fd != -1) {
				printf("[scan.cpp] parsing PAT\n");
				parse_pat(demux_fd, NULL, tI->second.original_network_id, tI->second.DiSEqC);
				printf("[scan.cpp] parsing BAT\n");
				parse_bat(demux_fd);
				printf("[scan.cpp] parsing NIT\n");
				parse_nit(tI->second.DiSEqC);
			}
		}
		else
		{
			printf("\n****************\nNo signal found on transponder (%u %u %hhu)\n****************\n\n", tI->second.feparams.Frequency, tI->second.feparams.u.qpsk.SymbolRate, tI->second.polarization);
			status = -1;
		}
	}

	if (demux_fd != -1)
		close(demux_fd);

	return status;
}

FILE *write_xml_header (const char *filename)
{
	FILE *fd = fopen(filename, "w");

	if (fd == NULL)
	{
		perror("[scan.cpp] fopen");
		stop_scan();
		pthread_exit(0);
	}
	else
	{
		fprintf(fd, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<zapit>\n");
	}

	return fd;
}

int write_xml_footer(FILE *fd)
{
	if (fd != NULL)
	{
		fprintf(fd, "</zapit>\n");
		status = fclose(fd);
	}
	else
	{
		status = -1;
	}
	return status;
}

void write_bouquets()
{
	if (bouquetMode == CZapitClient::BM_DELETEBOUQUETS)
	{
		printf("[zapit] removing existing bouquets.\n");
		unlink(BOUQUETS_XML);
	}

	else if ((bouquetMode == CZapitClient::BM_DONTTOUCHBOUQUETS))
	{
		printf("[zapit] leaving bouquets untouched.\n");
	}

	else
	{
		scanBouquetManager->saveBouquets();
	}
}

void write_transponder(FILE *fd, t_transport_stream_id transport_stream_id, t_original_network_id original_network_id, uint8_t diseqc)
{
	stiterator tI = scantransponders.find((transport_stream_id << 16) | original_network_id);

	switch (frontend->getInfo()->type)
	{
	case FE_QAM: /* cable */
		fprintf(fd,
			"\t\t<transponder id=\"%04x\" onid=\"%04x\" frequency=\"%u\" symbol_rate=\"%u\" fec_inner=\"%hhu\" modulation=\"%hhu\">\n",
			tI->second.transport_stream_id,
			tI->second.original_network_id,
			tI->second.feparams.Frequency,
			tI->second.feparams.u.qam.SymbolRate,
			tI->second.feparams.u.qam.FEC_inner,
			tI->second.feparams.u.qam.QAM);
		break;

	case FE_QPSK: /* satellite */
		fprintf(fd,
			"\t\t<transponder id=\"%04x\" onid=\"%04x\" frequency=\"%u\" symbol_rate=\"%u\" fec_inner=\"%hhu\" polarization=\"%hhu\">\n",
			tI->second.transport_stream_id,
			tI->second.original_network_id,
			tI->second.feparams.Frequency,
			tI->second.feparams.u.qpsk.SymbolRate,
			tI->second.feparams.u.qpsk.FEC_inner,
			tI->second.polarization);
		break;

	default:
		return;
	}

	for (tallchans_iterator cI = allchans.begin(); cI != allchans.end(); cI++)
	{
		if ((cI->second.getTransportStreamId() == transport_stream_id) && (cI->second.getOriginalNetworkId() == original_network_id))
		{
			if (cI->second.getName().length() == 0)
			{
				fprintf(fd,
					"\t\t\t<channel service_id=\"%04x\" name=\"%04x\" service_type=\"%02x\"/>\n",
					cI->second.getServiceId(),
					cI->second.getServiceId(),
					cI->second.getServiceType());
			}
			else
			{
				fprintf(fd,
					"\t\t\t<channel service_id=\"%04x\" name=\"%s\" service_type=\"%02x\"/>\n",
					cI->second.getServiceId(),
					convert_UTF8_To_UTF8_XML(cI->second.getName()).c_str(),
					cI->second.getServiceType());
			}
		}
	}

	fprintf(fd, "\t\t</transponder>\n");

	return;
}

FILE *write_provider(FILE *fd, const char *type, const char *provider_name, const uint8_t DiSEqC)
{
	if (!scantransponders.empty())
	{
		/* create new file if needed */
		if (fd == NULL)
		{
			fd = write_xml_header(SERVICES_XML);
		}

		/* cable tag */
		if (!strcmp(type, "cable"))
		{
			fprintf(fd, "\t<%s name=\"%s\">\n", type, provider_name);
		}

		/* satellite tag */
		else
		{
			fprintf(fd, "\t<%s name=\"%s\" diseqc=\"%hhd\">\n", type, provider_name, DiSEqC);
		}

		/* channels */
		for (stiterator tI = scantransponders.begin(); tI != scantransponders.end(); tI++)
		{
			write_transponder(fd, tI->second.transport_stream_id, tI->second.original_network_id, DiSEqC);
		}

		/* end tag */
		fprintf(fd, "\t</%s>\n", type);
	}

	/* clear results for next provider */
	allchans.clear();                  // different provider may have the same onid/sid pair // FIXME
	scantransponders.clear();

	return fd;
}

void *start_scanthread(void *param)
{
	FILE *fd = NULL;

	char providerName[32];
	char type[8];

	uint8_t diseqc_pos = 0;

	uint32_t frequency;
	uint32_t symbol_rate;
	uint8_t polarization;
	uint8_t fec_inner;
	uint8_t modulation;

	bool satfeed = false;

	scanBouquetManager = new CBouquetManager();

	curr_sat = 0;
	if ((frontend == NULL) || (frontend->isInitialized() == false))
	{
		printf("[scan.cpp] unable not scan without a frontend \n");
		stop_scan();
		pthread_exit(0);
	}

	switch (frontend->getInfo()->type)
	{
	case FE_QPSK:	/* satellite frontend */
		strcpy(type, "sat");
		modulation = 0;
		break;

	case FE_QAM:	/* cable frontend */
		strcpy(type, "cable");
		polarization = 0;
		break;

	default:	/* unsupported frontend */
		stop_scan();
		pthread_exit(0);
	}

	/* get first child */
	XMLTreeNode *search = scanInputParser->RootNode()->GetChild();
	XMLTreeNode *transponder = NULL;

	std::map <uint8_t, std::string>::iterator spI;

	/* read all sat or cable sections */
	while ((search) && (!strcmp(search->GetType(), type)))
	{
		/* get name of current satellite oder cable provider */
		strcpy(providerName, search->GetAttributeValue("name"));

		/* look whether provider is wanted */
		for (spI = scanProviders.begin(); spI != scanProviders.end(); spI++)
		{
			if (!strcmp(spI->second.c_str(), providerName))
			{
				break;
			}
		}

		/* provider is not wanted - jump to the next one */
		if (spI == scanProviders.end())
		{
			search = search->GetNext();
			continue;
		}

		/* Special mode for cable-users with sat-feed*/
		if (!strcmp(type, "cable") && search->GetAttributeValue("satfeed"))
			if (!strcmp(search->GetAttributeValue("satfeed"),"true"))
				satfeed = true;


		/* increase sat counter */
		curr_sat++;

		/* satellite tuners might need diseqc */
		if (frontend->getInfo()->type == FE_QPSK)
		{
			diseqc_pos = spI->first;
		}

		/* send sat name to client */
		eventServer->sendEvent(CZapitClient::EVT_SCAN_SATELLITE, CEventServer::INITID_ZAPIT, &providerName, strlen(providerName) + 1);
		transponder = search->GetChild();

		/* read all transponders */
		while ((transponder) && (!strcmp(transponder->GetType(), "transponder")))
		{
			/* generic */
			sscanf(transponder->GetAttributeValue("frequency"), "%u", &frequency);
			sscanf(transponder->GetAttributeValue("symbol_rate"), "%u", &symbol_rate);
			sscanf(transponder->GetAttributeValue("fec_inner"), "%hhu", &fec_inner);

			/* cable */
			if (frontend->getInfo()->type == FE_QAM)
			{
				sscanf(transponder->GetAttributeValue("modulation"), "%hhu", &modulation);
			}

			/* satellite */
			else
			{
				sscanf(transponder->GetAttributeValue("polarization"), "%hhu", &polarization);
			}

			if (!strcmp(type,"cable") && satfeed)
				/* build special transponder for cable with satfeed*/
				status = build_bf_transponder(frequency, symbol_rate, CFrontend::getFEC(fec_inner), CFrontend::getModulation(modulation));
			else
				/* read network information table */
				status = get_nits(frequency, symbol_rate, CFrontend::getFEC(fec_inner), polarization, diseqc_pos, CFrontend::getModulation(modulation));
			/* next transponder */
			transponder = transponder->GetNext();
		}

		/* 
		 * parse:
		 * service description table,
		 * program association table,
		 * bouquet association table,
		 * network information table
		 */
		status = get_sdts();

		/*
		 * channels from PAT do not have service_type set.
		 * some channels set the service_type in the BAT or the NIT.
		 * should the NIT be parsed on every transponder?
		 */
		std::map <t_channel_id, uint8_t>::iterator stI;
		for (stI = service_types.begin(); stI != service_types.end(); stI++)
		{
			tallchans_iterator scI = allchans.find(stI->first);

			if (scI != allchans.end())
			{
				if (scI->second.getServiceType() != stI->second)
				{
					printf("[scan.cpp] setting service_type of channel_id " PRINTF_CHANNEL_ID_TYPE " from %02x to %02x\n",
							stI->first,
							scI->second.getServiceType(),
							stI->second);

					scI->second.setServiceType(stI->second);
				}
			}
		}

		/* write services */
		fd = write_provider(fd, type, providerName, diseqc_pos);

		/* go to next satellite */
		search = search->GetNext();
	}

	/* clean up - should this be done before every GetNext() ? */
	delete transponder;
	delete search;

	/* close xml tags */
	if (write_xml_footer(fd) != -1)
	{
		/* write bouquets if channels did not fail */
		write_bouquets();
	}

	/* report status */
	printf("[scan.cpp] found %d transponders and %d channels\n", found_transponders, found_channels);

	/* load new services */
	CZapitClient myZapitClient;
	myZapitClient.reinitChannels();

	stop_scan();
	pthread_exit(0);
}
