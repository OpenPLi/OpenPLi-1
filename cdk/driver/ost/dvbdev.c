/* 
 * dvbdev.c
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

#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/kmod.h>

#include "dvbdev.h"

#ifdef MODULE
MODULE_DESCRIPTION("Device registrar for DVB drivers");
MODULE_AUTHOR("Marcus Metzler, Ralph Metzler");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL and additional rights");
#endif
#endif

#define DVB_MAJOR 250

static struct dvb_device *dvb_device[DVB_NUM_DEVICES];
static devfs_handle_t dvb_devfs_handle;

static inline struct dvb_device *
inode2dev (struct inode *inode)
{
        int minor=(MINOR(inode->i_rdev)>>6);
	
	return dvb_device[minor];
}

static inline int
inode2num(struct inode *inode)
{
        return (0x3f&MINOR(inode->i_rdev));
}

static ssize_t 
dvb_device_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
        struct inode *inode=file->f_dentry->d_inode;
        struct dvb_device *dvbdev=inode2dev(inode);

        if (!dvbdev)
	        return -ENODEV;
        return dvbdev->read(dvbdev, inode2num(inode), file, buf, count, ppos);
}

static ssize_t 
dvb_device_write(struct file *file, const char *buf, 
		 size_t count, loff_t *ppos)
{
        struct inode *inode=file->f_dentry->d_inode;
        struct dvb_device *dvbdev=inode2dev(inode);

        if (!dvbdev)
	        return -ENODEV;
        return dvbdev->write(dvbdev, inode2num(inode), file, buf, count, ppos);
}

static int 
dvb_device_open(struct inode *inode, struct file *file)
{
        struct dvb_device *dvbdev=inode2dev(inode);

        if (!dvbdev)
	        return -ENODEV;
        return dvbdev->open(dvbdev, inode2num(inode), inode, file);
}

static int 
dvb_device_release(struct inode *inode, struct file *file)
{
        struct dvb_device *dvbdev=inode2dev(inode);

        if (!dvbdev)
	        return -ENODEV;
        return dvbdev->close(dvbdev, inode2num(inode), inode, file);
}

static int 
dvb_device_ioctl(struct inode *inode, struct file *file,
		 unsigned int cmd, unsigned long arg)
{
        struct dvb_device *dvbdev=inode2dev(inode);

        if (!dvbdev)
	        return -ENODEV;
        return dvbdev->ioctl(dvbdev, inode2num(inode), file, cmd, arg);
}

static unsigned int 
dvb_device_poll(struct file *file, poll_table *wait)
{
        struct inode *inode=file->f_dentry->d_inode;
        struct dvb_device *dvbdev=inode2dev(inode);

        if (!dvbdev)
	        return -ENODEV;
        return dvbdev->poll(dvbdev, inode2num(inode), file, wait);
}


static struct file_operations dvb_device_fops =
{
	owner:		THIS_MODULE,
        read:		dvb_device_read,
	write:		dvb_device_write,
	ioctl:		dvb_device_ioctl,
	open:		dvb_device_open,
	release:	dvb_device_release,
	poll:		dvb_device_poll,
};


static char *dnames[] = { 
        "video", "audio", "sec", "frontend", "demux", "dvr", "ca",
	"net", "osd"
};


static void dvb_init_device(dvb_device_t *dev)
{
        int i, type;
	char name[64];

	sprintf(name, "card%d", dev->minor);
	dev->devfsh = devfs_mk_dir (dvb_devfs_handle, name, NULL);

	for (i=0; (type=dev->device_type(dev,i))>-2; i++) {
	        if (type==-1)
		        continue;

		sprintf(name, "%s%d", dnames[type>>2], type&3);
		devfs_register(dev->devfsh, name, DEVFS_FL_DEFAULT,
			       DVB_MAJOR, (dev->minor<<6)+i,
			       S_IFCHR | S_IRUSR | S_IWUSR,
			       &dvb_device_fops, dev);
	}

}

int dvb_register_device(dvb_device_t *dev)
{
	int i=0;

	for (i=0; i<DVB_NUM_DEVICES; i++) {
		if (dvb_device[i]==NULL) {
			dvb_device[i]=dev;
			dev->minor=i;
			dvb_init_device(dev);
			MOD_INC_USE_COUNT;
			return 0;
		}
	}
	return -ENFILE;
}

void dvb_unregister_device(dvb_device_t *dev)
{
        if (dvb_device[dev->minor]!=dev) {
		printk("dvbdev: bad unregister\n");
		return;
	}
        devfs_unregister(dev->devfsh);
        dvb_device[dev->minor]=NULL;
	MOD_DEC_USE_COUNT;
}

int __init dvbdev_init(void)
{
	int i=0;
	
	for(i=0; i<DVB_NUM_DEVICES; i++)
	        dvb_device[i]=NULL;
	dvb_devfs_handle = devfs_mk_dir (NULL, "dvb", NULL);
	if(devfs_register_chrdev(DVB_MAJOR,"DVB", &dvb_device_fops)) {
		printk("video_dev: unable to get major %d\n", DVB_MAJOR);
		return -EIO;
	}

	return 0;
}

#ifdef MODULE
static __init int 
init_dvbdev(void)
{
	return dvbdev_init();
}

static __exit void 
exit_dvbdev(void)
{
	devfs_unregister_chrdev(DVB_MAJOR, "DVB");
        devfs_unregister(dvb_devfs_handle);
}

module_init(init_dvbdev);
module_exit(exit_dvbdev);
#endif

EXPORT_SYMBOL(dvb_register_device);
EXPORT_SYMBOL(dvb_unregister_device);
