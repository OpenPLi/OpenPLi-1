/*
** initial coding by fx2
*/



#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <linux/fb.h>
#include <sys/time.h>
#include <sys/mman.h>

#ifndef USEX

#ifdef __i386__
#define fbdevname	"/dev/fb0"
#else
#define fbdevname	"/dev/fb/0"
#endif

#ifndef abs
#define abs(a)	((a>0)?a:-(a))
#endif

#include <draw.h>
#include <rcinput.h>
#include <pig.h>

static	int							fd = -1;
static	struct fb_var_screeninfo	screeninfo;
static	struct fb_var_screeninfo	oldscreen;
static	int							available = 0;
static	unsigned char				*lfb = 0;
static	int							stride;
static	int							bpp = 8;
static	struct fb_cmap				cmap;
static	unsigned short				red[ 256 ];
static	unsigned short				green[ 256 ];
static	unsigned short				blue[ 256 ];
static	unsigned short				trans[ 256 ];
static	int							lastcolor=0;
extern	unsigned short				actcode;
extern	unsigned short				realcode;
extern	int							doexit;


void	FBSetColor( int idx, uchar r, uchar g, uchar b )
{
	red[idx] = r<<8;
	green[idx] = g<<8;
	blue[idx] = b<<8;
	trans[idx] = idx ? 0 : 0xffff;

	if ( idx > lastcolor )
		lastcolor=idx;
}

void	FBSetColorEx( int idx, uchar r, uchar g, uchar b, uchar transp )
{
	red[idx] = r<<8;
	green[idx] = g<<8;
	blue[idx] = b<<8;
	trans[idx] = transp<<8;

	if ( idx > lastcolor )
		lastcolor=idx;

}

int	FBInitialize( int xRes, int yRes, int nbpp, int extfd )
{
	struct fb_fix_screeninfo	fix;

	if ( extfd != -1 )
	{
		fd = extfd;
	}
	else
	{
		fd = open( fbdevname, O_RDWR );
		if ( fd == -1 )
		{
			perror("failed - open " fbdevname);
			return(-1);
		}
	}
	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo)<0)
	{
		perror("failed - FBIOGET_VSCREENINFO");
		return(-1);
	}
	memcpy(&oldscreen,&screeninfo,sizeof(screeninfo));

	screeninfo.xres_virtual=screeninfo.xres=xRes;
	screeninfo.yres_virtual=screeninfo.yres=yRes;
	screeninfo.bits_per_pixel=nbpp;

	cmap.start=0;
	cmap.len=256;
	cmap.red=red;
	cmap.green=green;
	cmap.blue=blue;
	cmap.transp=trans;

	memset(red,100,sizeof(unsigned short)*256);
	memset(green,100,sizeof(unsigned short)*256);
	memset(blue,100,sizeof(unsigned short)*256);
	memset(trans,0xffff,sizeof(unsigned short)*256);

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0)
		perror("FBSetMode");
	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo)<0)
		perror("failed - FBIOGET_VSCREENINFO");

	FBSetColor( BLACK, 30, 30, 100 );
	FBSetColor( BNR0, 1, 1, 1 );
	FBSetColor( WHITE, 210, 210, 210 );
	FBSetColor( RED, 240, 50, 80 );

	if (ioctl(fd, FBIOPUTCMAP, &cmap )<0)
		perror("FBSetCMap");

	bpp = screeninfo.bits_per_pixel;

	if ( bpp != 8 )
		return(-1);

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("failed - FBIOGET_FSCREENINFO");
		return(-1);
	}

	available=fix.smem_len;
	stride = fix.line_length;
	lfb=(unsigned char*)mmap(0,available,PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);

	if ( !lfb )
	{
		perror("failed - mmap");
		return(-1);
	}

	memset(lfb,BLACK,stride * screeninfo.yres);

	return 0;
}

void	FBSetupColors( void )
{
	if (ioctl(fd, FBIOPUTCMAP, &cmap )<0)
		perror("FBSetCMap");
}

void	FBClose( void )
{
	/* clear screen */
	memset(lfb,0,stride * screeninfo.yres);

	if (available)
	{
        ioctl(fd, FBIOPUT_VSCREENINFO, &oldscreen);
		if (lfb)
			munmap(lfb, available);
	}
}

void	FBPaintPixel( int x, int y, unsigned char farbe )
{
	*(lfb + stride*y + x) = farbe;
}

unsigned char	FBGetPixel( int x, int y )
{
	return *(lfb + stride*y + x);
}

void	FBGetImage( int x1, int y1, int width, int height, unsigned char *to )
{
	int				y;
	unsigned char	*p=lfb + stride*y1 + x1;

	for( y=0; y<height; y++ )
	{
		memcpy(to,p,width);
		to+=width;
		p+=stride;
	}
}

void	FBDrawLine( int xa, int ya, int xb, int yb, unsigned char farbe )
{
	int dx = abs (xa - xb);
	int	dy = abs (ya - yb);
	int	x;
	int	y;
	int	End;
	int	step;

	if ( dx > dy )
	{
		int	p = 2 * dy - dx;
		int	twoDy = 2 * dy;
		int	twoDyDx = 2 * (dy-dx);

		if ( xa > xb )
		{
			x = xb;
			y = yb;
			End = xa;
			step = ya < yb ? -1 : 1;
		}
		else
		{
			x = xa;
			y = ya;
			End = xb;
			step = yb < ya ? -1 : 1;
		}

		FBPaintPixel (x, y, farbe);

		while( x < End )
		{
			x++;
			if ( p < 0 )
				p += twoDy;
			else
			{
				y += step;
				p += twoDyDx;
			}
			FBPaintPixel (x, y, farbe);
		}
	}
	else
	{
		int	p = 2 * dx - dy;
		int	twoDx = 2 * dx;
		int	twoDxDy = 2 * (dx-dy);

		if ( ya > yb )
		{
			x = xb;
			y = yb;
			End = ya;
			step = xa < xb ? -1 : 1;
		}
		else
		{
			x = xa;
			y = ya;
			End = yb;
			step = xb < xa ? -1 : 1;
		}

		FBPaintPixel (x, y, farbe);

		while( y < End )
		{
			y++;
			if ( p < 0 )
				p += twoDx;
			else
			{
				x += step;
				p += twoDxDy;
			}
			FBPaintPixel (x, y, farbe);
		}
	}
}

void	FBDrawHLine( int x, int y, int dx, unsigned char farbe )
{
	memset(lfb+x+stride*y,farbe,dx);
}

void	FBDrawVLine( int x, int y, int dy, unsigned char farbe )
{
	unsigned char	*pos = lfb + x + stride*y;
	int				i;

	for( i=0; i<dy; i++, pos += stride )
		*pos = farbe;
}

void	FBFillRect( int x, int y, int dx, int dy, unsigned char farbe )
{
	unsigned char	*pos = lfb + x + stride*y;
	int				i;

	for( i=0; i<dy; i++, pos += stride )
		memset(pos,farbe,dx);
}

void	FBDrawRect( int x, int y, int dx, int dy, unsigned char farbe )
{
	FBDrawHLine( x, y, dx, farbe );
	FBDrawHLine( x, y+dy, dx, farbe );
	FBDrawVLine( x, y, dy, farbe );
	FBDrawVLine( x+dx, y, dy, farbe );
}

void	FBCopyImage( int x, int y, int dx, int dy, unsigned char *src )
{
	int		i;

	if ( !dx || !dy )
		return;
	for( i=0; i < dy; i++ )
		memcpy(lfb+(y+i)*stride+x,src+dx*i,dx);
}

/* paint double size */
void	FB2CopyImage( int x1, int y1, int dx, int dy, unsigned char *src, int dbl )
{
	int				x;
	int				y;
	unsigned char	*d;
	unsigned char	*d1;
	unsigned char	*d2;
	unsigned char	*s;

	if ( !dx || !dy )
		return;
	if ( !dbl )
	{
		for( y=0; (y < dy) && (y+y1<screeninfo.yres); y++ )
		{
			for( x=0; (x < dx) && (x+x1<688) && (y+y1>=0); x++ )
			{
				if ( ( x+x1>=0 ) && *(src+dx*y+x) )
					*(lfb+(y1+y)*stride+x1+x) = *(src+dx*y+x);
			}
		}
		return;
	}
	s=src;
	for( y=0; (y < dy) && (y+y1+y<screeninfo.yres); y++ )
	{
		if ( y+y1+y<0 )
			continue;
		d=lfb+(y1+y+y)*stride+x1;
		d2=d+stride;
		d1=d-x1+688;
		for( x=0; (x < dx) && (d<d1); x++, s++ )
		{
			if ( (x+x1>=0) && *s )
			{
				*d=*s;
				d++;
				*d=*s;
				d++;
			}
			else
			{
				d+=2;
			}
		}
		// line 2
		if ( x )
			memcpy(d2,d-x-x,x+x);
	}
}

void	FBCopyImageCol( int x, int y, int dx, int dy, unsigned char col,
				unsigned char *src )
{
	int				i;
	int				k;
	unsigned char	*to;

	if ( !dx || !dy )
		return;
	for( i=0; i < dy; i++ )
	{
		to=lfb+(y+i)*stride+x;
		for( k=0; k<dx; k++, to++,src++ )
			*to=(*src + col);
	}
}

void	FBOverlayImage( int x, int y, int dx, int dy,
				int relx,					/* left start in under */
				int rely,					/* top start in under */
				unsigned char c1,			/* color instead of white in src */
				unsigned char *src,			/* on top (transp) */
				unsigned char *under,		/* links drunter */
				unsigned char *right,		/* rechts daneben */
				unsigned char *bottom )		/* darunter */
{
	int				i;
	int				k;
	unsigned char	*p;

#define SWC(a)	((a)==WHITE?c1:(a))

	if ( !dx || !dy )
		return;
	for( i=0; i < dy; i++ )
	{
		p = lfb+(y+i)*stride+x;
		for( k=0; k < dx; k++, p++ )
		{
			if ( *(src+dx*i+k) != BLACK )
				*p = SWC(*(src+dx*i+k));
			else if ( relx )
			{
				if ( relx+k >= dx )		/* use rigth */
				{
					if ( right )
						*p = *(right+dx*i+relx+k-dx);
					else
						*p = SWC(*(src+dx*i+k));
				}
				else
				{
					if ( under )
						*p = *(under+dx*i+relx+k);
					else
						*p = SWC(*(src+dx*i+k));
				}
			}
			else
			{
				if ( rely+i >= dy )		/* use bottom */
				{
					if ( bottom )
						*p = *(bottom+dx*(i+rely-dy)+k);
					else
						*p = SWC(*(src+dx*i+k));
				}
				else
				{
					if ( under )
						*p = *(under+dx*(i+rely)+k);
					else
						*p = SWC(*(src+dx*i+k));
				}
			}
		}
	}
}

void	FBPrintScreen( void )
{
	FILE			*fp;
	unsigned char	*p = lfb;
	int				y;
	int				x=0;

#define H(x)	((x/26)+65)
#define L(x)	((x%26)+65)

	fp = fopen( "/var/tmp/screen.xpm", "w" );
	if ( !fp )
	{
		return;
	}
	fprintf(fp,"/* XPM */\n");
	fprintf(fp,"static char *screen[] = {\n");
	fprintf(fp,"\"  %d    %d   %d    %d\"",screeninfo.xres,screeninfo.yres,
			lastcolor+1,lastcolor<=100?1:2);
	for( x=0; x < lastcolor; x++ )
	{
		if ( lastcolor <= 100 )
			fprintf(fp,",\n\"%c",x+65);
		else
			fprintf(fp,",\n\"%c%c",H(x),L(x));

		fprintf(fp, " c #%02x%02x%02x\"",
			(unsigned char)(red[x]>>8),
			(unsigned char)(green[x]>>8),
			(unsigned char)(blue[x]>>8));
	}
	for( y=0; y < screeninfo.yres; y++ )
	{
		fprintf(fp,",\n\"");
		for( x=0; x < screeninfo.xres; x++, p++ )
		{
			if ( lastcolor <= 100 )
				fprintf(fp,"%c",(*p)+65);
			else
				fprintf(fp,"%c%c",H(*p),L(*p));
		}
		fprintf(fp,"\"");
		fflush(fp);
	}
	fprintf(fp," };\n");
	fflush(fp);

	fclose(fp);
}

void	FBBlink( int x, int y, int dx, int dy, int count )
{
	unsigned char	*back;
	int				i;
	struct timeval	tv;

/* copy out */
	back = malloc(dx*dy);
	for( i=0; i < dy; i++ )
		memcpy(back+dx*i,lfb+(y+i)*stride+x,dx);

	for( i=0; i < count; i++ )
	{
		tv.tv_usec = 300000;
		tv.tv_sec = 0;
		select(0,0,0,0,&tv);
		FBFillRect( x, y, dx, dy, BLACK );
		tv.tv_usec = 300000;
		tv.tv_sec = 0;
		select(0,0,0,0,&tv);
		FBCopyImage( x, y, dx, dy, back );
	}
	free( back );
}

void	FBMove( int x, int y, int x2, int y2, int dx, int dy )
{
	unsigned char	*back;
	int				i;
	unsigned long	f=y*stride+x;
	unsigned long	t=y2*stride+x2;

	if ( f < t )
	{
		back=malloc(dx*dy);
		for( i=0; i < dy; i++, f+=stride )
			memcpy(back+(i*dx),lfb+f,dx);

		FBCopyImage( x2, y2, dx, dy, back );
		free( back );
		return;
	}
	for( i=0; i < dy; i++, f+=stride, t+=stride )
		memcpy(lfb+t,lfb+f,dx);
}

#if 0
static	void	FBDrawClock()
{
	float			winkel;
	struct timeval	tv;
	int				xe = 0;
	int				ye = 0;
	int				x;
	int				y;
	unsigned char	*back;
	int				i;

/* copy out */
	back = malloc(50*50);
	for( i=0; i < 50; i++ )
		memcpy(back+50*i,lfb+(280-25+i)*stride+360-25,50);

	for( winkel=0; winkel < 3.1415*2; winkel+=0.001 )
	{
		x = sin(winkel)*24;
		y = -cos(winkel)*24;
		if (( xe == x ) && (ye == y ))
			continue;
		xe=x;
		ye=y;
		FBDrawLine( 360, 280 , xe + 360,  ye + 280, 2 );
		tv.tv_usec = 1000;
		tv.tv_sec = 0;
		select(0,0,0,0,&tv);
	}
	xe=0;
	ye=0;
	for( winkel=0; winkel < 3.1415*2; winkel+=0.001 )
	{
		x = sin(winkel)*22;
		y = -cos(winkel)*22;
		if (( xe == x ) && (ye == y ))
			continue;
		xe=x;
		ye=y;
		FBDrawLine( 360, 280 , xe + 360,  ye + 280, 1 );
		tv.tv_usec = 1000;
		tv.tv_sec = 0;
		select(0,0,0,0,&tv);
	}

	FBCopyImage( 360-25, 280-25, 50, 50, back );

	free( back );
}
#endif

