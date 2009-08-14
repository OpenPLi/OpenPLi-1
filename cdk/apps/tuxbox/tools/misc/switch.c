/*
 *   switch.c - audio/video swtich tool (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 Hunz (hunz@hunz.org)
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
 *   $Log: switch.c,v $
 *   Revision 1.13  2002/08/21 08:39:35  obi
 *   no more compile warnings
 *
 *   Revision 1.12  2002/07/25 00:17:38  woglinde
 *
 *
 *
 *   fixing some compiler warnings
 *
 *   Revision 1.11  2002/02/23 20:22:29  woglinde
 *   fixing compiler warnings mulitple-literal strings
 *
 *   Revision 1.10  2001/07/23 16:19:56  gillem
 *   - add get type
 *
 *   Revision 1.9  2001/04/25 08:03:59  fnbrd
 *   Kleinigkeiten.
 *
 *   Revision 1.7  2001/04/01 10:30:15  gillem
 *   - fix volume
 *
 *   Revision 1.6  2001/03/25 13:57:24  gillem
 *   - update includes
 *
 *   Revision 1.5  2001/03/14 17:04:53  gillem
 *   - fix mute/unmute
 *
 *   Revision 1.4  2001/03/03 17:50:19  gillem
 *   - fix dev filename
 *
 *   Revision 1.3  2001/03/03 17:08:37  gillem
 *   - devfs support
 *
 *   Revision 1.2  2001/03/03 11:17:03  gillem
 *   - bugfix
 *
 *   Revision 1.1.1.1  2001/03/03 00:11:15  gillem
 *
 *
 *
 *   $Revision: 1.13 $
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <dbox/avs_core.h>

#define VERSION "0.1"

int s_ioctl(void*,int);
void help(char*);
int s_show(void*);

int fd;
char dests[3][4]={"TV ","VCR","AUX"};

char  srcs_tv[8][4]={"DE1","DE2","VCR","AUX","DE3","DE4","DE5","VM1"};
char srcs_vcr[8][4]={"DE1","DE2","VCR","AUX","DE3","VM1","VM2","VM3"};
char srcs_aux[8][4]={"DE1","VM1","VCR","AUX","DE2","VM2","VM3","VM4"};

/* for future use *g* -Hunz
   struct switch_dev {
   char name[3];
   int audio_src_set;
   int audio_src_get;
   int video_src_set;
   int video_src_get;
   char sources[8][3];
   };
*/

void help (char *prog_name) {
  printf("Version %s\n",VERSION);
  printf("Usage: %s <switches>\n\n",prog_name);
  printf("Switches:\n"
  "-h,     --help            help\n"
  "-s,     --show            show current settings\n"
  "-v,     --volume <vol>    set TV volume (0-63, 0 loudest)\n"
  "-m,     --mute            mute TV volume\n"
  "-u,     --unmute          unmute TV volume\n"
  "-rv,    --route-video <dest> <src>               route VIDEO\n"
  "-ra,    --route-audio <dest> <src>               route AUDIO\n"
  "-zcd,   --zero-cross-detector <on/off>           set ZCD\n"
  "-fnc,   --video-function-switch-control <0/1/2/3 set FNC\n"
  "-ycm,   --y-c-mix <0/1>                          set Y/C Mix (only cxa2092)\n"
  "-fblk,  --video-fast-blanking-control <0/1/2/3>  set FBLK\n"
  "-log1,  --logic1 <0/1>                           set logic output 1\n"
  "-log2,  --logic2 <0/1>                           set logic output 2\n"
  "-log3,  --logic3 <0/1>                           set logic output 3\n"
  "-log4,  --logic4 <0/1>                           set logic output 4\n"
  "-ttv,  --trash-my-tv                            trashes your tv\n\n");
  exit(0);
}

