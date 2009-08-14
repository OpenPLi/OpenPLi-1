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
 *
 */

/*
 * IDE support
 */
#include <mpc8xx_irq.h>
#include <ppcboot.h>
#include <config.h>
#include <command.h>
#include <image.h>
#ifdef CONFIG_IDE_PCMCIA
#include <pcmcia.h>
#endif
#include <ide.h>
#include <ata.h>
#include <cmd_ide.h>
#include <cmd_disk.h>
#include <mpc8xx.h>

/* stdlib.h causes some compatibility problems; should fixe these! -- wd */
#ifndef __ldiv_t_defined
typedef struct {
	long int quot;		/* Quotient	*/
	long int rem;		/* Remainder	*/
} ldiv_t;
extern ldiv_t ldiv (long int __numer, long int __denom);
# define __ldiv_t_defined	1
#endif

#undef	IDE_DEBUG

#ifdef	IDE_DEBUG
#define	PRINTF(fmt,args...)	do {				\
					printf (fmt ,##args);	\
				} while (0)
#else
#define PRINTF(fmt,args...)
#endif

#if (CONFIG_COMMANDS & CFG_CMD_IDE)

/* Timings for IDE Interface
 *
 * SETUP / LENGTH / HOLD - cycles valid for 50 MHz clk
 * 70      165      30     PIO-Mode 0, [ns]
 *  4        9       2                 [Cycles]
 * 50      125      20     PIO-Mode 1, [ns]
 *  3        7       2                 [Cycles]
 * 30      100      15     PIO-Mode 2, [ns]
 *  2        6       1                 [Cycles]
 * 30       80      10     PIO-Mode 3, [ns]
 *  2        5       1                 [Cycles]
 * 25       70      10     PIO-Mode 4, [ns]
 *  2        4       1                 [Cycles]
 */

const static pio_config_t pio_config_ns [IDE_MAX_PIO_MODE+1] =
{
    /*  Setup  Length  Hold  */
	{ 70,   165,    30 },		/* PIO-Mode 0, [ns]	*/
	{ 50,   125,    20 },		/* PIO-Mode 1, [ns]	*/
	{ 30,   101,    15 },		/* PIO-Mode 2, [ns]	*/
	{ 30,    80,    10 },		/* PIO-Mode 3, [ns]	*/
	{ 25,    70,    10 },		/* PIO-Mode 4, [ns]	*/
};

static pio_config_t pio_config_clk [IDE_MAX_PIO_MODE+1];

#ifndef	CFG_PIO_MODE
#define	CFG_PIO_MODE	0		/* use a relaxed default */
#endif
static int pio_mode = CFG_PIO_MODE;

/* Make clock cycles and always round up */

#define PCMCIA_MK_CLKS( t, T ) (( (t) * (T) + 999U ) / 1000U )

/* ------------------------------------------------------------------------- */

/* Current I/O Device	*/
static int curr_device = -1;

/* Current offset for IDE0 / IDE1 bus access	*/
static ulong bus_offset[CFG_IDE_MAXBUS] = {
	CFG_ATA_IDE0_OFFSET,
#ifdef CFG_ATA_IDE1_OFFSET
	CFG_ATA_IDE1_OFFSET,
#endif
};

#define	ATA_CURR_BASE(dev)	(CFG_ATA_BASE_ADDR+bus_offset[IDE_BUS(dev)])

typedef struct ide_dev_id {
	ulong	size;
	uchar	model[40];
	uchar	serial_no[20];
} ide_dev_id_t;

static int	    ide_bus_ok[CFG_IDE_MAXBUS];
static ide_dev_id_t ide_device[CFG_IDE_MAXDEVICE];

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_IDE_LED
static void  ide_led   (uchar led, uchar status);
#else
#define ide_led(a,b)	/* dummy */
#endif

#ifdef CONFIG_IDE_RESET
static void  ide_reset (void);
#else
#define ide_reset()	/* dummy */
#endif

static void  ide_ident (int device);
static void  ide_print (int device);
static uchar ide_wait  (int dev, ulong t);

