/*
 *   avia_gt_pcm.c - AViA eNX/GTX pcm driver (dbox-II-project)
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
 *
 *   $Log: avia_gt_pcm.c,v $
 *   Revision 1.19  2002/10/03 11:12:42  thegoodguy
 *   Reenable full volume
 *
 *   Revision 1.18  2002/09/25 18:50:52  Jolt
 *   Added 24000 and 12000 sample rate support
 *
 *   Revision 1.17  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.16  2002/08/19 00:02:01  TheDOC
 *   export the poll-stuff
 *
 *   Revision 1.15  2002/08/18 18:22:30  tmbinc
 *   added poll()-support for pcm device (untested)
 *
 *   Revision 1.14  2002/06/07 18:06:03  Jolt
 *   GCC31 fixes 2nd shot (GTX version) - sponsored by Frankster (THX!)
 *
 *   Revision 1.13  2002/06/07 17:53:45  Jolt
 *   GCC31 fixes 2nd shot - sponsored by Frankster (THX!)
 *
 *   Revision 1.12  2002/05/08 12:56:41  obi
 *   export avia_gt_pcm_set_mpeg_attenuation
 *   plus some cleanup
 *
 *   Revision 1.11  2002/04/22 17:40:01  Jolt
 *   Major cleanup
 *
 *   Revision 1.10  2002/04/14 18:06:19  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.9  2002/04/13 23:19:05  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.8  2002/04/13 21:00:28  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.7  2002/04/13 14:47:19  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.6  2002/04/12 13:50:37  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.5  2002/04/10 21:53:31  Jolt
 *   Further cleanups/bugfixes
 *   More OSS API stuff

 *   Revision 1.4  2002/04/05 23:15:13  Jolt
 *   Improved buffer management - MP3 is rocking solid now
 *
 *   Revision 1.3  2002/04/02 18:14:10  Jolt
 *   Further features/bugfixes. MP3 works very well now 8-)
 *
 *   Revision 1.2  2002/04/02 13:56:50  Jolt
 *   Dependency fixes
 *
 *   Revision 1.1  2002/04/01 22:23:22  Jolt
 *   Basic PCM driver for eNX - more to come later
 *
 *
 *
 *   $Revision: 1.19 $
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/byteorder/swab.h>

#include <dbox/avia_gt.h>
#include <dbox/avia_gt_pcm.h>

DECLARE_WAIT_QUEUE_HEAD(pcm_wait);

typedef struct {

	struct list_head list;
	unsigned int offset;
	unsigned int sample_count;
	unsigned char queued;

} sPcmBuffer;

LIST_HEAD(pcm_busy_buffer_list);
LIST_HEAD(pcm_free_buffer_list);

spinlock_t busy_buffer_lock = SPIN_LOCK_UNLOCKED;
spinlock_t free_buffer_lock = SPIN_LOCK_UNLOCKED;

static sAviaGtInfo *gt_info = (sAviaGtInfo *)NULL;
unsigned char swab_samples	= (unsigned char)0;
sPcmBuffer pcm_buffer_array[AVIA_GT_PCM_BUFFER_COUNT];
unsigned char swab_buffer[AVIA_GT_PCM_BUFFER_SIZE] = { 0 };

// Warning - result is _per_ channel
unsigned int avia_gt_pcm_calc_sample_count(unsigned int buffer_size)
{

	if (avia_gt_chip(ENX)) {

		if (enx_reg_s(PCMC)->W)
			buffer_size /= 2;

		if (enx_reg_s(PCMC)->C)
			buffer_size /= 2;

	} else if (avia_gt_chip(GTX)) {

		if (gtx_reg_s(PCMC)->W)
			buffer_size /= 2;

		if (gtx_reg_s(PCMC)->C)
			buffer_size /= 2;

	}

	return buffer_size;

}

// Warning - if stereo result is for _both_ channels
unsigned int avia_gt_pcm_calc_buffer_size(unsigned int sample_count)
{

	if (avia_gt_chip(ENX)) {

		if (enx_reg_s(PCMC)->W)
			sample_count *= 2;

		if (enx_reg_s(PCMC)->C)
			sample_count *= 2;

	} else if (avia_gt_chip(GTX)) {

		if (gtx_reg_s(PCMC)->W)
			sample_count *= 2;

		if (gtx_reg_s(PCMC)->C)
			sample_count *= 2;

	}

	return sample_count;

}

void avia_gt_pcm_queue_buffer(void)
{

	unsigned long			 flags			= (unsigned long)0;
	sPcmBuffer				*pcm_buffer	= (sPcmBuffer *)NULL;
	struct list_head	*ptr				= (struct list_head *)NULL;

	if (avia_gt_chip(ENX)) {

		if (!enx_reg_s(PCMA)->W)
			return;

	} else if (avia_gt_chip(GTX)) {

		if (!gtx_reg_s(PCMA)->W)
			return;

	}

	spin_lock_irqsave(&busy_buffer_lock, flags);

	list_for_each(ptr, &pcm_busy_buffer_list) {

		pcm_buffer = list_entry(ptr, sPcmBuffer, list);

		if (!pcm_buffer->queued) {

			if (avia_gt_chip(ENX)) {

				enx_reg_set(PCMS, NSAMP, pcm_buffer->sample_count);
				enx_reg_set(PCMA, Addr, pcm_buffer->offset >> 1);
				enx_reg_set(PCMA, W, 0);

			} else if (avia_gt_chip(GTX)) {

				gtx_reg_set(PCMA, NSAMP, pcm_buffer->sample_count);
				gtx_reg_set(PCMA, Addr, pcm_buffer->offset >> 1);
				gtx_reg_set(PCMA, W, 0);

			}

			pcm_buffer->queued = 1;

			break;

		}

	}

	spin_unlock_irqrestore(&busy_buffer_lock, flags);

}

static void avia_gt_pcm_irq(unsigned short irq)
{

	unsigned long		 flags			= (unsigned long)0;
	sPcmBuffer			*pcm_buffer	= (sPcmBuffer *)NULL;
	//int i = 0;
	//struct list_head *ptr;

	spin_lock_irqsave(&busy_buffer_lock, flags);

/*
	if ((irq == ENX_IRQ_PCM_AD) || (irq == GTX_IRQ_PCM_AD))
		printk("X");

	list_for_each(ptr, &pcm_busy_buffer_list) {

		i++;

	}

	printk("%d ", i);
*/

	if (!list_empty(&pcm_busy_buffer_list)) {

		pcm_buffer = list_entry(pcm_busy_buffer_list.next, sPcmBuffer, list);
		list_del(&pcm_buffer->list);

		pcm_buffer->queued = 0;

		spin_lock_irqsave(&free_buffer_lock, flags);

		list_add_tail(&pcm_buffer->list, &pcm_free_buffer_list);

		spin_unlock_irqrestore(&free_buffer_lock, flags);

	}

	spin_unlock_irqrestore(&busy_buffer_lock, flags);

	avia_gt_pcm_queue_buffer();

	wake_up_interruptible(&pcm_wait);

}

