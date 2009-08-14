/* 
 * dvbdev.h
 *
 * Copyright (C) 2000 Ralph  Metzler <ralph@convergence.de>
 *                  & Marcus Metzler <marcus@convergence.de>
                      for convergence integrated media GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Lesser Public License
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

#ifndef _DVBDEV_H_
#define _DVBDEV_H_

#include <linux/types.h>
#include <linux/version.h>
#include <linux/poll.h>
#include <linux/devfs_fs_kernel.h>

#define DVB_NUM_DEVICES 16

struct dvb_device
{
	char name[32];
        int type;
	int hardware;

	void *priv;
	int minor;
	devfs_handle_t devfs_handle;

        int (*open)(struct dvb_device *, int, struct inode *, struct file *);
	int (*close)(struct dvb_device *, int, struct inode *, struct file *);
        ssize_t (*read)(struct dvb_device *, int, struct file *, char *, 
			size_t, loff_t *);
        ssize_t (*write)(struct dvb_device *, int, struct file *, const char *, 
			 size_t, loff_t *);
	int (*ioctl)(struct dvb_device *, int, struct file *, 
		     unsigned int , unsigned long);
        unsigned int (*poll)(struct dvb_device *, int type,
			     struct file *file, poll_table * wait);

        int (*device_type)(struct dvb_device *, unsigned int device_num);
#define DVB_DEVICE_VIDEO_0      0
#define DVB_DEVICE_AUDIO_0      4
#define DVB_DEVICE_SEC_0        8
#define DVB_DEVICE_FRONTEND_0  12
#define DVB_DEVICE_DEMUX_0     16
#define DVB_DEVICE_DEMUX_1     17
#define DVB_DEVICE_DEMUX_2     18
#define DVB_DEVICE_DEMUX_3     19
#define DVB_DEVICE_DVR_0       20
#define DVB_DEVICE_CA_0        24
#define DVB_DEVICE_NET_0       28
#define DVB_DEVICE_OSD_0       32
        devfs_handle_t devfsh;
};

typedef struct dvb_device dvb_device_t;

int dvb_register_device(struct dvb_device *);
void dvb_unregister_device(struct dvb_device *);

#endif /* #ifndef __DVBDEV_H */
