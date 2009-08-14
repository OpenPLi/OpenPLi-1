
/*
 * PACKETIZED ELEMENTARY STREAM
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

#ifndef __PES_H__
#define __PES_H__

#include <sys/types.h>

#if BYTE_ORDER == BIG_ENDIAN
#define PES_START_CODE_PREFIX_RAW	0x000010
#else
#define PES_START_CODE_PREFIX_RAW	0x010000
#endif

#define PES_START_CODE_PREFIX	0x010000

typedef struct {
	u_int start_code_prefix			: 24;	// 000001
	u_char stream_id			: 8;
	u_char pes_packet_length_hi		: 8;
	u_char pes_packet_length_lo		: 8;
} pes_hdr_t;    

#define PES_HDR_LEN 6

typedef struct {
#if BYTE_ORDER == BIG_ENDIAN
	u_char start_code_prefix		: 2;	/* 0x02 */
	u_char pes_scrambling_control  		: 2;
	u_char pes_priority	       		: 1;
	u_char data_alignment_indicator		: 1;
	u_char copyright			: 1;
	u_char original_or_copy			: 1;

	u_char pts_dts_flags	       		: 2;
	u_char escr_flag			: 1;
	u_char es_rate_flag	       		: 1;
	u_char dsm_trick_mode_flag		: 1;
	u_char additional_copy_info_flag	: 1;
	u_char pes_crc_flag	       		: 1;
	u_char pes_extension_flag		: 1;
#else
	u_char original_or_copy			: 1;
	u_char copyright			: 1;
	u_char data_alignment_indicator		: 1;
	u_char pes_priority	       		: 1;
	u_char pes_scrambling_control  		: 2;
	u_char start_code_prefix		: 2;	/* 0x02 */

	u_char pes_extension_flag		: 1;
	u_char pes_crc_flag	       		: 1;
	u_char additional_copy_info_flag	: 1;
	u_char dsm_trick_mode_flag		: 1;
	u_char es_rate_flag	       		: 1;
	u_char escr_flag			: 1;
	u_char pts_dts_flags	       		: 2;
#endif
		
	u_char pes_header_data_length		: 8;
} pes_flag_t;

#define PES_FLAG_LEN 3

//TODO: check ENIANs and comvert pts stuff into bytes

struct pts_dts_flags_2_struct {
#if BYTE_ORDER == BIG_ENDIAN
	u_char marker_bit0			: 1;  
	u_char pts_32_30			: 3;
	u_char					: 4;		/* has to be 0010b */
#else
	u_char					: 4;		/* has to be 0010b */
	u_char pts_32_30			: 3;
	u_char marker_bit0			: 1;  
#endif

	u_short pts_29_15			: 15;
	u_char marker_bit1			: 1;  

	u_short pts_14_0			: 15;
	u_char marker_bit2			: 1;  
};

struct PTS_DTS_flags_3__struct {
	u_char					: 4;		/* has to be 0011b */
	u_char pts_32_30			: 3;
	u_char marker_bit0			: 1;

	u_short pts_29_15			: 15;
	u_char marker_bit1			: 1;

	u_short pts_14_0			: 15;
	u_char marker_bit2			: 1;

	u_char					: 2;		/* has to be 0001b */
	u_char dtp_32_30			: 3;
	u_char marker_bit3			: 1;
	u_short dtp_29_15			: 15;
	u_char marker_bit4			: 1;
	u_short dtp_14_0			: 15;
	u_char marker_bit5			: 1;
};

struct escr_flag_struct {
	u_char 					: 2;
	u_char escr_base_32_30			: 3;
	u_char marker_bit0			: 1;
	u_short escr_base_29_15			: 15;
	u_char marker_bit1			: 1;
	u_short escr_base_14_0			: 15;
	u_char marker_bit2			: 1;
	u_short escr_extension			: 9;
	u_char marker_bit3			: 1;
};

struct es_rate_flag_struct {
	u_char marker_bit0			: 1;
	u_int es_rate				: 22;
	u_char marker_bit1			: 1;
};

struct dsm_trick_mode_flag_struct {
	u_char trick_mode_control		: 3;
	union {
		struct {
			u_char field_id             : 2;
			u_char intra_slice_refresh  : 1;
			u_char frequency_truncation : 2;
		} fast_forward;
		
		struct {
			u_char rep_cntrl	: 5;
		} slow_motion;
		
		struct {
			u_char field_id		: 2;
			u_char 			: 3;
		} freeze_frame;
		
		struct {
			u_char field_id            	: 2;
			u_char intra_slice_refresh 	: 1;
			u_char frequency_truncation	: 2;
		} fast_reverse;
		
		struct {
			u_char rep_cntrli		: 5;
		} slow_reverse;
	} mode;
};

struct additional_copy_info_flag_struct {
	u_char marker_bit				: 1;
	u_char additional_copy_info			: 7;
};

struct pes_crc_flag_struct {
	u_short previous_pes_packet_crc			: 16;
};

struct pes_extension_flag_struct {
	u_char pes_private_data_flag			: 1;
	u_char pack_header_field_flag			: 1;
	u_char program_packet_sequence_counter_flag	: 1;
	u_char psdt_buffer_flag				: 1;
	u_char						: 3;
	u_char pes_extension_flag_2			: 1;
/* DENT: tjo ... just continue here to fill structs in ... */
}; 

#define STREAM_PROGRAM_MAP	0xBC
#define STREAM_PRIVATE_1	0xBD
#define STREAM_PRIVATE_2	0xBF
#define STREAM_PADDING		0xBE
#define STREAM_AUDIO		0xBC	// 110X XXXX
#define STREAM_VIDEO		0xE0	// 1110 XXXX
#define STREAM_ECM		0xF0
#define STREAM_EMM		0xF1
#define STREAM_DSMCC		0xF2
#define STREAM_ITU_A		0xF4	
#define STREAM_ITU_B		0xF5	
#define STREAM_ITU_C		0xF6	
#define STREAM_ITU_D		0xF7	
#define STREAM_ITU_E		0xF8	
#define STREAM_MASK_AUDIO	0xE0
#define STREAM_REST_AUDIO	0xC0
#define STREAM_MASK_VIDEO	0xF0
#define STREAM_REST_VIDEO	0xE0

#endif
