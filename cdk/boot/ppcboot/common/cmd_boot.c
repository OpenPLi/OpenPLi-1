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
#include <ppcboot.h>
#include <command.h>
#include <cmd_boot.h>
#include <s_record.h>


#if (CONFIG_COMMANDS & CFG_CMD_LOADS)
static ulong load_serial (ulong offset);
static int read_record (char *buf, ulong len);

static int do_echo = 1;
#endif


#if (CONFIG_COMMANDS & CFG_CMD_BDI)
void do_bdinfo (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	int i;
	ulong ip = bd->bi_ip_addr;
	char buf[32];

	printf ("  memstart    = 0x%08lx\n",  bd->bi_memstart   );
	printf ("  memsize     = 0x%08lx\n",  bd->bi_memsize    );
	printf ("  flashstart  = 0x%08lx\n",  bd->bi_flashstart );
	printf ("  flashsize   = 0x%08lx\n",  bd->bi_flashsize  );
	printf ("  flashoffset = 0x%08lx\n",  bd->bi_flashoffset);
	printf ("  sramstart   = 0x%08lx\n",  bd->bi_sramstart  );
	printf ("  sramsize    = 0x%08lx\n",  bd->bi_sramsize   );
#ifdef	CONFIG_8xx
	printf ("  immr_base   = 0x%08lx\n",  bd->bi_immr_base  );
#endif
	printf ("  bootflags   = 0x%08lx\n",  bd->bi_bootflags  );
#if defined(CFG_CLKS_IN_HZ)
	printf ("  intfreq     = %6s MHz\n",  strmhz(buf, bd->bi_intfreq));
	printf ("  busfreq     = %6s MHz\n",  strmhz(buf, bd->bi_busfreq));
#else
	printf ("  intfreq     = %6s MHz\n",  strmhz(buf, bd->bi_intfreq*1000000L));
	printf ("  busfreq     = %6s MHz\n",  strmhz(buf, bd->bi_busfreq*1000000L));
#endif	/* CFG_CLKS_IN_HZ */
	printf ("  ethaddr     =");
	for (i=0; i<6; ++i) {
		printf ("%c%02X", i ? ':' : ' ', bd->bi_enetaddr[i]);
	}
	printf ("\n  IP addr     =");
	for (i=0; i<4; ++i) {
		printf ("%c%ld", i ? '.' : ' ', (ip >> 24) & 0xFF);
		ip <<= 8;
	}
	printf ("\n  baudrate    = %6ld bps\n", bd->bi_baudrate   );
	printf ("  getc        = 0x%08lx\n",(ulong)bd->bi_mon_fnc->getc);
	printf ("  tstc        = 0x%08lx\n",(ulong)bd->bi_mon_fnc->tstc);
	printf ("  putc        = 0x%08lx\n",(ulong)bd->bi_mon_fnc->putc);
	printf ("  puts        = 0x%08lx\n",(ulong)bd->bi_mon_fnc->puts);
	printf ("  printf      = 0x%08lx\n",(ulong)bd->bi_mon_fnc->printf);
	printf ("  install_hdlr= 0x%08lx\n",(ulong)bd->bi_mon_fnc->install_hdlr);
	printf ("  free_hdlr   = 0x%08lx\n",(ulong)bd->bi_mon_fnc->free_hdlr);
	printf ("  malloc      = 0x%08lx\n",(ulong)bd->bi_mon_fnc->malloc);
	printf ("  free        = 0x%08lx\n",(ulong)bd->bi_mon_fnc->free);
}
#endif	/* CFG_CMD_BDI */

void do_go (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	ulong	addr, rc;

	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return;
	}

	addr = simple_strtoul(argv[1], NULL, 16);

	printf ("## Starting application at 0x%08lx ...\n", addr);

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	rc = ((ulong (*)(bd_t *, int, char *[]))addr) (bd, --argc, &argv[1]);

	printf ("## Application terminated, rc = 0x%lx\n", rc);
}

