/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>

#include <linux/kd.h>

#include <global.h>

#include "framebuffer.h"


CFrameBuffer::CFrameBuffer()
: active ( true )
{
	iconBasePath = "";
	available  = 0;
	cmap.start = 0;
	cmap.len = 256;
	cmap.red = red;
	cmap.green = green;
	cmap.blue  = blue;
	cmap.transp = trans;
	backgroundColor = 0;
	useBackgroundPaint = false;
	background = NULL;
	backgroundFilename = "";
	fd  = 0;
	tty = 0;
}

CFrameBuffer* CFrameBuffer::getInstance()
{
	static CFrameBuffer* frameBuffer = NULL;

	if(!frameBuffer)
	{
		frameBuffer = new CFrameBuffer();
		printf("[neutrino] frameBuffer Instance created\n");
	}
	else
	{
		//printf("[neutrino] frameBuffer Instace requested\n");
	}
	return frameBuffer;
}

void CFrameBuffer::init(string fbDevice)
{
	fd=open(fbDevice.c_str(), O_RDWR);
	if (fd<0)
	{
		perror(fbDevice.c_str());
		goto nolfb;
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo)<0)
	{
		perror("FBIOGET_VSCREENINFO");
		goto nolfb;
	}

	memcpy(&oldscreen, &screeninfo, sizeof(screeninfo));

	fb_fix_screeninfo fix;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("FBIOGET_FSCREENINFO");
		goto nolfb;
	}

	available=fix.smem_len;
	printf("%dk video mem\n", available/1024);
	lfb=(unsigned char*)mmap(0, available, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);

	if (!lfb)
	{
		perror("mmap");
		goto nolfb;
	}

	if ((tty=open("/dev/vc/0", O_RDWR))<0)
	{
		perror("open (tty)");
		goto nolfb;
	}

	struct sigaction act;

	memset(&act,0,sizeof(act));
	act.sa_handler  = switch_signal;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1,&act,NULL);
	sigaction(SIGUSR2,&act,NULL);

	struct vt_mode mode;

	if (-1 == ioctl(tty,KDGETMODE, &kd_mode)) {
		perror("ioctl KDGETMODE");
		goto nolfb;
	}

	if (-1 == ioctl(tty,VT_GETMODE, &vt_mode)) {
      		perror("ioctl VT_GETMODE");
		goto nolfb;
	}

	if (-1 == ioctl(tty,VT_GETMODE, &mode)) {
      		perror("ioctl VT_GETMODE");
		goto nolfb;
	}

	mode.mode   = VT_PROCESS;
	mode.waitv  = 0;
	mode.relsig = SIGUSR1;
	mode.acqsig = SIGUSR2;

	if (-1 == ioctl(tty,VT_SETMODE, &mode)) {
		perror("ioctl VT_SETMODE");
		goto nolfb;
	}

	if (-1 == ioctl(tty,KDSETMODE, KD_GRAPHICS)) {
		perror("ioctl KDSETMODE");
		goto nolfb;
	}

	return;

nolfb:
	printf("framebuffer not available.\n");
	lfb=0;
}


CFrameBuffer::~CFrameBuffer()
{
	if(background)
	{
		delete[] background;
	}

	if (-1 == ioctl(tty,KDSETMODE, kd_mode))
	  perror("ioctl KDSETMODE");

	if (-1 == ioctl(tty,VT_SETMODE, &vt_mode))
		perror("ioctl VT_SETMODE");

	/*
	if (available)
		ioctl(fd, FBIOPUT_VSCREENINFO, &oldscreen);
	if (lfb)
		munmap(lfb, available);
		*/
}

int CFrameBuffer::getFileHandle()
{
	return fd;
}

unsigned int CFrameBuffer::getStride()
{
	return stride;
}

unsigned char* CFrameBuffer::getFrameBufferPointer()
{
	return lfb;
}

bool CFrameBuffer::getActive()
{
	return active;
}

int CFrameBuffer::setMode(unsigned int nxRes, unsigned int nyRes, unsigned int nbpp)
{
	if (!available&&!active)
		return -1;

	screeninfo.xres_virtual=screeninfo.xres=nxRes;
	screeninfo.yres_virtual=screeninfo.yres=nyRes;
	screeninfo.bits_per_pixel=nbpp;

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0)
	{
		perror("FBIOPUT_VSCREENINFO");
		return -1;
	}

	if ((screeninfo.xres!=nxRes) && (screeninfo.yres!=nyRes) && (screeninfo.bits_per_pixel!=nbpp))
	{
		printf("SetMode failed: wanted: %dx%dx%d, got %dx%dx%d\n",
		       nxRes, nyRes, nbpp,
		       screeninfo.xres, screeninfo.yres, screeninfo.bits_per_pixel);
		return -1;
	}

	xRes = screeninfo.xres;
	yRes = screeninfo.yres;
	bpp  = screeninfo.bits_per_pixel;
	fb_fix_screeninfo fix;

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("FBIOGET_FSCREENINFO");
		return -1;
	}

	stride=fix.line_length;
	memset(lfb, 0, stride*yRes);
	return 0;
}