static void __inline__ outb(int dev, int port, unsigned char val);
static unsigned char __inline__ inb(int dev, int port);
static void input_swap_data(int dev, ulong *sect_buf, int words);
static void input_data(int dev, ulong *sect_buf, int words);
static void output_data(int dev, ulong *sect_buf, int words);
static void trim_trail (unsigned char *str, unsigned int len);

#ifdef CONFIG_IDE_PCMCIA
static void set_pcmcia_timing (int pmode);
#else
#define set_pcmcia_timing(a)	/* dummy */
#endif

/* ------------------------------------------------------------------------- */

void do_ide (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
    switch (argc) {
    case 0:
    case 1:
	printf ("Usage:\n%s\n", cmdtp->usage);
	return;
    case 2:
	if (strncmp(argv[1],"res",3) == 0) {
		puts ("\nReset IDE"
#ifdef CONFIG_IDE_PCMCIA
			" on PCMCIA " PCMCIA_SLOT_MSG
#endif
			": ");

		ide_init(bd);
		return;
	} else if (strncmp(argv[1],"inf",3) == 0) {
		int i;

		putc ('\n');

		for (i=0; i<CFG_IDE_MAXDEVICE; ++i) {
			printf ("IDE device %d: ", i);
			ide_print (i);
		}
		return;

	} else if (strncmp(argv[1],"dev",3) == 0) {
		if ((curr_device < 0) || (curr_device >= CFG_IDE_MAXDEVICE)) {
			puts ("\nno IDE devices available\n");
			return;
		}
		printf ("\nIDE device %d: ", curr_device);
		ide_print (curr_device);
		return;
	} else if (strncmp(argv[1],"part",4) == 0) {
		int dev, ok;

		for (ok=0, dev=0; dev<CFG_IDE_MAXDEVICE; ++dev) {
			if (ide_device[dev].size) {
				++ok;
				if (dev)
					putc ('\n');
				print_part_mac (dev);
			}
		}
		if (!ok)
			puts ("\nno IDE devices available\n");
		return;
	}
	printf ("Usage:\n%s\n", cmdtp->usage);
	return;
    case 3:
	if (strncmp(argv[1],"dev",3) == 0) {
		int dev = (int)simple_strtoul(argv[2], NULL, 10);

		printf ("\nIDE device %d: ", dev);
		if (dev >= CFG_IDE_MAXDEVICE) {
			puts ("unknown device\n");
			return;
		}

		ide_print (dev);

		if (ide_device[dev].size == 0) {
			return;
		}

		curr_device = dev;

		puts ("... is now current device\n");

		return;
	} else if (strncmp(argv[1],"part",4) == 0) {
		int dev = (int)simple_strtoul(argv[2], NULL, 10);

		if (ide_device[dev].size) {
			print_part_mac (dev);
		} else {
			printf ("\nIDE device %d not available\n", dev);
		}
		return;
#if 0
	} else if (strncmp(argv[1],"pio",4) == 0) {
		int mode = (int)simple_strtoul(argv[2], NULL, 10);

		if ((mode >= 0) && (mode <= IDE_MAX_PIO_MODE)) {
			puts ("\nSetting ");
			pio_mode = mode;
			ide_init (bd);
		} else {
			printf ("\nInvalid PIO mode %d (0 ... %d only)\n",
				mode, IDE_MAX_PIO_MODE);
		}
		return;
#endif
	}

	printf ("Usage:\n%s\n", cmdtp->usage);
	return;
    default:
	/* at least 4 args */

	if (strcmp(argv[1],"read") == 0) {
		ulong addr = simple_strtoul(argv[2], NULL, 16);
		ulong blk  = simple_strtoul(argv[3], NULL, 16);
		ulong cnt  = simple_strtoul(argv[4], NULL, 16);
		ulong n;

		printf ("\nIDE read: device %d block # %ld, count %ld ... ",
			curr_device, blk, cnt);

		n = ide_read (curr_device, blk, cnt, (ulong *)addr);

		printf ("%ld blocks read: %s\n",
			n, (n==cnt) ? "OK" : "ERROR");

		return;

	} else if (strcmp(argv[1],"write") == 0) {
		ulong addr = simple_strtoul(argv[2], NULL, 16);
		ulong blk  = simple_strtoul(argv[3], NULL, 16);
		ulong cnt  = simple_strtoul(argv[4], NULL, 16);
		ulong n;

		printf ("\nIDE write: device %d block # %ld, count %ld ... ",
			curr_device, blk, cnt);

		n = ide_write (curr_device, blk, cnt, (ulong *)addr);

		printf ("%ld blocks written: %s\n",
			n, (n==cnt) ? "OK" : "ERROR");

		return;
	} else {
		printf ("Usage:\n%s\n", cmdtp->usage);
	}

	return;
    }
}