unsigned int avia_gt_pcm_get_block_size(void)
{

	return avia_gt_pcm_calc_buffer_size(AVIA_GT_PCM_MAX_SAMPLES);

}

void avia_gt_pcm_reset(unsigned char reenable)
{

	if (avia_gt_chip(ENX)) {

		enx_reg_set(RSTR0, PCMA, 1);
		enx_reg_set(RSTR0, PCM, 1);

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_set(RR0, ACLK, 1);
		gtx_reg_set(RR0, PCM, 1);

	}

	if (reenable) {

		if (avia_gt_chip(ENX)) {

			enx_reg_set(RSTR0, PCM, 0);
			enx_reg_set(RSTR0, PCMA, 0);

		} else if (avia_gt_chip(GTX)) {

			gtx_reg_set(RR0, PCM, 0);
			gtx_reg_set(RR0, ACLK, 0);

		}

	}

}

void avia_gt_pcm_set_mpeg_attenuation(unsigned char left, unsigned char right)
{

	if (avia_gt_chip(ENX)) {

		enx_reg_set(PCMN, MPEGAL, left >> 1);
		enx_reg_set(PCMN, MPEGAR, right >> 1);

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_set(PCMN, MPEGAL, left >> 1);
		gtx_reg_set(PCMN, MPEGAR, right >> 1);

	}

}

void avia_gt_pcm_set_pcm_attenuation(unsigned char left, unsigned char right)
{

	if (avia_gt_chip(ENX)) {

		enx_reg_set(PCMN, PCMAL, left >> 1);
		enx_reg_set(PCMN, PCMAR, right >> 1);

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_set(PCMN, PCMAL, left >> 1);
		gtx_reg_set(PCMN, PCMAR, right >> 1);

	}

}

int avia_gt_pcm_set_rate(unsigned short rate)
{

	unsigned char divider_mode = 3;

	switch(rate) {

		case 48000:
		case 44100:

			divider_mode = 3;
		
		break;

		case 24000:
		case 22050:

			divider_mode = 2;
		
		break;

		case 12000:
		case 11025:

			divider_mode = 1;
		
		break;

		default:

			return -EINVAL;
			
		break;

	}

	if (avia_gt_chip(ENX))
		enx_reg_set(PCMC, R, divider_mode);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(PCMC, R, divider_mode);

	return 0;

}

