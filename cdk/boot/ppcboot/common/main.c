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
#include <command.h>

int  readline (const char *const prompt);
void run_default_command (int l, cmd_tbl_t *cmdtp, bd_t *bd, int flag);

static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen);
static int parse_line (char *, char *[]);
static void run_command (int len, cmd_tbl_t *cmdtp, bd_t *bd, int flag);

char        console_buffer[CFG_CBSIZE];		/* console I/O buffer	*/

static char erase_seq[] = "\b \b";		/* erase sequence	*/
static char   tab_seq[] = "        ";		/* used to expand TABs	*/

/****************************************************************************/

void main_loop(bd_t *bd)
{
	cmd_tbl_t *cmdtp;
#if (CONFIG_BOOTDELAY >= 0)
	char *s = getenv ("bootdelay");
	int bootdelay = s ? (int)simple_strtoul(s, NULL, 10) : 0;
	int autoboot  = 1;

#endif	/* CONFIG_BOOTDELAY */

#ifdef CONFIG_BOOTIMGSELECT
        char imageselect;
#endif

#if 0
	printf ("### main_loop entered:\n\n");
#endif

	/*
	 * Main Loop for Monitor Command Processing
	 */

	for (;;) {
		int flag = 0;
		int len;

#if (CONFIG_BOOTDELAY >= 0)

		if (autoboot)
		{ 
#ifdef CONFIG_BOOTIMGSELECT
			printf("Images:\n");
			printf("1: " CONFIG_BOOTIMG1 "\n");
			printf("2: " CONFIG_BOOTIMG2 "\n");
			printf("3: " CONFIG_BOOTIMG3 "\n");
			printf("4: " CONFIG_BOOTIMG4 "\n");
			printf("5: " CONFIG_BOOTIMG5 "\n");
			imageselect = '1';
			printf("Select image (1-5), other keys to stop autoboot: %2d ", bootdelay);
#else
			printf ("Hit any key to stop autoboot: %2d ", bootdelay);
#endif
		}

		while (bootdelay > 0) {
			int i;

			--bootdelay;
			/* delay 100 * 10ms */
			for (i=0; i<100; ++i) {
				if (tstc()) {	/* we got a key press	*/
					bootdelay = 0;	/* no more delays	*/
#ifdef CONFIG_BOOTIMGSELECT
					imageselect = getc();
					if (imageselect<'1' || imageselect>'5')
#endif
					autoboot  = 0;	/* don't auto boot	*/
#ifndef CONFIG_BOOTIMGSELECT
					(void) getc();  /* consume input */
#endif
					break;
				}
				udelay (10000);
			}

			printf ("\b\b\b%2d ", bootdelay);
		}

		putc ('\n');

#ifdef CONFIG_BOOTIMGSELECT
                /* save image */
                if (imageselect == '1') setenv("img", CONFIG_BOOTIMG1);
                if (imageselect == '2') setenv("img", CONFIG_BOOTIMG2);
                if (imageselect == '3') setenv("img", CONFIG_BOOTIMG3);
                if (imageselect == '4') setenv("img", CONFIG_BOOTIMG4);
                if (imageselect == '5') setenv("img", CONFIG_BOOTIMG5);
#endif

		if (autoboot) {
			autoboot = 0;

			run_default_command (1, cmdtp, bd, flag);

			continue;
		}
		else			/* No autoboot: read input		*/
#endif	/* CONFIG_BOOTDELAY */
		if ((len = readline (CFG_PROMPT)) < 0) {
			printf ("<INTERRUPT>\n");
			continue;
		}
		run_command (len, cmdtp, bd, flag);
	}
}

/****************************************************************************/

/*
 * Prompt for input and read a line.
 * Return number of read characters
 */