void do_diskboot (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	char *boot_device = NULL;
	char *ep;
	int dev, part;
	ulong cnt;
	ulong addr;
	disk_partition_t info;
	image_header_t *hdr;

	switch (argc) {
	case 1:
		addr = CFG_LOAD_ADDR;
		boot_device = getenv ("bootdevice");
		break;
	case 2:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = getenv ("bootdevice");
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		break;
	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return;
	}

	if (!boot_device) {
		puts ("\n** No boot device **\n");
		return;
	}

	dev = simple_strtoul(boot_device, &ep, 16);

	if (ide_device[dev].size == 0) {
		printf ("\n** Device %d not available\n", dev);
		return;
	}

	if (*ep) {
		if (*ep != ':') {
			puts ("\n** Invalid boot device, use `dev[:part]' **\n");
			return;
		}
		part = simple_strtoul(++ep, NULL, 16);
	}

	if (get_partition_info (dev, part, &info)) {
		return;
	}

	if (strncmp(info.type, BOOT_PART_TYPE, sizeof(info.type)) != 0) {
		printf ("\n** Invalid partition type \"%.32s\""
			" (expect \"" BOOT_PART_TYPE "\")\n",
			info.type);
		return;
	}

	printf ("\nLoading from IDE device %d, partition %d: "
		"Name: %.32s  Type: %.32s\n",
		dev, part, info.name, info.type);

	PRINTF ("First Block: %ld,  # of blocks: %ld, Block Size: %ld\n",
		info.start, info.size, info.blksz);

	if (ide_read (dev, info.start, 1, (ulong *)addr) != 1) {
		printf ("** Read error on %d:%d\n", dev, part);
		return;
	}

	hdr = (image_header_t *)addr;

	if (hdr->ih_magic == IH_MAGIC) {
		
		print_image_hdr (hdr);

		cnt = (hdr->ih_size + sizeof(image_header_t));
		cnt += info.blksz - 1;
		cnt /= info.blksz;
		cnt -= 1;
	} else {
		cnt = info.size - 1;
	}

	if (ide_read (dev, info.start+1, cnt,
		      (ulong *)(addr+info.blksz)) != cnt) {
		printf ("** Read error on %d:%d\n", dev, part);
		return;
	}


	/* Loading ok, update default load address */

	load_addr = addr;

	/* Check if we should attempt an auto-start */
	if (((ep = getenv("autostart")) != NULL) && (strcmp(ep,"yes") == 0)) {
		char *local_args[2];
		extern void do_bootm (cmd_tbl_t *, bd_t *, int, int, char *[]);

		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf ("Automatic boot of image at addr 0x%08lX ...\n", addr);

		do_bootm (cmdtp, bd, 0, 1, local_args);
	}
}

/* ------------------------------------------------------------------------- */