#if (CONFIG_COMMANDS & CFG_CMD_LOADS)
void do_load_serial (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	ulong offset = 0;
	ulong addr;
	int i;
	char *env_echo;
#ifdef	CFG_LOADS_BAUD_CHANGE
	int loads_baudrate = bd->bi_baudrate;
#endif

	if (((env_echo = getenv("loads_echo")) != NULL) && (*env_echo == '1')) {
		do_echo = 1;
	} else {
		do_echo = 0;
	}

#ifdef	CFG_LOADS_BAUD_CHANGE
	if (argc >= 2) {
		offset = simple_strtoul(argv[1], NULL, 16);
	}
	if (argc == 3) {
		loads_baudrate = (int)simple_strtoul(argv[2], NULL, 10);

		/* default to current baudrate */
		if (loads_baudrate == 0)
			loads_baudrate = bd->bi_baudrate;
	}
#else	/* ! CFG_LOADS_BAUD_CHANGE */
	if (argc == 2) {
		offset = simple_strtoul(argv[1], NULL, 16);
	}
#endif	/* CFG_LOADS_BAUD_CHANGE */

#ifdef	CFG_LOADS_BAUD_CHANGE
	if (loads_baudrate != bd->bi_baudrate) {
		printf ("## Switch baudrate to %d bps and press ENTER ...\n",
			loads_baudrate);
		udelay(50000);
		serial_setbrg (bd->bi_intfreq, loads_baudrate);
		udelay(50000);
		for (;;) {
			if (getc() == '\r')
				break;
		}
	}
#endif	/* CFG_LOADS_BAUD_CHANGE */
	printf ("## Ready for S-Record download ...\n");

	addr = load_serial (offset);

	/*
	 * Gather any trailing characters (for instance, the ^D which
	 * is sent by 'cu' after sending a file), and give the
	 * box some time (100 * 1 ms)
	 */
	for (i=0; i<100; ++i) {
		if (serial_tstc()) {
			(void) serial_getc();
		}
		udelay(1000);
	}

	if (addr == ~0) {
		printf ("## S-Record download aborted\n");
	} else {
		printf ("## Start Addr      = 0x%08lx\n", addr);
		load_addr = addr;
	}

#ifdef	CFG_LOADS_BAUD_CHANGE
	if (loads_baudrate != bd->bi_baudrate) {
		printf ("## Switch baudrate to %d bps and press ESC ...\n",
			(int)bd->bi_baudrate);
		udelay (50000);
		serial_setbrg (bd->bi_intfreq, bd->bi_baudrate);
		udelay (50000);
		for (;;) {
			if (getc() == 0x1B) /* ESC */
				break;
		}
	}
#endif
}

static ulong
load_serial (ulong offset)
{
	char	record[SREC_MAXRECLEN + 1];	/* buffer for one S-Record	*/
	char	binbuf[SREC_MAXBINLEN];		/* buffer for binary data	*/
	int	binlen;				/* no. of data bytes in S-Rec.	*/
	int	type;				/* return code for record type	*/
	ulong	addr;				/* load address from S-Record	*/
	ulong	store_addr;
	ulong	start_addr = ~0;
	ulong	end_addr   =  0;
	int	line_count =  0;

	while (read_record(record, SREC_MAXRECLEN + 1) >= 0) {
		type = srec_decode (record, &binlen, &addr, binbuf);

		if (type < 0) {
			return (~0);		/* Invalid S-Record		*/
		}

		switch (type) {
		case SREC_DATA2:
		case SREC_DATA3:
		case SREC_DATA4:
		    store_addr = addr + offset;
		    if (addr2info(store_addr)) {
			int rc; 

			switch (rc=flash_write((uchar *)binbuf,store_addr,binlen))
			{
			case 0:	break;
			case 1: printf ("\nError: Timeout writing to Flash\n");
				return (~0);
			case 2:	printf ("\nError: Flash not Erased\n");
				return (~0);
			case 4: printf ("\nError: Can't write to protected Flash sectors\n");
				return (~0);
			default:
				printf ("%s[%d] FIXME: rc=%d\n",
					__FILE__,__LINE__,rc);
				return (~0);
			}
		    } else {
			memcpy ((char *)(store_addr), binbuf, binlen);
		    }
		    if ((store_addr) < start_addr)
		    	start_addr = store_addr;
		    if ((store_addr+binlen-1) > end_addr)
		    	end_addr = store_addr+binlen-1;
		    break;
		case SREC_END2:
		case SREC_END3:
		case SREC_END4:
		    udelay (10000);
		    printf ("\n"
			    "## First Load Addr = 0x%08lx\n"
			    "## Last  Load Addr = 0x%08lx\n"
			    "## Total Size      = 0x%08lx = %ld Bytes\n",
			    start_addr, end_addr,
			    end_addr-start_addr+1,
			    end_addr-start_addr+1
		    );
		    return (addr);
		case SREC_START:
		    break;
		default:
		    break;
		}
		if (!do_echo) {	/* print a '.' every 100 lines */
			if ((++line_count % 100) == 0)
				putc ('.');
		}
	}

	return (~0);			/* Download aborted		*/
}

