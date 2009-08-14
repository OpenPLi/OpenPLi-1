/*
 *   switch.c - audio/video switch tool (dbox-II-project)
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
 *   $Log: switch_new.c,v $
 *   Revision 1.6  2001/04/25 08:03:59  fnbrd
 *   Kleinigkeiten.
 *
 *   Revision 1.5  2001/04/25 07:36:58  fnbrd
 *   Fixed mute/unmute.
 *
 *   Revision 1.4  2001/04/01 10:30:15  gillem
 *   - fix volume
 *
 *   Revision 1.3  2001/03/25 16:47:42  gillem
 *   - add options
 *
 *   Revision 1.2  2001/03/25 13:57:24  gillem
 *   - update includes
 *
 *   Revision 1.1  2001/03/20 21:16:00  gillem
 *   - switch rewrite
 *
 *
 *   $Revision: 1.6 $
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <argp.h>

#include "dbox/avs_core.h"

/* ---------------------------------------------------------------------- */

const char *argp_program_version = "switch 0.1";
const char *argp_program_bug_address = "<htoa@gmx.net>";
static char doc[] = "audio/video switch";

/* The options we understand. */
static struct argp_option options[] = {
  {"mute",                'm', 0,          0,  "Mute" },
  {"show",                's', 0,          0,  "Show current settings" },
  {"unmute",              'u', 0,          0,  "Unmute" },
  {"volume",              'l', "VOL",      0,  "Set volume (0-63, 0 loudest)" },
  {"zc-detector",         'z', "0/1",      0,  "Set zero cross detector" },
  {"video-fs-ctrl",       'f', "<0-3>",    0,  "Set video function switch control" },
  {"ycmix",               'y', "0/1",      0,  "Set y/c mix (only cxa2092)" },
  {"video-fb-ctrl",       'b', "<0-3>",    0,  "Set video fast blanking control" },
  {"logic",               'c', "0/1",      0,  "Set logic" },
  {"route-video",         'v', "SRC:DEST", 0,  "Route video" },
  {"route-audio",         'a', "SRC:DEST", 0,  "Route audio" },

  {0,0,0,0, "Routing sources:\nTV,VCR,AUX" },
  {0,0,0,0, "Routing TV destinations:\nDE1,DE2,VCR,AUX,DE3,DE4,DE5,VM1" },
  {0,0,0,0, "Routing VCR destinations:\nDE1,DE2,VCR,AUX,DE3,VM1,VM2,VM3" },
  {0,0,0,0, "Routing AUX destinations:\nDE1,VM1,VCR,AUX,DE2,VM2,VM3,VM4" },
  { 0 }
};

struct arguments
{
  int verbose;
};

/* ---------------------------------------------------------------------- */

static error_t parse_opt (int key, char *arg, struct argp_state *state);

int fd;

/* ---------------------------------------------------------------------- */

/* Our argp parser. */
static struct argp argp = { options, parse_opt, 0, doc };

char *dest[3] = {"TV","VCR","AUX"};

char *src[3][8] = {
{"DE1","DE2","VCR","AUX","DE3","DE4","DE5","VM1"},
{"DE1","DE2","VCR","AUX","DE3","VM1","VM2","VM3"},
{"DE1","VM1","VCR","AUX","DE2","VM2","VM3","VM4"}
};

/* ---------------------------------------------------------------------- */

int show_type()
{
	int i;

	if (ioctl(fd,AVSIOGTYPE,&i)< 0)
	{
		perror("AVSIOGTYPE:");
		return -1;
	}

	printf("Type: ");

	switch(i)
	{

		case CXA2092:
			printf("CXA2092");
			break;
		case CXA2126:
			printf("CXA2126");
			break;
		default:
			printf("unknown");
			break;
	}

	printf("\n");

	return 0;
}

int show_ycm()
{
	int i;

	if (ioctl(fd,AVSIOGYCM,&i)< 0)
	{
// Haesslich bei cxa2126		perror("AVSIOGYCM:");
		return -1;
	}

	printf("YCM: %d\n",i);

	return 0;
}

int show_zcd()
{
	int i;

	if (ioctl(fd,AVSIOGZCD,&i)< 0)
	{
		perror("AVSIOGZCD:");
		return -1;
	}

	printf("ZCD: %d\n",i);

	return 0;
}

int show_fnc()
{
	int i;

	if (ioctl(fd,AVSIOGFNC,&i)< 0)
	{
		perror("AVSIOGFNC:");
		return -1;
	}

	printf("FNC: %d\n",i);

	return 0;
}

int show_fblk()
{
	int i;

	if (ioctl(fd,AVSIOGFBLK,&i)< 0)
	{
		perror("AVSIOGFBLK:");
		return -1;
	}

	printf("FBLK: %d\n",i);

	return 0;
}

int show_volume()
{
	int i;

	if (ioctl(fd,AVSIOGVOL,&i)< 0)
	{
		perror("AVSIOGVOL:");
		return -1;
	}

	printf("TV Volume: %d\n",i);

	return 0;
}

int show_mute()
{
	int i;

	if (ioctl(fd,AVSIOGMUTE,&i)< 0)
	{
		perror("AVSIOGMUTE:");
		return -1;
	}

	if (i)
	{
		printf(" (muted)",i);
	}
	else
	{
		printf(" (unmuted)",i);
	}
	printf("\n");

	return 0;
}

