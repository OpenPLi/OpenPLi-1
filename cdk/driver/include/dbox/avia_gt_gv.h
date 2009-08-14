/*
 *   enx_gv.h - AViA eNX graphic viewport driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 Florian Schirmer (jolt@tuxbox.org)
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
 *   $Log: avia_gt_gv.h,v $
 *   Revision 1.8  2002/10/09 18:31:12  Jolt
 *   HW copy support
 *
 *   Revision 1.7  2002/04/25 22:10:39  Jolt
 *   FB cleanup
 *
 *   Revision 1.6  2002/04/24 19:56:00  Jolt
 *   GV driver updates
 *
 *   Revision 1.5  2002/04/21 14:36:07  Jolt
 *   Merged GTX fb support
 *
 *   Revision 1.4  2002/04/17 21:50:57  Jolt
 *   Capture driver fixes
 *
 *   Revision 1.3  2002/04/15 10:40:50  Jolt
 *   eNX/GTX
 *
 *   Revision 1.2  2002/04/15 04:44:24  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.1  2001/11/01 18:19:09  Jolt
 *   graphic viewport driver added
 *
 *
 *   $Revision: 1.8 $
 *
 */

#ifndef AVIA_GT_GV_H
#define AVIA_GT_GV_H

#define AVIA_GT_GV_INPUT_MODE_OFF	0x00
#define AVIA_GT_GV_INPUT_MODE_RGB4	0x01
#define AVIA_GT_GV_INPUT_MODE_RGB8	0x02
#define AVIA_GT_GV_INPUT_MODE_RGB16	0x03
#define AVIA_GT_GV_INPUT_MODE_RGB32	0x04

void avia_gt_gv_copyarea(u16 src_x, u16 src_y, u16 width, u16 height, u16 dst_x, u16 dst_y);
void avia_gt_gv_cursor_hide(void);
void avia_gt_gv_cursor_show(void);
void avia_gt_gv_get_clut(u8 clut_nr, u32 *transparency, u32 *red, u32 *green, u32 *blue);
u16 avia_gt_gv_get_stride(void);
void avia_gt_gv_get_info(u8 **gv_mem_phys, u8 **gv_mem_lin, u32 *gv_mem_size);
void avia_gt_gv_hide(void);
void avia_gt_gv_set_blevel(u8 class0, u8 class1);
void avia_gt_gv_set_clut(u8 clut_nr, u32 transparency, u32 red, u32 green, u32 blue);
int avia_gt_gv_set_input_mode(u8 mode);
int avia_gt_gv_set_input_size(u16 width, u16 height);
int avia_gt_gv_set_pos(u16 x, u16 y);
void avia_gt_gv_set_size(u16 width, u16 height);
int avia_gt_gv_show(void);

extern int avia_gt_gv_init(void);
extern void avia_gt_gv_exit(void);

#endif
