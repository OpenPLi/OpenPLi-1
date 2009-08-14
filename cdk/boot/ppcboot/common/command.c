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
 *  Command Processor Table
 */

#include <ppcboot.h>
#include <command.h>
#include <cmd_cache.h>
#include <cmd_mem.h>
#include <cmd_boot.h>
#include <cmd_flash.h>
#include <cmd_bootm.h>
#include <cmd_net.h>
#include <cmd_nvedit.h>
#include <cmd_misc.h>
#include <cmd_kgdb.h>
#include <cmd_ide.h>
#include <cmd_disk.h>
#include <cmd_console.h>

/*
 * HELP command
 */
#define	CMD_TBL_HELP	MK_CMD_TBL_ENTRY(					\
	"help",		1,	CFG_MAXARGS,	1,	do_help,		\
	"help    - print online help\n",					\
	"[command ...]\n"							\
	"    - show help information (for 'command')\n"				\
	"'help' prints online help for the monitor commands.\n\n"		\
	"Without arguments, it prints a short usage message for all commands.\n\n" \
	"To get detailed help information for specific commands you can type\n"	\
	"'help' with one or more command names as arguments.\n"			\
    ),

#define	CMD_TBL_QUES	MK_CMD_TBL_ENTRY(					\
	"?",		1,	CFG_MAXARGS,	1,	do_help,		\
	"?       - alias for 'help'\n",						\
	NULL									\
    ),

#define CMD_TBL_VERS	MK_CMD_TBL_ENTRY(					\
	"version",	4,	1,		1,	do_version,		\
	"version - print monitor version\n",					\
	NULL									\
    ),

void
do_version (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	extern char version_string[];
	printf ("\n%s\n", version_string);
}

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */
void
do_help (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	int i;

	if (argc == 1) {	/* print short help (usage) */

		for (cmdtp=&cmd_tbl[0]; cmdtp->name; cmdtp++) {
			/* allow user abort */
			if (tstc())
				return;

			if (cmdtp->usage == NULL)
				continue;
			puts (cmdtp->usage);
		}

		return;
	}

	/*
	 * command help (long version)
	 */
	for (i=1; i<argc; ++i) {
		for (cmdtp=&cmd_tbl[0]; cmdtp->name; cmdtp++) {
			if (strncmp(argv[i], cmdtp->name, cmdtp->lmin) == 0) {
#ifdef	CFG_LONGHELP
				/* found - print (long) help info */
				puts (cmdtp->name);
				putc (' ');
				if (cmdtp->help) {
					puts (cmdtp->help);
				} else {
					puts ("- No help available.\n");
				}
				putc ('\n');
#else	/* no long help available */
				if (cmdtp->usage)
					puts (cmdtp->usage);
#endif	/* CFG_LONGHELP */
				goto done;
			}
		}
		printf ("Unknown command '%s' - try 'help'"
			" without arguments for list of all known commands\n\n",
			argv[i]
		);
done:		;
	}
}

cmd_tbl_t cmd_tbl[] = {
	CMD_TBL_GO
	CMD_TBL_BOOTM
	CMD_TBL_BOOTP
	CMD_TBL_BOOTIDXFS
	CMD_TBL_INFOIDXFS
	CMD_TBL_TFTPB
	CMD_TBL_RARPB
	CMD_TBL_DISK
	CMD_TBL_BOOTD
	CMD_TBL_LOADS
	CMD_TBL_LOADB
	CMD_TBL_MD
	CMD_TBL_MM
	CMD_TBL_NM
	CMD_TBL_MW
	CMD_TBL_CP
	CMD_TBL_CMP
	CMD_TBL_CRC
	CMD_TBL_BASE
	CMD_TBL_PRINTENV
	CMD_TBL_SETENV
	CMD_TBL_SAVEENV
	CMD_TBL_PROTECT
	CMD_TBL_FLERASE
	CMD_TBL_FLINFO
	CMD_TBL_BDINFO
	CMD_TBL_IMINFO
	CMD_TBL_PCIINFO
	CMD_TBL_IRQINFO
	CMD_TBL_CONINFO
	CMD_TBL_IDE
	CMD_TBL_LOOP
	CMD_TBL_MTEST
	CMD_TBL_ICACHE
	CMD_TBL_DCACHE
	CMD_TBL_RESET
	CMD_TBL_KGDB
	CMD_TBL_VERS
	CMD_TBL_HELP
	CMD_TBL_QUES
	/* the following entry terminates this table */
	MK_CMD_TBL_ENTRY( NULL, 0, 0, 0, NULL, NULL, NULL )
};
