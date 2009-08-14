/*
 *   avia_gt_enx.c - AViA eNX core driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2002 Florian Schirmer (jolt@tuxbox.org)
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
 *   $Log: avia_gt_enx.c,v $
 *   Revision 1.13  2002/09/02 19:25:37  Jolt
 *   - DMX/NAPI cleanup
 *   - Compile fix
 *
 *   Revision 1.12  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.11  2002/06/07 17:53:45  Jolt
 *   GCC31 fixes 2nd shot - sponsored by Frankster (THX!)
 *
 *   Revision 1.10  2002/04/25 22:10:38  Jolt
 *   FB cleanup
 *
 *   Revision 1.9  2002/04/25 21:09:02  Jolt
 *   Fixes/Cleanups
 *
 *   Revision 1.8  2002/04/22 17:40:01  Jolt
 *   Major cleanup
 *
 *   Revision 1.7  2002/04/13 23:19:05  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.6  2002/04/12 14:00:20  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.5  2002/04/10 22:23:18  Jolt
 *   Cleanups Part2
 *
 *   Revision 1.4  2002/04/10 21:59:59  Jolt
 *   Cleanups
 *
 *   Revision 1.3  2001/12/01 06:37:06  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.2  2001/10/23 08:40:58  Jolt
 *   eNX capture and pig driver
 *
 *   Revision 1.1  2001/10/15 20:47:46  tmbinc
 *   re-added enx-core
 *
 *   Revision 1.24  2001/09/19 18:47:00  TripleDES
 *   small init fix
 *
 *   Revision 1.23  2001/09/17 21:21:40  TripleDES
 *   some small changes
 *
 *   Revision 1.22  2001/09/02 01:01:23  TripleDES
 *   -some fixes
 *
 *   Revision 1.21  2001/08/18 18:21:54  TripleDES
 *   moved the ucode-loading to dmx
 *
 *   Revision 1.20  2001/07/17 14:38:17  tmbinc
 *   sdram fixes, but problems still not solved
 *
 *   Revision 1.19  2001/05/15 22:42:03  kwon
 *   make do_firmread() do a printk on error even if not loaded with debug=1
 *
 *   Revision 1.17  2001/04/21 10:40:13  tmbinc
 *   fixes for eNX
 *
 *   Revision 1.16  2001/04/20 01:20:19  Jolt
 *   Final Merge :-)
 *
 *   Revision 1.15  2001/04/17 22:55:05  Jolt
 *   Merged framebuffer
 *
 *   Revision 1.14  2001/04/09 23:26:42  TripleDES
 *   some changes
 *
 *   Revision 1.12  2001/03/29 03:58:24  tmbinc
 *   chaned enx_reg_w to enx_reg_h and enx_reg_d to enx_reg_w.
 *   Improved framebuffer.
 *
 *   Revision 1.11  2001/03/29 02:26:22  tmbinc
 *   fixed defines and CRLFs
 *
 *   Revision 1.10  2001/03/29 02:23:19  fnbrd
 *   IRQ angepasst, load_ram aktiviert.
 *
 *   Revision 1.9  2001/03/29 01:28:23  TripleDES
 *   Some demux testing...still not working
 *
 *   Revision 1.5  2001/03/03 12:00:35  Jolt
 *   Firmware loader
 *
 *   Revision 1.4  2001/03/03 00:47:46  Jolt
 *   Firmware loader
 *
 *   Revision 1.3  2001/03/03 00:11:34  Jolt
 *   Version cleanup
 *
 *   Revision 1.2  2001/03/03 00:09:15  Jolt
 *   Typo fix
 *
 *   Revision 1.1  2001/03/02 23:56:34  gillem
 *   - initial release
 *
 *   $Revision: 1.13 $
 *
 */

#define __KERNEL_SYSCALLS__

#include <linux/string.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
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

#include "dbox/avia_gt.h"

static sAviaGtInfo *gt_info = (sAviaGtInfo *)NULL;

static int isr[] = {0x0100, 0x0102, 0x0104, 0x0106, 0x0108, 0x010A};
static int imr[] = {0x0110, 0x0112, 0x0114, 0x0116, 0x0118, 0x011A};

void avia_gt_enx_clear_irq(unsigned char irq_reg, unsigned char irq_bit)
{

    enx_reg_16n(isr[irq_reg]) = (1 << irq_bit);
	
}

unsigned short avia_gt_enx_get_irq_mask(unsigned char irq_reg)
{

	if (irq_reg <= 5)
		return enx_reg_16n(imr[irq_reg]);
	else
		return 0;
	
}

unsigned short avia_gt_enx_get_irq_status(unsigned char irq_reg)
{

	if (irq_reg <= 5)
		return enx_reg_16n(isr[irq_reg]);
	else
		return 0;
	
}