int get_video_source(int dest) {
  int i=-1;

  if (dest == 1) {
    if (ioctl(fd,AVSIOGVSW1,&i)< 0) {
      perror("AVSIOGVSW1");
    }
  }
  else if (dest == 2) {
    if (ioctl(fd,AVSIOGVSW2,&i)< 0) {
      perror("AVSIOGVSW2");
    }
  }
  else if (dest == 3) {
    if (ioctl(fd,AVSIOGVSW3,&i)< 0) {
      perror("AVSIOGVSW3");
    }
  }
  return i;
}

void set_video_source(int dest,int source) {
  int i;
  
  i=source;

  if (dest == 1) {
    if (ioctl(fd,AVSIOSVSW1,&i)< 0) {
      perror("AVSIOSVSW1");
    }
  }
  else if (dest == 2) {
    if (ioctl(fd,AVSIOSVSW2,&i)< 0) {
      perror("AVSIOSVSW2");
    }
  }
  else if (dest == 3) {
    if (ioctl(fd,AVSIOSVSW3,&i)< 0) {
      perror("AVSIOSVSW3");
    }
  }
}

int get_audio_source(int dest) {
  int i=-1;

  if (dest == 1) {
    if (ioctl(fd,AVSIOGASW1,&i)< 0) {
      perror("AVSIOGASW1");
    }
  }
  else if (dest == 2) {
    if (ioctl(fd,AVSIOGASW2,&i)< 0) {
      perror("AVSIOGASW2");
    }
  }
  else if (dest == 3) {
    if (ioctl(fd,AVSIOGASW3,&i)< 0) {
      perror("AVSIOGASW3");
    }
  }
  return i;
}

void set_audio_source(int dest,int source) {
  int i=source;
  
  if (dest == 1) {
    if (ioctl(fd,AVSIOSASW1,&i)< 0) {
      perror("AVSIOSASW1");
    }
  }
  else if (dest == 2) {
    if (ioctl(fd,AVSIOSASW2,&i)< 0) {
      perror("AVSIOSASW2");
    }
  }
  else if (dest == 3) {
    if (ioctl(fd,AVSIOSASW3,&i)< 0) {
      perror("AVSIOSASW3");
    }
  }
}

void routing_show() {
  int count,i;

  for(count=1;count<=3;count++) {
    printf("%3s: ",dests[count-1]);
    i=get_audio_source(count);
    printf("Audio: ");
    if (count == 1) printf("%3s ",srcs_tv[i]);
    else if (count == 2) printf("%3s ",srcs_vcr[i]);
    else if (count == 3) printf("%3s ",srcs_aux[i]);
    printf("(%d) ",i);
    i=get_video_source(count);
    printf(",Video: ");
    if (count == 1) printf("%3s ",srcs_tv[i]);
    else if (count == 2) printf("%3s ",srcs_vcr[i]);
    else if (count == 3) printf("%3s ",srcs_aux[i]);
    printf("(%d)",i);
    printf("\n");
  }
}

void volume_show() {
  int i=10;

  if (ioctl(fd,AVSIOGVOL,&i)< 0) {
    perror("AVSIOGVOL:");
  }
  printf("TV Volume: %d",i);
  if (ioctl(fd,AVSIOGMUTE,&i)< 0) {
    perror("AVSIOGMUTE:");
  }
  if (i)
    printf(" (muted)");
  else
    printf(" (unmuted)");
  printf("\n");
}

void volume_set(int i) {

  if (i < 0)  i=0;
  if (i > 63) i=63;
  if (ioctl(fd,AVSIOSVOL,&i)< 0) {
    perror("AVSIOGVOL:");
  }
}

void volume_mute() {
  int i;

  i=AVS_MUTE; //AVS_MUTE_IM;
  if (ioctl(fd,AVSIOSMUTE,&i)< 0) {
    perror("AVSIOSMUTE:");
  }
}

