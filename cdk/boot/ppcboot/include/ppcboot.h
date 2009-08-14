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

#ifndef _PPCBOOT_H_
#define _PPCBOOT_H_	1

#undef	_LINUX_CONFIG_H
#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

#include "config.h"
#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/string.h>
#include <asm/ptrace.h>
#include <stdarg.h>
#ifdef	CONFIG_8xx
#include <asm/8xx_immap.h>
#elif defined(CONFIG_8260)
#include <asm/immap_8260.h>
#endif
#ifdef	CONFIG_4xx
#include <ppc4xx.h>
#endif

typedef unsigned char		uchar;
typedef volatile unsigned long	vu_long;
typedef volatile unsigned short	vu_short;

#include <flash.h>
#include <image.h>

#ifdef	DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif

typedef	void (interrupt_handler_t)(void *);

typedef struct monitor_functions {
	int	(*getc)(void);
	int	(*tstc)(void);
	void	(*putc)(const char c);
	void	(*puts)(const char *s);
	void	(*printf)(const char *fmt, ...);
	void	(*install_hdlr)(int, interrupt_handler_t *, void *);
	void	(*free_hdlr)(int);
	void	*(*malloc)(size_t);
	void	(*free)(void *);
} mon_fnc_t;

/* A Board Information structure that is given to a program when
 * ppcboot starts it up.
 */
typedef struct bd_info {
#ifdef CFG_EXTBDINFO
	unsigned char	bi_s_version[4];  /* Version of this structure		*/
#endif
	unsigned long	bi_memstart;	/* start of  DRAM memory		*/
	unsigned long	bi_memsize;	/* size	 of  DRAM memory in bytes	*/
	unsigned long	bi_flashstart;	/* start of FLASH memory		*/
	unsigned long	bi_flashsize;	/* size	 of FLASH memory		*/
	unsigned long	bi_flashoffset; /* reserved area for startup monitor	*/
	unsigned long	bi_sramstart;	/* start of  SRAM memory		*/
	unsigned long	bi_sramsize;	/* size	 of  SRAM memory		*/
#if defined(CONFIG_8xx) || defined(CONFIG_8260)
	unsigned long	bi_immr_base;	/* base of IMMR register		*/
#endif
	unsigned long	bi_bootflags;	/* boot / reboot flag (for LynxOS)	*/
	unsigned long	bi_ip_addr;	/* IP Address				*/
	unsigned char	bi_enetaddr[6]; /* Ethernet adress			*/
	unsigned char	bi_reserved[2]; /* -- just for alignment --		*/
	unsigned long	bi_intfreq;	/* Internal Freq, in MHz		*/
	unsigned long	bi_busfreq;	/* Bus Freq, in MHz			*/
#if defined(CONFIG_8260)
	unsigned long	bi_cpmfreq;	/* CPM_CLK Freq, in MHz */
	unsigned long	bi_brgfreq;	/* BRG_CLK Freq, in MHz */
	unsigned long	bi_sccfreq;	/* SCC_CLK Freq, in MHz */
	unsigned long	bi_vco;		/* VCO Out from PLL, in MHz */
#endif
	unsigned long	bi_baudrate;	/* Console Baudrate			*/
#ifdef CFG_EXTBDINFO
	unsigned char	bi_r_version[32]; /* Version of the ROM (IBM)		*/
	unsigned int	bi_procfreq;	/* CPU (Internal) Freq, in Hz		*/
	unsigned int	bi_plb_busfreq;	/* PLB Bus speed, in Hz */
	unsigned int	bi_pci_busfreq;	/* PCI Bus speed, in Hz */
#endif
	mon_fnc_t	*bi_mon_fnc;	/* Pointer to monitor functions		*/
} bd_t;

/* The following data structure is placed in DPRAM to allow for a
 * minimum set of global variables during system initialization
 * (until we have set up the memory controller so that we can use
 * RAM).
 *
 * Keep it *SMALL* and remember to set CFG_INIT_DATA_SIZE > sizeof(init_data_t)
 */
