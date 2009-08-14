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

#ifndef __TDT_H__
#define __TDT_H__

#include <sys/types.h>

#define TDT_LEN 8

typedef struct {
	u_char	table_id			: 8; 

#if BYTE_ORDER == BIG_ENDIAN
	u_char	section_syntax_indicator	: 1;
	u_char					: 3;
	u_char	section_length_hi		: 4;
#else
	u_char	section_length_hi		: 4;
	u_char					: 3;
	u_char	section_syntax_indicator	: 1;
#endif

	u_char	section_length_lo		: 8;

        
	u_char	utc_time1		: 8;
	u_char	utc_time2		: 8;
	u_char	utc_time3		: 8;
	u_char	utc_time4		: 8;
	u_char	utc_time5		: 8;
} tdt_t;

#endif	
