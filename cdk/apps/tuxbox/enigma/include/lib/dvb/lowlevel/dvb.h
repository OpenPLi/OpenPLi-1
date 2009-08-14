
/*
 * PID AND TABLE IDENTIFIERS
 *
 * Copyright (C) 1998,1999  Thomas Mirlacher
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

#ifndef __DVB_H__
#define __DVB_H__

/* Program Identtifier */

#define PID_PAT 0x00	/* Program Association Table */
#define PID_CAT 0x01	/* Conditional Access Table */
#define PID_NIT 0x10	/* Network Information Table */
#define PID_BAT 0x11	/* Bouquet Association Table */
#define PID_SDT 0x11	/* Service Description Table */
#define PID_EIT 0x12	/* Event Information Table */
#define PID_RST 0x13	/* Running Status Table */
#define PID_TDT 0x14	/* Time Date Table */
#define PID_TOT 0x14	/* Time Offset Table */
#define PID_ST	0x14	/* Stuffing Table */
/* 0x15 - 0x1F */	/* Reserved for future use */

/* Table Identifier */

#define TID_PAT 0x00	/* Program Association Section */
#define TID_CAT 0x01	/* Conditional Access Section */
#define TID_PMT 0x02	/* Conditional Access Section */
/* 0x03 - 0x3F */	/* Reserved for future use */
#define TID_NIT_ACT 0x40	/* Network Information Section - actual */
#define TID_NIT_OTH 0x41	/* Network Information Section - other */
#define TID_SDT_ACT 0x42	/* Service Description Section - actual */
#define TID_SDT_OTH 0x46	/* Service Description Section - other */
#define TID_BAT			0x4A
#define TID_EIT_ACT 0x4E	/* Event Information Section - actual */
#define TID_EIT_OTH 0x4F	/* Event Information Section - other */
#define TID_EIT_ACT_SCH 0x50	/* Event Information Section - actual, schedule  */
#define TID_EIT_OTH_SCH 0x60	/* Event Information Section - other, schedule */
#define TID_TDT 0x70	/* Time Date Section */
#define TID_TOT 0x73	/* Time Offset Section */
#define TID_CA_ECM_0 0x80
#define TID_CA_ECM_1 0x81

#if 0
#define TID_BAT 0x01	/* Bouquet Association Section */
#define TID_BAT 0x01	/* Bouquet Association Section */

#define TID_EIT 0x12	/* Event Information Section */
#define TID_RST 0x13	/* Running Status Section */
#define TID_ST	0x14	/* Stuffung Section */
/* 0xFF */		/* Reserved for future use */
#endif

#endif
