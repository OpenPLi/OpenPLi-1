#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vncviewer.h"
#include "fbgl.h"

void
perror_exit(char *m)
{
	perror(m);
	exit(1);
}

void
die(char *m)
{
	fprintf(stderr, "Fatal: %s\n", m);
	exit(1);
}

#define FAIL(m) do { fprintf(stderr, "Warning: %s\n", (m)); return 0; } while(0)

void
skip_comment(FILE *f) {
	int c;
	
	if ((c=getc(f)) == '#') {
		while (getc(f) != '\n')
			;
	} else {
		ungetc(c, f);
	}
}

void
skip_whitespace(FILE *f) {
	int c;

	while (1) {
		c=getc(f);
		if (c!=' ' && c!='\t' && c!='\r' && c!='\n') break;
	}
	ungetc(c, f);
}

void
read_img_pixels(struct viewport *v, FILE *f, int format) {
	int i, j;
	int v_xsize = v->v_xsize;
	int v_ysize = v->v_ysize;
	Pixel *v_buf = v->v_buf;

	for(j=0; j<v_ysize; j++) {
		static int bitsleft;
		static int byte;

		Pixel *dst = v_buf + j*v_xsize;

		bitsleft=0;
		for (i=0; i<v_xsize; i++) {
			int r, g, b;
			Pixel p;

			if (format=='6') {
				r=getc(f);
				g=getc(f);
				b=getc(f);
			} else if (format=='5') {
				r=getc(f);
				g=r;
				b=r;
			} else if (format=='4') {
				if (!bitsleft) {
					byte=getc(f);
					bitsleft=8;
				}
				-- bitsleft;
				if (byte & (1<<bitsleft)) {
					r=g=b=0;
				} else {
					r=g=b=255;
				}
			} else {
				die("bad format");
				exit(1);
			}

#if 1
/*			p = ( (r<<7) & 0x7c00 )
			  | ( (g<<2) & 0x03e0 )
			  | ( (b>>3) & 0x001f )
			  | 0x8000;*/
			p = ( (r<<8) & 0xf800 )
			  | ( (g<<3) & 0x07e0 )
			  | ( (b>>3) & 0x001f );
#else
			/* Dither matrix:
			 *  0 3  =>  0   48
			 *  2 1      32  16
			 */
			if (j&1) {
				if (i&1) {
					r += 16; if (r>255) r=255;
					g += 16; if (g>255) g=255;
					b += 16; if (b>255) b=255;
				} else {
					r += 32; if (r>255) r=255;
					g += 32; if (g>255) g=255;
					b += 32; if (b>255) b=255;
				}
			} else { 
				if (i&1) {
					r += 48; if (r>255) r=255;
					g += 48; if (g>255) g=255;
					b += 48; if (b>255) b=255;
				}
			}

			p = ( (r<<8) & 0xc000 )
			  | ( (r<<6) & 0x3000 )
			  | ( (r<<4) & 0x0800 )

			  | ( (g<<3) & 0x0600 )
			  | ( (g<<1) & 0x0180 )
			  | ( (g>>1) & 0x0060 )

			  | ( (b>>3) & 0x0018 )
			  | ( (b>>5) & 0x0006 )
			  | ( (b>>7) & 0x0001 )
			;
#endif

			*dst++ = p;
		}
	}
}

int
img_read(struct viewport *vport, FILE *f)
{
	int xs=0, ys=0;
	int format;

	if (feof(f)) {
		return 0;
	}

	if (getc(f) != 'P') FAIL("not a pnmraw file");

	format=getc(f);
	if (format < '4' || format > '6') FAIL("bad ppm type");
	
	skip_whitespace(f);
	skip_comment(f);
	fscanf(f, "%d %d", &xs, &ys);

	skip_whitespace(f);
	skip_comment(f);
	fscanf(f, "255\n");

#if 1
	fprintf(stderr, "xs=%d ys=%d\n", xs, ys);
#endif

	if (!xs || !ys) FAIL("bad image file");
	
	vport->v_xsize = xs;
	vport->v_ysize = ys;

	vport->v_buf = (Pixel *)malloc(xs*ys*2);
	printf("0x%X : Alloc %d bytes in img_read\n",vport->v_buf,xs*ys*2);
	if (!vport->v_buf) FAIL("out of memory");

	read_img_pixels(vport, f, format);
	return 1;
}

#if 0

#define PBASE 8

