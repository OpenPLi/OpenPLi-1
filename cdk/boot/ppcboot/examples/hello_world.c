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

#define	MON_PRINT(fmt,args...)	\
	(*(void (*)(const char *, ...))(bd->bi_mon_fnc->printf)) (fmt ,##args)

#define MON_GETC		\
	(*(int (*)(void))(bd->bi_mon_fnc->getc))

#define MON_TSTC		\
	(*(int (*)(void))(bd->bi_mon_fnc->tstc))

int hello_world (bd_t *bd, int argc, char *argv[])
{
	int i;

	MON_PRINT ("Hello World\n");

	MON_PRINT ("argc = %d\n", argc);

	for (i=0; i<=argc; ++i) {
		MON_PRINT ("argv[%d] = \"%s\"\n",
			i,
			argv[i] ? argv[i] : "<NULL>");
	}

        MON_PRINT (" beginning ROM-DUMP:\n");
        for (i=0x10000000; i<0x10080000; i++)
           MON_PRINT ("%02x ", ((unsigned char*)0)[i]);

	MON_PRINT ("Hit any key to exit ... ");
	while (!MON_TSTC())
		;
	/* consume input */
	(void) MON_GETC();

	MON_PRINT ("\n\n");
	return (0);
}
