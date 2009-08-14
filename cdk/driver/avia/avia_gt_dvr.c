/*
 *   avia_gt_dvr.c - Queue-Insertion driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 Ronny "3des" Strutz (3des@tuxbox.org)
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
 *   $Log: avia_gt_dvr.c,v $
 *   Revision 1.9  2002/10/03 12:47:57  Jolt
 *   AViA AV cleanups
 *
 *   Revision 1.8  2002/09/21 00:02:05  Jolt
 *   Improved dvr - still buggy
 *
 *   Revision 1.7  2002/09/18 16:02:40  Jolt
 *   Mostly rewritten dvr driver
 *
 *   Revision 1.6  2002/08/22 14:18:58  Jolt
 *   DVR cleanups
 *
 *   Revision 1.5  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.4  2002/06/11 22:44:35  Jolt
 *   DVR fix
 *
 *   Revision 1.3  2002/06/11 22:37:18  Jolt
 *   DVR fixes
 *
 *   Revision 1.2  2002/06/11 22:12:52  Jolt
 *   DVR merge
 *
 *   Revision 1.1  2002/06/11 22:09:18  Jolt
 *   DVR driver added
 *
 *   Revision 1.0  2001/07/31 00:37:12  TripleDES
 *   - initial release
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/8xx_immap.h>
#include <asm/pgtable.h>
#include <asm/mpc8xx.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <linux/devfs_fs_kernel.h>

#include <dbox/avia_av.h>
#include <dbox/avia_gt.h>
#include <dbox/avia_gt_dmx.h>

extern void avia_set_pcr(u32 hi, u32 lo);

static sAviaGtInfo *gt_info = (sAviaGtInfo *)NULL;

static sAviaGtDmxQueue *video_queue;
static sAviaGtDmxQueue *audio_queue;

static u8 video_queue_nr;
static u8 audio_queue_nr;

static u8 video_pre_buffer = 1;
static u8 audio_pre_buffer = 1;

DECLARE_WAIT_QUEUE_HEAD(audio_wait);
DECLARE_WAIT_QUEUE_HEAD(video_wait);

static DECLARE_MUTEX_LOCKED(lock_open);
static DECLARE_MUTEX_LOCKED(alock_open);

static devfs_handle_t devfs_handle;
static ssize_t iframe_write (struct file *file, const char *buf, size_t count,loff_t *offset);

static devfs_handle_t adevfs_handle;
static ssize_t aiframe_write (struct file *file, const char *buf, size_t count,loff_t *offset);

static int iframe_open (struct inode *inode, struct file *file);
static int iframe_release (struct inode *inode, struct file *file);

static int aiframe_open (struct inode *inode, struct file *file);
static int aiframe_release (struct inode *inode, struct file *file);

static struct file_operations iframe_fops = {

	owner:		THIS_MODULE,
	write:		iframe_write,
	open:		iframe_open,
	release:	iframe_release,
	
};

static struct file_operations aiframe_fops = {

	owner:		THIS_MODULE,
	write:		aiframe_write,
	open:		aiframe_open,
	release:	aiframe_release,
	
};

void avia_gt_dvr_queue_irq_audio(u8 queue_nr, void *priv_data)
{

	wake_up_interruptible(&audio_wait);

}

void avia_gt_dvr_queue_irq_video(u8 queue_nr, void *priv_data)
{

	wake_up_interruptible(&video_wait);

}

static int iframe_open (struct inode *inode, struct file *file){

	unsigned int minor = MINOR(file->f_dentry->d_inode->i_rdev);
	s16 result;
	
	switch (minor) {
	
		case 0:
		
			if (file->f_flags & O_NONBLOCK)	{
			
				if (down_trylock(&lock_open))
					return -EAGAIN;
					
			} else {
			
				if (down_interruptible(&lock_open))
					return -ERESTARTSYS;
					
			}

			result = avia_gt_dmx_alloc_queue_video(avia_gt_dvr_queue_irq_video, NULL, NULL);
	
			if (result < 0) {
	
		        printk("avia_gt_dvr: can not allocate video queue\n");
					
		        return result;
	
			}
	
			video_queue_nr = result;

			video_queue = avia_gt_dmx_get_queue_info(video_queue_nr);
			video_queue->write_pos = 0;
			avia_gt_dmx_system_queue_set_pos(video_queue_nr, 0, 0);
			avia_gt_dmx_queue_irq_enable(video_queue_nr);

		    if (avia_gt_chip(ENX))
				enx_reg_set(CFGR0, VCP, 1);
			else if (avia_gt_chip(GTX))
				gtx_reg_set(CR1, VCP, 1);

			//wDR(AV_SYNC_MODE, 0);
			//wDR(VIDEO_PTS_DELAY, 0);
			//avia_flush_pcr();
			//avia_command(SetStreamType, 0x0B);
			//avia_command(NewChannel, 0, 0, 0);        
			
			video_pre_buffer = 1;
			
			return 0;
			
		break;
		
		default:
		
			return -ENODEV;
			
		break;
		
	}

	return 0;
	
}

static int iframe_release (struct inode *inode, struct file *file){

	unsigned int minor = MINOR(file->f_dentry->d_inode->i_rdev);
		
	switch (minor) {
	
		case 0:

			avia_gt_dmx_queue_irq_disable(video_queue_nr);
			avia_gt_dmx_free_queue(video_queue_nr);

		    if (avia_gt_chip(ENX))
				enx_reg_set(CFGR0, VCP, 0);
			else if (avia_gt_chip(GTX))
				gtx_reg_set(CR1, VCP, 0);

			up(&lock_open);

			return 0;
			
		break;
		
	}
	
	return -EINVAL;

}

static int aiframe_open (struct inode *inode, struct file *file){

	unsigned int minor = MINOR(file->f_dentry->d_inode->i_rdev);
	s16 result;

	switch (minor) {
	
		case 0:

			if (file->f_flags & O_NONBLOCK)	{
			
				if (down_trylock(&alock_open))
					return -EAGAIN;
					
			} else {
			
				if (down_interruptible(&alock_open))
					return -ERESTARTSYS;
					
			}

			result = avia_gt_dmx_alloc_queue_audio(avia_gt_dvr_queue_irq_audio, NULL, NULL);
	
			if (result < 0) {
	
		        printk("avia_gt_dvr: can not allocate audio queue\n");
		
		        return result;
	
			}

			audio_queue_nr = result;

			audio_queue = avia_gt_dmx_get_queue_info(audio_queue_nr);
			audio_queue->read_pos = 0;
			audio_queue->write_pos = 0;
			avia_gt_dmx_system_queue_set_pos(audio_queue_nr, 0, 0);
			avia_gt_dmx_queue_set_write_pos(audio_queue_nr, 0);
			avia_gt_dmx_queue_irq_enable(audio_queue_nr);

			if (avia_gt_chip(ENX))
				enx_reg_set(CFGR0, ACP, 1);
			else if (avia_gt_chip(GTX))
				gtx_reg_set(CR1, ACP, 1);

			wDR(AV_SYNC_MODE, 0);
			wDR(AUDIO_PTS_DELAY, 0);
			avia_flush_pcr();
			avia_command(SetStreamType, 0x0B);
			avia_command(NewChannel, 0, 0, 0);        
			
			audio_pre_buffer = 1;
			
			return 0;
			
		break;
		
		default:
		
			return -ENODEV;
			
		break;
		
	}

}

static int aiframe_release (struct inode *inode, struct file *file)
{

	unsigned int minor = MINOR(file->f_dentry->d_inode->i_rdev);
		
	switch (minor) {
	
		case 0:

			avia_gt_dmx_queue_irq_disable(audio_queue_nr);
			avia_gt_dmx_free_queue(audio_queue_nr);

			if (avia_gt_chip(ENX))
				enx_reg_set(CFGR0, ACP, 0);
			else if (avia_gt_chip(GTX))
				gtx_reg_set(CR1, ACP, 0);
	
			up(&alock_open);
			
			return 0;
			
		break;
		
	}
	
	return -EINVAL;
	
}

static ssize_t iframe_write (struct file *file, const char *buf, size_t count,loff_t *offset)
{

	u32 bytes_free = avia_gt_dmx_queue_get_bytes_free(video_queue_nr);

	while (bytes_free < count) {
	
		if (video_pre_buffer) {
		
			video_pre_buffer = 0;

			if (!audio_pre_buffer) {
			
				avia_gt_dmx_system_queue_set_write_pos(video_queue_nr, video_queue->write_pos);
				avia_gt_dmx_system_queue_set_write_pos(audio_queue_nr, audio_queue->write_pos);
			
			}
		
		}

		wait_event_interruptible(video_wait, (avia_gt_dmx_queue_get_bytes_free(video_queue_nr) >= count));
		
		bytes_free = avia_gt_dmx_queue_get_bytes_free(video_queue_nr);
		
	}
	
	count = video_queue->info.put_data(video_queue_nr, (void *)buf, count, 1);

	if (!video_pre_buffer)
		avia_gt_dmx_system_queue_set_write_pos(video_queue_nr, video_queue->write_pos);

	return count;
  
}        

static ssize_t aiframe_write (struct file *file, const char *buf, size_t count,loff_t *offset)
{

	u32 bytes_free = avia_gt_dmx_queue_get_bytes_free(audio_queue_nr);

/*	int						 i			= (int)0;
	u32						 ptc		= (u32)0;
	unsigned char	*buffer = (unsigned char *)buf;

		if(start)
		for(i=0;i<count-13;i++)
		{
			if(buffer[i]==0x00 && buffer[i+1]==0x00 && buffer[i+2]==0x01 &&
			 ((buffer[+7]&0xc0)==0xc0 ||	(buffer[+7]&0xc0)==0x80))
				if((buffer[i+9] & 0xF0) == 0x20 || (buffer[i+9] & 0xF0) == 0x30)
				{
					//for(x=i;x<i+14;x++)printk("%02x ",buffer[x]);
					//printk("\n");
					
					ptc = (buffer[i+13] & 0xfe) >> 1;
					ptc|=  buffer[i+12]<< 7;
					ptc|= (buffer[i+11]>> 1)<<15;
					ptc|=  buffer[i+10]<< 22;
					ptc|=(((buffer[i+9] & 0xe) >> 1)&0x7F )<<30;

					printk("%08x\n",ptc & 0xffffff);
					//avia_set_pcr((buffer[i+9] & 8)<<31|ptc>>1,(ptc & 1)<<31);
					start++;
					if(start>=1)start=0;
					break;
				}
		}			
*/