void ide_init (bd_t *bd)
{
#ifdef CONFIG_IDE_PCMCIA
	volatile immap_t *immr = (immap_t *)CFG_IMMR;
	volatile pcmconf8xx_t *pcmp = &(immr->im_pcmcia);
#endif
	unsigned char c;
	int i, bus;

	/* Initialize PIO timing tables */
	for (i=0; i <= IDE_MAX_PIO_MODE; ++i) {
	    pio_config_clk[i].t_setup  = PCMCIA_MK_CLKS(pio_config_ns[i].t_setup,
							    bd->bi_busfreq);
	    pio_config_clk[i].t_length = PCMCIA_MK_CLKS(pio_config_ns[i].t_length,
							    bd->bi_busfreq);
	    pio_config_clk[i].t_hold   = PCMCIA_MK_CLKS(pio_config_ns[i].t_hold,
							    bd->bi_busfreq);
	    PRINTF ("PIO Mode %d: setup=%2d ns/%d clk"
		    "  len=%3d ns/%d clk"
		    "  hold=%2d ns/%d clk\n",
		    i,
		    pio_config_ns[i].t_setup,  pio_config_clk[i].t_setup,
		    pio_config_ns[i].t_length, pio_config_clk[i].t_length,
		    pio_config_ns[i].t_hold,   pio_config_clk[i].t_hold);
	}


	/* Reset the IDE just to be sure.
	 * Light LED's to show
	 */
	ide_led ((LED_IDE1 | LED_IDE2), 1);		/* LED's on	*/
	ide_reset ();

#ifdef CONFIG_IDE_PCMCIA
	/* PCMCIA / IDE initialization for common mem space */
	pcmp->pcmc_pgcrb = 0;
#endif

	/* start in PIO mode 0 - most relaxed timings */
	pio_mode = 0;
	set_pcmcia_timing (pio_mode);

	/*
	 * Wait for IDE to get ready.
	 * According to spec, this can take up to 31 seconds!
	 */
	for (bus=0; bus<CFG_IDE_MAXBUS; ++bus) {
		int dev = bus * (CFG_IDE_MAXDEVICE / CFG_IDE_MAXBUS);

		printf ("Bus %d: ", bus);

		ide_bus_ok[bus] = 0;

		/* Select device
		 */
		udelay (100000);		/* 100 ms */
		outb (dev, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(dev));
		udelay (100000);		/* 100 ms */

		i = 0;
		do {
			udelay (10000);		/* 10 ms */

			c = inb (dev, ATA_STATUS);
			if (i > (ATA_RESET_TIME * 100)) {
				puts ("** Timeout **\n");
				ide_led ((LED_IDE1 | LED_IDE2), 0); /* LED's off */
				return;
			}
			if ((i >= 100) && ((i%100)==0)) {
				putc ('.');
			}
			i++;
		} while (c & ATA_STAT_BUSY);

		if (c & (ATA_STAT_BUSY | ATA_STAT_FAULT)) {
			printf ("Status 0x%02x ", c);
		} else if ((c & ATA_STAT_READY) == 0) {
			puts ("not available  ");
		} else {
			puts ("OK  ");
			ide_bus_ok[bus] = 1;
		}
	}
	putc ('\n');

	ide_led ((LED_IDE1 | LED_IDE2), 0);	/* LED's off	*/

	curr_device = -1;
	for (i=0; i<CFG_IDE_MAXDEVICE; ++i) {
#ifdef CONFIG_IDE_LED
		int led = (IDE_BUS(i) == 0) ? LED_IDE1 : LED_IDE2;
#endif

		if (!ide_bus_ok[IDE_BUS(i)])
			continue;
		ide_led (led, 1);		/* LED on	*/
		ide_ident (i);
		ide_led (led, 0);		/* LED off	*/
		ide_print (i);

		/* make first available device current */
		if ((ide_device[i].size > 0) && (curr_device < 0)) {
			curr_device = i;
		}
	}
}

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_IDE_PCMCIA

