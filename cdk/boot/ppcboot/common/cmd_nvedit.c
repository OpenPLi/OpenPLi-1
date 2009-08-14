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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Support for persistent environment data
 */
#include <ppcboot.h>
#include <command.h>
#include <cmd_nvedit.h>
#ifdef CONFIG_4xx
#include <malloc.h>
#endif

/*
 * The environment storages is simply a list of '\0'-terminated
 * "name=value" strings, the end of the list marked by a double '\0'.
 * New entries are always addrd at the end. Deleting an entry shifts
 * the remaining entries to the front. Replacing an entry is a
 * combination of deleting the old and adding the new value.
 */

#ifdef CONFIG_NVRAM_ENV
uchar *environment = (uchar *)CFG_NVRAM_VAR_ADDR;
ulong env_size = CFG_NVRAM_ENV_SIZE;
#else

#if defined(CFG_FLASH_ENV_ADDR)
static uchar environment[CFG_FLASH_ENV_SIZE];
static ulong env_size = CFG_FLASH_ENV_SIZE;

/* need both ENV and flash */
#if ((CONFIG_COMMANDS & (CFG_CMD_ENV|CFG_CMD_FLASH)) == (CFG_CMD_ENV|CFG_CMD_FLASH))
static uchar *flash_addr = (uchar *)CFG_FLASH_ENV_ADDR;
#endif	/* ENV, FLASH */

/*
 * the contents of the rom_addr pointer will not change when the monitor
 * is relocated into RAM and so will contain the old address of the
 * environment[] array in flash, but the environment[] array itself *will*
 * be relocated. Hence, we can tell whether the monitor has been relocated
 * by comparing the value of rom_addr with environment.
 */
static uchar *rom_addr = environment;

#if defined(CONFIG_COGENT) && !defined(CONFIG_CMA302)
#define ISVALID(p)	0	/* can't get motherboard flash working yet */
#else
#define ISVALID(p)	(crc32(0, (char *)(p), env_size - sizeof (ulong)) == \
			    *(ulong *)((char *)(p) + env_size - sizeof (ulong)))
#endif
#define MAKEVALID(p, s)	(*(ulong *)((char *)(p) + (s) - sizeof (ulong)) = \
			    crc32(0, (char *)(p), (s) - sizeof (ulong)))

#else /* CFG_FLASH_ENV_ADDR */
extern uchar environment[];
extern ulong env_size;

/* need both ENV and flash */
#if ((CONFIG_COMMANDS & (CFG_CMD_ENV|CFG_CMD_FLASH)) == (CFG_CMD_ENV|CFG_CMD_FLASH))
static uchar *flash_addr = environment;
#endif	/* ENV, FLASH */
#endif	/* CFG_FLASH_ENV_ADDR */
#endif	/* CONFIG_NVRAM_ENV */

static uchar *envmatch (uchar *, uchar *);
#if defined(CFG_FLASH_ENV_ADDR)
static uchar *env_init(void);
#else
static void env_init(void);
#endif	/* CFG_FLASH_ENV_ADDR */

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

static uchar default_environment[] = {
#ifdef	CONFIG_BOOTARGS
	"bootargs="	CONFIG_BOOTARGS			"\0"
#endif
#ifdef	CONFIG_BOOTCOMMAND
	"bootcmd="	CONFIG_BOOTCOMMAND		"\0"
#endif
#if (CONFIG_BOOTDELAY >= 0)
	"bootdelay="	MK_STR(CONFIG_BOOTDELAY)	"\0"
#endif
#if (CONFIG_BAUDRATE >= 0)
	"baudrate="	MK_STR(CONFIG_BAUDRATE)		"\0"
#endif
#ifdef	CONFIG_ETHADDR
	"ethaddr="	MK_STR(CONFIG_ETHADDR)		"\0"
#endif
#ifdef	CONFIG_IPADDR
	"ipaddr="	MK_STR(CONFIG_IPADDR)		"\0"
#endif
#ifdef	CONFIG_SERVERIP
	"serverip="	MK_STR(CONFIG_SERVERIP)		"\0"
#endif
	"\0"
};

char *getenv (uchar *name)
{
	uchar *env, *nxt;

#if defined(CFG_FLASH_ENV_ADDR)
	uchar *environment = env_init();
#else
	env_init();
#endif	/* CFG_FLASH_ENV_ADDR */

	for (env=environment; *env; env=nxt+1) {
		char *val;

		for (nxt=env; *nxt; ++nxt)
			;
		val=envmatch(name, env);
		if (!val)
			continue;
		return (val);
	}

	return (NULL);
}