/* ---------------------------------------------------------------------- */

int mute()
{
	int i;

	i=AVS_MUTE;

	if (ioctl(fd,AVSIOSMUTE,&i)< 0)
	{
		perror("AVSIOSMUTE:");
		return -1;
	}

	return 0;
}

int unmute()
{
	int i;

	i=AVS_UNMUTE;

	if (ioctl(fd,AVSIOSMUTE,&i)< 0)
	{
		perror("AVSIOSMUTE:");
		return -1;
	}

	return 0;
}

int set_volume(int i)
{
	if (i < 0)
	{
		i=0;
	}
	else if (i > 63)
	{
		i=63;
	}

	if (ioctl(fd,AVSIOSVOL,&i)< 0)
	{
		perror("AVSIOGVOL:");
		return -1;
	}

	return 0;
}

int set_zcd(int i)
{
	if (i < 0)
	{
		i=0;
	}
	else if (i > 1)
	{
		i=1;
	}

	if (ioctl(fd,AVSIOSZCD,&i)< 0)
	{
		perror("AVSIOSZCD:");
		return -1;
	}

	return 0;
}

int set_fnc(int i)
{
	if (i < 0)
	{
		i=0;
	}
	else if (i > 3)
	{
		i=3;
	}

	if (ioctl(fd,AVSIOSFNC,&i)< 0)
	{
		perror("AVSIOSFNC:");
		return -1;
	}


	return 0;
}

int set_ycm(int i)
{
	if (i < 0)
	{
		i=0;
	}
	else if (i > 1)
	{
		i=1;
	}

	if (ioctl(fd,AVSIOSYCM,&i)< 0)
	{
		perror("AVSIOSYCM:");
		return -1;
	}

	return 0;
}

int set_fblk(int i)
{
	if (i < 0)
	{
		i=0;
	}
	else if (i > 3)
	{
		i=3;
	}

	if (ioctl(fd,AVSIOSFBLK,&i)< 0)
	{
		perror("AVSIOSFBLK:");
		return -1;
	}

	return 0;
}

int set_video( int src, int dest )
{
	int i;

	switch(dest)
	{
		case 0:
			i = AVSIOSVSW1;
			break;

		case 1:
			i = AVSIOSVSW2;
			break;

		case 2:
			i = AVSIOSVSW3;
			break;

		default:
			return -1;
	}

	if (ioctl(fd,i,&src)< 0)
	{
		perror("AVSIOSVSW");
		return -1;
	}

	return 0;
}

int set_audio( int src, int dest )
{
	int i;

	switch(dest)
	{
		case 0:
			i = AVSIOSASW1;
			break;

		case 1:
			i = AVSIOSASW2;
			break;

		case 2:
			i = AVSIOSASW3;
			break;

		default:
			return -1;
	}

	if (ioctl(fd,i,&src)< 0)
	{
		perror("AVSIOSASW");
		return -1;
	}

	return 0;
}

/* ---------------------------------------------------------------------- */

int parse_route_arg(char *arg, int *s, int *d)
{
	int i,z;
	char *cs,*cd=0;

	if (!arg || !s || !d)
	{
		return -1;
	}

	cs = arg;

	for(i=0;i<strlen(arg);i++)
	{
		if (arg[i]==':')
		{
			z=i;
			cd = arg+i+1;
		}
	}

	if (cd==0)
	{
		return -1;
	}

	*d = -1;

	for(i=0;i<3;i++)
	{
		if ( strncmp(cd,dest[i],strlen(dest[i])) == 0 )
		{
			if ( strlen(cd) == strlen(dest[i]) )
			{
				*d = i;
			}
		}
	}

	if (*d==-1)
	{
		return -1;
	}

	*s = -1;

	for(i=0;i<8;i++)
	{
		if ( strncmp(cs,src[*d][i],strlen(src[*d][i])) == 0 )
		{
			if ( z == strlen(src[*d][i]) )
			{
				*s = i;
			}
		}
	}

	if (*s==-1)
	{
		return -1;
	}

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	int s,d;
	struct arguments *arguments = state->input;

	switch (key)
	{
		case 's':
			show_type();
			show_zcd();
			show_fnc();
			show_ycm();
			show_fblk();
			show_volume();
			show_mute();
			break;

		case 'm':
	        	mute();
			break;

		case 'u':
	        	unmute();
			break;

		case 'l':
			set_volume(atoi(arg));
			break;

		case 'z':
			set_zcd(atoi(arg));
			break;

		case 'f':
			set_fnc(atoi(arg));
			break;

		case 'y':
			set_ycm(atoi(arg));
			break;

		case 'v':
			if ( parse_route_arg(arg,&s,&d) < 0 )
			{
				return ARGP_ERR_UNKNOWN;
			}

			set_video(s,d);

			break;

		case 'a':
			if ( parse_route_arg(arg,&s,&d) < 0 )
			{
				return ARGP_ERR_UNKNOWN;
			}

			set_audio(s,d);

			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

/* ---------------------------------------------------------------------- */

int main (int argc, char **argv)
{
	int count,i;
	struct arguments arguments;

	if ((fd = open("/dev/dbox/avs0",O_RDWR)) <= 0)
	{
		perror("open");
		return -1;
	}

	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	close(fd);

	return 0;
}

/* ---------------------------------------------------------------------- */
