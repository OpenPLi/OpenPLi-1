
/*
 * PROGRAM STREAM
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

#ifndef __PS_H__
#define __PS_H__

#include <sys/types.h>

#define PS_SYSTEM_HDR_LEN 12

typedef struct {
	u_char header_length_hi			: 8;
	u_char header_length_lo			: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char 					: 1;
	u_char rate_bound_1			: 7;
#else
	u_char rate_bound_1			: 7;
	u_char 					: 1;
#endif

	u_char rate_bound_2			: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char rate_bound_3			: 7;
	u_char 					: 1;

	u_char audio_bound			: 6;
	u_char fixed_flag			: 1;
	u_char CSPS_flag			: 1;

	u_char system_audio_lock_flag		: 1;
	u_char system_video_lock_flag		: 1;
	u_char					: 1;
	u_char video_bound			: 5;

	u_char packet_rate_restriction_flag	: 1;
	u_char reserved_byte			: 7;
#else
	u_char 					: 1;
	u_char rate_bound_3			: 7;

	u_char CSPS_flag			: 1;
	u_char fixed_flag			: 1;
	u_char audio_bound			: 6;

	u_char video_bound			: 5;
	u_char					: 1;
	u_char system_video_lock_flag		: 1;
	u_char system_audio_lock_flag		: 1;

	u_char reserved_byte			: 7;
	u_char packet_rate_restriction_flag	: 1;
#endif
} ps_system_hdr_t;

struct ps_system_hdr_2_struct {
	u_char stream_id			: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 2; // has to be 11
	u_char PSTD_buffer_bound_scale		: 1;
	u_char PSDT_buffer_size_bound_hi	: 5;
#else
	u_char PSDT_buffer_size_bound_hi	: 5;
	u_char PSTD_buffer_bound_scale		: 1;
	u_char					: 2; // has to be 11
#endif

	u_char PSDT_buffer_size_bound_lo		: 8;
} ps_system_hdr_2_t;

#define PS_STREAM_PACK_HDR	0xBA
#define PS_STREAM_SYSTEM_HDR	0xBB
#define PS_STREAM_VIDEO		0xE0

#if BYTE_ORDER == BIG_ENDIAN
#define PS_START_CODE_PREFIX_RAW	0x000010
#else
#define PS_START_CODE_PREFIX_RAW	0x010000
#endif

#define PS_START_CODE_PREFIX 0x000010

#define PS_HDR_LEN 4

typedef struct {
	u_int	start_code_prefix		: 24;
	u_char	stream_id			: 8;
} ps_hdr_t;

typedef struct {
#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 2;	// should be '01'
	u_char	system_clock_reference_1	: 3;
	u_char					: 1;
	u_char	system_clock_reference_2	: 2;
#else
	u_char	system_clock_reference_2	: 2;
	u_char					: 1;
	u_char	system_clock_reference_1	: 3;
	u_char					: 2;	// should be '01'
#endif

	u_char	system_clock_reference_3	: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char	system_clock_reference_4	: 5;
	u_char					: 1;
	u_char	system_clock_reference_5	: 2;
#else	
	u_char	system_clock_reference_5	: 2;
	u_char					: 1;
	u_char	system_clock_reference_4	: 5;
#endif

	u_char	system_clock_reference_6	: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char	system_clock_reference_7	: 5;
	u_char					: 1;
	u_char	system_clock_reference_ext_1	: 2;

	u_char	system_clock_reference_ext_2	: 7;
	u_char					: 1;
#else
	u_char	system_clock_reference_ext_1	: 2;
	u_char					: 1;
	u_char	system_clock_reference_7	: 5;

	u_char					: 1;
	u_char	system_clock_reference_ext_2	: 7;
#endif

	u_char	program_mux_rate_1		: 8;
	u_char	program_mux_rate_2		: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char	program_mux_rate_3		: 6;
	u_char					: 2;

	u_char					: 5;	//reserved
	u_char	pack_stuffing_length		: 3;
#else
	u_char					: 2;
	u_char	program_mux_rate_3		: 6;

	u_char	pack_stuffing_length		: 3;
	u_char					: 5;	//reserved
#endif
} ps_pack_t;

#define PS_PACK_LEN 10
#endif
