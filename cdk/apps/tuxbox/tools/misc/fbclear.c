/*
 * fbclear.c - clear the f***ing fb
 */

#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define FBDEV "/dev/fb/0"

static int colortable(int fd)
{
	unsigned short red[256];
	unsigned short green[256];
	unsigned short blue[256];
	unsigned short transp[256];
	struct fb_cmap colormap;
	int count;

	colormap.red = red;
	colormap.green = green;
	colormap.blue = blue;
	colormap.transp = transp;
	colormap.start = 0;
	colormap.len = 256;

	for (count = 0; count < 256; count++) {
		switch (count & 3) {
		case 0:
			transp[count] = 0x0000;
			break;
		case 1:
			transp[count] = 0xFFFF;
			break;
		default:
			transp[count] = 0x1FFF;
			break;
		}

		red[count] = (count & 0xc0) << 8;
		green[count] = (count & 0x30) << 10;
		blue[count] = (count & 0x0c) << 12;
	}

	if (ioctl(fd, FBIOPUTCMAP, &colormap) < 0) {
		perror("FBIOPUTCMAP");
		return -1;
	}

	return 0;
}

int main(void)
{
	int fd;
	struct fb_fix_screeninfo fb_fix;
	char *mapped;

	if ((fd = open(FBDEV, O_RDWR)) < 0) {
		perror(FBDEV);
		return 1;
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix) < 0) {
		perror("FBIOGET_FSCREENINFO");
		return 1;
	}

	if (colortable(fd) < 0)
		return 1;

	mapped = mmap(0, fb_fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (!mapped) {
		perror("mmap");
		return 1;
	}

	memset(mapped, 1, fb_fix.smem_len);

	return 0;
}

