/*
 *   avia_gt_gtx.c - AViA GTX core driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 Felix "tmbinc" Domke (tmbinc@gmx.net)
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
 *   $Log: avia_gt_gtx.c,v $
 *   Revision 1.17  2002/09/15 18:06:26  Jolt
 *   Cleanup
 *
 *   Revision 1.16  2002/09/02 20:56:06  Jolt
 *   - HW section fix (GTX)
 *   - DMX/NAPI cleanups
 *
 *   Revision 1.15  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.14  2002/06/07 18:06:03  Jolt
 *   GCC31 fixes 2nd shot (GTX version) - sponsored by Frankster (THX!)
 *
 *   Revision 1.13  2002/05/09 22:25:23  obi
 *   cleanup, use structs
 *
 *   Revision 1.12  2002/05/03 17:03:36  obi
 *   replaced r*() by gtx_reg_*()
 *   formatted source
 *
 *   Revision 1.11  2002/04/22 19:50:25  Jolt
 *   Missing init stuff
 *
 *   Revision 1.10  2002/04/22 17:40:01  Jolt
 *   Major cleanup
 *
 *   Revision 1.9  2002/04/16 15:57:23  Jolt
 *   GTX bugfix
 *
 *   Revision 1.8  2002/04/15 21:58:57  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.7  2002/04/13 23:19:05  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.6  2002/04/13 14:47:19  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.5  2002/04/12 23:20:25  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.4  2002/04/12 21:31:37  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.3  2002/03/06 09:04:10  gillem
 *   - clean module unload
 *
 *   Revision 1.2  2001/12/12 17:20:38  obi
 *   version history was gone
 *
 *   Revision 1.1  2001/12/12 01:47:10  obi
 *   re-added with correct file rights
 *
 *   Revision 1.2  2001/10/15 21:04:33  tmbinc
 *   sorry, CRLF sucks
 *
 *   Revision 1.1  2001/10/15 20:59:47  tmbinc
 *   re-added because of lameness
 *
 *   Revision 1.26  2001/08/18 18:21:10  TripleDES
 *   moved the ucode loading to dmx
 *
 *   Revision 1.25  2001/07/22 09:55:38  gillem
 *   - define bug fixed
 *
 *   Revision 1.24  2001/07/19 22:22:46  gillem
 *   - add proc fs
 *
 *   Revision 1.23  2001/05/15 22:42:03  kwon
 *   make do_firmread() do a printk on error even if not loaded with debug=1
 *
 *   Revision 1.22  2001/04/20 01:20:19  Jolt
 *   Final Merge :-)
 *
 *   Revision 1.21  2001/04/19 23:32:27  Jolt
 *   Merge Part II
 *
 *   Revision 1.20  2001/04/17 22:55:05  Jolt
 *   Merged framebuffer
 *
 *   Revision 1.19  2001/04/03 22:38:32  kwon
 *   make /proc/bus/gtx writable, just in case...
 *
 *   Revision 1.18  2001/03/21 15:30:25  tmbinc
 *   Added SYNC-delay for avia, resulting in faster zap-time.
 *
 *   Revision 1.17  2001/03/18 00:03:35  Hunz
 *   framebuffer fix
 *
 *   Revision 1.16  2001/03/04 13:02:25  tmbinc
 *   Added uCode interface for debugging.
 *
 *   Revision 1.15  2001/03/03 11:27:17  gillem
 *   - fix dprintk
 *
 *   Revision 1.14  2001/02/16 20:05:44  gillem
 *   - add new options ucode,debug
 *
 *   Revision 1.13  2001/02/11 15:53:25  tmbinc
 *   section filtering (not yet working)
 *
 *   Revision 1.12  2001/02/03 16:39:17  tmbinc
 *   sound fixes
 *
 *   Revision 1.11  2001/02/03 14:48:16  gillem
 *   - more audio fixes :-/
 *
 *   Revision 1.10  2001/02/03 11:30:10  gillem
 *   - fix audio
 *
 *   Revision 1.9  2001/01/31 17:17:46  tmbinc
 *   Cleaned up avia drivers. - tmb
 *
 *
 *   $Revision: 1.17 $
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

static int isr[] = {GTX_REG_ISR0, GTX_REG_ISR1, GTX_REG_ISR2, GTX_REG_ISR3};
static int imr[] = {GTX_REG_IMR0, GTX_REG_IMR1, GTX_REG_IMR2, GTX_REG_IMR3};

void avia_gt_gtx_clear_irq(unsigned char irq_reg, unsigned char irq_bit)
{

	gtx_reg_16n(isr[irq_reg]) |= (1 << irq_bit);

}

unsigned short avia_gt_gtx_get_irq_mask(unsigned char irq_reg)
{

	if (irq_reg <= 3)
		return gtx_reg_16n(imr[irq_reg]);
	else
		return 0;

}

unsigned short avia_gt_gtx_get_irq_status(unsigned char irq_reg)
{

	if (irq_reg <= 3)
		return gtx_reg_16n(isr[irq_reg]);
	else
		return 0;

}

void avia_gt_gtx_mask_irq(unsigned char irq_reg, unsigned char irq_bit)
{

	gtx_reg_16n(imr[irq_reg]) &= ~(1 << irq_bit);

}

void avia_gt_gtx_unmask_irq(unsigned char irq_reg, unsigned char irq_bit)
{

	gtx_reg_16n(imr[irq_reg]) |= (1 << irq_bit);

}

static void avia_gt_gtx_intialize_interrupts(void)
{

	gtx_reg_16(IPR0) = -1;
	gtx_reg_16(IPR1) = -1;
	gtx_reg_16(IPR2) = -1;
	gtx_reg_16(IPR3) = -1;

	gtx_reg_16(IMR0) = 0;
	gtx_reg_16(IMR1) = 0;
	gtx_reg_16(IMR2) = 0;
	gtx_reg_16(IMR3) = 0;

	gtx_reg_16(ISR0) = 0;
	gtx_reg_16(ISR1) = 0;
	gtx_reg_16(ISR2) = 0;
	gtx_reg_16(ISR3) = 0;

	gtx_reg_set(RR0, INT, 0);

	gtx_reg_16(IMR0) = 0xFFFF;
	gtx_reg_16(IMR1) = 0xFFFF;

}

static void avia_gt_gtx_close_interrupts(void)
{

	gtx_reg_set(RR0, INT, 1);

	gtx_reg_16(IMR0) = 0;
	gtx_reg_16(IMR1) = 0;
	gtx_reg_16(IMR2) = 0;
	gtx_reg_16(IMR3) = 0;

	gtx_reg_16(IPR0) = -1;
	gtx_reg_16(IPR1) = -1;
	gtx_reg_16(IPR2) = -1;
	gtx_reg_16(IPR3) = -1;

	gtx_reg_16(ISR0) = 0;
	gtx_reg_16(ISR1) = 0;
	gtx_reg_16(ISR2) = 0;
	gtx_reg_16(ISR3) = 0;

}

void avia_gt_gtx_reset(void)
{
	gtx_reg_set(RR0, PIG, 1);
	gtx_reg_set(RR0, VCAP, 1);
	gtx_reg_set(RR0, VID, 1);
	gtx_reg_set(RR0, ACLK, 1);
	gtx_reg_set(RR0, COPY, 1);
	gtx_reg_set(RR0, DRAM, 1);
	gtx_reg_set(RR0, PCM, 1);
	gtx_reg_set(RR0, SPI, 1);
	gtx_reg_set(RR0, IR, 1);
	gtx_reg_set(RR0, BLIT, 1);
	gtx_reg_set(RR0, CRC, 1);
	gtx_reg_set(RR0, INT, 1);
	gtx_reg_set(RR0, SCD, 1);
	gtx_reg_set(RR0, SRX, 1);
	gtx_reg_set(RR0, STX, 1);
	gtx_reg_set(RR0, GV, 1);
	gtx_reg_set(RR1, TTX, 1);
	gtx_reg_set(RR1, DAC, 1);
	gtx_reg_set(RR1, RISC, 1);
	gtx_reg_set(RR1, FRMR, 1);
	gtx_reg_set(RR1, CHAN, 1);
	gtx_reg_set(RR1, AVD, 1);
	gtx_reg_set(RR1, IDC, 1);
	gtx_reg_set(RR1, DESC, 1);
}

void avia_gt_gtx_init(void)
{

	printk("avia_gt_gtx: $Id: avia_gt_gtx.c,v 1.17 2002/09/15 18:06:26 Jolt Exp $\n");

	gt_info = avia_gt_get_info();

	if (!avia_gt_chip(GTX)) {

		printk("avia_gt_gtx: Unsupported chip type\n");

		return;

	}

	avia_gt_gtx_reset();

	// solle nach avia_gt_gtx_reset() wenn ueberhaupt noetig ...
	udelay (500);

	gtx_reg_set(RR0, VID, 0);
	gtx_reg_set(RR0, DRAM, 0);
	gtx_reg_set(RR0, BLIT, 0);

	// ?
	udelay (500);

	memset (gt_info->mem_addr, 0xFF, GTX_MEM_SIZE);	  // clear ram

	gtx_reg_set(CR0, DOD, 0);	// DAC Output Disable (0: enable)
	gtx_reg_set(CR0, _16M, 1);	// 16 Mbit DRAM Select (1: 16 Mbit)
	gtx_reg_set(CR0, DD1, 0);	// Delay DTACK (2 clocks delay)
	gtx_reg_set(CR0, DD0, 1);
	gtx_reg_set(CR0, RFD, 0);	// Refresh Disable

	avia_gt_gtx_intialize_interrupts ();

	// workaround for framebuffer?
	//atomic_set (&THIS_MODULE->uc.usecount, 1);

}

void avia_gt_gtx_exit(void)
{

	avia_gt_gtx_close_interrupts();

	//gtx_reg_16(CR0) = 0x0030;
	//gtx_reg_16(CR1) = 0x0000;
	gtx_reg_set(CR0, DOD, 1);
	gtx_reg_set(CR0, RFD, 1);

	avia_gt_gtx_reset();

	// take modules in reset state
	//gtx_reg_16(RR0) = 0xFBFF;
	//gtx_reg_16(RR1) = 0x00FF;

	// disable dram module, don't work :-/ why ????
	//gtx_reg_16(RR0) |= (1<<10);

}
