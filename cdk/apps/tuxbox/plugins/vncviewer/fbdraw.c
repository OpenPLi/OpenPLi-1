#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "vncviewer.h"
#include "fbgl.h"
#include "overlay.h"

#if 0
#define DEBUG_CLIP
#endif

void
vp_pan_virt(int x0, int y0) {
	int w, h;
	IMPORT_FRAMEBUFFER_VARS
	
	if(v_scale==23)
	{
		w = 3 * pv_xsize / 2;
		h = 3 * pv_ysize / 2;
	}
	else
	{
		w = v_scale * pv_xsize;
		h = v_scale * pv_ysize;
	}

	if (v_xsize > w) {
		/* virtual screen wider than display */
		if (x0 + w > v_xsize) x0 = v_xsize - w;
		if (x0 < 0) x0 = 0;
	} else {
		/* width fits inside display */
		if (-x0 + v_xsize > w) x0 = v_xsize - w;
		if (x0 > 0) x0 = 0;
	}

	if (v_ysize > h) {
		/* virtual screen higher than display */
		if (y0 + h > v_ysize) y0 = v_ysize - h;
		if (y0 < 0) y0 = 0;
	} else {
		/* height fits inside display */
		if (-y0 + v_ysize > h) y0 = v_ysize - h;
		if (y0 > 0) y0 = 0;
	}

	/* ensure that x0 and y0 are integer multiples of the scale */
	if (v_scale==1) {
		/* ensure that coordinates are even */
		x0 &= ~1;
		y0 &= ~1;
	} else if (v_scale <5) {
		x0 = (x0 / v_scale) * v_scale;
		y0 = (y0 / v_scale) * v_scale;
	} else if (v_scale == 23) {
		//???
	}

	global_framebuffer.v_x0 = x0;
	global_framebuffer.v_y0 = y0;

	if ( 0 ) {
		/* HACK to tell window manager about new geometry */
		FILE *o;

		o=fopen("/tmp/fbwm.geom", "w");
		fprintf(o, "%d %d %d %d\n", x0, y0,
			v_xsize <= w ? v_xsize : w,
			v_ysize <= h ? v_ysize : h);
		fclose(o);

		system("killall -USR2 xfwm");
	}
	
	redraw_phys_all();
}

void
grid_pan(int dx, int dy) {
	int x0, y0;
	int gx, gy;
	IMPORT_FRAMEBUFFER_VARS

	gx = pv_xsize * v_scale / 2;
	gy = pv_ysize * v_scale / 2;

	if ((v_x0/gx)*gx == v_x0) {
		/* on grid */
		x0 = v_x0 + gx*dx;
	} else {
		/* snap to grid */
		x0 = (v_x0/gx)*gx;
		if (dx>0) x0 += gx;
	}
	if ((v_y0/gy)*gy == v_y0) {
		/* on grid */
		y0 = v_y0 + gy*dy;
	} else {
		/* snap to grid */
		y0 = (v_y0/gy)*gy;
		if (dy>0) y0 += gy;
	}

	if (dx>0 && v_x0 + v_scale * pv_xsize >= v_xsize) x0=v_x0;
	if (dy>0 && v_y0 + v_scale * pv_ysize >= v_ysize) y0=v_y0;

	if (dx==0) x0=v_x0;
	if (dy==0) y0=v_y0;

	if (x0 != v_x0 || y0 != v_y0)
		vp_pan_virt(x0, y0);
}

void
vp_pan(int pdx, int pdy) {
	int x0, y0;
	IMPORT_FRAMEBUFFER_VARS

	if(v_scale==23)
	{
		x0 = v_x0 + pdx*3/2;
		y0 = v_y0 + pdy*3/2;
	}
	else
	{
		x0 = v_x0 + pdx*v_scale;
		y0 = v_y0 + pdy*v_scale;
	}
	
	vp_pan_virt(x0, y0);
}

/* pixel format: 
 *   1111110000000000
 *   5432109876543210
 *   rrrrrggggggbbbbb
 *
 * intermediate format:
 *   33222222222211111111110000000000
 *   10987654321098765432109876543210
 *   xxRRRRRRRRRRGGGGGGGGGGBBBBBBBBBB
 *
 * maximum value = (max_pixel_component) * (scale*scale)
 *               = 63 * 16 = 63 * 4 * 4
 *               = 1008
 *		 < 1024 (10 bits per component)
 */

