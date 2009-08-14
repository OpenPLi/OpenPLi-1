/*
 *   avia_gt_vbi.c - vbi driver for AViA eNX/GTX (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2001-2002 Florian Schirmer (jolt@tuxbox.org)
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
#include <linux/list.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>

#include <dbox/avia_gt.h>
#include <dbox/avia_gt_vbi.h>

static u8 ttx_flag = 0;
static sAviaGtInfo *gt_info = NULL;

//#define VBI_IRQ

static void avia_gt_vbi_reset(u8 reenable)
{

	if (avia_gt_chip(ENX))
		enx_reg_set(RSTR0, TTX, 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(RR1, TTX, 1);

	if (reenable) {

		ttx_flag = 1;

		if (avia_gt_chip(ENX))
			enx_reg_set(RSTR0, TTX, 0);
		else if (avia_gt_chip(GTX))
			gtx_reg_set(RR1, TTX, 0);

	}

}

void avia_gt_vbi_start(void)
{

	avia_gt_vbi_stop();

	dprintk("avia_gt_vbi: starting vbi reinsertion\n");

	if (!ttx_flag)
		avia_gt_vbi_reset(1);

	if (avia_gt_chip(ENX))
		enx_reg_set(TCNTL, GO, 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(TTCR, GO, 1);

}

void avia_gt_vbi_stop(void)
{

	if (avia_gt_chip(ENX))
		enx_reg_set(TCNTL, GO, 0);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(TTCR, GO, 0);

	dprintk("avia_gt_vbi: stopped vbi reinsertion\n");

}

#ifdef VBI_IRQ
static void avia_gt_vbi_irq(u16 irq)
{

	u8 tt_error = 0;
	u8 tt_pts = 0;

	if (avia_gt_chip(ENX)) {
	
		tt_error = enx_reg_s(TSTATUS)->E;
		tt_pts = enx_reg_s(TSTATUS)->R;
		
		enx_reg_set(TSTATUS, E, 0);
		enx_reg_set(TSTATUS, R, 0);
	
	} else if (avia_gt_chip(GTX)) {

		tt_error = gtx_reg_s(TSR)->E;
		tt_pts = gtx_reg_s(TSR)->R;

		gtx_reg_set(TSR, E, 0);
		gtx_reg_set(TSR, R, 0);
	
	}
	
	if (tt_error) {
	
		printk("avia_gt_vbi: error in TS stream\n");	
		
		avia_gt_vbi_stop();
		avia_gt_vbi_reset(1);
		avia_gt_vbi_start();
		
	}

	if (tt_pts)
		printk("avia_gt_vbi: got pts\n");	

}
#endif

int __init avia_gt_vbi_init(void)
{

#ifdef VBI_IRQ
	u16 irq_nr = 0;
#endif

	printk("avia_gt_vbi: $Id: avia_gt_vbi.c,v 1.17 2002/10/05 15:01:12 Jolt Exp $\n");

	gt_info = avia_gt_get_info();

	if ((!gt_info) || ((!avia_gt_chip(ENX)) && (!avia_gt_chip(GTX)))) {

		printk("avia_gt_vbi: Unsupported chip type\n");

		return -EIO;

	}

#ifdef VBI_IRQ
	if (avia_gt_chip(ENX))
		irq_nr = ENX_IRQ_TT;
	else if (avia_gt_chip(GTX))
		irq_nr = GTX_IRQ_TT;
	
	if (avia_gt_alloc_irq(irq_nr, avia_gt_vbi_irq)) {

		printk("avia_gt_pcm: unable to get vbi interrupt\n");
		
		return -EIO;
			
	}	
#endif
	
	if (avia_gt_chip(ENX))
		enx_reg_set(CFGR0, TCP, 0);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(CR1, TCP, 0);

	return 0;

}

void __exit avia_gt_vbi_exit(void)
{

	avia_gt_vbi_stop();
	avia_gt_vbi_reset(0);

#ifdef VBI_IRQ
	if (avia_gt_chip(ENX))
		avia_gt_free_irq(ENX_IRQ_TT);
	else if (avia_gt_chip(GTX))
		avia_gt_free_irq(GTX_IRQ_TT);
#endif

}

#ifdef MODULE
EXPORT_SYMBOL(avia_gt_vbi_start);
EXPORT_SYMBOL(avia_gt_vbi_stop);
#endif

#if defined(MODULE) && defined(STANDALONE)
MODULE_AUTHOR("Florian Schirmer <jolt@tuxbox.org>");
MODULE_DESCRIPTION("AViA VBI driver");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
module_init(avia_gt_vbi_init);
module_exit(avia_gt_vbi_exit);
#endif