//	printk("begin: free %08d wp %08d rp %08d count %08d\n", avia_gt_dmx_queue_get_bytes_free(audio_queue_nr), audio_queue->write_pos, audio_queue->hw_read_pos, count);

	while (bytes_free < count) {
	
		if (audio_pre_buffer) {
		
			audio_pre_buffer = 0;
			
			if (!video_pre_buffer) {
			
				avia_gt_dmx_system_queue_set_write_pos(video_queue_nr, video_queue->write_pos);
				avia_gt_dmx_system_queue_set_write_pos(audio_queue_nr, audio_queue->write_pos);
			
			}
			
		}

		wait_event_interruptible(audio_wait, (avia_gt_dmx_queue_get_bytes_free(audio_queue_nr) >= count));
		
		bytes_free = avia_gt_dmx_queue_get_bytes_free(audio_queue_nr);
		
//		printk("sleep\n");
		
	}

//	printk("mid:   free %08d wp %08d rp %08d count %08d\n", avia_gt_dmx_queue_get_bytes_free(audio_queue_nr), audio_queue->write_pos, audio_queue->hw_read_pos, count);
	
	count = audio_queue->info.put_data(audio_queue_nr, (void *)buf, count, 1);

	if (!audio_pre_buffer)
		avia_gt_dmx_system_queue_set_write_pos(audio_queue_nr, audio_queue->write_pos);

