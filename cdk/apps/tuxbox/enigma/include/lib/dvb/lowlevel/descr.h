
/*
 * DESCRIPTORS
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


#ifndef __DESCR_H__
#define __DESCR_H__

#include <sys/types.h>

#define DESCR_GEN_LEN 2

struct descr_gen_struct {
	u_char	descriptor_tag		: 8;
	u_char	descriptor_length	: 8;
};
typedef struct descr_gen_struct descr_gen_t;

#define get_descr(x) (((descr_gen_t *) x)->descriptor_tag)
#define get_descr_len(x) (((descr_gen_t *) x)->descriptor_length)

#define DESCR_REGISTRATION 0x05
#define DESCR_ISO639_LANGUAGE	0x0A

#define DESCR_CAROUSEL_ID 0x13
#define DESCR_NW_NAME		0x40
#define DESCR_SERVICE_LIST	0x41
#define DESCR_STUFFING		0x42
#define DESCR_SAT_DEL_SYS	0x43
#define DESCR_CABLE_DEL_SYS	0x44
#define DESCR_BOUQUET_NAME	0x47
#define DESCR_SERVICE		0x48
#define DESCR_COUNTRY_AVAIL	0x49
#define DESCR_LINKAGE		0x4A
#define DESCR_NVOD_REF		0x4B
#define DESCR_TIME_SHIFTED_SERVICE	0x4C
#define DESCR_SHORT_EVENT	0x4D
#define DESCR_EXTENDED_EVENT	0x4E
#define DESCR_TIME_SHIFTED_EVENT	0x4F
#define DESCR_COMPONENT		0x50
#define DESCR_MOSAIC		0x51
#define DESCR_STREAM_ID		0x52
#define DESCR_CA_IDENT		0x53
#define DESCR_CONTENT		0x54
#define DESCR_PARENTAL_RATING	0x55
#define DESCR_TELETEXT		0x56
#define DESCR_TELEPHONE		0x57
#define DESCR_LOCAL_TIME_OFF	0x58
#define DESCR_SUBTITLING	0x59
#define DESCR_TERR_DEL_SYS	0x5A
#define DESCR_ML_NW_NAME	0x5B
#define DESCR_ML_BQ_NAME	0x5C
#define DESCR_ML_SERVICE_NAME	0x5D
#define DESCR_ML_COMPONENT	0x5E
#define DESCR_PRIV_DATA_SPEC	0x5F
#define DESCR_SERVICE_MOVE	0x60
#define DESCR_SHORT_SMOOTH_BUF	0x61
#define DESCR_FREQUENCY_LIST	0x62
#define DESCR_PARTIAL_TP_STREAM	0x63
#define DESCR_DATA_BROADCAST	0x64
#define DESCR_CA_SYSTEM		0x65
#define DESCR_DATA_BROADCAST_ID	0x66
#define DESCR_AC3	0x6A
#define DESCR_LESRADIOS  0xC5
#define DESCR_MHW_DATA	0xC2

#ifdef ENABLE_DISH_EPG
	#define DESCR_DISH_0x86              0x86 // tier info for this channel?
	#define DESCR_DISH_0x87              0x87 // eventid + ??
	#define DESCR_DISH_0x88              0x88
	#define DESCR_DISH_MOVIE_RATING      0x89
	#define DESCR_DISH_0x90              0x90
	#define DESCR_DISH_EVENT_NAME        0x91
	#define DESCR_DISH_DESCRIPTION       0x92
	#define DESCR_DISH_0x93              0x93 // more tier/subscription info?
	#define DESCR_DISH_DESCRIPTION_PARSE 0x94 // in parseable format? "{#|<text>} {#|<text>}..."
	#define DESCR_DISH_TV_RATING         0x95
	#define DESCR_DISH_0x96              0x96
#endif

/* 0x43 satellite delivery system descriptor */

struct descr_satellite_delivery_system_struct {
	u_char	descriptor_tag		: 8;
	u_char	descriptor_length	: 8;
	u_char	frequency1		: 8;
	u_char	frequency2		: 8;
	u_char	frequency3		: 8;
	u_char	frequency4		: 8;
	u_char	orbital_position1	: 8;
	u_char	orbital_position2	: 8;

			// fixed(?) by tmb
#if BYTE_ORDER == BIG_ENDIAN
	u_char	west_east_flag		: 1;
	u_char	polarization		: 2;
	u_char	modulation		: 5;
#else
	u_char	modulation		: 5;
	u_char	polarization		: 2;
	u_char	west_east_flag		: 1;
#endif

	u_char	symbol_rate1		: 8;
	u_char	symbol_rate2		: 8;
	u_char	symbol_rate3		: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char	symbol_rate4		: 4;
	u_char	fec_inner		: 4;
#else
	u_char	fec_inner		: 4;
	u_char	symbol_rate4		: 4;
#endif
};


/* 0x44 cable delivery system descriptor */

struct descr_cable_delivery_system_struct {
	u_char	descriptor_tag		: 8;
	u_char	descriptor_length	: 8;
	u_char	frequency1		: 8;
	u_char	frequency2		: 8;
	u_char	frequency3		: 8;
	u_char	frequency4		: 8;
        u_char	reserved1	        : 8;

#if BYTE_ORDER == BIG_ENDIAN
        u_char	reserved2	        : 4;
        u_char	fec_outer               : 4;
#else
        u_char	fec_outer               : 4;
        u_char	reserved2	        : 4;
#endif