static int
read_record (char *buf, ulong len)
{
	char *p;
	char c;

	--len;	/* always leave room for terminating '\0' byte */

	for (p=buf; p < buf+len; ++p) {
		c = serial_getc();		/* read character		*/
		if (do_echo)
			serial_putc (c);	/* ... and echo it		*/

		switch (c) {
		case '\r':
		case '\n':
			*p = '\0';
			return (p - buf);
		case '\0':
		case 0x03:			/* ^C - Control C		*/
			return (-1);
		default:
			*p = c;
		}

	    // Check for the console hangup (if any different from serial) 

	    if (bd_ptr->bi_mon_fnc->getc != serial_getc)
	    {
		if (tstc())
		{
		    switch (getc()){
		    case '\0':
		    case 0x03:			/* ^C - Control C		*/
			return (-1);
		    }
		}
	    }
	}

	/* line too long - truncate */
	*p = '\0';
	return (p - buf);
}
#endif	/* CFG_CMD_LOADS */


#if (CONFIG_COMMANDS & CFG_CMD_LOADB)  /* loadb command (load binary) included */

#define XON_CHAR        17
#define XOFF_CHAR       19
#define START_CHAR      0x01
#define END_CHAR        0x0D
#define SPACE           0x20
#define K_ESCAPE        0x23
#define SEND_TYPE       'S'
#define DATA_TYPE       'D'
#define ACK_TYPE        'Y'
#define NACK_TYPE       'N'
#define BREAK_TYPE      'B'
#define tochar(x) ((char) (((x) + SPACE) & 0xff))
#define untochar(x) ((int) (((x) - SPACE) & 0xff))

extern int os_data_count;
extern int os_data_header[8];

void set_kerm_bin_mode(unsigned long *);
int k_recv(void);
int s1boot(unsigned long *, unsigned long *, int *);
static ulong load_serial_bin (ulong offset);


char his_eol;        /* character he needs at end of packet */
int  his_pad_count;  /* number of pad chars he needs */
char his_pad_char;   /* pad chars he needs */
char his_quote;      /* quote chars he'll use */


void do_load_serial_bin (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	ulong offset = 0;
	ulong addr;
	int i;
	int loadb_baudrate = bd->bi_baudrate;

	if (argc >= 2) {
		offset = simple_strtoul(argv[1], NULL, 16);
	}
	if (argc == 3) {
		loadb_baudrate = (int)simple_strtoul(argv[2], NULL, 10);

		/* default to current baudrate */
		if (loadb_baudrate == 0)
			loadb_baudrate = bd->bi_baudrate;
	}

	if (loadb_baudrate != bd->bi_baudrate) {
		printf ("## Switch baudrate to %d bps and press ENTER ...\n",
			loadb_baudrate);
		udelay(50000);
		serial_setbrg (bd->bi_intfreq, loadb_baudrate);
		udelay(50000);
		for (;;) {
			if (getc() == '\r')
				break;
		}
	}
	printf ("## Ready for binary (kermit) download ...\n");

	addr = load_serial_bin (offset);

	/*
	 * Gather any trailing characters (for instance, the ^D which
	 * is sent by 'cu' after sending a file), and give the
	 * box some time (100 * 1 ms)
	 */
	for (i=0; i<100; ++i) {
		if (serial_tstc()) {
			(void) serial_getc();
		}
		udelay(1000);
	}

	if (addr == ~0) {
		printf ("## Binary (kermit) download aborted\n");
	} else {
		printf ("## Start Addr      = 0x%08lx\n", addr);
		load_addr = addr;
	}

	if (loadb_baudrate != bd->bi_baudrate) {
		printf ("## Switch baudrate to %d bps and press ESC ...\n",
			(int)bd->bi_baudrate);
		udelay (50000);
		serial_setbrg (bd->bi_intfreq, bd->bi_baudrate);
		udelay (50000);
		for (;;) {
			if (getc() == 0x1B) /* ESC */
				break;
		}
	}
}