void	FBPause( void )
{
	int				i;
	struct timeval	tv;

#if 0
	/* old pause style, disabled by alexW */

	static int			pos[64] =
	{
	0, 2, 11, 23, 5, 31, 33, 29, 14, 53, 59, 36, 35, 3, 49, 1,
	16, 19, 56, 34, 58, 32, 51, 12, 4, 52, 63, 7, 57, 50, 6, 24,
	43, 48, 39, 8, 20, 44, 27, 42, 10, 55, 61, 21, 17, 37, 47, 25,
	54, 28, 18, 46, 60, 9, 30, 38, 62, 26, 15, 13, 41, 22, 40, 45
	};
	unsigned char	*back;
	int				dx = screeninfo.xres;
	int				dy = screeninfo.yres;
	int				x;
	int				y;
	int				sx;
	int				sy;

	Fx2PigPause();

	back = malloc( available );
	if ( back )		/* dimm out */
	{
		memcpy(back,lfb,available);

		for( i=0; i < 64; i++ )
		{
			sx = pos[i] >> 3;
			sy = pos[i] & 0x7;
			for( y=0; y < dy; y+=8 )
			{
				for( x=0; x < dx; x+=8 )
				{
					FBPaintPixel(x+sx,y+sy,0);
				}
			}
		}
	}

	while( realcode != 0xee )
		RcGetActCode();
	actcode = 0xee;

	while( actcode != RC_SPKR )
	{
		tv.tv_usec = 100000;
		tv.tv_sec = 0;
		select(0,0,0,0,&tv);
		RcGetActCode();
	}

	if ( back )		/* dimm in */
	{
		for( i=0; i < 64; i++ )
		{
			sx = pos[i] >> 3;
			sy = pos[i] & 0x7;
			for( y=0; y < dy; y+=8 )
			{
				for( x=0; x < dx; x+=8 )
				{
					*(lfb+(y+sy)*stride+x+sx) = *(back+(y+sy)*stride+x+sx);
				}
			}
		}

		free(back);
	}

/* nochmal leer lesen */
	while( realcode != 0xee )
		RcGetActCode();
	actcode = 0xee;

/*	FBDrawClock(); */

	Fx2PigResume();
#else
	int j;
	unsigned short	trans_sav[ 256 ];
	unsigned char 	img_sav[ 42*100];

	memcpy(trans_sav, trans, 256 * sizeof( unsigned short) );

	Fx2PigPause();

	if (available)
	{
		//save image
		FBGetImage( 50, 50, 100, 42, img_sav );

		/* dimm out */
		for( i = 0; i < 129; i++ )
		{
			for( j = 0; j < 256; j++ )
			{
				if( (trans[j]>>8) < 128 )
				{
					trans[j] += 0x100;
				}
  			}
			FBSetupColors();
		}

		FBSetColor( RESERVED, 150, 210, 210 );
		FBSetupColors();
		FBDrawString( 50, 50, 42, "Pause", RESERVED, 0 );
	}

	while( realcode != 0xee )
		RcGetActCode();
	actcode = 0xee;
	while( realcode == 0xee )
	{
		tv.tv_usec = 100000;
		tv.tv_sec = 0;
		select(0,0,0,0,&tv);
		RcGetActCode();
	}

	if (available)
	{
		//restore image
		FBCopyImage( 50, 50, 100, 42, img_sav );

		/* dimm in */
		for( i = 0; i < 129; i++ )
		{
			for( j = 0; j < 256; j++ )
			{
				if( trans[j] > trans_sav[j] )
				{
					trans[j]-= 0x100;
				}
			}
			FBSetupColors();
		}
	}

	while( realcode != 0xee )
		RcGetActCode();
	actcode = 0xee;

	Fx2PigResume();
#endif
}

#else	/* USEX */

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>

#include <draw.h>
#include <rcinput.h>
#include <pig.h>

static	Display			*dpy = 0;
static	Window			window;
static	GC				gc;
static	unsigned long	colors[ 256 ];
static	int				planes=16;
static	int				xres;
static	int				yres;
extern	unsigned short	actcode;
extern	unsigned short	realcode;
extern	int				doexit;

void	FBSetColor( int idx, uchar r, uchar g, uchar b )
{
	switch( planes )
	{
	case 24 :
		colors[idx] = r<<16 | g<<8 | b;
		break;
	case 16 :
		colors[idx] = (r&0xf8)<<8 | (g&0xfc) << 3 | (b&0xf8)>>3;
		break;
	}
}

void	FBSetColorEx( int idx, uchar r, uchar g, uchar b, uchar transp )
{
	FBSetColor( idx, r, g, b );
}

void	FBSetupColors( void )
{
}

int	FBInitialize( int xRes, int yRes, int bpp, int extfd )
{
	dpy = XOpenDisplay(0);
	if ( !dpy )
		return -1;

	xres = xRes;
	yres = yRes;

	window = XCreateSimpleWindow(dpy,RootWindow(dpy,0),
			100, 100, xRes, yRes, 0, 0, 0 );

	XMapWindow(dpy,window);

	planes=DisplayPlanes(dpy,0);

	gc = XCreateGC( dpy, window, 0, 0 );
	XSetFunction( dpy,gc,GXcopy);

	FBSetColor( BLACK, 30, 30, 100 );
	FBSetColor( BNR0, 1, 1, 1 ); 
	FBSetColor( WHITE, 210, 210, 210 );
	FBSetColor( RED, 240, 50, 80 );

	XFlush(dpy);
	return dpy ? 0 : -1;
}

void	FBClose( void )
{
	XCloseDisplay(dpy);
}

void	FBPaintPixel( int x, int y, unsigned char col )
{
	XSetForeground(dpy,gc,colors[col]);
	XDrawPoint( dpy, window, gc, x, y );
}

unsigned char	FBGetPixel( int x, int y )
{
	XImage			*image;
	unsigned long	c;
	int				i;

	image=XGetImage(dpy,window,x,y,1,1,-1,ZPixmap);
	c=XGetPixel(image,0,0);
	XDestroyImage(image);
	for(i=0;i<256;i++)
		if (colors[i]==c)
			return i;
	return 0;
}

void	FBGetImage( int x1, int y1, int width, int height, unsigned char *to )
{
	XImage			*image;
	unsigned long	c;
	int				i;
	int				x;
	int				y;

	image=XGetImage(dpy,window,x1,y1,width,height,-1,ZPixmap);

	for( y=0; y<height; y++ )
	{
		for( x=0; x<width; x++, to++ )
		{
			c=XGetPixel(image,x,y);
			for(i=0;i<256;i++)
				if (colors[i]==c)
					break;
			*to=(unsigned char)(i&0xff);
		}
	}
	XDestroyImage(image);
}

void	FBDrawLine( int xa, int ya, int xb, int yb, unsigned char col )
{
	XSetForeground(dpy,gc,colors[col]);
	XDrawLine( dpy, window, gc, xa, ya, xb, yb );
}

void	FBDrawHLine( int x, int y, int dx, unsigned char col )
{
	XSetForeground(dpy,gc,colors[col]);
	XDrawLine( dpy, window, gc, x, y, x+dx, y );
}

void	FBDrawVLine( int x, int y, int dy, unsigned char col )
{
	XSetForeground(dpy,gc,colors[col]);
	XDrawLine( dpy, window, gc, x, y, x, y+dy );
}

void	FBFillRect( int x, int y, int dx, int dy, unsigned char col )
{
	XSetForeground(dpy,gc,colors[col]);
	XFillRectangle( dpy, window, gc, x, y, dx, dy );
}

void	FBDrawRect( int x, int y, int dx, int dy, unsigned char col )
{
	XSetForeground(dpy,gc,colors[col]);
	XDrawRectangle( dpy, window, gc, x, y, dx, dy );
}

void	FBCopyImage( int x, int y, int dx, int dy, unsigned char *src )
{
	int	i;
	int	k;

	for( k=0; k < dy; k++ )
		for( i=0; i < dx; i++, src++ )
			FBPaintPixel(i+x,k+y,*src);
}

void	FB2CopyImage( int x, int y, int dx, int dy, unsigned char *src, int dbl )
{
	int				i;
	int				k;

	if ( !dx || !dy )
		return;
	if( !dbl )
	{
		for( k=0; k < dy; k++ )
		{
			for( i=0; i < dx; i++, src++ )
			{
				if ( *src && ( i+x < 720 ) )
					FBPaintPixel(i+x,k+y,*src);
			}
		}
		return;
	}
	for( k=0; k < dy+dy; k+=2 )
	{
		for( i=0; i < dx+dx; i++, src++ )
		{
			if ( *src && ( i+x < 720 ) )
			{
				FBPaintPixel(i+x,k+y,*src);
				FBPaintPixel(i+x,k+y+1,*src);
				i++;
				FBPaintPixel(i+x,k+y,*src);
				FBPaintPixel(i+x,k+y+1,*src);
			}
			else
				i++;
		}
	}
}

void	FBOverlayImage(int x, int y, int dx, int dy, int relx, int rely,
					unsigned char c1,
					unsigned char *src,
					unsigned char *under,
					unsigned char *right,
					unsigned char *bottom )
{
	int				i;
	int				k;

#define SWC(a)	((a)==WHITE?c1:(a))

	if ( !dx || !dy )
		return;
	for( i=0; i < dy; i++ )
	{
		for( k=0; k < dx; k++ )
		{
			if ( *(src+dx*i+k) != BLACK )
				FBPaintPixel(k+x,i+y,SWC(*(src+dx*i+k)));
			else if ( relx )
			{
				if ( relx+k >= dx )		/* use rigth */
				{
					if ( right )
						FBPaintPixel(k+x,i+y,*(right+dx*i+relx+k-dx));
					else
						FBPaintPixel(k+x,i+y,SWC(*(src+dx*i+k)));
				}
				else
				{
					if ( under )
						FBPaintPixel(k+x,i+y,*(under+dx*i+relx+k));
					else
						FBPaintPixel(k+x,i+y,SWC(*(src+dx*i+k)));
				}
			}
			else
			{
				if ( rely+i >= dy )		/* use bottom */
				{
					if ( bottom )
						FBPaintPixel(k+x,i+y,*(bottom+dx*(i+rely-dy)+k));
					else
						FBPaintPixel(k+x,i+y,SWC(*(src+dx*i+k)));
				}
				else
				{
					if ( under )
						FBPaintPixel(k+x,i+y,*(under+dx*(i+rely)+k));
					else
						FBPaintPixel(k+x,i+y,SWC(*(src+dx*i+k)));
				}
			}
		}
	}
}

void	FBCopyImageCol( int x, int y, int dx, int dy, unsigned char col,
					unsigned char *src )
{
	int	i;
	int	k;

	for( k=0; k < dy; k++ )
		for( i=0; i < dx; i++, src++ )
			FBPaintPixel(i+x,k+y,*src+col);
}

void	FBBlink( int x, int y, int dx, int dy, int count )
{
}

void	FBMove( int x, int y, int x2, int y2, int dx, int dy )
{
	XCopyArea( dpy, window, window, gc, x, y, dx, dy, x2, y2 );
}

void	FBPrintScreen( void )
{
}

void	FBFlushGrafic( void )
{
	XFlush(dpy);
}

void	FBPause( void )
{
	int				dx = xres;
	int				dy = yres;
	int				x;
	int				y;
	int				sx;
	int				sy;
	int				i;
	struct timeval	tv;
static int			pos[64] =
{
0, 2, 11, 23, 5, 31, 33, 29, 14, 53, 59, 36, 35, 3, 49, 1,
16, 19, 56, 34, 58, 32, 51, 12, 4, 52, 63, 7, 57, 50, 6, 24,
43, 48, 39, 8, 20, 44, 27, 42, 10, 55, 61, 21, 17, 37, 47, 25,
54, 28, 18, 46, 60, 9, 30, 38, 62, 26, 15, 13, 41, 22, 40, 45
};

	for( i=0; i < 64; i++ )
	{
		sx = pos[i] >> 3;
		sy = pos[i] & 0x7;
		for( y=0; y < dy; y+=8 )
		{
			for( x=0; x < dx; x+=8 )
			{
				FBPaintPixel(x+sx,y+sy,0);
			}
		}
	}

printf("pause()\n");

	actcode = 0xee;

	while( actcode == 0xee )
	{
		tv.tv_usec = 300000;
		tv.tv_sec = 0;
		select(0,0,0,0,&tv);
		RcGetActCode();
	}
}

#endif

/* for all */

#define MAXMSG 1024
#define DWIDTH 132
#define NCHARS 128
#define NBYTES 9271

/* Pointers into data_table for each ASCII char */
const int asc_ptr[NCHARS] = {
/* ^@ */   0,      0,      0,      0,      0,      0,      0,      0,
/* ^H */   0,      0,      0,      0,      0,      0,      0,      0,
/* ^P */   0,      0,      0,      0,      0,      0,      0,      0,
/* ^X */   0,      0,      0,      0,      0,      0,      0,      0,
/*    */   1,      3,     50,     81,    104,    281,    483,    590,
/*  ( */ 621,    685,    749,    851,    862,    893,    898,    921,
/*  0 */1019,   1150,   1200,   1419,   1599,   1744,   1934,   2111,
/*  8 */2235,   2445,   2622,   2659,      0,   2708,      0,   2715,
/*  @ */2857,   3072,   3273,   3403,   3560,   3662,   3730,   3785,
/*  H */3965,   4000,   4015,   4115,   4281,   4314,   4432,   4548,
/*  P */4709,   4790,   4999,   5188,   5397,   5448,   5576,   5710,
/*  X */5892,   6106,   6257,      0,      0,      0,      0,      0,
/*  ` */  50,   6503,   6642,   6733,   6837,   6930,   7073,   7157,
/*  h */7380,   7452,   7499,   7584,   7689,   7702,   7797,   7869,
/*  p */7978,   8069,   8160,   8222,   8381,   8442,   8508,   8605,
/*  x */8732,   8888,   9016,      0,      0,      0,      0,      0
};

/*
 * Table of stuff to print. Format:
 * 128+n -> print current line n times.
 * 64+n  -> this is last byte of char.
 * else, put m chars at position n (where m
 * is the next elt in array) and goto second
 * next element in array.
 */
