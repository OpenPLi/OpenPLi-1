/*
 *   avia_osd.c - AViA OSD driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2001 Gillem (htoa@gmx.net)
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
 *   $Log: avia_av_osd.c,v $
 *   Revision 1.14  2002/10/03 12:47:57  Jolt
 *   AViA AV cleanups
 *
 *   Revision 1.13  2002/10/01 20:22:59  Jolt
 *   Cleanups
 *
 *   Revision 1.12  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.11  2002/05/09 07:29:21  waldi
 *   add correct license
 *
 *   Revision 1.10  2002/05/07 16:59:19  Jolt
 *   Misc stuff and cleanups
 *
 *   Revision 1.9  2001/12/01 06:37:06  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.8  2001/03/07 19:18:38  gillem
 *   - fix 600er bug
 *
 *   Revision 1.7  2001/03/06 21:49:50  gillem
 *   - some rewrites
 *
 *   Revision 1.6  2001/03/04 22:17:46  gillem
 *   - add read function ... avia500 multiframe not work
 *
 *   Revision 1.5  2001/03/04 20:04:41  gillem
 *   - show 16 frames ... avia600 only ????
 *
 *   Revision 1.4  2001/03/04 17:57:13  gillem
 *   - add more frames, not work !
 *
 *   Revision 1.3  2001/03/03 00:24:44  gillem
 *   - not ready ...
 *
 *   Revision 1.2  2001/02/22 22:49:08  gillem
 *   - add functions
 *
 *   Revision 1.1  2001/02/22 15:30:59  gillem
 *   - initial release
 *
 *
 *   $Revision: 1.14 $
 *
 */

/* ---------------------------------------------------------------------- */

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

#include <dbox/avia_av.h>
#include <dbox/avia_av_osd.h>

#include <linux/devfs_fs_kernel.h>

#ifndef CONFIG_DEVFS_FS
#error no devfs
#endif

/* ---------------------------------------------------------------------- */

static int osd_ioctl (struct inode *inode, struct file *file,
                         unsigned int cmd, unsigned long arg);
static int osd_open (struct inode *inode, struct file *file);

/* ---------------------------------------------------------------------- */

static devfs_handle_t devfs_handle;

static struct file_operations osd_fops = {
	owner:		THIS_MODULE,
	ioctl:		osd_ioctl,
	open:		osd_open,
};

/* ---------------------------------------------------------------------- */

typedef struct osd_header {

	u32 res01 	: 8   __attribute__((packed));
	u32 next  	: 22  __attribute__((packed));	// next frame pointer
	u32 res02 	: 2   __attribute__((packed));

	u32 res03 	: 8   __attribute__((packed));
	u32 bmsize1 : 18  __attribute__((packed));
	u32 n1 		: 1   __attribute__((packed)); 	// set to 1
	u32 res05 	: 5   __attribute__((packed));

	u32 res06 	: 14  __attribute__((packed));
	u32 n2 		: 1   __attribute__((packed)); 	// set to 1
	u32 res08 	: 3   __attribute__((packed));
	u32 n3 		: 1   __attribute__((packed)); 	// set to 1
	u32 res10 	: 2   __attribute__((packed));
	u32 bmsize2 : 9   __attribute__((packed)); 	// 8:0 bms
	u32 res11 	: 2   __attribute__((packed));

	u32 res12 	: 8   __attribute__((packed));
	u32 bmp 	: 22  __attribute__((packed));
	u32 res13 	: 2   __attribute__((packed));

	u32 res14 	: 23  __attribute__((packed));
	u32 peltyp	: 2   __attribute__((packed));
	u32 n4 		: 1   __attribute__((packed)); 	// set to 1
	u32 gbf 	: 5   __attribute__((packed));
	u32 n5 		: 1   __attribute__((packed)); 	// set to 1

	u32 res17 	: 12  __attribute__((packed));
	u32 colstart: 10  __attribute__((packed));
	u32 colstop : 10  __attribute__((packed));

	u32 res18 	: 12  __attribute__((packed));
	u32 rowstart: 10  __attribute__((packed));
	u32 rowstop : 10  __attribute__((packed));

	u32 res19 	: 8   __attribute__((packed));
	u32 palette : 22  __attribute__((packed));
	u32 res20 	: 2   __attribute__((packed));
} osd_header;

#define OSDH_SIZE sizeof(osd_header)

typedef struct osd_font {
	u16 width;
	u16 height;
	u8 *font;
} osd_font;

