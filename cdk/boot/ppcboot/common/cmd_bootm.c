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
#include <image.h>
#include <malloc.h>
#include <zlib.h>
#include <idxfs.h>

int  gunzip (void *, int, unsigned char *, int *);
void run_default_command (int len, cmd_tbl_t *cmdtp, bd_t *bd, int flag);

static void *zalloc(void *, unsigned, unsigned);
static void zfree(void *, void *, unsigned);

static void image_info (unsigned long addr);
static void print_type (image_header_t *hdr);

image_header_t header;

ulong load_addr = CFG_LOAD_ADDR;		/* Default Load Address */

//static void process_macros (char *input, char *output)
void process_macros (char *input, char *output, char delim)
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
		if ((c == delim) && (prev != '\\')) {
//		if ((c == '$') && (prev != '\\')) {
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
			*(output++) = delim;
//			*(output++) = '$';
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


void do_bootm (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	ulong	iflag;
	ulong	addr;
	ulong	sp;
	ulong	data, len, checksum;
	ulong	initrd_start, initrd_end;
	ulong	cmd_start, cmd_end;
	ulong  *len_ptr;
	ulong	initrd_high;
	int	i, verify;
	char    *cmdline;
	char	*name, *s;
	bd_t	*kbd;
	int	(*appl)(cmd_tbl_t *, bd_t *, int, int, char *[]);
	void	(*kernel)(bd_t *, ulong, ulong, ulong, ulong);
	image_header_t *hdr = &header;

	s = getenv ("verify");
	verify = (s && (*s == 'n')) ? 0 : 1;

	if ((s = getenv ("initrd_high")) != NULL) {
		/* a value of "no" or a similar string will act like 0,
		 * truning the "load high" feature off. This is intentional.
		 */
		initrd_high = simple_strtoul(s, NULL, 16);
	} else {			/* not set, no restrictions to load high */
		initrd_high = ~0;
	}

	if (argc < 2) {
		addr = load_addr;
	} else {
		addr = simple_strtoul(argv[1], NULL, 16);
	}

	printf ("## Booting Linux kernel at %08lx ...\n", addr);

	/* Copy header so we can blank CRC field for re-calculation */
	memcpy (&header, (char *)addr, sizeof(image_header_t));

	if (hdr->ih_magic  != IH_MAGIC) {
		printf ("Bad Magic Number\n");
		return;
	}

	data = (ulong)&header;
	len  = sizeof(image_header_t);

	checksum = hdr->ih_hcrc;
	hdr->ih_hcrc = 0;

	if (crc32 (0, (char *)data, len) != checksum) {
		printf ("Bad Header Checksum\n");
		return;
	}

	/* for multi-file images we need the data part, too */
	print_image_hdr ((image_header_t *)addr);

	data = addr + sizeof(image_header_t);
	len  = hdr->ih_size;

	if (verify) {
		printf ("   Verifying Checksum ... ");
		if (crc32 (0, (char *)data, len) != hdr->ih_dcrc) {
			printf ("Bad Data CRC\n");
			return;
		}
		printf ("OK\n");
	}

	len_ptr = (ulong *)data;

	if (hdr->ih_arch != IH_CPU_PPC) {
		printf ("Unsupported Architecture\n");
		return;
	}

	switch (hdr->ih_type) {
	case IH_TYPE_STANDALONE:	name = "Standalone Application";
					break;
	case IH_TYPE_KERNEL:		name = "Kernel Image";
					break;
	case IH_TYPE_MULTI:		name = "Multi-File Image";
					len  = len_ptr[0];
					/* OS kernel is always the first image */
					data += 8; /* kernel_len + terminator */
					for (i=1; len_ptr[i]; ++i)
						data += 4;
					break;
	default: printf ("Wrong Image Type for %s command\n", cmdtp->name);
		 return;
	}

	/*
	 * We have reached the point of no return: we are going to
	 * overwrite all exception vector code, so we cannot easily
	 * recover from any failures any more...
	 */

	iflag = disable_interrupts();

	switch (hdr->ih_comp) {
	case IH_COMP_NONE:
		printf ("   Loading %s ... ", name);
		memcpy ((void *)hdr->ih_load, (uchar *)data, len);
		break;
	case IH_COMP_GZIP:
		printf ("   Uncompressing %s ... ", name);
		if (gunzip ((void *)hdr->ih_load, 0x400000,
			    (uchar *)data, (int *)&len) != 0) {
			printf ("GUNZIP ERROR - must RESET board to recover\n");
			do_reset (cmdtp, bd, flag, argc, argv);
		}
		break;
	default:
		if (iflag)
			enable_interrupts();
		printf ("Unimplemented compression type %d\n", hdr->ih_comp);
		return;
	}
	printf ("OK\n");

	switch (hdr->ih_type) {
	case IH_TYPE_STANDALONE:
		appl = (int (*)(cmd_tbl_t *, bd_t *, int, int, char *[]))hdr->ih_ep;

		(*appl)(cmdtp, bd, flag, argc-1, &argv[1]);
		/* just in case we return */
		if (iflag)
			enable_interrupts();
		break;
	case IH_TYPE_KERNEL:
	case IH_TYPE_MULTI:
		/* handled below */
		break;
	default:
		if (iflag)
			enable_interrupts();
		printf ("Can't boot image type %d\n", hdr->ih_type);
		return;
	}

	/*
	 * Booting a (Linux) kernel image
	 *
	 * Allocate space for command line and board info - the
	 * address should be as high as possible within the reach of
	 * the kernel (see CFG_BOOTMAPSZ settings), but in unused
	 * memory, which means far enough below the current stack
	 * pointer.
	 */

	asm( "mr %0,1": "=r"(sp) : );

	sp -= 1024;		/* just to be sure */
	if (sp > CFG_BOOTMAPSZ)
		sp = CFG_BOOTMAPSZ;
	sp &= ~0xF;

	cmdline = (char *)((sp - CFG_BARGSIZE) & ~0xF);
	kbd = (bd_t *)(((ulong)cmdline - sizeof(bd_t)) & ~0xF);

	if ((s = getenv("bootargs")) == NULL)
		s = "";

        process_macros(s, cmdline, '$');
//	strcpy (cmdline, s);
	
	cmd_start    = (ulong)&cmdline[0];
	cmd_end      = cmd_start + strlen(cmdline);

	*kbd = *bd;

	kernel = (void (*)(bd_t *, ulong, ulong, ulong, ulong))hdr->ih_ep;

	/*
	 * Check if there is an initrd image
	 */
	if (argc >= 3) {
		addr = simple_strtoul(argv[2], NULL, 16);

		printf ("## Loading RAMDisk Image at %08lx ...\n", addr);

		/* Copy header so we can blank CRC field for re-calculation */
		memcpy (&header, (char *)addr, sizeof(image_header_t));

		if (hdr->ih_magic  != IH_MAGIC) {
			printf ("Bad Magic Number\n");
			do_reset (cmdtp, bd, flag, argc, argv);
		}

		data = (ulong)&header;
		len  = sizeof(image_header_t);

		checksum = hdr->ih_hcrc;
		hdr->ih_hcrc = 0;

		if (crc32 (0, (char *)data, len) != checksum) {
			printf ("Bad Header Checksum\n");
			do_reset (cmdtp, bd, flag, argc, argv);
		}

		print_image_hdr (hdr);

		data = addr + sizeof(image_header_t);
		len  = hdr->ih_size;

		if (verify) {
			ulong csum = 0;
#if defined(CONFIG_WATCHDOG)
			ulong cdata = data, edata = cdata + len;
#endif	/* CONFIG_WATCHDOG */

			printf ("   Verifying Checksum ... ");

#if defined(CONFIG_WATCHDOG) 
# define CHUNKSZ (64 * 1024)

			while (cdata < edata) {
				ulong chunk = edata - cdata;

				if (chunk > CHUNKSZ)
					chunk = CHUNKSZ;
				csum = crc32 (csum, (char *)cdata, chunk);
				cdata += chunk;
				watchdog_reset();
			}
#else	/* !CONFIG_WATCHDOG */
			csum = crc32 (0, (char *)data, len);
#endif	/* CONFIG_WATCHDOG */

			if (csum != hdr->ih_dcrc) {
				printf ("Bad Data CRC\n");
				do_reset (cmdtp, bd, flag, argc, argv);
			}
			printf ("OK\n");
		}

		if ((hdr->ih_os   != IH_OS_LINUX)	||
		    (hdr->ih_arch != IH_CPU_PPC)	||
		    (hdr->ih_type != IH_TYPE_RAMDISK)	) {
			printf ("No Linux PPC Ramdisk Image\n");
			do_reset (cmdtp, bd, flag, argc, argv);
		}

		initrd_start  = (ulong)kbd - hdr->ih_size;
		initrd_start &= ~(4096 - 1);	/* align on page */

		if (initrd_high) {
			ulong nsp;

			/*
			 * the inital ramdisk does not need to be within
			 * CFG_BOOTMAPSZ as it is not accessed until after
			 * the mm system is initialised.
			 *
			 * do the stack bottom calculation again and see if
			 * the initrd will fit just below the monitor stack
			 * bottom without overwriting the area allocated
			 * above for command line args and board info.
			 */
			asm( "mr %0,1": "=r"(nsp) : );
			nsp -= 1024;		/* just to be sure */
			nsp &= ~0xF;
			if (nsp > initrd_high)	/* limit as specified */
				nsp = initrd_high;
			nsp -= hdr->ih_size;
			nsp &= ~(4096 - 1);	/* align on page */
			if (nsp >= sp)
				initrd_start = nsp;
		}

		initrd_end    = initrd_start + hdr->ih_size;
		printf ("   Loading Ramdisk to %08lx, end %08lx ... ",
			initrd_start, initrd_end);
		memcpy ((void *)initrd_start,
			(void *)(addr + sizeof(image_header_t)),
			hdr->ih_size );
		printf ("OK\n");
	} else if ((hdr->ih_type==IH_TYPE_MULTI) && (len_ptr[1])) {
		u_long i_start = data + len_ptr[0];
		u_long tail    = len_ptr[0] % 4;

		if (tail) {
			i_start += 4 - tail;
		}

		initrd_start = (ulong)kbd - len_ptr[1];
		initrd_start &= ~(4096 - 1);	/* align on page */
		initrd_end    = initrd_start + len_ptr[1];
		printf ("   Loading Ramdisk to %08lx ... ", initrd_start);
		memcpy ((void *)initrd_start, (void *)i_start, len_ptr[1]);
		printf ("OK\n");
	} else {
		/*
		 * no initrd image
		 */
		initrd_start = 0;
		initrd_end   = 0;
	}

#ifdef DEBUG
	printf ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong)kernel);