void volume_unmute() {
  int i;

  i=AVS_UNMUTE; //AVS_UNMUTE_IM;
  if (ioctl(fd,AVSIOSMUTE,&i)< 0) {
    perror("AVSIOSMUTE:");
  }
}

/* insert by gillem */

void zcd_show() {
  int i=10;

  if (ioctl(fd,AVSIOGZCD,&i)< 0) {
    perror("AVSIOGZCD:");
  }
  else
   printf("ZCD: %d\n",i);
}

void zcd_set(int i) {
  /* nice ;-) */
  if (i < 0)  i=0;
  if (i > 1)  i=1;
  if (ioctl(fd,AVSIOSZCD,&i)< 0) {
    perror("AVSIOSZCD:");
  }
}

void fnc_show() {
  int i=10;

  if (ioctl(fd,AVSIOGFNC,&i)< 0) {
    perror("AVSIOGFNC:");
  }
  else
   printf("FNC: %d\n",i);
}

void fnc_set(int i) {
  /* nice ;-) */
  if (i < 0)  i=0;
  if (i > 3)  i=3;
  if (ioctl(fd,AVSIOSFNC,&i)< 0) {
    perror("AVSIOSFNC:");
  }
}

void fblk_show() {
  int i=10;

  if (ioctl(fd,AVSIOGFBLK,&i)< 0) {
    perror("AVSIOGFBLK:");
  }
  else
   printf("FBLK: %d\n",i);
}

void fblk_set(int i) {
  /* nice ;-) */
  if (i < 0)  i=0;
  if (i > 3)  i=3;
  if (ioctl(fd,AVSIOSFBLK,&i)< 0) {
    perror("AVSIOSFBLK:");
  }
}

/* cxa2092 extra stuff */
void ycm_show() {
  int i=10;

  if (ioctl(fd,AVSIOGYCM,&i)< 0) {
//Haesslich bei cxa2126 <- das geht hier nicht nach schoenheit ...
    perror("AVSIOGYCM:");
  }
  else
   printf("YCM: %d\n",i);
}

void ycm_set(int i) {
  /* nice ;-) */
  if (i < 0)  i=0;
  if (i > 1)  i=1;
  if (ioctl(fd,AVSIOSYCM,&i)< 0) {
    perror("AVSIOSYCM:");
  }
}

/* end insert */