const unsigned char data_table[NBYTES] = {
/*             0     1     2     3     4     5     6     7     8     9 */
/*    0 */   129,  227,  130,   34,    6,   90,   19,  129,   32,   10,
/*   10 */    74,   40,  129,   31,   12,   64,   53,  129,   30,   14, 
/*   20 */    54,   65,  129,   30,   14,   53,   67,  129,   30,   14, 
/*   30 */    54,   65,  129,   31,   12,   64,   53,  129,   32,   10, 
/*   40 */    74,   40,  129,   34,    6,   90,   19,  129,  194,  130,
/*   50 */    99,    9,  129,   97,   14,  129,   96,   18,  129,   95,
/*   60 */    22,  129,   95,   16,  117,    2,  129,   95,   14,  129, 
/*   70 */    96,   11,  129,   97,    9,  129,   99,    6,  129,  194, 
/*   80 */   129,   87,    4,  101,    4,  131,   82,   28,  131,   87, 
/*   90 */     4,  101,    4,  133,   82,   28,  131,   87,    4,  101,
/*  100 */     4,  131,  193,  129,   39,    1,   84,   27,  129,   38, 
/*  110 */     3,   81,   32,  129,   37,    5,   79,   35,  129,   36, 
/*  120 */     5,   77,   38,  129,   35,    5,   76,   40,  129,   34, 
/*  130 */     5,   75,   21,  103,   14,  129,   33,    5,   74,   19, 
/*  140 */   107,   11,  129,   32,    5,   73,   17,  110,    9,  129,
/*  150 */    32,    4,   73,   16,  112,    7,  129,   31,    4,   72, 
/*  160 */    15,  114,    6,  129,   31,    4,   72,   14,  115,    5, 
/*  170 */   129,   30,    4,   71,   15,  116,    5,  129,   27,   97, 
/*  180 */   131,   30,    4,   69,   14,  117,    4,  129,   30,    4, 
/*  190 */    68,   15,  117,    4,  132,   30,    4,   68,   14,  117, 
/*  200 */     4,  129,   27,   97,  131,   30,    5,   65,   15,  116, 
/*  210 */     5,  129,   31,    4,   65,   14,  116,    4,  129,   31, 
/*  220 */     6,   64,   15,  116,    4,  129,   32,    7,   62,   16, 
/*  230 */   115,    4,  129,   32,    9,   61,   17,  114,    5,  129, 
/*  240 */    33,   11,   58,   19,  113,    5,  129,   34,   14,   55, 
/*  250 */    21,  112,    5,  129,   35,   40,  111,    5,  129,   36, 
/*  260 */    38,  110,    5,  129,   37,   35,  109,    5,  129,   38, 
/*  270 */    32,  110,    3,  129,   40,   27,  111,    1,  129,  193, 
/*  280 */   129,   30,    4,  103,    9,  129,   30,    7,  100,   15, 
/*  290 */   129,   30,   10,   99,   17,  129,   33,   10,   97,    6, 
/*  300 */   112,    6,  129,   36,   10,   96,    5,  114,    5,  129, 
/*  310 */    39,   10,   96,    4,  115,    4,  129,   42,   10,   95, 
/*  320 */     4,  116,    4,  129,   45,   10,   95,    3,  117,    3, 
/*  330 */   129,   48,   10,   95,    3,  117,    3,  129,   51,   10,
/*  340 */    95,    4,  116,    4,  129,   54,   10,   96,    4,  115, 
/*  350 */     4,  129,   57,   10,   96,    5,  114,    5,  129,   60, 
/*  360 */    10,   97,    6,  112,    6,  129,   63,   10,   99,   17, 
/*  370 */   129,   66,   10,  100,   15,  129,   69,   10,  103,    9, 
/*  380 */   129,   39,    9,   72,   10,  129,   36,   15,   75,   10, 
/*  390 */   129,   35,   17,   78,   10,  129,   33,    6,   48,    6, 
/*  400 */    81,   10,  129,   32,    5,   50,    5,   84,   10,  129, 
/*  410 */    32,    4,   51,    4,   87,   10,  129,   31,    4,   52, 
/*  420 */     4,   90,   10,  129,   31,    3,   53,    3,   93,   10, 
/*  430 */   129,   31,    3,   53,    3,   96,   10,  129,   31,    4, 
/*  440 */    52,    4,   99,   10,  129,   32,    4,   51,    4,  102, 
/*  450 */    10,  129,   32,    5,   50,    5,  105,   10,  129,   33, 
/*  460 */     6,   48,    6,  108,   10,  129,   35,   17,  111,   10, 
/*  470 */   129,   36,   15,  114,    7,  129,   40,    9,  118,    4, 
/*  480 */   129,  193,  129,   48,   18,  129,   43,   28,  129,   41, 
/*  490 */    32,  129,   39,   36,  129,   37,   40,  129,   35,   44, 
/*  500 */   129,   34,   46,  129,   33,   13,   68,   13,  129,   32, 
/*  510 */     9,   73,    9,  129,   32,    7,   75,    7,  129,   31, 
/*  520 */     6,   77,    6,  129,   31,    5,   78,    5,  129,   30, 
/*  530 */     5,   79,    5,  129,   20,   74,  132,   30,    4,   80,
/*  540 */     4,  129,   31,    3,   79,    4,  129,   31,    4,   79,
/*  550 */     4,  129,   32,    3,   78,    4,  129,   32,    4,   76, 
/*  560 */     6,  129,   33,    4,   74,    7,  129,   34,    4,   72, 
/*  570 */     8,  129,   35,    5,   72,    7,  129,   37,    5,   73, 
/*  580 */     4,  129,   39,    4,   74,    1,  129,  129,  193,  130,
/*  590 */   111,    6,  129,  109,   10,  129,  108,   12,  129,  107,
/*  600 */    14,  129,   97,    2,  105,   16,  129,   99,   22,  129,
/*  610 */   102,   18,  129,  105,   14,  129,  108,    9,  129,  194, 
/*  620 */   130,   63,   25,  129,   57,   37,  129,   52,   47,  129, 
/*  630 */    48,   55,  129,   44,   63,  129,   41,   69,  129,   38, 
/*  640 */    75,  129,   36,   79,  129,   34,   83,  129,   33,   28, 
/*  650 */    90,   28,  129,   32,   23,   96,   23,  129,   32,   17,
/*  660 */   102,   17,  129,   31,   13,  107,   13,  129,   30,    9, 
/*  670 */   112,    9,  129,   30,    5,  116,    5,  129,   30,    1, 
/*  680 */   120,    1,  129,  194,  130,   30,    1,  120,    1,  129, 
/*  690 */    30,    5,  116,    5,  129,   30,    9,  112,    9,  129, 
/*  700 */    31,   13,  107,   13,  129,   32,   17,  102,   17,  129, 
/*  710 */    32,   23,   96,   23,  129,   33,   28,   90,   28,  129, 
/*  720 */    34,   83,  129,   36,   79,  129,   38,   75,  129,   41, 
/*  730 */    69,  129,   44,   63,  129,   48,   55,  129,   52,   47, 
/*  740 */   129,   57,   37,  129,   63,   25,  129,  194,  129,   80, 
/*  750 */     4,  130,   80,    4,  129,   68,    2,   80,    4,   94, 
/*  760 */     2,  129,   66,    6,   80,    4,   92,    6,  129,   67, 
/*  770 */     7,   80,    4,   90,    7,  129,   69,    7,   80,    4, 
/*  780 */    88,    7,  129,   71,    6,   80,    4,   87,    6,  129, 
/*  790 */    72,   20,  129,   74,   16,  129,   76,   12,  129,   62, 
/*  800 */    40,  131,   76,   12,  129,   74,   16,  129,   72,   20, 
/*  810 */   129,   71,    6,   80,    4,   87,    6,  129,   69,    7, 
/*  820 */    80,    4,   88,    7,  129,   67,    7,   80,    4,   90, 
/*  830 */     7,  129,   66,    6,   80,    4,   92,    6,  129,   68, 
/*  840 */     2,   80,    4,   94,    2,  129,   80,    4,  130,  193, 
/*  850 */   129,   60,    4,  139,   41,   42,  131,   60,    4,  139, 
/*  860 */   193,  130,   34,    6,  129,   32,   10,  129,   31,   12, 
/*  870 */   129,   30,   14,  129,   20,    2,   28,   16,  129,   22, 
/*  880 */    22,  129,   24,   19,  129,   27,   15,  129,   31,    9, 
/*  890 */   129,  194,  129,   60,    4,  152,  193,  130,   34,    6, 
/*  900 */   129,   32,   10,  129,   31,   12,  129,   30,   14,  131, 
/*  910 */    31,   12,  129,   32,   10,  129,   34,    6,  129,  194, 
/*  920 */   129,   30,    4,  129,   30,    7,  129,   30,   10,  129, 
/*  930 */    33,   10,  129,   36,   10,  129,   39,   10,  129,   42, 
/*  940 */    10,  129,   45,   10,  129,   48,   10,  129,   51,   10, 
/*  950 */   129,   54,   10,  129,   57,   10,  129,   60,   10,  129, 
/*  960 */    63,   10,  129,   66,   10,  129,   69,   10,  129,   72, 
/*  970 */    10,  129,   75,   10,  129,   78,   10,  129,   81,   10, 
/*  980 */   129,   84,   10,  129,   87,   10,  129,   90,   10,  129, 
/*  990 */    93,   10,  129,   96,   10,  129,   99,   10,  129,  102, 
/* 1000 */    10,  129,  105,   10,  129,  108,   10,  129,  111,   10, 
/* 1010 */   129,  114,    7,  129,  117,    4,  129,  193,  129,   60, 
/* 1020 */    31,  129,   53,   45,  129,   49,   53,  129,   46,   59, 
/* 1030 */   129,   43,   65,  129,   41,   69,  129,   39,   73,  129, 
/* 1040 */    37,   77,  129,   36,   79,  129,   35,   15,  101,   15, 
/* 1050 */   129,   34,   11,  106,   11,  129,   33,    9,  109,    9, 
/* 1060 */   129,   32,    7,  112,    7,  129,   31,    6,  114,    6, 
/* 1070 */   129,   31,    5,  115,    5,  129,   30,    5,  116,    5,
/* 1080 */   129,   30,    4,  117,    4,  132,   30,    5,  116,    5,
/* 1090 */   129,   31,    5,  115,    5,  129,   31,    6,  114,    6, 
/* 1100 */   129,   32,    7,  112,    7,  129,   33,    9,  109,    9, 
/* 1110 */   129,   34,   11,  106,   11,  129,   35,   15,  101,   15,
/* 1120 */   129,   36,   79,  129,   37,   77,  129,   39,   73,  129,
/* 1130 */    41,   69,  129,   43,   65,  129,   46,   59,  129,   49,
/* 1140 */    53,  129,   53,   45,  129,   60,   31,  129,  193,  129, 
/* 1150 */    30,    4,  129,   30,    4,  100,    1,  129,   30,    4, 
/* 1160 */   100,    3,  129,   30,    4,  100,    5,  129,   30,   76,
/* 1170 */   129,   30,   78,  129,   30,   80,  129,   30,   82,  129, 
/* 1180 */    30,   83,  129,   30,   85,  129,   30,   87,  129,   30, 
/* 1190 */    89,  129,   30,   91,  129,   30,    4,  132,  193,  129, 
/* 1200 */    30,    3,  129,   30,    7,  129,   30,   10,  112,    1, 
/* 1210 */   129,   30,   13,  112,    2,  129,   30,   16,  112,    3, 
/* 1220 */   129,   30,   18,  111,    5,  129,   30,   21,  111,    6, 
/* 1230 */   129,   30,   23,  112,    6,  129,   30,   14,   47,    8, 
/* 1240 */   113,    6,  129,   30,   14,   49,    8,  114,    5,  129, 
/* 1250 */    30,   14,   51,    8,  115,    5,  129,   30,   14,   53, 
/* 1260 */     8,  116,    4,  129,   30,   14,   55,    8,  116,    5, 
/* 1270 */   129,   30,   14,   56,    9,  117,    4,  129,   30,   14, 
/* 1280 */    57,    9,  117,    4,  129,   30,   14,   58,   10,  117, 
/* 1290 */     4,  129,   30,   14,   59,   10,  117,    4,  129,   30, 
/* 1300 */    14,   60,   11,  117,    4,  129,   30,   14,   61,   11, 
/* 1310 */   116,    5,  129,   30,   14,   62,   11,  116,    5,  129, 
/* 1320 */    30,   14,   63,   12,  115,    6,  129,   30,   14,   64, 
/* 1330 */    13,  114,    7,  129,   30,   14,   65,   13,  113,    8, 
/* 1340 */   129,   30,   14,   65,   15,  111,    9,  129,   30,   14, 
/* 1350 */    66,   16,  109,   11,  129,   30,   14,   67,   17,  107, 
/* 1360 */    12,  129,   30,   14,   68,   20,  103,   16,  129,   30, 
/* 1370 */    14,   69,   49,  129,   30,   14,   70,   47,  129,   30, 
/* 1380 */    14,   71,   45,  129,   30,   14,   73,   42,  129,   30, 
/* 1390 */    15,   75,   38,  129,   33,   12,   77,   34,  129,   36, 
/* 1400 */    10,   79,   30,  129,   40,    6,   82,   23,  129,   44, 
/* 1410 */     3,   86,   15,  129,   47,    1,  129,  193,  129,  129, 
/* 1420 */    38,    3,  129,   37,    5,  111,    1,  129,   36,    7, 
/* 1430 */   111,    2,  129,   35,    9,  110,    5,  129,   34,    8, 
/* 1440 */   110,    6,  129,   33,    7,  109,    8,  129,   32,    7, 
/* 1450 */   110,    8,  129,   32,    6,  112,    7,  129,   31,    6, 
/* 1460 */   113,    6,  129,   31,    5,  114,    6,  129,   30,    5, 
/* 1470 */   115,    5,  129,   30,    5,  116,    4,  129,   30,    4, 
/* 1480 */   117,    4,  131,   30,    4,  117,    4,  129,   30,    4, 
/* 1490 */    79,    2,  117,    4,  129,   30,    5,   78,    4,  117, 
/* 1500 */     4,  129,   30,    5,   77,    6,  116,    5,  129,   30, 
/* 1510 */     6,   76,    8,  115,    6,  129,   30,    7,   75,   11, 
/* 1520 */   114,    6,  129,   30,    8,   73,   15,  112,    8,  129, 
/* 1530 */    31,    9,   71,   19,  110,    9,  129,   31,   11,   68, 
/* 1540 */    26,  107,   12,  129,   32,   13,   65,   14,   82,   36, 
/* 1550 */   129,   32,   16,   61,   17,   83,   34,  129,   33,   44, 
/* 1560 */    84,   32,  129,   34,   42,   85,   30,  129,   35,   40, 
/* 1570 */    87,   27,  129,   36,   38,   89,   23,  129,   38,   34, 
/* 1580 */    92,   17,  129,   40,   30,   95,   11,  129,   42,   26, 
/* 1590 */   129,   45,   20,  129,   49,   11,  129,  193,  129,   49, 
/* 1600 */     1,  129,   49,    4,  129,   49,    6,  129,   49,    8, 
/* 1610 */   129,   49,   10,  129,   49,   12,  129,   49,   14,  129,
/* 1620 */    49,   17,  129,   49,   19,  129,   49,   21,  129,   49,
/* 1630 */    23,  129,   49,   14,   65,    9,  129,   49,   14,   67, 
/* 1640 */     9,  129,   49,   14,   69,    9,  129,   49,   14,   71, 
/* 1650 */    10,  129,   49,   14,   74,    9,  129,   49,   14,   76, 
/* 1660 */     9,  129,   49,   14,   78,    9,  129,   49,   14,   80,
/* 1670 */     9,  129,   49,   14,   82,    9,  129,   49,   14,   84,
/* 1680 */     9,  129,   30,    4,   49,   14,   86,   10,  129,   30, 
/* 1690 */     4,   49,   14,   89,    9,  129,   30,    4,   49,   14, 
/* 1700 */    91,    9,  129,   30,    4,   49,   14,   93,    9,  129, 
/* 1710 */    30,   74,  129,   30,   76,  129,   30,   78,  129,   30, 
/* 1720 */    81,  129,   30,   83,  129,   30,   85,  129,   30,   87, 
/* 1730 */   129,   30,   89,  129,   30,   91,  129,   30,    4,   49, 
/* 1740 */    14,  132,  193,  129,   37,    1,  129,   36,    3,   77, 
/* 1750 */     3,  129,   35,    5,   78,   11,  129,   34,    7,   78, 
/* 1760 */    21,  129,   33,    7,   79,   29,  129,   32,    7,   79, 
/* 1770 */    38,  129,   32,    6,   80,    4,   92,   29,  129,   31, 
/* 1780 */     6,   80,    5,  102,   19,  129,   31,    5,   80,    6, 
/* 1790 */   107,   14,  129,   31,    4,   81,    5,  107,   14,  129, 
/* 1800 */    30,    5,   81,    6,  107,   14,  129,   30,    4,   81, 
/* 1810 */     6,  107,   14,  130,   30,    4,   81,    7,  107,   14, 
/* 1820 */   129,   30,    4,   80,    8,  107,   14,  130,   30,    5, 
/* 1830 */    80,    8,  107,   14,  129,   30,    5,   79,    9,  107, 
/* 1840 */    14,  129,   31,    5,   79,    9,  107,   14,  129,   31, 
/* 1850 */     6,   78,   10,  107,   14,  129,   32,    6,   76,   11, 
/* 1860 */   107,   14,  129,   32,    8,   74,   13,  107,   14,  129, 
/* 1870 */    33,   10,   71,   16,  107,   14,  129,   33,   15,   67, 
/* 1880 */    19,  107,   14,  129,   34,   51,  107,   14,  129,   35, 
/* 1890 */    49,  107,   14,  129,   36,   47,  107,   14,  129,   37, 
/* 1900 */    45,  107,   14,  129,   39,   41,  107,   14,  129,   41, 
/* 1910 */    37,  107,   14,  129,   44,   32,  107,   14,  129,   47, 
/* 1920 */    25,  111,   10,  129,   51,   16,  115,    6,  129,  119, 
/* 1930 */     2,  129,  193,  129,   56,   39,  129,   51,   49,  129, 
/* 1940 */    47,   57,  129,   44,   63,  129,   42,   67,  129,   40, 
/* 1950 */    71,  129,   38,   75,  129,   37,   77,  129,   35,   81, 
/* 1960 */   129,   34,   16,   74,    5,  101,   16,  129,   33,   11, 
/* 1970 */    76,    5,  107,   11,  129,   32,    9,   77,    5,  110, 
/* 1980 */     9,  129,   32,    7,   79,    4,  112,    7,  129,   31, 
/* 1990 */     6,   80,    4,  114,    6,  129,   31,    5,   81,    4, 
/* 2000 */   115,    5,  129,   30,    5,   82,    4,  116,    5,  129, 
/* 2010 */    30,    4,   82,    4,  116,    5,  129,   30,    4,   82, 
/* 2020 */     5,  117,    4,  131,   30,    5,   82,    5,  117,    4, 
/* 2030 */   129,   31,    5,   81,    6,  117,    4,  129,   31,    6, 
/* 2040 */    80,    7,  117,    4,  129,   32,    7,   79,    8,  117, 
/* 2050 */     4,  129,   32,    9,   77,    9,  116,    5,  129,   33, 
/* 2060 */    11,   75,   11,  116,    4,  129,   34,   16,   69,   16, 
/* 2070 */   115,    5,  129,   35,   49,  114,    5,  129,   37,   46, 
/* 2080 */   113,    5,  129,   38,   44,  112,    6,  129,   40,   41, 
/* 2090 */   112,    5,  129,   42,   37,  113,    3,  129,   44,   33, 
/* 2100 */   114,    1,  129,   47,   27,  129,   51,   17,  129,  193, 
/* 2110 */   129,  103,    2,  129,  103,    6,  129,  104,    9,  129, 
/* 2120 */   105,   12,  129,  106,   15,  129,  107,   14,  135,   30, 
/* 2130 */    10,  107,   14,  129,   30,   17,  107,   14,  129,   30,
/* 2140 */    25,  107,   14,  129,   30,   31,  107,   14,  129,   30, 
/* 2150 */    37,  107,   14,  129,   30,   42,  107,   14,  129,   30,
/* 2160 */    46,  107,   14,  129,   30,   50,  107,   14,  129,   30,
/* 2170 */    54,  107,   14,  129,   30,   58,  107,   14,  129,   59, 
/* 2180 */    32,  107,   14,  129,   64,   30,  107,   14,  129,   74,
/* 2190 */    23,  107,   14,  129,   81,   18,  107,   14,  129,   86, 
/* 2200 */    16,  107,   14,  129,   91,   14,  107,   14,  129,   96,
/* 2210 */    25,  129,  100,   21,  129,  104,   17,  129,  107,   14,
/* 2220 */   129,  111,   10,  129,  114,    7,  129,  117,    4,  129, 
/* 2230 */   120,    1,  129,  193,  129,   48,   13,  129,   44,   21, 
/* 2240 */   129,   42,   26,  129,   40,   30,   92,   12,  129,   38, 
/* 2250 */    34,   88,   20,  129,   36,   37,   86,   25,  129,   35, 
/* 2260 */    39,   84,   29,  129,   34,   13,   63,   12,   82,   33, 
/* 2270 */   129,   33,   11,   67,    9,   80,   36,  129,   32,    9, 
/* 2280 */    70,    7,   79,   38,  129,   31,    8,   72,   46,  129, 
/* 2290 */    30,    7,   74,   22,  108,   11,  129,   30,    6,   75, 
/* 2300 */    19,  111,    9,  129,   30,    5,   75,   17,  113,    7, 
/* 2310 */   129,   30,    5,   74,   16,  114,    6,  129,   30,    4, 
/* 2320 */    73,   16,  115,    6,  129,   30,    4,   72,   16,  116, 
/* 2330 */     5,  129,   30,    4,   72,   15,  117,    4,  129,   30, 
/* 2340 */     4,   71,   16,  117,    4,  129,   30,    5,   70,   16, 
/* 2350 */   117,    4,  129,   30,    5,   70,   15,  117,    4,  129, 
/* 2360 */    30,    6,   69,   15,  116,    5,  129,   30,    7,   68, 
/* 2370 */    17,  115,    5,  129,   30,    9,   67,   19,  114,    6, 
/* 2380 */   129,   30,   10,   65,   22,  113,    6,  129,   31,   12, 
/* 2390 */    63,   27,  110,    9,  129,   32,   14,   60,   21,   84, 
/* 2400 */     9,  106,   12,  129,   33,   47,   85,   32,  129,   34, 
/* 2410 */    45,   86,   30,  129,   35,   43,   88,   26,  129,   36, 
/* 2420 */    40,   90,   22,  129,   38,   36,   93,   17,  129,   40, 
/* 2430 */    32,   96,   10,  129,   42,   28,  129,   44,   23,  129, 
/* 2440 */    48,   15,  129,  193,  129,   83,   17,  129,   77,   27, 
/* 2450 */   129,   36,    1,   74,   33,  129,   35,    3,   72,   37, 
/* 2460 */   129,   34,    5,   70,   41,  129,   33,    6,   69,   44, 
/* 2470 */   129,   33,    5,   68,   46,  129,   32,    5,   67,   49, 
/* 2480 */   129,   31,    5,   66,   17,  101,   16,  129,   31,    5, 
/* 2490 */    66,   11,  108,   10,  129,   30,    4,   65,    9,  110, 
/* 2500 */     9,  129,   30,    4,   64,    8,  112,    7,  129,   30, 
/* 2510 */     4,   64,    7,  114,    6,  129,   30,    4,   64,    6, 
/* 2520 */   115,    5,  129,   30,    4,   64,    5,  116,    5,  129, 
/* 2530 */    30,    4,   64,    5,  117,    4,  131,   30,    4,   65, 
/* 2540 */     4,  117,    4,  129,   30,    5,   65,    4,  116,    5, 
/* 2550 */   129,   31,    5,   66,    4,  115,    5,  129,   31,    6, 
/* 2560 */    67,    4,  114,    6,  129,   32,    7,   68,    4,  112, 
/* 2570 */     7,  129,   32,    9,   69,    5,  110,    9,  129,   33, 
/* 2580 */    11,   70,    5,  107,   11,  129,   34,   16,   72,    5, 
/* 2590 */   101,   16,  129,   35,   81,  129,   37,   77,  129,   38, 
/* 2600 */    75,  129,   40,   71,  129,   42,   67,  129,   44,   63, 
/* 2610 */   129,   47,   57,  129,   51,   49,  129,   56,   39,  129, 
/* 2620 */   193,  130,   34,    6,   74,    6,  129,   32,   10,   72, 
/* 2630 */    10,  129,   31,   12,   71,   12,  129,   30,   14,   70, 
/* 2640 */    14,  131,   31,   12,   71,   12,  129,   32,   10,   72,
/* 2650 */    10,  129,   34,    6,   74,    6,  129,  194,  130,   34, 
/* 2660 */     6,   74,    6,  129,   32,   10,   72,   10,  129,   31, 
/* 2670 */    12,   71,   12,  129,   30,   14,   70,   14,  129,   20, 
/* 2680 */     2,   28,   16,   70,   14,  129,   22,   22,   70,   14, 
/* 2690 */   129,   24,   19,   71,   12,  129,   27,   15,   72,   10,
/* 2700 */   129,   31,    9,   74,    6,  129,  194,  129,   53,    4,
/* 2710 */    63,    4,  152,  193,  130,   99,    7,  129,   97,   13, 
/* 2720 */   129,   96,   16,  129,   96,   18,  129,   96,   19,  129, 
/* 2730 */    97,   19,  129,   99,    6,  110,    7,  129,  112,    6, 
/* 2740 */   129,  114,    5,  129,   34,    6,   57,    5,  115,    4,
/* 2750 */   129,   32,   10,   54,   12,  116,    4,  129,   31,   12,
/* 2760 */    53,   16,  117,    3,  129,   30,   14,   52,   20,  117, 
/* 2770 */     4,  129,   30,   14,   52,   23,  117,    4,  129,   30, 
/* 2780 */    14,   52,   25,  117,    4,  129,   31,   12,   52,   27, 
/* 2790 */   117,    4,  129,   32,   10,   53,   10,   70,   11,  116, 
/* 2800 */     5,  129,   34,    6,   55,    5,   73,   10,  115,    6, 
/* 2810 */   129,   74,   11,  114,    7,  129,   75,   12,  112,    9, 
/* 2820 */   129,   76,   13,  110,   10,  129,   77,   16,  106,   14, 
/* 2830 */   129,   78,   41,  129,   80,   38,  129,   81,   36,  129, 
/* 2840 */    82,   34,  129,   84,   30,  129,   86,   26,  129,   88, 
/* 2850 */    22,  129,   92,   14,  129,  194,  129,   55,   15,  129, 
/* 2860 */    50,   25,  129,   47,   32,  129,   45,   13,   70,   12, 
/* 2870 */   129,   43,    9,   76,   10,  129,   42,    6,   79,    8, 
/* 2880 */   129,   41,    5,   81,    7,  129,   40,    4,   84,    6, 
/* 2890 */   129,   39,    4,   59,   12,   85,    6,  129,   38,    4, 
/* 2900 */    55,   19,   87,    5,  129,   37,    4,   53,   23,   88, 
/* 2910 */     4,  129,   36,    4,   51,    8,   71,    6,   89,    4, 
/* 2920 */   129,   36,    4,   51,    6,   73,    4,   89,    4,  129, 
/* 2930 */    36,    4,   50,    6,   74,    4,   90,    3,  129,   35, 
/* 2940 */     4,   50,    5,   75,    3,   90,    4,  129,   35,    4, 
/* 2950 */    50,    4,   75,    4,   90,    4,  131,   35,    4,   50, 
/* 2960 */     5,   75,    4,   90,    4,  129,   36,    4,   51,    5, 
/* 2970 */    75,    4,   90,    4,  129,   36,    4,   51,    6,   75, 
/* 2980 */     4,   90,    4,  129,   36,    4,   53,   26,   90,    4, 
/* 2990 */   129,   37,    4,   54,   25,   90,    4,  129,   37,    4, 
/* 3000 */    52,   27,   90,    3,  129,   38,    4,   52,    4,   89, 
/* 3010 */     4,  129,   39,    4,   51,    4,   88,    4,  129,   40, 
/* 3020 */     4,   50,    4,   87,    5,  129,   41,    4,   50,    4, 
/* 3030 */    86,    5,  129,   42,    4,   50,    4,   85,    5,  129, 
/* 3040 */    43,    3,   50,    4,   83,    6,  129,   44,    2,   51, 
/* 3050 */     5,   80,    7,  129,   46,    1,   52,    6,   76,    9, 
/* 3060 */   129,   54,   28,  129,   56,   23,  129,   60,   16,  129, 
/* 3070 */   193,  129,   30,    4,  132,   30,    5,  129,   30,    8, 
/* 3080 */   129,   30,   12,  129,   30,   16,  129,   30,    4,   37, 
/* 3090 */    12,  129,   30,    4,   41,   12,  129,   30,    4,   44, 
/* 3100 */    13,  129,   30,    4,   48,   13,  129,   52,   13,  129, 
/* 3110 */    56,   12,  129,   58,   14,  129,   58,    4,   64,   12, 
/* 3120 */   129,   58,    4,   68,   12,  129,   58,    4,   72,   12, 
/* 3130 */   129,   58,    4,   75,   13,  129,   58,    4,   79,   13, 
/* 3140 */   129,   58,    4,   83,   13,  129,   58,    4,   87,   13, 
/* 3150 */   129,   58,    4,   91,   12,  129,   58,    4,   95,   12,
/* 3160 */   129,   58,    4,   96,   15,  129,   58,    4,   93,   22, 
/* 3170 */   129,   58,    4,   89,   30,  129,   58,    4,   85,   36, 
/* 3180 */   129,   58,    4,   81,   38,  129,   58,    4,   77,   38, 
/* 3190 */   129,   58,    4,   73,   38,  129,   58,    4,   70,   37, 
/* 3200 */   129,   58,    4,   66,   37,  129,   58,   41,  129,   58,
/* 3210 */    37,  129,   54,   38,  129,   30,    4,   50,   38,  129, 
/* 3220 */    30,    4,   46,   38,  129,   30,    4,   42,   38,  129, 
/* 3230 */    30,    4,   38,   39,  129,   30,   43,  129,   30,   39,
/* 3240 */   129,   30,   35,  129,   30,   31,  129,   30,   27,  129,
/* 3250 */    30,   24,  129,   30,   20,  129,   30,   16,  129,   30, 
/* 3260 */    12,  129,   30,    8,  129,   30,    5,  129,   30,    4, 
/* 3270 */   132,  193,  129,   30,    4,  117,    4,  132,   30,   91, 
/* 3280 */   137,   30,    4,   80,    4,  117,    4,  138,   30,    4,
/* 3290 */    80,    5,  116,    5,  129,   30,    5,   79,    6,  116,
/* 3300 */     5,  130,   30,    6,   78,    8,  115,    6,  129,   31, 
/* 3310 */     6,   77,    9,  115,    6,  129,   31,    7,   76,   11, 
/* 3320 */   114,    6,  129,   31,    8,   75,   14,  112,    8,  129, 
/* 3330 */    32,    8,   74,   16,  111,    9,  129,   32,    9,   73, 
/* 3340 */    19,  109,   10,  129,   33,   10,   71,   24,  106,   13, 
/* 3350 */   129,   33,   13,   68,   12,   83,   35,  129,   34,   16, 
/* 3360 */    64,   15,   84,   33,  129,   35,   43,   85,   31,  129, 
/* 3370 */    36,   41,   86,   29,  129,   37,   39,   88,   25,  129, 
/* 3380 */    38,   37,   90,   21,  129,   40,   33,   93,   15,  129, 
/* 3390 */    42,   29,   96,    9,  129,   45,   24,  129,   49,   16, 
/* 3400 */   129,  193,  129,   63,   25,  129,   57,   37,  129,   53, 
/* 3410 */    45,  129,   50,   51,  129,   47,   57,  129,   45,   61, 
/* 3420 */   129,   43,   65,  129,   41,   69,  129,   39,   73,  129, 
/* 3430 */    38,   25,   92,   21,  129,   36,   21,   97,   18,  129, 
/* 3440 */    35,   18,  102,   14,  129,   34,   16,  106,   11,  129, 
/* 3450 */    33,   14,  108,   10,  129,   32,   12,  111,    8,  129, 
/* 3460 */    32,   10,  113,    6,  129,   31,   10,  114,    6,  129, 
/* 3470 */    31,    8,  115,    5,  129,   30,    8,  116,    5,  129, 
/* 3480 */    30,    7,  116,    5,  129,   30,    6,  117,    4,  130, 
/* 3490 */    30,    5,  117,    4,  131,   31,    4,  116,    5,  129, 
/* 3500 */    32,    4,  116,    4,  129,   32,    5,  115,    5,  129, 
/* 3510 */    33,    4,  114,    5,  129,   34,    4,  112,    6,  129, 
/* 3520 */    35,    4,  110,    7,  129,   37,    4,  107,    9,  129, 
/* 3530 */    39,    4,  103,   12,  129,   41,    4,  103,   18,  129, 
/* 3540 */    43,    4,  103,   18,  129,   45,    5,  103,   18,  129, 
/* 3550 */    48,    5,  103,   18,  129,   51,    1,  129,  193,  129, 
/* 3560 */    30,    4,  117,    4,  132,   30,   91,  137,   30,    4, 
/* 3570 */   117,    4,  135,   30,    5,  116,    5,  130,   30,    6, 
/* 3580 */   115,    6,  130,   31,    6,  114,    6,  129,   31,    7, 
/* 3590 */   113,    7,  129,   32,    7,  112,    7,  129,   32,    8, 
/* 3600 */   111,    8,  129,   33,    9,  109,    9,  129,   33,   12, 
/* 3610 */   106,   12,  129,   34,   13,  104,   13,  129,   35,   15, 
/* 3620 */   101,   15,  129,   36,   19,   96,   19,  129,   37,   24, 
/* 3630 */    90,   24,  129,   39,   73,  129,   40,   71,  129,   42, 
/* 3640 */    67,  129,   44,   63,  129,   46,   59,  129,   49,   53, 
/* 3650 */   129,   52,   47,  129,   56,   39,  129,   61,   29,  129, 
/* 3660 */   193,  129,   30,    4,  117,    4,  132,   30,   91,  137,
/* 3670 */    30,    4,   80,    4,  117,    4,  140,   30,    4,   79, 
/* 3680 */     6,  117,    4,  129,   30,    4,   77,   10,  117,    4, 
/* 3690 */   129,   30,    4,   73,   18,  117,    4,  132,   30,    4, 
/* 3700 */   117,    4,  130,   30,    5,  116,    5,  130,   30,    7, 
/* 3710 */   114,    7,  129,   30,    8,  113,    8,  129,   30,   11,
/* 3720 */   110,   11,  129,   30,   18,  103,   18,  132,  193,  129, 
/* 3730 */    30,    4,  117,    4,  132,   30,   91,  137,   30,    4, 
/* 3740 */    80,    4,  117,    4,  132,   80,    4,  117,    4,  136, 
/* 3750 */    79,    6,  117,    4,  129,   77,   10,  117,    4,  129, 
/* 3760 */    73,   18,  117,    4,  132,  117,    4,  130,  116,    5, 
/* 3770 */   130,  114,    7,  129,  113,    8,  129,  110,   11,  129,
/* 3780 */   103,   18,  132,  193,  129,   63,   25,  129,   57,   37,
/* 3790 */   129,   53,   45,  129,   50,   51,  129,   47,   57,  129, 
/* 3800 */    45,   61,  129,   43,   65,  129,   41,   69,  129,   39, 
/* 3810 */    73,  129,   38,   25,   92,   21,  129,   36,   21,   97, 
/* 3820 */    18,  129,   35,   18,  102,   14,  129,   34,   16,  106,
/* 3830 */    11,  129,   33,   14,  108,   10,  129,   32,   12,  111,
/* 3840 */     8,  129,   32,   10,  113,    6,  129,   31,   10,  114, 
/* 3850 */     6,  129,   31,    8,  115,    5,  129,   30,    8,  116, 
/* 3860 */     5,  129,   30,    7,  116,    5,  129,   30,    6,  117, 
/* 3870 */     4,  130,   30,    5,  117,    4,  131,   30,    5,   75, 
/* 3880 */     4,  116,    5,  129,   31,    5,   75,    4,  116,    4, 
/* 3890 */   129,   31,    6,   75,    4,  115,    5,  129,   32,    7, 
/* 3900 */    75,    4,  114,    5,  129,   32,    9,   75,    4,  112, 
/* 3910 */     6,  129,   33,   11,   75,    4,  110,    7,  129,   34, 
/* 3920 */    15,   75,    4,  107,    9,  129,   35,   44,  103,   12, 
/* 3930 */   129,   36,   43,  103,   18,  129,   38,   41,  103,   18, 
/* 3940 */   129,   39,   40,  103,   18,  129,   41,   38,  103,   18, 
/* 3950 */   129,   44,   35,  129,   48,   31,  129,   52,   27,  129, 
/* 3960 */    61,   18,  129,  193,  129,   30,    4,  117,    4,  132, 
/* 3970 */    30,   91,  137,   30,    4,   80,    4,  117,    4,  132, 
/* 3980 */    80,    4,  140,   30,    4,   80,    4,  117,    4,  132, 
/* 3990 */    30,   91,  137,   30,    4,  117,    4,  132,  193,  129, 
/* 4000 */    30,    4,  117,    4,  132,   30,   91,  137,   30,    4, 
/* 4010 */   117,    4,  132,  193,  129,   44,    7,  129,   40,   13, 
/* 4020 */   129,   37,   17,  129,   35,   20,  129,   34,   22,  129, 
/* 4030 */    33,   23,  129,   32,   24,  129,   32,   23,  129,   31, 
/* 4040 */     6,   41,   13,  129,   31,    5,   42,   11,  129,   30, 
/* 4050 */     5,   44,    7,  129,   30,    4,  132,   30,    5,  130, 
/* 4060 */    31,    5,  129,   31,    6,  117,    4,  129,   31,    8, 
/* 4070 */   117,    4,  129,   32,    9,  117,    4,  129,   33,   11, 
/* 4080 */   117,    4,  129,   34,   87,  129,   35,   86,  129,   36, 
/* 4090 */    85,  129,   37,   84,  129,   38,   83,  129,   40,   81, 
/* 4100 */   129,   42,   79,  129,   45,   76,  129,   50,   71,  129, 
/* 4110 */   117,    4,  132,  193,  129,   30,    4,  117,    4,  132, 
/* 4120 */    30,   91,  137,   30,    4,   76,    8,  117,    4,  129, 
/* 4130 */    30,    4,   73,   13,  117,    4,  129,   30,    4,   70, 
/* 4140 */    18,  117,    4,  129,   30,    4,   67,   23,  117,    4, 
/* 4150 */   129,   65,   26,  129,   62,   31,  129,   59,   35,  129, 
/* 4160 */    56,   29,   89,    7,  129,   53,   29,   91,    7,  129, 
/* 4170 */    50,   29,   93,    7,  129,   47,   29,   95,    6,  129,
/* 4180 */    30,    4,   45,   29,   96,    7,  129,   30,    4,   42, 
/* 4190 */    29,   98,    7,  129,   30,    4,   39,   30,  100,    6, 
/* 4200 */   129,   30,    4,   36,   30,  101,    7,  129,   30,   33, 
/* 4210 */   103,    7,  117,    4,  129,   30,   30,  105,    6,  117, 
/* 4220 */     4,  129,   30,   27,  106,    7,  117,    4,  129,   30,
/* 4230 */    25,  108,    7,  117,    4,  129,   30,   22,  110,   11, 
/* 4240 */   129,   30,   19,  111,   10,  129,   30,   16,  113,    8, 
/* 4250 */   129,   30,   13,  115,    6,  129,   30,   11,  116,    5, 
/* 4260 */   129,   30,    8,  117,    4,  129,   30,    5,  117,    4, 
/* 4270 */   129,   30,    4,  117,    4,  130,   30,    4,  130,  193, 
/* 4280 */   129,   30,    4,  117,    4,  132,   30,   91,  137,   30, 
/* 4290 */     4,  117,    4,  132,   30,    4,  144,   30,    5,  130, 
/* 4300 */    30,    7,  129,   30,    8,  129,   30,   11,  129,   30, 
/* 4310 */    18,  132,  193,  129,   30,    4,  117,    4,  132,   30,
/* 4320 */    91,  132,   30,    4,  103,   18,  129,   30,    4,   97,
/* 4330 */    24,  129,   30,    4,   92,   29,  129,   30,    4,   87, 
/* 4340 */    34,  129,   81,   40,  129,   76,   45,  129,   70,   49, 
/* 4350 */   129,   65,   49,  129,   60,   49,  129,   55,   49,  129, 
/* 4360 */    50,   48,  129,   44,   49,  129,   39,   48,  129,   33,
/* 4370 */    49,  129,   30,   47,  129,   34,   37,  129,   40,   26,
/* 4380 */   129,   46,   19,  129,   52,   19,  129,   58,   19,  129, 
/* 4390 */    64,   19,  129,   70,   19,  129,   76,   19,  129,   82, 
/* 4400 */    19,  129,   30,    4,   88,   18,  129,   30,    4,   94, 
/* 4410 */    18,  129,   30,    4,  100,   18,  129,   30,    4,  106, 
/* 4420 */    15,  129,   30,   91,  137,   30,    4,  117,    4,  132, 
/* 4430 */   193,  129,   30,    4,  117,    4,  132,   30,   91,  132, 
/* 4440 */    30,    4,  107,   14,  129,   30,    4,  104,   17,  129, 
/* 4450 */    30,    4,  101,   20,  129,   30,    4,   99,   22,  129, 
/* 4460 */    96,   25,  129,   93,   28,  129,   91,   28,  129,   88, 
/* 4470 */    29,  129,   85,   29,  129,   82,   29,  129,   79,   29, 
/* 4480 */   129,   76,   29,  129,   74,   29,  129,   71,   29,  129, 
/* 4490 */    68,   29,  129,   65,   29,  129,   62,   29,  129,   60, 
/* 4500 */    29,  129,   57,   29,  129,   54,   29,  129,   51,   29, 
/* 4510 */   129,   49,   28,  129,   46,   29,  129,   43,   29,  129, 
/* 4520 */    40,   29,  117,    4,  129,   37,   29,  117,    4,  129, 
/* 4530 */    35,   29,  117,    4,  129,   32,   29,  117,    4,  129, 
/* 4540 */    30,   91,  132,  117,    4,  132,  193,  129,   63,   25, 
/* 4550 */   129,   57,   37,  129,   53,   45,  129,   50,   51,  129, 
/* 4560 */    47,   57,  129,   45,   61,  129,   43,   65,  129,   41, 
/* 4570 */    69,  129,   39,   73,  129,   38,   21,   92,   21,  129, 
/* 4580 */    36,   18,   97,   18,  129,   35,   14,  102,   14,  129, 
/* 4590 */    34,   11,  106,   11,  129,   33,   10,  108,   10,  129, 
/* 4600 */    32,    8,  111,    8,  129,   32,    6,  113,    6,  129, 
/* 4610 */    31,    6,  114,    6,  129,   31,    5,  115,    5,  129, 
/* 4620 */    30,    5,  116,    5,  130,   30,    4,  117,    4,  132, 
/* 4630 */    30,    5,  116,    5,  130,   31,    5,  115,    5,  129, 
/* 4640 */    31,    6,  114,    6,  129,   32,    6,  113,    6,  129, 
/* 4650 */    32,    8,  111,    8,  129,   33,   10,  108,   10,  129, 
/* 4660 */    34,   11,  106,   11,  129,   35,   14,  102,   14,  129, 
/* 4670 */    36,   18,   97,   18,  129,   38,   21,   92,   21,  129, 
/* 4680 */    39,   73,  129,   41,   69,  129,   43,   65,  129,   45,
/* 4690 */    61,  129,   47,   57,  129,   50,   51,  129,   53,   45, 
/* 4700 */   129,   57,   37,  129,   63,   25,  129,  193,  129,   30, 
/* 4710 */     4,  117,    4,  132,   30,   91,  137,   30,    4,   80, 
/* 4720 */     4,  117,    4,  132,   80,    4,  117,    4,  134,   80, 
/* 4730 */     5,  116,    5,  131,   80,    6,  115,    6,  130,   81,
/* 4740 */     6,  114,    6,  129,   81,    8,  112,    8,  129,   81, 
/* 4750 */     9,  111,    9,  129,   82,   10,  109,   10,  129,   82, 
/* 4760 */    13,  106,   13,  129,   83,   35,  129,   84,   33,  129, 
/* 4770 */    85,   31,  129,   86,   29,  129,   88,   25,  129,   90, 
/* 4780 */    21,  129,   93,   15,  129,   96,    9,  129,  193,  129, 
/* 4790 */    63,   25,  129,   57,   37,  129,   53,   45,  129,   50, 
/* 4800 */    51,  129,   47,   57,  129,   45,   61,  129,   43,   65, 
/* 4810 */   129,   41,   69,  129,   39,   73,  129,   38,   21,   92, 
/* 4820 */    21,  129,   36,   18,   97,   18,  129,   35,   14,  102, 
/* 4830 */    14,  129,   34,   11,  106,   11,  129,   33,   10,  108, 
/* 4840 */    10,  129,   32,    8,  111,    8,  129,   32,    6,  113, 
/* 4850 */     6,  129,   31,    6,  114,    6,  129,   31,    5,  115,
/* 4860 */     5,  129,   30,    5,  116,    5,  130,   30,    4,   39,
/* 4870 */     2,  117,    4,  129,   30,    4,   40,    4,  117,    4, 
/* 4880 */   129,   30,    4,   41,    5,  117,    4,  129,   30,    4, 
/* 4890 */    41,    6,  117,    4,  129,   30,    5,   40,    8,  116, 
/* 4900 */     5,  129,   30,    5,   39,   10,  116,    5,  129,   31,
/* 4910 */     5,   38,   11,  115,    5,  129,   31,   18,  114,    6,
/* 4920 */   129,   32,   17,  113,    6,  129,   32,   16,  111,    8, 
/* 4930 */   129,   33,   15,  108,   10,  129,   33,   14,  106,   11, 
/* 4940 */   129,   32,   17,  102,   14,  129,   31,   23,   97,   18, 
/* 4950 */   129,   31,   28,   92,   21,  129,   30,   82,  129,   30, 
/* 4960 */    80,  129,   30,   11,   43,   65,  129,   30,   10,   45, 
/* 4970 */    61,  129,   31,    8,   47,   57,  129,   32,    6,   50, 
/* 4980 */    51,  129,   33,    5,   53,   45,  129,   35,    4,   57, 
/* 4990 */    37,  129,   38,    2,   63,   25,  129,  193,  129,   30, 
/* 5000 */     4,  117,    4,  132,   30,   91,  137,   30,    4,   76, 
/* 5010 */     8,  117,    4,  129,   30,    4,   73,   11,  117,    4, 
/* 5020 */   129,   30,    4,   70,   14,  117,    4,  129,   30,    4, 
/* 5030 */    67,   17,  117,    4,  129,   65,   19,  117,    4,  129, 
/* 5040 */    62,   22,  117,    4,  129,   59,   25,  117,    4,  129, 
/* 5050 */    56,   28,  117,    4,  129,   53,   31,  117,    4,  129, 
/* 5060 */    50,   34,  117,    4,  129,   47,   29,   80,    5,  116, 
/* 5070 */     5,  129,   30,    4,   45,   29,   80,    5,  116,    5, 
/* 5080 */   129,   30,    4,   42,   29,   80,    5,  116,    5,  129, 
/* 5090 */    30,    4,   39,   30,   80,    6,  115,    6,  129,   30, 
/* 5100 */     4,   36,   30,   80,    6,  115,    6,  129,   30,   33, 
/* 5110 */    81,    6,  114,    6,  129,   30,   30,   81,    8,  112, 
/* 5120 */     8,  129,   30,   27,   81,    9,  111,    9,  129,   30, 
/* 5130 */    25,   82,   10,  109,   10,  129,   30,   22,   82,   13, 
/* 5140 */   106,   13,  129,   30,   19,   83,   35,  129,   30,   16, 
/* 5150 */    84,   33,  129,   30,   13,   85,   31,  129,   30,   11, 
/* 5160 */    86,   29,  129,   30,    8,   88,   25,  129,   30,    5, 
/* 5170 */    90,   21,  129,   30,    4,   93,   15,  129,   30,    4, 
/* 5180 */    96,    9,  129,   30,    4,  130,  193,  129,   30,   18, 
/* 5190 */   130,   30,   18,   89,   15,  129,   30,   18,   85,   23,
/* 5200 */   129,   34,   11,   83,   27,  129,   34,    9,   81,   31, 
/* 5210 */   129,   33,    8,   79,   35,  129,   33,    6,   78,   16, 
/* 5220 */   106,    9,  129,   32,    6,   77,   15,  109,    7,  129, 
/* 5230 */    32,    5,   76,   14,  111,    6,  129,   31,    5,   75, 
/* 5240 */    14,  113,    5,  129,   31,    4,   74,   15,  114,    5,
/* 5250 */   129,   31,    4,   74,   14,  115,    4,  129,   30,    4, 
/* 5260 */    73,   15,  116,    4,  129,   30,    4,   73,   14,  116, 
/* 5270 */     4,  129,   30,    4,   73,   14,  117,    4,  129,   30, 
/* 5280 */     4,   72,   15,  117,    4,  130,   30,    4,   71,   15, 
/* 5290 */   117,    4,  130,   30,    4,   70,   15,  117,    4,  129, 
/* 5300 */    30,    5,   70,   15,  117,    4,  129,   30,    5,   69, 
/* 5310 */    15,  116,    5,  129,   30,    6,   68,   16,  115,    5, 
/* 5320 */   129,   31,    6,   67,   16,  114,    6,  129,   31,    7, 
/* 5330 */    66,   17,  113,    6,  129,   32,    7,   64,   18,  111, 
/* 5340 */     8,  129,   32,    8,   62,   19,  109,    9,  129,   33, 
/* 5350 */     9,   60,   20,  107,   10,  129,   34,   11,   57,   22, 
/* 5360 */   103,   13,  129,   35,   43,  103,   18,  129,   36,   41, 
/* 5370 */   103,   18,  129,   38,   38,  103,   18,  129,   39,   35, 
/* 5380 */   103,   18,  129,   41,   31,  129,   43,   27,  129,   46, 
/* 5390 */    22,  129,   49,   14,  129,  193,  129,  103,   18,  132,
/* 5400 */   110,   11,  129,  113,    8,  129,  114,    7,  129,  116,
/* 5410 */     5,  130,  117,    4,  132,   30,    4,  117,    4,  132, 
/* 5420 */    30,   91,  137,   30,    4,  117,    4,  132,  117,    4, 
/* 5430 */   132,  116,    5,  130,  114,    7,  129,  113,    8,  129, 
/* 5440 */   110,   11,  129,  103,   18,  132,  193,  129,  117,    4,
/* 5450 */   132,   56,   65,  129,   50,   71,  129,   46,   75,  129,
/* 5460 */    44,   77,  129,   42,   79,  129,   40,   81,  129,   38, 
/* 5470 */    83,  129,   36,   85,  129,   35,   86,  129,   34,   20, 
/* 5480 */   117,    4,  129,   33,   17,  117,    4,  129,   32,   15, 
/* 5490 */   117,    4,  129,   32,   13,  117,    4,  129,   31,   12, 
/* 5500 */   129,   31,   10,  129,   31,    9,  129,   30,    9,  129, 
/* 5510 */    30,    8,  130,   30,    7,  132,   31,    6,  130,   31, 
/* 5520 */     7,  129,   32,    6,  129,   32,    7,  129,   33,    7, 
/* 5530 */   129,   34,    7,  129,   35,    8,  129,   36,    9,  117, 
/* 5540 */     4,  129,   38,    9,  117,    4,  129,   40,   10,  117, 
/* 5550 */     4,  129,   42,   12,  117,    4,  129,   44,   77,  129, 
/* 5560 */    46,   75,  129,   50,   71,  129,   56,   43,  100,   21, 
/* 5570 */   129,  117,    4,  132,  193,  129,  117,    4,  132,  115, 
/* 5580 */     6,  129,  110,   11,  129,  105,   16,  129,  101,   20, 
/* 5590 */   129,   96,   25,  129,   92,   29,  129,   87,   34,  129, 
/* 5600 */    83,   38,  129,   78,   43,  129,   74,   47,  129,   70, 
/* 5610 */    42,  117,    4,  129,   65,   42,  117,    4,  129,   60, 
/* 5620 */    43,  117,    4,  129,   56,   42,  129,   51,   42,  129, 
/* 5630 */    46,   43,  129,   42,   43,  129,   37,   44,  129,   33, 
/* 5640 */    43,  129,   30,   42,  129,   33,   34,  129,   38,   25, 
/* 5650 */   129,   42,   16,  129,   47,   15,  129,   52,   15,  129, 
/* 5660 */    57,   15,  129,   61,   16,  129,   66,   16,  129,   71, 
/* 5670 */    16,  129,   76,   16,  129,   80,   16,  129,   85,   16, 
/* 5680 */   117,    4,  129,   90,   16,  117,    4,  129,   95,   16, 
/* 5690 */   117,    4,  129,  100,   21,  129,  105,   16,  129,  110, 
/* 5700 */    11,  129,  114,    7,  129,  117,    4,  132,  193,  129,
/* 5710 */   117,    4,  132,  115,    6,  129,  110,   11,  129,  105, 
/* 5720 */    16,  129,  101,   20,  129,   96,   25,  129,   92,   29, 
/* 5730 */   129,   87,   34,  129,   83,   38,  129,   78,   43,  129, 
/* 5740 */    74,   47,  129,   70,   42,  117,    4,  129,   65,   42, 
/* 5750 */   117,    4,  129,   60,   43,  117,    4,  129,   56,   42,
/* 5760 */   129,   51,   42,  129,   46,   43,  129,   42,   43,  129, 
/* 5770 */    37,   44,  129,   33,   43,  129,   30,   42,  129,   33, 
/* 5780 */    34,  129,   38,   25,  129,   42,   16,  129,   47,   15, 
/* 5790 */   129,   52,   15,  129,   57,   15,  129,   61,   16,  129, 
/* 5800 */    65,   17,  129,   60,   27,  129,   56,   36,  129,   51, 
/* 5810 */    42,  129,   46,   43,  129,   42,   43,  129,   37,   44, 
/* 5820 */   129,   33,   43,  129,   30,   42,  129,   33,   34,  129, 
/* 5830 */    38,   25,  129,   42,   16,  129,   47,   15,  129,   52, 
/* 5840 */    15,  129,   57,   15,  129,   61,   16,  129,   66,   16, 
/* 5850 */   129,   71,   16,  129,   76,   16,  129,   80,   16,  129, 
/* 5860 */    85,   16,  117,    4,  129,   90,   16,  117,    4,  129, 
/* 5870 */    95,   16,  117,    4,  129,  100,   21,  129,  105,   16, 
/* 5880 */   129,  110,   11,  129,  114,    7,  129,  117,    4,  132, 
/* 5890 */   193,  129,   30,    4,  117,    4,  132,   30,    4,  115, 
/* 5900 */     6,  129,   30,    4,  112,    9,  129,   30,    6,  109, 
/* 5910 */    12,  129,   30,    9,  106,   15,  129,   30,   11,  103, 
/* 5920 */    18,  129,   30,   14,  100,   21,  129,   30,    4,   38, 
/* 5930 */     9,   98,   23,  129,   30,    4,   40,   10,   95,   26,
/* 5940 */   129,   30,    4,   43,    9,   92,   29,  129,   46,    9,
/* 5950 */    89,   32,  129,   49,    8,   86,   28,  117,    4,  129, 
/* 5960 */    51,    9,   83,   28,  117,    4,  129,   54,    9,   80, 
/* 5970 */    28,  117,    4,  129,   57,    8,   77,   28,  117,    4, 
/* 5980 */   129,   59,    9,   74,   28,  129,   62,   37,  129,   64,
/* 5990 */    33,  129,   66,   28,  129,   63,   28,  129,   60,   28,
/* 6000 */   129,   57,   28,  129,   54,   33,  129,   51,   39,  129, 
/* 6010 */    48,   29,   83,    9,  129,   30,    4,   45,   29,   86, 
/* 6020 */     9,  129,   30,    4,   42,   29,   89,    9,  129,   30, 
/* 6030 */     4,   39,   29,   92,    8,  129,   30,    4,   36,   29, 
/* 6040 */    94,    9,  129,   30,   32,   97,    9,  129,   30,   29, 
/* 6050 */   100,    8,  117,    4,  129,   30,   26,  103,    8,  117, 
/* 6060 */     4,  129,   30,   23,  105,    9,  117,    4,  129,   30, 
/* 6070 */    20,  108,   13,  129,   30,   18,  111,   10,  129,   30, 
/* 6080 */    15,  113,    8,  129,   30,   12,  116,    5,  129,   30, 
/* 6090 */     9,  117,    4,  129,   30,    6,  117,    4,  129,   30, 
/* 6100 */     4,  117,    4,  132,  193,  129,  117,    4,  132,  114, 
/* 6110 */     7,  129,  111,   10,  129,  108,   13,  129,  105,   16, 
/* 6120 */   129,  102,   19,  129,  100,   21,  129,   96,   25,  129, 
/* 6130 */    93,   28,  129,   90,   31,  129,   87,   34,  129,   84, 
/* 6140 */    30,  117,    4,  129,   30,    4,   81,   30,  117,    4, 
/* 6150 */   129,   30,    4,   78,   30,  117,    4,  129,   30,    4, 
/* 6160 */    75,   30,  117,    4,  129,   30,    4,   72,   30,  129, 
/* 6170 */    30,   69,  129,   30,   66,  129,   30,   63,  129,   30, 
/* 6180 */    60,  129,   30,   57,  129,   30,   54,  129,   30,   51, 
/* 6190 */   129,   30,   48,  129,   30,   51,  129,   30,    4,   73, 
/* 6200 */    12,  129,   30,    4,   76,   12,  129,   30,    4,   80, 
/* 6210 */    12,  129,   30,    4,   83,   12,  129,   87,   12,  129,
/* 6220 */    90,   12,  117,    4,  129,   94,   11,  117,    4,  129, 
/* 6230 */    97,   12,  117,    4,  129,  101,   12,  117,    4,  129, 
/* 6240 */   104,   17,  129,  108,   13,  129,  111,   10,  129,  115, 
/* 6250 */     6,  129,  117,    4,  134,  193,  129,   30,    1,  103, 
/* 6260 */    18,  129,   30,    4,  103,   18,  129,   30,    7,  103,
/* 6270 */    18,  129,   30,    9,  103,   18,  129,   30,   12,  110, 
/* 6280 */    11,  129,   30,   15,  113,    8,  129,   30,   18,  114, 
/* 6290 */     7,  129,   30,   21,  116,    5,  129,   30,   24,  116, 
/* 6300 */     5,  129,   30,   27,  117,    4,  129,   30,   30,  117, 
/* 6310 */     4,  129,   30,   33,  117,    4,  129,   30,    4,   37, 
/* 6320 */    28,  117,    4,  129,   30,    4,   40,   28,  117,    4, 
/* 6330 */   129,   30,    4,   42,   29,  117,    4,  129,   30,    4, 
/* 6340 */    45,   29,  117,    4,  129,   30,    4,   48,   29,  117, 
/* 6350 */     4,  129,   30,    4,   51,   29,  117,    4,  129,   30, 
/* 6360 */     4,   54,   29,  117,    4,  129,   30,    4,   57,   29, 
/* 6370 */   117,    4,  129,   30,    4,   59,   30,  117,    4,  129, 
/* 6380 */    30,    4,   62,   30,  117,    4,  129,   30,    4,   65, 
/* 6390 */    30,  117,    4,  129,   30,    4,   68,   30,  117,    4, 
/* 6400 */   129,   30,    4,   71,   30,  117,    4,  129,   30,    4, 
/* 6410 */    74,   30,  117,    4,  129,   30,    4,   77,   30,  117, 
/* 6420 */     4,  129,   30,    4,   80,   30,  117,    4,  129,   30, 
/* 6430 */     4,   83,   30,  117,    4,  129,   30,    4,   86,   35, 
/* 6440 */   129,   30,    4,   89,   32,  129,   30,    4,   91,   30, 
/* 6450 */   129,   30,    4,   94,   27,  129,   30,    5,   97,   24, 
/* 6460 */   129,   30,    5,  100,   21,  129,   30,    7,  103,   18, 
/* 6470 */   129,   30,    8,  106,   15,  129,   30,   11,  109,   12,
/* 6480 */   129,   30,   18,  112,    9,  129,   30,   18,  115,    6,
/* 6490 */   129,   30,   18,  117,    4,  129,   30,   18,  120,    1, 
/* 6500 */   129,  193,  129,   42,    8,  129,   38,   16,  129,   36, 
/* 6510 */    20,  129,   34,   24,   71,    5,  129,   33,   26,   69, 
/* 6520 */    10,  129,   32,   28,   68,   13,  129,   31,   30,   68,
/* 6530 */    14,  129,   31,    9,   52,    9,   68,   15,  129,   30,
/* 6540 */     8,   54,    8,   69,   14,  129,   30,    7,   55,    7, 
/* 6550 */    71,    4,   78,    6,  129,   30,    6,   56,    6,   79, 
/* 6560 */     5,  129,   30,    6,   56,    6,   80,    4,  130,   31, 
/* 6570 */     5,   56,    5,   80,    4,  129,   31,    5,   56,    5, 
/* 6580 */    79,    5,  129,   32,    5,   55,    5,   78,    6,  129, 
/* 6590 */    33,    5,   54,    5,   77,    7,  129,   34,    6,   52, 
/* 6600 */     6,   74,    9,  129,   35,   48,  129,   33,   49,  129, 
/* 6610 */    32,   49,  129,   31,   49,  129,   30,   49,  129,   30, 
/* 6620 */    47,  129,   30,   45,  129,   30,   41,  129,   30,    6, 
/* 6630 */   129,   30,    4,  129,   30,    3,  129,   30,    2,  129, 
/* 6640 */   193,  129,   30,    4,  117,    4,  130,   31,   90,  136, 
/* 6650 */    37,    5,   72,    5,  129,   35,    5,   74,    5,  129, 
/* 6660 */    33,    5,   76,    5,  129,   32,    5,   77,    5,  129, 
/* 6670 */    31,    5,   78,    5,  129,   31,    4,   79,    4,  129, 
/* 6680 */    30,    5,   79,    5,  131,   30,    6,   78,    6,  129, 
/* 6690 */    30,    7,   77,    7,  129,   31,    8,   75,    8,  129, 
/* 6700 */    31,   11,   72,   11,  129,   32,   15,   67,   15,  129, 
/* 6710 */    33,   48,  129,   34,   46,  129,   35,   44,  129,   37, 
/* 6720 */    40,  129,   39,   36,  129,   42,   30,  129,   46,   22,
/* 6730 */   129,  193,  129,   48,   18,  129,   43,   28,  129,   41, 
/* 6740 */    32,  129,   39,   36,  129,   37,   40,  129,   35,   44, 
/* 6750 */   129,   34,   46,  129,   33,   13,   68,   13,  129,   32, 
/* 6760 */     9,   73,    9,  129,   32,    7,   75,    7,  129,   31, 
/* 6770 */     6,   77,    6,  129,   31,    5,   78,    5,  129,   30,
/* 6780 */     5,   79,    5,  129,   30,    4,   80,    4,  133,   31, 
/* 6790 */     3,   79,    4,  129,   31,    4,   79,    4,  129,   32, 
/* 6800 */     3,   78,    4,  129,   32,    4,   76,    6,  129,   33, 
/* 6810 */     4,   74,    7,  129,   34,    4,   72,    8,  129,   35, 
/* 6820 */     5,   72,    7,  129,   37,    5,   73,    4,  129,   39, 
/* 6830 */     4,   74,    1,  129,  129,  193,  129,   46,   22,  129, 
/* 6840 */    42,   30,  129,   39,   36,  129,   37,   40,  129,   35, 
/* 6850 */    44,  129,   34,   46,  129,   33,   48,  129,   32,   15, 
/* 6860 */    67,   15,  129,   31,   11,   72,   11,  129,   31,    8, 
/* 6870 */    75,    8,  129,   30,    7,   77,    7,  129,   30,    6, 
/* 6880 */    78,    6,  129,   30,    5,   79,    5,  131,   31,    4, 
/* 6890 */    79,    4,  129,   31,    5,   78,    5,  129,   32,    5, 
/* 6900 */    77,    5,  129,   33,    5,   76,    5,  129,   35,    5, 
/* 6910 */    74,    5,  117,    4,  129,   37,    5,   72,    5,  117, 
/* 6920 */     4,  129,   30,   91,  136,   30,    4,  130,  193,  129, 
/* 6930 */    48,   18,  129,   43,   28,  129,   41,   32,  129,   39, 
/* 6940 */    36,  129,   37,   40,  129,   35,   44,  129,   34,   46, 
/* 6950 */   129,   33,   13,   55,    4,   68,   13,  129,   32,    9, 
/* 6960 */    55,    4,   73,    9,  129,   32,    7,   55,    4,   75, 
/* 6970 */     7,  129,   31,    6,   55,    4,   77,    6,  129,   31, 
/* 6980 */     5,   55,    4,   78,    5,  129,   30,    5,   55,    4, 
/* 6990 */    79,    5,  129,   30,    4,   55,    4,   80,    4,  132, 
/* 7000 */    30,    4,   55,    4,   79,    5,  129,   31,    3,   55, 
/* 7010 */     4,   78,    5,  129,   31,    4,   55,    4,   77,    6,
/* 7020 */   129,   32,    3,   55,    4,   75,    7,  129,   32,    4,
/* 7030 */    55,    4,   73,    9,  129,   33,    4,   55,    4,   68, 
/* 7040 */    13,  129,   34,    4,   55,   25,  129,   35,    5,   55, 
/* 7050 */    24,  129,   37,    5,   55,   22,  129,   39,    4,   55, 
/* 7060 */    20,  129,   55,   18,  129,   55,   16,  129,   55,   11,
/* 7070 */   129,  193,  129,   80,    4,  129,   30,    4,   80,    4,
/* 7080 */   130,   30,   78,  129,   30,   82,  129,   30,   85,  129, 
/* 7090 */    30,   87,  129,   30,   88,  129,   30,   89,  129,   30, 
/* 7100 */    90,  130,   30,    4,   80,    4,  115,    6,  129,   30, 
/* 7110 */     4,   80,    4,  117,    4,  129,   80,    4,  105,    6, 
/* 7120 */   117,    4,  129,   80,    4,  103,   10,  116,    5,  129, 
/* 7130 */    80,    4,  102,   19,  129,   80,    4,  101,   19,  129, 
/* 7140 */   101,   19,  129,  101,   18,  129,  102,   16,  129,  103, 
/* 7150 */    12,  129,  105,    6,  129,  193,  129,   12,   10,   59, 
/* 7160 */    11,  129,    9,   16,   55,   19,  129,    7,   20,   53, 
/* 7170 */    23,  129,    6,    7,   23,    5,   32,    6,   51,   27, 
/* 7180 */   129,    4,    7,   25,   16,   50,   29,  129,    3,    6, 
/* 7190 */    27,   16,   49,   31,  129,    2,    6,   28,   16,   48, 
/* 7200 */    33,  129,    1,    6,   27,   18,   47,   35,  129,    1, 
/* 7210 */     6,   27,   31,   71,   12,  129,    1,    5,   26,   15, 
/* 7220 */    44,   10,   75,    8,  129,    1,    5,   25,   14,   45, 
/* 7230 */     7,   77,    7,  129,    1,    5,   25,   13,   45,    5,
/* 7240 */    79,    5,  129,    1,    5,   24,   14,   45,    4,   80, 
/* 7250 */     4,  129,    1,    5,   24,   13,   45,    4,   80,    4, 
/* 7260 */   129,    1,    5,   23,   14,   45,    4,   80,    4,  129, 
/* 7270 */     1,    5,   23,   13,   45,    4,   80,    4,  129,    1, 
/* 7280 */     6,   22,   13,   45,    5,   79,    5,  129,    1,    6,
/* 7290 */    21,   14,   45,    7,   77,    7,  129,    1,    7,   21, 
/* 7300 */    13,   46,    8,   75,    8,  129,    1,    8,   20,   13, 
/* 7310 */    46,   12,   71,   12,  129,    1,   10,   18,   15,   47, 
/* 7320 */    35,  129,    2,   30,   48,   33,  129,    3,   29,   49, 
/* 7330 */    32,  129,    4,   27,   50,   31,  129,    5,   25,   51, 
/* 7340 */    27,   80,    2,   86,    4,  129,    7,   21,   53,   23, 
/* 7350 */    80,    3,   85,    6,  129,    9,   17,   55,   19,   80, 
/* 7360 */    12,  129,   12,   12,   59,   11,   81,   11,  129,   82, 
/* 7370 */    10,  129,   84,    7,  129,   86,    4,  129,  193,  129, 
/* 7380 */    30,    4,  117,    4,  130,   30,   91,  136,   30,    4, 
/* 7390 */    72,    5,  129,   30,    4,   74,    5,  129,   75,    5, 
/* 7400 */   129,   76,    5,  129,   76,    6,  129,   77,    6,  130, 
/* 7410 */    77,    7,  130,   76,    8,  129,   30,    4,   75,    9, 
/* 7420 */   129,   30,    4,   72,   12,  129,   30,   54,  129,   30, 
/* 7430 */    53,  130,   30,   52,  129,   30,   51,  129,   30,   49, 
/* 7440 */   129,   30,   46,  129,   30,   42,  129,   30,    4,  130, 
/* 7450 */   193,  129,   30,    4,   80,    4,  129,   30,    4,   80, 
/* 7460 */     4,  100,    6,  129,   30,   54,   98,   10,  129,   30, 
/* 7470 */    54,   97,   12,  129,   30,   54,   96,   14,  131,   30, 
/* 7480 */    54,   97,   12,  129,   30,   54,   98,   10,  129,   30, 
/* 7490 */    54,  100,    6,  129,   30,    4,  130,  193,  129,    7, 
/* 7500 */     6,  129,    4,   11,  129,    3,   13,  129,    2,   14, 
/* 7510 */   129,    1,   15,  130,    1,    3,    6,    9,  129,    1, 
/* 7520 */     3,    7,    6,  129,    1,    3,  130,    1,    4,  129, 
/* 7530 */     1,    5,   80,    4,  129,    1,    7,   80,    4,  100, 
/* 7540 */     6,  129,    2,   82,   98,   10,  129,    3,   81,   97, 
/* 7550 */    12,  129,    4,   80,   96,   14,  129,    5,   79,   96,
/* 7560 */    14,  129,    7,   77,   96,   14,  129,   10,   74,   97,
/* 7570 */    12,  129,   14,   70,   98,   10,  129,   19,   65,  100, 
/* 7580 */     6,  129,  193,  129,   30,    4,  117,    4,  130,   30, 
/* 7590 */    91,  136,   30,    4,   57,    9,  129,   30,    4,   55, 
/* 7600 */    12,  129,   52,   17,  129,   50,   20,  129,   48,   24,
/* 7610 */   129,   46,   27,  129,   44,   21,   69,    6,  129,   41,
/* 7620 */    22,   70,    6,   80,    4,  129,   30,    4,   39,   21, 
/* 7630 */    72,    6,   80,    4,  129,   30,    4,   36,   22,   73, 
/* 7640 */    11,  129,   30,   26,   75,    9,  129,   30,   23,   76, 
/* 7650 */     8,  129,   30,   21,   78,    6,  129,   30,   19,   79, 
/* 7660 */     5,  129,   30,   16,   80,    4,  129,   30,   14,   80, 
/* 7670 */     4,  129,   30,   12,  129,   30,   10,  129,   30,    7, 
/* 7680 */   129,   30,    5,  129,   30,    4,  130,  193,  129,   30, 
/* 7690 */     4,  117,    4,  130,   30,   91,  136,   30,    4,  130, 
/* 7700 */   193,  129,   30,    4,   80,    4,  130,   30,   54,  136, 
/* 7710 */    30,    4,   72,    5,  129,   30,    4,   74,    5,  129, 
/* 7720 */    75,    5,  129,   76,    5,  129,   30,    4,   75,    7, 
/* 7730 */   129,   30,    4,   74,    9,  129,   30,   54,  132,   30, 
/* 7740 */    53,  129,   30,   52,  129,   30,   51,  129,   30,   48,
/* 7750 */   129,   30,    4,   72,    5,  129,   30,    4,   74,    5, 
/* 7760 */   129,   75,    5,  129,   76,    5,  129,   30,    4,   75, 
/* 7770 */     7,  129,   30,    4,   74,    9,  129,   30,   54,  132, 
/* 7780 */    30,   53,  129,   30,   52,  129,   30,   51,  129,   30, 
/* 7790 */    48,  129,   30,    4,  130,  193,  129,   30,    4,   80,
/* 7800 */     4,  130,   30,   54,  136,   30,    4,   72,    5,  129, 
/* 7810 */    30,    4,   74,    5,  129,   75,    5,  129,   76,    5, 
/* 7820 */   129,   76,    6,  129,   77,    6,  130,   77,    7,  130, 
/* 7830 */    76,    8,  129,   30,    4,   75,    9,  129,   30,    4, 
/* 7840 */    72,   12,  129,   30,   54,  129,   30,   53,  130,   30, 
/* 7850 */    52,  129,   30,   51,  129,   30,   49,  129,   30,   46, 
/* 7860 */   129,   30,   42,  129,   30,    4,  130,  193,  129,   48, 
/* 7870 */    18,  129,   43,   28,  129,   41,   32,  129,   39,   36, 
/* 7880 */   129,   37,   40,  129,   35,   44,  129,   34,   46,  129, 
/* 7890 */    33,   13,   68,   13,  129,   32,    9,   73,    9,  129, 
/* 7900 */    32,    7,   75,    7,  129,   31,    6,   77,    6,  129, 
/* 7910 */    31,    5,   78,    5,  129,   30,    5,   79,    5,  129, 
/* 7920 */    30,    4,   80,    4,  132,   30,    5,   79,    5,  130, 
/* 7930 */    31,    5,   78,    5,  129,   31,    6,   77,    6,  129, 
/* 7940 */    32,    7,   75,    7,  129,   32,    9,   73,    9,  129, 
/* 7950 */    33,   13,   68,   13,  129,   34,   46,  129,   35,   44, 
/* 7960 */   129,   37,   40,  129,   39,   36,  129,   41,   32,  129, 
/* 7970 */    43,   28,  129,   48,   18,  129,  193,  129,    1,    3, 
/* 7980 */    80,    4,  130,    1,   83,  137,   37,    5,   72,    5, 
/* 7990 */   129,   35,    5,   74,    5,  129,   33,    5,   76,    5, 
/* 8000 */   129,   32,    5,   77,    5,  129,   31,    5,   78,    5, 
/* 8010 */   129,   31,    4,   79,    4,  129,   30,    5,   79,    5, 
/* 8020 */   131,   30,    6,   78,    6,  129,   30,    7,   77,    7, 
/* 8030 */   129,   31,    8,   75,    8,  129,   31,   11,   72,   11, 
/* 8040 */   129,   32,   15,   67,   15,  129,   33,   48,  129,   34, 
/* 8050 */    46,  129,   35,   44,  129,   37,   40,  129,   39,   36, 
/* 8060 */   129,   42,   30,  129,   46,   22,  129,  193,  129,   46, 
/* 8070 */    22,  129,   42,   30,  129,   39,   36,  129,   37,   40, 
/* 8080 */   129,   35,   44,  129,   34,   46,  129,   33,   48,  129, 
/* 8090 */    32,   15,   67,   15,  129,   31,   11,   72,   11,  129,
/* 8100 */    31,    8,   75,    8,  129,   30,    7,   77,    7,  129,
/* 8110 */    30,    6,   78,    6,  129,   30,    5,   79,    5,  131, 
/* 8120 */    31,    4,   79,    4,  129,   31,    5,   78,    5,  129, 
/* 8130 */    32,    5,   77,    5,  129,   33,    5,   76,    5,  129, 
/* 8140 */    35,    5,   74,    5,  129,   37,    5,   72,    5,  129,
/* 8150 */     1,   83,  136,    1,    3,   80,    4,  130,  193,  129,
/* 8160 */    30,    4,   80,    4,  130,   30,   54,  136,   30,    4, 
/* 8170 */    68,    6,  129,   30,    4,   70,    6,  129,   71,    7, 
/* 8180 */   129,   72,    7,  129,   73,    7,  129,   74,    7,  129, 
/* 8190 */    74,    8,  129,   75,    8,  130,   69,   15,  129,   67, 
/* 8200 */    17,  129,   66,   18,  129,   65,   19,  130,   65,   18, 
/* 8210 */   130,   66,   16,  129,   67,   13,  129,   69,    8,  129, 
/* 8220 */   193,  129,   30,   13,   64,    8,  129,   30,   13,   61, 
/* 8230 */    14,  129,   30,   13,   59,   18,  129,   30,   13,   57, 
/* 8240 */    22,  129,   33,    8,   56,   24,  129,   32,    7,   55, 
/* 8250 */    26,  129,   32,    6,   54,   28,  129,   31,    6,   53,
/* 8260 */    16,   77,    6,  129,   31,    5,   53,   14,   79,    4, 
/* 8270 */   129,   30,    5,   52,   14,   80,    4,  129,   30,    5, 
/* 8280 */    52,   13,   80,    4,  129,   30,    4,   52,   13,   80, 
/* 8290 */     4,  129,   30,    4,   52,   12,   80,    4,  129,   30, 
/* 8300 */     4,   51,   13,   80,    4,  130,   30,    4,   50,   13,
/* 8310 */    79,    5,  129,   30,    4,   50,   13,   78,    5,  129, 
/* 8320 */    30,    5,   49,   14,   77,    6,  129,   31,    4,   49, 
/* 8330 */    13,   76,    6,  129,   31,    5,   48,   14,   75,    7, 
/* 8340 */   129,   32,    5,   47,   14,   73,    8,  129,   32,    6, 
/* 8350 */    45,   16,   71,   13,  129,   33,   27,   71,   13,  129, 
/* 8360 */    34,   26,   71,   13,  129,   35,   24,   71,   13,  129, 
/* 8370 */    37,   20,  129,   39,   16,  129,   43,    9,  129,  193, 
/* 8380 */   129,   80,    4,  131,   41,   56,  129,   37,   60,  129, 
/* 8390 */    35,   62,  129,   33,   64,  129,   32,   65,  129,   31, 
/* 8400 */    66,  129,   30,   67,  130,   30,   11,   80,    4,  129, 
/* 8410 */    30,    9,   80,    4,  129,   30,    8,   80,    4,  129, 
/* 8420 */    31,    7,   80,    4,  129,   31,    6,  129,   32,    5, 
/* 8430 */   129,   33,    5,  129,   35,    4,  129,   38,    3,  129, 
/* 8440 */   193,  129,   80,    4,  130,   42,   42,  129,   38,   46, 
/* 8450 */   129,   35,   49,  129,   33,   51,  129,   32,   52,  129, 
/* 8460 */    31,   53,  130,   30,   54,  129,   30,   12,  129,   30, 
/* 8470 */     9,  129,   30,    8,  129,   30,    7,  130,   31,    6, 
/* 8480 */   130,   32,    6,  129,   33,    5,  129,   34,    5,  129, 
/* 8490 */    35,    5,   80,    4,  129,   37,    5,   80,    4,  129, 
/* 8500 */    30,   54,  136,   30,    4,  130,  193,  129,   80,    4, 
/* 8510 */   130,   77,    7,  129,   74,   10,  129,   70,   14,  129, 
/* 8520 */    66,   18,  129,   62,   22,  129,   59,   25,  129,   55, 
/* 8530 */    29,  129,   51,   33,  129,   47,   37,  129,   44,   32, 
/* 8540 */    80,    4,  129,   40,   32,   80,    4,  129,   36,   32, 
/* 8550 */   129,   32,   33,  129,   30,   31,  129,   33,   24,  129, 
/* 8560 */    36,   17,  129,   40,   12,  129,   44,   12,  129,   48, 
/* 8570 */    12,  129,   51,   13,  129,   55,   13,  129,   59,   13, 
/* 8580 */    80,    4,  129,   63,   13,   80,    4,  129,   67,   17, 
/* 8590 */   129,   71,   13,  129,   74,   10,  129,   78,    6,  129, 
/* 8600 */    80,    4,  131,  193,  129,   80,    4,  130,   77,    7, 
/* 8610 */   129,   74,   10,  129,   70,   14,  129,   66,   18,  129, 
/* 8620 */    62,   22,  129,   59,   25,  129,   55,   29,  129,   51, 
/* 8630 */    33,  129,   47,   37,  129,   44,   32,   80,    4,  129,
/* 8640 */    40,   32,   80,    4,  129,   36,   32,  129,   32,   33,
/* 8650 */   129,   30,   31,  129,   33,   24,  129,   36,   17,  129, 
/* 8660 */    40,   12,  129,   44,   12,  129,   47,   13,  129,   44, 
/* 8670 */    20,  129,   40,   28,  129,   36,   31,  129,   32,   32, 
/* 8680 */   129,   30,   30,  129,   33,   24,  129,   36,   17,  129,
/* 8690 */    40,   12,  129,   44,   12,  129,   48,   12,  129,   51,
/* 8700 */    13,  129,   55,   13,  129,   59,   13,   80,    4,  129, 
/* 8710 */    63,   13,   80,    4,  129,   67,   17,  129,   71,   13, 
/* 8720 */   129,   74,   10,  129,   78,    6,  129,   80,    4,  131, 
/* 8730 */   193,  129,   30,    4,   80,    4,  130,   30,    4,   79, 
/* 8740 */     5,  129,   30,    5,   77,    7,  129,   30,    6,   74, 
/* 8750 */    10,  129,   30,    8,   72,   12,  129,   30,   11,   69, 
/* 8760 */    15,  129,   30,   13,   67,   17,  129,   30,    4,   37,
/* 8770 */     8,   64,   20,  129,   30,    4,   39,    8,   62,   22, 
/* 8780 */   129,   41,    8,   59,   25,  129,   43,    8,   57,   27, 
/* 8790 */   129,   45,    8,   55,   22,   80,    4,  129,   47,   27, 
/* 8800 */    80,    4,  129,   49,   23,  129,   47,   22,  129,   44, 
/* 8810 */    23,  129,   42,   22,  129,   30,    4,   39,   27,  129,
/* 8820 */    30,    4,   37,   31,  129,   30,   27,   62,    8,  129, 
/* 8830 */    30,   25,   64,    8,  129,   30,   22,   66,    8,   80, 
/* 8840 */     4,  129,   30,   20,   68,    8,   80,    4,  129,   30, 
/* 8850 */    17,   70,    8,   80,    4,  129,   30,   15,   73,   11, 
/* 8860 */   129,   30,   12,   75,    9,  129,   30,   10,   77,    7, 
/* 8870 */   129,   30,    7,   79,    5,  129,   30,    5,   80,    4, 
/* 8880 */   129,   30,    4,   80,    4,  130,  193,  129,    4,    5, 
/* 8890 */    80,    4,  129,    2,    9,   80,    4,  129,    1,   11, 
/* 8900 */    77,    7,  129,    1,   12,   74,   10,  129,    1,   12, 
/* 8910 */    70,   14,  129,    1,   12,   66,   18,  129,    1,   11, 
/* 8920 */    62,   22,  129,    2,    9,   59,   25,  129,    4,   11, 
/* 8930 */    55,   29,  129,    7,   12,   51,   33,  129,   10,   12, 
/* 8940 */    47,   37,  129,   14,   12,   44,   32,   80,    4,  129, 
/* 8950 */    17,   13,   40,   32,   80,    4,  129,   21,   13,   36, 
/* 8960 */    32,  129,   25,   40,  129,   29,   32,  129,   33,   24, 
/* 8970 */   129,   36,   17,  129,   40,   12,  129,   44,   12,  129, 
/* 8980 */    48,   12,  129,   51,   13,  129,   55,   13,  129,   59, 
/* 8990 */    13,   80,    4,  129,   63,   13,   80,    4,  129,   67, 
/* 9000 */    17,  129,   71,   13,  129,   74,   10,  129,   78,    6, 
/* 9010 */   129,   80,    4,  131,  193,  129,   30,    1,   71,   13, 
/* 9020 */   129,   30,    3,   71,   13,  129,   30,    6,   71,   13, 
/* 9030 */   129,   30,    9,   75,    9,  129,   30,   11,   77,    7, 
/* 9040 */   129,   30,   14,   79,    5,  129,   30,   17,   79,    5, 
/* 9050 */   129,   30,   19,   80,    4,  129,   30,   22,   80,    4, 
/* 9060 */   129,   30,   25,   80,    4,  129,   30,   27,   80,    4, 
/* 9070 */   129,   30,    4,   36,   24,   80,    4,  129,   30,    4, 
/* 9080 */    38,   25,   80,    4,  129,   30,    4,   41,   24,   80, 
/* 9090 */     4,  129,   30,    4,   44,   24,   80,    4,  129,   30, 
/* 9100 */     4,   46,   25,   80,    4,  129,   30,    4,   49,   25, 
/* 9110 */    80,    4,  129,   30,    4,   52,   24,   80,    4,  129, 
/* 9120 */    30,    4,   54,   30,  129,   30,    4,   57,   27,  129, 
/* 9130 */    30,    4,   59,   25,  129,   30,    4,   62,   22,  129, 
/* 9140 */    30,    4,   65,   19,  129,   30,    5,   67,   17,  129, 
/* 9150 */    30,    5,   70,   14,  129,   30,    7,   73,   11,  129, 
/* 9160 */    30,    9,   76,    8,  129,   30,   13,   78,    6,  129, 
/* 9170 */    30,   13,   81,    3,  129,   30,   13,  129,  193,    2,
/* 9180 */     9,   59,   25,  129,    4,   11,   55,   29,  129,    7,
/* 9190 */    12,   51,   33,  129,   10,   12,   47,   37,  129,   14,
/* 9200 */    12,   44,   32,   80,    4,  129,   17,   13,   40,   32,
/* 9210 */    80,    4,  129,   21,   13,   36,   32,  129,   25,   40,
/* 9220 */   129,   29,   32,  129,   33,   24,  129,   36,   17,  129,
/* 9230 */    40,   12,  129,   44,   12,  129,   48,   12,  129,   51,
/* 9240 */    13,  129,   55,   13,  129,   59,   13,   80,    4,  129,
/* 9250 */    63,   13,   80,    4,  129,   67,   17,  129,   71,   13,
/* 9260 */   129,   74,   10,  129,   78,    6,  129,   80,    4,  131,
/* 9270 */   193
};

