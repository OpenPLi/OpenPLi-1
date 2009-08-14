/*
 * $Id: sdt.h,v 1.11 2002/10/12 20:19:44 obi Exp $
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
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

#ifndef __sdt_h__
#define __sdt_h__

#include "types.h"

enum service_type_e
{
	RESERVED,
	DIGITAL_TELEVISION_SERVICE,
	DIGITAL_RADIO_SOUND_SERVICE,
	TELETEXT_SERVICE,
	NVOD_REFERENCE_SERVICE,
	NVOD_TIME_SHIFTED_SERVICE,
	MOSAIC_SERVICE,
	PAL_CODED_SIGNAL,
	SECAM_CODED_SIGNAL,
	D_D2_MAC,
	FM_RADIO,
	NTSC_CODED_SIGNAL,
	DATA_BROADCAST_SERVICE,
	COMMON_INTERFACE_RESERVED,
	RCS_MAP,
	RCS_FLS,
	DVB_MHP_SERVICE
};

struct sdt_generic_descriptor {
        u_char  descriptor_tag                  : 8;
        u_char  descriptor_length               : 8;
};

int parse_sdt(const uint8_t DiSEqC);
unsigned int get_sdt_TsidOnid ();

#endif /* __sdt_h__ */
