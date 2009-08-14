#ifndef DISABLE_FILE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <linux/fb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <string.h>
#include <errno.h>
#include "fb_display.h"
#include <lib/picviewer/pictureviewer.h>
#include <lib/gdi/fb.h>
#include <lib/system/info.h>
#include <lib/base/eerror.h>


/*
 * FrameBuffer Image Display Function
 * (c) smoku/2000
 *
 */

/* Public Use Functions:
 *
 * extern void fb_display(unsigned char *rgbbuff,
 *     int x_size, int y_size,
 *     int x_pan, int y_pan,
 *     int x_offs, int y_offs,
 *     int winx, int winy);
 *
 * extern void getCurrentRes(int *x,int *y);
 *
 */

typedef unsigned int uint32_t;

unsigned short red[256], green[256], blue[256], transp[256];
struct fb_cmap map332 = {0, 256, red, green, blue, transp};
unsigned short red_b[256], green_b[256], blue_b[256], transp_b[256];
struct fb_cmap map_back = {0, 256, red_b, green_b, blue_b, transp_b};

unsigned char *lfb = 0;

//////////// YUV framebuffer support ///////////////////

int yuv_initialized;
unsigned short lut[3 * (NR_RED + NR_GREEN + NR_BLUE) ];

void build_yuv_lut(void)
{
	int i;
	for (i=0; i<NR_RED; ++i)
	{
		int r = i * 256 / NR_RED;
		lut_r_y[i] = Y(r, 0, 0);
		lut_r_u[i] = U(r, 0, 0);
		lut_r_v[i] = V(r, 0, 0);
	}
	
	for (i=0; i<NR_GREEN; ++i)
	{
		int g = i * 256 / NR_GREEN;
		lut_g_y[i] = Y(0, g, 0);
		lut_g_u[i] = U(0, g, 0);
		lut_g_v[i] = V(0, g, 0);
	}
	
	for (i=0; i<NR_BLUE; ++i)
	{
		int b = i * 256 / NR_BLUE;
		lut_b_y[i] = Y(0, 0, b);
		lut_b_u[i] = U(0, 0, b);
		lut_b_v[i] = V(0, 0, b);
	}
}

void clear_yuv_fb()
{
	unsigned char *dest = lfb;
	for (int i=0; i < 720*576; ++i)
	{
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
		dest[0] = (lut_r_y[r] + lut_g_y[g] + lut_b_y[b]) >> 8;
		if (i & 1)
			dest[720*576] = ((lut_r_u[r] + lut_g_u[g] + lut_b_u[b]) >> 8) + 128;
		else
			dest[720*576] = ((lut_r_v[r] + lut_g_v[g] + lut_b_v[b]) >> 8) + 128;
		++dest;
	}
}

void blit_to_yuv_fb(int x_pos, int y_pos, int x_size, int y_size, unsigned char *source)
{
	for (int y=y_pos; y < y_pos+y_size; ++y)
	{
		unsigned char *dest = lfb + y * 720 + x_pos;
		int x=x_pos;
		while(x < x_pos+x_size)
		{
			unsigned char r = *(source++);
			unsigned char g = *(source++);
			unsigned char b = *(source++);
			dest[0] = (lut_r_y[r] + lut_g_y[g] + lut_b_y[b]) >> 8;
			if (x & 1)
				dest[720*576] = ((lut_r_v[r] + lut_g_v[g] + lut_b_v[b]) >> 8) + 128;
			else
				dest[720*576] = ((lut_r_u[r] + lut_g_u[g] + lut_b_u[b]) >> 8) + 128;
			++dest;
			++x;
		}
	}
}

/////////////////// YUV end /////////////////////////////


int openFB(const char *name);
//void closeFB(int fh);
//void getVarScreenInfo(int fh, struct fb_var_screeninfo *var);
//void setVarScreenInfo(int fh, struct fb_var_screeninfo *var);
void getFixScreenInfo(struct fb_fix_screeninfo *fix);
void set332map();
void blit2FB(void *fbbuff,
		unsigned int pic_xs, unsigned int pic_ys,
		unsigned int scr_xs, unsigned int scr_ys,
		unsigned int xp, unsigned int yp,
		unsigned int xoffs, unsigned int yoffs,
		unsigned int winx, unsigned int winy,
		int cpp);
