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
 *  Definitions for Configuring the monitor commands
 */
#ifndef	_CMD_CONFIG_H
#define	_CMD_CONFIG_H

/*
 * Configurable monitor commands
 */
#define CFG_CMD_BDI		0x00000001	/* bdinfo			*/
#define CFG_CMD_LOADS		0x00000002	/* loads			*/
#define CFG_CMD_LOADB		0x00000004	/* loadb			*/
#define CFG_CMD_IMI		0x00000008	/* iminfo			*/
#define CFG_CMD_CACHE		0x00000010	/* icache, dcache		*/
#define CFG_CMD_FLASH		0x00000020	/* flinfo, erase, protect	*/
#define CFG_CMD_MEMORY		0x00000040	/* md, mm, nm, mw, cp, cmp,	*/
				    		/* crc, base, loop, mtest	*/
#define CFG_CMD_NET		0x00000080	/* bootp, tftpboot, rarpboot	*/
#define CFG_CMD_ENV		0x00000100	/* saveenv			*/
#define CFG_CMD_KGDB		0x00000200	/* kgdb				*/
#define	CFG_CMD_IDE		0x00000400	/* IDE harddisk support		*/
#define	CFG_CMD_PCI	  	0x00000800	/* pciinfo			*/
#define	CFG_CMD_IRQ		0x00001000	/* irqinfo			*/
#define CFG_CMD_BOOTD		0x00002000	/* bootd			*/
#define CFG_CMD_CONSOLE		0x00004000	/* coninfo			*/
#define CFG_CMD_BOOTIDXFS	0x00008000	/* bootidxfs			*/
#define CFG_CMD_INFOIDXFS	0x00010000	/* infoidxfs			*/

#define CFG_CMD_ALL	0xFFFFFFFF	/* ALL commands			*/

/* Commands that are considered "non-standard" for some reason
 * (memory hogs, requires special hardware, not fully tested, etc.)
 */
#define CFG_CMD_NONSTD	(CFG_CMD_KGDB | CFG_CMD_IDE | CFG_CMD_PCI | CFG_CMD_IRQ | CFG_CMD_BOOTIDXFS | CFG_CMD_INFOIDXFS)

/* Default configuration
 */
#define	CONFIG_CMD_DFL	(CFG_CMD_ALL & ~CFG_CMD_NONSTD)

#ifndef CONFIG_COMMANDS
#define CONFIG_COMMANDS	CONFIG_CMD_DFL
#endif



/*
 * optional BOOTP fields
 */

#define CONFIG_BOOTP_SUBNETMASK		0x00000001
#define CONFIG_BOOTP_GATEWAY		0x00000002
#define CONFIG_BOOTP_HOSTNAME		0x00000004
#define CONFIG_BOOTP_NISDOMAIN		0x00000008
#define CONFIG_BOOTP_BOOTPATH		0x00000010
#define CONFIG_BOOTP_BOOTFILESIZE	0x00000020
#define CONFIG_BOOTP_DNS		0x00000040

#define CONFIG_BOOTP_ALL		(~0)

#define CONFIG_BOOTP_DEFAULT		(CONFIG_BOOTP_SUBNETMASK | \
					CONFIG_BOOTP_GATEWAY	 | \
					CONFIG_BOOTP_HOSTNAME	 | \
					CONFIG_BOOTP_BOOTPATH)

#ifndef CONFIG_BOOTP_MASK
#define CONFIG_BOOTP_MASK		CONFIG_BOOTP_DEFAULT
#endif

#endif	/* _CMD_CONFIG_H */
