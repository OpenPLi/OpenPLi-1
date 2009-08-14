#ifndef __FB_H__
#define __FB_H__

#include <crypt.h>
#include <dbox/fp.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kd.h>
#include <linux/fb.h>
#include <errno.h>
#include <string>

#define USEFREETYPEFB
#define FB_DEV "/dev/fb/0"

#ifdef USEFREETYPEFB
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H
#define FONT "/share/fonts/pakenham.ttf"
#endif


class fbClass
{
#ifdef USEFREETYPEFB
	FT_Library		library;
	FTC_Manager		manager;
	FTC_SBitCache		cache;
	FTC_SBit		sbit;
#if FREETYPE_MAJOR  == 2 && FREETYPE_MINOR == 0
	FTC_ImageDesc		desc;
#else
	FTC_ImageTypeRec	desc;
#endif
	FT_Face			face;
	FT_UInt			prev_glyphindex;
	int use_kerning;
	int RenderChar(FT_ULong currentchar, int sx, int sy, int ex, int r, int g, int b);
	int GetStringLen(unsigned char *string);
#endif
	int fd, available;
	unsigned int xRes, yRes, stride, bpp;
	unsigned long x_stride;
	struct fb_var_screeninfo screeninfo, oldscreen;
	unsigned short red[256], green[256], blue[256], trans[256];

	static fbClass *instance;
	int showConsole(int state);

	void blit2FB(void *fbbuff,unsigned char *alpha,unsigned int pic_xs,unsigned int pic_ys,unsigned int scr_xs,unsigned int scr_ys, unsigned int xp, unsigned int yp,unsigned int xoffs, unsigned int yoffs,int cpp);

	void make332map();
	void make8map();
	void palette();

	void PaintPixel(int x, int y, int r, int g, int b);

	unsigned char *lfb;
	unsigned char* c_rect_buffer;
	int c_rect_buffer_cpp;

public:
	enum {LEFT, CENTER, RIGHT};
	static fbClass *getInstance();
	fbClass();
	~fbClass();

	int SetSAA(int value);//setzt den saa
	int SetMode(unsigned int nxRes, unsigned int nyRes, unsigned int nbpp);//setzt den Framebuffer
#ifdef USEFREETYPEFB
	FT_Error FTC_Face_Requester(FTC_FaceID face_id, FT_Face* aface);
	void RenderString(std::string word, int sx, int sy, int maxwidth, int layout, int size, int r, int g, int b);
#else
	int DrawString( int xpos, int ypos, int height, const char *msg, int r,int g, int b);
#endif
	void FillRect(int x, int y, int width, int height, int r, int g, int b);
	void DrawRect( int x, int y, int width, int height, int r, int g, int b);
	void DrawVLine(int x, int y, int sy, int r, int g, int b);
	void DrawHLine(int x, int y, int sx, int r, int g, int b);
	void RenderCircle(int sx, int sy, int r, int g, int b);
	void fb_display(unsigned char *rgbbuff,unsigned char * alpha,int x_size,int y_size,int x_pan,int y_pan,int x_offs,int y_offs);//für Bilder
	void W_buffer(int sx, int sy, int width, int height); //stellt den Bildausschnitt wieder her
	void Fill_buffer(int sx, int sy, int width, int height);//merkt sich einen Bildausschnitt
};

#endif //__FB_H__