void clearFB(int bpp, int cpp, int trans);
inline unsigned short make16color(uint32_t r, uint32_t g,
				uint32_t b, uint32_t rl,
				uint32_t ro, uint32_t gl,
				uint32_t go, uint32_t bl,
				uint32_t bo, uint32_t tl,
				uint32_t to);

void fb_display(unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs, int winx, int winy)
{
	struct fb_var_screeninfo *var;
	unsigned short *fbbuff = NULL;
	int bp = 0;
	if (rgbbuff == NULL)
		return;
	/* read current video mode */
	var = fbClass::getInstance()->getScreenInfo();
	lfb = fbClass::getInstance()->lfb;

	/* correct panning */
	if (x_pan > x_size - (int)var->xres) x_pan = 0;
	if (y_pan > y_size - (int)var->yres) y_pan = 0;
	/* correct offset */
	if (x_offs + x_size > (int)var->xres) x_offs = 0;
	if (y_offs + y_size > (int)var->yres) y_offs = 0;

	bool yuv_fb = eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM600PVR ||
		eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM500PLUS;

	if (!yuv_fb || var->bits_per_pixel < 16)
	{
		/* blit buffer 2 fb */
		fbbuff = (unsigned short *) convertRGB2FB(rgbbuff, x_size * y_size, var->bits_per_pixel, &bp);
		if (fbbuff == NULL)
			return;
		/* ClearFB if image is smaller */
		if (x_size < (int)var->xres || y_size < (int)var->yres)
			clearFB(var->bits_per_pixel, bp, 0xFF);
		blit2FB(fbbuff, x_size, y_size, var->xres, var->yres, x_pan, y_pan, x_offs, y_offs, winx, winy, bp);
		free(fbbuff);
	}
	else
	{
		if (yuv_fb && var->bits_per_pixel == 16)
		{
			if (!yuv_initialized)
			{
				build_yuv_lut();
				yuv_initialized=1;
			}
			if (x_size < (int)var->xres || y_size < (int)var->yres)
				clear_yuv_fb();
			blit_to_yuv_fb(x_offs, y_offs, x_size, y_size, rgbbuff);
		}
	}
}

void getCurrentRes(int *x, int *y)
{
	struct fb_var_screeninfo *var;
	if (fbClass::getInstance())
	{
		var = fbClass::getInstance()->getScreenInfo();
		*x = var->xres;
		*y = var->yres;
	}
	else
	{
		*x = 720;
		*y = 576;
	}
}

void make332map(struct fb_cmap *map)
{
	int rs, gs, bs, i;
	int r = 8, g = 8, b = 4;

	map->start = 0;
	map->len   = 256;

	map->red = red;
	map->green = green;
	map->blue = blue;
	map->transp = transp;

	rs = 256 / (r - 1);
	gs = 256 / (g - 1);
	bs = 256 / (b - 1);

	for (i = 0; i < 256; i++)
	{
		map->red[i]   = (rs * ((i / (g * b)) % r)) << 8 ;
		map->green[i] = (gs * ((i / b) % g)) << 8 ;
		map->blue[i]  = (bs * ((i) % b))  << 8;
		map->transp[i]  = 0;
	}
}

/*
void set8map(int fh, struct fb_cmap *map)
{
	if (ioctl(fh, FBIOPUTCMAP, map) < 0) 
	{
		fprintf(stderr, "Error putting colormap");
		exit(1);
	}
}

void get8map(int fh, struct fb_cmap *map)
{
	if (ioctl(fh, FBIOGETCMAP, map) < 0) 
	{
		fprintf(stderr, "Error getting colormap");
		exit(1);
	}
}
*/

void set332map()
{
    make332map(&map332);
    fbClass::getInstance()->paletteSet(&map332);
}