static void
set_pcmcia_timing (int pmode)
{
	volatile immap_t *immr = (immap_t *)CFG_IMMR;
	volatile pcmconf8xx_t *pcmp = &(immr->im_pcmcia);
	ulong timings;

	timings = PCMCIA_SHT(pio_config_clk[pmode].t_hold)
		| PCMCIA_SST(pio_config_clk[pmode].t_setup)
		| PCMCIA_SL (pio_config_clk[pmode].t_length)
		;

	/* IDE 0
	 */
	pcmp->pcmc_pbr0 = CFG_PCMCIA_PBR0;
	pcmp->pcmc_por0 = CFG_PCMCIA_POR0
#if (CFG_PCMCIA_POR0 != 0)
			| timings
#endif
			;
	PRINTF ("PBR0: %08x  POR0: %08x\n", pcmp->pcmc_pbr0, pcmp->pcmc_por0);

	pcmp->pcmc_pbr1 = CFG_PCMCIA_PBR1;
	pcmp->pcmc_por1 = CFG_PCMCIA_POR1
#if (CFG_PCMCIA_POR1 != 0)
			| timings
#endif
			;
	PRINTF ("PBR1: %08x  POR1: %08x\n", pcmp->pcmc_pbr1, pcmp->pcmc_por1);

	pcmp->pcmc_pbr2 = CFG_PCMCIA_PBR2;
	pcmp->pcmc_por2 = CFG_PCMCIA_POR2
#if (CFG_PCMCIA_POR2 != 0)
			| timings
#endif
			;
	PRINTF ("PBR2: %08x  POR2: %08x\n", pcmp->pcmc_pbr2, pcmp->pcmc_por2);

	pcmp->pcmc_pbr3 = CFG_PCMCIA_PBR3;
	pcmp->pcmc_por3 = CFG_PCMCIA_POR3
#if (CFG_PCMCIA_POR3 != 0)
			| timings
#endif
			;
	PRINTF ("PBR3: %08x  POR3: %08x\n", pcmp->pcmc_pbr3, pcmp->pcmc_por3);

	/* IDE 1
	 */
	pcmp->pcmc_pbr4 = CFG_PCMCIA_PBR4;
	pcmp->pcmc_por4 = CFG_PCMCIA_POR4
#if (CFG_PCMCIA_POR4 != 0)
			| timings
#endif
			;
	PRINTF ("PBR4: %08x  POR4: %08x\n", pcmp->pcmc_pbr4, pcmp->pcmc_por4);

	pcmp->pcmc_pbr5 = CFG_PCMCIA_PBR5;
	pcmp->pcmc_por5 = CFG_PCMCIA_POR5
#if (CFG_PCMCIA_POR5 != 0)
			| timings
#endif
			;
	PRINTF ("PBR5: %08x  POR5: %08x\n", pcmp->pcmc_pbr5, pcmp->pcmc_por5);

	pcmp->pcmc_pbr6 = CFG_PCMCIA_PBR6;
	pcmp->pcmc_por6 = CFG_PCMCIA_POR6
#if (CFG_PCMCIA_POR6 != 0)
			| timings
#endif
			;
	PRINTF ("PBR6: %08x  POR6: %08x\n", pcmp->pcmc_pbr6, pcmp->pcmc_por6);

	pcmp->pcmc_pbr7 = CFG_PCMCIA_PBR7;
	pcmp->pcmc_por7 = CFG_PCMCIA_POR7
#if (CFG_PCMCIA_POR7 != 0)
			| timings
#endif
			;
	PRINTF ("PBR7: %08x  POR7: %08x\n", pcmp->pcmc_pbr7, pcmp->pcmc_por7);

}

#endif	/* CONFIG_IDE_PCMCIA */

/* ------------------------------------------------------------------------- */

static void __inline__
outb(int dev, int port, unsigned char val)
{
	/* Ensure I/O operations complete */
	__asm__ volatile("eieio");
	*((uchar *)(ATA_CURR_BASE(dev)+port)) = val;
#if 0
printf ("OUTB: 0x%08lx <== 0x%02x\n", ATA_CURR_BASE(dev)+port, val);
#endif
}

static unsigned char __inline__
inb(int dev, int port)
{
	uchar val;
	/* Ensure I/O operations complete */
	__asm__ volatile("eieio");
	val = *((uchar *)(ATA_CURR_BASE(dev)+port));
#if 0
printf ("INB: 0x%08lx ==> 0x%02x\n", ATA_CURR_BASE(dev)+port, val);
#endif
	return (val);
}

__inline__ unsigned ld_le16(const volatile unsigned short *addr)
{
	unsigned val;

	__asm__ __volatile__ ("lhbrx %0,0,%1" : "=r"(val) : "r"(addr), "m"(*addr));
	return val;
}	