typedef	struct	init_data {
#if !defined(CONFIG_8260)
	unsigned long	cpu_clk;	/* VCOOUT = CPU clock in Hz!		*/
#else
	/* There are many clocks on the MPC8260 - see page 9-5 */
	unsigned long	vco_out;
	unsigned long	cpm_clk;
	unsigned long	bus_clk;
	unsigned long	scc_clk;
	unsigned long	brg_clk;
	unsigned long	cpu_clk;
	/* contents of reset status register at boot */
	unsigned long	reset_status;
#endif
	unsigned long	relocated;	/* Set when relocated to RAM	 	*/
	mon_fnc_t	bi_mon_fnc;	/* Monitor functions			*/
#ifdef CONFIG_8xx
	unsigned int	dp_alloc_base;
	unsigned int	dp_alloc_top;
#endif
} init_data_t;

/*
 * Function Prototypes
 */

void	main_loop     (bd_t *);
void	hang	      (void);


/* */
long int initdram (int);
void	display_options	 (void);

/* common/main.c */
int	readline      (const char *const prompt);
void	command_repeat_off (void);

/* common/board.c */
void	board_init_f  (ulong);
void	board_init_r  (bd_t *, ulong);
int	checkboard    (void);
int	checkflash    (void);
int	checkdram     (void);
char *	strmhz(char *buf, long hz);
#if defined(CONFIG_COGENT)
int	misc_init_f   (void);
void	misc_init_r   (bd_t *);
#endif	/* CONFIG_COGENT */

/* common/cmd_bootm.c */
void print_image_hdr (image_header_t *hdr);

extern ulong load_addr;		/* Default Load Address	*/

/* common/cmd_nvedit.c */
char 	       *getenv (uchar *);
void inline 	setenv (char *, char *);

/* board/flash.c */
int flash_write (uchar *, ulong, ulong);
flash_info_t *addr2info (ulong);

#ifdef CONFIG_CPCI405
/* $(CPU)/405gp_pci.c */
void    pci_init      (void);
void    pciinfo       (int);
#endif

#ifdef CONFIG_SPD823TS
/* $(BOARD)/spd8xx.c */
void	reset_phy     (void);
#endif

#ifdef CONFIG_MBX
/* $(BOARD)/mbx8xx.c */
void	mbx_init (void);
void	board_get_enetaddr (uchar *addr);
void	board_serial_init (void);
void	board_ether_init (void);
#endif

/* $(CPU)/serial.c */
void	serial_init   (ulong, int);
void	serial_setbrg (ulong, int);
void	serial_putc   (const char);
void	serial_puts   (const char *);
void	serial_addr   (unsigned int);
int	serial_getc   (void);
int	serial_tstc   (void);

/* $(CPU)/start.S */
#ifdef	CONFIG_8xx
uint	get_immr      (uint);
#endif
uint	get_pvr	      (void);
uint	rd_ic_cst     (void);
void	wr_ic_cst     (uint);
void	wr_ic_adr     (uint);
uint	rd_dc_cst     (void);
void	wr_dc_cst     (uint);
void	wr_dc_adr     (uint);
int	icache_status (void);
void	icache_enable (void);
void	icache_disable(void);
int	dcache_status (void);
void	dcache_enable (void);
void	dcache_disable(void);
void	relocate_code (ulong, bd_t *, ulong);
ulong	get_endaddr   (void);
void	trap_init     (ulong);
#ifdef  CONFIG_4xx
unsigned char   in8(unsigned int);
void            out8(unsigned int, unsigned char);
unsigned short  in16(unsigned int);
unsigned short  in16r(unsigned int);
void            out16(unsigned int, unsigned short value);
void            out16r(unsigned int, unsigned short value);
unsigned long   in32(unsigned int);
unsigned long   in32r(unsigned int);
void            out32(unsigned int, unsigned long value);
void            out32r(unsigned int, unsigned long value);
void            ppcDcbf(unsigned long value);
void            ppcDcbi(unsigned long value);
void            ppcSync(void);
#endif

