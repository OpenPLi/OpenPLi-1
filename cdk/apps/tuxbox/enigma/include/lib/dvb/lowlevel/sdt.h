
/*
 * SERVICE DESCRIPTION TABLE
 *
 * Copyright (C) 1998  Thomas Mirlacher
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
 * The author may be reached as dent@cosy.sbg.ac.at, or
 * Thomas Mirlacher, Jakob-Haringerstr. 2, A-5020 Salzburg,
 * Austria
 *
 *------------------------------------------------------------
 *
 */

#ifndef __SDT_H__
#define __SDT_H__

#include <sys/types.h>

#define SDT_LEN 11

typedef struct {
	u_char	table_id			: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char	section_syntax_indicator	: 1;
	u_char					: 3;
	u_char	section_length_hi		: 4;
#else
	u_char	section_length_hi		: 4;
	u_char					: 3;
	u_char	section_syntax_indicator	: 1;
#endif

	u_char	section_length_lo		: 8;

	u_char	transport_stream_id_hi		: 8;
	u_char	transport_stream_id_lo		: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 2;
	u_char	version_number			: 5;
	u_char	current_next_indicator		: 1;
#else
	u_char	current_next_indicator		: 1;
	u_char	version_number			: 5;
	u_char					: 2;
#endif

	u_char	section_number			: 8;
	u_char	last_section_number		: 8;
	u_char	original_network_id_hi		: 8;
	u_char	original_network_id_lo		: 8;
	u_char					: 8;
} sdt_t;

#define SDT_DESCR_LEN 5

struct sdt_descr_struct {
	u_char	service_id_hi			: 8;
	u_char	service_id_lo			: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 6;
	u_char	EIT_schedule_flag		: 1;
	u_char	EIT_present_following_flag	: 1;

	u_char	running_status			: 3;
	u_char	free_ca_mode			: 1;
	u_char	descriptors_loop_length_hi	: 4;
#else
	u_char	EIT_present_following_flag	: 1;
	u_char	EIT_schedule_flag		: 1;
	u_char					: 6;

	u_char	descriptors_loop_length_hi	: 4;
	u_char	free_ca_mode			: 1;
	u_char	running_status			: 3;
#endif

	u_char	descriptors_loop_length_lo	: 8;
};
typedef struct sdt_descr_struct sdt_descr_t;

#define SDT_SERVICE_DESCRIPTOR 0x48

struct sdt_generic_descriptor {
	u_char	descriptor_tag			: 8;
	u_char	descriptor_length		: 8;
};
	
struct sdt_service_descriptor_1 {
	u_char	service_type			: 8;
	u_char	service_provider_name_length	: 8;
};

struct sdt_service_descriptor_2 {
	u_char	service_name_length		: 8;
};

struct sdt_service_desc {
	u_char	description_tag			: 8;
	u_char	description_length		: 8;
	u_char	service_type			: 8;
	u_char	service_provider_name_length	: 8;	
};

#endif	
