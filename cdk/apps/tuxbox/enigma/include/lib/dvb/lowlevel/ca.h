/*
 * TIME DESCRIPTION TABLE
 *
 * Copyright (C) 2000 Ralph Metzler
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
 */

#ifndef __CA_H__
#define __CA_H__

#include <sys/types.h>

#define CA_LEN 8

typedef struct  {
	u_char	table_id			: 8; 

#if BYTE_ORDER == BIG_ENDIAN
	u_char	section_syntax_indicator	: 1;
	u_char	zero    			: 1;
	u_char	        			: 2;
	u_char	section_length_hi		: 4;
#else
	u_char	section_length_hi		: 4;
	u_char	        			: 2;
	u_char	zero    			: 1;
	u_char	section_syntax_indicator	: 1;
#endif

	u_char	section_length_lo		: 8;
  
        u_char  reserved_1      		: 8;
        u_char  reserved_2      		: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char	        			: 2;
	u_char version_number 			: 5;
	u_char current_next_indicator		: 1;
#else
	u_char current_next_indicator		: 1;
	u_char version_number 			: 5;
	u_char	        			: 2;
#endif

	u_char section_number			: 8;

	u_char last_section_number		: 8;

} ca_t;



#define CA_MESSAGE_LEN 3

typedef struct  {
	u_char	table_id			: 8; 

#if BYTE_ORDER == BIG_ENDIAN
	u_char	section_syntax_indicator	: 1;
	u_char	DVB_reserved			: 1;
	u_char	ISO_reserved			: 2;
	u_char	section_length_hi		: 4;
#else
	u_char	section_length_hi		: 4;
	u_char	ISO_reserved			: 2;
	u_char	DVB_reserved			: 1;
	u_char	section_syntax_indicator	: 1;
#endif

	u_char	section_length_lo		: 8;

        // CA_data_bytes
} ca_message_t;


#define CA_DESCR_LEN 6

typedef struct  {
	u_char	descriptor_tag			: 8; 
	u_char	descriptor_length		: 8; 

	u_char	CA_system_ID_hi            	: 8;
	u_char	CA_system_ID_lo            	: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char	                             	: 3;
	u_char	CA_PID_hi                   	: 5;
#else
	u_char	CA_PID_hi                   	: 5;
	u_char	                             	: 3;
#endif

	u_char	CA_PID_lo                   	: 8;

} ca_descr_t;

#endif