int	FBDrawString( int xpos, int ypos, int height, char *msg,
			unsigned char col,		/* text color */
			unsigned char backcol )	/* background 0==transp */
{ 
	char	line[DWIDTH];
	char	message[MAXMSG];
	char	print[DWIDTH];
	int		i, j, linen, max, nchars, pc, term, x, y;
	int		gx = xpos;
	int		gy = ypos + height -1;

	strcpy(message,msg);

	memset(print,0,DWIDTH);

	for (i = 0; i < height; i++)
	{
		j = i * 132 / height;
		print[j] = 1;
	}

	nchars = strlen(message);

	/* check message to make sure it's legal */
	for (i = 0; i < nchars; i++)
	{
		if ((u_char) message[i] >= NCHARS ||
		    asc_ptr[(u_char) message[i]] == 0)
		{
			return(gx-xpos);
		}
	}

	for (i = 0; i < nchars; i++)
	{
		for (j = 0; j < DWIDTH; j++)
			line[j] = ' ';
		pc = asc_ptr[(u_char) message[i]];
		term = 0;
		max = 0;
		linen = 0;
		while (!term)
		{
			if (pc < 0 || pc > NBYTES)
			{
				return(gx-xpos);
			}
			x = data_table[pc] & 0377;
			if (x >= 128)
			{
				if (x>192) term++;
				x = x & 63;
				while (x--)
				{
					if (print[linen++])
					{
						for (j=0; j <= max; j++)
						{
							if (print[j])
							{
								if ( line[j] == '#' )
								{
									FBPaintPixel(gx,gy,col);
								}
								else
								{
									if ( backcol )
										FBPaintPixel(gx,gy,backcol);
								}
								gy--;
							}
						}
						for (;backcol && (j<DWIDTH);j++)
						{
							if (print[j])
							{
								FBPaintPixel(gx,gy,backcol);
								gy--;
							}
						}
						gx++;
						gy=ypos+height-1;
					}
				}
				for (j = 0; j < DWIDTH; j++)
					line[j] = ' ';
				pc++;
			}
			else
			{
				y = data_table[pc+1];
				/* compensate for narrow teminals */
#ifdef notdef
				x = (x*height + (DWIDTH/2)) / DWIDTH;
				y = (y*height + (DWIDTH/2)) / DWIDTH;
#endif
				max = x+y;
				while (x < max) line[x++] = '#';
				pc += 2;
			}
		}
	}

	return(gx-xpos);
}