void CFrameBuffer::paletteFade(int i, __u32 rgb1, __u32 rgb2, int level)
{
	__u16 *r = cmap.red+i;
	__u16 *g = cmap.green+i;
	__u16 *b = cmap.blue+i;
	*r= ((rgb2&0xFF0000)>>16)*level;
	*g= ((rgb2&0x00FF00)>>8 )*level;
	*b= ((rgb2&0x0000FF)    )*level;
	*r+=((rgb1&0xFF0000)>>16)*(255-level);
	*g+=((rgb1&0x00FF00)>>8 )*(255-level);
	*b+=((rgb1&0x0000FF)    )*(255-level);
}

void CFrameBuffer::setTransparency( int tr )
{
	if (!active)
		return;

	if (tr> 8)
		tr= 8;

	int val = (tr << 8) | tr;
	if (ioctl(fd, AVIA_GT_GV_SET_BLEV, val ))
		perror("AVIA_GT_GV_SET_BLEV");
}

void CFrameBuffer::setAlphaFade(int in, int num, int tr)
{
	for (int i=0; i<num; i++)
	{
		cmap.transp[in+i]=tr;
		//tr++;
	}
}

void CFrameBuffer::paletteGenFade(int in, __u32 rgb1, __u32 rgb2, int num, int tr)
{
	for (int i=0; i<num; i++)
	{
		paletteFade(in+i, rgb1, rgb2, i*(255/(num-1)));
		cmap.transp[in+i]=tr;
		tr++;
	}
}

void CFrameBuffer::paletteSetColor(int i, __u32 rgb, int tr)
{
	cmap.red[i]    =(rgb&0xFF0000)>>8;
	cmap.green[i]  =(rgb&0x00FF00)   ;
	cmap.blue[i]   =(rgb&0x0000FF)<<8;
	cmap.transp[i] =tr;
}

void CFrameBuffer::paletteSet()
{
	if (!active)
		return;

	ioctl(fd, FBIOPUTCMAP, &cmap);
}


void CFrameBuffer::paintBoxRel(int x, int y, int dx, int dy, unsigned char col)
{
	unsigned char* pos = lfb + x + stride*y;
	for(int count=0;count<dy;count++)
	{
		memset(pos, col, dx);
		pos += stride;
	}
}

void CFrameBuffer::paintBox(int xa, int ya, int xb, int yb, unsigned char col)
{
	if (!active)
		return;

	unsigned char* pos = lfb + xa + stride*ya;
	int dx = xb-xa;
	int dy = yb-ya;
	for(int count=0;count<dy;count++)
	{
		memset(pos, col, dx);
		pos += stride;
	}
}

void CFrameBuffer::paintVLine(int x, int ya, int yb, unsigned char col)
{
	if (!active)
		return;

	unsigned char* pos = lfb + x + stride*ya;
	int dy = yb-ya;
	for(int count=0;count<dy;count++)
	{
		*pos = col;
		pos += stride;
	}
}

void CFrameBuffer::paintVLineRel(int x, int y, int dy, unsigned char col)
{
	if (!active)
		return;

	unsigned char* pos = lfb + x + stride*y;
	for(int count=0;count<dy;count++)
	{
		*pos = col;
		pos += stride;
	}
}

void CFrameBuffer::paintHLine(int xa, int xb, int y, unsigned char col)
{
	if (!active)
		return;

	unsigned char* pos = lfb + xa + stride*y;
	int dx = xb -xa;
	memset(pos, col, dx);
}

void CFrameBuffer::paintHLineRel(int x, int dx, int y, unsigned char col)
{
	if (!active)
		return;

	unsigned char* pos = lfb + x + stride*y;
	memset(pos, col, dx);
}

void CFrameBuffer::setIconBasePath(string iconPath)
{
	iconBasePath = iconPath;
}

