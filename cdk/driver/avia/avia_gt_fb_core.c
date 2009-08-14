/*
 *   avia_gt_fb.c - AViA eNX/GTX framebuffer driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 Felix "tmbinc" Domke (tmbinc@gmx.net)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *   $Log: avia_gt_fb_core.c,v $
 *   Revision 1.38.2.4  2003/03/19 19:43:12  zwen
 *   - forgot some lines while applying last delta
 *
 *   Revision 1.38.2.3  2003/03/05 09:12:12  zwen
 *   - eNX red & blue swap fix
 *   - fixed mmio address for eNX (by obi)
 *
 *   Revision 1.38.2.2  2003/02/28 16:06:55  obi
 *   - fix mmio offset
 *   - set accelerator id
 *
 *   Revision 1.38.2.1  2003/02/20 10:11:49  thegoodguy
 *   Fix ENX Red <-> Blue problem
 *
 *   Revision 1.38  2002/10/10 14:49:33  Jolt
 *   FB cleanups
 *
 *   Revision 1.37  2002/10/09 18:31:06  Jolt
 *   HW copy support
 *
 *   Revision 1.36  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.35  2002/08/11 13:07:34  waldi
 *   color 0 is transparent if
 *   - console in text mode
 *   - enabled
 *
 *   Revision 1.34  2002/08/11 11:33:24  TheDOC
 *   whoever did this crap, shame on you! 4 and 8 bit-modes are fixed now, 16 bit needs to be fixed, can't test this right now...
 *
 *   Revision 1.33  2002/07/19 22:59:21  waldi
 *   * use sun8x16 font
 *   * color 0 is transparent
 *   * use 8bit clut mode by default
 *
 *   Revision 1.32  2002/07/10 23:10:06  tmbinc
 *   some magic to let 8bpp/32bpp console work. may interfere with some guis, but who cares.. use pzap\!
 *
 *   Revision 1.31  2002/05/30 00:39:03  obi
 *   fixed 640x480 fullscreen console
 *
 *   Revision 1.30  2002/05/03 16:47:35  obi
 *   changed fb_ioctl
 *
 *   Revision 1.29  2002/04/25 22:10:38  Jolt
 *   FB cleanup
 *
 *   Revision 1.28  2002/04/25 21:09:02  Jolt
 *   Fixes/Cleanups
 *
 *   Revision 1.27  2002/04/24 21:38:13  Jolt
 *   Framebuffer cleanups
 *
 *   Revision 1.26  2002/04/24 19:56:00  Jolt
 *   GV driver updates
 *
 *   Revision 1.25  2002/04/22 17:40:01  Jolt
 *   Major cleanup
 *
 *   Revision 1.24  2002/04/21 14:36:07  Jolt
 *   Merged GTX fb support
 *
 *   Revision 1.23  2002/04/15 19:32:44  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.22  2002/04/15 19:05:03  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.21  2002/04/15 12:04:53  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.20  2002/04/14 18:06:19  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.19  2002/04/13 23:19:05  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.18  2002/04/12 23:20:25  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.17  2002/04/12 18:59:29  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.16  2002/03/29 19:16:29  obi
 *   - simplify gtx blev code
 *   - ioctl return value fix
 *
 *   Revision 1.15  2002/03/27 13:13:06  derget
 *   nix
 *
 *   Revision 1.14  2002/03/27 12:54:49  derget
 *   l33t h4xoring for kernel console on fb :)
 *   ntsc disabled
 *   xres -> yres -> vxres -> vyres check disabled
 *   ioctrls for blev
 *   ioctrls for Graphics Viewport Position
 *   ioctrls for Transparent Color Register (schwarz durchsichtig oder nicht)
 *
 *   Revision 1.13  2002/03/06 09:02:44  gillem
 *   - fix output
 *
 *   Revision 1.12  2001/12/01 06:37:06  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.11  2001/10/23 08:49:35  Jolt
 *   eNX capture and pig driver
 *
 *   Revision 1.10  2001/09/13 17:11:37  field
 *   Fixed CRLFs (verdammter Editor!)
 *
 *   Revision 1.9  2001/09/13 17:07:00  field
 *   Fixed ENX-Framebuffer position bug
 *
 *   Revision 1.8  2001/07/13 17:08:23  McClean
 *   fix framebuffer screenposition bug
 *
 *   Revision 1.7  2001/06/09 18:49:55  tmbinc
 *   fixed gtx-setcolreg.
 *
 *   Revision 1.6  2001/05/27 20:47:51  TripleDES
 *
 *   fixed the transparency for fb-console but this works not very fine with other modes (no black) :(  ...only that the fb-console runs, again ;)
 *   -
 *
 *   Revision 1.5  2001/05/26 22:22:24  TripleDES
 *
 *   fixed the grey-backgrund
 *
 *   Revision 1.4  2001/05/04 21:07:55  fnbrd
 *   Rand gefixed.
 *
 *   Revision 1.3  2001/04/30 21:51:38  tmbinc
 *   fixed setcolreg for eNX
 *
 *   Revision 1.2  2001/04/22 13:56:35  tmbinc
 *   other philips- (and maybe sagem?) fixes
 *
 *   Revision 1.1  2001/04/20 01:21:33  Jolt
 *   Final Merge :-)
 *
 *   Revision 1.11  2001/03/23 08:00:30  gillem
 *   - adjust xpos of fb
 *
 *   Revision 1.10  2001/03/08 01:15:14  tmbinc
 *   smem_length 1MB now, transparent color, defaults to dynaclut.
 *
 *   Revision 1.9  2001/02/11 16:32:08  tmbinc
 *   fixed viewport position
 *
 *   Revision 1.8  2001/02/04 20:46:14  tmbinc
 *   improved framebuffer.
 *
 *   Revision 1.7  2001/01/31 17:17:46  tmbinc
 *   Cleaned up avia drivers. - tmb
 *
 *   $Revision: 1.38.2.4 $
 *
 */

 /*
    This is the improved FB.
    Report bugs as usual.

    CLUTs completely untested. (just saw: they work.)

    There were other attempts to rewrite this driver, but i don't
    know the state of this work.

    roh suxx.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <video/fbcon.h>
#include <video/fbcon-cfb8.h>
#include <video/fbcon-cfb16.h>
#include <video/fbcon-cfb32.h>
#include "dbox/fb.h"
#include "dbox/avia_gt.h"
#include "dbox/avia_gt_gv.h"
#include <linux/kd.h>
#include <linux/vt_kern.h>

#define RES_X	   720
#define RES_Y	   576

static sAviaGtInfo *gt_info	= (sAviaGtInfo *)NULL;

#ifdef MODULE
MODULE_PARM(console_transparent, "i");
#endif

static int console_transparent = 0;

static struct fb_var_screeninfo default_var = {
    /* 720x576, 16 bpp */	       // TODO: NTSC
    RES_X, RES_Y, RES_X, RES_Y,	 /* W,H, W, H (virtual) load xres,xres_virtual*/
    0, 0,			       /* virtual -> visible no offset */
    8, 0,			      /* depth -> load bits_per_pixel */
    {0, 0, 0},			 /* ARGB 1555 */
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},			 /* transparency */
    0,				  /* standard pixel format */
    FB_ACTIVATE_NOW,
    -1, -1,
    0,				  /* accel flags */
    20000, 64, 64, 32, 32, 64, 2, 0,    /* sync stuff */
    FB_VMODE_INTERLACED
};


