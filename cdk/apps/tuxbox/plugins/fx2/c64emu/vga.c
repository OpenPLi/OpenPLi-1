#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stropts.h>
#include <linux/fb.h>
#include <sys/mman.h>

extern	int	atoi(char* in);

#define XOFF		150
#define YOFF		130

#include <fbemul.h>

#ifdef i386
#define fbdevname	"/dev/fb0"
#else
#define fbdevname	"/dev/fb/0"
#endif

static	int							fd = -1;
static	struct fb_var_screeninfo	screeninfo;
static	struct fb_var_screeninfo	oldscreen;
static	int							available;
static	unsigned char				*lfb = 0;
static	int							stride;
static	int							bpp = 8;
static	struct fb_cmap				cmap;
static	unsigned short				red[ 256 ];
static	unsigned short				green[ 256 ];
static	unsigned short				blue[ 256 ];
static	unsigned short				trans[ 256 ];
static	struct fb_fix_screeninfo	fix;
static	vga_modeinfo				mymode;

typedef	unsigned char				uchar;

void	vga_setpalette( int i, unsigned char r, unsigned char g, unsigned char b )
{
	if ( !r )
		r++;
	if ( !g )
		g++;
	if ( !b )
		b++;
	red[i] = r<<8;
	green[i] = g<<8;
	blue[i] = b<<8;
	trans[i] = 0;

	if ( i==255 )
	{
		cmap.start=0;
		cmap.len=256;
		cmap.red=red;
		cmap.green=green;
		cmap.blue=blue;
		cmap.transp=trans;

		if ( fd != -1 )
			ioctl(fd,FBIOPUTCMAP,&cmap);
	}
}

int		vga_setlinearaddressing( void )
{
	return 0;
}

int	vga_setmode( int mode )
{
	if ( mode == 1 )
	{
		if ( fd == -1 )
			return 0;

		memset(lfb,0,stride*screeninfo.yres);
		ioctl(fd,FBIOPUT_VSCREENINFO, &oldscreen);
		munmap(lfb,available);
		return 0;
	}

	if ( fd != -1 )
		return 0;

	fd = open( fbdevname, O_RDWR );
	if ( fd == -1 )
		return -1;

	if ( ioctl(fd,FBIOGET_VSCREENINFO, &screeninfo)<0)
	{
		perror("failed - FBIOGET_VSCREENINFO");
		return -1;
	}
	memcpy(&oldscreen,&screeninfo,sizeof(screeninfo));

	screeninfo.xres_virtual=screeninfo.xres=720;
	screeninfo.yres_virtual=screeninfo.yres=576;
	screeninfo.bits_per_pixel=8;
	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0)
		perror("FBSetMode");
#if 0
	screeninfo.xres_virtual=screeninfo.xres=320;
	screeninfo.yres_virtual=screeninfo.yres=288;
	screeninfo.bits_per_pixel=8;

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0)
	{
		perror("FBSetMode");
		screeninfo.xres_virtual=screeninfo.xres=720;
		screeninfo.yres_virtual=screeninfo.yres=576;
		screeninfo.bits_per_pixel=8;
		if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0)
			perror("FBSetMode");
	}
#endif
	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo)<0)
		perror("failed - FBIOGET_VSCREENINFO");

	bpp = screeninfo.bits_per_pixel;

	if ( bpp != 8 )
		return -1;

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("failed - FBIOGET_FSCREENINFO");
		return -1;
	}

	available=fix.smem_len;
	stride = fix.line_length;
	lfb=(unsigned char*)mmap(0,available,PROT_WRITE|PROT_READ, MAP_SHARED,fd,0);

	if ( !lfb )
	{
		perror("failed - mmap");
		return -1;
	}
	return 0;
}

vga_modeinfo	*vga_getmodeinfo( int mode )
{
	vga_setmode(2);		// switch to graphic

	if ( fd == -1 )
		return 0;

	mymode.width = screeninfo.xres;
#ifdef YOFF
	mymode.height = screeninfo.yres - YOFF - 1;
#endif
	mymode.bytesperpixel = 1;
	mymode.colors = 256;
	mymode.linewidth = mymode.width;
	mymode.memory = (available-stride*YOFF-XOFF) / 1024;
	mymode.flags = CAPABLE_LINEAR;

	return &mymode;
}

