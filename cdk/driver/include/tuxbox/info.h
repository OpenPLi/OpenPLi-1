/*
 * tuxbox_info.h - TuxBox hardware info
 *
 * Copyright (C) 2003 Florian Schirmer <jolt@tuxbox.org>
 *                    Bastian Blank <waldi@tuxbox.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: info.h,v 1.2.2.1 2003/03/09 16:35:45 waldi Exp $
 */

#ifndef TUXBOX_INFO_H
#define TUXBOX_INFO_H

#include <tuxbox/kernel.h>

typedef enum tuxbox_capabilities
{
	TUXBOX_CAPABILITIES_IR_RC		= (1<<0),
	TUXBOX_CAPABILITIES_IR_KEYBOARD		= (1<<1),
	TUXBOX_CAPABILITIES_LCD			= (1<<2),
	TUXBOX_CAPABILITIES_NETWORK		= (1<<3),
	TUXBOX_CAPABILITIES_HDD			= (1<<4),
	TUXBOX_CAPABILITIES_CAM_CI		= (1<<5),
	TUXBOX_CAPABILITIES_CAM_EMBEDDED	= (1<<6),
}
tuxbox_capabilities_t;

typedef enum tuxbox_model
{
	TUXBOX_MODEL_DBOX2			= 1,
	TUXBOX_MODEL_DREAMBOX			= 2,
	TUXBOX_MODEL_PCI			= 3,
}
tuxbox_model_t;

typedef enum tuxbox_submodel
{
	TUXBOX_SUBMODEL_DBOX2			= 1,
	TUXBOX_SUBMODEL_DREAMBOX_DM7000		= 2,
	TUXBOX_SUBMODEL_DREAMBOX_DM5600		= 3,
	TUXBOX_SUBMODEL_TTPCI			= 4,
}
tuxbox_submodel_t;

typedef enum tuxbox_vendor
{
	TUXBOX_VENDOR_NOKIA			= 1,
	TUXBOX_VENDOR_PHILIPS			= 2,
	TUXBOX_VENDOR_SAGEM			= 3,
	TUXBOX_VENDOR_DREAM_MM			= 4,
	TUXBOX_VENDOR_TECHNOTREND		= 5,
}
tuxbox_vendor_t;

#endif /* TUXBOX_INFO_H */