struct gtxfb_info
{

  struct fb_info_gen gen;

  unsigned char *videobase;
  int offset;
  unsigned int videosize;
  unsigned char *pvideobase;

};

struct gtxfb_par
{

  int bpp;	      // 4, 8, 16
  int lowres;	   // 0: 720 or 640 pixels per line 1: 360 or 320 pixels per line
  int interlaced;       // 0: 480 or 576 lines interlaced 1: 240 or 288 lines non-interlaced
  int xres, yres;       // calculated of the above stuff
  int stride;

};

static struct gtxfb_info fb_info;
static struct gtxfb_par current_par;
static int current_par_valid=0;
static struct display disp;

static char default_fontname[40] = { 0 };

#ifdef FBCON_HAS_CFB4
static u16 fbcon_cfb4_cmap[16];
#endif
#ifdef FBCON_HAS_CFB8
static u16 fbcon_cfb8_cmap[16];
#endif
#ifdef FBCON_HAS_CFB16
static u16 fbcon_cfb16_cmap[16];
#endif

static int avia_gt_fb_encode_fix(struct fb_fix_screeninfo *fix, const void *fb_par,
			struct fb_info_gen *info);
static int avia_gt_fb_decode_var(const struct fb_var_screeninfo *var, void *fb_par,
			struct fb_info_gen *info);
static int avia_gt_fb_encode_var(struct fb_var_screeninfo *var, const void *fb_par,
			struct fb_info_gen *info);