void do_printenv (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	uchar *env, *nxt;
	int i;

#if defined(CFG_FLASH_ENV_ADDR)
	uchar *environment = env_init();
#else
	env_init();
#endif	/* CFG_FLASH_ENV_ADDR */

	if (argc == 1) {		/* Print all env variables	*/
		uchar *start = environment;
		for (env=environment; *env; env=nxt+1) {
			for (nxt=env; *nxt; ++nxt)
				;
			puts (env);
			putc  ('\n');

			if (tstc()) {
				getc ();
				printf ("\n ** Abort\n");
				return;
			}
		}

		printf("\nEnvironment size: %d bytes\n", env-start);

		return;
	}

	for (i=1; i<argc; ++i) {	/* print single env variables	*/
		char *name = argv[i];
		char *val = NULL;

		for (env=environment; *env; env=nxt+1) {

			for (nxt=env; *nxt; ++nxt)
				;
			val=envmatch(name, env);
			if (val) {
				puts (name);
				putc ('=');
				puts (val);
				putc ('\n');
				break;
			}
		}
		if (!val)
			printf ("## Error: \"%s\" not defined\n", name);
	}
}

void _do_setenv (bd_t *bd, int flag, int argc, char *argv[])
{
	int   i, len;
	int   console = -1;
	uchar *env, *nxt;
	uchar *oldval = NULL;
	uchar *name;

#if defined(CFG_FLASH_ENV_ADDR)
	uchar *environment = env_init();
	ulong real_env_size = env_size;
	ulong env_size = real_env_size - sizeof (ulong);
#else
	env_init();
#endif	/* CFG_FLASH_ENV_ADDR */

	name = argv[1];

	/*
	 * search if variable with this name already exists
	 */
	for (env=environment; *env; env=nxt+1) {
		for (nxt=env; *nxt; ++nxt)
			;
		if ((oldval=envmatch(name, env)) != NULL)
			break;
	}

	/*
	 * Delete any existing definition
	 */
	if (oldval) {
#ifndef CONFIG_ENV_OVERWRITE
		/*
		 * Ethernet Address and serial# can be set only once
		 */
		if ((strcmp (name, "ethaddr") == 0) ||
		    (strcmp (name, "serial#") == 0) ) {
			printf ("Can't overwrite \"%s\"\n", name);
			return;
		}
#endif

		/* Check for console redirection */
		if (strcmp(name,"stdin") == 0) {
			console = stdin;
		} else if (strcmp(name,"stdout") == 0) {
			console = stdout;
		} else if (strcmp(name,"stderr") == 0) {
			console = stderr;
		}

		if (console != -1) {
			if (argc < 3)		/* Cannot delete it! */
				return;

			/* Try assigning specified device */
			if (console_assign (console, argv[2]) < 0)
				return;
		}

		if (*++nxt == '\0') {
			*env = '\0';
		} else {
			for (;;) {
				*env = *nxt++;
				if ((*env == '\0') && (*nxt == '\0'))
					break;
				++env;
			}
		}
		*++env = '\0';
	}

#if defined(CFG_FLASH_ENV_ADDR)
	MAKEVALID(environment, real_env_size);
#endif	/* CFG_FLASH_ENV_ADDR */

	/* Delete only ? */
	if (argc < 3)
		return;

	/*
	 * Append new definition at the end
	 */
	for (env=environment; *env || *(env+1); ++env)
		;
	if (env > environment)
		++env;
	/*
	 * Overflow when:
	 * "name" + "=" + "val" +"\0\0"  > env_size - (env-environment)
	 */
	len = strlen(name) + 2;
	/* add '=' for first arg, ' ' for all others */
	for (i=2; i<argc; ++i) {
		len += strlen(argv[i]) + 1;
	}
	if (len > (&environment[env_size]-env)) {
		printf ("## Error: environment overflow, \"%s\" deleted\n", name);
		return;
	}
	while ((*env = *name++) != '\0')
		env++;
	for (i=2; i<argc; ++i) {
		char *val = argv[i];

		*env = (i==2) ? '=' : ' ';
		while ((*++env = *val++) != '\0')
			;
	}

	/* end is marked with double '\0' */
	*++env = '\0';

#ifdef CONFIG_NVRAM_ENV
	*(ulong *)CFG_NVRAM_VAR_CRC_ADDR = crc32(0, (char *)environment, env_size);
#endif
#if defined(CFG_FLASH_ENV_ADDR)
	MAKEVALID(environment, real_env_size);
#endif	/* CFG_FLASH_ENV_ADDR */

        /* Changes of the Ethernet or IP address should be reflected
         * in the board info structure.
	 */

	if (strcmp(argv[1],"ethaddr") == 0) {
		char *s = argv[2];	/* always use only one arg */
		char *e;
		for (i=0; i<6; ++i) {
			bd->bi_enetaddr[i] = s ? simple_strtoul(s, &e, 16) : 0;
			if (s) s = (*e) ? e+1 : e;
		}
		return;
	}

	if (strcmp(argv[1],"ipaddr") == 0) {
		char *s = argv[2];	/* always use only one arg */
		char *e;
		bd->bi_ip_addr = 0;
		for (i=0; i<4; ++i) {
			ulong val = s ? simple_strtoul(s, &e, 10) : 0;
			bd->bi_ip_addr <<= 8;
			bd->bi_ip_addr  |= (val & 0xFF);
			if (s) s = (*e) ? e+1 : e;
		}
	}
}