void	FBDrawFx2Logo( int x, int y )
{
	FBDrawString( x, y, 64, "fx", WHITE, 0 );
	FBDrawString( x+21, y+9, 64, "2", RED, 0 );
}

static	char	*keyw[12] = {
"-+.", "ABC", "DEF", "GHI", "JKL", "MNO", "PQRS", "TUV", "WXYZ", "", " ", ""
};
static	char	keywi[12] = {17,35,33,28,31,35,43,35,51,2,2,2};
static	char	*keynum[12] ={"1","2","3","4","5","6","7","8","9","","0",""};
static	char	keyn[12] = {6,13,13,13,12,12,13,13,12,9,13,6};
static	char	text[128];

typedef struct _T9Words
{
	char	*in;
	char	*out;
} T9Words;

static	T9Words	wlist[] = {
{ "A4",			"Biggi" },
{ "AGPGPTGAM",	"Christian" },
{ "AIDPT",		"Biggest" },
{ "DAPAKA",		"faralla" },
{ "DWA",		"fx2" },
{ "EPGDT",		"derget" },
{ "GTMW",		"Hunz" },
{ "JAMGMD",		"Janine" },
{ "JATJDP",		"Katjes" },
{ "JMJT",		"Jolt" },
{ "MAPJGMGM",	"markinho" },
{ "MAG",		"obi" },
{ "MAWJ",		"Mayk" },
{ "MBJDAM",		"McClean" },
{ "MPA",		"Opa" },
{ "NA",			"Oma" },
{ "PGADMW",		"Shadow" },
{ "PGJTGA",		"Silvia" },
{ "PMTAPT",		"Rotart" },
{ "TGEMA-",		"TheDOC1" },
{ "TMAGMA",		"tmbinc" },
{ "WAJDG",		"waldi" },
{ 0,			0 }
};