static void avia_gt_fb_get_par(void *fb_par, struct fb_info_gen *info);
static void avia_gt_fb_set_par(const void *fb_par, struct fb_info_gen *info);
static int avia_gt_fb_getcolreg(u_int regno, u_int *red, u_int *green, u_int *blue,
		u_int *transp, struct fb_info *info);
static int avia_gt_fb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
		u_int transp, struct fb_info *info);
static int avia_gt_fb_blank(int blank, struct fb_info_gen *info);
static void avia_gt_fb_set_disp(const void *fb_par, struct display *disp,
		struct fb_info_gen *info);

static void avia_gt_fb_detect(void)
{

	return;

}

static int avia_gt_fb_encode_fix(struct fb_fix_screeninfo *fix, const void *fb_par, struct fb_info_gen *info)
{

	struct gtxfb_par *par = (struct gtxfb_par *)fb_par;

	strcpy(fix->id, "AViA eNX/GTX Framebuffer improved version");
	
	fix->type = FB_TYPE_PACKED_PIXELS;
	fix->type_aux = 0;

	if (par->bpp != 16)
		fix->visual = FB_VISUAL_PSEUDOCOLOR;
	else
		fix->visual = FB_VISUAL_TRUECOLOR;

   fix->line_length=par->stride;
   fix->smem_start=(unsigned long)fb_info.pvideobase;
   fix->smem_len=1024*1024;			    /* FIXME: AVIA_GT_MEM_GV_SIZE? */

   fix->xpanstep=0;
   fix->ypanstep=0;
   fix->ywrapstep=0;

   if (avia_gt_chip(GTX))
   {
      fix->accel = FB_ACCEL_CCUBE_AVIA_GTX;
      fix->mmio_start = (unsigned long)GTX_REG_BASE;
      fix->mmio_len = 0x10000; /* FIXME: GTX_REG_SIZE? */
   }
   else if (avia_gt_chip(ENX))
   {
      fix->accel = FB_ACCEL_CCUBE_AVIA_ENX;
      fix->mmio_start = (unsigned long)ENX_REG_BASE;
      fix->mmio_len = 0x10000; /* FIXME: ENX_REG_SIZE? */
   }
	
	return 0;

}

static int avia_gt_fb_decode_var(const struct fb_var_screeninfo *var, void *fb_par, struct fb_info_gen *info)
{

	struct gtxfb_par *par = (struct gtxfb_par *)fb_par;
	int yres = 0;

	if ((var->bits_per_pixel != 4) && (var->bits_per_pixel != 8) && (var->bits_per_pixel != 16))
		return -EINVAL;

	par->bpp=var->bits_per_pixel;

	yres = var->yres;

	if (var->xres < 640)
		par->lowres = 1;
	else
		par->lowres = 0;

	if (var->yres >= 480) {
	
		par->interlaced = 1;
		yres >>= 1;

	} else {
	
		par->interlaced = 0;
		
	}

	par->xres = var->xres;
	par->yres = var->yres;

	if (par->bpp == 4)
		par->stride = var->xres / 2;
	else if (par->bpp == 8)
		par->stride = var->xres;
	else if (par->bpp == 16)
		par->stride = var->xres * 2;

	return 0;

}

static int avia_gt_fb_encode_var(struct fb_var_screeninfo *var, const void *fb_par, struct fb_info_gen *info)
{

	struct gtxfb_par *par = (struct gtxfb_par *)fb_par;

	var->xres = par->xres;
	var->yres = par->yres;
	var->bits_per_pixel = par->bpp;
	var->left_margin = 126;
	var->right_margin = 18;
	var->upper_margin = 21;
	var->lower_margin = 5;
	var->hsync_len = 0;
	var->vsync_len = 0;
	var->sync = 0;

	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres;

	var->xoffset = 0;
	var->yoffset = 0;

	var->red.length = 5;
	var->red.msb_right = 0;
   var->red.offset = 10;

	var->green.length = 5;
	var->green.msb_right = 0;
	var->green.offset = 5;

	var->blue.length = 5;
	var->blue.msb_right = 0;
   var->blue.offset = 0;

	var->transp.length = 1;
	var->transp.msb_right = 0;
	var->transp.offset = 15;

	var->grayscale = 0;

	var->xoffset = var->yoffset = 0;
	var->pixclock = 0;
	var->nonstd = 0;
	var->activate = 0;
	var->height = var->width - 1;
	var->accel_flags = 0;

	return 0;

}

