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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_PPC405GP		1	/* This is a PPC405 CPU		*/
#define CONFIG_CPCI405		1	/* ...on a CPCI405 board	*/

#define CONFIG_CPUCLOCK		200
#define CONFIG_BUSCLOCK		100
#define CONFIG_SYS_CLK_FREQ     33333333 /* external frequency to pll   */

#define CONFIG_BAUDRATE		9600
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#if 1
#define CONFIG_BOOTCOMMAND	"bootm ffc00000" /* autoboot command	*/
#else
#define CONFIG_BOOTCOMMAND	"bootp" /* autoboot command		*/
#endif

#if 0
#define CONFIG_BOOTARGS		"root=/dev/nfs rw "			\
				"nfsroot=10.0.0.2:/LinuxPPC "		\
				"nfsaddrs=10.0.0.99:10.0.0.2"
#else
#define CONFIG_BOOTARGS		"root=/dev/hda1 "			\
   "ip=192.168.2.176:192.168.2.190:192.168.2.79:255.255.255.0"

#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_PCI_PNP		1	   /* include pci plug-and-play */

#if 1
#define CONFIG_COMMANDS		\
	((CONFIG_CMD_DFL | CFG_CMD_PCI | CFG_CMD_IRQ | CFG_CMD_IDE) & ~(CFG_CMD_ENV))
#else
#define CONFIG_COMMANDS		\
	((CONFIG_CMD_DFL | CFG_CMD_PCI | CFG_CMD_IRQ) & ~(CFG_CMD_ENV))
#endif

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_NVRAM_ENV	1	/* use NVRAM for environment vars	*/
#define CFG_NVRAM_BASE_ADDR	0xf0200000		/* NVRAM base address	*/
#define CFG_NVRAM_SIZE		(32*1024)		/* NVRAM size		*/
#define CFG_NVRAM_ENV_SIZE	0x0ffc		/* Size of Environment vars	*/
#define CFG_NVRAM_VAR_ADDR	\
	(CFG_NVRAM_BASE_ADDR+CFG_NVRAM_SIZE-CFG_NVRAM_ENV_SIZE-4)	/* Env	*/
#define CFG_NVRAM_VAR_CRC_ADDR	(CFG_NVRAM_VAR_ADDR+CFG_NVRAM_ENV_SIZE)

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CFG_BAUDRATE_MIN	300	/* lowest possible baudrate	*/
#define CFG_BAUDRATE_MAX	115200	/* highest possible baudrate	*/
#define CFG_BAUDRATE_DEFAULT	CONFIG_BAUDRATE	    /* default baudrate */

#define CFG_LOAD_ADDR		0x100000	/* default load address */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks	*/

/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 *-----------------------------------------------------------------------
 */
#undef  CONFIG_IDE_PCMCIA               /* no pcmcia interface required */
#undef  CONFIG_IDE_LED                  /* no led for ide supported     */
#undef  CONFIG_IDE_RESET                /* no reset for ide supported   */

#define	CFG_IDE_MAXBUS	        1		/* max. 1 IDE busses	*/
#define	CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS*1) /* max. 1 drives per IDE bus */

#define	CFG_ATA_BASE_ADDR	0xF0100000
#define	CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_DATA_OFFSET	0x0000	/* Offset for data I/O			*/
#define	CFG_ATA_REG_OFFSET	0x0000	/* Offset for normal register accesses	*/
#define CFG_ATA_ALT_OFFSET	0x0000	/* Offset for alternate registers	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFFD0000
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(192 * 1024)	/* Reserve 196 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_ENV_OFFSET	0x9000	/*   Offset   of Environment Sector	*/
#define CFG_FLASH_ENV_SIZE	0x1000	/* Total Size of Environment Sector	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xFF800000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0xFFC00000	/* FLASH bank #1	*/


/* On Chip Memory location */
#define OCM_DATA_ADDR		0xF8000000

/* Configuration Port location */
#define CONFIG_PORT_ADDR	0xF0000500

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	OCM_DATA_ADDR      /* On Chip SRAM (4K) */
#define CFG_INIT_RAM_END	0x0f00	/* End of used area in DPRAM	*/
#define CFG_INIT_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_INIT_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_INIT_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_INIT_DATA_OFFSET


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */
