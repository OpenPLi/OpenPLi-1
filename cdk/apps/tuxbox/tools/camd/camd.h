/*
 * $Id: camd.h,v 1.2 2002/07/17 02:14:37 obi Exp $
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

#define MAX_PIDS	4

typedef struct descrambleservice_t
{
	unsigned char valid;
	unsigned char started;
	unsigned short status;
	unsigned short onID;
	unsigned short sID;
	unsigned short Unkwn;
	unsigned short caID;
	unsigned short ecmPID;
	unsigned char numpids;
	unsigned short pid[MAX_PIDS];

} descrambleservice_s;

typedef struct ca_descriptor_s
{
	unsigned char descriptor_tag		: 8;
	unsigned char descriptor_length		: 8;
	unsigned short ca_system_id		: 16;
	unsigned char reserved			: 3;
	unsigned short ca_pid			: 13;
	unsigned char * private_data_byte;
} __attribute__ ((packed)) ca_descriptor;

typedef struct ca_pmt_program_info_s
{
	unsigned char ca_pmt_cmd_id		: 8;
	ca_descriptor * descriptor;
} __attribute__ ((packed)) ca_pmt_program_info;

typedef struct ca_pmt_es_info_s
{
	unsigned char stream_type		: 8;
	unsigned char reserved			: 3;
	unsigned short elementary_pid		: 13;
	unsigned char reserved2			: 4;
	unsigned short es_info_length		: 12;
	ca_pmt_program_info * program_info;
} __attribute__ ((packed)) ca_pmt_es_info;

typedef struct ca_pmt_s
{
	unsigned char ca_pmt_list_management	: 8;
	unsigned short program_number		: 16;
	unsigned char reserved1			: 2;
	unsigned char version_number		: 5;
	unsigned char current_next_indicator	: 1;
	unsigned char reserved2			: 4;
	unsigned short program_info_length	: 12;
	ca_pmt_program_info * program_info;
	ca_pmt_es_info * es_info;
} __attribute__ ((packed)) ca_pmt;

