#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <errno.h>
#include <string>

#define FB_DEV "/dev/fb/0"

int fd, available;
unsigned int xRes, yRes, stride, bpp;
unsigned long x_stride;
struct fb_var_screeninfo screeninfo;
unsigned char *lfb;
fb_fix_screeninfo fix;

/* crypting 1 226:
This is the PLi Framebuffer clear tool. 
This is to overcome a bug in head.ko that forgets to clear the frame buffer at startup.
It can be use in the boot process to clear the framebuffer as soon as head.ko is loaded.

*/
const char *herald =
    "\x4e\x09\xeb\xd2\xd0\x10\x0e\x32\x5e\xe9\xe4\x9a\x0c\x68\x81\x64\x49\x69\x61\x49"
    "\x7b\x75\xd7\x93\x2a\x42\x70\x96\x53\x89\xbf\x6e\xc3\x0c\x78\x4b\x5e\x22\xc3\x00"
    "\x7e\x9d\xe4\x3b\xdc\x48\xaf\x6c\x45\x3d\x66\x3b\x4e\x21\xaa\x50\x59\xe0\xb0\x93"
    "\x98\x1c\xbc\x4c\xf9\x28\x4f\x06\x6c\x2d\x73\x00\x41\xb6\x85\xf6\x26\x7d\xf5\x63"
    "\xe5\x4e\x6a\x44\x1f\x79\x51\x72\x6e\x90\xc1\x7b\xd4\xbc\x6a\x6f\xd4\x8c\x63\xc0"
    "\x05\x2f\x2e\x10\x4a\xe5\x36\x5e\x62\x62\xd6\x72\xd1\x3b\x62\x94\xd3\x68\xfb\x2d"
    "\x69\xf4\x35\xcd\xfa\x32\x4b\x6e\xd5\x51\x77\x23\xb4\x69\x94\x86\x65\xb0\x96\xe1"
    "\xd5\x55\x32\x64\xa2\x7b\x70\xa0\xea\x25\x4b\x50\x62\x3a\x63\x5b\x2d\x1c\xa5\x66"
    "\xc5\x1b\x69\x12\x64\x60\x04\xf1\x71\x63\x62\xbb\xe7\x90\x75\x2a\x14\xde\x78\xdf"
    "\x0b\x68\x58\x1f\x67\x50\x7f\xba\x8d\x1c\x40\x62\xfe\x32\x2a\xff\x04\x2a\x9c\xd0"
    "\x05\x08\x5a\x27\xc0\x94\xda\x5f\xc8\x0a\x60\xca\x0c\x61\x9c\x49\x71\xc6\xd3\x3e"
    "\x65\x80\x01\x2c\xe9\xb9\x60\xa0\x97\x74\x7c\xc3\x2e\xb0\xa0\x9b\xfb\x14\xec\x70"
    "\xbf\x49\x63\xef\x00\x21\x6c\xc0\x6f\x4e\x2f\x17\x31\xb6\xa0\x7f\x45\x5a\x64\x60"
    "\x3a\x70\x76\x92\x69\xeb\xe3\xd1\x64\x9f\x00\x61\xed\x36\x60\x7d\xdd\x63\x16\xc5"
    "\x24\xf2\x30\x28\x9f\xf1\xcd\x6a\x20\x23\x29\x7e\x9f\x63\x56\x02\x78\x8c\x65\xfd"
    "\xd1\x57\xbb\x69\x90\xf2\x29\x1f\x1b\x64\xf9\xc9\x65\x34\x2e\xa9\x6d\xf0\x62\x25"
    "\x88\x96\x6d\xcd\x83\x61\x35\xc5\x6c\x14\xb1\x24\x86\x64\x74\x61\xde\x76\x2f\x69"
    "\x9a\x60\x7c\x7b\x6f\x04\x8f\x08\x19\xb0\xc9\x23\x84\x3b\x7f\xa5\x63\x60\x75\xb8"
    "\x62\xfa\x51\x70\xcd\xd4\x07\x7e\x30\x26\x25\xbe\x87\x6d\x83\x76\x69\x37\xdf\xbc"
    "\xcb\x72\xbf\x72\x07\xa8\x68\x3b\x5c\x6b\xc8\x75\x79\x62\x34\xc6\xb1\x33\xe7\x78"
    "\x60\xf8\x28\x8a\x24\x74\x9c\x84\x69\x4e\xaf\xfb\x89\x20\x42\x26\x03\x50\x65\x7a"
    "\x92\x6b\xb2\x6c\x6d\xf9\x95\xb1\xda\xf1\x26\x62\x52\x07\x71\xf0\xfc\x20\x49\xc0"
    "\x7f\xf9\x44\xa8\x4f\x58\xab\x62\xe5\xe1\x65\xd1\x7d\x2f\x74\xd0\x63\xea\x36\x7b"
    "\x11\x52\x1c\x7b\x11\x76\x61\x87\x22\x67\x77\xdd\x63\xb8\xd5\x7a\x7c\xf0\xa9\x29"
    "\x22\x97\x66\xdf\xc6\x7d\xfa\x25\x62\x2a\xe6\x40\x5a\xf6\x40\x62\x65\x5e\x6b\x55"
    "\x02\x7e\x5a\x02\x2f\x2d\x30\xa0\x06\x11\x73\x61\xe4\xc9\x7f\xfd\xb2\x20\x9d\x90"
    "\x72\xef\x43\xa5\x87\xd4\xaa\x79\xd1\xe0\x69\xd4\x1f\x76\xaf\x12\x7c\x22\x64\x0e"
    "\x3d\x85\x52\x76\x80\x36\x7e\x8f\x24\x23\xdb\x0e\x05\x7f\xaa\xbe\xfd\xd9\xd6\x45"
    "\xc4\xd3\x77\x73\xfb\x25\x29\x10\x63\x3e\xc3\x87\xb5\x81\x02\x6b\xce\x12\x67\xaf"
    "\x31\x2e\xf3\x60\x66\x82\x42\xcf\x8e\x15\xfa\x62\x60\xf2\x29\xdf\xb9\x7f\x93\xf5"
    "\x7a\xec\xd3\xd8\xcd\xc5\xe3\x64\x50\xae\x25\xef\x9f\x61\x46\x19\x6d\x1b\xfe\xa8"
    "\xee\x50\xb0\x29\x64\x4a\x71\xae\x73\x68\x90\xb8\x6e\x98\x55\x1a\x0c\xd0\xd9\x2c"
    "\x52\x3d\x6d\x60\x5c\x68\xba\x1f\x6b\x28\x8f\x60\xe3\x74\x70\x75\x90\x24\x2b\x62"
    "\x07\x79\xeb\x70\x71\x15\xc2\x4a\xe5\x3f\x29\x6c\x73\xd0\x64\xc4\x59\x6a\x8a\x15"
    "\x71\x3f\x73\xda\x7a\x63\x2c\x7d\x10\x8e\x21\x07\xfe\x71\xc8\x94\x60\x26\x7f\xa3"
    "\x9d\xd0\x72\x23\xc3\xf6\x65\x27\x86\x6c\x34\xfc\x67\x18\xc5\xd2\x06\x81\x28\x63"
    "\x32\xbf\x7c\x06\xca\x2b\x63\xd0\x7a\x0e\xa4\xb0\x98\x08\x64\x6c\x05\xf5\x68\x91"
    "\x0f\x27\x43\x60\x6c\x6b\x16\x3a\x39\x42\x3f\x7f\x41\x83\x62\x58\x87\x64\x25\xcd"
    "\x6d\x25\xe5\x89\x4e\xf2\xa0\x66\xd5\x1b\x79\xa8\xde\x60\x9e\x46\x6f\x57\xf6\xc8"
    "\xde\xc5\xf5\x60\x32\xb6\x75\x71\xcc\x2b\xc1\xf0\x6d\x66\xc1\x71\x81\x83\xdc\x7a"
    "\xf0\x6c\x23\xb3\xa7\x7b\xee\xf3\x63\x9c\x2f\xfe\x9a\x3f\x5d\x6f\xfe\xb3\x60\x2c"
    "\x9b\x27\x0a\x60\x67\x80\x91\xaa\x2e\xb3\x15\x7b\x40\xd1\x26\x23\xdf\x66\x4c\xb8"
    "\x63\x81\x65\xe1\xb4\x51\xfb\x60\x04\xa0\x61\x56\xf9\x23\x18\x5e\x6f\x8a\xdb\x57"
    "\xd8\x5f\x7b\x67\x40\x1c\x26\xf2\x9a\x6e\x60\xd9\x70\x08\xc3\x27\x0d\x60\x20\x2f"
    "\x3c\x1d\x65\xda\x76\x64\xce\xdf\x61\xeb\x61\x0d\x1a\x84\xb6\x65\x35\x48\x69\x03"
    "\x70\x62\x87\xd4\x2a\xf8\x6e\xee\x7c\x6a\xa7\x07\x36\x26\x33\xb1";

