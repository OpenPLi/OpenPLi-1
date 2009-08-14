/*
 *   avia_gt_capture.h - capture driver for eNX/GTX (dbox-II-project)
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

#ifndef AVIA_GT_CAPTURE_H
#define AVIA_GT_CAPTURE_H

#define AVIA_GT_CAPTURE_MAX_OUTPUT_X		(720 / 2)
#define AVIA_GT_CAPTURE_MAX_OUTPUT_Y		(576 / 2)

#define AVIA_GT_CAPTURE_SET_INPUT_POS		1
#define AVIA_GT_CAPTURE_SET_INPUT_SIZE		2
#define AVIA_GT_CAPTURE_SET_OUTPUT_SIZE		3
#define AVIA_GT_CAPTURE_START			4
#define AVIA_GT_CAPTURE_STOP			5

#define capture_set_input_pos(fd, x, y)			ioctl(fd, AVIA_GT_CAPTURE_SET_INPUT_POS, ((x & 0xFFFF) | ((y & 0xFFFF) << 16)))
#define capture_set_input_size(fd, width, height)	ioctl(fd, AVIA_GT_CAPTURE_SET_INPUT_SIZE, ((width & 0xFFFF) | ((height & 0xFFFF) << 16)))
#define capture_set_output_size(fd, width, height)	ioctl(fd, AVIA_GT_CAPTURE_SET_OUTPUT_SIZE, ((width & 0xFFFF) | ((height & 0xFFFF) << 16)))
#define capture_start(fd)				ioctl(fd, AVIA_GT_CAPTURE_START, 0)
#define capture_stop(fd)				ioctl(fd, AVIA_GT_CAPTURE_STOP, 0)

extern int avia_gt_capture_set_input_pos(unsigned short x, unsigned short y, unsigned char pig);
extern int avia_gt_capture_set_input_size(unsigned short width, unsigned short height, unsigned char pig);
extern int avia_gt_capture_set_output_size(unsigned short width, unsigned short height, unsigned char pig);
extern int avia_gt_capture_start(unsigned char **capture_buffer, unsigned short *stride, unsigned char pig);
extern void avia_gt_capture_stop(unsigned char pig);

extern int avia_gt_capture_init(void);
extern void avia_gt_capture_exit(void);

#endif
