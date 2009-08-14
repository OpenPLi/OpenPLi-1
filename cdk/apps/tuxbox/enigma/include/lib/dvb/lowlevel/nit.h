
/*
 * NETWORK INFORMATION TABLE
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

#ifndef __NIT_H__
#define __NIT_H__

#include <sys/types.h>

#define NIT_LEN 10 

typedef struct { 
	u_char table_id				:8; 
	
#if BYTE_ORDER == BIG_ENDIAN
	u_char section_syntax_indicator		:1; 
	u_char 					:3; 
	u_char section_length_hi		:4; 
#else 
	u_char section_length_hi		:4; 
	u_char 					:3; 
	u_char section_syntax_indicator		:1; 
#endif
	 
	u_char section_length_lo		:8; 
 
	u_char network_id_hi			:8; 
	 
	u_char network_id_lo			:8; 
 
#if BYTE_ORDER == BIG_ENDIAN
	u_char 					:2; 
	u_char version_number			:5; 
	u_char current_next_indicator		:1; 
#else
	u_char current_next_indicator		:1; 
	u_char version_number			:5; 
	u_char 					:2; 
#endif
 
	u_char section_number			:8; 
 
	u_char last_section_number		:8; 

#if BYTE_ORDER == BIG_ENDIAN 
	u_char 					:4; 
	u_char network_descriptor_length_hi	:4; 
#else
	u_char network_descriptor_length_hi	:4; 
	u_char 					:4; 
#endif
	 
	u_char network_descriptor_length_lo	:8; 
  /* descriptors */
} nit_t; 
 
#define SIZE_NIT_mid 2 
typedef struct {	// after descriptors 
#if BYTE_ORDER == BIG_ENDIAN
	u_char 					:4; 
	u_char transport_stream_loop_length_hi	:4; 
#else
	u_char transport_stream_loop_length_hi	:4; 
	u_char 					:4; 
#endif
	 
	u_char transport_stream_loop_length_lo	:8; 
} nit_mid_t; 
 
#define SIZE_NIT_end 4 
struct nit_end_struct { 
	long CRC; 
}; 
 
#define NIT_TS_LEN 6 
typedef struct { 
	u_char transport_stream_id_hi		:8; 
	u_char transport_stream_id_lo		:8; 
	u_char original_network_id_hi		:8; 
	u_char original_network_id_lo		:8; 

#if BYTE_ORDER == BIG_ENDIAN
	u_char 					:4; 
	u_char transport_descriptors_length_hi	:4; 
#else  
	u_char transport_descriptors_length_hi	:4; 
	u_char 					:4; 
#endif

	u_char transport_descriptors_length_lo	:8; 

	/* descriptors  */
} nit_ts_t;

#endif