#endif

	/*
	 * Linux Kernel Parameters:
	 *   r3: ptr to board info data
	 *   r4: initrd_start or 0 if no initrd
	 *   r5: initrd_end - unused if r4 is 0
	 *   r6: Start of command line string
	 *   r7: End   of command line string
	 */
	(*kernel) (kbd, initrd_start, initrd_end, cmd_start, cmd_end);

#ifdef DEBUG
	printf ("\n## Control returned to monitor - resetting...\n");
	do_reset (cmdtp, bd, flag, argc, argv);
#endif
}


#if (CONFIG_COMMANDS & CFG_CMD_BOOTIDXFS)
void do_bootidxfs (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	unsigned int size, offset = 0;
	char argv0[20];
	char argv1[20];
	char *newargv[2] = {argv0, argv1};

	idxfs_file_info((unsigned char*)IDXFS_OFFSET, 0, "kernel", &offset, &size);
	
	if (!offset) {
  	  printf("Kernel image at: none\n");
	} else {  
  	  sprintf(argv1, "%X", IDXFS_OFFSET + offset);
	  printf("Kernel image at: 0x%X (0x%X bytes)\n", IDXFS_OFFSET + offset, size);
          do_bootm (cmdtp, bd, flag, 2, newargv);
	}  
}
#endif
	
 
#if (CONFIG_COMMANDS & CFG_CMD_INFOIDXFS)
void do_infoidxfs (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	idxfs_dump_info((unsigned char*)IDXFS_OFFSET, 0);
}
#endif
	
 
#if (CONFIG_COMMANDS & CFG_CMD_BOOTD)
void do_bootd (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	run_default_command (-1, cmdtp, bd, flag);
}
#endif


