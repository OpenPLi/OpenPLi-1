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

#include <ppcboot.h>
#include <command.h>
#include <malloc.h>
#include <devices.h>
#if (CONFIG_COMMANDS & CFG_CMD_IDE)
#include <ide.h>
#endif
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#include <kgdb.h>
#endif
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif
#include <version.h>

static char *failed = "*** failed ***\n";

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
static	ulong	mem_malloc_start = 0;
static	ulong	mem_malloc_end	 = 0;
static	ulong	mem_malloc_brk	 = 0;

/*
 * The Malloc area is immediately below the monitor copy in DRAM
 */
static void mem_malloc_init (ulong dest_addr)
{
	mem_malloc_end	 = dest_addr;
	mem_malloc_start = dest_addr - CFG_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;

	memset ((void *)mem_malloc_start, 0, mem_malloc_end - mem_malloc_start);
}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) ||
	    (new > mem_malloc_end) ) {
		return (NULL);
	}
	mem_malloc_brk = new;
	return ((void *)old);
}

char *
strmhz(char *buf, long hz)
{
    long l, n;
#if defined(CFG_CLKS_IN_HZ)
    long m;
#endif

    n = hz / 1000000L;

    l = sprintf(buf, "%ld", n);

#if defined(CFG_CLKS_IN_HZ)
    m = (hz % 1000000L) / 1000L;

    if (m != 0)
	sprintf(buf+l, ".%03ld", m);
#endif

    return (buf);
}

/*
 * Breath some life into the board...
 *
 * Initialize an SMC for serial comms, and carry out some hardware
 * tests.
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */
void
board_init_f (ulong bootflag)
{
    bd_t	*bd;
    ulong	reg, len;
    int		board_type;
    ulong	addr_moni, addr_sp;
    ulong	dram_size;
    char	*s, *e;
    int		baudrate;
    /* Pointer to initial global data area */
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);

#if defined(CONFIG_8260)

    get_8260_clks();

#else	/* !CONFIG_8260 */

    /* CPU Clock Speed */
    idata->cpu_clk = get_gclk_freq ();		/* in  Hz */

#endif	/* CONFIG_8260 */

    s = getenv ("baudrate");
    baudrate = s ? (int)simple_strtoul(s, NULL, 10) : CONFIG_BAUDRATE;

    /* set up serial port */
    serial_init (idata->cpu_clk, baudrate);

    /* Initialize the console (before the relocation) */
    console_init_f ();

    /* now we can use standard printf/puts/getc/tstc functions */
    display_options();

#if defined(CONFIG_8260)
    prt_8260_rsr();
    prt_8260_clks();
#endif	/* CONFIG_8260 */

    printf ("Initializing...\n  CPU:   ");		/* Check CPU		*/

    if (checkcpu(idata->cpu_clk) < 0) {
    	printf (failed);
	hang();
    }

    printf ("  Board: ");				/* Check Board		*/

    if ((board_type = checkboard()) < 0) {
    	printf (failed);
	hang();
    }

#ifdef CONFIG_COGENT
    /* miscellaneous platform dependent initialisations */
    if (misc_init_f() < 0) {
	printf (failed);
	hang();
    }
#endif

    /* we need the timebase for udelay() in DRAM setup */
    init_timebase ();

    printf ("  DRAM:  ");

    if ((dram_size = initdram (board_type)) > 0) {
	printf ("%2ld MB\n", dram_size >> 20);
    } else {
	printf (failed);
	hang();
    }

    /*
     * Now that we have DRAM mapped and working, we can
     * relocate the code and continue running from DRAM.
     *
     * First reserve memory for monitor code at end of DRAM.
     */
    len = get_endaddr() - CFG_MONITOR_BASE;
    if (len > CFG_MONITOR_LEN) {
	printf ("*** PPCBoot size %ld > reserved memory (%d)\n",
		len, CFG_MONITOR_LEN);
	hang();
    }
    if (CFG_MONITOR_LEN > len)
	len = CFG_MONITOR_LEN;
    /* round up to next 4 kB limit */
    len = (len + (4096 - 1)) & ~(4096 - 1);

    addr_moni = CFG_SDRAM_BASE + dram_size - len;

