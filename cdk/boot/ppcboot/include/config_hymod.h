/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@cmst.csiro.au>
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
 * Config header file for Hymod board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8260		1	/* This is an MPC8260 CPU	*/
#define CONFIG_HYMOD		1	/* ...on a Hymod board		*/

/*
 * select serial console configuration
 *
 * if either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 *
 * if CONFIG_CONS_NONE is defined, then the serial console routines must
 * defined elsewhere (for example, on the cogent platform, there are serial
 * ports on the motherboard which are used for the serial console - see
 * cogent/cma101/serial.[ch]).
 */
#undef	CONFIG_CONS_ON_SMC		/* define if console on SMC */
#define	CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef	CONFIG_CONS_NONE		/* define if console on neither */
#define CONFIG_CONS_INDEX	1	/* which SMC/SCC channel for console */

/* 
 * select ethernet configuration
 *
 * if either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC)
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CFG_CMD_NET must be removed
 * from CONFIG_COMMANDS to remove support for networking.
 */
#undef	CONFIG_ETHER_ON_SCC		/* define if ethernet on SCC	*/
#undef	CONFIG_ETHER_ON_FCC		/* define if ethernet on FCC	*/
#define	CONFIG_ETHER_NONE		/* define if ethernet on neither */
#define CONFIG_ETHER_INDEX	1	/* which SCC/FCC channel for ethernet */

/* system clock rate (CLKIN) - equal to the 60x and local bus speed */
#define CONFIG_8260_CLKIN	66666666	/* in Hz */

#if 0
#define CONFIG_BAUDRATE		115200
#else
#define CONFIG_BAUDRATE		9600
#endif

#define CONFIG_COMMANDS		(CFG_CMD_ALL & ~(CFG_CMD_NET|CFG_CMD_KGDB|CFG_CMD_IDE|CFG_CMD_PCI))

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif
#define CONFIG_BOOTCOMMAND	"bootm ? ?" /* autoboot command*/

#define CONFIG_BOOTARGS		"root=/dev/ram rw"

#if 0
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port */
#else
#define CONFIG_KGDB_BAUDRATE	9600	/* speed to run kgdb serial port */
#endif

#undef	CONFIG_WATCHDOG			/* turn on platform specific watchdog */

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
#define	CFG_MAXARGS	8		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x03c00000	/* 4 ... 60 MB in DRAM	*/

#define CFG_CLKS_IN_HZ	1		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0x100000	/* default load address	*/

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 */
#define CFG_HRCW_MASTER	(HRCW_BPS11|HRCW_CIP|HRCW_L2CPC01|HRCW_DPPC10|\
			 HRCW_ISB100|HRCW_BMS|HRCW_MMR11|HRCW_APPC10|\
			 HRCW_MODCK_H0101)
/* no slaves so just duplicate the master hrcw */
#define CFG_HRCW_SLAVE1	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE2	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE3	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE4	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE5	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE6	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE7	CFG_HRCW_MASTER

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xF0000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define	CFG_INIT_RAM_END	0x4000	/* End of used area in DPRAM	*/
#define	CFG_INIT_DATA_SIZE	128  /* size in bytes reserved for initial data */
#define CFG_INIT_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_INIT_DATA_SIZE)
#define	CFG_INIT_SP_OFFSET	CFG_INIT_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define	CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		TEXT_BASE
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor*/
#define	CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_BOOTMAPSZ		(8 << 20)	/* Initial Mem map for Linux*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max num of memory banks	*/
#define CFG_MAX_FLASH_SECT	67	/* max num of sects on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Flash Erase Timeout (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/

#define	CFG_FLASH_ENV_SIZE	0x1000	/* Total Size of Environment Sector */
#define CFG_FLASH_ENV_BUF	0x40000	/* see README - env sect real size */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For MPC8260 CPU		*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers			 2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control - initially enable both caches and
 * invalidate contents, then the final state leaves only the instruction
 * cache enabled. Note that Power-On and Hard reset invalidate the caches,
 * but Soft reset does not.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CFG_HID0_INIT	(HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI|\
				HID0_IFEM|HID0_ABE)
#define CFG_HID0_FINAL	(HID0_ICE|HID0_IFEM|HID0_ABE)
#define CFG_HID2	0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register					 5-5
 *-----------------------------------------------------------------------
 * turn off Checkstop Reset Enable
 */
#define CFG_RMR		RMR_CSRE

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration					 4-25
 *-----------------------------------------------------------------------
 */
#define CFG_BCR		(BCR_ETM)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				 4-31
 *-----------------------------------------------------------------------
 */
#define CFG_SIUMCR	(SIUMCR_DPPC10|SIUMCR_L2CPC01|SIUMCR_APPC10)

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				 4-35
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR	(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP|SYPCR_SWE)
#else
#define CFG_SYPCR	(SYPCR_BMT|SYPCR_PBME|SYPCR_LBME)
#endif /* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control			 4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CFG_TMCNTSC	(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CFG_PISCR	(PISCR_PS|PISCR_PTF|PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control					 9-8
 *-----------------------------------------------------------------------
 * Ensure DFBRG is Divide by 16
 */
#define CFG_SCCR	(SCCR_DFBRG01)

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration				13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR	0

/*
 * Init Memory Controller:
 *
 * Bank	Bus	Machine	PortSz	Device
 * ----	---	-------	------	------
 *  0	60x	GPCM	32 bit	FLASH
 *  1	60x	GPCM	32 bit	FLASH (same as 0 - unused for now)
 *  2	60x	SDRAM	64 bit	SDRAM
 *  3	Local	UPMC	 8 bit	Main Xilinx configuration (unimplemented)
 *  4	Local	GPCM	32 bit	Main Xilinx register mode (unimplemented)
 *  5	Local	UPMB	32 bit	Main Xilinx port mode (unimplemented)
 *  6	Local	UPMC	 8 bit	Mezz Xilinx configuration (unimplemented)
 */