static ulong
load_serial_bin (ulong offset)
{
  set_kerm_bin_mode((ulong *)offset);
  k_recv();
  return offset;
}

void send_pad(void)
{
   int count = his_pad_count;
   while (count-- > 0) serial_putc(his_pad_char);
}

/* converts escaped kermit char to binary char */
char ktrans(char in)
{
   if ((in & 0x60) == 0x40)
   {
      return (char) (in & ~0x40);
   }
   else if ((in & 0x7f) == 0x3f)
   {
      return (char) (in | 0x40);
   }
   else return in;
}

int chk1(char *buffer)
{
   int total = 0;
   while (*buffer)
   {
      total += *buffer++;
   }
   return (int) ((total + ((total  >> 6) & 0x03)) & 0x3f);
}

void s1_sendpacket(char *packet)
{
   send_pad();
   while (*packet)
   {
      serial_putc(*packet++);
   }
}

static char a_b[24];
void send_ack(int n)
{
   a_b[0] = START_CHAR;
   a_b[1] = tochar(3);
   a_b[2] = tochar(n);
   a_b[3] = ACK_TYPE;
   a_b[4] = '\0';
   a_b[4] = tochar(chk1(&a_b[1]));
   a_b[5] = his_eol;
   a_b[6] = '\0';
   s1_sendpacket(a_b);
}

void send_nack(int n)
{
   a_b[0] = START_CHAR;
   a_b[1] = tochar(3);
   a_b[2] = tochar(n);
   a_b[3] = NACK_TYPE;
   a_b[4] = '\0';
   a_b[4] = tochar(chk1(&a_b[1]));
   a_b[5] = his_eol;
   a_b[6] = '\0';
   s1_sendpacket(a_b);
}



/* os_data_* takes an OS Open image and puts it into memory, and
   puts the boot header in an array named os_data_header

   if image is binary, no header is stored in os_data_header.
*/
void (*os_data_init)(void);
void (*os_data_char)(char new_char);
static int os_data_state, os_data_state_saved;
int os_data_count;
static int os_data_count_saved;
static char *os_data_addr, *os_data_addr_saved;
static char *bin_start_address;
int os_data_header[8];
void image_data_init(void)
{
   os_data_state = 0;
   os_data_count = 32;
   os_data_addr = (char *) os_data_header;
}
void bin_data_init(void)
{
   os_data_state = 0;
   os_data_count = 0;
   os_data_addr = bin_start_address;
}
void os_data_save(void)
{
   os_data_state_saved = os_data_state;
   os_data_count_saved = os_data_count;
   os_data_addr_saved = os_data_addr;
}
void os_data_restore(void)
{
   os_data_state = os_data_state_saved;
   os_data_count = os_data_count_saved;
   os_data_addr = os_data_addr_saved;
}
void bin_data_char(char new_char)
{
   switch (os_data_state)
   {
      case 0: /* data */
         *os_data_addr++ = new_char;
         --os_data_count;
         break;
   }
}
void set_kerm_bin_mode(unsigned long *addr)
{
   bin_start_address = (char *) addr;
   os_data_init = bin_data_init;
   os_data_char = bin_data_char;
}


/* k_data_* simply handles the kermit escape translations */
static int k_data_escape, k_data_escape_saved;
void k_data_init(void)
{
   k_data_escape = 0;
   os_data_init();
}
void k_data_save(void)
{
   k_data_escape_saved = k_data_escape;
   os_data_save();
}
void k_data_restore(void)
{
   k_data_escape = k_data_escape_saved;
   os_data_restore();
}
void k_data_char(char new_char)
{
   if (k_data_escape)
   {
      /* last char was escape - translate this character */
      os_data_char(ktrans(new_char));
      k_data_escape = 0;
   }
   else
   {
      if (new_char == his_quote)
      {
         /* this char is escape - remember */
         k_data_escape = 1;
      }
      else
      {
         /* otherwise send this char as-is */
         os_data_char(new_char);
      }
   }
}