bool CFrameBuffer::paintIcon8(string filename, int x, int y, unsigned char offset)
{
	if (!active)
		return false;

	short width, height;
	unsigned char tr;

	int fd;
	filename = iconBasePath + filename;

	fd = open(filename.c_str(), O_RDONLY );

	if (fd == -1)
	{
		printf("error while loading icon: %s", filename.c_str() );
		return false;
	}

	read(fd, &width,  2 );
	read(fd, &height, 2 );
	read(fd, &tr, 1 );

	width= ((width & 0xff00) >> 8) | ((width & 0x00ff) << 8);
	height=((height & 0xff00) >> 8) | ((height & 0x00ff) << 8);

	unsigned char pixbuf[768];
	unsigned char *d = lfb + x +stride*y;
	unsigned char *d2;
	for (int count=0; count<height; count ++ )
	{
		read(fd, &pixbuf, width );
		unsigned char *pixpos = (unsigned char*) &pixbuf;
		d2 = d;
		for (int count2=0; count2<width; count2 ++ )
		{
			unsigned char color = *pixpos;
			if (color!=tr)
			{
				*d2 = color + offset;
			}
			d2++;
			pixpos++;
		}
		d += stride;
	}
	close(fd);
	return true;
}

bool CFrameBuffer::paintIcon(string filename, int x, int y, unsigned char offset)
{
	if (!active)
		return false;

	short width, height;
	unsigned char tr;

	int fd;
	filename = iconBasePath + filename;

	fd = open(filename.c_str(), O_RDONLY );

	if (fd == -1)
	{
		printf("error while loading icon: %s", filename.c_str() );
		return false;
	}

	read(fd, &width,  2 );
	read(fd, &height, 2 );
	read(fd, &tr, 1 );

	width= ((width & 0xff00) >> 8) | ((width & 0x00ff) << 8);
	height=((height & 0xff00) >> 8) | ((height & 0x00ff) << 8);

	unsigned char pixbuf[768];
	unsigned char *d = lfb + x +stride*y;
	unsigned char *d2;
	for (int count=0; count<height; count ++ )
	{
		read(fd, &pixbuf, width >> 1 );
		unsigned char *pixpos = (unsigned char*) &pixbuf;
		d2 = d;
		for (int count2=0; count2<width >> 1; count2 ++ )
		{
			unsigned char compressed = *pixpos;
			unsigned char pix1 = (compressed & 0xf0) >> 4;
			unsigned char pix2 = (compressed & 0x0f);

			if (pix1 != tr)
			{
				*d2 = pix1 + offset;
			}
			d2++;
			if (pix2 != tr)
			{
				*d2 = pix2 + offset;
			}
			d2++;
			pixpos++;
		}
		d += stride;
	}

	close(fd);
	return true;
}
void CFrameBuffer::loadPal(string filename, unsigned char offset, unsigned char endidx )
{
	if (!active)
		return;

	int fd;
	struct rgbData rgbdata;
	filename = iconBasePath + filename;

	fd = open(filename.c_str(), O_RDONLY );

	if (fd == -1)
	{
		printf("error while loading palette: %s", filename.c_str() );
		return;
	}

	int pos = 0;
	int readb = read(fd, &rgbdata,  sizeof(rgbdata) );
	while(readb)
	{
		__u32 rgb = (rgbdata.r<<16) | (rgbdata.g<<8) | (rgbdata.b);
		int colpos = offset+pos;
		if( colpos>endidx)
			break;
		paletteSetColor(colpos, rgb ,0);
		readb = read(fd, &rgbdata,  sizeof(rgbdata) );
		pos++;
	}
	paletteSet();
	close(fd);
}

void CFrameBuffer::paintPixel(int x, int y, unsigned char col)
{
	if (!active)
		return;

	unsigned char* pos = lfb + x + stride*y;
	*pos = col;
}

void CFrameBuffer::paintLine(int xa, int ya, int xb, int yb, unsigned char col)
{
	if (!active)
		return;

	int dx = abs (xa - xb);
	int dy = abs (ya - yb);
	int x;
	int y;
	int End;
	int step;

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

		paintPixel (x, y, col);

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
			paintPixel (x, y, col);
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

		paintPixel (x, y, col);

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
			paintPixel (x, y, col);
		}
	}
}

void CFrameBuffer::setBackgroundColor(int color)
{
	backgroundColor = color;
}

bool CFrameBuffer::loadPicture2Mem(string filename, unsigned char* memp)
{
	short width, height;
	unsigned char tr;
	int fd;
	filename = iconBasePath + filename;

	fd = open(filename.c_str(), O_RDONLY );

	if (fd==-1)
	{
		printf("error while loading icon: %s", filename.c_str() );
		return false;
	}

	read(fd, &width,  2 );
	read(fd, &height, 2 );
	read(fd, &tr, 1 );

	width= ((width & 0xff00) >> 8) | ((width & 0x00ff) << 8);
	height=((height & 0xff00) >> 8) | ((height & 0x00ff) << 8);

	read(fd, memp, width*height);

	close(fd);
	return true;
}