int avia_gt_pcm_set_width(unsigned char width)
{

	if ((width == 8) || (width == 16)) {

		if (avia_gt_chip(ENX))
			enx_reg_set(PCMC, W, (width == 16));
		else if (avia_gt_chip(GTX))
			gtx_reg_set(PCMC, W, (width == 16));

	} else {

		return -EINVAL;

	}

	return 0;

}

int avia_gt_pcm_set_channels(unsigned char channels)
{

	if ((channels == 1) || (channels == 2)) {

		if (avia_gt_chip(ENX))
			enx_reg_set(PCMC, C, (channels == 2));
		else if (avia_gt_chip(GTX))
			gtx_reg_set(PCMC, C, (channels == 2));

	} else {

		return -EINVAL;

	}

	return 0;

}

int avia_gt_pcm_set_signed(unsigned char signed_samples)
{

	if ((signed_samples == 0) || (signed_samples == 1)) {

		if (avia_gt_chip(ENX))
			enx_reg_set(PCMC, S, (signed_samples == 1));
		else if (avia_gt_chip(GTX))
			gtx_reg_set(PCMC, S, (signed_samples == 1));

	} else {

		return -EINVAL;

	}

	return 0;

}

int avia_gt_pcm_set_endian(unsigned char be)
{

	if ((be == 0) || (be == 1))
		swab_samples = (be == 0);
	else
		return -EINVAL;

	return 0;

}

int avia_gt_pcm_play_buffer(void *buffer, unsigned int buffer_size, unsigned char block) {

	unsigned char bps_16      = (char)'\0';
	unsigned long flags       = (unsigned long)0;
	sPcmBuffer *pcm_buffer    = (sPcmBuffer *)NULL;
	unsigned int sample_nr    = (unsigned int)0;
	unsigned short *swab_dest = (unsigned short *)NULL;
	unsigned short *swab_src  = (unsigned short *)NULL;
	unsigned int sample_count = (unsigned int)0;
	unsigned char stereo      = (unsigned char)'\0';

	sample_count = avia_gt_pcm_calc_sample_count(buffer_size);

	if (sample_count > AVIA_GT_PCM_MAX_SAMPLES)
		sample_count = AVIA_GT_PCM_MAX_SAMPLES;

	if (avia_gt_chip(ENX)) {

		bps_16 = enx_reg_s(PCMC)->W;
		stereo = enx_reg_s(PCMC)->C;

	} else if (avia_gt_chip(GTX)) {

		bps_16 = gtx_reg_s(PCMC)->W;
		stereo = gtx_reg_s(PCMC)->C;

	}

	// If 8-bit mono then sample count has to be even
	if ((!bps_16) && (!stereo))
		sample_count &= ~1;

	while (list_empty(&pcm_free_buffer_list)) {

		if (block) {

			if (wait_event_interruptible(pcm_wait, !list_empty(&pcm_free_buffer_list)))
				return -ERESTARTSYS;

		} else {

			return -EWOULDBLOCK;

		}

	}

	spin_lock_irqsave(&free_buffer_lock, flags);

	pcm_buffer = list_entry(pcm_free_buffer_list.next, sPcmBuffer, list);
	list_del(&pcm_buffer->list);

	spin_unlock_irqrestore(&free_buffer_lock, flags);

	if ((bps_16) && (swab_samples)) {

		copy_from_user(swab_buffer, buffer, avia_gt_pcm_calc_buffer_size(sample_count));

		swab_dest = (unsigned short *)(gt_info->mem_addr + pcm_buffer->offset);
		swab_src = (unsigned short *)swab_buffer;

		for (sample_nr = 0; sample_nr < avia_gt_pcm_calc_buffer_size(sample_count) / 2; sample_nr++)
			swab_dest[sample_nr] = swab16(swab_src[sample_nr]);

	} else {

		copy_from_user(gt_info->mem_addr + pcm_buffer->offset, buffer, avia_gt_pcm_calc_buffer_size(sample_count));

	}

	pcm_buffer->sample_count = sample_count;

	spin_lock_irqsave(&busy_buffer_lock, flags);

	list_add_tail(&pcm_buffer->list, &pcm_busy_buffer_list);

	spin_unlock_irqrestore(&busy_buffer_lock, flags);

	avia_gt_pcm_queue_buffer();

	return avia_gt_pcm_calc_buffer_size(sample_count);

}

unsigned int avia_gt_pcm_poll(struct file *file, struct poll_table_struct *wait)
{
	poll_wait(file, &pcm_wait, wait);
	if (!list_empty(&pcm_free_buffer_list))
		return POLLOUT|POLLWRNORM;
	return 0;
}

void avia_gt_pcm_stop(void)
{
/*
	if (avia_gt_chip(ENX))
		enx_reg_set(PCMC, T, 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(PCMC, T, 1);
*/
	return;
}

int avia_gt_pcm_init(void)
{

	unsigned char  buf_nr  = (unsigned char)'\0';
	unsigned short irq_ad  = (unsigned short)0;
	unsigned short irq_pf  = (unsigned short)0;

	printk("avia_gt_pcm: $Id: avia_gt_pcm.c,v 1.19 2002/10/03 11:12:42 thegoodguy Exp $\n");

	gt_info = avia_gt_get_info();

	if ((!gt_info) || ((!avia_gt_chip(ENX)) && (!avia_gt_chip(GTX)))) {

		printk("avia_gt_pcm: Unsupported chip type\n");

		return -EIO;

	}

	if (avia_gt_chip(ENX)) {

		irq_ad = ENX_IRQ_PCM_AD;
		irq_pf = ENX_IRQ_PCM_PF;

	} else if (avia_gt_chip(GTX)) {

		irq_ad = GTX_IRQ_PCM_AD;
		irq_pf = GTX_IRQ_PCM_PF;

	}

	if (avia_gt_alloc_irq(irq_ad, avia_gt_pcm_irq)) {

		printk("avia_gt_pcm: unable to get pcm-ad interrupt\n");

		return -EIO;

	}

	if (avia_gt_alloc_irq(irq_pf, avia_gt_pcm_irq)) {

		printk("avia_gt_pcm: unable to get pcm-pf interrupt\n");

		avia_gt_free_irq(irq_ad);

		return -EIO;

	}

	avia_gt_pcm_reset(1);

	for (buf_nr = 0; buf_nr < AVIA_GT_PCM_BUFFER_COUNT; buf_nr++) {

		pcm_buffer_array[buf_nr].offset = AVIA_GT_MEM_PCM_OFFS + (AVIA_GT_PCM_BUFFER_SIZE * buf_nr);
		pcm_buffer_array[buf_nr].queued = 0;

		list_add_tail(&pcm_buffer_array[buf_nr].list, &pcm_free_buffer_list);

	}

	// Use external clock from AViA 500/600
	if (avia_gt_chip(ENX))
		enx_reg_set(PCMC, I, 0);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(PCMC, I, 0);

	// Pass through mpeg samples
	avia_gt_pcm_set_mpeg_attenuation(0x80, 0x80);

	// Set a default mode
	avia_gt_pcm_set_rate(44100);
	avia_gt_pcm_set_width(16);
	avia_gt_pcm_set_channels(2);
	avia_gt_pcm_set_signed(1);
	avia_gt_pcm_set_endian(1);

	return 0;

}

void avia_gt_pcm_exit(void)
{

	if (avia_gt_chip(ENX)) {

		avia_gt_free_irq(ENX_IRQ_PCM_AD);
		avia_gt_free_irq(ENX_IRQ_PCM_PF);

	} else if (avia_gt_chip(GTX)) {

		avia_gt_free_irq(GTX_IRQ_PCM_AD);
		avia_gt_free_irq(GTX_IRQ_PCM_PF);

	}

	avia_gt_pcm_reset(0);

}

#ifdef MODULE
EXPORT_SYMBOL(avia_gt_pcm_play_buffer);
EXPORT_SYMBOL(avia_gt_pcm_stop);
EXPORT_SYMBOL(avia_gt_pcm_set_signed);
EXPORT_SYMBOL(avia_gt_pcm_set_endian);
EXPORT_SYMBOL(avia_gt_pcm_set_rate);
EXPORT_SYMBOL(avia_gt_pcm_set_width);
EXPORT_SYMBOL(avia_gt_pcm_set_channels);
EXPORT_SYMBOL(avia_gt_pcm_set_mpeg_attenuation);
EXPORT_SYMBOL(avia_gt_pcm_set_pcm_attenuation);
EXPORT_SYMBOL(avia_gt_pcm_get_block_size);
EXPORT_SYMBOL(avia_gt_pcm_poll);
#endif

#if defined(MODULE) && defined(STANDALONE)
module_init(avia_gt_pcm_init);
module_exit(avia_gt_pcm_exit);
#endif