#define SEND_DATA_SIZE  20
char send_parms[SEND_DATA_SIZE];
char *send_ptr;

/* handle_send_packet interprits the protocol info and builds and
   sends an appropriate ack for what we can do */
void handle_send_packet(int n)
{
   int length = 3;
   int bytes;

   /* initialize some protocol parameters */
   his_eol = END_CHAR; /* default end of line character */
   his_pad_count = 0;
   his_pad_char = '\0';
   his_quote = K_ESCAPE;

   /* ignore last character if it filled the buffer */
   if (send_ptr == &send_parms[SEND_DATA_SIZE - 1]) --send_ptr;
   bytes = send_ptr - send_parms; /* how many bytes we'll process */
   do
   {
      if (bytes-- <= 0) break;
      /* handle MAXL - max length */
      /* ignore what he says - most I'll take (here) is 94 */
      a_b[++length] = tochar(94);
      if (bytes-- <= 0) break;
      /* handle TIME - time you should wait for my packets */
      /* ignore what he says - don't wait for my ack longer than 1 second */
      a_b[++length] = tochar(1);
      if (bytes-- <= 0) break;
      /* handle NPAD - number of pad chars I need */
      /* remember what he says - I need none */
      his_pad_count = untochar(send_parms[2]);
      a_b[++length] = tochar(0);
      if (bytes-- <= 0) break;
      /* handle PADC - pad chars I need */
      /* remember what he says - I need none */
      his_pad_char = ktrans(send_parms[3]);
      a_b[++length] = 0x40; /* He should ignore this */
      if (bytes-- <= 0) break;
      /* handle EOL - end of line he needs */
      /* remember what he says - I need CR */
      his_eol = untochar(send_parms[4]);
      a_b[++length] = tochar(END_CHAR);
      if (bytes-- <= 0) break;
      /* handle QCTL - quote control char he'll use */
      /* remember what he says - I'll use '#' */
      his_quote = send_parms[5];
      a_b[++length] = '#';
      if (bytes-- <= 0) break;
      /* handle QBIN - 8-th bit prefixing */
      /* ignore what he says - I refuse */
      a_b[++length] = 'N';
      if (bytes-- <= 0) break;
      /* handle CHKT - the clock check type */
      /* ignore what he says - I do type 1 (for now) */
      a_b[++length] = '1';
      if (bytes-- <= 0) break;
      /* handle REPT - the repeat prefix */
      /* ignore what he says - I refuse (for now) */
      a_b[++length] = 'N';
      if (bytes-- <= 0) break;
      /* handle CAPAS - the capabilities mask */
      /* ignore what he says - I only do long packets - I don't do windows */
      a_b[++length] = tochar(2); /* only long packets */
      a_b[++length] = tochar(0); /* no windows */
      a_b[++length] = tochar(94); /* large packet msb */
      a_b[++length] = tochar(94); /* large packet lsb */
   } while (0);

   a_b[0] = START_CHAR;
   a_b[1] = tochar(length);
   a_b[2] = tochar(n);
   a_b[3] = ACK_TYPE;
   a_b[++length] = '\0';
   a_b[length] = tochar(chk1(&a_b[1]));
   a_b[++length] = his_eol;
   a_b[++length] = '\0';
   s1_sendpacket(a_b);
}