//#define RGBC(c) (((c) & 0x1f)<<1 | ((c)&0x7e0)<<5 | ((c)&0xf800)<<10)
#define RGBC(c) (((c) & 0x1f) | ((c)&0x3e0)<<5 | ((c)&0x7C00)<<10)
#define CR(c) ((c)>>20)
#define CG(c) ((c)>>10&1023)
#define CB(c) ((c)&1023)

typedef void aa_line_func(Pixel*src, Pixel*dst, int wp);

void
s1aa_line(Pixel *src, Pixel *dst, int wp)
{
	memcpy(dst, src, wp * sizeof(Pixel));
}

Pixel
s2aa_pixel(Pixel *src)
{
	CARD32 rgb, p0, p1;
	IMPORT_FRAMEBUFFER_VARS

	/* Dirty trick: read two pixels at once. 
	 * The order of halfwords within a word doesn't actually
	 * matter, because they are averaged anyway.
	 */
	p0 = *(CARD32*)src;
	p1 = *(CARD32*)(src+v_xsize);

	rgb = RGBC(p0&0xffff) + RGBC(p0>>16)
	    + RGBC(p1&0xffff) + RGBC(p1>>16);

//	return (rgb>>3 & 0x1f) | (rgb>>7 & 0x7e0) | (rgb>>12 & 0xf800);
	return (rgb>>2 & 0x1f) | (rgb>>7 & 0x7e0) | (rgb>>12 & 0x7C00);
}

void s2_3aa_line(Pixel *src, Pixel *dst, int wp)
{
	int i;
	IMPORT_FRAMEBUFFER_VARS

	for (i=0; i<wp; i+=2) {
		*dst = s2aa_pixel(src);
		dst++;
		src+=2;
		*dst = *src;
		src++;
		dst++;
	}
}

void s2_3aa_line2(Pixel *src, Pixel *dst, int wp)
{
	int i;
	IMPORT_FRAMEBUFFER_VARS

	for (i=0; i<wp; i+=2) {
		*(CARD32*)dst = *(CARD32*)src;
		dst+=2;
		src+=3;
	}
}

void
s2aa_line(Pixel *src, Pixel *dst, int wp)
{
	int i;
	IMPORT_FRAMEBUFFER_VARS
	
	for (i=0; i<wp; i++) {
		*dst = s2aa_pixel(src);
		src += 2;
		dst++;
	}
}

Pixel
s4aa_pixel(Pixel *src)
{
	CARD32 rgb = 0;
	int js;
	IMPORT_FRAMEBUFFER_VARS

	for (js=0; js<4; js++) {
		CARD32 p0, p1;
		Pixel *buf = src + js*v_xsize;

		p0 = *(CARD32*)(buf);
		p1 = *(CARD32*)(buf+2);

		rgb += RGBC(p0&0xffff) + RGBC(p0>>16)
		     + RGBC(p1&0xffff) + RGBC(p1>>16);
	}

	return (rgb>>4 & 0x1f) | (rgb>>9 & 0x7e0) | (rgb>>14 & 0x7c00);

}

void
s4aa_line(Pixel *src, Pixel *dst, int wp)
{
	int i;
	IMPORT_FRAMEBUFFER_VARS
	
	for (i=0; i<wp; i++) {
		*dst = s4aa_pixel(src);
		src += 4;
		dst++;
	}
}