typedef struct osd_palette {
	u16 size;
	u32 *palette;
} osd_palette;

typedef struct osd_bitmap {
	u16 size;
	u32 *bitmap;
} osd_bitmap;

typedef struct osd_frame {
	int framenr;
	struct osd_header even;
	struct osd_header odd;
	struct osd_font *font;
	struct osd_bitmap *bitmap;
	struct osd_palette * palette;
} osd_frame;

typedef struct osd_frames {
	struct osd_frame frame[16];
} osd_frames;

struct osd_frames frames;

u32 osds = (u32)0,osde = (u32)0;

/* ---------------------------------------------------------------------- */

/*static void osd_set_font( unsigned char * font, int width, int height )
{
}*/

/* ---------------------------------------------------------------------- */

static int osd_show_frame( struct osd_frame * frame, u32 * palette, \
						int psize, u32* bitmap, int bmsize )
{
    int i = (int)0;

	frame->odd.bmsize1 = frame->even.bmsize1 = (bmsize>>9);
	frame->odd.bmsize2 = frame->even.bmsize2 = (bmsize&0x1ff);

	frame->odd.palette = frame->even.palette = (osds+0x1000)>>2;
	frame->odd.bmp = frame->even.bmp = (osds+0x2000)>>2;

	/* copy palette */
	for(i=0;i<psize;i++)
	{
		wDR( osds+0x1000+(i*4), palette[i] );
	}

	/* copy bitmap */
	for(i=0;i<bmsize;i++)
	{
		wDR( osds+0x2000+(i*4), bitmap[i] );
	}

	return 0;
}

/* ---------------------------------------------------------------------- */

static void osd_create_frame( struct osd_frame * frame, int x, int y, \
						int w, int h, int gbf, int pel )
{
	frame->odd.gbf = frame->even.gbf = gbf;

	frame->odd.peltyp = frame->even.peltyp = pel;

	frame->odd.colstart = frame->even.colstart = x;
	frame->odd.colstop  = frame->even.colstop  = x+w;

	/* pal rulez */
	frame->even.rowstart = 22+((y/2));
	frame->even.rowstop  = 22+(((y+h)/2));

	frame->odd.rowstart = 335+((y-1)/2);
	frame->odd.rowstop  = 335+(((y+h)-1)/2);

	printk("OSD COL/ROW: %d %d %d %d %d %d\n",
		frame->odd.colstart,
		frame->odd.colstop,
		frame->even.rowstart,
		frame->even.rowstop,
		frame->odd.rowstart,
		frame->odd.rowstop);
}

/* ---------------------------------------------------------------------- */

/*static void osd_read_frame( struct osd_frame * frame )
{
	u32 *odd,*even;
	u32 osdsp;
    int i;

	osdsp = osds+(OSDH_SIZE*2*frame->framenr);

	printk("OSD FP: %08X\n",osdsp);

	// copy header 
	even = (u32*)&frame->even;
	odd  = (u32*)&frame->odd;

	for(i=0;i<OSDH_SIZE;i+=4,osdsp+=4,even++,odd++)
	{
		*even = rDR(osdsp);
		*odd  = rDR(osdsp+OSDH_SIZE);
	}
}*/

/* ---------------------------------------------------------------------- */

static void osd_write_frame( struct osd_frame * frame )
{
	u32 *odd = (u32 *)NULL,*even = (u32 *)NULL;
	u32 osdsp = (u32)0;
    int i = (int)0;

	osdsp = osds+(OSDH_SIZE*2*frame->framenr);

	printk("OSD FP: %08X\n",osdsp);

	/* copy header */
	even = (u32*)&frame->even;
	odd  = (u32*)&frame->odd;

	for(i=0;i<OSDH_SIZE;i+=4,osdsp+=4,even++,odd++)
	{
		wDR(osdsp,*even);
		wDR(osdsp+OSDH_SIZE,*odd);
	}

	printk("OSD FNR: %d E: %08X O: %08X\n",frame->framenr,frame->even.next<<2,frame->odd.next<<2);
}

/* ---------------------------------------------------------------------- */