/* crypting 3 39:
Oops, I failed to start on this system

*/
const char *sorry =
    "\x98\x4b\x5d\x3e\xf0\x70\x82\x57\xef\xce\x48\xc9\x8c\x69\x55\xdf\x73\x23\x90\xec"
    "\x77\x53\x5e\x71\x8c\x82\x2f\xd6\xf3\x22\xda\x60\x46\x67\xf9\xb7\x9b\x70\x46\x2d"
    "\x26\x1f\x6e\x14\xf2\x67\x83\x11\x6e\x69\x59\x38\xf6\x3c\x76\x6c\xa5\xb5\x62\x96"
    "\xca\x67\xb4\x04\x20\x99\x90\xed\x7b\x34\x06\x73\x9f\x01\x63\xf0\x61\x2a\x5f\x20"
    "\x75\x7e\x93\x7a\x08\x24\xa9\x72\x51\x60\x60\xbc\xdd\x7d\x5c\xb2\x74\xbe\x44\xa5"
    "\xfb\x70\xf9\x29\x1f\x4c\x64\xe1\x35\x6c\xbf\x1e\x23\x2c\x70\xa3\x94\xc4\x47\x71"
    "\x58\x58\x68\x92\x0a\x6e\x10\x69\x72\x8a\x23\x58\x60\x40\x78\x22\xd3\x97\x7d\x2d"
    "\xa1\x71\x88\x19\x7d\x83\x03\x15\x55\x74\xcf\x7c\x05\x72\x6d\x3e\xe4\x6b\xfd\x9d"
    "\x07\x58\x8a\x63\x2a\xf7\x42\x64";