bool CFrameBuffer::savePictureFromMem(string filename, unsigned char* memp)
{
	short width = 720;
	short height = 576;
	unsigned char tr = 0;
	int fd;
	filename = iconBasePath + filename;

	fd = open(filename.c_str(), O_WRONLY | O_CREAT);

	if (fd==-1)
	{
		printf("error while saving icon: %s", filename.c_str() );
		return false;
	}

	width= ((width & 0xff00) >> 8) | ((width & 0x00ff) << 8);
	height=((height & 0xff00) >> 8) | ((height & 0x00ff) << 8);

	write(fd, &width,  2 );
	write(fd, &height, 2 );
	write(fd, &tr, 1 );

	write(fd, memp, 720*576);

	close(fd);
	return true;
}

bool CFrameBuffer::loadBackground(string filename, unsigned char col)
{
	if(backgroundFilename==filename)
	{
		//ist bereits geladen
		return true;
	}

	if(background)
	{
		delete[] background;
	}

	short width, height;
	unsigned char tr;
	int fd;
	filename = iconBasePath + filename;

	fd = open(filename.c_str(), O_RDONLY );

	if (fd==-1)
	{
		printf("error while loading icon: %s", filename.c_str() );
		return false;
	}

	read(fd, &width,  2 );
	read(fd, &height, 2 );
	read(fd, &tr, 1 );

	width= ((width & 0xff00) >> 8) | ((width & 0x00ff) << 8);
	height=((height & 0xff00) >> 8) | ((height & 0x00ff) << 8);

	if((width!=720) || (height!=576))
	{
		printf("error while loading icon: %s - invalid resulution = %dx%d", filename.c_str(), width, height );
		return false;
	}

	background = new unsigned char[720*576];

	read(fd, background, 720*576);

	if(col!=0)//pic-offset
	{
		unsigned char *bpos = background;
		int pos = 720*576;
		while(pos>0)
		{
			*bpos += col;
			bpos += 1;
			pos--;
		}
	}
	backgroundFilename = filename;
	close(fd);
	return false;
}

void CFrameBuffer::useBackground(bool ub)
{
	useBackgroundPaint = ub;
}

void CFrameBuffer::paintBackgroundBox(int xa, int ya, int xb, int yb)
{
	int dx = xb - xa;
	int dy = yb - ya;
	paintBackgroundBoxRel(xa, ya, dx, dy);
}

void CFrameBuffer::paintBackgroundBoxRel(int x, int y, int dx, int dy)
{
	if (!active)
		return;

	if(!useBackgroundPaint)
	{
		paintBoxRel(x, y, dx, dy, backgroundColor);
	}
	else
	{
		unsigned char *fbpos = lfb + x + stride*y;
		unsigned char *bkpos = background + x + stride*y;
		for(int count=0;count<dy;count++)
		{
			memcpy(fbpos, bkpos, dx);
			fbpos += stride;
			bkpos += stride;
		}
	}
}

void CFrameBuffer::paintBackgroundBoxRel(CPoint origin, CDimension dimension)
{
	paintBackgroundBoxRel( origin.getXPos(), origin.getYPos(), dimension.getWidth(), dimension.getHeight() );
}

void CFrameBuffer::paintBackground()
{
	if (!active)
		return;

	if(!useBackgroundPaint)
	{
		memset(lfb, 255, stride*576);
	}
	else
	{
		memcpy(lfb, background, stride*576);
	}
}

void CFrameBuffer::SaveScreen(int x, int y, int dx, int dy, unsigned char* memp)
{
	if (!active)
		return;

    unsigned char *fbpos = lfb + x + stride*y;
	unsigned char *bkpos = memp;
	for(int count=0;count<dy;count++)
	{
		memcpy(bkpos, fbpos, dx);
		fbpos += stride;
		bkpos += dx;
	}

}

void CFrameBuffer::RestoreScreen(int x, int y, int dx, int dy, unsigned char* memp)
{
	if (!active)
		return;

	unsigned char *fbpos = lfb + x + stride*y;
	unsigned char *bkpos = memp;
	for(int count=0;count<dy;count++)
	{
		memcpy(fbpos, bkpos, dx);
		fbpos += stride;
		bkpos += dx;
	}
}

void CFrameBuffer::switch_signal (int signal)
{
	CFrameBuffer * thiz = CFrameBuffer::getInstance();
	if (signal == SIGUSR1) {
		ioctl(thiz->tty, VT_RELDISP, 1);
		thiz->active = false;
		printf ("release display\n");
	}
	else if (signal == SIGUSR2) {
		ioctl(thiz->tty, VT_RELDISP, VT_ACKACQ);
		thiz->active = true;
		printf ("ackquire display\n");
		thiz->paletteSet();
		memset(thiz->lfb, 0, thiz->stride*thiz->yRes);
	}
}
