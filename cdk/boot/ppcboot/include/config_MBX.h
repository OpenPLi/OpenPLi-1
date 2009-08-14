/*
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Configuation settings for the MBX8xx board.
 * 
 * -----------------------------------------------------------------
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
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC860		1	/* This is a MPC860 CPU		*/
#define CONFIG_MBX	 	1	/* ...on an MBX module		*/

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600
#if 1
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif
#define CONFIG_BOOTCOMMAND	"bootm 20000" /* autoboot command	*/

#define CONFIG_BOOTARGS		"root=/dev/nfs rw "			\
				"nfsroot=10.0.0.2:/LinuxPPC "		\
				"nfsaddrs=10.0.0.99:10.0.0.2"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL & ~(CFG_CMD_ENV))

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CFG_LOAD_ADDR		0x100000	/* default load address	*/

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Physical memory map as defined by the MBX PGM
 */
#define CFG_IMMR		0xFA200000 /* Internal Memory Mapped Register*/
#define CFG_NVRAM_BASE		0xFA000000 /* NVRAM                          */
#define CFG_NVRAM_OR		0xffe00000 /* w/o speed dependent flags!!    */
#define CFG_CSR_BASE		0xFA100000 /* Control/Status Registers       */
#define CFG_PCIMEM_BASE		0x80000000 /* PCI I/O and Memory Spaces      */
#define CFG_PCIMEM_OR		0xA0000108
#define CFG_PCIBRIDGE_BASE	0xFA210000 /* PCI-Bus Bridge Registers       */
#define CFG_PCIBRIDGE_OR	0xFFFF0108

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define	CFG_INIT_RAM_END	0x2f00	/* End of used area in DPRAM	*/
#define	CFG_INIT_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_INIT_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_INIT_DATA_SIZE)
#define	CFG_INIT_VPD_SIZE	256 /* size in bytes reserved for vpd buffer */
#define CFG_INIT_VPD_OFFSET	(CFG_INIT_DATA_OFFSET - CFG_INIT_VPD_SIZE)
#define	CFG_INIT_SP_OFFSET	(CFG_INIT_VPD_OFFSET-8)

/*-----------------------------------------------------------------------
 * Offset in DPMEM where we keep the VPD data
 */
#define CFG_DPRAMVPD		(CFG_INIT_VPD_OFFSET - 0x2000)

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define	CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xfe000000
#ifdef	DEBUG
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 128 kB for Monitor	*/
#endif
#undef  CFG_MONITOR_BASE	0x200000 /* to run ppcboot from RAM */
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define	CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	4	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	16	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CFG_FLASH_ENV_OFFSET	0x40000	/*   Offset   of Environment Sector	*/
#define	CFG_FLASH_ENV_SIZE	0x10000	/* Total Size of Environment Sector	*/

/*-----------------------------------------------------------------------
 * NVRAM Configuration
 * 
 * Note: the MBX is special because there is already a firmware on this
 * board: EPPC-Bug from Motorola. To avoid collisions in NVRAM Usage, we
 * access the NVRAM at the offset 0x1000.
 */
#define CONFIG_NVRAM_ENV        1 /* turn on NVRAM env feature */
#define CFG_NVRAM_VAR_ADDR      (CFG_NVRAM_BASE + 0x1000)
#define CFG_NVRAM_ENV_SIZE      0x0ffc
#define CFG_NVRAM_VAR_CRC_ADDR  (CFG_NVRAM_VAR_ADDR + CFG_NVRAM_ENV_SIZE)

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWP)
#else
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF)
#endif

/*-----------------------------------------------------------------------
 * SUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DPC | SIUMCR_MLRC10 | \
			 SIUMCR_SEME)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR	(PISCR_PS | PISCR_PITF | PISCR_PTE)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit - leave PLL multiplication factor unchanged !
 */
#define CFG_PLPRCR	(PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	(SCCR_RTDIV | SCCR_RTSEL)
#define CFG_SCCR	SCCR_TBS

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */
#define CFG_PCMCIA_MEM_ADDR	(0xE0000000)
#define CFG_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CFG_PCMCIA_DMA_ADDR	(0xE4000000)
#define CFG_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CFG_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CFG_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CFG_PCMCIA_IO_ADDR	(0xEC000000)
#define CFG_PCMCIA_IO_SIZE	( 64 << 20 )

#define CFG_PCMCIA_INTERRUPT	SIU_LEVEL6

/*-----------------------------------------------------------------------
 * Debug Entry Mode 
 *-----------------------------------------------------------------------
 *
 */
#define CFG_DER	0

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */
