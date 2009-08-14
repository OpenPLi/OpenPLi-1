/* 
TuxTerm v0.2

Written 10/2006 by Seddi
Contact: seddi@ihad.tv / http://www.ihad.tv
*/

#ifndef __COLORS_H__

#define __COLORS_H__

/* colormap */
const unsigned short defaultcolors[] =	/* 0x0bgr */
{
	// black,red,green,yellow,blue,magneta,cyan,white
	//0x000, 0x00f, 0x0f0, 0x0ff, 0xf00, 0xf0f, 0xff0, 0xfff, 0x000
	0x000, 0x00f, 0x0f0, 0x0ff, 0xf22, 0xf0f, 0xff0, 0xfff, 0x000
};

/* 32bit colortable */
unsigned char bgra[][5] = { 
"\0\0\0\xFF\0", "\0\0\0\xFF\0", "\0\0\0\xFF\0", "\0\0\0\xFF\0",
"\0\0\0\xFF\0", "\0\0\0\xFF\0", "\0\0\0\xFF\0", "\0\0\0\xFF\0", "\0\0\0\0\0" };

/* 8bit color table */
unsigned short rd0[] = {0xff,0,0,0,0,0,0,0xff,0};
unsigned short gn0[] = {0,0,0,0,0,0,0,0xff,0};
unsigned short bl0[] = {0,0,0,0,0,0,0,0xff,0};
unsigned short tr0[] = {0,0,0,0,0,0,0,0,0xFFFF};
struct fb_cmap colormap_0 = {0, SIZECOLTABLE, rd0, gn0, bl0, tr0};

#endif
