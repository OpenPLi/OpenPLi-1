#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <errno.h>
#include <string>

#define FB_DEV "/dev/fb/0"

int fd, available;
unsigned int sav_x, sav_y, sav_bpp, xRes, yRes, stride, bpp;
struct fb_var_screeninfo screeninfo;
unsigned char *lfb;
fb_fix_screeninfo fix;
fb_cmap cmap;
__u16 red[256], green[256], blue[256], trans[256];

//{
//        __u32 start;                    /* First entry  */
//        __u32 len;                      /* Number of entries */
//        __u16 *red;                     /* Red values   */
//        __u16 *green;
//        __u16 *blue;
//        __u16 *transp;                  /* transparency, can be NULL */
//};


void view_screeninfo(struct fb_var_screeninfo screeninfo, fb_fix_screeninfo fix);
int SetMode(unsigned int nxRes, unsigned int nyRes, unsigned int nbpp);


int main(int argc, char ** argv)
{
	available = 0;
	unsigned *buf;
	unsigned *prev;
	int i, j;
		
	fd = open(FB_DEV, O_RDWR);
	if (fd < 0)
	{
		perror(FB_DEV);
		exit(1);
	}

	// CURRENT SETTINGS
	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo) < 0)
	{
		perror("FBIOGET_VSCREENINFO");
		exit(1);
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix) < 0)
	{
		perror("FBIOGET_FSCREENINFO");
		exit(1);
	}
	view_screeninfo(screeninfo, fix);

	close(fd);

}

void view_screeninfo(struct fb_var_screeninfo screeninfo, fb_fix_screeninfo fix)
{
	printf("[Framebuffer] %dk video memory 0x%x\n", fix.smem_len/1024, fix.smem_len);
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

        printf("[Framebuffer] stride %d\n", fix.line_length);                   /* angle we rotate counter clockwise */
        printf("[Framebuffer] id ");
	for (int i = 0; i < 16 && fix.id[i] != 0; i++)
            putchar(fix.id[i]);                    /* identification string eg "TT Builtin" */
	putchar('\n');
        printf("[Framebuffer] smem_start 0x%lx\n", fix.smem_start);        /* Start of frame buffer mem (physical address) */
        printf("[Framebuffer] smem_len 0x%x\n", fix.smem_len);                 /* Length of frame buffer mem */
        printf("[Framebuffer] type %u\n", fix.type);                     /* see FB_TYPE_*                */
        printf("[Framebuffer] interleave %u\n", fix.type_aux);                 /* Interleave for interleaved Planes */
        printf("[Framebuffer] visual %u\n", fix.visual);                   /* see FB_VISUAL_*              */
        printf("[Framebuffer] xpanstep %d\n", fix.xpanstep);                 /* zero if no hardware panning  */
        printf("[Framebuffer] ypanstep %d\n", fix.ypanstep);                 /* zero if no hardware panning  */
        printf("[Framebuffer] ywrapstep %d\n", fix.ywrapstep);                /* zero if no hardware ywrap    */
        printf("[Framebuffer[ mmio_start 0x%lx\n", fix.mmio_start);       /* Start of Memory Mapped I/O   (physical address) */
        printf("[Framebuffer] mmio_len 0x%x\n", fix.mmio_len);                 /* Length of Memory Mapped I/O  */
        printf("[Framebuffer] accel %d\n",  fix.accel);                    /* Indicate to driver which     */

	printf("[Framebuffer] available.\n");

}


int SetMode(unsigned int nxRes, unsigned int nyRes, unsigned int nbpp)
{
        screeninfo.xres_virtual=screeninfo.xres=nxRes;
        screeninfo.yres_virtual=screeninfo.yres=nyRes;
        screeninfo.height=0;
        screeninfo.width=0;
        screeninfo.xoffset=screeninfo.yoffset=0;
        screeninfo.bits_per_pixel=nbpp;
        if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0)
        {
                perror("FBIOPUT_VSCREENINFO");
                printf("fb failed\n");
                return -1;
        }
        if ((screeninfo.xres!=nxRes) && (screeninfo.yres!=nyRes) && (screeninfo.bits_per_pixel!=nbpp))
        {
                printf("SetMode failed: wanted: %dx%dx%d, got %dx%dx%d",
                        nxRes, nyRes, nbpp,
                        screeninfo.xres, screeninfo.yres, screeninfo.bits_per_pixel);
        }
        xRes=screeninfo.xres;
        yRes=screeninfo.yres;
        bpp=screeninfo.bits_per_pixel;
        // fb_fix_screeninfo fix;
        if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
        {
                perror("FBIOGET_FSCREENINFO");
                printf("fb failed\n");
        }
        stride=fix.line_length;
	//printf("Clearing fb\n");
        //memset(lfb, 0, stride*yRes);
        return 0;
}