void
s3aa_line(Pixel *src, Pixel *dst, int wp)
{
	int i;
	static bool initialized = 0;
	static Pixel pix_r[568], pix_g[568], pix_b[568];
	int xs;
	IMPORT_FRAMEBUFFER_VARS

	xs = v_xsize;
	if (!initialized) {
//		for (i=0; i<568; i++) {
		for (i=0; i<280; i++) {
			pix_b[i] = (i/9);
//			pix_g[i] = (i/9) <<5;
			pix_g[i] = (i/9) <<5;
//			pix_r[i] = (i/9) >>1<<11;
			pix_r[i] = (i/9) <<10;
		}
		initialized=1;
	}

	/***************************
	 *        src              *
	 *        [p0]p0 p0        *
	 *         p1 p1 p1        *
	 *         p2 p2 p2        *
	 *          r =====        *
	 *         === g ==        *
	 *         ====== b        *
	 *                         *
	 ***************************/

	for (i=0; i<wp; i++) {
		CARD32 p0, p1, p2, p;

		p0=RGBC(*(src     ))+RGBC(*(src+1     ))+RGBC(*(src+2     ));
		p1=RGBC(*(src + xs))+RGBC(*(src+1 + xs))+RGBC(*(src+2 + xs));
		p2=RGBC(*(src+2*xs))+RGBC(*(src+1+2*xs))+RGBC(*(src+2+2*xs));

		p = p0 + p1 + p2;
		*dst = pix_b[CB(p)] | pix_g[CG(p)] | pix_r[CR(p)];

		src += 3;
		dst++;
	}
}

void
draw_border(int xp, int yp, int wp, int hp,
	int oxp, int oyp, int owp, int ohp)
{
	Pixel *buf;
	int j;
	IMPORT_FRAMEBUFFER_VARS

#ifdef DEBUG_CLIP
	fprintf(stderr, "border xp=%d yp=%d wp=%d hp=%d oxp=%d oyp=%d owp=%d ohp=%d\n",
		xp, yp, wp, hp,
		oxp, oyp, owp, ohp);
#endif

	if (!wp || !hp) return;

	if (xp < oxp) wp-=oxp-xp, xp=oxp;
	if (yp < oyp) hp-=oyp-yp, yp=oyp;

	if (xp+wp > oxp+owp) wp = oxp-xp+owp;
	if (yp+hp > oyp+ohp) hp = oyp-yp+ohp;

	buf = p_buf + (yp+p_yoff)*p_xsize + xp + p_xoff;

	for (j=0; j<hp; j++) {
		memset(buf, 0, wp*sizeof(Pixel));
		buf += p_xsize;
	}
}

void 
redraw_phys_landscape(int xp, int yp, int wp, int hp)
{
	Pixel *src, *dst;
	int j;
	aa_line_func *aa_line = 0;
	int xv, yv;
	IMPORT_FRAMEBUFFER_VARS

	if(v_scale==23)
	{
		xv = v_x0 + xp*3/2;
		yv = v_y0 + yp*3/2;
	}
	else
	{
		xv = v_x0 + xp*v_scale;
		yv = v_y0 + yp*v_scale;
	}

	src = v_buf + yv*v_xsize + xv;
	dst = p_buf + (yp+p_yoff)*p_xsize + xp + p_xoff;

	switch(v_scale) {
	case 1: aa_line = s1aa_line; break;
	case 2: aa_line = s2aa_line; break;
	case 3: aa_line = s3aa_line; break;
	case 4: aa_line = s4aa_line; break;
	case 23: aa_line = s2_3aa_line; break;

		default:
			printf("Bad Scale: %d\n", v_scale);
		cleanup_and_exit("bad scale", EXIT_ERROR);
	}

	if(v_scale < 5)
	{
		for (j=0; j<hp; j++) {
			aa_line(src, dst, wp);
			src += v_xsize * v_scale;
			dst += p_xsize;
		}
	}
	else if(v_scale == 23)
	{
		for (j=0; j<hp; j+=2) {
			s2_3aa_line(src, dst, wp);
			src += 2*v_xsize;
			dst += p_xsize;
			s2_3aa_line2(src, dst, wp);
			src += v_xsize;
			dst += p_xsize;
		}
	}
}

