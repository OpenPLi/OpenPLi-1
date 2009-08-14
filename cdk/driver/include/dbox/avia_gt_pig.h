/*
 *   avia_gt_pig.h - pig driver for AViA eNX/GTX (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2001-2002 Florian Schirmer <jolt@tuxbox.org>
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
 */

extern int avia_gt_pig_hide(unsigned char pig_nr);
extern int avia_gt_pig_set_pos(unsigned char pig_nr, unsigned short x, unsigned short y);
extern int avia_gt_pig_set_size(unsigned char pig_nr, unsigned short width, unsigned short height, unsigned char stretch);
extern int avia_gt_pig_set_stack(unsigned char pig_nr, unsigned char stack_order);
extern int avia_gt_pig_show(unsigned char pig_nr);

extern int avia_gt_pig_init(void);
extern void avia_gt_pig_exit(void);

#define AVIA_PIG_HIDE	 	1
#define AVIA_PIG_SET_POS 	2
#define AVIA_PIG_SET_SIZE 	3
#define AVIA_PIG_SET_SOURCE 	4
#define AVIA_PIG_SET_STACK 	5
#define AVIA_PIG_SHOW	 	6

#define avia_pig_hide(fd) 		ioctl(fd, AVIA_PIG_HIDE, 0)
#define avia_pig_set_pos(fd, x, y) 	ioctl(fd, AVIA_PIG_SET_POS, (y | (x << 16)))
#define avia_pig_set_size(fd, x, y) 	ioctl(fd, AVIA_PIG_SET_SIZE, (y | (x << 16)))
#define avia_pig_set_source(fd, x, y) 	ioctl(fd, AVIA_PIG_SET_SOURCE, (y | (x << 16)))
#define avia_pig_set_stack(fd, order) 	ioctl(fd, AVIA_PIG_SET_STACK, order)
#define avia_pig_show(fd) 		ioctl(fd, AVIA_PIG_SHOW, 0)
