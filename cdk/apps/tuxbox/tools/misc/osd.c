/*
 *   osd.c - AViA OSD application (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2001 Gillem (htoa@gmx.net)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *   $Log: osd.c,v $
 *   Revision 1.4  2002/10/03 14:26:50  Jolt
 *   AViA AV cleanups
 *
 *   Revision 1.3  2002/08/21 08:37:05  obi
 *   no more compile warnings
 *
 *   Revision 1.2  2001/03/07 19:18:00  gillem
 *   - change some lines
 *
 *   Revision 1.1  2001/03/06 21:49:23  gillem
 *   - initial release
 *
 *
 *
 *   $Revision: 1.4 $
 *
 */

/* ---------------------------------------------------------------------- */

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <dbox/avia_av_osd.h>
#include "tuxbox.h"

/* ---------------------------------------------------------------------- */

void rgb2crycb( int r, int g, int b, int blend, uint32_t * pale )
{
	int cr,y,cb;

	if(!pale)
		return;

	y  = ((257*r  + 504*g + 98*b)/1000 + 16)&0x7f;
	cr = ((439*r  - 368*g - 71*b)/1000 + 128)&0x7f;
	cb = ((-148*r - 291*g + 439*b)/1000 + 128)&0x7f;

	*pale = (y<<16)|(cr<<9)|(cb<<2)|(blend&3);

	printf("OSD DATA: %d %d %d\n",cr,y,cb);
}

/* ---------------------------------------------------------------------- */

int main (int argc, char **argv)
{
	int fd;
	int x,y,z;

	struct sosd_create_frame osdf;
	uint32_t palette[16];
	uint32_t bitmap[20000];
	uint32_t i;
	uint32_t pale;
	unsigned char r,g,b;

	if ((fd = open("/dev/dbox/osd0",O_RDWR)) <= 0)
	{
		perror("open");
		return -1;
	}

	memset(bitmap,0,60000);

	osdf.framenr = 0;
	osdf.x = 100;
	osdf.y = 100;
	osdf.w = 200-1;
	osdf.h = 100*2; // odd + even
    osdf.gbf = 0x1f;
	osdf.pel = 1; // 4 bit * pixel

//	rgb2crycb( 100, 0, 0, 3, &pale );
//	palette[0] = 0;pale;

	/* set palette */
	for(i=1;i<16;i++)
	{
		r = header_data_cmap[i][0];
		g = header_data_cmap[i][1];
		b = header_data_cmap[i][2];
		rgb2crycb( r, g, b, 3, &pale );
		palette[i] = pale;
	}

	y=0;z=0;i=0;
	for(x=0;x<200*100;x++)
	{
		i |= (header_data[x]<<(28-(y*4)));

//		printf("%08X %d %d\n",i,y,header_data[x]);
		y++;
		if(y==8)
		{
			bitmap[z] = i;
			z++;
			y=0;
			i=0;
		}
	}

	printf("%d %d\n",z,x);

	osdf.psize = 16;
	osdf.palette = &palette;
	osdf.bsize = 200*100*4/32;	// half words ??? f*ck
	osdf.bitmap = &bitmap;

	if ( ioctl( fd, OSD_IOCTL_CREATE_FRAME, &osdf ) < 0 )
	{
		perror("ioctl");
	}

	osdf.framenr = 1;
	osdf.x = 350;
	osdf.y = 350;

	if ( ioctl( fd, OSD_IOCTL_CREATE_FRAME, &osdf ) < 0 )
	{
		perror("ioctl");
	}

	close(fd);

  return 0;
}