static void
input_swap_data(int dev, ulong *sect_buf, int words)
{
	volatile ushort	*pbuf = (ushort *)(ATA_CURR_BASE(dev)+ATA_DATA_REG);
	ushort	*dbuf = (ushort *)sect_buf;

	while (words--) {
		*dbuf++ = ld_le16(pbuf);
		*dbuf++ = ld_le16(pbuf);
	}
}

static void
output_data(int dev, ulong *sect_buf, int words)
{
	ushort	*dbuf;
	volatile ushort	*pbuf;

	pbuf = (ushort *)(ATA_CURR_BASE(dev)+ATA_DATA_REG);
	dbuf = (ushort *)sect_buf;
	while (words--) {
		__asm__ volatile ("eieio");
		*pbuf = *dbuf++;
		__asm__ volatile ("eieio");
		*pbuf = *dbuf++;
	}
}

static void
input_data(int dev, ulong *sect_buf, int words)
{
	ushort	*dbuf;
	volatile ushort	*pbuf;

	pbuf = (ushort *)(ATA_CURR_BASE(dev)+ATA_DATA_REG);
	dbuf = (ushort *)sect_buf;
	while (words--) {
		__asm__ volatile ("eieio");
		*dbuf++ = *pbuf;
		__asm__ volatile ("eieio");
		*dbuf++ = *pbuf;
	}
}

/* -------------------------------------------------------------------------
 */
static void ide_ident (int device)
{
	ulong iobuf[ATA_SECTORWORDS];
	unsigned char c;
	hd_driveid_t *iop = (hd_driveid_t *)iobuf;
	ide_dev_id_t *idp = &(ide_device[device]);
#if 0
	int mode, cycle_time;
#endif

	printf ("    Device %d: ", device);

	ide_led (DEVICE_LED(device), 1);	/* LED on	*/
	/* Select device
	 */
	outb (device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));

	/* Start Ident Command
	 */
	outb (device, ATA_COMMAND, ATA_CMD_IDENT);

	/* Wait for completion
	 */
	c = ide_wait (device, 100);

	ide_led (DEVICE_LED(device), 0);	/* LED off	*/

	if (((c & ATA_STAT_READY) == 0) ||
	    ((c & (ATA_STAT_FAULT|ATA_STAT_ERR)) != 0) ) {
		idp->size = 0;
		idp->model[0] = idp->serial_no[0] = '\0';
		return;
	}

	input_swap_data (device, iobuf, ATA_SECTORWORDS);

	trim_trail (iop->model,     sizeof(iop->model));
	trim_trail (iop->serial_no, sizeof(iop->serial_no));

#if 0
	/*
	 * Drive PIO mode autoselection
	 */
	mode = iop->tPIO;

	printf ("tPIO = 0x%02x = %d\n",mode, mode);
	if (mode > 2) {		/* 2 is maximum allowed tPIO value */
		mode = 2;
		PRINTF ("Override tPIO -> 2\n");
	}
	if (iop->field_valid & 2) {	/* drive implements ATA2? */
		PRINTF ("Drive implements ATA2\n");
		if (iop->capability & 8) {	/* drive supports use_iordy? */
			cycle_time = iop->eide_pio_iordy; 
		} else {
			cycle_time = iop->eide_pio;
		}
		PRINTF ("cycle time = %d\n", cycle_time);
		mode = 4;
		if (cycle_time > 120) mode = 3;	/* 120 ns for PIO mode 4 */
		if (cycle_time > 180) mode = 2;	/* 180 ns for PIO mode 3 */
		if (cycle_time > 240) mode = 1;	/* 240 ns for PIO mode 4 */
		if (cycle_time > 383) mode = 0;	/* 383 ns for PIO mode 4 */
	}
printf ("PIO mode to use: PIO %d\n", mode);
#endif

	/* swap shorts */
	idp->size = (iop->lba_capacity << 16) | (iop->lba_capacity >> 16);

	strncpy (idp->model,     iop->model,     sizeof(idp->model));
	strncpy (idp->serial_no, iop->serial_no, sizeof(idp->serial_no));
}

/* ------------------------------------------------------------------------- */