/* k_recv receives a OS Open image file over kermit line */
int k_recv(void)
{
   char new_char;
   char k_state, k_state_saved;
   int sum;
   int done;
   int length;
   int n, last_n;
   int z = 0;
   int len_lo, len_hi;

   /* initialize some protocol parameters */
   his_eol = END_CHAR; /* default end of line character */
   his_pad_count = 0;
   his_pad_char = '\0';
   his_quote = K_ESCAPE;

   /* initialize the k_recv and k_data state machine */
   done = 0;
   k_state = 0;
   k_data_init();
   k_state_saved = k_state;
   k_data_save();
   n = 0; /* just to get rid of a warning */
   last_n = -1;

   /* expect this "type" sequence (but don't check):
      S: send initiate
      F: file header
      D: data (multiple)
      Z: end of file
      B: break transmission
   */

   /* enter main loop */
   while (!done)
   {
      /* set the send packet pointer to begining of send packet parms */
      send_ptr = send_parms;

      /* With each packet, start summing the bytes starting with the length.
         Save the current sequence number.
         Note the type of the packet.
         If a character less than SPACE (0x20) is received - error.
      */

#if 0
      /* OLD CODE, Prior to checking sequence numbers */
      /* first have all state machines save current states */
      k_state_saved = k_state;
      k_data_save();
#endif

      /* get a packet */
      /* wait for the starting character */
      while (serial_getc() != START_CHAR);
      /* get length of packet */
      sum = 0;
      new_char = serial_getc();
      if ((new_char & 0xE0) == 0) goto packet_error;
      sum += new_char & 0xff;
      length = untochar(new_char);
      /* get sequence number */
      new_char = serial_getc();
      if ((new_char & 0xE0) == 0) goto packet_error;
      sum += new_char & 0xff;
      n = untochar(new_char);
      --length;

      /* NEW CODE - check sequence numbers for retried packets */
      /* Note - this new code assumes that the sequence number is correctly
         received.  Handling an invalid sequence number adds another layer
         of complexity that may not be needed - yet!  At this time, I'm hoping
         that I don't need to buffer the incoming data packets and can write
         the data into memory in real time.  */
      if (n == last_n)
      {
         /* same sequence number, restore the previous state */
         k_state = k_state_saved;
         k_data_restore();
      }
      else
      {
         /* new sequence number, checkpoint the download */
         last_n = n;
         k_state_saved = k_state;
         k_data_save();
      }
      /* END NEW CODE */

      /* get packet type */
      new_char = serial_getc();
      if ((new_char & 0xE0) == 0) goto packet_error;
      sum += new_char & 0xff;
      k_state = new_char;
      --length;
      /* check for extended length */
      if (length == -2)
      {
         /* (length byte was 0, decremented twice) */
         /* get the two length bytes */
         new_char = serial_getc();
         if ((new_char & 0xE0) == 0) goto packet_error;
         sum += new_char & 0xff;
         len_hi = untochar(new_char);
         new_char = serial_getc();
         if ((new_char & 0xE0) == 0) goto packet_error;
         sum += new_char & 0xff;
         len_lo = untochar(new_char);
         length = len_hi * 95 + len_lo;
         /* check header checksum */
         new_char = serial_getc();
         if ((new_char & 0xE0) == 0) goto packet_error;
         if (new_char != tochar((sum + ((sum  >> 6) & 0x03)) & 0x3f))
            goto packet_error;
         sum += new_char & 0xff;
         /* --length; *//* new length includes only data and block check to come */
      }
      /* bring in rest of packet */
      while (length > 1)
      {
         new_char = serial_getc();
         if ((new_char & 0xE0) == 0) goto packet_error;
         sum += new_char & 0xff;
         --length;
         if (k_state == DATA_TYPE)
         {
            /* pass on the data if this is a data packet */
            k_data_char(new_char);
         }
         else if (k_state == SEND_TYPE)
         {
            /* save send pack in buffer as is */
            *send_ptr++ = new_char;
            /* if too much data, back off the pointer */
            if (send_ptr >= &send_parms[SEND_DATA_SIZE]) --send_ptr;
         }
      }
      /* get and validate checksum character */
      new_char = serial_getc();
      if ((new_char & 0xE0) == 0) goto packet_error;
      if (new_char != tochar((sum + ((sum  >> 6) & 0x03)) & 0x3f))
         goto packet_error;
      /* get END_CHAR */
      new_char = serial_getc();
      if (new_char != END_CHAR)
      {
packet_error:
         /* restore state machines */
         k_state = k_state_saved;
         k_data_restore();
         /* send a negative acknowledge packet in */
         send_nack(n);
      }
      else if (k_state == SEND_TYPE)
      {
         /* crack the protocol parms, build an appropriate ack packet */
         handle_send_packet(n);
      }
      else
      {
         /* send simple acknowledge packet in */
         send_ack(n);
         /* quit if end of transmission */
         if (k_state == BREAK_TYPE) done = 1;
      }
      ++z;
   }
   return 0;
}

#endif	/* CFG_CMD_LOADB */
