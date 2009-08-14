/*
 * tuxbox.c - TuxBox hardware info
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
 * $Id: tuxbox.c,v 1.7.2.2 2003/03/09 16:26:41 waldi Exp $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define need_TUXBOX_GET
#include "tuxbox.h"

int tuxbox_read_proc (char *type)
{
	FILE *file;
	char filename[64];
	char *line = NULL;
	size_t len = sizeof (line);
	int ret = 0;

	if (!type)
		return ret;

	snprintf (filename, sizeof (filename), "/proc/bus/tuxbox/%s", type);

	file = fopen(filename, "r");

	if (!file) {
		perror("open");
		return ret;
	}

	if (getline(&line, &len, file) != -1)
		ret = strtol (line, NULL, 0);

	fclose(file);

	if (line)
		free (line);

	return ret;
}

TUXBOX_GET(capabilities);
TUXBOX_GET(model);
TUXBOX_GET(submodel);
TUXBOX_GET(vendor);

const char *tuxbox_get_model_str (void)
{
	switch(tuxbox_get_model ()) {
		case TUXBOX_MODEL_DBOX2:
			return "D-BOX2";
		case TUXBOX_MODEL_DREAMBOX:
			return "Dreambox";
		default:
			return "Unknown";
	}
}

const char *tuxbox_get_submodel_str (void)
{
	switch(tuxbox_get_submodel ()) {
		case TUXBOX_SUBMODEL_DBOX2:
			return "";
		case TUXBOX_SUBMODEL_DREAMBOX_DM5600:
			return "5600";
		case TUXBOX_SUBMODEL_DREAMBOX_DM7000:
			return "7000";
		default:
			return "Unknown";
	}
}

const char *tuxbox_get_vendor_str (void)
{
	switch(tuxbox_get_vendor ()) {
		case TUXBOX_VENDOR_NOKIA:
			return "Nokia";
		case TUXBOX_VENDOR_SAGEM:
			return "Sagem";
		case TUXBOX_VENDOR_PHILIPS:
			return "Philips";
		case TUXBOX_VENDOR_DREAM_MM:
			return "Dream Multimedia TV";
		default:
			return "Unknown";
	}
}