static void ide_print (int device)
{
	ldiv_t mb, gb;
	ide_dev_id_t *idp = &(ide_device[device]);
	char *mod, *ser;

	if (idp->size == 0) {
		puts ("not available\n");
		return;
	}

	mb = ldiv(idp->size, ((1024 * 1024) / 512)); /* MB */
	/* round to 1 digit */
	mb.rem *= 10 * 512;
	mb.rem += 512 * 1024;
	mb.rem /= 1024 * 1024;

	gb = ldiv(10 * mb.quot + mb.rem, 10240);
	gb.rem += 512;
	gb.rem /= 1024;

	mod = idp->model;	while (*mod && (*mod==' ')) ++mod;
	ser = idp->serial_no;	while (*ser && (*ser==' ')) ++ser;

	printf ("Model: %s  Serial #: %s  ", mod, ser);
	printf ("Capacity: %ld.%ld MB = %ld.%ld GB\n",
		mb.quot, mb.rem, gb.quot, gb.rem);
}

/* ------------------------------------------------------------------------- */

ulong ide_read (int device, ulong blknr, ulong blkcnt, ulong *buffer)
{
	ulong n = 0;
	unsigned char c;

	ide_led (DEVICE_LED(device), 1);	/* LED on	*/

	/* Select device
	 */
	outb (device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));

	while (blkcnt-- > 0) {

		c = ide_wait (device, 500);

		if (c & ATA_STAT_BUSY) {
			printf ("IDE read: device %d not ready\n", device);
			goto RD_OUT;
		}

		outb (device, ATA_SECT_CNT, 1);
		outb (device, ATA_LBA_LOW,  (blknr >>  0) & 0xFF);
		outb (device, ATA_LBA_MID,  (blknr >>  8) & 0xFF);
		outb (device, ATA_LBA_HIGH, (blknr >> 16) & 0xFF);
		outb (device, ATA_DEV_HD,   ATA_LBA | ATA_DEVICE(device));
		outb (device, ATA_COMMAND,  ATA_CMD_READ);

		udelay (50);

		c = ide_wait (device, 500);	/* can't take over 500 ms */

		if ((c&(ATA_STAT_DRQ|ATA_STAT_BUSY|ATA_STAT_ERR)) != ATA_STAT_DRQ) {
			printf ("Error (no IRQ) dev %d blk %ld: status 0x%02x\n",
				device, blknr, c);
			goto RD_OUT;
		}

		input_data (device, buffer, ATA_SECTORWORDS);
		(void) inb (device, ATA_STATUS);	/* clear IRQ */

		++n;
		++blknr;
		buffer += ATA_SECTORWORDS;
	}
RD_OUT:
	ide_led (DEVICE_LED(device), 0);	/* LED off	*/
	return (n);
}

/* ------------------------------------------------------------------------- */


ulong ide_write (int device, ulong blknr, ulong blkcnt, ulong *buffer)
{
	ulong n = 0;
	unsigned char c;

	ide_led (DEVICE_LED(device), 1);	/* LED on	*/

	/* Select device
	 */
	outb (device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));

	while (blkcnt-- > 0) {

		c = ide_wait (device, 500);

		if (c & ATA_STAT_BUSY) {
			printf ("IDE read: device %d not ready\n", device);
			goto WR_OUT;
		}

		outb (device, ATA_SECT_CNT, 1);
		outb (device, ATA_LBA_LOW,  (blknr >>  0) & 0xFF);
		outb (device, ATA_LBA_MID,  (blknr >>  8) & 0xFF);
		outb (device, ATA_LBA_HIGH, (blknr >> 16) & 0xFF);
		outb (device, ATA_DEV_HD,   ATA_LBA | ATA_DEVICE(device));
		outb (device, ATA_COMMAND,  ATA_CMD_WRITE);

		udelay (50);

		c = ide_wait (device, 500);	/* can't take over 500 ms */

		if ((c&(ATA_STAT_DRQ|ATA_STAT_BUSY|ATA_STAT_ERR)) != ATA_STAT_DRQ) {
			printf ("Error (no IRQ) dev %d blk %ld: status 0x%02x\n",
				device, blknr, c);
			goto WR_OUT;
		}

		output_data (device, buffer, ATA_SECTORWORDS);
		c = inb (device, ATA_STATUS);	/* clear IRQ */
		++n;
		++blknr;
		buffer += ATA_SECTORWORDS;
	}