#ifdef DEBUG
    printf ("  Relocating to: %08lx\n", addr_moni);
#endif

    /*
     * Then we (permanently) allocate a Board Info struct.
     *
     * We leave room for the malloc() arena.
     */
    bd = (bd_t *)(addr_moni - sizeof(bd_t) - CFG_MALLOC_LEN);

#ifdef DEBUG
    printf ("  Board Info at: %08lx\n", (ulong)bd);
#endif

    /*
     * Finally, we set up a new (bigger) stack.
     *
     * Leave some safety gap for SP, force alignment on 16 byte boundary
     */
    addr_sp  = (ulong)bd - 128;
    addr_sp &= ~0xF;

#if defined(CFG_FLASH_ENV_BUF)
    /*
     * Unfortunately, some boards (like the Cogent CMA302 flash I/O
     * Module) have ridiculously large sectors (512KB) because they
     * have 8 x 8 bit wide flash chips arranged so that each chip has
     * one of the byte lanes in a 64 bit word. Not only that, the
     * Intel 28F008S5 flash chips have 64K sectors (16 of them, for
     * 1Mbyte each). So effectively, the CMA302 has 16 x 512KB
     * sectors. We need some space to store the data when programming
     * one of these flash sectors, so the only way I can think of at
     * the moment is to allocate it here, between the board data and
     * the top of the stack.
     */
    addr_sp -= CFG_FLASH_ENV_BUF;
#endif /* CFG_FLASH_ENV_BUF */

    /*
     * Save local variables to board info struct
     */

    bd->bi_memstart    = CFG_SDRAM_BASE;  /* start of  DRAM memory		*/
    bd->bi_memsize     = dram_size;	  /* size  of  DRAM memory in bytes	*/
    bd->bi_flashstart  = CFG_FLASH_BASE;  /* start of FLASH memory		*/
    bd->bi_flashsize   = 0;	 	  /* size  of FLASH memory (PRELIM)	*/
    
#if CFG_MONITOR_BASE == CFG_FLASH_BASE
    bd->bi_flashoffset = CFG_MONITOR_LEN; /* reserved area for startup monitor	*/
#else
    bd->bi_flashoffset = 0;
#endif

    bd->bi_sramstart   = 0; /* FIXME */	  /* start of  SRAM memory		*/
    bd->bi_sramsize    = 0; /* FIXME */	  /* size  of  SRAM memory		*/

#if defined(CONFIG_8xx) || defined(CONFIG_8260)
    bd->bi_immr_base   = CFG_IMMR;	  /* base  of IMMR register		*/
#endif

    bd->bi_bootflags   = bootflag;	  /* boot / reboot flag (for LynxOS)	*/

    /* IP Address */
    bd->bi_ip_addr = 0;
    s = getenv ("ipaddr");
    for (reg=0; reg<4; ++reg) {
    	ulong val = s ? simple_strtoul(s, &e, 10) : 0;
	bd->bi_ip_addr <<= 8;
	bd->bi_ip_addr  |= (val & 0xFF);
	if (s)
		s = (*e) ? e+1 : e;
    }

    s = getenv ("ethaddr");

#ifdef	CONFIG_MBX
    if (s == NULL)
	board_get_enetaddr(bd->bi_enetaddr);
    else
#endif
#ifdef CONFIG_DBOX
    if (s == NULL)
      memcpy(bd->bi_enetaddr, (void*)0x1001FFE3, 6);
    else
#endif
    for (reg=0; reg<6; ++reg) {
	bd->bi_enetaddr[reg] = s ? simple_strtoul(s, &e, 16) : 0;
	if (s)
		s = (*e) ? e+1 : e;
    }
    
    printf("  Ethernet: ");
    for (reg=0; reg<6; reg++)  {
      printf("%02x%s", bd->bi_enetaddr[reg], reg==5?"\n":"-");
    }

#if defined(CFG_CLKS_IN_HZ)
    bd->bi_intfreq  = idata->cpu_clk;			/* Internal Freq, in Hz	*/
    bd->bi_busfreq  = get_bus_freq(idata->cpu_clk);	/* Bus Freq,      in Hz	*/
