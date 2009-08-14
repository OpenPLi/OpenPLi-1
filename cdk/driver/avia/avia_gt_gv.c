/*
 *   avia_gt_gv.c - AViA eNX/GTX graphic viewport driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2002 Florian Schirmer (jolt@tuxbox.org)
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
 *   $Log: avia_gt_gv.c,v $
 *   Revision 1.25.2.2  2003/04/10 14:53:43  zwen
 *   - fixed avia_gt_gv_get_clut (eNX)
 *
 *   Revision 1.25.2.1  2003/03/05 09:12:12  zwen
 *   - eNX red & blue swap fix
 *   - fixed mmio address for eNX (by obi)
 *
 *   Revision 1.25  2002/10/11 09:57:54  Jolt
 *   HW copy stuff
 *
 *   Revision 1.24  2002/10/09 20:20:07  Jolt
 *   Uhhh :)
 *
 *   Revision 1.23  2002/10/09 18:31:06  Jolt
 *   HW copy support
 *
 *   Revision 1.22  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.21  2002/07/21 15:16:22  waldi
 *   add workaround for broken memory access on philips boxes
 *
 *   Revision 1.20  2002/06/07 18:06:03  Jolt
 *   GCC31 fixes 2nd shot (GTX version) - sponsored by Frankster (THX!)
 *
 *   Revision 1.19  2002/06/07 17:53:45  Jolt
 *   GCC31 fixes 2nd shot - sponsored by Frankster (THX!)
 *
 *   Revision 1.18  2002/05/30 00:39:03  obi
 *   fixed 640x480 fullscreen console
 *
 *   Revision 1.17  2002/05/09 22:23:30  obi
 *   fixed avia_gt_set_blevel()
 *
 *   Revision 1.16  2002/05/08 03:47:49  obi
 *   use slab.h instead of malloc.h
 *
 *   Revision 1.15  2002/05/03 17:31:44  obi
 *   bugfix
 *
 *   Revision 1.14  2002/05/03 17:04:40  obi
 *   moved some code from fb_core to gv_init
 *   replaced r*() by gtx_reg_*()
 *
 *   Revision 1.13  2002/04/28 20:16:09  Jolt
 *   GTX fbworkaround
 *
 *   Revision 1.12  2002/04/25 22:10:38  Jolt
 *   FB cleanup
 *
 *   Revision 1.11  2002/04/25 21:09:02  Jolt
 *   Fixes/Cleanups
 *
 *   Revision 1.10  2002/04/24 21:38:13  Jolt
 *   Framebuffer cleanups
 *
 *   Revision 1.9  2002/04/24 19:56:00  Jolt
 *   GV driver updates
 *
 *   Revision 1.8  2002/04/24 08:01:00  obi
 *   more gtx support
 *
 *   Revision 1.7  2002/04/22 17:40:01  Jolt
 *   Major cleanup
 *
 *   Revision 1.6  2002/04/21 14:36:07  Jolt
 *   Merged GTX fb support
 *
 *   Revision 1.5  2002/04/16 13:58:16  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.4  2002/04/15 19:32:44  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.3  2002/04/15 10:40:50  Jolt
 *   eNX/GTX
 *
 *   Revision 1.2  2002/04/14 18:14:08  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.1  2001/11/01 18:19:09  Jolt
 *   graphic viewport driver added
 *
 *
 *   $Revision: 1.25.2.2 $
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <linux/init.h>

#include <dbox/avia_gt.h>
#include <dbox/avia_gt_gv.h>
#include <dbox/avia_gt_accel.h>

static u16 input_height = 576;
static u8 input_mode = AVIA_GT_GV_INPUT_MODE_RGB16;
static sAviaGtInfo *gt_info = (sAviaGtInfo *)NULL;
static u16 input_width = 720;
static u16 output_x = 0;
static u16 output_y = 0;

u8 avia_gt_get_bpp(void);
void avia_gt_gv_set_stride(void);

void avia_gt_gv_copyarea(u16 src_x, u16 src_y, u16 width, u16 height, u16 dst_x, u16 dst_y)
{

	u16 bpp = avia_gt_get_bpp();
	u16 line;
	u16 stride = avia_gt_gv_get_stride();
	
	for (line = 0; line < height; line++)
		avia_gt_accel_copy(AVIA_GT_MEM_GV_OFFS + (src_y + line) * stride + src_x * bpp, AVIA_GT_MEM_GV_OFFS + (dst_y + line) * stride + dst_x * bpp, width * bpp, 0);

}

void avia_gt_gv_cursor_hide(void)
{

	if (avia_gt_chip(ENX))
		enx_reg_set(GMR1, C, 0);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(GMR, C, 0);

}

void avia_gt_gv_cursor_show(void)
{

	if (avia_gt_chip(ENX))
		enx_reg_set(GMR1, C, 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(GMR, C, 1);

}

u8 avia_gt_get_bpp(void) 
{

	switch(input_mode) {

		case AVIA_GT_GV_INPUT_MODE_RGB4:

			return 1;

		break;
		case AVIA_GT_GV_INPUT_MODE_RGB8:

			return 1;

		break;
		case AVIA_GT_GV_INPUT_MODE_RGB16:

			return 2;

		break;
		case AVIA_GT_GV_INPUT_MODE_RGB32:

			return 4;

		break;

	}

	return 2;

}

void avia_gt_gv_get_clut(u8 clut_nr, u32 *transparency, u32 *red, u32 *green, u32 *blue)
{

	u32 val = (u32)0;

	if (avia_gt_chip(ENX)) {

		enx_reg_16(CLUTA) = clut_nr;

		mb();

		val = enx_reg_32(CLUTD);

		if (transparency)
			 *transparency = ((val & 0xFF000000) >> 24);

		if (red)
			 *red = ((val & 0x00FF0000) >> 16);

		if (green)
			 *green = ((val & 0x0000FF00) >> 8);

		if (blue)
			 *blue = (val & 0x000000FF);

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_set(CLTA, Addr, clut_nr);

		mb();

#define TCR_COLOR 0xFC0F

		val = gtx_reg_16(CLTD);

		if (val == TCR_COLOR) {

			 if (transparency)
				*transparency = 255;

			 if (red)
				*red = 0;

			 if (green)
				*green = 0;

			 if (blue)
				*blue = 0;

		} else {

			 //if (transparency)
				//*transparency = (val & 0x8000) ? BLEVEL : 0;

			 if (red)
				*red = (val & 0x7C00) << 9;

			 if (green)
				*green = (val & 0x03E0) << 14;

			 if (blue)
				*blue = (val & 0x001F) << 19;

		}

	}

}

void avia_gt_gv_get_info(u8 **gv_mem_phys, u8 **gv_mem_lin, u32 *gv_mem_size)
{

	if (avia_gt_chip(ENX)) {

		if (gv_mem_phys)
			 *gv_mem_phys = (u8 *)(ENX_MEM_BASE + AVIA_GT_MEM_GV_OFFS);

	} else if (avia_gt_chip(GTX)) {

		if (gv_mem_phys)
			 *gv_mem_phys = (u8 *)(GTX_MEM_BASE + AVIA_GT_MEM_GV_OFFS);

	}

	if (gv_mem_lin)
		*gv_mem_lin = gt_info->mem_addr + AVIA_GT_MEM_GV_OFFS;

	if (gv_mem_size)
		*gv_mem_size = AVIA_GT_MEM_GV_SIZE;

}

u16 avia_gt_gv_get_stride(void)
{

	u16 stride = (u16)0;

	if (avia_gt_chip(ENX))
		stride = enx_reg_s(GMR1)->STRIDE << 2;
	else if (avia_gt_chip(GTX))
		stride = gtx_reg_s(GMR)->STRIDE << 1;

	return stride;

}

void avia_gt_gv_hide(void)
{

	if (avia_gt_chip(ENX))
		enx_reg_set(GMR1, GMD, AVIA_GT_GV_INPUT_MODE_OFF);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(GMR, GMD, AVIA_GT_GV_INPUT_MODE_OFF);

}

void avia_gt_gv_set_blevel(u8 class0, u8 class1)
{

	if ((class0 > 0x08) && (class0 != 0x0F))
		return;

	if ((class1 > 0x08) && (class1 != 0x0F))
		return;

	if (avia_gt_chip(ENX)) {

		if (class0 == 0x0F)
			enx_reg_set(GBLEV1, BLEV10, 0xFF);
		else
			enx_reg_set(GBLEV1, BLEV10, class0 << 4);

		if (class1 == 0x0F)
			enx_reg_set(GBLEV1, BLEV11, 0xFF);
		else
			enx_reg_set(GBLEV1, BLEV11, class1 << 4);

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_set(GMR, BLEV0, class0);
		gtx_reg_set(GMR, BLEV1, class1);

	}

}

void avia_gt_gv_set_clut(u8 clut_nr, u32 transparency, u32 red, u32 green, u32 blue)
{

	if (avia_gt_chip(ENX)) {

		transparency >>= 8;
		red >>= 8;
		green >>= 8;
		blue >>= 8;

		enx_reg_16(CLUTA) = clut_nr;

		mb();

		enx_reg_32(CLUTD) = ((transparency << 24) | (red << 16) | (green << 8) | (blue));

	} else if (avia_gt_chip(GTX)) {

		transparency >>= 8;
		red >>= 11;
		green >>= 11;
		blue >>= 11;

		gtx_reg_16(CLTA) = clut_nr;

		mb();

		if (transparency >= 0x80) {	  // full transparency

#define TCR_COLOR 0xFC0F

			gtx_reg_16(CLTD) = TCR_COLOR;

		} else {

			if (!transparency)
				transparency = 1;
			else
				transparency = 0;

			gtx_reg_16(CLTD) = (transparency << 15) | (red << 10) | (green << 5) | (blue);

		}

	}

}

int avia_gt_gv_set_input_mode(u8 mode)
{

	printk("avia_gt_gv: set_input_mode (mode=%d)\n", mode);

	input_mode = mode;

	// Since mode changed, we have to recalculate some stuff
	avia_gt_gv_set_stride();

	return 0;

}

int avia_gt_gv_set_input_size(u16 width, u16 height)
{

	printk("avia_gt_gv: set_input_size (width=%d, height=%d)\n", width, height);

	if (width == 720) {

		if (avia_gt_chip(ENX)) {

			enx_reg_set(GMR1, L, 0);
			enx_reg_set(GMR1, F, 0);

		} else if (avia_gt_chip(GTX)) {

			 gtx_reg_set(GMR, L, 0);
			 gtx_reg_set(GMR, F, 0);

		}

	} else if (width == 640) {

		/*
		 * F = 0
		 * 1 would stretch 640x480 to 720x576
		 * this allows seeing a full screen console on tv
		 */

		if (avia_gt_chip(ENX)) {

			enx_reg_set(GMR1, L, 0);
			enx_reg_set(GMR1, F, 0);

		} else if (avia_gt_chip(GTX)) {

			gtx_reg_set(GMR, L, 0);
			gtx_reg_set(GMR, F, 0);

		}

	} else if (width == 360) {

		if (avia_gt_chip(ENX)) {

			enx_reg_set(GMR1, L, 1);
			enx_reg_set(GMR1, F, 0);

		} else if (avia_gt_chip(GTX)) {

			gtx_reg_set(GMR, L, 1);
			gtx_reg_set(GMR, F, 0);

		}

	} else if (width == 320) {

		if (avia_gt_chip(ENX)) {

			enx_reg_set(GMR1, L, 1);
			enx_reg_set(GMR1, F, 1);

		} else if (avia_gt_chip(GTX)) {

			gtx_reg_set(GMR, L, 1);
			gtx_reg_set(GMR, F, 1);

		}

	} else {

		return -EINVAL;

	}

	if ((height == 576) || (height == 480)) {

		if (avia_gt_chip(ENX))
			enx_reg_set(GMR1, I, 0);
		else if (avia_gt_chip(GTX))
			gtx_reg_set(GMR, I, 0);

	} else if ((height == 288) || (height == 240)) {

		if (avia_gt_chip(ENX))
			enx_reg_set(GMR1, I, 1);
		else if (avia_gt_chip(GTX))
			gtx_reg_set(GMR, I, 1);

	} else {

		return -EINVAL;

	}

	input_height = height;
	input_width = width;

	// Since width changed, we have to recalculate some stuff
	avia_gt_gv_set_pos(output_x, output_y);
	avia_gt_gv_set_stride();

	return 0;

}

