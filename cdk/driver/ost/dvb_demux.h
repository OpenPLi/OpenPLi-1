/* 
 * dvb_demux.h - DVB kernel demux API
 *
 * Copyright (C) 2000-2001 Marcus Metzler <marcus@convergence.de>
 *                       & Ralph  Metzler <ralph@convergence.de>
 *                         for convergence integrated media GmbH
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

#ifndef _DVB_DEMUX_H_
#define _DVB_DEMUX_H_

#if LINUX_VERSION_CODE < 0x020300
#define WAIT_QUEUE                 struct wait_queue*
#define init_waitqueue_head(wq)    *(wq) = NULL;
#define DECLARE_WAITQUEUE(wait, current) struct wait_queue wait = { current, NULL }
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)
#else
#define WAIT_QUEUE                 wait_queue_head_t
#endif

#ifdef __DVB_PACK__
#include <ost/demux.h>
#else
#include <linux/ost/demux.h>
#endif

#define DMX_TYPE_TS  0
#define DMX_TYPE_SEC 1
#define DMX_TYPE_PES 2

#define DMX_STATE_FREE      0
#define DMX_STATE_ALLOCATED 1
#define DMX_STATE_SET       2
#define DMX_STATE_READY     3
#define DMX_STATE_GO        4

#define DVB_DEMUX_MASK_MAX 18

typedef struct dvb_demux_filter_s {
        dmx_section_filter_t filter;
        struct dvb_demux_filter_s *next;
        struct dvb_demux_feed_s *feed;
        int index;
        int state;
        int type;
	int pesto;

        u32 flags;
        u16 handle;
        u16 hw_handle;
        struct timer_list timer;
	int ts_state;

        u16 pid;  //to be removed
} dvb_demux_filter_t;

typedef struct dvb_demux_feed_s {
        union {
	        dmx_ts_feed_t ts;
	        dmx_section_feed_t sec;
	        dmx_pes_feed_t pes;
	} feed;

        union {
	        dmx_ts_cb ts;
	        dmx_section_cb sec;
	        dmx_pes_cb pes;
	} cb;

        struct dvb_demux_s *demux;
        int type;
        int state;
        u16 pid;
        u8 *buffer;
        int buffer_size;
        int descramble;
        int check_crc;

        struct timespec timeout; 
        dvb_demux_filter_t *filter;
        int cb_length;
  
        int ts_type;
        dmx_ts_pes_t pes_type;

        u8 secbuf[4096];
        int secbufp;
        int seclen;
        int cc;

        u16 peslen;
} dvb_demux_feed_t;

typedef struct dvb_demux_s {
        dmx_demux_t dmx;
        void *priv;
        int filternum;
        int feednum;
        int (*start_feed)(dvb_demux_feed_t *);
        int (*stop_feed)(dvb_demux_feed_t *);
        int (*write_to_decoder)(dvb_demux_feed_t *, u8 *, size_t);

  
        int users;
#define MAX_DVB_DEMUX_USERS 10
        dvb_demux_filter_t *filter;
        dvb_demux_feed_t *feed;

        struct list_head frontend_list;

        dvb_demux_feed_t *pesfilter[DMX_TS_PES_OTHER];
        u16 pids[DMX_TS_PES_OTHER];
        int playing; 
        int recording; 

#define DMX_MAX_PID 0x2000
        dvb_demux_feed_t *pid2feed[DMX_MAX_PID+1];
        u8 tsbuf[188];
        int tsbufp;

	struct semaphore mutex;
	spinlock_t lock;
} dvb_demux_t;


int DvbDmxInit(dvb_demux_t *dvbdemux);
int DvbDmxRelease(dvb_demux_t *dvbdemux);
void DvbDmxSWFilterPackets(dvb_demux_t *dvbdmx, const u8 *buf, int count);

#endif /* _DVB_DEMUX_H_ */