void
scale_block(struct img *out, struct img *in, int i, int j, int scale)
{
	CARD32 t[PBASE+1];
	CARD32 u0=0;
	int x, y;
	int xf, yf;
	int xsi, ysi, xso, yso;
	Pixel *src, *dst;

	/* printf("i=%d j=%d\n", i, j); */
	
	xsi = in->xsize;
	ysi = in->ysize;

	xso = out->xsize;
	yso = out->ysize;

	src = in->pix + (j * xsi + i)*PBASE;
	dst = out->pix + (j * xso + i)*scale;

	xf = 0;
	yf = 0;

	for (y=0; y<scale; y++) {
		t[y] = 0;
	}
	
	for (y=0; y<PBASE; y++) {
		CARD32 *ti = t;

		Pixel *dp = dst;

		/*** on entry ***
		 *
		 * y: t[0]  t[1]  t[2] ...
		 *
		 ****************/

		yf += scale;

		for (x=0; x<PBASE; x++) {
			/*** on entry ***
			 *
			 *     x-1      x     x+1
			 * y           t[x]  t[x+1]  t[x+2]
			 * y+1 t[x-1]  u0
			 *
			 *** during calculation ***
			 *
			 *     x-1      x     x+1
			 *            +------------+
			 * y          |out   T[x+1]| t[x+2]
			 * y+1 t[x-1] |T[x]  u0    |
			 *            +-update-----+
			 *
			 ****/
			Pixel p;
			CARD32 v;

			/* printf("x=%d y=%d\n", x, y); */
			p = src[x];

			v = ( (p&0xf800) << (21-11))
			  | ( (p&0x07c0) << (10-6))
			  | ( (p&0x001e) >> (1));

			xf += scale;

			if (xf < PBASE) {
				if (yf < PBASE) {
					*ti += v * scale*scale;
				} else {
					*ti += v * (scale-(yf-PBASE))*scale;
					u0  += v * (yf-PBASE)*scale;
				}
			} else {
				xf -= PBASE;

				if (yf < PBASE) {
					*ti += v * (scale-xf)*scale;
					++ ti;
					*ti += v * xf * scale;
				} else {
					CARD32 o;
					Pixel p;

					o = *ti + v * (scale-(yf-PBASE))*(scale-xf);
					p = ( (o&0xf8000000) >> (27-11))
					  | ( (o&0x001f8000) >> (15-5))
					  | ( (o&0x000003c0) >> 5);
					*dp++ = p;

					*ti = u0 + v * (yf-PBASE)*(scale-xf);

					++ ti;
					*ti += v * (scale-(yf-PBASE))*xf;
					u0 = v * (yf-PBASE)*xf;
				}
			}
		}
		if (yf >= PBASE) {
			yf -= PBASE;
			dst += xso;
		}
		src += xsi;
	}
}

void
img_scale(struct img *out, struct img *in, int scale)
{
	int xsi, ysi, xso, yso;
	int j;

	xsi = in->xsize;
	ysi = in->ysize;

	xso = xsi * scale / PBASE;
	yso = ysi * scale / PBASE;

	out->xsize = xso;
	out->ysize = yso;

	out->pix = (Pixel *)malloc(xso*yso*2);
	printf("Warning: leaking %d bytes in img_scale\n",xso*yso*2);
	if (!out->pix) die("out of memory!");

	if (scale > PBASE) die("scale >1 not implemented yet!");

	for (j=0; j<ysi/PBASE; ++j) {
		int i;

		for (i=0; i<xsi/PBASE; ++i) {
			scale_block(out, in, i, j, scale);
		}
	}
}

int 
main(int argc, char *argv[])
{
	FILE *f;
	char *vcmd = 0;
	int to_fb = 1;
	int scale;
	struct img img_in, img_out;
	clock_t t0, t1;

	if (argc<2) usage(argv[0]);

	scale=atoi(argv[1]);
	if (scale < 1 || scale > PBASE*PBASE) die("bad scale");

	if (argc<3) {
		f = stdin;
	} else {
		if (argv[2][0] == '|') {
			vcmd = argv[2]+1;
			f = stdin;
		} else {
			f=fopen(argv[2], "rb");
			if (!f) perror_exit(argv[2]);
		}
	}

	do {
		img_read(&img_in, f);

		t0 = clock();
		img_scale(&img_out, &img_in, scale);
		t1 = clock();
		free(img_in.pix);

#if 0
		fprintf(stderr, "%.0f msec\n", (t1-t0)*1000.0 / CLOCKS_PER_SEC); 
#endif

		if (vcmd) {
			FILE *o;
			o=popen(vcmd, "w");
			if (!o) perror("popen");
			img_write_ppm(&img_out, o);
			fclose(o);
		} else if (to_fb) {
			FILE *o;
			o=fopen("/dev/fb0", "wb");
			if (!o) perror("open /dev/fb0");
			img_write_fb(&img_out, o);
			fclose(o);
		} else {
			img_write_ppm(&img_out, stdout);
		}
		free(img_out.pix);
	} while (!feof(f));

	return 0;
}

#endif