int avia_gt_gv_set_pos(u16 x, u16 y) {

	u8 input_div = 0;

#define BLANK_TIME		132
#define ENX_VID_PIPEDELAY	16
#define GTX_VID_PIPEDELAY	5
#define GFX_PIPEDELAY		3

	if (input_width == 720)
		input_div = 8;
	else if (input_width == 640)
		input_div = 9;
	else if (input_width == 360)
		input_div = 16;
	else if (input_width == 320)
		input_div = 18;
	else
		return -EINVAL;

	if (avia_gt_chip(ENX)) {

		enx_reg_set(GVP1, SPP, (((BLANK_TIME - ENX_VID_PIPEDELAY) + x) * 8) % input_div);
		enx_reg_set(GVP1, XPOS, ((((BLANK_TIME - ENX_VID_PIPEDELAY) + x) * 8) / input_div) - GFX_PIPEDELAY);
		enx_reg_set(GVP1, YPOS, 42 + y);

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_set(GVP, SPP, (((BLANK_TIME - GTX_VID_PIPEDELAY) + x) * 8) % input_div);
		//gtx_reg_set(GVP, XPOS, ((((BLANK_TIME - GTX_VID_PIPEDELAY) + x) * 8) / input_div) - GFX_PIPEDELAY);
		gtx_reg_set(GVP, XPOS, ((((BLANK_TIME - GTX_VID_PIPEDELAY - 55) + x) * 8) / input_div) - GFX_PIPEDELAY);	//FIXME
		gtx_reg_set(GVP, YPOS, 42 + y);

	}

	output_x = x;
	output_y = y;

	return 0;

}

