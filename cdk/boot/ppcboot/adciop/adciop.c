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
#include "adciop.h"

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

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

    if (!s || strncmp(s, "ADCIOP", 6)) {
	printf ("### No HW ID - assuming ADCIOP\n");
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
#if 0  /* test-only */
    volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
    volatile memctl8xx_t *memctl = &immap->im_memctl;
    long int size_b0, size_b1, size8, size9;

    /*
     * Preliminary prescaler for refresh (depends on number of
     * banks): This value is selected for four cycles every 62.4 us
     * with two SDRAM banks or four cycles every 31.2 us with one
     * bank. It will be adjusted after memory sizing.
     */
    memctl->memc_mptpr = CFG_MPTPR_2BK_8K;

    memctl->memc_mamr = CFG_MAMR_8COL;

    upmconfig(UPMA, (uint *)sdram_table, sizeof(sdram_table)/sizeof(uint));

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

    /* perform SDRAM initializsation sequence */
    memctl->memc_mar  = 0x00000088;

    memctl->memc_mcr  = 0x80004105;	/* SDRAM bank 0 */

    
    if (board_type == 0) {	/* "L" type boards have only one bank SDRAM */
	memctl->memc_mcr  = 0x80006105;	/* SDRAM bank 1 */
    }

    memctl->memc_mcr  = 0x80004230;	/* SDRAM bank 0 - execute twice */

    if (board_type == 0) {	/* "L" type boards have only one bank SDRAM */
	memctl->memc_mcr  = 0x80006230;	/* SDRAM bank 1 - execute twice */
    }

    /*
     * Check Bank 0 Memory Size for re-configuration
     *
     * try 8 column mode
     */
    size8 = dram_size (CFG_MAMR_8COL, (ulong *)SDRAM_BASE2_PRELIM, SDRAM_MAX_SIZE);

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

    /*
     * Adjust refresh rate depending on SDRAM type, both banks
     * For types > 128 MBit leave it at the current (fast) rate
     */
    if ((size_b0 < 0x02000000) && (size_b1 < 0x02000000)) {
	/* reduce to 15.6 us (62.4 us / quad) */
	memctl->memc_mptpr = CFG_MPTPR_2BK_4K;
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

    return (size_b0 + size_b1);
#else
    return (16 * 1024*1024);
#endif
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
    /* TODO: XXX XXX XXX */
    printf ("test: 16 MB - ok\n");

    return (0);
}

/* ------------------------------------------------------------------------- */
