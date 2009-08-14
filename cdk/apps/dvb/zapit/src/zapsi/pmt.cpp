/*
 * $Id: pmt.cpp,v 1.27 2002/10/12 20:19:45 obi Exp $
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
 * (C) 2002 by Frank Bormann <happydude@berlios.de>
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

/* system c */
#include <stdio.h>
#include <unistd.h>

/* system c++ */
#include <string>

/* zapit */
#include <zapit/descriptors.h>
#include <zapit/dmx.h>
#include <zapit/pmt.h>

#define PMT_SIZE 1024

/*
 * Stream types
 * ------------
 * 0x01 ISO/IEC 11172 Video
 * 0x02 ITU-T Rec. H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream
 * 0x03 ISO/IEC 11172 Audio
 * 0x04 ISO/IEC 13818-3 Audio
 * 0x05 ITU-T Rec. H.222.0 | ISO/IEC 13818-1 private_sections, e.g. MHP Application signalling stream
 * 0x06 ITU-T Rec. H.222.0 | ISO/IEC 13818-1 PES packets containing private data, e.g. teletext or ac3
 * 0x0b ISO/IEC 13818-6 type B
 * 0x81 User Private (MTV)
 * 0x90 User Private (Premiere Mail, BD_DVB)
 * 0xc0 User Private (Canal+)
 * 0xc1 User Private (Canal+)
 * 0xc6 User Private (Canal+)
 */

unsigned short parse_ES_info (unsigned char * buffer, CZapitChannel * channel, CCaPmt * caPmt)
{
	unsigned short ES_info_length;
	unsigned short pos;
	unsigned char descriptor_tag;
	unsigned char descriptor_length;
	unsigned char i;

	bool isAc3 = false;
	std::string description = "";
	unsigned char componentTag = 0xFF;

	/* elementary stream info for ca pmt */
	CEsInfo * esInfo = new CEsInfo();

	esInfo->stream_type = buffer[0];
	esInfo->reserved1 = buffer[1] >> 5;
	esInfo->elementary_PID = ((buffer[1] & 0x1F) << 8) | buffer[2];
	esInfo->reserved2 = buffer[3] >> 4;

	ES_info_length = ((buffer[3] & 0x0F) << 8) | buffer[4];

	for (pos = 5; pos < ES_info_length + 5; pos += descriptor_length + 2)
	{
		descriptor_tag = buffer[pos];
		descriptor_length = buffer[pos + 1];

		switch (descriptor_tag)
		{
			case 0x02:
				video_stream_descriptor(buffer + pos);
				break;

			case 0x03:
				audio_stream_descriptor(buffer + pos);
				break;

			case 0x09:
				esInfo->addCaDescriptor(buffer + pos);
				break;

			case 0x0A: /* ISO_639_language_descriptor */
				for (i = 0; i < 3; i++)
				{
					description += buffer[pos + i + 2];
				}
				break;

			case 0x13: /* Defined in ISO/IEC 13818-6 */
				break;

			case 0x0E:
				Maximum_bitrate_descriptor(buffer + pos);
				break;

			case 0x0F:
				Private_data_indicator_descriptor(buffer + pos);
				break;

			case 0x11:
				STD_descriptor(buffer + pos);
				break;

			case 0x45:
				VBI_data_descriptor(buffer + pos);
				break;

			case 0x52: /* stream_identifier_descriptor */
				componentTag = buffer[pos + 2];
				break;

			case 0x56: /* teletext descriptor */
				channel->setTeletextPid(esInfo->elementary_PID);
				break;

			case 0x59:
				subtitling_descriptor(buffer + pos);
				break;

			case 0x5F:
				private_data_specifier_descriptor(buffer + pos);
				break;

			case 0x66:
				data_broadcast_id_descriptor(buffer + pos);
				break;

			case 0x6A: /* AC3 descriptor */
				isAc3 = true;
				break;

			case 0x6F: /* unknown, Astra 19.2E */
				break;

			case 0x90: /* unknown, Astra 19.2E */
				break;

			case 0xB1: /* unknown, Astra 19.2E */
				break;

			case 0xC0: /* unknown, Astra 19.2E */
				break;

			case 0xC1: /* unknown, Astra 19.2E */
				break;

			case 0xC2: /* User Private descriptor - Canal+ */
				printf("[pmt.cpp] 0xC2 dump:\n");
				for (i = 0; i < descriptor_length; i++)
				{
					printf("%c", buffer[pos + 2 + i]);

					if (((i+1) % 8) == 0)
					{
						printf("\n");
					}
				}
				break;

			case 0xC5: /* User Private descriptor - Canal+ Radio */
				for (i = 0; i < 24; i++)
				{
					description += buffer[pos + i + 3];
				}
				break;

			case 0xC6: /* unknown, Astra 19.2E */
				break;

			case 0xFD: /* unknown, Astra 19.2E */
				break;

			case 0xFE: /* unknown, Astra 19.2E */
				break;

			default:
				printf("[pmt.cpp] descriptor_tag (b): %02x\n", descriptor_tag);
				break;
		}
	}

	switch (esInfo->stream_type)
	{
	case 0x01:
	case 0x02:
		channel->setVideoPid(esInfo->elementary_PID);
		break;

	case 0x03:
	case 0x04:
		if (description == "")
		{
			description = esInfo->elementary_PID;
		}
		channel->addAudioChannel(esInfo->elementary_PID, false, description, componentTag);
		break;

	case 0x05:
		break;

	case 0x06:
		if (isAc3)
		{
			if (description == "")
			{
				description = esInfo->elementary_PID;
				description += " (AC3)";
			}
			channel->addAudioChannel(esInfo->elementary_PID, true, description, componentTag);
		}
		break;

	case 0x0B:
		break;

	case 0x90:
		break;

	case 0x93:
		break;

	case 0xC0:
		break;

	case 0xC1:
		break;

	case 0xC6:
		break;

	default:
		printf("[pmt.cpp] stream_type: %02x\n", esInfo->stream_type);
		break;
	}

	caPmt->es_info.insert(caPmt->es_info.end(), esInfo);

	return ES_info_length;
}