void avia_gt_gv_set_size(u16 width, u16 height) {

	if (avia_gt_chip(ENX)) {

		enx_reg_set(GVSZ1, IPP, 0);
		enx_reg_set(GVSZ1, XSZ, width);
		enx_reg_set(GVSZ1, YSZ, height);

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_set(GVS, IPS, 0);
		gtx_reg_set(GVS, XSZ, width);
		gtx_reg_set(GVS, YSZ, height);

	}

}

void avia_gt_gv_set_stride(void) {

	if (avia_gt_chip(ENX))
		enx_reg_set(GMR1, STRIDE, ((input_width * avia_gt_get_bpp()) + 3) >> 2);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(GMR, STRIDE, ((input_width * avia_gt_get_bpp()) + 1) >> 1);

}

int avia_gt_gv_show(void) {

	switch(input_mode) {

		case AVIA_GT_GV_INPUT_MODE_OFF:

			if (avia_gt_chip(ENX))
				enx_reg_set(GMR1, GMD, 0x00);
			else if (avia_gt_chip(GTX))
				gtx_reg_set(GMR, GMD, 0x00);

		break;
		case AVIA_GT_GV_INPUT_MODE_RGB4:

			if (avia_gt_chip(ENX))
				enx_reg_set(GMR1, GMD, 0x02);
			else if (avia_gt_chip(GTX))
				gtx_reg_set(GMR, GMD, 0x01);

		break;
		case AVIA_GT_GV_INPUT_MODE_RGB8:

			if (avia_gt_chip(ENX))
				enx_reg_set(GMR1, GMD, 0x06);
			else if (avia_gt_chip(GTX))
				gtx_reg_set(GMR, GMD, 0x02);

		break;
		case AVIA_GT_GV_INPUT_MODE_RGB16:

			if (avia_gt_chip(ENX))
				enx_reg_set(GMR1, GMD, 0x03);
			else if (avia_gt_chip(GTX))
				gtx_reg_set(GMR, GMD, 0x03);

		break;
		case AVIA_GT_GV_INPUT_MODE_RGB32:

			if (avia_gt_chip(ENX))
				enx_reg_set(GMR1, GMD, 0x07);
			else if (avia_gt_chip(GTX))
				return -EINVAL;

		break;
		default:

			return -EINVAL;

		break;

	}

	return 0;

}

