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

/*
 * Boot support
 */
#ifndef	_CMD_NVEDIT_H
#define	_CMD_NVEDIT_H

#define	CMD_TBL_PRINTENV	MK_CMD_TBL_ENTRY(				\
	"printenv",	8,	CFG_MAXARGS,	1,	do_printenv,		\
	"printenv- print environment variables\n",				\
	"\n    - print values of all environment variables\n"			\
	"printenv name ...\n"							\
	"    - print value of environment variable 'name'\n"			\
),
void do_printenv (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);

#define CMD_TBL_SETENV		MK_CMD_TBL_ENTRY(				\
	"setenv",	6,	CFG_MAXARGS,	1,	do_setenv,		\
	"setenv  - set environment variables\n",				\
	"name value ...\n"							\
	"    - set environment variable 'name' to 'value ...'\n"		\
	"setenv name\n"								\
	"    - delete environment variable 'name'\n"				\
),
void do_setenv   (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);

#if ((CONFIG_COMMANDS & (CFG_CMD_ENV|CFG_CMD_FLASH)) == (CFG_CMD_ENV|CFG_CMD_FLASH))
#define	CMD_TBL_SAVEENV		MK_CMD_TBL_ENTRY(				\
	"saveenv",	4,	1,		1,	do_saveenv,		\
	"saveenv - save environment variables to persistent storage\n",		\
	NULL									\
),
void do_saveenv  (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);
#else
#define CMD_TBL_SAVEENV
#endif	/* CFG_CMD_ENV */

#endif	/* _CMD_NVEDIT_H */
