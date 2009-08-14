/*
 * $Id: pat.h,v 1.15 2002/10/12 20:19:44 obi Exp $
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

#ifndef __pat_h__
#define __pat_h__

#include "channel.h"
#include "frontend.h"

int parse_pat (const int demux_fd, CZapitChannel * channel, const t_original_network_id original_network_id = 0x0000, const uint8_t DiSEqC = 0);
int fake_pat (uint32_t TsidOnid, FrontendParameters feparams, uint8_t polarity, uint8_t DiSEqC);

#endif /* __pat_h__ */
