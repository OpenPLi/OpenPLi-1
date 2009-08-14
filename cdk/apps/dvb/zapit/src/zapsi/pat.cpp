/*
 * $Id: pat.cpp,v 1.39 2002/10/12 20:19:45 obi Exp $
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org> jaja :)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* libevent */
#include <eventserver.h>

#include <zapit/client/zapitclient.h>
#include <zapit/dmx.h>
#include <zapit/pat.h>
#include <zapit/scan.h>

#define PAT_SIZE 1024

extern tallchans allchans;   //  defined in zapit.cpp
extern CEventServer * eventServer;
extern unsigned int found_channels;
extern unsigned int found_transponders;
static int status = 0;

int fake_pat (uint32_t TsidOnid, FrontendParameters feparams, uint8_t polarity, uint8_t DiSEqC)
{
	if (scantransponders.find(TsidOnid) == scantransponders.end())
	{
		status = 0;
		found_transponders++;

		eventServer->sendEvent
		(
			CZapitClient::EVT_SCAN_NUM_TRANSPONDERS,
			CEventServer::INITID_ZAPIT,
			&found_transponders,
			sizeof(found_transponders)
		);

		scantransponders.insert
		(
			std::pair <unsigned int, transpondermap>
			(
				TsidOnid,
				transpondermap
				(
					(TsidOnid >> 16),
					TsidOnid,
					feparams,
					polarity,
					DiSEqC
				)
			)
		);
	}
	else
		status = 1;
	return status;
}

int parse_pat (const int demux_fd, CZapitChannel * channel, const t_original_network_id original_network_id, const uint8_t DiSEqC)
{
	/* buffer for program association table */
	unsigned char buffer[PAT_SIZE];

	/* current positon in buffer */
	unsigned short i;

	unsigned char filter[DMX_FILTER_SIZE];
	unsigned char mask[DMX_FILTER_SIZE];

	memset(filter, 0x00, DMX_FILTER_SIZE);
	memset(mask, 0x00, DMX_FILTER_SIZE);

	mask[0] = 0xFF;
	mask[4] = 0xFF;

	do
	{
		/* set filter for program association section */
		if ((status = setDmxSctFilter(demux_fd, 0x0000, filter, mask)) < 0)
		{
			return status;
		}
		
		/* read section */
		if ((status = read(demux_fd, buffer, PAT_SIZE)) < 0)
		{
			perror("[pat.cpp] read");
			return status;
		}

		t_transport_stream_id transport_stream_id = (buffer[3] << 8) | buffer[4];

		/* loop over service id / program map table pid pairs */
		for (i = 8; i < (((buffer[1] & 0x0F) << 8) | buffer[2]) + 3; i += 4)
		{
			t_service_id service_id = (buffer[i] << 8) | buffer[i+1];

			if (channel == NULL)
			{
				if ((service_id != 0x0000) && (original_network_id != 0x0000))
				{
					tallchans_iterator I = allchans.find(CREATE_CHANNEL_ID);

					if (I == allchans.end())
					{
						char service_name[5];
						sprintf(service_name, "%hx", service_id);
						
						found_channels++;
						eventServer->sendEvent
						(
							CZapitClient::EVT_SCAN_NUM_CHANNELS,
							CEventServer::INITID_ZAPIT,
							&found_channels,
							sizeof(found_channels)
						);

						allchans.insert
						(
							std::pair <t_channel_id, CZapitChannel>
							(
								CREATE_CHANNEL_ID,
								CZapitChannel
								(
									string(service_name),
									service_id,
									transport_stream_id,
									original_network_id,
									0x00, // dummy service_type
									DiSEqC
								)
							)
						);
					}
				}
			}
			
			/* compare service id */
			else if (channel->getServiceId() == service_id)
			{
				/* store program map table pid */
				channel->setPmtPid(((buffer[i+2] & 0x1F) << 8) | buffer[i+3]);
				return 0;
			}
		}
	}
	while (filter[4]++ != buffer[7]);

	return status;
}

