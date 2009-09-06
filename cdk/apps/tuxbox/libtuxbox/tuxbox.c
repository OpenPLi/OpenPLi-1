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
 * $Id: tuxbox.c,v 1.10 2009/02/11 21:29:13 rhabarber1848 Exp $
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
		perror(filename);
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
		case TUXBOX_MODEL_PCI:
			return "PCI";
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
		case TUXBOX_SUBMODEL_TTPCI:
			return "Premium";
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
		case TUXBOX_VENDOR_TECHNOTREND:
			return "Technotrend";
		default:
			return "Unknown";
	}
}

# include <fcntl.h>
# include <sys/ioctl.h>
# include <linux/dvb/frontend.h>

const char *tuxbox_get_tuner (void)
{
	int fd;

	struct dvb_frontend_info info;

	if ((fd = open("/dev/dvb/adapter0/frontend0", O_RDONLY)) == -1)
	{
		return "unknown";
	}

	if (ioctl(fd, FE_GET_INFO, &info) == -1)
	{
		return "unknown";
	}

	switch (info.type)
	{
		case FE_QPSK:
			return "SAT";
			break;
		case FE_QAM:
			return "CABLE";
			break;
		default:
		case FE_OFDM:
			return "TERRESTRIAL";
			break;
	}
 }

const char *tuxbox_get_chipinfo (void)
{
	/* the 1xI boxen have mtd0: 00020000 00020000 "BR bootloader"
	   the 2xI boxen have mtd0: 00020000 00004000 "BR bootloader"
	                                  (n-6)--^     ^--(n) */

	char line[55];
	FILE* fp;
	
	fp = fopen("/proc/mtd", "r");
	if (fp)
	{
	  while (fgets(line, 55, fp) != NULL)
	  {
		  if (strstr(line, "00004000 \"BR bootloader\""))
		  {
			  fclose (fp);
			  return "2";
		  }
		  else
		  if (strstr(line, "00020000 \"BR bootloader\""))
		  {
			  fclose (fp);
			  return "1";
		  }
		  
	  }
	  fclose(fp);
	}

	return "unknown";
}