void blit2FB(void *fbbuff,
	unsigned int pic_xs, unsigned int pic_ys,
	unsigned int scr_xs, unsigned int scr_ys,
	unsigned int xp, unsigned int yp,
	unsigned int xoffs, unsigned int yoffs,
	unsigned int winx, unsigned int winy,
	int cpp)
{
	int i, xc, yc;
	unsigned char *cp; unsigned short *sp; unsigned int *ip;
	ip = (unsigned int *) fbbuff;
	sp = (unsigned short *) ip;
	cp = (unsigned char *) sp;

	xc = (winx > scr_xs) ? scr_xs : winx;
	yc = (winy > scr_ys) ? scr_ys : winy;

	unsigned int stride = fbClass::getInstance()->Stride();

	// eDebug("[pictview_blit2FB] cpp=%d stride=%u, pic_x=%u, pic_y=%u scr_x=%u, scr_y=%u, xp=%u, yp=%u, xoffs=%u, yoffs=%u, size=%dk\n", cpp, stride, pic_xs, pic_ys, scr_xs, scr_ys, xp, yp, xoffs, yoffs, xc*yc*cpp/1024);
	switch(cpp)
	{
	case 1:
		set332map();	
		for (i = 0; i < yc; i++)
		{
			memcpy(lfb + (i + yoffs) * stride + xoffs, cp + (i + yp) * pic_xs + xp, xc);
	 	}
	 	break;
	case 2:
		for (i = 0; i < yc; i++)
		{
			memcpy(lfb + (i + yoffs) * stride + xoffs * cpp, sp + (i + yp) * pic_xs + xp, xc * cpp);
		}
		break;
	case 4:
		for (i = 0; i < yc; i++)
		{
			memcpy(lfb + (i + yoffs) * stride + xoffs * cpp, ip + (i + yp) * pic_xs + xp, xc * cpp);
		}
		break;
	}
	// eDebug("[pictview_blit2FB] done");
}

void clearFB(int bpp, int cpp, int trans)
{
	int x, y;
	getCurrentRes(&x, &y);
	unsigned int stride = fbClass::getInstance()->Stride();

	// eDebug("[clearFB] x=%d, y=%d, bpp=%d, cpp=%d, stride=%u", x, y, bpp, cpp, stride);
	switch(cpp)
	{
	case 2:
	{
		uint32_t rl, ro, gl, go, bl, bo, tl, to;
		unsigned int i;
		struct fb_var_screeninfo *var;
		var = fbClass::getInstance()->getScreenInfo();
		rl = (var->red).length;
		ro = (var->red).offset;
		gl = (var->green).length;
		go = (var->green).offset;
		bl = (var->blue).length;
		bo = (var->blue).offset;
		tl = (var->transp).length;
		to = (var->transp).offset;
		short black = make16color(0, 0, 0, rl, ro, gl, go, bl, bo, tl, to);
		unsigned short *s_fbbuff = (unsigned short *) malloc(y * stride / 2 * sizeof(unsigned short));
		if (s_fbbuff == NULL)
		{
			printf("Error: malloc\n");
			return;
		}
		for (i = 0; i < y * stride / 2; i++)
			s_fbbuff[i] = black;
		memcpy(lfb, s_fbbuff, y*stride);
		free(s_fbbuff);
		break;
	}
	case 4:
		int i;
		__u32 *dstp = (__u32*)lfb;
		trans = (trans & 0xFF) << 24;
		for (i = 0; i < x; i++)
			*dstp++ = trans;
		__u8 *dstptr = lfb;
		dstptr += stride;
		for (i = 1; i < y; i++)
		{
			memcpy(dstptr, lfb, stride);
			dstptr += stride;
		}
		break;
	default:
		memset(lfb, 0, stride * y);
	}
}

inline unsigned char make8color(unsigned char r, unsigned char g, unsigned char b)
{
	return (
	(((r >> 5) & 7) << 5) |
	(((g >> 5) & 7) << 2) |
	 ((b >> 6) & 3));
}