#if (CONFIG_COMMANDS & CFG_CMD_IMI)
void do_iminfo (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	int	arg;
	ulong	addr;

	if (argc < 2) {
		image_info (load_addr);
		return;
	} 

	for (arg=1; arg <argc; ++arg) {
		addr = simple_strtoul(argv[arg], NULL, 16);
		image_info (addr);
	}
}

static void image_info (ulong addr)
{
	ulong	data, len, checksum;
	image_header_t *hdr = &header;

	printf ("\n## Checking Image at %08lx ...\n", addr);

	/* Copy header so we can blank CRC field for re-calculation */
	memcpy (&header, (char *)addr, sizeof(image_header_t));

	if (hdr->ih_magic  != IH_MAGIC) {
		printf ("   Bad Magic Number\n");
		return;
	}

	data = (ulong)&header;
	len  = sizeof(image_header_t);

	checksum = hdr->ih_hcrc;
	hdr->ih_hcrc = 0;

	if (crc32 (0, (char *)data, len) != checksum) {
		printf ("   Bad Header Checksum\n");
		return;
	}

	/* for multi-file images we need the data part, too */
	print_image_hdr ((image_header_t *)addr);

	data = addr + sizeof(image_header_t);
	len  = hdr->ih_size;

	printf ("   Verifying Checksum ... ");
	if (crc32 (0, (char *)data, len) != hdr->ih_dcrc) {
		printf ("   Bad Data CRC\n");
		return;
	}
	printf ("OK\n");
}
#endif	/* CFG_CMD_IMI */