# if defined(CONFIG_8260)
    bd->bi_cpmfreq = idata->cpm_clk;
    bd->bi_brgfreq = idata->brg_clk;
    bd->bi_sccfreq = idata->scc_clk;
    bd->bi_vco	   = idata->vco_out;
# endif /* CONFIG_8260 */
#else
    bd->bi_intfreq  = idata->cpu_clk / 1000000L;
    bd->bi_busfreq  = get_bus_freq(idata->cpu_clk) / 1000000L;
# if defined(CONFIG_8260)
    bd->bi_cpmfreq = idata->cpm_clk / 1000000L;
    bd->bi_brgfreq = idata->brg_clk / 1000000L;
    bd->bi_sccfreq = idata->scc_clk / 1000000L;
    bd->bi_vco	   = idata->vco_out / 1000000L;
# endif	/* CONFIG_8260 */
#endif	/* CFG_CLKS_IN_HZ */

    bd->bi_baudrate = baudrate;			/* Console Baudrate		*/

#ifdef CFG_EXTBDINFO
    strncpy(bd->bi_s_version, "1.2", sizeof(bd->bi_s_version));
    strncpy(bd->bi_r_version, PPCBOOT_VERSION, sizeof(bd->bi_r_version));

    bd->bi_procfreq    = idata->cpu_clk; /* Processor Speed, In Hz */
    bd->bi_plb_busfreq = bd->bi_busfreq;
    bd->bi_pci_busfreq = bd->bi_busfreq;
#endif

    /* Function pointers must be added after code relocation */
#if 0
    bd->bi_mon_fnc.getc   = NULL;	/* Addr of getc() from Console	*/
    bd->bi_mon_fnc.tstc   = NULL;	/* Addr of tstc() from Console	*/
    bd->bi_mon_fnc.putc   = NULL;	/* Addr of putc()   to Console	*/
    bd->bi_mon_fnc.putstr = NULL;	/* Addr of putstr() to Console	*/
    bd->bi_mon_fnc.printf = NULL;	/* Addr of printf() to Console	*/
    bd->bi_mon_fnc.install_hdlr = NULL;
    bd->bi_mon_fnc.free_hdlr    = NULL;
    bd->bi_mon_fnc.malloc	= NULL;
    bd->bi_mon_fnc.free		= NULL;
#endif

#ifdef DEBUG
    printf ("  New Stack Pointer is: %08lx\n", addr_sp);
#endif

    relocate_code (addr_sp, bd, addr_moni);

    /* NOTREACHED - relocate_code() does not return */
}

void    board_init_r  (bd_t *bd, ulong dest_addr)
{
    cmd_tbl_t	*cmdtp;
    ulong	flash_size;
    ulong	reloc_off = dest_addr - CFG_MONITOR_BASE;
    /* Pointer to initial global data area */
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);

    extern void malloc_bin_reloc (ulong);
#if defined(CONFIG_SPD823TS) || defined(CONFIG_IVMS8)
    void reset_phy(void);
#endif

#ifdef DEBUG
    printf("  Now running in RAM - dest_addr = 0x%08lx\n", dest_addr);