void setenv (char *varname, char *varvalue)
{
    char *argv[4] = { "setenv", varname, varvalue, NULL };
    _do_setenv (bd_ptr, 0, 3, argv);
}

void do_setenv   (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
    if (argc < 2) {
	printf ("Usage:\n%s\n", cmdtp->usage);
	return;
    }

    _do_setenv (bd, flag, argc, argv);
}

/* need both ENV and flash */
#if ((CONFIG_COMMANDS & (CFG_CMD_ENV|CFG_CMD_FLASH)) == (CFG_CMD_ENV|CFG_CMD_FLASH))

void do_saveenv  (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	int rc;
	extern void flash_sect_protect (int p, ulong addr_first, ulong addr_last);
	extern void flash_sect_erase (ulong addr_first, ulong addr_last);
#if defined(CFG_FLASH_ENV_BUF)
	uchar *sector_buffer;
#endif	/* CFG_FLASH_ENV_BUF */

#if defined(CFG_FLASH_ENV_ADDR)
	 uchar *environment = env_init();
#endif	/* CFG_FLASH_ENV_BUF */

#ifdef CONFIG_4xx
	uchar *ed_buf;

	/*
	 * On ppc4xx still saved somewhere within a flash sector (no sector
	 * reserved for the environment variables). This will be changed in a
	 * future release.
	 */
        ulong sector_flash_addr;
        ulong sector_flash_size;
        ulong sector_flash_offs;
        int i;
        flash_info_t *info;

# ifndef CFG_FLASH_ENV_ADDR
	env_init();
# endif

        /*
         * Calculate environment variables sector address and size
         */
        info = addr2info((ulong)flash_addr);
        for (i=0; i<info->sector_count; i++)
          {
            if (info->start[i] >= (ulong)flash_addr)
              break;
          }
        sector_flash_addr = info->start[i-1];
        sector_flash_size = info->start[i] - info->start[i-1];
        sector_flash_offs = (ulong)flash_addr - info->start[i-1];

	/*
	 * Allocate temp buffer to edit environment
	 */
	if ((ed_buf = malloc(sector_flash_size)) == NULL) {
		printf ("## malloc(%lu) failed\n", sector_flash_size);
		return;
	}

        /*
         * Copy sector down to ram
         */
	memcpy(ed_buf, (uchar *)sector_flash_addr, sector_flash_size);

        /*
         * Copy new environment variables to ram image of flash sector
         */
	memcpy(ed_buf+sector_flash_offs, (uchar *)environment, env_size);

	flash_sect_protect (0, sector_flash_addr, sector_flash_addr+sector_flash_size-1);

	printf ("Erasing Flash...");
	flash_sect_erase (sector_flash_addr, sector_flash_addr+sector_flash_size-1);

	printf ("Saving Environment to Flash...\n");
	switch (rc = flash_write (ed_buf, sector_flash_addr, sector_flash_size)) {
	case 0: break;
	case 1: printf ("Timeout writing to Flash\n");
		break;
	case 2: printf ("Flash not Erased\n");
		break;
	case 4: printf ("Can't write to protected Flash sectors\n");
		break;
	default:
		printf ("%s[%d] FIXME: rc=%d\n",__FILE__,__LINE__,rc);
	}

	free (ed_buf);

	flash_sect_protect (1, sector_flash_addr, sector_flash_addr+sector_flash_size-1);
#else	/* ! CONFIG_4xx */

# ifndef CFG_FLASH_ENV_ADDR
	env_init();
# endif

# if defined(CFG_FLASH_ENV_BUF)
	/* this buffer area was reserved in board_init_f() */
	sector_buffer = (uchar *)((ulong)bd - CFG_FLASH_ENV_BUF);
	/* copy the environment into the sector buffer */
	memcpy(sector_buffer, environment, env_size);
	/* override the old names */
# define environment sector_buffer
# define env_size CFG_FLASH_ENV_BUF
# endif	/* CFG_FLASH_ENV_BUF */

	flash_sect_protect (0, (ulong)flash_addr, (ulong)flash_addr+env_size-1);

	printf ("Erasing Flash...");
	flash_sect_erase ((ulong)flash_addr, (ulong)flash_addr+env_size-1);

	printf ("Saving Environment to Flash...\n");
	switch (rc = flash_write (environment, (ulong)flash_addr, env_size)) {
	case 0: break;
	case 1: printf ("Timeout writing to Flash\n");
		break;
	case 2: printf ("Flash not Erased\n");
		break;
	case 4: printf ("Can't write to protected Flash sectors\n");
		break;
	case 8: printf ("Outside available Flash\n");
		return;
	default:
		printf ("%s[%d] FIXME: rc=%d\n",__FILE__,__LINE__,rc);
	}

	flash_sect_protect (1, (ulong)flash_addr, (ulong)flash_addr+env_size-1);

# if defined(CFG_FLASH_ENV_BUF)
# undef environment
# undef env_size
# endif	/* CFG_FLASH_ENV_BUF */

#endif	/* CONFIG_4xx */
}

