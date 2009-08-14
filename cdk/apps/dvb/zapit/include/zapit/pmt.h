/*
 * $Id: pmt.h,v 1.7 2002/10/12 20:19:44 obi Exp $
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

#ifndef __pmt_h__
#define __pmt_h__

#include "channel.h"
#include "ci.h"

int parse_pmt (int demux_fd, CZapitChannel * channel);
unsigned short parse_ES_info (unsigned char *buffer, CZapitChannel * channel, CCaPmt * caPmt);

#endif /* __pmt_h__ */