#endif

    /* Save a global pointer to the board info struct */
    bd_ptr = bd ;
    /* Set the monitor function pointer to DPAM init data */
    bd->bi_mon_fnc = &idata->bi_mon_fnc;

    /*
     * We have to relocate the command table manually
     */
    for (cmdtp=&cmd_tbl[0]; cmdtp->name; cmdtp++) {
	ulong addr;
	
	addr = (ulong)(cmdtp->cmd) + reloc_off;
#if 0
	printf ("Command \"%s\": 0x%08lx => 0x%08lx\n",
	    cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
	cmdtp->cmd = (void (*)(struct cmd_tbl_s*,bd_t*,int,int,char*[]))addr;

	addr = (ulong)(cmdtp->name) + reloc_off;
	cmdtp->name = (char *)addr;

	if (cmdtp->usage) {
		addr = (ulong)(cmdtp->usage) + reloc_off;
		cmdtp->usage = (char *)addr;
	}
#ifdef	CFG_LONGHELP
	if (cmdtp->help) {
		addr = (ulong)(cmdtp->help) + reloc_off;
		cmdtp->help = (char *)addr;
	}
#endif
    }

    asm ("sync ; isync");

    /*
     * Setup trap handlers
     */
    trap_init(dest_addr);

    printf ("  FLASH: ");

    if ((flash_size = flash_init ()) > 0) {
    	if (flash_size >= (1 << 20)) {
		printf ("%2ld MB\n", flash_size >> 20);
	} else {
		printf ("%2ld kB\n", flash_size >> 10);
	}
    } else {
	printf (failed);
	hang();
    }
    bd->bi_flashsize = flash_size;   /* size of FLASH memory (final value)	*/

#if defined(CFG_FLASH_ENV_ADDR)
    /*
     * Protect the sector that the environment is in.
     *
     * This scheme (i.e. bi_flashoffset in the board info) cannot
     * really reflect an arbitrarily placed environment flash sector,
     * but lets just do our best, and assume that if CFG_FLASH_ENV_ADDR
     * is defined it will be in a sensible location e.g. either the
     * first sector, or the sector just after the monitor, if it is
     * in flash.
     */
    {
	ulong addr, newoff;
	flash_info_t *fip;
	int i, j;

	/* address of last byte in the environment */
	addr = CFG_FLASH_ENV_ADDR + CFG_FLASH_ENV_SIZE - 1;

	/* make sure it is within the flash */
	if ((fip = addr2info(addr)) == NULL)
	    panic("environment address 0x%08lx is not within flash!\n", addr);

	/* move it to the sector boundary */
	for (i = 0, j = 1; j < fip->sector_count; i++, j++)
	    if (addr >= fip->start[i] && addr < fip->start[j])
		break;
	if (j == fip->sector_count)
	    addr = fip->start[0] + fip->size;
	else
	    addr = fip->start[j];

	/* adjust flashoffset if required */
	newoff = addr - bd->bi_flashstart;
	if (bd->bi_flashoffset < newoff)
	    bd->bi_flashoffset = newoff;
    }
#endif

    /* initialize higher level parts of CPU like time base and timers */
    cpu_init_r (bd);

#ifdef CONFIG_COGENT
    /* miscellaneous platform dependent initialisations */
    misc_init_r(bd);
#endif

#if defined(CONFIG_SPD823TS) || defined(CONFIG_IVMS8)
# ifdef DEBUG
    printf("Reset Ethernet PHY\n");
# endif
    reset_phy ();
#endif

#ifdef CONFIG_PCMCIA
    printf("  PCMCIA: ");
    pcmcia_init();
#endif

#if (CONFIG_COMMANDS & CFG_CMD_IDE)
    printf ("  IDE:   ");
    ide_init(bd);
#endif

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
    printf ("  KGDB:  ");
    kgdb_init();
#endif

#ifdef DEBUG
    printf("Monitor relocated to 0x%08lx\n", dest_addr);
#endif

    /*
     * Enable Interrupts
     */
    interrupt_init (bd);
#ifdef CONFIG_STATUS_LED
    status_led_set (STATUS_LED_BLINKING);
#endif

    udelay(20);

    set_timer(0);

    /* Insert function pointers now that we have relocated the code */

    bd->bi_mon_fnc->install_hdlr = irq_install_handler;
    bd->bi_mon_fnc->free_hdlr    = irq_free_handler;

    /* Initialize other board modules */
#ifdef CONFIG_PCI_PNP
    /*
     * Do pci plug-n-play configuration
     */
    pci_init();
#endif

    bd->bi_mon_fnc->malloc = malloc;
    bd->bi_mon_fnc->free   = free;

    /* initialize malloc() area */
    mem_malloc_init (dest_addr);
    malloc_bin_reloc (reloc_off);

/** LEAVE THIS HERE **/
    /* Initialize devices */
    devices_init();

#ifdef CONFIG_DBOX
    lcd_init();
    fb_init();
#endif

    /* Initialize the console (after the relocation and devices init) */
    console_init_r ();
    putc('\n');
/**********************/

    /* Initialization complete - start the monitor */

    main_loop (bd);

    /* NOTREACHED - start_main does not return; just in case: loop here */
    for (;;);
}

void hang(void)
{
	printf ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
