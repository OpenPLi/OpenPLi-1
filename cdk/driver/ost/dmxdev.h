/* 
 * dmxdev.h
 *
 * Copyright (C) 2000 Ralph  Metzler <ralph@convergence.de>
 *                  & Marcus Metzler <marcus@convergence.de>
                      for convergence integrated media GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef _DMXDEV_H_
#define _DMXDEV_H_

#ifndef __KERNEL__ 
#define __KERNEL__ 
#endif 

#ifdef __DVB_PACK__
#include <ost/demux.h>
#include <ost/dmx.h>
#else
#include <linux/ost/demux.h>
#include <linux/ost/dmx.h>
#endif
#include <linux/version.h>
#include <linux/wait.h>
#include <linux/types.h>
#include <linux/fs.h>

#if LINUX_VERSION_CODE < 0x020300
#define WAIT_QUEUE                 struct wait_queue*
#define init_waitqueue_head(wq)    *(wq) = NULL;
#define DECLARE_WAITQUEUE(wait, current) struct wait_queue wait = { current, NULL }
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)
#else
#define WAIT_QUEUE                 wait_queue_head_t
#endif

typedef enum {
	DMXDEV_TYPE_NONE,
	DMXDEV_TYPE_SEC,
	DMXDEV_TYPE_PES,
} dmxdev_type_t;

typedef enum {
	DMXDEV_STATE_FREE,
	DMXDEV_STATE_ALLOCATED,
	DMXDEV_STATE_SET,
	DMXDEV_STATE_GO,
	DMXDEV_STATE_DONE,
	DMXDEV_STATE_TIMEDOUT
} dmxdev_state_t;

typedef struct dmxdev_buffer_s {
        uint8_t *data;
        uint32_t size;
        int32_t  pread;
        int32_t  pwrite;
	WAIT_QUEUE queue;
        int error;
} dmxdev_buffer_t;


typedef struct dmxdev_filter_s {
        union {
	        dmx_pes_filter_t *pes;
	        dmx_section_filter_t *sec;
	} filter;

        union {
                dmx_ts_feed_t *ts;
                dmx_section_feed_t *sec;
	} feed;

        union {
	        struct dmxSctFilterParams sec;
	        struct dmxPesFilterParams pes;
	} params;

        int type;
        dmxdev_state_t state;
        struct dmxdev_s *dev;
        dmxdev_buffer_t buffer;

	struct semaphore mutex;

        // only for sections
        struct timer_list timer;
        int todo;
        uint8_t secheader[3];

        u16 pid;

	// for sending over network
	struct socket *s;
} dmxdev_filter_t;


typedef struct dmxdev_dvr_s {
        int state;
        struct dmxdev_s *dev;
        dmxdev_buffer_t buffer;
} dmxdev_dvr_t;


typedef struct dmxdev_s {
        dmxdev_filter_t *filter;
        dmxdev_dvr_t *dvr;
        dmx_demux_t *demux;

        int filternum;
        int capabilities;
#define DMXDEV_CAP_DUPLEX 1
        dmx_frontend_t *dvr_orig_fe;

        dmxdev_buffer_t dvr_buffer;
#define DVR_BUFFER_SIZE (512*1024)

	struct semaphore mutex;
	spinlock_t lock;
} dmxdev_t;


int DmxDevInit(dmxdev_t *dmxdev);
void DmxDevRelease(dmxdev_t *dmxdev);

int DmxDevFilterAlloc(dmxdev_t *dmxdev, struct file *file);
int DmxDevFilterFree(dmxdev_t *dmxdev, struct file *file);
int DmxDevFilterStart(dmxdev_filter_t *dmxdevfilter);
int DmxDevFilterStop(dmxdev_filter_t *dmxdevfilter);
int DmxDevIoctl(dmxdev_t *dmxdev, struct file *file, 
		unsigned int cmd, unsigned long arg);
unsigned int DmxDevPoll(dmxdev_t *dmxdev, struct file *file, poll_table * wait);
ssize_t DmxDevRead(dmxdev_t *dmxdev, struct file *file, 
		   char *buf, size_t count, loff_t *ppos);

int DmxDevDVROpen(dmxdev_t *dmxdev, struct file *file);
int DmxDevDVRClose(dmxdev_t *dmxdev, struct file *file);
ssize_t DmxDevDVRWrite(dmxdev_t *dmxdev, struct file *file, 
		       const char *buf, size_t count, loff_t *ppos);
ssize_t DmxDevDVRRead(dmxdev_t *dmxdev, struct file *file, 
		      char *buf, size_t count, loff_t *ppos);
int DmxDevDVRIoctl(dmxdev_t *dmxdev, struct file *file, 
		   unsigned int cmd, unsigned long arg);
unsigned int DmxDevDVRPoll(dmxdev_t *dmxdev, struct file *file, poll_table * wait);

void DmxNetSend(u8 *b1, size_t l_b1, u8 *b2, size_t l_b2, struct socket *s, dmxOutput_t otype, dmxdev_buffer_t *dvrbuf);
#endif /* _DMXDEV_H_ */