void avia_gt_enx_mask_irq(unsigned char irq_reg, unsigned char irq_bit)
{

	enx_reg_16n(imr[irq_reg]) = 1 << irq_bit;
	
}

void avia_gt_enx_unmask_irq(unsigned char irq_reg, unsigned char irq_bit)
{

	enx_reg_16n(imr[irq_reg]) = (1 << irq_bit) | 1;
	
}

void enx_dac_init(void)
{

    enx_reg_set(RSTR0, DAC, 0);	// Get dac out of reset state
    enx_reg_16(DAC_PC) = 0x0000;
    enx_reg_16(DAC_CP) = 0x0009;
	
}

void enx_video_init(void)
{

    enx_reg_set(RSTR0, VDEO, 0);		// Get video out of reset state
    enx_reg_16(VHT) = 857 | 0x5000;
    enx_reg_16(VLT) = (623 | (21 << 11));

}

void enx_irq_enable(void)
{

  enx_reg_32(EHIDR) = 0;	 // IRQs an Hostprozessor weiterreichen
  enx_reg_32(IPR4) = 0x55555555; // alles auf HIRQ0
  enx_reg_32(IPR5) = 0x55555555; // das auch noch

  enx_reg_16(ISR0) = 0xFFFE;		// Clear all irq states
  enx_reg_16(ISR1) = 0xFFFE;		// Clear all irq states
  enx_reg_16(ISR2) = 0xFFFE;		// Clear all irq states
  enx_reg_16(ISR3) = 0xFFFE;		// Clear all irq states
  enx_reg_16(ISR4) = 0xFFFE;		// Clear all irq states
  enx_reg_16(ISR5) = 0xFFFE;		// Clear all irq states

  enx_reg_16(IMR0) = 0x0001;		// mask all IRQ's (=disable them)
  enx_reg_16(IMR1) = 0x0001;
  enx_reg_16(IMR2) = 0x0001;
  enx_reg_16(IMR3) = 0x0001;
  enx_reg_16(IMR4) = 0x0001;
  enx_reg_16(IMR5) = 0x0001;
  enx_reg_32(IDR) = 0;

}

void enx_irq_disable(void) {

  enx_reg_16(IMR0) = 0xFFFE;		// Mask all IRQ's
  enx_reg_16(IMR1) = 0xFFFE;		// Mask all IRQ's
  enx_reg_16(IMR2) = 0xFFFE;		// Mask all IRQ's
  enx_reg_16(IMR3) = 0xFFFE;		// Mask all IRQ's
  enx_reg_16(IMR4) = 0xFFFE;		// Mask all IRQ's
  enx_reg_16(IMR5) = 0xFFFE;		// Mask all IRQ's

  enx_reg_16(IMR0) = 0x0001;		// Mask all IRQ's
  enx_reg_16(IMR1) = 0x0001;		// Mask all IRQ's
  enx_reg_16(IMR2) = 0x0001;		// Mask all IRQ's
  enx_reg_16(IMR3) = 0x0001;		// Mask all IRQ's
  enx_reg_16(IMR4) = 0x0001;		// Mask all IRQ's
  enx_reg_16(IMR5) = 0x0001;		// Mask all IRQ's

}

void enx_reset(void) {

  enx_reg_32(RSTR0) = 0xFCF6BEFF;	// Reset all modules
  
}

void enx_sdram_ctrl_init(void) {

  enx_reg_32(SCSC) = 0x00000000;	// Set sd-ram start address
  enx_reg_set(RSTR0, SDCT, 0);		// Get sd-ram controller out of reset state
  enx_reg_32(MC) = 0x00001011;		// Write memory configuration
  enx_reg_32n(0x88) |= 0x3E << 4;
  
}

void avia_gt_enx_init(void)
{

    printk("avia_gt_enx: $Id: avia_gt_enx.c,v 1.13 2002/09/02 19:25:37 Jolt Exp $\n");
    
    gt_info = avia_gt_get_info();
    
    if (!avia_gt_chip(ENX)) {
    
	printk("avia_gt_enx: Unsupported chip type\n");
	
	return;
    
    }

    enx_reset();
    enx_sdram_ctrl_init();
    enx_dac_init();
    enx_video_init();
    enx_irq_enable();
  
    memset(gt_info->mem_addr, 0xF, 1024 * 1024 /*ENX_MEM_SIZE*/);

    //bring out of reset state
    enx_reg_32(RSTR0) &= ~(1 << 27);  // AV - Decoder
    enx_reg_32(RSTR0) &= ~(1 << 13);  // Queue Manager
    enx_reg_32(RSTR0) &= ~(1 << 6);   // Blitter / Color expander

    enx_reg_32(CFGR0) &= ~(1 << 1);   // disable clip mode audio
    enx_reg_32(CFGR0) &= ~(1 << 0);   // disable clip mode video
    
}

void avia_gt_enx_exit(void)
{

    enx_irq_disable();
    enx_reset();
    
}

