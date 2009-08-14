
/*
 * MULTI PROTOCOL ENCAPSULATION
 *
 * Copyright (C) 1999 Thomas Mirlacher
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

#ifndef __MPE_H__
#define __MPE_H__

#include <sys/types.h>

#define MPE_TABLE_ID	0x3E
#define MPE_HEADER_SIZE 12

typedef struct mpe_header_struct {
	u_char table_id			: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char section_syntax_indicator	: 1;
	u_char private_indicator	: 1;
	u_char				: 2;
	u_char section_length_hi	: 4;
#else
	u_char section_length_hi	: 4;
	u_char				: 2;
	u_char private_indicator	: 1;
	u_char section_syntax_indicator	: 1;
#endif

	u_char section_length_lo	: 8;

	u_char MAC_address_6		: 8;

	u_char MAC_address_5		: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char				: 2;
	u_char payload_scrambling_control: 2;
	u_char address_scrambling_control: 2;
	u_char LLC_SNAP_flag		: 1;
	u_char current_next_indicator	: 1;
#else
	u_char current_next_indicator	: 1;
	u_char LLC_SNAP_flag		: 1;
	u_char address_scrambling_control: 2;
	u_char payload_scrambling_control: 2;
	u_char				: 2;
#endif

	u_char section_number		: 8;

	u_char last_section_number	: 8;

	u_char MAC_address_4		: 8;

	u_char MAC_address_3		: 8;

	u_char MAC_address_2		: 8;

	u_char MAC_address_1		: 8;
} mpe_hdr_t;

#endif
