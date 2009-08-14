/*
 *   avia_av_proc.c - AViA 500/600 proc driver (dbox-II-project)
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
 *   $Log: avia_av_proc.c,v $
 *   Revision 1.2  2002/10/03 12:47:57  Jolt
 *   AViA AV cleanups
 *
 *   Revision 1.1  2002/10/01 20:22:59  Jolt
 *   Cleanups
 *
 *
 *
 *
 *   $Revision: 1.2 $
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
#include <linux/proc_fs.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <linux/init.h>

#include <dbox/avia_av.h>
#include <dbox/avia_av_proc.h>

int avia_av_proc_read_bitstream_settings(char *buf, char **start, off_t offset, int len, int *eof, void *private)
{

	int nr = 0;

	nr = sprintf(buf,"Bitstream Settings:\n");
	
	nr += sprintf(buf + nr, "H_SIZE:  %d\n", rDR(H_SIZE) & 0xFFFF);
	nr += sprintf(buf + nr, "V_SIZE:  %d\n", rDR(V_SIZE) & 0xFFFF);
	nr += sprintf(buf + nr, "A_RATIO: %d\n", rDR(ASPECT_RATIO) & 0xFFFF);
	nr += sprintf(buf + nr, "F_RATE:  %d\n", rDR(FRAME_RATE) & 0xFFFF);
	nr += sprintf(buf + nr, "B_RATE:  %d\n", rDR(BIT_RATE) & 0xFFFF);
	nr += sprintf(buf + nr, "VB_SIZE: %d\n", rDR(VBV_SIZE) & 0xFFFF);
	nr += sprintf(buf + nr, "A_TYPE:  %d\n", rDR(AUDIO_TYPE) & 0xFFFF);

	return nr;
	
}

int avia_av_proc_init(void)
{

	struct proc_dir_entry *proc_bus_avia;

	printk("avia_av_proc: $Id: avia_av_proc.c,v 1.2 2002/10/03 12:47:57 Jolt Exp $\n");

	if (!proc_bus) {
	
		printk("avia_av_proc: /proc/bus does not exist");

		return -ENOENT;
		
	}

	proc_bus_avia = create_proc_entry("bitstream", 0, proc_bus);

	if (!proc_bus_avia) {
	
		printk("avia_av_proc: could not create /proc/bus/bitstream");

		return -ENOENT;
		
	}

	proc_bus_avia->owner = THIS_MODULE;
	proc_bus_avia->read_proc = &avia_av_proc_read_bitstream_settings;
	
	return 0;

}

void avia_av_proc_exit(void)
{

	remove_proc_entry("bitstream", proc_bus);

}

#ifdef MODULE
//EXPORT_SYMBOL();
#endif

#if defined(MODULE) && defined(STANDALONE)
module_init(avia_av_proc_init);
module_exit(avia_av_proc_exit);
#endif
