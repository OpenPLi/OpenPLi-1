/*
 * $Id: bat.cpp,v 1.9 2002/10/12 20:19:45 obi Exp $
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
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

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* debug */
#include <stdlib.h>

#include <zapit/bat.h>
#include <zapit/descriptors.h>
#include <zapit/dmx.h>

#define BAT_SIZE 1024

int parse_bat (int demux_fd)
{
	unsigned char buffer[BAT_SIZE];

	/* position in buffer */
	unsigned short pos;
	unsigned short pos2;
	unsigned short pos3;

	/* bouquet_association_section elements */
	unsigned short section_length;
	unsigned short bouquet_id;
	unsigned short bouquet_descriptors_length;
	unsigned short transport_stream_loop_length;
	unsigned short transport_stream_id;
	unsigned short original_network_id;
	unsigned short transport_descriptors_length;

	unsigned char filter[DMX_FILTER_SIZE];
	unsigned char mask[DMX_FILTER_SIZE];

	memset(filter, 0x00, DMX_FILTER_SIZE);
	memset(mask, 0x00, DMX_FILTER_SIZE);

	filter[0] = 0x4A;
	filter[4] = 0x00;
	mask[0] = 0xFF;
	mask[4] = 0xFF;

	do
	{
		if (setDmxSctFilter(demux_fd, 0x0011, filter, mask) < 0)
		{
			return -1;
		}

		if (read(demux_fd, buffer, BAT_SIZE) < 0)
		{
			perror("[bat.cpp] read");
			return -1;
		}

		section_length = ((buffer[1] & 0x0F) << 8) | buffer[2];
		bouquet_id = (buffer[3] << 8) | buffer[4];
		bouquet_descriptors_length = ((buffer[8] & 0x0F) << 8) | buffer[9];

		for (pos = 10; pos < bouquet_descriptors_length + 10; pos += buffer[pos + 1] + 2)
		{
			switch (buffer[pos])
			{
			case 0x47:
				bouquet_name_descriptor(buffer + pos);
				break;

			case 0x49:
				country_availability_descriptor(buffer + pos);
				break;

			case 0x4A:
				linkage_descriptor(buffer + pos);
				break;

			case 0x5F:
				private_data_specifier_descriptor(buffer + pos);
				break;

			case 0x80:
			case 0x91:
			case 0xF0:
				break;

			default:
				printf("[bat.cpp] descriptor_tag (a): %02x\n", buffer[pos]);
				generic_descriptor(buffer + pos);
				break;
			}
		}

		transport_stream_loop_length = ((buffer[pos] & 0x0F) << 8) | buffer[pos + 1];

		for (pos2 = pos + 2; pos2 < pos + 2 + transport_stream_loop_length; pos2 += transport_descriptors_length + 6)
		{
			transport_stream_id = (buffer[pos2] << 8) | buffer[pos2 + 1];
			original_network_id = (buffer[pos2 + 2] << 8) | buffer[pos2 + 3];
			transport_descriptors_length = ((buffer[pos2 + 4] & 0x0F) << 8) | buffer[pos2 + 5];

			for (pos3 = pos2 + 6; pos3 < transport_descriptors_length + pos2 + 6; pos3 += buffer[pos3 + 1] + 2)
			{
				switch (buffer[pos3])
				{
				case 0x41:
					service_list_descriptor(buffer + pos3, original_network_id);
					break;

				case 0x42:
					stuffing_descriptor(buffer + pos3);
					break;

				case 0x5F:
					private_data_specifier_descriptor(buffer + pos3);
					break;

				case 0x80:
				case 0x81:
				case 0x83:
				case 0x93:
				case 0xC9:
				case 0xD3:
					break;

				default:
					printf("[bat.cpp] descriptor_tag (b): %02x\n", buffer[pos3]);
					generic_descriptor(buffer + pos3);
					break;
				}
			}
		}
	}
	while (filter[4]++ != buffer[7]);

	return 0;
}