WR_OUT:
	ide_led (DEVICE_LED(device), 0);	/* LED off	*/
	return (n);
}

/* ------------------------------------------------------------------------- */

/* Trim trailing blanks, and NUL-terminate string
 */
static void trim_trail (unsigned char *str, unsigned int len)
{
	unsigned char *p = str + len - 1;

	while (len-- > 0) {
		*p-- = '\0';
		if (*p != ' ') {
			return;
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * Wait until Busy bit is off, or timeout (in ms)
 * Return last status
 */
static uchar ide_wait (int dev, ulong t)
{
	ulong delay = 10 * t;		/* poll every 100 us */
	uchar c;

	while ((c = inb(dev, ATA_STATUS)) & ATA_STAT_BUSY) {
		udelay (100);
		if (delay-- == 0) {
			break;
		}
	}
	return (c);
}

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_IDE_RESET

static void ide_reset (void)
{
	int i;
	volatile immap_t *immr = (immap_t *)CFG_IMMR;

	curr_device = -1;
	for (i=0; i<CFG_IDE_MAXBUS; ++i)
		ide_bus_ok[i] = 0;
	for (i=0; i<CFG_IDE_MAXDEVICE; ++i)
		ide_device[i].size = 0;

#if defined(CFG_PC_IDE_RESET)
	/* Configure PC for IDE Reset Pin
	 */
	immr->im_ioport.iop_pcdat &= ~(CFG_PC_IDE_RESET);	/* Set reset bit */
	immr->im_ioport.iop_pcpar &= ~(CFG_PC_IDE_RESET);
	immr->im_ioport.iop_pcso  &= ~(CFG_PC_IDE_RESET);
	immr->im_ioport.iop_pcdir |=   CFG_PC_IDE_RESET;	/* Make output	*/

#if 1
	/* assert IDE RESET signal */
	immr->im_ioport.iop_pcdat &= ~(CFG_PC_IDE_RESET);
	udelay (20000);
	/* de-assert RESET signal of IDE */
	immr->im_ioport.iop_pcdat |=   CFG_PC_IDE_RESET;
#else
	/* assert IDE RESET signal */
	immr->im_ioport.iop_pcdat |=   CFG_PC_IDE_RESET;
	udelay (20000);
	/* de-assert RESET signal of IDE */
	immr->im_ioport.iop_pcdat &= ~(CFG_PC_IDE_RESET);
#endif
#else
#error IDE reset pin not configured
#endif

#ifdef CFG_PB_IDE_MOTOR
	immr->im_cpm.cp_pbpar &= ~(CFG_PB_IDE_MOTOR);	/* IDE Motor in pin */
	immr->im_cpm.cp_pbodr &= ~(CFG_PB_IDE_MOTOR);
	immr->im_cpm.cp_pbdir &= ~(CFG_PB_IDE_MOTOR); /* input */
	if ((immr->im_cpm.cp_pbdat & CFG_PB_IDE_MOTOR) == 0) {
		printf ("\nWarning: 5V for IDE Motor missing\n");
	}
#endif	/* CFG_PB_IDE_MOTOR */

	for (i=0; i<25; ++i) {
		udelay (10000);
	}
}

#endif	/* CONFIG_IDE_RESET */

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_IDE_LED

static	uchar	led_buffer = 0;		/* Buffer for current LED status	*/

static void ide_led (uchar led, uchar status)
{
	uchar *led_port = LED_PORT;

	if (status)	{		/* switch LED on	*/
		led_buffer |=  led;
	} else {			/* switch LED off	*/
		led_buffer &= ~led;
	}

	*led_port = led_buffer;
}

#endif	/* CONFIG_IDE_LED */

/* ------------------------------------------------------------------------- */

#endif	/* CONFIG_COMMANDS & CFG_CMD_IDE */
