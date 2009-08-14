/************************************************************************
 * ein weiteres wenig sinnvolles programm für die dbox2 (C) 2001 tmbinc *
 *                                                                      *
 * Dieses Programm unterliegt der GPL, also nix klauen ;o)              *
 * bzw. gerade klauen aber dann auch GPL. der groesste teil hier stammt *
 * von Ge0rg, oder anderen, der tolle intensity-algo aber von mir und   *
 * ueberhaupt und bla.                                                  *
 * change as you need.                                                  *
 ************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <dbox/lcd-ks0713.h>
#include <config.h>


#if HAVE_OLD_CAPTURE_API
#if HAVE_DVB_API_VERSION >= 3
 #include <linux/dvb/avia/avia_gt_capture.h>
#else
 #include <dbox/avia_gt_capture.h>
#endif
#else
#include <linux/videodev.h>
#define V4L2DEV "/dev/v4l/video0"
#endif

#include <rcinput.h>
#include <plugin.h>
extern	int	doexit;
extern	unsigned short	actcode;


typedef unsigned char screen_t[LCD_BUFFER_SIZE];

int lcd_fd;
int stride;

inline int compute(int l, int d)
{
	switch (d)
	{
	case 0:
		return l;
	case 1:
		return l+2;
	case 2:
		return l+8;
	case 3:
		return 0x1C;
	case 4:
		return 0x30;
	case 5:
		return 0x44;
	case 6:
		return 0x58;
	case 7:
		return 0x6c;
	case 8:
		return 0x80;
	case 9:
		return 0x94;
	case 10:
		return 0xa8;
	case 11:
		return 0xbc;
	case 12:
		return 0xd0;
	case 13:
		return 0xe4;
	case 14:
		return l-8;
	case 15:
		return l-2;
	default:
		return 0;
	}
}

#if HAVE_OLD_CAPTURE_API
#define XRES	120
#define YRES	65
//#define XRES	160
//#define YRES  72
#else
#define XRES	120	// smallest possible capture size
#define YRES 	72
#endif

void clr() {
	if (ioctl(lcd_fd,LCD_IOCTL_CLEAR) < 0) {
		perror("clr()");
		exit(1);
	}
}

void init() {
	doexit=0;

/*	int i;
	if ((lcd_fd = open("/dev/dbox/lcd0",O_RDWR)) < 0) {
		perror("open(/dev/dbox/lcd0)");
		exit(1);
	}*/
	clr();

/*	i = LCD_MODE_BIN;
	if (ioctl(lcd_fd,LCD_IOCTL_ASC_MODE,&i) < 0) {
		perror("init(LCD_MODE_BIN)");
		exit(1);
	}*/
}

void draw_screen(screen_t s) {
	write(lcd_fd, s, LCD_BUFFER_SIZE);
}

void draw_gol(unsigned char *src, screen_t s) 
{
	int x, y, yy, pix;

	for (y=0; y<LCD_ROWS; y++) {
		for (x=0; x<LCD_COLS; x++) {
			pix = 0;
			for (yy=0; yy<8; yy++) {
				pix = (pix<<1) + src[(8*y+yy)*LCD_COLS+x];
			}
			s[y*LCD_COLS+x] = pix;
		}
	}
}

void read_frame(unsigned char *out, int fd)
{
	unsigned short buffer[stride*YRES/2];
	int x, y, lum=0;
	read(fd, buffer, stride * YRES);
	for (y=0; y<YRES; y++)
		for (x=0; x<XRES/2; x++)
		{
			int val=buffer[y*stride/2+x];
			int dy[2]={val&0xF, (val>>8)&0xF};
			lum=compute(lum, dy[1]);
			out[y*XRES+x*2]=lum;
			lum=compute(lum, dy[0]);
			out[y*XRES+x*2+1]=lum;
		}
}

unsigned char image[(LCD_ROWS+1)*LCD_COLS*8], intensity[LCD_ROWS*LCD_COLS*8];