int readline (const char *const prompt)
{
	char   *p = console_buffer;
	int	n = 0;				/* buffer index		*/
	int	plen = strlen (prompt);		/* prompt length	*/
	int	col;				/* output column cnt	*/
	char	c;

	/* print prompt */
	if (prompt)
		puts (prompt);
	col = plen;

	for (;;) {
		c = getc();

		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':				/* Enter		*/
		case '\n':
			*p = '\0';
			puts ("\r\n");
			return (p - console_buffer);

		case 0x03:				/* ^C - break		*/
			return (-1);

		case 0x15:				/* ^U - erase line	*/
			while (col > plen) {
				puts (erase_seq);
				--col;
			}
			p = console_buffer;
			n = 0;
			continue;

		case 0x17:				/* ^W - erase word 	*/
			p=delete_char(console_buffer, p, &col, &n, plen);
			while ((n > 0) && (*p != ' ')) {
				p=delete_char(console_buffer, p, &col, &n, plen);
			}
			continue;

		case 0x08:				/* ^H  - backspace	*/
		case 0x7F:				/* DEL - backspace	*/
			p=delete_char(console_buffer, p, &col, &n, plen);
			continue;

		default:
			/*
			 * Must be a normal character then
			 */
			if (n < CFG_CBSIZE-2) {
				if (c == '\t') {	/* expand TABs		*/
					puts (tab_seq+(col&07));
					col += 8 - (col&07);
				} else {
					++col;		/* echo input		*/
					putc (c);
				}
				*p++ = c;
				++n;
			} else {			/* Buffer full		*/
				putc ('\a');
			}
		}
	}
}

/****************************************************************************/

