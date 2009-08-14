/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@cmst.csiro.au> and
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

/*
 * KGDB support
 */
#ifndef	_CMD_KGDB_H
#define	_CMD_KGDB_H

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define	CMD_TBL_KGDB	MK_CMD_TBL_ENTRY(				\
	"kgdb",	4,	CFG_MAXARGS,	1,	do_kgdb,		\
	"kgdb    - enter gdb remote debug mode\n",			\
	"\n    - executes a breakpoint so that kgdb mode is\n"		\
	"      entered via the exception handler. To return\n"		\
	"      to the monitor, the remote gdb debugger must\n"		\
	"      execute a \"continue\" command.\n"			\
),

void do_kgdb (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);
#else
#define CMD_TBL_KGDB
#endif

#endif	/* _CMD_KGDB_H */