void fuzz(const char * buf)
{
	unsigned int si, i, j, len, val;


	si = (buf[3] & 0x30) >> 4;
        len = ((buf[4] & 0x0F) << 12) | ((buf[5] & 0x0F) << 8) | ((buf[6] & 0x0F) << 4) | (buf[7] & 0x0F);

	// printf("startindex = %d (%0x), len = %d\n", startindex, buf[3], len);

	if ((si+len) % 4 != (buf[len*4+8+3] & 0x30) >> 4)
	{
		fprintf(stderr, "invalid string\n");
		return;
	}

	for (i = 0; i < len; i++)
	{
		j = 8 + 4*i;
		val = (buf[j + 3 - si] & 0x0F) | (buf[j + 3 - ((si+2)%4)] & 0xF0);
		putchar(val);
		si = (si+1)%4;
	}
}

void prfuzz(const char *buf)
{
	fuzz(buf);
}


int main(int argc, char ** argv)
{
	available = 0;
        int fake = 0;
        int verbose = 0;
	char * progname;

	if (argv && argv[1])
	{
		if (argv[1][0] == '-' && argv[1][1] == 'd')
		{
			fake = 1;
			verbose = 1;
		}
		else if (argv[1][0] == '-' && argv[1][1] == 'v')
			verbose = 1;
	}
		
	prfuzz(herald);

	progname = argv[0];
	if (strchr(progname, '/'))
	{
		progname = strrchr(progname, '/');
		progname++;
	}
	if (strlen(progname) != 8)
	{
		putchar('\n');
		prfuzz(sorry);
		exit(1);
	}
	if (
		progname[0] - 0x12 != 0x5e  || // 70 p
		progname[5] + 0x31 != 0x9d  || // 6c l
		progname[1] - 0x60 != 0x0c  || // 6c l
		progname[3] - 0x4d != 0x12  || // 5f _
		progname[4] - 0x16 != 0x4d  || // 63 c
		progname[6] - 0x25 != 0x41  || // 66 f
		progname[7] + 0x67 != 0xc9  || // 62 b
		progname[2] + 0x85 != 0xee  || // 69 i
		progname[8] != 0 )
	{
		putchar('\n');
		prfuzz(sorry);
		exit(2);
	}

	fd = open(FB_DEV, O_RDWR);
	if (fd < 0)
	{
		perror(FB_DEV);
		exit(1);
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("[Framebuffer] <FBIOGET_FSCREENINFO>");
		exit(1);
	}
	available = fix.smem_len;

	if (!fake)
	{
		lfb = (unsigned char*)mmap(0, available, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
		if (!lfb)
		{
			perror("mmap");
			printf("[Framebuffer] <not available>\n");
			exit(1);
		}

		// clear whole framebuffer
		memset(lfb, 0, available);
		munmap(lfb, available);
	}

	if (!verbose)
		exit(0);

	// Show meta info about frambuffer
	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo)<0)
	{
		perror("FBIOGET_VSCREENINFO");
		exit(1);
	}

	stride=fix.line_length;
	x_stride = (fix.line_length*8)/screeninfo.bits_per_pixel;

	xRes = 720;
	yRes = 576;
	printf("[Framebuffer] %dk video memory\n", available/1024);
        printf("[Framebuffer] xres %d\n", screeninfo.xres);                     /* visible resolution           */
        printf("[Framebuffer] yres %d\n", screeninfo.yres);
        printf("[Framebuffer] xres_virtual %d\n", screeninfo.xres_virtual);             /* virtual resolution           */
        printf("[Framebuffer] xres_virtual %d\n", screeninfo.yres_virtual);
        printf("[Framebuffer] xoffset %d\n", screeninfo.xoffset);                  /* offset from virtual to visible */
        printf("[Framebuffer] xoffset %d\n", screeninfo.yoffset);                  /* resolution                   */

        printf("[Framebuffer] bits_per_pixel %d\n", screeninfo.bits_per_pixel);           /* guess what                   */
        printf("[Framebuffer] greyscale %d\n", screeninfo.grayscale);                /* != 0 Graylevels instead of colors */

        //struct fb_bitfield red;         /* bitfield in fb mem if true color, */
        //struct fb_bitfield green;       /* else only length is significant */
        //struct fb_bitfield blue;
        //struct fb_bitfield transp;      /* transparency                 */

        printf("[Framebuffer] nonstd %d\n", screeninfo.nonstd);                   /* != 0 Non standard pixel format */

        printf("[Framebuffer] activate %d\n", screeninfo.activate);                 /* see FB_ACTIVATE_*            */

        printf("[Framebuffer] height %d mm\n", screeninfo.height);                   /* height of picture in mm    */
        printf("[Framebuffer] width %d mm\n", screeninfo.width);                    /* width of picture in mm     */

        /* Timing: All values in pixclocks, except pixclock (of course) */
        printf("[Framebuffer] pixclock %d\n", screeninfo.pixclock);                 /* pixel clock in ps (pico seconds) */
        printf("[Framebuffer] left_margin %d\n", screeninfo.left_margin);              /* time from sync to picture    */
        printf("[Framebuffer] right_margin %d\n", screeninfo.right_margin);             /* time from picture to sync    */
        printf("[Framebuffer] upper_margin %d\n", screeninfo.upper_margin);             /* time from sync to picture    */
        printf("[Framebuffer] lower_margin %d\n", screeninfo.lower_margin);
        printf("[Framebuffer] hsync_len %d\n", screeninfo.hsync_len);                /* length of horizontal sync    */
        printf("[Framebuffer] vsync_len %d\n", screeninfo.vsync_len);                /* length of vertical sync      */
        printf("[Framebuffer] sync %d\n", screeninfo.sync);                     /* see FB_SYNC_*                */
        printf("[Framebuffer] vmode %d\n", screeninfo.vmode);                    /* see FB_VMODE_*               */
        printf("[Framebuffer] rotate angle %d\n", screeninfo.rotate);                   /* angle we rotate counter clockwise */

        printf("[Framebuffer] stride %d\n", stride);                   /* angle we rotate counter clockwise */
        printf("[Framebuffer] xstride %ld\n", x_stride);                   /* angle we rotate counter clockwise */

	printf("[Framebuffer] available.\n");

#if 0
	screeninfo.xres_virtual=screeninfo.xres = xRes;
	screeninfo.yres_virtual=screeninfo.yres = yRes;
	screeninfo.bits_per_pixel = bpp;
	screeninfo.height = 0;
	screeninfo.width = 0;
	screeninfo.xoffset=screeninfo.yoffset = 0;
	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0)
	{
		perror("[FB-SET] <FBIOPUT_VSCREENINFO>");
		exit(1);
	}

	if ((screeninfo.xres!=xRes) && (screeninfo.yres!=yRes) && (screeninfo.bits_per_pixel!=bpp))
	{
		printf("[FB-SET] <SetMode failed: wanted: %dx%dx%d, got %dx%dx%d>\n", xRes, yRes, bpp,
			screeninfo.xres,screeninfo.yres, screeninfo.bits_per_pixel);
	}
	xRes=screeninfo.xres;
	yRes=screeninfo.yres;
	bpp=screeninfo.bits_per_pixel;
	printf("[Framebuffer] xRes:%d yRes:%d bpp:%d\n", xRes, yRes, bpp);
#endif

}
