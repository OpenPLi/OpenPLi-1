
/*
 * PROGRAM ASSOCIATION TABLE
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

#ifndef __PAT_H__
#define __PAT_H__

#include <sys/types.h>

#define PAT_LEN 8

typedef struct {
	u_char table_id				:8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char section_syntax_indicator		:1;
	u_char dummy				:1;	// has to be 0
	u_char 					:2;
	u_char section_length_hi		:4;
#else
	u_char section_length_hi		:4;
	u_char 					:2;
	u_char dummy				:1;	// has to be 0
	u_char section_syntax_indicator		:1;
#endif

	u_char section_length_lo		:8;

	u_char transport_stream_id_hi		:8;
	u_char transport_stream_id_lo		:8;

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
} pat_t;

#define PAT_PROG_LEN 4

typedef struct {
	u_char program_number_hi		:8;
	u_char program_number_lo		:8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char 					:3;
	u_char network_pid_hi			:5;
#else
	u_char network_pid_hi			:5;
	u_char 					:3;
#endif

	u_char network_pid_lo			:8; 
	/* or program_map_pid (if prog_num=0)*/
} pat_prog_t;

#endif
