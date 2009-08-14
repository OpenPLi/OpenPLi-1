/*
 * tuxbox_hardware_dreambox.c - TuxBox hardware info - dreambox
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
 * $Id: tuxbox_hardware_dreambox.c,v 1.1.2.2 2003/03/09 16:34:30 waldi Exp $
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <asm/io.h>

#include <tuxbox/tuxbox_hardware_dreambox.h>

int tuxbox_hardware_info_read (void)
{
	int ret;

	tuxbox_vendor = TUXBOX_VENDOR_DREAM_MM;
	tuxbox_model = TUXBOX_MODEL_DREAMBOX;

	switch(mfspr(PVR)) {
		case TUXBOX_HARDWARE_DREAMBOX_ID_DM5600:
			tuxbox_submodel = TUXBOX_SUBMODEL_DREAMBOX_DM5600;
			tuxbox_capabilities = TUXBOX_HARDWARE_DREAMBOX_DM5600_CAPABILITIES;
			break;

		case TUXBOX_HARDWARE_DREAMBOX_ID_DM7000:
			tuxbox_submodel = TUXBOX_SUBMODEL_DREAMBOX_DM7000;
			tuxbox_capabilities = TUXBOX_HARDWARE_DREAMBOX_DM7000_CAPABILITIES;
			break;

		default:
			return -EINVAL
	}

	return 0;
}

int tuxbox_hardware_read (void)
{
	return 0;
}

int tuxbox_hardware_proc_create (void)
{
	return 0;
}