void *update_thread(void*dummy)
{
	unsigned char pic[XRES*YRES];	
	int x, y;

#if HAVE_OLD_CAPTURE_API
	int capture=open("/dev/dbox/capture0", O_RDONLY);
	capture_stop(capture);
	capture_set_input_pos(capture, 0, 0);
	capture_set_input_size(capture, 720, 576);
	capture_set_output_size(capture, XRES, YRES*2);
	stride = capture_start(capture);
	
	printf("Capture driver reports stride=%d\n", stride);
#else
	int capture=open(V4L2DEV, O_RDONLY);
	if (capture<0){
		printf ("outdoor: could not open v4l2-device\n");
		pthread_exit(NULL);
	}
	struct v4l2_crop crop;
	struct v4l2_format format;

	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl (capture, VIDIOC_G_CROP, &crop);
	crop.c.left = 0;
	crop.c.top = 0;
	crop.c.width = 720;
	crop.c.height = 576;
	ioctl (capture, VIDIOC_S_CROP, &crop);

	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl (capture, VIDIOC_G_FMT, &format);
	format.fmt.pix.width = XRES;
	format.fmt.pix.height = YRES;
	ioctl (capture, VIDIOC_S_FMT, &format);
	ioctl (capture, VIDIOC_G_FMT, &format);

	stride = format.fmt.pix.bytesperline;
	printf("V4L2 reports stride=%d\n", stride);
#endif
	
	while (!doexit)
	{
		read_frame(pic, capture);
		for (y=0; y<65; y++)
		{
			for (x=0; x<120; x++)
				image[LCD_COLS*y+x]=pic[y*XRES+x];
		}
	}
#if HAVE_OLD_CAPTURE_API
	capture_stop(capture);
#endif
	close(capture);
	pthread_exit(NULL);
}

int outdoor_exec()
{
	screen_t screen;
	int x, y;
//	int D=atoi(argv[1]);
#define D	3
	// try other values
	pthread_t ut;
	init();
	
	memset(intensity, 0, LCD_ROWS*LCD_COLS*8);
	if (pthread_create(&ut, 0, update_thread, 0)){
		perror("outdoor: could not create thread: ");
		return errno;
	}

	for (y=0; y<65; y++)
	{
		for (x=0; x<120; x++)
			image[LCD_COLS*y+x]=0;
	}

	while (!doexit)
	{
		int x, y, yy, pix;
		for (y=0; y<LCD_ROWS; y++) {
			for (x=0; x<LCD_COLS; x++) {
				pix = 0;
				for (yy=0; yy<8; yy++) {
					int off=(8*y+(7-yy))*LCD_COLS+x;
					int val, t=image[off];
					
					if (intensity[off] < t)
						val=1;
					else
						val=0;

					pix = (pix<<1) + val;
					
					intensity[off]=(intensity[off])*(256-D)/256;
					intensity[off]+=val*D;
				}
				screen[y*LCD_COLS+x] = pix;
			}
		}

		draw_screen(screen);

		RcGetActCode();
		if( doexit==3)
			doexit=1;
		else
			doexit=0;
	}

	pthread_join (ut, NULL);
	clr();
	return 0;
}

int plugin_exec( PluginParam *par )
{
	int a, fd_rc=-1;

	for( ; par; par=par->next )
	{
		if ( !strcmp(par->id,P_ID_RCINPUT) )
			fd_rc=_atoi(par->val);
		else if(!strcmp(par->id, P_ID_LCD))
			lcd_fd=_atoi(par->val);
	}
	
	if ( RcInitialize( fd_rc ) < 0 )
		return -1;

	if ( lcd_fd <= 0 )
		if( (lcd_fd=open("/dev/dbox/lcd0", O_RDWR)) < 0 )
		{
			perror("can't open(/dev/dbox/lcd0)");
			return -1;
		}

	a=outdoor_exec();

  	RcClose();
	return a;
}