void	*vga_getgraphmem( void )
{
	return lfb + XOFF + stride*YOFF;
}

#define FX_WIDTH	45

#define _	0
#define W	1
#define R	2

static	unsigned char data_fx2[] = {
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,W,W,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,W,W,W,_,_,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,W,W,W,_,_,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,R,R,R,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,R,R,R,R,R,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,_,_,_,_,R,R,R,R,R,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,_,_,_,_,_,_,R,R,R,R,R,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,_,
W,W,W,W,W,W,W,W,W,_,_,_,W,W,W,W,W,W,W,_,_,_,_,W,W,W,W,W,_,_,_,_,_,_,_,R,R,R,_,_,_,_,_,_,_,
W,W,W,W,W,W,W,W,W,_,_,_,W,W,W,W,W,W,W,_,_,_,_,W,W,W,W,W,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,W,W,W,W,_,_,_,_,_,_,_,W,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,W,W,W,W,W,_,_,_,_,_,W,W,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,W,W,W,W,W,_,_,_,_,_,W,_,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,W,W,W,W,_,_,_,_,W,W,_,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,W,W,W,W,W,_,_,_,W,W,_,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,W,W,W,W,_,_,W,W,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,W,W,W,W,W,_,W,W,_,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,W,W,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,W,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,W,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,W,W,W,W,W,_,_,_,_,_,_,_,_,R,R,R,R,R,R,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,W,W,W,W,_,_,_,_,_,_,_,_,R,R,R,R,R,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,W,W,W,W,W,W,_,_,_,_,_,_,R,R,R,R,R,R,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,_,W,W,W,W,W,W,_,_,_,_,_,R,R,R,R,R,R,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,W,W,_,W,W,W,W,_,_,_,_,_,R,R,R,R,R,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,_,W,W,_,W,W,W,W,W,_,_,_,R,R,R,R,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,W,W,_,_,_,W,W,W,W,_,_,R,R,R,R,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,_,W,W,_,_,_,W,W,W,W,W,_,R,R,R,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,W,W,_,_,_,_,W,W,W,W,W,R,R,R,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,_,W,W,_,_,_,_,_,W,W,W,W,R,R,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,W,W,W,W,_,_,_,_,_,_,_,_,W,W,_,_,_,_,_,_,W,W,W,R,R,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,W,W,W,W,W,W,_,_,_,_,_,W,W,W,W,W,_,_,_,_,W,W,W,W,R,R,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,W,W,W,W,W,W,_,_,_,_,_,W,W,W,W,W,_,_,_,_,W,W,W,R,R,W,W,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,_,_,_,_,_,_,_,_,_,_,_,_,R,R,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
};

void	vga_drawlogo( void )
{
	unsigned char	*to;
	unsigned char	*from = data_fx2;
	int				x;
	int				y;

	for (y=0;y<64;y++)
	{
		to = lfb + (stride*(y+64)) - 100;
		memcpy(to,from,FX_WIDTH);
		from+=FX_WIDTH;
	}
}

void	vga_printscreen( char *fname )
{
	FILE			*fp;
	unsigned int	x;
	unsigned int	y;
	unsigned int	i;
	unsigned char	*p=lfb;

#define	H(x)	((x/26)+65)
#define	L(x)	((x%26)+65)

	fp=fopen(fname,"w");
	if ( !fp )
		return;
	fprintf(fp,"/* XPM */\n");
	fprintf(fp,"static char *screen[] = {\n");
	fprintf(fp,"\"  %d   %d   256   2\"",screeninfo.xres,screeninfo.yres);
	for(x=0; x<256; x++ )
		fprintf(fp,",\n\"%c%c c #%02x%02x%02x\"",H(x),L(x),
				(unsigned char)(red[x] >> 8),
				(unsigned char)(green[x] >> 8),
				(unsigned char)(blue[x] >> 8) );
	for(y=0;y<screeninfo.yres;y++)
	{
		fprintf(fp,",\n\"");
		for(x=0;x<screeninfo.xres;x++,p++)
		{
			fprintf(fp,"%c%c",H(*p),L(*p));
		}
		fprintf(fp,"\"");
		fflush(fp);
	}
	fprintf(fp," };\n");
	fclose(fp);
}