inline unsigned short make15color(unsigned char r, unsigned char g, unsigned char b)
{
	return (
	(((b >> 3) & 31) << 10) |
	(((g >> 3) & 31) << 5)  |
	 ((r >> 3) & 31));
}

inline unsigned short make16color(uint32_t r, uint32_t g, uint32_t b,
				 uint32_t rl, uint32_t ro,
				 uint32_t gl, uint32_t go,
				 uint32_t bl, uint32_t bo,
				 uint32_t tl, uint32_t to)
{
	return (
//		((0xFF >> (8 - tl)) << to) |
		((r >> (8 - rl)) << ro) |
		((g >> (8 - gl)) << go) |
		((b >> (8 - bl)) << bo));
}

void* convertRGB2FB(unsigned char *rgbbuff, unsigned long count, int bpp, int *cpp)
{
	unsigned long i;
	void *fbbuff = NULL;
	unsigned char *c_fbbuff;
	unsigned short *s_fbbuff;
	unsigned int *i_fbbuff;
	uint32_t rl, ro, gl, go, bl, bo, tl, to;

	struct fb_var_screeninfo *var;
	var = fbClass::getInstance()->getScreenInfo();
	rl = (var->red).length;
	ro = (var->red).offset;
	gl = (var->green).length;
	go = (var->green).offset;
	bl = (var->blue).length;
	bo = (var->blue).offset;
	tl = (var->transp).length;
	to = (var->transp).offset;

	// eDebug("[convertRGB2FB] bpp=%d count=%lu", bpp, count);
	switch(bpp)
	{
	case 8:
		*cpp = 1;
		c_fbbuff = (unsigned char *) malloc(count * sizeof(unsigned char));
		if (c_fbbuff == NULL)
		{
			eDebug("picviewer: convertRGB Error: malloc");
			return NULL;
		}
		for (i = 0; i < count; i++)
			c_fbbuff[i] = make8color(rgbbuff[i * 3], rgbbuff[i * 3 + 1], rgbbuff[i * 3 + 2]);
		fbbuff = (void *)c_fbbuff;
		break;
	case 15:
		*cpp = 2;
		s_fbbuff = (unsigned short *)malloc(count * sizeof(unsigned short));
		if (s_fbbuff == NULL)
		{
			eDebug("picviewer: convertRGB Error: malloc");
			return NULL;
		}
		for (i = 0; i < count; i++)
			s_fbbuff[i] = make15color(rgbbuff[i * 3], rgbbuff[i * 3 + 1], rgbbuff[i * 3 + 2]);
		fbbuff = (void *) s_fbbuff;
		break;
	case 16:
		*cpp = 2;
		s_fbbuff = (unsigned short *)malloc(count * sizeof(unsigned short));
		if (s_fbbuff == NULL)
		{
			eDebug("picviewer: convertRGB Error: malloc");
			return NULL;
		}
		for (i = 0; i < count; i++)
			s_fbbuff[i] = make16color(rgbbuff[i * 3], rgbbuff[i * 3 + 1], rgbbuff[i * 3 + 2], rl, ro, gl, go, bl, bo, tl, to);
		fbbuff = (void *)s_fbbuff;
		break;
	case 24:
	case 32:
		*cpp = 4;
		i_fbbuff = (unsigned int *) malloc(count * sizeof(unsigned int));
		if (i_fbbuff == NULL)
		{
			eDebug("picviewer: convertRGB Error: malloc");
			return NULL;
		}
		for (i = 0; i < count; i++)
			i_fbbuff[i] = 0xFF000000 | ((rgbbuff[i * 3] << 16) & 0xFF0000) |
						   ((rgbbuff[i * 3 + 1] << 8) & 0xFF00) |
						    (rgbbuff[i * 3 + 2] & 0xFF);
		fbbuff = (void *) i_fbbuff;
		break;
	default:
		eDebug("[convertRGB2FB] unknown colordepth %d.  Quitting", bpp);
		return NULL;
	}
	// eDebug("[convertRGB2FB] done");
	return fbbuff;
}
#endif
