
/*
 * Audio Video PES
 *
 * Copyright (C) 1999  Thomas Mirlacher
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


#ifndef __AVP_H__
#define __AVP_H__

#include <sys/types.h>

typedef struct {
	u_char prefix1		:8;	// 'A'
	u_char prefix2		:8;	// 'V'
	u_char stream_id	:8;	// 0x01 ... Video, 0x02 ... Audio 
	u_char cont_count	:8;	// continuity counter
	u_char reserved		:8;	// 0x55

#if BYTE_ORDER == BIG_ENDIAN
	u_char 			:3;
	u_char pts_present	:1;	// presentation time stamp		
	u_char pad_length	:2;	// bytes left for 4 byte pad
	u_char pad_length	:2;	// bytes left for 4 byte pad (last pkt)
#else
	u_char 			:3;
	u_char pts_present	:1;	// presentation time stamp		
	u_char pad_length	:2;	// bytes left for 4 byte pad
	u_char pad_length	:2;	// bytes left for 4 byte pad (last pkt)
#endif

	u_char length_hi	:8;	// payload length (high byte)
	u_char length_lo	:8;	// payload length (low byte)
} avp_hdr_t;

#define AVP_HDR_LEN 8

#endif