//	printk("end:   free %08d wp %08d rp %08d count %08d\n", avia_gt_dmx_queue_get_bytes_free(audio_queue_nr), audio_queue->write_pos, audio_queue->hw_read_pos, count);

	return count;
	
}        

int __init avia_gt_dvr_init(void)
{

    printk("avia_gt_dvr: $Id: avia_gt_dvr.c,v 1.9 2002/10/03 12:47:57 Jolt Exp $\n");

    gt_info = avia_gt_get_info();
		
    if ((!gt_info) || ((!avia_gt_chip(ENX)) && (!avia_gt_chip(GTX)))) {
		
        printk("avia_gt_dvr: Unsupported chip type\n");
					
        return -EIO;
							
    }
	
    devfs_handle = devfs_register(NULL, "dvrv", DEVFS_FL_DEFAULT, 0, 0, S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, &iframe_fops, NULL);
    adevfs_handle = devfs_register(NULL, "dvra", DEVFS_FL_DEFAULT, 0, 0, S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, &aiframe_fops, NULL);
										
    if (avia_gt_chip(ENX)) {

    } else if (avia_gt_chip(GTX)) {

		gtx_reg_16(CR0) &= ~(1<<8);

	}

	up(&lock_open);
	up(&alock_open);

    return 0;
	
}

void __exit avia_gt_dvr_exit(void)
{

	devfs_unregister(devfs_handle);
 	devfs_unregister(adevfs_handle);

	down(&lock_open);
	down(&alock_open);

}

#ifdef MODULE
MODULE_AUTHOR("Ronny Strutz <3des@elitedvb.com>");
MODULE_DESCRIPTION("Video/Audio Playback Driver");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
#endif

#if defined(MODULE) 
//&& defined(STANDALONE)
module_init(avia_gt_dvr_init);
module_exit(avia_gt_dvr_exit);
#endif