static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
	char *s;

	if (*np == 0) {
		return (p);
	}

	if (*(--p) == '\t') {			/* will retype the whole line	*/
		while (*colp > plen) {
			puts (erase_seq);
			(*colp)--;
		}
		for (s=buffer; s<p; ++s) {
			if (*s == '\t') {
				puts (tab_seq+((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} else {
				++(*colp);
				putc (*s);
			}
		}
	} else {
		puts (erase_seq);
		(*colp)--;
	}
	(*np)--;
	return (p);
}

/****************************************************************************/

int parse_line (char *line, char *argv[])
{
	int nargs = 0;

	while (nargs < CFG_MAXARGS) {

		/* skip any white space */
		while ((*line == ' ') || (*line == '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			return (nargs);
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		while (*line && (*line != ' ') && (*line != '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			return (nargs);
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf ("** Too many args (max. %d) **\n", CFG_MAXARGS);

	return (nargs);
}

/****************************************************************************/

static void process_macros (char *input, char *output)
{
	char c, prev, *varname_start;
	int inputcnt  = strlen (input);
	int outputcnt = CFG_CBSIZE;
	int state = 0;	/* 0 = waiting for '$'	*/
			/* 1 = waiting for '('	*/
			/* 2 = waiting for ')'	*/

#ifdef DEBUG_PARSER
	char *output_start = output;

	printf ("[PROCESS_MACROS] INPUT=%s\n", input);
#endif

	prev = '\0';			/* previous character	*/

	while (inputcnt && outputcnt) {
	    c = *input++;
	    inputcnt--;

	    /* remove one level of escape characters */
	    if ((c == '\\') && (prev != '\\')) {
		if (inputcnt-- == 0)
			break;
		prev = c;
	    	c = *input++;
	    }

	    switch (state) {
	    case 0:			/* Waiting for (unescaped) $	*/
		if ((c == '$') && (prev != '\\')) {
			state++;
		} else {
			*(output++) = c;
			outputcnt--;
		}
		break;
	    case 1:			/* Waiting for (	*/
		if (c == '(') {
			state++;
			varname_start = input;
		} else {
			state = 0;
			*(output++) = '$';
			outputcnt--;

			if (outputcnt) {
				*(output++) = c;
				outputcnt--;
			}
		}
		break;
	    case 2:			/* Waiting for )	*/
		if (c == ')') {
			int i;
			char envname[CFG_CBSIZE], *envval;
			int envcnt = input-varname_start-1; /* Varname # of chars */

			/* Get the varname */
			for (i = 0; i < envcnt; i++) {
				envname[i] = varname_start[i];
			}
			envname[i] = 0;

			/* Get its value */
			envval = getenv (envname);

			/* Copy into the line if it exists */
			if (envval != NULL)
				while ((*envval) && outputcnt) {
					*(output++) = *(envval++);
					outputcnt--;
				}
			/* Look for another '$' */
			state = 0;
		}
		break;
	    }

	    prev = c;
	}

	if (outputcnt)
		*output = 0;

#ifdef DEBUG_PARSER
	printf ("[PROCESS_MACROS] OUTPUT=%s\n", output_start);
#endif
}

/****************************************************************************/


static int process_separators (char *s, int len,
			       cmd_tbl_t *cmdtp, bd_t *bd, int flag)
{
	extern void do_bootd (cmd_tbl_t *, bd_t *, int, int, char *[]);

	char token[CFG_CBSIZE];
	char finaltoken[CFG_CBSIZE];
	char *str = s;
	char *argv[CFG_MAXARGS + 1];	/* NULL terminated	*/
	int argc;
	int found;
	int repeatable = 1;

#ifdef DEBUG_PARSER
	printf ("[PROCESS_SEPARATORS] %s\n", s);
#endif
	while (*s) {
		char *sep;
		char *t;

		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		for (sep = str; *sep; sep++) {
			if ((*sep == ';') &&	/* separator		*/
			    ( sep != str) &&	/* past string start	*/
			    (*(sep-1) != '\\'))	/* and NOT escaped	*/
				break;
		}

		/*
		 * Extract the token between separators
		 */
		for (t = token; str < sep;) {
			*t++ = *str++;
			if (t >= token + CFG_CBSIZE - 1)
				break;			/* just in case */
		}
		*t = '\0';
#ifdef DEBUG_PARSER
		printf ("token: \"%s\"\n", token);
#endif

		/* Process macros into this token and replace them */
		process_macros (token, finaltoken);

		/* Extract arguments */
		argc = parse_line (finaltoken, argv);

		/* Search command table - Use linear search - it's a small table */
		found = 0;
		for (cmdtp = &cmd_tbl[0]; (!found) && cmdtp->name; cmdtp++) {
			if (strncmp (argv[0], cmdtp->name, cmdtp->lmin) == 0) {
				/* found - check max args */
				if (argc > cmdtp->maxargs) {
					printf ("Usage:\n%s\n", cmdtp->usage);
					return 0;
				}

#if (CONFIG_COMMANDS & CFG_CMD_BOOTD)
				/* avoid "bootd" recursion */
				if ((len < 0) && (cmdtp->cmd == do_bootd)) {
#ifdef DEBUG_PARSER
					printf ("[%s]\n", finaltoken);
#endif
					printf ("'bootd' recursion detected\n");
					return 0;
				}
#endif	/* CFG_CMD_BOOTD */
				/* OK - call function */
				(cmdtp->cmd) (cmdtp, bd, flag, argc, argv);

				repeatable &= cmdtp->repeatable;
				found = 1;
			}
		}
		if (!found)
			printf ("Unknown command '%s' - try 'help'\n", argv[0]);

		/* Did the user stop this? */
		if (tstc ())
			return 0;

		if (*sep == '\0')
			break;

		str = sep + 1;
	}

	return repeatable;
}

/****************************************************************************/

/*
 * "len" is used to indicate if we're executing normal input
 * (len > 0), just re-executing the last command when the user
 * presses only ENTER (len == 0), or executing the default command
 * (len < 0).
 */

static void run_command (int len,
		  cmd_tbl_t *cmdtp, bd_t *bd, int flag)
{
	static char lastcommand[CFG_CBSIZE] = { 0, };
	static int  lastlen = 0;

	if (len > 0) {			/* Process a new command		*/
		memcpy (lastcommand, console_buffer, len);
		lastcommand[len] = 0;
		lastlen = len;
	} else if (len < 0) {		/* Process default command		*/
		char *str = getenv ("bootcmd");

		if (!str)
			return;

		strcpy (lastcommand, str);
		lastlen = len;
	} else {			/* Process last command (len = 0)	*/
		/* Check if we have a valid command stored */
		if (lastcommand[0] == 0)
			return;

		flag |= CMD_FLAG_REPEAT;
	}

#ifdef DEBUG_PARSER
printf ("[RUN_COMMAND] lastlen=%d -> %s\n", lastlen, lastcommand);
#endif
	/* Process separators and check for non-valid repeatable commands	*/
	if (process_separators (lastcommand, lastlen, cmdtp, bd, flag) == 0) {
		lastcommand[0] = 0;
		lastlen = 0;
	}
}

/****************************************************************************/

void run_default_command (int l, cmd_tbl_t *cmdtp, bd_t *bd, int flag)
{
	run_command(-1, cmdtp, bd, flag) ;
}

/****************************************************************************/
