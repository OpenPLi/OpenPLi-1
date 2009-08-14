
/*
 * PROGRAM MAP TABLE
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

#ifndef __PMT_H__
#define __PMT_H__

#include <sys/types.h>

#define PMT_LEN 12

typedef struct pmt_struct {
	u_char table_id				: 8;
  
#if BYTE_ORDER == BIG_ENDIAN
	u_char section_syntax_indicator		: 1;
	u_char dummy				: 1; // has to be 0
	u_char					: 2;
	u_char section_length_hi		: 4;
#else
	u_char section_length_hi		: 4;
	u_char					: 2;
	u_char dummy				: 1; // has to be 0
	u_char section_syntax_indicator		: 1;
#endif

	u_char section_length_lo		: 8;
  
	u_char program_number_hi		: 8;
  
	u_char program_number_lo		: 8;


#if BYTE_ORDER == BIG_ENDIAN
	u_char 					:2;
	u_char version_number			:5;
	u_char current_next_indicator		:1;
#else
	u_char current_next_indicator		:1;
	u_char version_number			:5;
	u_char 					:2;
#endif

	u_char section_number			: 8;
	
	u_char last_section_number		: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 3;
	u_char PCR_PID_hi			: 5;
#else
	u_char PCR_PID_hi			: 5;
	u_char					: 3;
#endif

	u_char PCR_PID_lo			: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 4;
	u_char program_info_length_hi		: 4;
#else
	u_char program_info_length_hi		: 4;
	u_char					: 4;
#endif

	u_char program_info_length_lo		: 8;
	//descriptors
} pmt_t;

#define PMT_info_LEN 5

typedef struct pmt_info_struct {
	u_char stream_type			: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 3;
	u_char elementary_PID_hi		: 5;
#else
	u_char elementary_PID_hi		: 5;
	u_char					: 3;
#endif  
	u_char elementary_PID_lo		: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 4;
	u_char ES_info_length_hi		: 4;
#else
	u_char ES_info_length_hi		: 4;
	u_char					: 4;
#endif

	u_char ES_info_length_lo		: 8;
	// descriptors
} pmt_info_t;

#endif