void
redraw_virt(int xv, int yv, int wv, int hv) {
	int xp, yp, hp, wp;
	IMPORT_FRAMEBUFFER_VARS

#ifdef DEBUG_CLIP
	fprintf(stderr, "redraw_virt: xv=%d yv=%d wv=%d hv=%d\n", xv, yv, wv, hv);
#endif

	if (v_scale == 1) {
			xp = xv - v_x0;
			yp = yv - v_y0;
			wp = wv;
			hp = hv;
	} else {
		/* enlarge source to ensure that the rectangle edge lengths
		 * are evenly divisible by v_scale */
		int dx, dy;

		if (v_scale == 3) {
			/* need additional pixels for subpixel antialiasing */
			xv -= 3;
			wv += 6;
		}
		if(v_scale==23)
		{
			dx = xv % 3;
			dy = yv % 3;

			xv -= dx;
			yv -= dy;
			wv += dx;
			hv += dy;

			xp = (xv - v_x0) *2 / 3;
			yp = (yv - v_y0) *2 / 3;
			wp = (wv + v_scale-1) *2 / 3;
			hp = (hv + v_scale-1) *2 / 3;
			/* watch for right and bottom edge - might be 1 pixel 
			 * too large if size not divisible by scale */
			if (xv+3*wp/2 > v_xsize) wp--;
			if (yv+3*hp/2 > v_ysize) hp--;
		}
		else
		{
			dx = xv % v_scale;
			dy = yv % v_scale;

			xv -= dx;
			yv -= dy;
			wv += dx;
			hv += dy;

			xp = (xv - v_x0) / v_scale;
			yp = (yv - v_y0) / v_scale;
			wp = (wv + v_scale-1) / v_scale;
			hp = (hv + v_scale-1) / v_scale;
			/* watch for right and bottom edge - might be 1 pixel 
			 * too large if size not divisible by scale */
			if (xv+v_scale*wp > v_xsize) wp--;
			if (yv+v_scale*hp > v_ysize) hp--;
		}

	}

	/* clip to screen size */
	if (xp < 0) { wp += xp; xp = 0; }
	if (yp < 0) { hp += yp; yp = 0; }
	if (xp+wp > pv_xsize) wp = pv_xsize - xp;
	if (yp+hp > pv_ysize) hp = pv_ysize - yp;

	/* entirely off-screen ? */
	if (wp<=0 || hp<=0) return;

#ifdef DEBUG_CLIP
	fprintf(stderr, "\txp=%d yp=%d hp=%d wp=%d\n", xp, yp, hp, wp);
#endif

	redraw_phys_landscape(xp, yp, wp, hp);
	if (!hide_overlays) redraw_overlays(xp, yp, wp, hp);
}

void
redraw_phys(int xp, int yp, int wp, int hp) {
	int xv, yv;
	int oxp=xp, oyp=yp, owp=wp, ohp=hp;
	IMPORT_FRAMEBUFFER_VARS

#ifdef DEBUG_CLIP
	fprintf(stderr, "redraw_phys: xp=%d yp=%d wp=%d hp=%d\n", xp, yp, wp, hp);
#endif

	if(v_scale==23)
	{
		xv = v_x0 + xp*3/2;
		yv = v_y0 + yp*3/2;
	}
	else
	{
		xv = v_x0 + xp*v_scale;
		yv = v_y0 + yp*v_scale;
	}

	if (yv<0) {
		yp = -v_y0/v_scale;
		yv = 0;
		draw_border(0, 0, pv_xsize, yp,
						oxp, oyp, owp, ohp);
	}
	if (yv+hp*v_scale > v_ysize) {
		hp = (v_ysize - yv)/v_scale;
		draw_border(0, yp+hp, pv_xsize, pv_ysize-yp-hp,
						oxp, oyp, owp, ohp);
	}
	if (xv<0) {
		xp = -v_x0/v_scale;
		xv = 0;
		draw_border(0, yp, xp, hp,
						oxp, oyp, owp, ohp);
	}
	if (xv+wp*v_scale > v_xsize) {
		wp = (v_xsize - xv)/v_scale;
		draw_border(xp+wp, yp, pv_xsize-xp-wp, hp,
						oxp, oyp, owp, ohp);
	}

#ifdef DEBUG_CLIP
	fprintf(stderr, "\tclip: xp=%d yp=%d wp=%d hp=%d\n", xp, yp, wp, hp);
#endif

	redraw_phys_landscape(xp, yp, wp, hp);
	if (!hide_overlays) redraw_overlays(oxp, oyp, owp, ohp);
}

void
redraw_phys_all(void)
{
	IMPORT_FRAMEBUFFER_VARS
	
	redraw_phys(0, 0, pv_xsize, pv_ysize);
	redraw_all_overlays();
}