static void avia_gt_fb_get_par(void *fb_par, struct fb_info_gen *info)
{

	struct gtxfb_par *par = (struct gtxfb_par *)fb_par;

	if (current_par_valid)
		*par = current_par;
	else
		avia_gt_fb_decode_var(&default_var, par, info);

}

static void avia_gt_fb_set_par(const void *fb_par, struct fb_info_gen *info)
{

	struct gtxfb_par *par = (struct gtxfb_par *)fb_par;

	switch (par->bpp) {

		case 4:

			avia_gt_gv_set_input_mode(AVIA_GT_GV_INPUT_MODE_RGB4);

		break;

		case 8:

			avia_gt_gv_set_input_mode(AVIA_GT_GV_INPUT_MODE_RGB8);

		break;

		case 16:

			avia_gt_gv_set_input_mode(AVIA_GT_GV_INPUT_MODE_RGB16);

		break;

		case 32:

			avia_gt_gv_set_input_mode(AVIA_GT_GV_INPUT_MODE_RGB32);

		break;

	}

	avia_gt_gv_set_blevel(0x5D, 0);
	avia_gt_gv_set_pos(0, 0);
	avia_gt_gv_set_input_size(par->xres, par->yres);

	avia_gt_gv_set_size(par->xres, par->yres);
	avia_gt_gv_show();

	current_par = *par;
	current_par_valid = 1;

}

static int avia_gt_fb_getcolreg(u_int regno, u_int *red, u_int *green, u_int *blue, u_int *transp, struct fb_info *info)
{

	if (regno > 255)
		return 1;

	avia_gt_gv_get_clut(regno, transp, red, green, blue);

	return 0;

}

static int avia_gt_fb_setcolreg(u_int regno, u_int red, u_int green, u_int blue, u_int transp, struct fb_info *info)
{

	unsigned short vc_text = 0;

	if (regno > 255)
		return 1;

	if (info->display_fg)
		vc_text = (vt_cons[info->display_fg->vc_num]->vc_mode == KD_TEXT);

	switch (current_par.bpp) {
#ifdef FBCON_HAS_CFB4
		case 4:
		
			if (regno == 0 && console_transparent && vc_text)
				avia_gt_gv_set_clut(0, 0xffff, 0, 0, 0);
			else
				avia_gt_gv_set_clut(regno, transp, red, green, blue);
				
		break;
#endif

#ifdef FBCON_HAS_CFB8
		case 8:

			if ((regno == 0) && console_transparent && vc_text)
				avia_gt_gv_set_clut(0, 0xFFFF, 0, 0, 0);
			else
				avia_gt_gv_set_clut(regno, transp, red, green, blue);

		break;
#endif

#ifdef FBCON_HAS_CFB16
		case 16:
		
			red >>= 11;
			green >>= 11;
			blue >>= 11;
			transp >>= 15;

			if ((regno == 0) && console_transparent && vc_text)
				fbcon_cfb16_cmap[0] = 0xfc0f;
				
			if (regno < 16)
				fbcon_cfb16_cmap[regno] = (transp << 15) | (red << 10) | (green << 5) | (blue);
				
		break;
#endif
		default:
		
			return 1;
			
		break;
		
	}

	return 0;
	
}

static int avia_gt_fb_blank(int blank, struct fb_info_gen *info)
{

	return 0;
	
}

static void avia_gt_fb_set_disp(const void *fb_par, struct display *disp, struct fb_info_gen *info)
{

	struct gtxfb_par *par = (struct gtxfb_par *)fb_par;

	disp->screen_base = (char *)fb_info.videobase;

	switch (par->bpp) {

#ifdef FBCON_HAS_CFB4
		case 4:
		
			disp->dispsw = &fbcon_cfb4;
			disp->dispsw_data = &fbcon_cfb4_cmap;
			
		break;
#endif
#ifdef FBCON_HAS_CFB8
		case 8:
		
			disp->dispsw = &fbcon_cfb8;
			disp->dispsw_data = &fbcon_cfb8_cmap;
			
		break;
#endif
#ifdef FBCON_HAS_CFB16
		case 16:
		
			disp->dispsw = &fbcon_cfb16;
			disp->dispsw_data = &fbcon_cfb16_cmap;
			
		break;
#endif
		default:
		
			disp->dispsw = &fbcon_dummy;
			
		break;
		
	}

#ifdef CONFIG_FBCON_SHIFT
	disp->shift_x = 4;
	disp->shift_y = 2;
#endif /* CONFIG_FBCON_SHIFT */

	disp->scrollmode = SCROLL_YREDRAW;
	
}