/* $(CPU)/cpu.c */
int	checkcpu      (long);
int	checkicache   (void);
int	checkdcache   (void);
void	upmconfig     (unsigned int, unsigned int *, unsigned int);
ulong	get_tbclk     (void);
#if defined(CONFIG_WATCHDOG)
void	watchdog_reset(void);
#endif	/* CONFIG_WATCHDOG */

/* $(CPU)/speed.c */
#if defined(CONFIG_8260)
void	get_8260_clks (void);
void	prt_8260_clks (void);
#elif defined(CONFIG_8xx) || defined(CONFIG_IOP480) || defined(CONFIG_PPC405GP)
ulong	get_gclk_freq (void);
#endif

ulong	get_bus_freq  (ulong);

#if defined(CONFIG_PPC405GP)
void    get_sys_info  (PPC405_SYS_INFO *);
#endif

/* $(CPU)/cpu_init.c */
#if defined(CONFIG_8xx) || defined(CONFIG_8260)
void	cpu_init_f    (volatile immap_t *immr);
#endif
#ifdef	CONFIG_4xx
void	cpu_init_f    (void);
#endif
void	cpu_init_r    (bd_t *bd);
#if defined(CONFIG_8260)
void	prt_8260_rsr  (void);
#endif

/* $(CPU)/interrupts.c */
void	interrupt_init     (bd_t *bd);
void	timer_interrupt    (struct pt_regs *);
void	external_interrupt (struct pt_regs *);
void	irq_install_handler(int, interrupt_handler_t *, void *);
void	irq_free_handler   (int);
void	reset_timer	   (void);
ulong	get_timer	   (ulong base);
void	set_timer	   (ulong t);
void	enable_interrupts  (void);
int	disable_interrupts (void);

/* $(CPU)/traps.c */

/* ppc/ticks.S */
unsigned long long get_ticks(void);
void	wait_ticks    (unsigned long);

/* ppc/time.c */
void	udelay	      (unsigned long);
ulong	usec2ticks    (unsigned long usec);
ulong	ticks2usec    (unsigned long ticks);
void	init_timebase (void);

/* ppc/vsprintf.c */
ulong	simple_strtoul(const char *cp,char **endp,unsigned int base);
long	simple_strtol(const char *cp,char **endp,unsigned int base);
void	panic(const char *fmt, ...);
int	sprintf(char * buf, const char *fmt, ...);
int 	vsprintf(char *buf, const char *fmt, va_list args);

/* ppc/crc32.c */
ulong crc32 (ulong, const unsigned char *, uint);

/* disk/part_mac.c */
void print_part_mac (int);

/* common/console.c */
bd_t	*bd_ptr ;

void	console_init_f(void);	/* Before relocation; uses the serial  stuff	*/
void	console_init_r(void);	/* After  relocation; uses the console stuff	*/
int	console_assign (int file, char *devname);	/* Assign the console	*/

/*
 * STDIO based functions (can always be used)
 */

/* serial stuff */
void	serial_printf (const char *fmt, ...);

/* stdin */
int	getc(void);
int	tstc(void);

/* stdout */
void	putc(const char c);
void	puts(const char *s);
void	printf(const char *fmt, ...);

/* stderr */
#define eputc(c)		fputc(stderr, c)
#define eputs(s)		fputs(stderr, s)
#define eprintf(fmt,args...)	fprintf(stderr,fmt ,##args)

/*
 * FILE based functions (can only be used AFTER relocation!)
 */

#define stdin		0
#define stdout		1
#define stderr		2
#define MAX_FILES	3

void	fprintf(int file, const char *fmt, ...);
void	fputs(int file, const char *s);
void	fputc(int file, const char c);
int	ftstc(int file);
int	fgetc(int file);

#ifdef CONFIG_PCMCIA
int	pcmcia_init (void);
#endif

#endif	/* _PPCBOOT_H_ */
