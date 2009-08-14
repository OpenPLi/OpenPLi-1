/*
 *   avia_gt_pcm.h - pcm driver for AViA (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2002 Florian Schirmer (jolt@tuxbox.org)
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

#include "avia_gt.h"

#ifndef AVIA_GT_PCM_H
#define AVIA_GT_PCM_H

#define AVIA_GT_PCM_BUFFER_COUNT	25
#define AVIA_GT_PCM_MAX_SAMPLES		1023
#define AVIA_GT_PCM_MAX_BPS		4
#define AVIA_GT_PCM_BUFFER_SIZE		(AVIA_GT_PCM_MAX_SAMPLES * AVIA_GT_PCM_MAX_BPS)

#if (AVIA_GT_PCM_BUFFER_SIZE * AVIA_GT_PCM_BUFFER_COUNT) > AVIA_GT_MEM_PCM_SIZE
#error AViA pcm buffer exceeds AViA pcm memory pool!
#endif

unsigned int avia_gt_pcm_get_block_size(void);
void avia_gt_pcm_set_mpeg_attenuation(unsigned char left, unsigned char right);
void avia_gt_pcm_set_pcm_attenuation(unsigned char left, unsigned char right);
int avia_gt_pcm_set_rate(unsigned short rate);
int avia_gt_pcm_set_width(unsigned char width);
int avia_gt_pcm_set_channels(unsigned char channels);
int avia_gt_pcm_set_signed(unsigned char signed_samples);
int avia_gt_pcm_set_endian(unsigned char be);
int avia_gt_pcm_play_buffer(void *buffer, unsigned int buffer_size, unsigned char block);
unsigned int avia_gt_pcm_poll(struct file *file, struct poll_table_struct *wait);
void avia_gt_pcm_stop(void);

extern int avia_gt_pcm_init(void);
extern void avia_gt_pcm_exit(void);
	    
#endif
