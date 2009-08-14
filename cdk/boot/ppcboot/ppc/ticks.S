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

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

/*
 * unsigned long long get_ticks(void);
 *
 * read timebase as "long long"
 */
	.globl	get_ticks
get_ticks:
1:	mftbu	r3
	mftb	r4
	mftbu	r5
	cmp	0,r3,r5
	bne	1b
	blr

/*
 * Delay for a number of ticks
 */
	.globl	wait_ticks
wait_ticks:
1:	mftbu	r4
	mftb	r5
	mftbu	r6
	cmp	0,r4,r6
	bne	1b		/* Get [synced] base time */
	addc	r9,r5,r3	/* Compute end time */
	addze	r8,r4
2:	mftbu	r4
	cmp	0,r4,r8
	blt	2b
	bgt	3f
	mftb	r5
	cmp	0,r5,r9
	blt	2b
3:	blr