char	*FBEnterWord( int xpos, int ypos, int height,int len,unsigned char col)
{
	struct timeval	tv;
	int				xoffs=500;
	int				yoffs=32;
	int				x;
	int				y;
	int				i;
	short			lastcode=0xee;
	short			last=0xee;
	int				idx=0;
	int				subidx=0;
	char			*pos;
	int				dlen = 0;
	char			blocker=0;
	char			autot9=0;

	/* draw help */
	FBDrawRect( xoffs,yoffs, 3*52,4*52+4,WHITE);
	FBDrawRect( xoffs+1,yoffs+1, 3*52,4*52+4,WHITE);
	for( i=0; i<12; i++ )
	{
		y=(i/3)*52 + yoffs + 20;
		x=(i%3)*52 + xoffs + 26 - keyn[i]/2;
		FBDrawString( x,y,44,keynum[i],RED,0);

		y=(i/3)*52 + yoffs + 5;
		x=(i%3)*52 + xoffs + 26 - keywi[i]/2;
		FBDrawString( x,y,32,keyw[i],WHITE,0);
		if ( !i )
			FBDrawString( x+1,y+1,32,keyw[i],WHITE,0);
	}
	FBDrawHLine( xoffs,yoffs+56,3*52,WHITE);
	FBDrawHLine( xoffs,yoffs+57,3*52,WHITE);
	FBDrawHLine( xoffs,yoffs+108,3*52,WHITE);
	FBDrawHLine( xoffs,yoffs+109,3*52,WHITE);
	FBDrawHLine( xoffs,yoffs+160,3*52,WHITE);
	FBDrawHLine( xoffs,yoffs+161,3*52,WHITE);
	FBDrawVLine( xoffs + 52, yoffs, 4*52+4, WHITE );
	FBDrawVLine( xoffs + 53, yoffs, 4*52+4, WHITE );
	FBDrawVLine( xoffs + 104, yoffs, 4*52+4, WHITE );
	FBDrawVLine( xoffs + 105, yoffs, 4*52+4, WHITE );

	/* now read word */
	*text=0;
	memset(text,0,32);
	pos=text-1;
	i=0;
	while( i<len && !doexit )
	{
		tv.tv_usec = 100000;
		tv.tv_sec = 0;
		select(0,0,0,0,&tv);

		actcode=0xee;
		RcGetActCode();

		if ( realcode == 0xee )
			blocker=0;

		if ( blocker && ( actcode == last ))
			continue;

		last = actcode;
		blocker=1;
#ifdef USEX
		FBFlushGrafic();
#endif
		autot9=0;
		if ( actcode <= 9 )	/* RC_0 .. RC_9 */
		{
			if ( actcode != lastcode )
			{
				pos++;
				idx=actcode-1;
				if ( idx == -1 )
					idx=10;
				subidx=0;
				autot9=1;
				*pos=keyw[idx][subidx];
				i++;
				lastcode=actcode;
			}
			else
			{
				subidx++;
				if ( keyw[idx][subidx] )
				{
					*pos=keyw[idx][subidx];
				}
				else
				{
					subidx=-1;
					*pos=*keynum[idx];
				}
			}
			*(pos+1)=' ';
			dlen=FBDrawString( xpos, ypos, 64, text, WHITE, BLACK);
		}
		if ( actcode == RC_LEFT )
		{
			if ( i > 0 )
			{
				*pos=' ';
				*(pos+1)=0;
				pos--;
				i--;
				FBFillRect( xpos, ypos, dlen, 64, BLACK );
				dlen=FBDrawString( xpos, ypos, 64, text, WHITE, BLACK);
			}
		}
		if ( actcode == RC_RIGHT )
		{
			pos++;
			i++;
			subidx=-1;
			*(pos+1)=' ';
		}
		if ( actcode == RC_OK )
		{
			if ( i > 0 )
			{
				pos++;
				i++;
				break;
			}
		}
		if ( actcode == RC_BLUE )
		{
			if ( i > 0 )
			{
				autot9=1;
				pos++;
				i++;
				break;
			}
		}
	}
	*pos=0;
	while( realcode != 0xee )
		RcGetActCode();
	actcode=0xee;

	if ( autot9 )
	{
		for( i=0; i<sizeof(wlist)/sizeof(T9Words); i++ )
		{
			y = strcmp(wlist[i].in,text);
			if ( !y )
			{
				strcpy(text,wlist[i].out);
				break;
			}
			if ( y > 0 )
				break;
		}
	}
	FBFillRect( xoffs,yoffs, 3*52+3,4*52+4+2,BLACK);
	return( text );
}