int parse_pmt (int demux_fd, CZapitChannel * channel)
{
	unsigned char buffer[PMT_SIZE];

	/* current position in buffer */
	unsigned short i;

	/* length of elementary stream description */
	unsigned short ES_info_length;

	/* TS_program_map_section elements */
	unsigned short section_length;
	unsigned short program_info_length;

	unsigned char filter[DMX_FILTER_SIZE];
	unsigned char mask[DMX_FILTER_SIZE];

	memset(filter, 0x00, DMX_FILTER_SIZE);
	memset(mask, 0x00, DMX_FILTER_SIZE);

	filter[0] = 0x02;
	filter[1] = channel->getServiceId() >> 8;
	filter[2] = channel->getServiceId();
	mask[0] = 0xFF;
	mask[1] = 0xFF;
	mask[2] = 0xFF;

	if (channel->getPmtPid() == 0)
	{
		return -1;
	}

	if (setDmxSctFilter(demux_fd, channel->getPmtPid(), filter, mask) < 0)
	{
		return -1;
	}

	if (read(demux_fd, buffer, PMT_SIZE) < 0)
	{
		perror("[pmt.cpp] read");
		return -1;
	}

	CCaPmt * caPmt = new CCaPmt();

	/* ca pmt */
	caPmt->program_number = (buffer[3] << 8) + buffer[4];
	caPmt->reserved1 = buffer[5] >> 6;
	caPmt->version_number = (buffer[5] >> 1) & 0x1F;
	caPmt->current_next_indicator = buffer[5] & 0x01;
	caPmt->reserved2 = buffer[10] >> 4;

	/* pmt */
	section_length = ((buffer[1] & 0x0F) << 8) + buffer[2];
	channel->setPcrPid(((buffer[8] & 0x1F) << 8) + buffer[9]);
	program_info_length = ((buffer[10] & 0x0F) << 8) | buffer[11];

	if (program_info_length != 0)
	{
		for (i = 12; i < 12 + program_info_length; i += buffer[i + 1] + 2)
		{
			switch (buffer[i])
			{
			case 0x09:
				caPmt->addCaDescriptor(buffer + i);
				break;

			default:
				printf("[pmt.cpp] decriptor_tag (a): %02x\n", buffer[i]);
				break;
			}
		}
	}

	/* pmt */
	for (i = 12 + program_info_length; i < section_length - 1; i += ES_info_length + 5)
		ES_info_length = parse_ES_info(buffer + i, channel, caPmt);

	channel->setCaPmt(caPmt);
	channel->setPidsFlag();

	return 0;
}

