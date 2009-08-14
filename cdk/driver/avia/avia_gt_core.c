/*
 *   avia_gt_core.c - AViA eNX/GTX core driver (dbox-II-project)
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
 *   $Log: avia_gt_core.c,v $
 *   Revision 1.23  2002/10/05 15:01:12  Jolt
 *   New NAPI compatible VBI interface
 *
 *   Revision 1.22  2002/09/13 22:53:55  Jolt
 *   HW CRC support
 *
 *   Revision 1.21  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.20  2002/06/07 14:00:23  tmbinc
 *   off-by-one aka. "streamen auf gtx ist kaputt" fixed.
 *
 *   *bugsuche sponsored by enigma*
 *
 *   Revision 1.19  2002/05/09 19:54:19  obi
 *   added some tabs
 *
 *   Revision 1.18  2002/05/09 07:29:21  waldi
 *   add correct license
 *
 *   Revision 1.17  2002/05/07 16:59:19  Jolt
 *   Misc stuff and cleanups
 *
 *   Revision 1.16  2002/05/07 16:40:32  Jolt
 *   IR stuff
 *
 *   Revision 1.15  2002/05/03 16:15:31  obi
 *   - formatted source
 *   - disabled one dprintk
 *
 *   Revision 1.14  2002/05/02 18:13:52  Jolt
 *   Autodetect
 *
 *   Revision 1.13  2002/05/01 21:51:35  Jolt
 *   Merge
 *
 *   Revision 1.12  2002/04/24 08:17:42  obi
 *   enabled pig
 *
 *   Revision 1.11  2002/04/22 17:40:01  Jolt
 *   Major cleanup
 *
 *   Revision 1.10  2002/04/19 08:54:48  Jolt
 *   Merged vbi driver
 *
 *   Revision 1.9  2002/04/17 05:56:17  Jolt
 *   Capture driver fixes
 *
 *   Revision 1.8  2002/04/15 21:58:57  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.7  2002/04/15 10:40:50  Jolt
 *   eNX/GTX
 *
 *   Revision 1.6  2002/04/14 18:06:19  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.5  2002/04/13 23:19:05  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.4  2002/04/13 14:47:19  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.3  2002/04/12 21:29:35  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.2  2002/04/12 18:59:29  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.1  2002/04/12 13:50:37  Jolt
 *   eNX/GTX merge
 *
 *
 *   $Revision: 1.23 $
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

#include <dbox/info.h>
#include <dbox/avia_gt.h>
#include <dbox/avia_gt_accel.h>
#include <dbox/avia_gt_dmx.h>
#include <dbox/avia_gt_gv.h>
#include <dbox/avia_gt_pcm.h>
#include <dbox/avia_gt_capture.h>
#include <dbox/avia_gt_pig.h>
#include <dbox/avia_gt_ir.h>
#include <dbox/avia_gt_vbi.h>

#ifdef MODULE
MODULE_PARM(chip_type, "i");
#endif

int chip_type = -1;
unsigned char init_state = 0;
static sAviaGtInfo *gt_info = NULL;

void (*gt_isr_proc_list[128])(unsigned short irq);

void avia_gt_clear_irq(unsigned char irq_reg, unsigned char irq_bit)
{

	if (avia_gt_chip(ENX))
		avia_gt_enx_clear_irq(irq_reg, irq_bit);
	else if (avia_gt_chip(GTX))
		avia_gt_gtx_clear_irq(irq_reg, irq_bit);

}

sAviaGtInfo *avia_gt_get_info(void)
{

	return gt_info;

}

unsigned short avia_gt_get_irq_mask(unsigned char irq_reg)
{

	if (avia_gt_chip(ENX))
		return avia_gt_enx_get_irq_mask(irq_reg);
	else if (avia_gt_chip(GTX))
		return avia_gt_gtx_get_irq_mask(irq_reg);

	return 0;

}

unsigned short avia_gt_get_irq_status(unsigned char irq_reg)
{

	if (avia_gt_chip(ENX))
		return avia_gt_enx_get_irq_status(irq_reg);
	else if (avia_gt_chip(GTX))
		return avia_gt_gtx_get_irq_status(irq_reg);

	return 0;

}

void avia_gt_mask_irq(unsigned char irq_reg, unsigned char irq_bit)
{

	if (avia_gt_chip(ENX))
		avia_gt_enx_mask_irq(irq_reg, irq_bit);
	else if (avia_gt_chip(GTX))
		avia_gt_gtx_mask_irq(irq_reg, irq_bit);

}

void avia_gt_unmask_irq(unsigned char irq_reg, unsigned char irq_bit)
{

	if (avia_gt_chip(ENX))
		avia_gt_enx_unmask_irq(irq_reg, irq_bit);
	else if (avia_gt_chip(GTX))
		avia_gt_gtx_unmask_irq(irq_reg, irq_bit);

}

int avia_gt_alloc_irq(unsigned short irq, void (*isr_proc)(unsigned short irq))
{

	dprintk("avia_gt_core: alloc_irq reg %d bit %d\n", AVIA_GT_IRQ_REG(irq), AVIA_GT_IRQ_BIT(irq));

	if (gt_isr_proc_list[AVIA_GT_ISR_PROC_NR(AVIA_GT_IRQ_REG(irq), AVIA_GT_IRQ_BIT(irq))]) {

		printk("avia_gt_core: irq already used\n");

		return -EBUSY;

	}

	gt_isr_proc_list[AVIA_GT_ISR_PROC_NR(AVIA_GT_IRQ_REG(irq), AVIA_GT_IRQ_BIT(irq))] = isr_proc;

	avia_gt_unmask_irq(AVIA_GT_IRQ_REG(irq), AVIA_GT_IRQ_BIT(irq));
	avia_gt_clear_irq(AVIA_GT_IRQ_REG(irq), AVIA_GT_IRQ_BIT(irq));

	return 0;

}

void avia_gt_free_irq(unsigned short irq)
{

	dprintk("avia_gt_core: free_irq reg %d bit %d\n", AVIA_GT_IRQ_REG(irq), AVIA_GT_IRQ_BIT(irq));

	avia_gt_mask_irq(AVIA_GT_IRQ_REG(irq), AVIA_GT_IRQ_BIT(irq));

	gt_isr_proc_list[AVIA_GT_ISR_PROC_NR(AVIA_GT_IRQ_REG(irq), AVIA_GT_IRQ_BIT(irq))] = NULL;

}

static void avia_gt_irq_handler(int irq, void *dev, struct pt_regs *regs)
{

	unsigned char		irq_reg			= (unsigned char)0;
	unsigned char		irq_bit			= (unsigned char)0;
	unsigned short	irq_mask		= (unsigned short)0;
	unsigned short	irq_status	= (unsigned short)0;

	for (irq_reg = 0; irq_reg < 6; irq_reg++) {

		irq_mask = avia_gt_get_irq_mask(irq_reg);
		irq_status = avia_gt_get_irq_status(irq_reg);

		for (irq_bit = 0; irq_bit < 16; irq_bit++) {

			if (irq_status & (1 << irq_bit)) {

				//dprintk("avia_gt_core: interrupt reg %d bit %d\n", irq_reg, irq_bit);

				if (gt_isr_proc_list[AVIA_GT_ISR_PROC_NR(irq_reg, irq_bit)]) {

					gt_isr_proc_list[AVIA_GT_ISR_PROC_NR(irq_reg, irq_bit)](AVIA_GT_IRQ(irq_reg, irq_bit));

				} else {

					if (irq_mask & (1 << irq_bit)) {

						printk("avia_gt_core: masking unhandled irq reg %d bit %d\n", irq_reg, irq_bit);

						avia_gt_mask_irq(irq_reg, irq_bit);

					}

				}

				avia_gt_clear_irq(irq_reg, irq_bit);

			}

		}

	}

}

int __init avia_gt_init(void)
{

	struct dbox_info_struct	*dbox_info	= (struct dbox_info_struct *)NULL;
	int											 result			=	(int)0;

	printk("avia_gt_core: $Id: avia_gt_core.c,v 1.23 2002/10/05 15:01:12 Jolt Exp $\n");

	if (chip_type == -1) {

		printk("avia_gt_core: autodetecting chip type... ");

		dbox_get_info_ptr(&dbox_info);

		if (dbox_info->enxID != -1) {

			chip_type = AVIA_GT_CHIP_TYPE_ENX;

			printk("AViA eNX found\n");

		} else if (dbox_info->gtxID != -1) {

			chip_type = AVIA_GT_CHIP_TYPE_GTX;

			printk("AViA GTX found\n");

		} else {

			printk("no supported chip type found\n");

		}

	}

	if ((chip_type != AVIA_GT_CHIP_TYPE_ENX) && (chip_type != AVIA_GT_CHIP_TYPE_GTX)) {

		printk("avia_gt_core: Unsupported chip type (gt_info->chip_type = %d)\n", gt_info->chip_type);

		return -EIO;

	}

	memset(gt_isr_proc_list, 0, sizeof(gt_isr_proc_list));

	gt_info = kmalloc(sizeof(gt_info), GFP_KERNEL);

	if (!gt_info) {

		printk(KERN_ERR "avia_gt_core: Could not allocate info memory!\n");

		avia_gt_exit();

		return -1;

	}

	gt_info->chip_type = chip_type;

	init_state = 1;

	if (avia_gt_chip(ENX))
		gt_info->reg_addr = (unsigned char *)ioremap(ENX_REG_BASE, ENX_REG_SIZE);
	else if (avia_gt_chip(GTX))
		gt_info->reg_addr = (unsigned char *)ioremap(GTX_REG_BASE, GTX_REG_SIZE);

	if (!gt_info->reg_addr) {

		printk(KERN_ERR "avia_gt_core: Failed to remap register space.\n");

		return -1;

	}

	init_state = 2;

	if (avia_gt_chip(ENX))
		gt_info->mem_addr = (unsigned char*)ioremap(ENX_MEM_BASE, ENX_MEM_SIZE);
	else if (avia_gt_chip(GTX))
		gt_info->mem_addr = (unsigned char*)ioremap(GTX_MEM_BASE, GTX_MEM_SIZE);

	if (!gt_info->mem_addr) {

		printk(KERN_ERR "avia_gt_core: Failed to remap memory space.\n");

		avia_gt_exit();

		return -1;

	}

	init_state = 3;

	if (avia_gt_chip(ENX))
		result = request_8xxirq(ENX_INTERRUPT, avia_gt_irq_handler, 0, "avia_gt", 0);
	else if (avia_gt_chip(GTX))
		result = request_8xxirq(GTX_INTERRUPT, avia_gt_irq_handler, 0, "avia_gt", 0);

	if (result) {

		printk(KERN_ERR "avia_gt_core: Could not allocate IRQ!\n");

		avia_gt_exit();

		return -1;

	}

	init_state = 4;

	if (avia_gt_chip(ENX))
		avia_gt_enx_init();
	else if (avia_gt_chip(GTX))
		avia_gt_gtx_init();

	init_state = 5;

#if (!defined(MODULE)) || (defined(MODULE) && !defined(STANDALONE))
	if (avia_gt_accel_init()) {

		avia_gt_exit();

		return -1;

	}

	init_state = 6;

	if (avia_gt_dmx_init()) {

		avia_gt_exit();

		return -1;

	}

	init_state = 7;

	if (avia_gt_gv_init()) {

		avia_gt_exit();

		return -1;

	}

	init_state = 8;

	if (avia_gt_pcm_init()) {

		avia_gt_exit();

		return -1;

	}

	init_state = 9;

	if (avia_gt_capture_init()) {

		avia_gt_exit();

		return -1;

	}

	init_state = 10;

	if (avia_gt_pig_init()) {

		avia_gt_exit();

		return -1;

	}

	init_state = 11;

	if (avia_gt_ir_init()) {

		avia_gt_exit();

		return -1;

	}

	init_state = 12;

	if (avia_gt_vbi_init()) {

		avia_gt_exit();

		return -1;

	}
	
	avia_gt_vbi_start();

	init_state = 13;

#endif

	printk(KERN_NOTICE "avia_gt_core: Loaded AViA eNX/GTX driver\n");

	return 0;

}

void avia_gt_exit(void)
{

#if (!defined(MODULE)) || (defined(MODULE) && !defined(STANDALONE))
	if (init_state >= 13)
		avia_gt_vbi_exit();

	if (init_state >= 12)
		avia_gt_ir_exit();

	if (init_state >= 11)
		avia_gt_pig_exit();

	if (init_state >= 10)
		avia_gt_capture_exit();

	if (init_state >= 9)
		avia_gt_pcm_exit();

	if (init_state >= 8)
		avia_gt_gv_exit();

	if (init_state >= 7)
		avia_gt_dmx_exit();

	if (init_state >= 6)
		avia_gt_accel_exit();
#endif

	if (init_state >= 5) {

		if (avia_gt_chip(ENX))
			avia_gt_enx_exit();
		else if (avia_gt_chip(GTX))
			avia_gt_gtx_exit();

	}

	if (init_state >= 4) {

		if (avia_gt_chip(ENX))
			free_irq(ENX_INTERRUPT, 0);
		else if (avia_gt_chip(GTX))
			free_irq(GTX_INTERRUPT, 0);

	}

	if (init_state >= 3)
		iounmap(gt_info->mem_addr);

	if (init_state >= 2)
		iounmap(gt_info->reg_addr);

	if (init_state >= 1)
		kfree(gt_info);

}

#ifdef MODULE
MODULE_AUTHOR("Florian Schirmer <jolt@tuxbox.org>");
MODULE_DESCRIPTION("Avia eNX/GTX driver");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

EXPORT_SYMBOL(avia_gt_alloc_irq);
EXPORT_SYMBOL(avia_gt_free_irq);
EXPORT_SYMBOL(avia_gt_get_info);

module_init(avia_gt_init);
module_exit(avia_gt_exit);
#endif