void
print_image_hdr (image_header_t *hdr)
{
/*	time_t timestamp = (time_t)hdr->ih_time;	*/

	printf ("   Image Name:   %.*s\n", IH_NMLEN, hdr->ih_name);
/*	printf ("   Created:      %s", ctime(&timestamp)); */
	printf ("   Image Type:   "); print_type(hdr); printf ("\n");
	printf ("   Data Size:    %d Bytes = %d kB = %d MB\n",
		hdr->ih_size, hdr->ih_size>>10, hdr->ih_size>>20);
	printf ("   Load Address: %08x\n", hdr->ih_load);
	printf ("   Entry Point:  %08x\n", hdr->ih_ep);

	if (hdr->ih_type == IH_TYPE_MULTI) {
		int i;
		ulong *len_ptr = (ulong *)((ulong)hdr + sizeof(image_header_t));

		printf ("   Contents:\n");
		for (i=0; *len_ptr; ++i, ++len_ptr) {
			printf ("   Image %d: %8ld Bytes = %ld kB = %ld MB\n",
				i, *len_ptr, *len_ptr>>10, *len_ptr>>20);
		}
	}
}


static void
print_type (image_header_t *hdr)
{
	char *os, *arch, *type, *comp;

	switch (hdr->ih_os) {
	case IH_OS_INVALID:	os = "Invalid OS";		break;
	case IH_OS_LINUX:	os = "Linux";			break;
	default:		os = "Unknown OS";		break;
	}

	switch (hdr->ih_arch) {
	case IH_CPU_INVALID:	arch = "Invalid CPU";		break;
	case IH_CPU_ALPHA:	arch = "Alpha";			break;
	case IH_CPU_ARM:	arch = "ARM";			break;
	case IH_CPU_I386:	arch = "Intel x86";		break;
	case IH_CPU_IA64:	arch = "IA64";			break;
	case IH_CPU_MIPS:	arch = "MIPS";			break;
	case IH_CPU_MIPS64:	arch = "MIPS 64 Bit";		break;
	case IH_CPU_PPC:	arch = "PowerPC";		break;
	case IH_CPU_S390:	arch = "IBM S390";		break;
	case IH_CPU_SH:		arch = "SuperH";		break;
	case IH_CPU_SPARC:	arch = "SPARC";			break;
	case IH_CPU_SPARC64:	arch = "SPARC 64 Bit";		break;
	default:		arch = "Unknown Architecture";	break;
	}

	switch (hdr->ih_type) {
	case IH_TYPE_INVALID:	type = "Invalid Image";		break;
	case IH_TYPE_STANDALONE:type = "Standalone Program";	break;
	case IH_TYPE_KERNEL:	type = "Kernel Image";		break;
	case IH_TYPE_RAMDISK:	type = "RAMDisk Image";		break;
	case IH_TYPE_MULTI:	type = "Multi-File Image";	break;
	default:		type = "Unknown Image";		break;
	}

	switch (hdr->ih_comp) {
	case IH_COMP_NONE:	comp = "uncompressed";		break;
	case IH_COMP_GZIP:	comp = "gzip compressed";	break;
	case IH_COMP_BZIP2:	comp = "bzip2 compressed";	break;
	default:		comp = "unknown compression";	break;
	}

	printf ("%s %s %s (%s)", arch, os, type, comp);
}

