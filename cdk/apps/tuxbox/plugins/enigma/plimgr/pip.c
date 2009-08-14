#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <linux/fb.h>

#if HAVE_DVB_API_VERSION < 3
#include <dbox/avia_gt_pig.h>
#else
#include <linux/input.h>
#include <linux/videodev.h>
#endif

#include <sys/ioctl.h>

#include <dbox/avs_core.h>
#include <dbox/saa7126_core.h>
#include <dbox/fp.h>
#include <dbox/lcd-ks0713.h>


/* devices */
#define AVS "/dev/dbox/avs0"
#define SAA "/dev/dbox/saa0"
#if HAVE_DVB_API_VERSION < 3
#define PIG "/dev/dbox/pig0"
#else
#define PIG "/dev/v4l/video0"
#endif



int pig, avs, saa;
int sx, ex, sy, ey;
int fnc_old, saa_old;
int  screen_mode1 = 0; /* 0 or 1: 4x3 or 16x9 */
int screen_mode2 = 0;
const int fncmodes[] = {AVS_FNCOUT_EXT43, AVS_FNCOUT_EXT169};
const int saamodes[] = {SAA_WSS_43F, SAA_WSS_169F};

void SwitchScreenMode(int screenmode);
void FullScreen(void);
void SetScreenMode(int left, int width, int top, int height);

int main(int argc, char ** argv)
{
	int x, y, w, h, steps, xstep, ystep;
	int screenmode=0;
	// sx=20, sy=20, ex=699, ey=555;
	sx=0x0026;
	ex=0x02ab;
	sy=0x0014;
	ey=0x022d;


       // eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/left", sx);
       // eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/top", sy);
       // eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/right", ex);
       // eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/bottom", ey);
	printf("Viewport: x = %d, w = %d, y = %d, h = %d\n", sx, ex-sx, sy, ey-sy);

       if (*++argv && **argv)
	    screenmode = atoi(*argv);

#if 0
	/* open avs */
	if ((avs = open(AVS, O_RDWR)) == -1)
	{
		perror("TuxTxt <open AVS>");
		return 0;
	}
	ioctl(avs, AVSIOGSCARTPIN8, &fnc_old);
//	ioctl(avs, AVSIOSSCARTPIN8, &fncmodes[screen_mode1]);


	/* open saa */
	if ((saa = open(SAA, O_RDWR)) == -1)
	{
		perror("TuxTxt <open SAA>");
		return 0;
	}
	ioctl(saa, SAAIOGWSS, &saa_old);
//	ioctl(saa, SAAIOSWSS, &saamodes[screen_mode1]);
#endif


	/* open pig */
	if ((pig = open(PIG, O_RDWR)) == -1)
	{
		perror("TuxTxt <open PIG>");
		return 0;
	}


	// SwitchScreenMode(screenmode);


        // sleep(5);


	w = 270;
	h = 200;
	steps = 20;
	xstep = (ex - sx - w) / steps;
	ystep = (ey - sy - h) / steps;
	printf("Loop 1\n");
	for (x = 0, y = 0; (x < ex - sx - w) & (y < ey - sy - h); x += xstep, y += ystep)
	{
		printf("x = %d, w = %d, y = %d, h = %d\n", x, w, y, h);
		SetScreenMode(x, w, y, h);
		usleep(300000);
	}
//	x = ex - sx - w;
//	y = ey - sy - h;
	for (; y > 0; y -= ystep)
	{
		printf("x = %d, w = %d, y = %d, h = %d\n", x, w, y, h);
		SetScreenMode(x, w, y, h);
		usleep(300000);
	}
	for (; (x >0 ) & (y < ey - sy - h); x -= xstep, y += ystep)
	{
		printf("x = %d, w = %d, y = %d, h = %d\n", x, w, y, h);
		SetScreenMode(x, w, y, h);
		usleep(300000);
	}
	for (; y > 0; y -= ystep)
	{
		printf("x = %d, w = %d, y = %d, h = %d\n", x, w, y, h);
		SetScreenMode(x, w, y, h);
		usleep(300000);
	}

	x = 0;
	y = 0;
	x = 0;
	y = 0;
	xstep = (ex - sx - 10) / steps;
	ystep = (ey - sy - 10) / steps;
	printf("Loop 2\n");
	for (w = 10, h = 10; w < ex - sx && y < ey - sy - h; w += xstep, h += ystep)
	{
		printf("x = %d, w = %d, y = %d, h = %d\n", x, w, y, h);
		SetScreenMode(x, w, y, h);
		usleep(300000);
	}


	/* restore videoformat */
	FullScreen();
#if 0
	ioctl(avs, AVSIOSSCARTPIN8, &fnc_old);
	ioctl(saa, SAAIOSWSS, &saa_old);
#endif

	exit(0);

}



