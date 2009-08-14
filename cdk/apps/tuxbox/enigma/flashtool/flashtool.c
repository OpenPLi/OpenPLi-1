#include <unistd.h>
#include <sys/mman.h>
#include <sys/reboot.h>
#include <linux/version.h>
#include <linux/fb.h>
#include <linux/vt.h>
#include <linux/kd.h>

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>
#include <stdint.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7)
#include <linux/compiler.h>
#include <mtd/mtd-user.h>
#else
#include <linux/mtd/mtd.h>
#endif

int main( int argc, char **argv)
{
	struct fb_var_screeninfo screeninfo, oldscreen;
	struct fb_fix_screeninfo fix;
	struct vt_mode mode, vt_mode;
	struct fb_cmap cmap;
	mtd_info_t meminfo;
	int fbdev=-1, fd=-1, fd2=-1, fd3=-1, lfb=0, tty=-1, kd_mode=0, imagesize=0;
	char mtd[128];

	if ((fbdev = open("/dev/fb/0", O_RDWR))<0)
	{
		perror("open fb");
		return -1;
	}

	if (ioctl(fbdev, FBIOGET_VSCREENINFO, &oldscreen)<0)
	{
		perror("FBIOGET_VSCREENINFO");
		return -1;
	}

	memcpy(&screeninfo, &oldscreen, sizeof(struct fb_var_screeninfo));

	screeninfo.xres_virtual=720;
	screeninfo.yres_virtual=576;
	screeninfo.height=0;
	screeninfo.width=0;
	screeninfo.xoffset=screeninfo.yoffset=0;
	screeninfo.bits_per_pixel=8;
	if (ioctl(fbdev, FBIOPUT_VSCREENINFO, &screeninfo)<0)
	{
		perror("FBIOPUT_VSCREENINFO");
		return -1;
	}

	if (ioctl(fbdev, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("FBIOGET_FSCREENINFO");
		return -1;
	}

	lfb=(struct fb_pixel_t*)mmap(0, fix.smem_len, PROT_WRITE|PROT_READ, MAP_SHARED, fbdev, 0);
	if (!lfb)
	{
		perror("mmap");
		return -1;
	}

	memset(lfb, 0, 720*576);

	if ((tty=open("/dev/vc/0", O_RDWR))<0)
	{
		perror("open (tty)");
		return -1;
	}

	if (-1 == ioctl(tty,KDGETMODE, &kd_mode)) { perror("ioctl KDGETMODE"); return -1; }
	if (-1 == ioctl(tty,VT_GETMODE, &vt_mode)) { perror("ioctl VT_GETMODE"); return -1; }
	if (-1 == ioctl(tty,VT_GETMODE, &mode)) { perror("ioctl VT_GETMODE"); return -1; }
	if (-1 == ioctl(tty,VT_SETMODE, &mode)) { perror("[ioctl VT_SETMODE"); return -1; }
	if (-1 == ioctl(tty,KDSETMODE, KD_GRAPHICS)) { perror("ioctl KDSETMODE"); return -1; }

	if ((fd=open("/tmp/mtd.txt", O_RDONLY)) < 0)
	{
		perror("open /tmp/mtd.txt");
		return -1;
	}
	mtd[read(fd, mtd, 256)]=0;
	close(fd);
	fd=-1;

	fd = open("/tmp/cmap", O_RDONLY);
	if ( fd < 0 )
	{
		perror("open /tmp/cmap");
		return -1;
	}

	read(fd, &cmap.start, sizeof(cmap.start));
	read(fd, &cmap.len, sizeof(cmap.len));

	cmap.red = malloc(cmap.len*sizeof(__u16));
	cmap.green = malloc(cmap.len*sizeof(__u16));
	cmap.blue = malloc(cmap.len*sizeof(__u16));
	cmap.transp = malloc(cmap.len*sizeof(__u16));

	read(fd, cmap.red, cmap.len*sizeof(__u16));
	read(fd, cmap.green, cmap.len*sizeof(__u16));
	read(fd, cmap.blue, cmap.len*sizeof(__u16));
	read(fd, cmap.transp, cmap.len*sizeof(__u16));
	close(fd);
	fd = -1;

	fd = open("/tmp/root.cramfs", O_RDONLY);
	if ( fd < 0 )
	{
		perror("open image file");
		return -1;
	}

	imagesize = lseek( fd, 0, SEEK_END);
	lseek( fd, 0, SEEK_SET);

	if ( ioctl(fbdev, FBIOPUTCMAP, &cmap) < 0 )
	{	
		perror("FBIOPUTCMAP");
		return -1;
	}
	free(cmap.red);
	free(cmap.green);
	free(cmap.blue);
	free(cmap.transp);

	{
		int fillPosX=0, fillPosY=0, fillWidth=0, fillHeight=0, fillColor=0, filesize=0, fsize=0;
		erase_info_t erase;

		if( (fd2 = open(mtd, O_RDWR )) < 0 )
		{
			perror("open mtd");
			return -1;
		}

		if( ioctl( fd2, MEMGETINFO, &meminfo ) < 0 )
		{
			perror("MTD MEMGETINFO");
			return -1;
		}

		if (imagesize > meminfo.size)
		{
			/* flashing a large image into a 'maxvar' partition layout, we cannot keep the jffs2 partition */
			::close(fd2);
			if (fd2 = open("/dev/mtd/3", O_RDWR)) < 0)
			{
				perror("open mtd");
				return -1;
			}
			if( ioctl( fd2, MEMGETINFO, &meminfo ) < 0 )
			{
				perror("MTD MEMGETINFO");
				return -1;
			}
		}
		else if (meminfo.size == 0x600000 && imagesize <= 0x500000)
		{
			/* flashing a maxvar image into a standard partition layout */
			/* TODO: try to keep (move) the existing jffs2 partition */
			::close(fd2);
			if (fd2 = open("/dev/mtd/3", O_RDWR)) < 0)
			{
				perror("open mtd");
				return -1;
			}
			if( ioctl( fd2, MEMGETINFO, &meminfo ) < 0 )
			{
				perror("MTD MEMGETINFO");
				return -1;
			}
		}

		if ( (fd3 = open("/tmp/update1.raw", O_RDONLY)) < 0 )
		{
			perror("open /tmp/update1.raw");
			return -1;
		}

		read(fd3, &fillPosX, sizeof(int));
		read(fd3, &fillPosY, sizeof(int));
		read(fd3, &fillWidth, sizeof(int));
		read(fd3, &fillHeight, sizeof(int));
		read(fd3, &fillColor, sizeof(int));
		if ( read(fd3, lfb, 720*576) != 720*576 )
		{
			perror("read /tmp/update1.raw failed");
			return -1;
		}
		close(fd3);
		fd3=-1;

//		printf("erasing now...");

		erase.length = meminfo.erasesize;
		for (erase.start = 0; erase.start <= meminfo.size; erase.start += meminfo.erasesize )
		{
			int y=fillPosY;
			int perc = erase.start*100/meminfo.size;
			int part = fillWidth*100;

//			printf("\rErasing %u Kibyte @ %x -- %2u %% complete",
//				meminfo.erasesize/1024, erase.start,
//				perc );
//			fflush(stdout);

			for (; y<fillPosY+fillHeight; y++)
				memset(((__u8*)lfb)+y*fix.line_length+fillPosX, fillColor, perc*part/10000 );

			if(erase.start < meminfo.size && ioctl( fd2, MEMERASE, &erase) != 0)
			{
				perror("Flash Erase failed");
				return -1;
			}
		}
	}

	{
		int fillPosX=0, fillPosY=0, fillWidth=0, fillHeight=0, fillColor=0, fsize=imagesize, rbytes=0;
		char buf[meminfo.erasesize];

		if ( (fd3 = open("/tmp/update2.raw", O_RDONLY)) < 0 )
		{
			perror("open /tmp/update2.raw");
			return -1;
		}

		read(fd3, &fillPosX, sizeof(int));
		read(fd3, &fillPosY, sizeof(int));
		read(fd3, &fillWidth, sizeof(int));
		read(fd3, &fillHeight, sizeof(int));
		read(fd3, &fillColor, sizeof(int));
		if ( read(fd3, lfb, 720*576) != 720*576 )
		{
			perror("read /tmp/update2.raw failed");
			return -1;
		}
		close(fd3);
		fd3=-1;

//		printf("flashing now...\n");

		while( ( rbytes = read( fd, buf, sizeof(buf) ) ) )
		{
			int y=fillPosY;
			int part = fillWidth*100;
			int perc = (imagesize-fsize)*100/imagesize;

//			printf("\rWriting %u Kibyte @ %x -- %2u %% complete",
//				rbytes/1024, imagesize-fsize,
//				perc );
//			fflush(stdout);

			fsize -= write( fd2, buf, rbytes );
			for (; y<fillPosY+fillHeight; y++)
				memset(((__u8*)lfb)+y*fix.line_length+fillPosX, fillColor, perc*part/10000 );
		}
		memset(lfb, 0, 720*576);
		reboot(RB_AUTOBOOT);
	}

	return 0;
}
