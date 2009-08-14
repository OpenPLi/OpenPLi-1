
/*
 * BOUQUET ASSOCIATION TABLE
 *
 * Copyright (C) 2001  Felix Domke
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
 * The author may be reached as tmbinc@gmx.net.
 *
 *------------------------------------------------------------
 *
 */

#ifndef __BAT_H__
#define __BAT_H__

// Service Description Section
#include <sys/types.h>

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

	u_char	bouquet_id_hi			: 8;
	u_char	bouquet_id_lo			: 8;

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
#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 4;
	u_char	bouquet_descriptors_length_hi		: 4;
#else
	u_char	bouquet_descriptors_length_hi		: 4;
	u_char					: 4;
#endif
	u_char	bouquet_descriptors_length_lo : 8;
} bat_t;

#define BAT_SIZE 10

struct bat_loop_struct {
	u_char	transport_stream_id_hi			: 8;
	u_char	transport_stream_id_lo			: 8;

	u_char	original_network_id_hi			: 8;
	u_char	original_network_id_lo			: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 4;
	u_char	transport_descriptors_length_hi		: 4;
#else
	u_char	transport_descriptors_length_hi		: 4;
	u_char					: 4;
#endif
	u_char	transport_descriptors_length_lo :8;
};

#define BAT_LOOP_SIZE 6

#endif	
