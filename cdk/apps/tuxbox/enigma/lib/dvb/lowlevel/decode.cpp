
/*
 *
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
 * $log$
 */


#include <sys/types.h>

static char fec_tbl[][6] = {
	"ndef",
	"1/2",
	"2/3",
	"3/4",
	"5/6",
	"7/8",
	"nconv",
	"reser"
};

static char service_type_tbl[][50] = {
	"Reserved",
	"digital television service",
	"digital radion sound service",
	"television service",
	"NVOD reference service",
	"NVOD time-shifted service",
	"mosaic service",
	"PAL coded signal",
	"D/D2-MAC",
	"FM Radio",
	"NTSC coded signal",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

static char stream_type_tbl[][70] = {
	"ITU-T|ISO/IEC Reserved",
	"ISO/IEC Video",
	"13818-2 Video or 11172-2 constrained parameter video stream",
	"ISO/IEC 11172 Audio",
	"ISO/IEC 13818-3 Audio",
	"private_sections",
	"packets containing private data",
	"ISO/IEC 13522 MPEG",
	"ITU-T Rec. H.222.1",
	"ISO/IEC 13818-6 type A",
	"ISO/IEC 13818-6 type B",
	"ISO/IEC 13818-6 type C",
	"ISO/IEC 13818-6 type D",
	"ISO/IEC 13818-1 auxiliary",
	"ITU-T Rec. H.222.0 | ISO 13818-1 Reserved",
	"User private"
};

static char descr_tbl[][50] = {
// defined by ISO/IEC 13818-1 P64
	"Reserved", 
	"Reserved", 
	"Video Stream", 
	"Audio Stream", 
	"Hierarchy", 
	"Registration", 
	"Data Stream Alignment", 
	"Target Background Grid", 
	"Video Window", 
	"CA", 
	"ISO 639 Language", 
	"System Clock", 
	"Multiplex Buffer Utilization", 
	"Copyright", 
	"Maximum Bitrate", 
	"Private Data Indicator", 
	"Smoothing Buffer", 
	"STD", 
	"IBP", 
	"ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved", 
// defined by ETSI
	"Network Name", 
	"Service List", 
	"Stuffing", 
	"Satellite Delivery System", 
	"Cable Delivery System", 
	"Reserved for future use", 
	"Reserved for future use", 
	"Bouquet Name", 
	"Service", 
	"Country Availability", 
	"Linkage", 
	"NVOD Reference", 
	"Time Shifted Service", 
	"Short Event", 
	"Extended Event", 
	"Time Shifted Event", 
	"Component", 
	"Mosaic", 
	"Stream Identifier", 
	"CA Identifier", 
	"Content", 
	"Parental Rating", 
	"Teletext",
	"Telephone",
	"Local Time Offset",
	"Subtitling",
	"Terrestrial Delivery System",
	"Multilingual Network Name",
	"Multilingual Bouquet Name",
	"Multilingual Service Name",
	"Multilingual Component",
	"Private Data Specifier",
	"Service Move",
	"Short Smoothing Buffer",
	"Reserved for future use",
	"User defined",
	"FORBIDDEN",
};

char *decode_fec (u_char fec)
{
	if (fec <= 0x05)
		return fec_tbl[fec];
	else if (fec == 0x0F)
		return fec_tbl[0x06];
	else
		return fec_tbl[0x07];
}

char *decode_service_type (u_char type)
{
        if (type>=0x10)
                return "illegal";
        return service_type_tbl[type];
}


char *decode_stream_type (u_char _index)
{
	int index  = _index;

	if (_index > 0x0F && _index <= 0x7F)
		index = 0x0E;
	if (_index >= 0x80)
		index = 0x0F;

	return stream_type_tbl[index];
}


char *decode_descr (u_char _index) {
	int index = _index;

	if (_index>=0x13 && _index<=0x3F) 
		index = 0x13;

	if (_index>=0x40)
		index -= (0x3F - 0x13);

	if (_index>=0x62 && _index<=0x7F)
		index = 0x62 - (_index - index);

	if (_index>=0x80)
		index -= (0x7F - 0x62);

	if (_index>=0x80 && _index<=0xFE)
		index = 0x80 - (_index - index);

	if (_index == 0xFF)
		index = 0xFF - (_index - index) - (0xFE - 0x80);

	return descr_tbl[index];
}