struct fbgen_hwswitch avia_gt_fb_switch = {

	blank: avia_gt_fb_blank,
	decode_var: avia_gt_fb_decode_var,
	detect: avia_gt_fb_detect,
	encode_fix: avia_gt_fb_encode_fix,
	encode_var: avia_gt_fb_encode_var,
	get_par: avia_gt_fb_get_par,
	getcolreg: avia_gt_fb_getcolreg,
	set_disp: avia_gt_fb_set_disp,
	set_par: avia_gt_fb_set_par,
	setcolreg: avia_gt_fb_setcolreg,

};

static int fb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg, int con, struct fb_info *info)
{

	fb_copyarea copyarea;

	switch (cmd) {
	
		case AVIA_GT_GV_COPYAREA:
		
			if (copy_from_user(&copyarea, (void *)arg, sizeof(copyarea)))
				return -EFAULT;
			
			avia_gt_gv_copyarea(copyarea.sx, copyarea.sy, copyarea.width, copyarea.height, copyarea.dx, copyarea.dy);
		
		break;

		case AVIA_GT_GV_SET_BLEV:

			avia_gt_gv_set_blevel((arg >> 8) & 0xFF, arg & 0xFF);
			
		break;

		case AVIA_GT_GV_SET_POS:

			avia_gt_gv_set_pos((arg >> 16) & 0xFFFF, arg & 0xFFFF);
			
		break;

		case AVIA_GT_GV_HIDE:

			avia_gt_gv_hide();
			
		break;

		case AVIA_GT_GV_SHOW:

			avia_gt_gv_show();
			
		break;

		default:

			return -EINVAL;
	}

	return 0;

}

static struct fb_ops avia_gt_fb_ops = {

	owner:			THIS_MODULE,
	fb_get_fix:		fbgen_get_fix,
	fb_get_var:		fbgen_get_var,
	fb_set_var:		fbgen_set_var,
	fb_get_cmap:	fbgen_get_cmap,
	fb_set_cmap:	fbgen_set_cmap,
	fb_ioctl:		fb_ioctl

};

int __init avia_gt_fb_init(void)
{

	printk("avia_gt_fb: $Id: avia_gt_fb_core.c,v 1.38.2.4 2003/03/19 19:43:12 zwen Exp $\n");

	gt_info = avia_gt_get_info();

	if ((!gt_info) || ((!avia_gt_chip(ENX)) && (!avia_gt_chip(GTX)))) {

		printk("avia_gt_fb: Unsupported chip type\n");

		return -EIO;

	}

	avia_gt_gv_get_info(&fb_info.pvideobase, &fb_info.videobase, &fb_info.videosize);

	fb_info.offset = AVIA_GT_MEM_GV_OFFS;

	fb_info.gen.info.node = -1;
	fb_info.gen.info.flags = FBINFO_FLAG_DEFAULT;
	fb_info.gen.info.fbops = &avia_gt_fb_ops;
	fb_info.gen.info.disp = &disp;
	fb_info.gen.info.changevar = NULL;
	fb_info.gen.info.switch_con = &fbgen_switch;
	fb_info.gen.info.updatevar = &fbgen_update_var;
	fb_info.gen.info.blank = &fbgen_blank;
	strcpy(fb_info.gen.info.fontname, default_fontname);
	fb_info.gen.parsize = sizeof(struct gtxfb_par);
	fb_info.gen.fbhw = &avia_gt_fb_switch;
	fb_info.gen.fbhw->detect();

	strcpy(fb_info.gen.info.modename, "AViA eNX/GTX Framebuffer");

	fbgen_get_var(&disp.var, -1, &fb_info.gen.info);
	disp.var.activate = FB_ACTIVATE_NOW;
	fbgen_do_set_var(&disp.var, 1, &fb_info.gen);
	fbgen_set_disp(-1, &fb_info.gen);
	fbgen_install_cmap(0, &fb_info.gen);

	strcpy (fb_info.gen.info.fontname, "SUN8x16");

	if (register_framebuffer(&fb_info.gen.info) < 0)
		return -EINVAL;

	printk(KERN_INFO "avia_gt_fb: fb%d: %s frame buffer device\n", GET_FB_IDX(fb_info.gen.info.node), fb_info.gen.info.modename);

	avia_gt_gv_show();

	return 0;

}

void __exit avia_gt_fb_exit(void)
{

	unregister_framebuffer(&fb_info.gen.info);

}

#ifdef MODULE
module_init(avia_gt_fb_init);
module_exit(avia_gt_fb_exit);
#endif