	u_char	modulation		: 8;

	u_char	symbol_rate1		: 8;
	u_char	symbol_rate2		: 8;
	u_char	symbol_rate3		: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char	symbol_rate4		: 4;
	u_char	fec_inner		: 4;
#else
	u_char	fec_inner		: 4;
	u_char	symbol_rate4		: 4;
#endif
};


/* 0x5a terrestrial delivery system descriptor */

struct descr_terrestrial_delivery_system_struct {
	u_char	descriptor_tag		: 8;
	u_char	descriptor_length	: 8;
	u_char	centre_frequency1	: 8;
	u_char	centre_frequency2	: 8;
	u_char	centre_frequency3	: 8;
	u_char	centre_frequency4	: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char	bandwidth		: 3;
	u_char	reserved1		: 5;
	u_char	constellation		: 2;
	u_char	hierarchy_information	: 3;
	u_char	code_rate_hp_stream	: 3;
	u_char	code_rate_lp_stream	: 3;
	u_char	guard_interval		: 2;
	u_char	transmission_mode	: 2;
	u_char	other_frequency_flag	: 1;
#else
	u_char	reserved1		: 5;
	u_char	bandwidth		: 3;
	u_char	code_rate_hp_stream	: 3;
	u_char	hierarchy_information	: 3;
	u_char	constellation		: 2;
	u_char	other_frequency_flag	: 1;
	u_char	transmission_mode	: 2;
	u_char	guard_interval		: 2;
	u_char	code_rate_lp_stream	: 3;
#endif

	u_char	reserved2		: 8;
	u_char	reserved3		: 8;
	u_char	reserved4		: 8;
	u_char	reserved5		: 8;
};


/* 0x50 component descriptor */

struct descr_component_struct {
	u_char	descriptor_tag		: 8;
	u_char	descriptor_length	: 8;
#if BYTE_ORDER == BIG_ENDIAN
        u_char	reserved	        : 4;
        u_char	stream_content          : 4;
#else
        u_char	stream_content          : 4;
        u_char	reserved	        : 4;
#endif
	u_char	component_type		: 8;
	u_char	component_tag		: 8;
	u_char	lang_code1		: 8;
	u_char	lang_code2		: 8;
	u_char	lang_code3		: 8;
};

struct descr_content_entry_struct
{
#if BYTE_ORDER == BIG_ENDIAN
	u_char	content_nibble_level_1	: 4;
	u_char	content_nibble_level_2	: 4;
	u_char	user_nibble_1		: 4;
	u_char	user_nibble_2		: 4;
#else
	u_char	content_nibble_level_2	: 4;
	u_char	content_nibble_level_1	: 4;
	u_char	user_nibble_2		: 4;
	u_char	user_nibble_1		: 4;
#endif
};

struct descr_lesradios_struct
{
	u_char descriptor_tag : 8;
	u_char descriptor_length : 8;
	u_char id;
		// name
};

struct descr_linkage_struct
{
	u_char	descriptor_tag		: 8;
	u_char	descriptor_length	: 8;

        u_char  transport_stream_id_hi          : 8;
        u_char  transport_stream_id_lo          : 8;
        u_char  original_network_id_hi          : 8;
        u_char  original_network_id_lo          : 8;
        u_char  service_id_hi                   : 8;
        u_char  service_id_lo                   : 8;
        u_char  linkage_type                    : 8;

#if BYTE_ORDER == BIG_ENDIAN
        u_char	handover_type	        : 4;
        u_char	reserved                : 3;
        u_char	origin_type             : 1;
#else
        u_char	origin_type             : 1;
        u_char	reserved                : 3;
        u_char	handover_type	        : 4;
#endif
};

#define  LINKAGE_LEN    9

struct descr_time_shifted_service_struct
{
	u_char	descriptor_tag		: 8;
	u_char	descriptor_length	: 8;

        u_char  reference_service_id_hi : 8;
        u_char  reference_service_id_lo : 8;
};

struct descr_time_shifted_event_struct
{
	u_char	descriptor_tag		: 8;
	u_char	descriptor_length	: 8;

        u_char  reference_service_id_hi : 8;
        u_char  reference_service_id_lo : 8;

        u_char  reference_event_id_hi : 8;
        u_char  reference_event_id_lo : 8;  
};

struct descr_stream_identifier_struct
{
	u_char  descriptor_tag          : 8;
	u_char  descriptor_length       : 8;
	u_char  component_tag           : 8;
};

struct descr_mhw_data_struct
{
	u_char	descriptor_tag		: 8;
	u_char	descriptor_length	: 8;
  u_char  type[8];
};

struct descr_carousel_identifier_struct
{
	u_char	descriptor_tag		: 8;
	u_char	descriptor_length	: 8;
	u_char carousel_id[4];
};

struct descr_data_broadcast_id_struct
{
	u_char descriptor_tag		: 8;
	u_char descriptor_length	: 8;
	u_char data_broadcast_id_hi : 8;
	u_char data_broadcast_id_lo : 8;
	u_char application_type_hi : 8;
	u_char application_type_lo : 8;
	u_char boot_priority_hint : 8;
};

#endif