int main (int argc, char **argv) {

  int count;
  int type;

  if ((fd = open("/dev/dbox/avs0",O_RDWR)) <= 0) {
    perror("open");
    return -1;
  }

	if (ioctl(fd,AVSIOGTYPE,&type)< 0) {
    	perror("can't get avs-type:");
		close(fd);
		return -1;
	}

	switch(type)
	{
		case CXA2092:
			printf("CXA2092 found\n");
			break;
		case CXA2126:
			printf("CXA2126 found\n");
			break;
		case STV6412:
			printf("STV6412 found\n");
			break;
		default:
			printf("unknown avs type\n");
			close(fd);
			return -1;
	}

  for(count=1;count<argc;count++) {

    /* -h or --help */
    if ((strcmp("-h",argv[count]) == 0) || (strcmp("--help",argv[count]) == 0)) {
        help(argv[0]);
    }

    /* -s or --show */
    else if ((strcmp("-s",argv[count]) == 0) || (strcmp("--show",argv[count]) == 0)) {
        routing_show();
        volume_show();
        zcd_show();
        fnc_show();
        ycm_show();
        fblk_show();
    }
    /* -v or --vol */
    else if ((strcmp("-v",argv[count]) == 0) || (strcmp("--vol",argv[count]) == 0)) {
        if (argc == count+1) {
            printf("No volume given\n");
            exit(1);

        } else if ((argv[count+1][0] < 0x30) || (argv[count+1][0] > 0x39)) {
            printf("No volume given\n");
            exit(1);
        } else {
            count++;
            volume_set(atoi(argv[count]));
        }
    }

    /* -ttv or --trash-my-tv */
    else if ((strcmp("-ttv",argv[count]) == 0) || (strcmp("--trash-my-tv",argv[count]) == 0)) {
      printf("\nGet your TV repaired for a low price here at our sponsor: http://www.dvb-shop.de\n\n    greetings -Hunz :-]\n\n");
      while(1) {
	fnc_set(0);
	printf(".");
//	fflush(stdout);
	usleep(500);
	fnc_set(1);
	printf("O");
//	fflush(stdout);
	usleep(500);
      }
    }

    /* -zcd or --zero-cross-detector */
    else if ((strcmp("-zcd",argv[count]) == 0) || (strcmp("--zero-cross-detector",argv[count]) == 0)) {
        if (argc == count+1) {
            printf("No status given\n");
            exit(1);
        } else {
            count++;
            if (strcmp("on",argv[count]) == 0)
                zcd_set(1);
            else
                zcd_set(0);
        }
    }

    /* -ycm */
    else if ((strcmp("-ycm",argv[count]) == 0) || (strcmp("--y-c-mix",argv[count]) == 0)) {
        if (argc == count+1) {
            printf("No status given\n");
            exit(1);
        } else {
            count++;
            if (strcmp("on",argv[count]) == 0)
                ycm_set(1);
            else
                ycm_set(0);
        }
    }

    /* -fnc */
    else if ((strcmp("-fnc",argv[count]) == 0) || (strcmp("--video-function-switch-control",argv[count]) == 0)) {
        if (argc == count+1) {
            printf("No value given\n");
            exit(1);
        } else if ((argv[count+1][0] < 0x30) || (argv[count+1][0] > 0x39)) {
            printf("No value given\n");
            exit(1);
        } else {
            count++;
            fnc_set(atoi(argv[count]));
        }
    }

    /* -fblk */
    else if ((strcmp("-fblk",argv[count]) == 0) || (strcmp("--video-fast-blanking-control",argv[count]) == 0)) {
        if (argc == count+1) {
            printf("No value given\n");
            exit(1);
        } else if ((argv[count+1][0] < 0x30) || (argv[count+1][0] > 0x39)) {
            printf("No value given\n");
            exit(1);
        } else {
            count++;
            fblk_set(atoi(argv[count]));
        }
    }

    /* -m or --mute */
    else if ((strcmp("-m",argv[count]) == 0) || (strcmp("--mute",argv[count]) == 0)) {
        volume_mute();
    }

    /* -u or --unmute */
    else if ((strcmp("-u",argv[count]) == 0) || (strcmp("--unmute",argv[count]) == 0)) {
        volume_unmute();
    }

    /* -rv or --route-video */
    else if ((strcmp("-rv",argv[count]) == 0) || (strcmp("--route-video",argv[count]) == 0)) {
        if (argc < count+3) {
            printf("No source/destination given\n");
            exit(1);
        } else if ((argv[count+1][0] < 0x31) || (argv[count+1][0] > 0x33) || (argv[count+2][0] < 0x30) || (argv[count+2][0] > 0x37)) {
            printf("No source/destination given\n");
            exit(1);
        } else {
            count+=2;
            set_video_source(argv[count-1][0]-0x30,argv[count][0]-0x30);
        }
    }

    /* -ra or --route-audio */
    else if ((strcmp("-ra",argv[count]) == 0) || (strcmp("--route-audio",argv[count]) == 0)) {
        if (argc < count+3) {
            printf("No source/destination given\n");
            exit(1);
        } else if ((argv[count+1][0] < 0x31) || (argv[count+1][0] > 0x33) || (argv[count+2][0] < 0x30) || (argv[count+2][0] > 0x37)) {
            printf("No source/destination given\n");
            exit(1);
        } else {
            count+=2;
            set_audio_source(argv[count-1][0]-0x30,argv[count][0]-0x30);
        }
    } else {
        help(argv[0]);
    }
  }

  close(fd);

  return 0;
}