#endif	/* CFG_CMD_ENV + CFG_CMD_FLASH */

/*
 * s1 is either a simple 'name', or a 'name=value' pair.
 * s2 is a 'name=value' pair.
 * If the names match, return the value of s2, else NULL.
 */

static uchar *
envmatch (uchar *s1, uchar *s2)
{

	while (*s1 == *s2++)
		if (*s1++ == '=')
			return(s2);
	if (*s1 == '\0' && *(s2-1) == '=')
		return(s2);
	return(NULL);
}

/*
 * Prevent confusion if running from erased flash memory
 */
#if defined(CFG_FLASH_ENV_ADDR)
static uchar *env_init(void)
#else
static void env_init(void)
#endif	/* CFG_FLASH_ENV_ADDR */
{
#ifdef CONFIG_NVRAM_ENV
	if (crc32(0, (char *)environment, env_size) !=
	    *(ulong *)CFG_NVRAM_VAR_CRC_ADDR) {
		int i;
#if 0 /* still some problems with this printf - don't know why */
		printf ("*** Warning - Environment CRC mismatch, using defaults\n\n");
#endif
		for (i=0; i<env_size; i++)
			environment[i] = 0x00;
		memcpy (environment,
			default_environment,
			sizeof(default_environment));
		*(ulong *)CFG_NVRAM_VAR_CRC_ADDR =
			crc32(0, (char *)environment, env_size);
	}

#else	/* ! CONFIG_NVRAM_ENV */

#if defined(CFG_FLASH_ENV_ADDR)
	/*
	 * check if monitor has been relocated yet
	 */
	if (environment == rom_addr) {
		/*
		 * the monitor is still running from ROM. if the flash copy
		 * is valid, use that, otherwise use the default environment
		 *
		 * NOTE: can't printf here, because getenv() is called from
		 * serial_init() (and maybe before that) - hence the serial
		 * port is not ready - we can yell later that we are using
		 * the default environment... see below.
		 */
		if (ISVALID(flash_addr))
			return (flash_addr);
		else
			return (default_environment);
	}

	/*
	 * the monitor has been relocated - environment will now
	 * refer to the "in-memory" copy. If it isn't valid, copy it
	 * from the flash, or the default environment, if that isn't
	 * valid either.
	 */
	if (!ISVALID(environment)) {
		/*
		 * "in-memory" copy must be initialised
		 */
		if (!ISVALID(flash_addr)) {
			/* flash isn't valid either - use default */
			printf ("\n*** Warning - no Environment,"
				" using defaults\n\n");
			memcpy(environment, default_environment,
				sizeof (default_environment));
			MAKEVALID(environment, env_size);
		}
		else
			/* copy flash environment into RAM */
			memcpy(environment, flash_addr, env_size);
	}

	return (environment);

#else	/* !CFG_FLASH_ENV_ADDR */

	if (environment[0] == 0xFF) {
		printf ("*** Warning - no Environment, using defaults\n\n");
		memcpy (environment,
			default_environment,
			sizeof(default_environment));
	}

#endif	/* CFG_FLASH_ENV_ADDR */

#endif	/* CONFIG_NVRAM_ENV */
}
