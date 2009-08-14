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
#include "mpc8xx.h"

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] =
{
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEAFBC04, 0x11AF7C04, 0xEFBAFC00,
	0x1FF5FC47, /* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
		    0x1FF5FC34, 0xEFEABC34, 0x1FB57C35, /* last */
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEAFBC04, 0x10AF7C04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBAFC00, 0x1FF5FC47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEABBC00, 0x01B27C04, 0x1FF5FC47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEABBC00, 0x10A77C00, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BAFC04, 0x1FF5FC47, /* last */
					    _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1FFD7C84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07, /* last */
				_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7FFFFC07, /* last */
		    _NOT_USED_, _NOT_USED_, _NOT_USED_,
};

const uint sdram_table_upmb_nokia[] =
{
	0xFFF3CC04, 0xCFF7CC04, 0x0FFFDC04, 0x0FFFDC84,
	0x0FFFDC04, 0x0FFFDC84, 0x0FF3CC00, 0xFFF3CC47,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	0xFFFFCC04, 0xCFFFCC04, 0x00FFCC04, 0x00FFDC84,
	0x00FFDC04, 0x00FFDC84, 0x0FF3CC00, 0xFFF7CC47,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	0x7FFFFC07, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 * Test TQ ID string (TQM8xx...)
 * If present, check for "L" type (no second DRAM bank),
 * otherwise "L" type is assumed as default.
 *
 * Return 1 for "L" type, 0 else.
 */

int checkboard (void)
{
    unsigned char *s = getenv("serial#");
    unsigned char *e;
    int l_type;

    if (!s || strncmp(s, "TQM8", 4)) {
	printf ("### No HW ID - assuming TQM8xxL\n");
	return (1);
    }

    l_type = (*(s+6) == 'L');

    for (e=s; *e; ++e) {
	if (*e == ' ')
	    break;
    }

    for ( ; s<e; ++s) {
	putc (*s);
    }
    putc ('\n');

    return (l_type);
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
    volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
    volatile memctl8xx_t *memctl = &immap->im_memctl;
    long int size_b0, size_b1, size8, size9;
    
    unsigned char mID = *(char*)(0x1001ffe0);

    switch (mID)
    {
    case 1:
    	upmconfig(UPMB, (uint *)sdram_table_upmb_nokia, sizeof(sdram_table_upmb_nokia)/sizeof(uint));
	break;
    }

    printf("(faked) ");
    return 32*1024*1024;

    upmconfig(UPMA, (uint *)sdram_table, sizeof(sdram_table)/sizeof(uint));

    /*
     * Preliminary prescaler for refresh (depends on number of
     * banks): This value is selected for four cycles every 62.4 us
     * with two SDRAM banks or four cycles every 31.2 us with one
     * bank. It will be adjusted after memory sizing.
     */
    memctl->memc_mptpr = CFG_MPTPR_2BK_8K;

    memctl->memc_mar  = 0x00000088;

    /*
     * Map controller banks 2 and 3 to the SDRAM banks 2 and 3 at
     * preliminary addresses - these have to be modified after the
     * SDRAM size has been determined.
     */
    memctl->memc_or2 = CFG_OR2_PRELIM;
    memctl->memc_br2 = CFG_BR2_PRELIM;

    if (board_type == 0) {	/* "L" type boards have only one bank SDRAM */
	memctl->memc_or3 = CFG_OR3_PRELIM;
	memctl->memc_br3 = CFG_BR3_PRELIM;
    }

    memctl->memc_mamr = CFG_MAMR_8COL & (~(MAMR_PTAE)); /* no refresh yet */

    udelay(200);

    /* perform SDRAM initializsation sequence */

    memctl->memc_mcr  = 0x80004105;	/* SDRAM bank 0 */
    udelay(1);
    memctl->memc_mcr  = 0x80004230;	/* SDRAM bank 0 - execute twice */
    udelay(1);

    if (board_type == 0) {	/* "L" type boards have only one bank SDRAM */
	memctl->memc_mcr  = 0x80006105;	/* SDRAM bank 1 */
	udelay(1);
	memctl->memc_mcr  = 0x80006230;	/* SDRAM bank 1 - execute twice */
	udelay(1);
    }

    memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

    udelay (1000);

    /*
     * Check Bank 0 Memory Size for re-configuration
     *
     * try 8 column mode
     */
    size8 = dram_size (CFG_MAMR_8COL, (ulong *)SDRAM_BASE2_PRELIM, SDRAM_MAX_SIZE);

    udelay (1000);

    /*
     * try 9 column mode
     */
    size9 = dram_size (CFG_MAMR_9COL, (ulong *)SDRAM_BASE2_PRELIM, SDRAM_MAX_SIZE);

    if (size8 < size9) {		/* leave configuration at 9 columns	*/
	size_b0 = size9;
/*	debug ("SDRAM Bank 0 in 9 column mode: %ld MB\n", size >> 20);	*/
    } else {				/* back to 8 columns			*/
	size_b0 = size8;
	memctl->memc_mamr = CFG_MAMR_8COL;
	udelay(500);
/*	debug ("SDRAM Bank 0 in 8 column mode: %ld MB\n", size >> 20);	*/
    }

    if (board_type == 0) {	/* "L" type boards have only one bank SDRAM	*/
	/*
	 * Check Bank 1 Memory Size
	 * use current column settings
	 * [9 column SDRAM may also be used in 8 column mode,
	 *  but then only half the real size will be used.]
	 */
	size_b1 = dram_size (memctl->memc_mamr, (ulong *)SDRAM_BASE3_PRELIM,
				SDRAM_MAX_SIZE);
/*	debug ("SDRAM Bank 1: %ld MB\n", size8 >> 20);	*/
    } else {
	size_b1 = 0;
    }

    udelay (1000);

    /*
     * Adjust refresh rate depending on SDRAM type, both banks
     * For types > 128 MBit leave it at the current (fast) rate
     */
    if ((size_b0 < 0x02000000) && (size_b1 < 0x02000000)) {
	/* reduce to 15.6 us (62.4 us / quad) */
	memctl->memc_mptpr = CFG_MPTPR_2BK_4K;
	udelay(1000);
    }

    /*
     * Final mapping: map bigger bank first
     */
    if (size_b1 > size_b0) {	/* SDRAM Bank 1 is bigger - map first	*/

	memctl->memc_or3 = ((-size_b1) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
	memctl->memc_br3 = (CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

	if (size_b0 > 0) {
	    /*
	     * Position Bank 0 immediately above Bank 1
	     */
	    memctl->memc_or2 = ((-size_b0) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
	    memctl->memc_br2 = ((CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V)
	    		       + size_b1;
	} else {
	    unsigned long reg;
	    /*
	     * No bank 0
	     *
	     * invalidate bank
	     */
	    memctl->memc_br2 = 0;

	    /* adjust refresh rate depending on SDRAM type, one bank */
	    reg = memctl->memc_mptpr;
	    reg >>= 1;	/* reduce to CFG_MPTPR_1BK_8K / _4K */
	    memctl->memc_mptpr = reg;
	}

    } else {			/* SDRAM Bank 0 is bigger - map first	*/

	memctl->memc_or2 = ((-size_b0) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
	memctl->memc_br2 = (CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

	if (size_b1 > 0) {
	    /*
	     * Position Bank 1 immediately above Bank 0
	     */
	    memctl->memc_or3 = ((-size_b1) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
	    memctl->memc_br3 = ((CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V)
	    		       + size_b0;
	} else {
	    unsigned long reg;
	    /*
	     * No bank 1
	     *
	     * invalidate bank
	     */
	    memctl->memc_br3 = 0;

	    /* adjust refresh rate depending on SDRAM type, one bank */
	    reg = memctl->memc_mptpr;
	    reg >>= 1;	/* reduce to CFG_MPTPR_1BK_8K / _4K */
	    memctl->memc_mptpr = reg;
	}
    }

    udelay(10000);

    return (size_b0 + size_b1);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value, long int *base, long int maxsize)
{
    volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
    volatile memctl8xx_t *memctl = &immap->im_memctl;
    volatile long int	 *addr;
    long int		  cnt, val;

    memctl->memc_mamr = mamr_value;

    for (cnt = maxsize/sizeof(long); cnt > 0; cnt >>= 1) {
	addr = base + cnt;	/* pointer arith! */

	*addr = ~cnt;
    }

    /* write 0 to base address */
    addr = base;
    *addr = 0;

    /* check at base address */
    if ((val = *addr) != 0) {
	return (0);
    }

    for (cnt = 1; ; cnt <<= 1) {
	addr = base + cnt;	/* pointer arith! */

	val = *addr;

	if (val != (~cnt)) {
	    return (cnt * sizeof(long));
	}
    }
    /* NOTREACHED */
}