int avia_gt_gv_init(void)
{

	printk("avia_gt_gv: $Id: avia_gt_gv.c,v 1.25.2.2 2003/04/10 14:53:43 zwen Exp $\n");

	gt_info = avia_gt_get_info();

	if ((!gt_info) || ((!avia_gt_chip(ENX)) && (!avia_gt_chip(GTX)))) {

		printk("avia_gv_pig: Unsupported chip type\n");

		return -EIO;

	}

	if (avia_gt_chip(ENX)) {

		enx_reg_set(RSTR0, GFIX, 1);
		enx_reg_set(RSTR0, GFIX, 0);

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_set(RR0, GV, 1);
		gtx_reg_set(RR0, GV, 0);

	}

	//avia_gt_gv_hide();
	avia_gt_gv_cursor_hide();
#ifdef WORKAROUND_MEMORY_TIMING
	udelay(100);
#endif /* WORKAROUND_MEMORY_TIMING */
	avia_gt_gv_set_pos(0, 0);
#ifdef WORKAROUND_MEMORY_TIMING
	udelay(100);
#endif /* WORKAROUND_MEMORY_TIMING */
	avia_gt_gv_set_input_size(720, 576);
#ifdef WORKAROUND_MEMORY_TIMING
	udelay(100);
#endif /* WORKAROUND_MEMORY_TIMING */
	avia_gt_gv_set_size(720, 576);

	if (avia_gt_chip(ENX)) {

#ifdef WORKAROUND_MEMORY_TIMING
		udelay(1000);
#endif /* WORKAROUND_MEMORY_TIMING */

		//enx_reg_set(GMR1, P, 1);
		enx_reg_set(GMR1, S, 1);
		enx_reg_set(GMR1, B, 0);
		//enx_reg_set(GMR1, BANK, 1);

		//enx_reg_set(BALP, AlphaOut, 0x00);
		//enx_reg_set(BALP, AlphaIn, 0x00);

		enx_reg_set(G1CFR, CFT, 0x1);
		enx_reg_set(G2CFR, CFT, 0x1);

		enx_reg_set(GBLEV1, BLEV11, 0x00);
		enx_reg_set(GBLEV1, BLEV10, 0x20);

#ifdef WORKAROUND_MEMORY_TIMING
		udelay(1000);
#endif /* WORKAROUND_MEMORY_TIMING */

		// schwarzer consolen hintergrund nicht transpartent
		enx_reg_set(TCR1, E, 0x1);
		enx_reg_set(TCR1, Red, 0xFF);
		enx_reg_set(TCR1, Green, 0x00);
		enx_reg_set(TCR1, Blue, 0x7F);

		// disabled - we don't need since we have 7bit true alpha
		enx_reg_set(TCR2, E, 0x0);
		enx_reg_set(TCR2, Red, 0xFF);
		enx_reg_set(TCR2, Green, 0x00);
		enx_reg_set(TCR2, Blue, 0x7F);

#ifdef WORKAROUND_MEMORY_TIMING
		udelay(1000);
#endif /* WORKAROUND_MEMORY_TIMING */

		enx_reg_set(VBR, E, 0x0);
		enx_reg_set(VBR, Y, 0x00);
		enx_reg_set(VBR, Cr, 0x00);
		enx_reg_set(VBR, Cb, 0x00);

		enx_reg_set(VCR, D, 0x1);
		/* enx_reg_set(VCR, C, 0x1); chroma sense - do not use */

		enx_reg_set(VMCR, FFM, 0x0);

		enx_reg_set(GVSA1, Addr, AVIA_GT_MEM_GV_OFFS >> 2);

	} else if (avia_gt_chip(GTX)) {

		// chroma filter. evtl. average oder decimate, bei text
		gtx_reg_set(GMR, CFT, 0x3);
		gtx_reg_set(GMR, BLEV1, 0x00);
		gtx_reg_set(GMR, BLEV0, 0x02);

		// ekelhaftes rosa als transparent
		gtx_reg_set(TCR, E, 0x1);
		gtx_reg_set(TCR, R, 0x1F);
		gtx_reg_set(TCR, G, 0x00);
		gtx_reg_set(TCR, B, 0x0F);

		gtx_reg_set(VHT, Width, 858);
		gtx_reg_set(VLT, VBI, 21); // NTSC = 18, PAL = 21
		gtx_reg_set(VLT, Lines, 623);

		// white cursor
		gtx_reg_set(CCR, R, 0x1F);
		gtx_reg_set(CCR, G, 0x1F);
		gtx_reg_set(CCR, B, 0x1F);

		// decoder sync. HSYNC polarity einstellen? low vs. high active?
		gtx_reg_set(VCR, HP, 0x2);
		gtx_reg_set(VCR, FP, 0x0);
		gtx_reg_set(VCR, D, 0x1);

		// enable dynamic clut
		gtx_reg_set(GFUNC, D, 0x1);

		// disable background
		gtx_reg_set(VBR, E, 0x0);
		gtx_reg_set(VBR, Y, 0x00);
		gtx_reg_set(VBR, Cr, 0x00);
		gtx_reg_set(VBR, Cb, 0x00);

		gtx_reg_set(GVSA, Addr, AVIA_GT_MEM_GV_OFFS >> 1);

	}

	return 0;

}

void __exit avia_gt_gv_exit(void)
{

//	avia_gt_gv_hide();

	if (avia_gt_chip(ENX))
		enx_reg_set(RSTR0, GFIX, 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(RR0, GV, 1);

}

#ifdef MODULE
EXPORT_SYMBOL(avia_gt_gv_copyarea);
EXPORT_SYMBOL(avia_gt_gv_get_clut);
EXPORT_SYMBOL(avia_gt_gv_get_info);
EXPORT_SYMBOL(avia_gt_gv_set_blevel);
EXPORT_SYMBOL(avia_gt_gv_set_clut);
EXPORT_SYMBOL(avia_gt_gv_set_input_mode);
EXPORT_SYMBOL(avia_gt_gv_set_input_size);
EXPORT_SYMBOL(avia_gt_gv_set_pos);
EXPORT_SYMBOL(avia_gt_gv_set_size);
EXPORT_SYMBOL(avia_gt_gv_hide);
EXPORT_SYMBOL(avia_gt_gv_show);
#endif

#if defined(MODULE) && defined(STANDALONE)
module_init(avia_gt_gv_init);
module_exit(avia_gt_gv_exit);
#endif
