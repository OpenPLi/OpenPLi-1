/*
 * $Id: getservices.cpp,v 1.54 2002/10/14 23:54:38 obi Exp $
 */

#include <stdio.h>

#include <zapit/bouquets.h>
#include <zapit/channel.h>
#include <zapit/frontend.h>
#include <zapit/getservices.h>
#include <zapit/settings.h>
#include <zapit/xmlinterface.h>

uint8_t curr_diseqc = 0;

extern std::map <uint32_t, transponder> transponders;
extern tallchans allchans;

void ParseTransponders (XMLTreeNode *node, uint8_t DiSEqC)
{
	t_transport_stream_id transport_stream_id;
	t_original_network_id original_network_id;
	FrontendParameters feparams;
	uint8_t polarization = 0;
	uint8_t tmp;

	/* FIXME: get inversion from services list */
	feparams.Inversion = INVERSION_AUTO;

	/* read all transponders */
	while ((node != NULL) && (!strcmp(node->GetType(), "transponder")))
	{
		/* common */
		sscanf(node->GetAttributeValue("id"), "%hx", &transport_stream_id);
		sscanf(node->GetAttributeValue("onid"), "%hx", &original_network_id);
		sscanf(node->GetAttributeValue("frequency"), "%u", &feparams.Frequency);

		/* cable */
		if (DiSEqC == 0xFF)
		{
			sscanf(node->GetAttributeValue("symbol_rate"), "%u", &feparams.u.qam.SymbolRate);
			sscanf(node->GetAttributeValue("fec_inner"), "%hhu", &tmp);
			feparams.u.qam.FEC_inner = CFrontend::getFEC(tmp);
			sscanf(node->GetAttributeValue("modulation"), "%hhu", &tmp);
			feparams.u.qam.QAM = CFrontend::getModulation(tmp);
		}

		/* satellite */
		else
		{
			sscanf(node->GetAttributeValue("symbol_rate"), "%u", &feparams.u.qpsk.SymbolRate);
			sscanf(node->GetAttributeValue("fec_inner"), "%hhu", &tmp);
			feparams.u.qpsk.FEC_inner = CFrontend::getFEC(tmp);
			sscanf(node->GetAttributeValue("polarization"), "%hhu", &polarization);
		}

		/* add current transponder to list */
		transponders.insert
		(
			std::pair <uint32_t, transponder>
			(
				(transport_stream_id << 16) | original_network_id,
				transponder
				(
					transport_stream_id,
					feparams,
					polarization,
					DiSEqC,
					original_network_id
				)
			)
		);

		/* read channels that belong to the current transponder */
		ParseChannels(node->GetChild(), transport_stream_id, original_network_id, DiSEqC);

		/* hop to next transponder */
		node = node->GetNext();
	}

	return;
}

void ParseChannels (XMLTreeNode *node, t_transport_stream_id transport_stream_id, t_original_network_id original_network_id, uint8_t DiSEqC)
{
	t_service_id service_id;
	std::string  name;
	uint8_t      service_type;

	while ((node != NULL) && (!strcmp(node->GetType(), "channel")))
	{
		sscanf(node->GetAttributeValue("service_id"), "%hx", &service_id);
		name = node->GetAttributeValue("name");
		sscanf(node->GetAttributeValue("service_type"), "%hhx", &service_type);

		switch (service_type)
		{
		case DIGITAL_TELEVISION_SERVICE:
		case NVOD_REFERENCE_SERVICE:
		case NVOD_TIME_SHIFTED_SERVICE:
		case DIGITAL_RADIO_SOUND_SERVICE:
			allchans.insert
			(
				std::pair <t_channel_id, CZapitChannel>
				(
					CREATE_CHANNEL_ID,
					CZapitChannel
					(
						name,
						service_id,
						transport_stream_id,
						original_network_id,
						service_type,
						DiSEqC
					)
				)
			);

			break;

		default:
			break;
		}

		node = node->GetNext();
	}

	return;
}

void FindTransponder (XMLTreeNode *search)
{
	uint8_t DiSEqC;

	while (search)
	{
		/* cable */
		if (!(strcmp(search->GetType(), "cable")))
		{
			printf("[getservices.cpp] going to parse cable %s\n", search->GetAttributeValue("name"));
			ParseTransponders(search->GetChild(), 0xFF);
		}

		/* satellite */
		else if (!(strcmp(search->GetType(), "sat")))
		{
			printf("[getservices.cpp] going to parse satellite %s\n", search->GetAttributeValue("name"));
			sscanf(search->GetAttributeValue("diseqc"), "%hhu", &DiSEqC);
			ParseTransponders(search->GetChild(), DiSEqC);
		}

		/* hop to next satellite */
		search = search->GetNext();
	}
}

int LoadServices(void)
{
	XMLTreeParser *parser = parseXmlFile(string(SERVICES_XML));

	if (parser == NULL)
		return -1;

	FindTransponder(parser->RootNode()->GetChild());
	delete parser;
	return 0;
}

