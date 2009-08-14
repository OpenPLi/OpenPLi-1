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
#include "cpci405.h"

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

    if (!s || strncmp(s, "CPCI405", 7)) {
	printf ("### No HW ID - assuming CPCI405\n");
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
  return (16 * 1024*1024);
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
    /* TODO: XXX XXX XXX */
    printf ("test: 16 MB - ok\n");

    return (0);
}

/* ------------------------------------------------------------------------- */
