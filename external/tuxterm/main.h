/* 
TuxTerm v0.2

Written 10/2006 by Seddi
Contact: seddi@ihad.tv / http://www.ihad.tv
*/

#ifndef __MAIN_H__

#define __MAIN_H__

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H

#ifdef HAVE_CONFIG_H
        #include "config.h"
#endif

//freetype stuff
//#define FONT "/usr/share/fonts/md_khmurabi_10.ttf"
//#define FONT "/usr/share/fonts/courier.ttf"
#define FONT "/usr/share/fonts/lucon.ttf"
#define FONT2 "/var/tuxbox/config/lucon.ttf"

enum {LEFT, CENTER, RIGHT};
enum {VERY_SMALL, SMALL, BIG};

FT_Error		error;
FT_Library		library;
FTC_Manager		manager;
FTC_SBitCache		cache;
FTC_SBit		sbit;
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
FTC_ImageTypeRec	desc;
#else
FTC_Image_Desc		desc;
#endif
FT_Face			face;
FT_UInt			prev_glyphindex;
FT_Bool			use_kerning;

#define FONTHEIGHT_VERY_SMALL 16
#define FONTHEIGHT_SMALL      32
#define FONTHEIGHT_BIG        40

unsigned char *lfb, *lbb, *lbc;

int StartX;
int StartY;
int PosX;
int PosY;
int fb;

enum {FILL, GRID};

struct fb_fix_screeninfo fix_screeninfo;
struct fb_var_screeninfo var_screeninfo;
struct fb_var_screeninfo var_screeninfo_original;

/* define color names */
enum
{
	black = 0,
	red, /* 1 */
	green, /* 2 */
	yellow, /* 3 */
	blue,	/* 4 */
	magenta,	/* 5 */
	cyan,	/* 6 */
	white, /* 7 */
	transp,
	SIZECOLTABLE
};

/* color line templates */
#ifdef FB8BIT
unsigned char colorline[SIZECOLTABLE][720*1];
#else
unsigned char colorline[SIZECOLTABLE][720*4];
#endif
	
#endif