#define	ZALLOC_ALIGNMENT	16

static void *zalloc(void *x, unsigned items, unsigned size)
{
	void *p;

	size *= items;
	size = (size + ZALLOC_ALIGNMENT - 1) & ~(ZALLOC_ALIGNMENT - 1);

	p = malloc (size);

	return (p);
}

static void zfree(void *x, void *addr, unsigned nb)
{
	free (addr);
}

#define HEAD_CRC	2
#define EXTRA_FIELD	4
#define ORIG_NAME	8
#define COMMENT		0x10
#define RESERVED	0xe0

#define DEFLATED	8

int gunzip(void *dst, int dstlen, unsigned char *src, int *lenp)
{
	z_stream s;
	int r, i, flags;

	/* skip header */
	i = 10;
	flags = src[3];
	if (src[2] != DEFLATED || (flags & RESERVED) != 0) {
		printf ("Error: Bad gzipped data\n");
		return (-1);
	}
	if ((flags & EXTRA_FIELD) != 0)
		i = 12 + src[10] + (src[11] << 8);
	if ((flags & ORIG_NAME) != 0)
		while (src[i++] != 0)
			;
	if ((flags & COMMENT) != 0)
		while (src[i++] != 0)
			;
	if ((flags & HEAD_CRC) != 0)
		i += 2;
	if (i >= *lenp) {
		printf ("Error: gunzip out of data in header\n");
		return (-1);
	}

	s.zalloc = zalloc;
	s.zfree = zfree;
#if defined(CONFIG_WATCHDOG)
	s.outcb = (cb_func)watchdog_reset;
#else
	s.outcb = Z_NULL;
#endif	/* CONFIG_WATCHDOG */

	r = inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		printf ("Error: inflateInit2() returned %d\n", r);
		return (-1);
	}
	s.next_in = src + i;
	s.avail_in = *lenp - i;
	s.next_out = dst;
	s.avail_out = dstlen;
	r = inflate(&s, Z_FINISH);
	if (r != Z_OK && r != Z_STREAM_END) {
		printf ("Error: inflate() returned %d\n", r);
		return (-1);
	}
	*lenp = s.next_out - (unsigned char *) dst;
	inflateEnd(&s);

	return (0);
}