/******************************************************************************
 * SwitchScreenMode                                                           *
 ******************************************************************************/

void SwitchScreenMode(int screenmode)
{
#if HAVE_DVB_API_VERSION >= 3
	struct v4l2_format format;
#endif


#define TV43STARTX (ex - 146) //(StartX + 2 + (40-nofirst)*fontwidth_topmenumain + (40*fontwidth_topmenumain/abx))
#define TV43WIDTH 144 /* 120 */
#define TV43STARTY (sy + (ey-sy)/2 - TV43HEIGHT)
#define TV43HEIGHT 116 /* 96 */
#define TV169FULLSTARTX (sx+ 8*40) //(sx +(ex +1 - sx)/2)
#define TV169FULLWIDTH  (ex - sx)/2
#define TV169FULLSTARTY sy
#define TV169FULLHEIGHT (ey - sy)

	/* set mode */
	if (screenmode)								 /* split */
	{
		int tx, ty, tw, th;

		if (screenmode==1) /* split with topmenu */
		{
			tw = TV43WIDTH;
			tx = TV43STARTX;
			ty = TV43STARTY;
			th = TV43HEIGHT;
		}
		else /* 2: split with full height tv picture */
		{
			tx = TV169FULLSTARTX;
			ty = TV169FULLSTARTY;
			tw = TV169FULLWIDTH;
			th = TV169FULLHEIGHT;
		}

#if HAVE_DVB_API_VERSION < 3
		avia_pig_hide(pig);
		avia_pig_set_pos(pig, tx, ty);
		avia_pig_set_size(pig, tw, th);
		avia_pig_set_stack(pig, 2);
		avia_pig_show(pig);
#else
		int sm = 0;
		ioctl(pig, VIDIOC_OVERLAY, &sm);
		sm = 1;
		ioctl(pig, VIDIOC_G_FMT, &format);
		format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
		format.fmt.win.w.left   = tx;
		format.fmt.win.w.top    = ty;
		format.fmt.win.w.width  = tw;
		format.fmt.win.w.height = th;
		ioctl(pig, VIDIOC_S_FMT, &format);
		ioctl(pig, VIDIOC_OVERLAY, &sm);
#endif
	//	ioctl(avs, AVSIOSSCARTPIN8, &fncmodes[screen_mode2]);
	//	ioctl(saa, SAAIOSWSS, &saamodes[screen_mode2]);
	}
	else /* not split */
	{
#if HAVE_DVB_API_VERSION < 3
		avia_pig_hide(pig);
#else
		ioctl(pig, VIDIOC_OVERLAY, &screenmode);
#endif

	//	ioctl(avs, AVSIOSSCARTPIN8, &fncmodes[screen_mode1]);
	//	ioctl(saa, SAAIOSWSS, &saamodes[screen_mode1]);
	}
}


void FullScreen()
{
#if HAVE_DVB_API_VERSION < 3
	avia_pig_hide(pig);
#else
	int screenmode = 0;

	ioctl(pig, VIDIOC_OVERLAY, &screenmode);
#endif
}


void SetScreenMode(int left, int width, int top, int height)
{
#if HAVE_DVB_API_VERSION >= 3
	struct v4l2_format format;
#endif
	int tx, ty, tw, th;

	tx = sx + left;
	tw = width;
	ty = sy + top;
	th = height;
	if (tx > ex)
		tx = ex - 10; 
	if (tx + tw > ex)
		tw = ex - tx; 
	if (ty > ey)
		ty = ey - 10; 
	if (ty + th > ey)
		th = ey - ty; 

#if HAVE_DVB_API_VERSION < 3
	//avia_pig_hide(pig);
	avia_pig_set_pos(pig, tx, ty);
	avia_pig_set_size(pig, tw, th);
	avia_pig_set_stack(pig, 2);
	avia_pig_show(pig);
#else
	int sm = 0;
	ioctl(pig, VIDIOC_OVERLAY, &sm);
	sm = 1;
	ioctl(pig, VIDIOC_G_FMT, &format);
	format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	format.fmt.win.w.left   = tx;
	format.fmt.win.w.top    = ty;
	format.fmt.win.w.width  = tw;
	format.fmt.win.w.height = th;
	ioctl(pig, VIDIOC_S_FMT, &format);
	ioctl(pig, VIDIOC_OVERLAY, &sm);
#endif
}


