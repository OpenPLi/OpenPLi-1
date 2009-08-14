
/*
 * TRANSPORT STREAM
 *
 * Copyright (C) 1998, 1999  Thomas Mirlacher
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

#ifndef __TS_H__
#define __TS_H__

#include <sys/types.h>

#define TS_SYNC_BYTE	0x47

#define TS_LEN	188
#define TS_HDR_LEN 4
#define TS_DATA_LEN (TS_LEN - TS_HDR_LEN)

typedef struct {
	u_char sync_byte			: 8;	/* should be 0x47 */

#if BYTE_ORDER == BIG_ENDIAN
	u_char transport_error_indicator	: 1;
	u_char payload_unit_start_indicator	: 1;
	u_char transport_priority		: 1;
	u_char PID_hi				: 5;
#else
	u_char PID_hi				: 5;
	u_char transport_priority		: 1;
	u_char payload_unit_start_indicator	: 1;
	u_char transport_error_indicator	: 1;
#endif
	u_char PID_lo				: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char transport_scrambling_control	: 2;
	u_char adaptation_field_control		: 2;
	u_char continuity_counter		: 4;
#else
	u_char continuity_counter		: 4;
	u_char adaptation_field_control		: 2;
	u_char transport_scrambling_control	: 2;
#endif
} ts_hdr_t;

#define TS_ADAPTATION_FIELD_LEN 1

typedef struct {
#if BYTE_ORDER == BIG_ENDIAN
	u_char discontinuity_indicator	       		: 1;
	u_char random_access_indicator	       		: 1;
	u_char elementary_stream_priority_indicator	: 1;
	u_char pcr_flag			       		: 1;
	u_char opcr_flag				: 1;
	u_char splicing_point_flag	       		: 1;
	u_char transport_private_data_flag     		: 1;
	u_char adaptation_field_extension_flag 		: 1;
#else
	u_char adaptation_field_extension_flag 		: 1;
	u_char transport_private_data_flag     		: 1;
	u_char splicing_point_flag	       		: 1;
	u_char opcr_flag				: 1;
	u_char pcr_flag			       		: 1;
	u_char elementary_stream_priority_indicator	: 1;
	u_char random_access_indicator		       	: 1;
	u_char discontinuity_indicator		       	: 1;
#endif
} ts_adapt_fld_t;

#define TS_PCR_LEN 6

typedef struct {
	u_char program_clock_reference_base_1		: 8;

	u_char program_clock_reference_base_2		: 8;

	u_char program_clock_reference_base_3		: 8;

	u_char program_clock_reference_base_4		: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char program_clock_reference_base_5		: 1;
	u_char 						: 6;
	u_char program_clock_reference_extension_hi	: 1;
#else
	u_char program_clock_reference_extension_hi	: 1;
	u_char 						: 6;
	u_char program_clock_reference_base_5		: 1;
#endif

	u_char program_clock_reference_extension_lo	: 8;
} ts_pcr_t;

struct ts_opcr_struct {
	unsigned long long original_program_clock_reference_base: 33;
	u_char reserved				: 6;
	u_short original_program_clock_reference_extension: 9;
};

struct ts_splicing_point_struct {
	u_char splice_countdown				: 8;
};

struct ts_transport_private_data_struct {
	u_char transport_private_data_length		: 8;
};

struct ts_adaptation_field_extension_struct {
#if BYTE_ORDER == BIG_ENDIAN
	u_char reserved					: 5;
	u_char seamless_splice_flag			: 1;
	u_char piecewise_rate_flag			: 1;
	u_char ltw_flag					: 1;
	u_char adaptation_field_extension_length	: 8;
#else
	u_char adaptation_field_extension_length	: 8;
	u_char ltw_flag					: 1;
	u_char piecewise_rate_flag			: 1;
	u_char seamless_splice_flag			: 1;
	u_char reserved					: 5;
#endif
};

// TODO: CHECK DOWN HERE

struct ts_ltw_struct {
	u_char ltw_calid_flag				: 1;
	u_short ltw_offset				: 15;
};

struct ts_piecewise_rate_struct {
	u_char reserved					: 2;
	u_int piecewise_rate				: 22;
};

struct ts_seamless_splice_struct {
	u_char splice_type				: 4;
	u_char DTS_next_AU_32_30			: 3;
	u_char marker_bit0				: 1;
	u_short DTS_next_AU_29_15			: 15;
	u_char marker_bit1				: 1;
	u_short DTS_next_AU_14_0			: 15;
	u_char marker_bit2				: 1;
};

#define TS_PAYLEN	184
#define TS_NOSYNC	-1	/* no valid sync byte */
#define TS_NODATA	-2	/* no payload */
#define TS_SCRAMBLED	-3	/* scrambled - and we can't descramble it */
#define TS_INPKT	-4	/* doing our reassembling task */
#define TS_UNKNOWN	-5	/* an unknown error appeared */

#endif
