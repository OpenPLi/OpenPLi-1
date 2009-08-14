/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Miscellanious commands
 */
#ifndef	_CMD_MISC_H
#define	_CMD_MISC_H

#if (CONFIG_COMMANDS & CFG_CMD_PCI)

#define CMD_TBL_PCIINFO	MK_CMD_TBL_ENTRY(					\
	"pciinfo",      3,	2,	1,	do_pciinfo,			\
	"pciinfo - print information about PCI devices\n",			\
	"[ bus ]\n"								\
	"    - print information about PCI devices on PCI-Bus 'bus'\n"		\
),

/* Implemented in $(BOARD)/pci.c */
void do_pciinfo (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_PCIINFO
#endif  /* CFG_CMD_PCI */

#if (CONFIG_COMMANDS & CFG_CMD_IRQ)

#define CMD_TBL_IRQINFO	MK_CMD_TBL_ENTRY(					\
	"irqinfo",      3,	1,	1,	do_irqinfo,			\
	"irqinfo - print information about IRQs\n",				\
	NULL									\
),

/* Implemented in $(CPU)/interrupts.c */
void do_irqinfo (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_IRQINFO
#endif  /* CONFIG_COMMANDS & CFG_CMD_IRQ */

#endif	/* _CMD_MISC_H */