static void osd_init_frame( struct osd_frame * frame, int framenr )
{
	memset(frame,0,sizeof(osd_frame));

	frame->framenr = framenr;

	frame->even.n1 = 1;
	frame->even.n2 = 1;
	frame->even.n3 = 1;
	frame->even.n4 = 1;
	frame->even.n5 = 1;

	frame->odd.n1 = 1;
	frame->odd.n2 = 1;
	frame->odd.n3 = 1;
	frame->odd.n4 = 1;
	frame->odd.n5 = 1;

	if ( framenr < 15 )
	{
		frame->even.next = (osds+(OSDH_SIZE*2*(framenr+1)))>>2;
		frame->odd.next  = (osds+(OSDH_SIZE*2*(framenr+1))+OSDH_SIZE)>>2;
	}
}

/* ---------------------------------------------------------------------- */

static void osd_init_frames( struct osd_frames * frames )
{
	int i = (int)0;

	for(i=0;i<16;i++)
	{
		osd_init_frame(&frames->frame[i],i);
		osd_write_frame(&frames->frame[i]);
	}
}

/* ---------------------------------------------------------------------- */

int osd_ioctl (struct inode *inode, struct file *file, unsigned int cmd,
                  unsigned long arg)
{

	struct sosd_create_frame osdf;

	switch(cmd)
	{
		case OSD_IOCTL_CREATE_FRAME:
			if (copy_from_user( &osdf, (void*)arg, sizeof(sosd_create_frame)))
				return -EFAULT;

			osd_create_frame( &frames.frame[osdf.framenr], osdf.x, osdf.y, osdf.w, osdf.h, osdf.gbf, osdf.pel );
			osd_show_frame( &frames.frame[osdf.framenr], osdf.palette, osdf.psize, osdf.bitmap, osdf.bsize );
			osd_write_frame( &frames.frame[osdf.framenr] );

			break;
		case OSD_IOCTL_DESTROY_FRAME:
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

/* ---------------------------------------------------------------------- */

int osd_open (struct inode *inode, struct file *file)
{
	return 0;
}

/* ---------------------------------------------------------------------- */

static int init_avia_osd(void)
{
	u32 i = (u32)0;
//	u32 palette[16];
//	static u32 bitmap[0x1000];

	printk("OSD STATUS: %08X\n", rDR(OSD_VALID));

	osds = (rDR(OSD_BUFFER_START)&(~3));
	osde = rDR(OSD_BUFFER_END);

	if (osds<rDR(OSD_BUFFER_START))
	{
		osds+=4;
	}

	/* clear osd mem */
	for(i=osds;i<osde;i+=4)
	{
		wDR(i,0);
	}

    osd_init_frames(&frames);

/*
	for(i=0;i<16;i++)
	{
		osd_create_frame( &frames.frame[i], 60+(i*22), 60+(i*22), 20, 20, 0x1f, 1 );
		osd_show_frame( &frames.frame[i], palette, 16, bitmap, 20*20*4/8 );
		osd_write_frame( &frames.frame[i] );
	}
*/

	/* enable osd */
	wDR(OSD_ODD_FIELD, osds+OSDH_SIZE );
	wDR(OSD_EVEN_FIELD, osds);

	printk("OSD: %08X\n",rDR(OSD_BUFFER_IDLE_START));
	printk("OSD: %08X\n",rDR(OSD_BUFFER_START));
	printk("OSD: %08X\n",rDR(OSD_BUFFER_END));
	printk("OSD: %08X\n",rDR(OSD_ODD_FIELD));
	printk("OSD: %08X\n",rDR(OSD_EVEN_FIELD));

	udelay(1000*100);

	printk("OSD STATUS: %08X\n", rDR(OSD_VALID));

	devfs_handle = devfs_register ( NULL, "dbox/osd0", DEVFS_FL_DEFAULT,
                                  0, 0,
                                  S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                  &osd_fops, NULL );

	if ( ! devfs_handle )
	{
		return -EIO;
	}

	return 0;
	
}

/* ---------------------------------------------------------------------- */

#ifdef MODULE
MODULE_AUTHOR("Gillem <htoa@gmx.net>");
MODULE_DESCRIPTION("AVIA OSD driver");
MODULE_PARM(debug,"i");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

#if defined(MODULE) && defined(STANDALONE)
module_init(avia_av_osd_init);
module_exit(avia_av_osd_exit);
#endif

/*int init_module(void)
{
	return init_avia_osd();
}

void cleanup_module(void)
{
	wDR(OSD_EVEN_FIELD, 0 );
	wDR(OSD_ODD_FIELD, 0 );

	devfs_unregister ( devfs_handle );

	return;
}*/
#endif

/* ---------------------------------------------------------------------- */