/*
 * Bank 0 - FLASH
 *
 * Quotes from the HYMOD IO Board Reference manual:
 *
 * "The flash memory is two Intel StrataFlash chips, each configured for
 *  16 bit operation and connected to give a 32 bit wide port."
 *
 * "The chip select logic is configured to respond to both *CS0 and *CS1.
 *  Therefore the FLASH memory will be mapped to both bank 0 and bank 1.
 *  It is suggested that bank 0 be read-only and bank 1 be read/write. The
 *  FLASH will then appear as ROM during boot."
 *
 * Initially, we are only going to use bank 0 in read/write mode.
 */

/* 32 bit, read-write, GPCM on 60x bus */
#define	CFG_BR0_PRELIM	((CFG_FLASH_BASE&BRx_BA_MSK)|\
				BRx_PS_32|BRx_MS_GPCM_P|BRx_V)
/* up to 32 Mb */
#define	CFG_OR0_PRELIM	(MEG_TO_AM(32)|ORxG_CSNT|ORxG_ACS_DIV2|ORxG_SCY_10_CLK)

/*
 * Bank 2 - SDRAM
 *
 * Quotes from the HYMOD IO Board Reference manual:
 *
 * "The main memory is implemented using TC59SM716FTL-10 SDRAM and has a
 *  fixed size of 64 Mbytes. The Toshiba TC59SM716FTL-10 is a CMOS synchronous
 *  dynamic random access memory organised as 4 banks by 4096 rows by 512
 *  columns by 16 bits. Four chips provide a 64-bit port on the 60x bus."
 *
 * "The locations in SDRAM are accessed using multiplexed address pins to
 *  specify row and column. The pins also act to specify commands. The state
 *  of the inputs *RAS, *CAS and *WE defines the required action. The a10/AP
 *  pin may function as a row address or as the AUTO PRECHARGE control line,
 *  depending on the cycle type. The 60x bus SDRAM machine allows the MPC8260
 *  address lines to be configured to the required multiplexing scheme."
 */

#define CFG_SDRAM_SIZE	64

/* 64 bit, read-write, SDRAM on 60x bus */
#define	CFG_BR2_PRELIM	((CFG_SDRAM_BASE&BRx_BA_MSK)|\
				BRx_PS_64|BRx_MS_SDRAM_P|BRx_V)
/* 64 Mb, 4 int banks per dev, row start addr bit = A6, 12 row addr lines */
#define	CFG_OR2_PRELIM	(MEG_TO_AM(CFG_SDRAM_SIZE)|\
				ORxS_BPD_4|ORxS_ROWST_PBI1_A6|ORxS_NUMR_12)

/* 
 * The 60x Bus SDRAM Mode Register (PDSMR) is set as follows:
 *
 * Page Based Interleaving, Refresh Enable, Address Multiplexing where A5
 * is output on A16 pin (A6 on A17, and so on), use address pins A14-A16
 * as bank select, A7 is output on SDA10 during an ACTIVATE command,
 * earliest timing for ACTIVATE command after REFRESH command is 6 clocks,
 * earliest timing for ACTIVATE or REFRESH command after PRECHARGE command
 * is 2 clocks, earliest timing for READ/WRITE command after ACTIVATE
 * command is 2 clocks, earliest timing for PRECHARGE after last data
 * was read is 1 clock, earliest timing for PRECHARGE after last data
 * was written is 1 clock, CAS Latency is 2.
 */

#define CFG_PSDMR	(PSDMR_PBI|PSDMR_RFEN|PSDMR_SDAM_A16_IS_A5|\
				PSDMR_BSMA_A14_A16|PSDMR_SDA10_PBI1_A7|\
				PSDMR_RFRC_6_CLK|PSDMR_PRETOACT_2W|\
				PSDMR_ACTTORW_2W|PSDMR_LDOTOPRE_1C|\
				PSDMR_WRC_1C|PSDMR_CL_2)

/*
 * The 60x bus-assigned SDRAM Refresh Timer (PSRT) (10-31) and the Refresh
 * Timers Prescale (PTP) value in the Memory Refresh Timer Prescaler Register
 * (MPTPR) (10-32) must also be set up (it used to be called the Periodic Timer
 * Prescaler, hence the P instead of the R). The refresh timer period is given
 * by (note that there was a change in the 8260 UM Errata):
 *
 *	TimerPeriod = (PSRT + 1) / Fmptc
 *
 * where Fmptc is the BusClock divided by PTP. i.e.
 *
 *	TimerPeriod = (PSRT + 1) / (BusClock / PTP)
 *
 * or
 *
 *	TImerPeriod = (PTP * (PSRT + 1)) / BusClock
 *
 * The requirement for the Toshiba TC59SM716FTL-10 is that there must be
 * 4K refresh cycles every 64 ms. i.e. one refresh cycle every 64000/4096
 * = 15.625 usecs.
 *
 * So PTP * (PSRT + 1) <= 15.625 * BusClock. At 66.666MHz, PSRT=31 and PTP=32
 * appear to be reasonable.
 */

#define CFG_PSRT	31
#define CFG_MPTPR	((32<<MPTPR_PTP_SHIFT)&MPTPR_PTP_MSK)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH*/
#define BOOTFLAG_WARM	0x02		/* Software reboot		*/

#endif	/* __CONFIG_H */
