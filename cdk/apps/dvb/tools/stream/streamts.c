/*
 * $Id: streamts.c,v 1.21 2008/01/04 15:39:49 obi Exp $
 * 
 * inetd style daemon for streaming avpes, ps and ts
 * 
 * Copyright (C) 2002 Andreas Oberritter <obi@tuxbox.org>
 *
 * based on code which is
 * Copyright (C) 2001 TripleDES
 * Copyright (C) 2000, 2001 Marcus Metzler <marcus@convergence.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 *
 * usage:
 * 	http://dbox:<port>/apid,vpid (for ps or avpes)
 * 	http://dbox:<port>/pid1,pid2,.... (for ts)
 *	http://dbox:<port>/filename (for ts file)
 *
 * 	each pid must be a hexadecimal number
 *
 * command line parameters:
 *      -pes    send a packetized elementary stream (2 pids)
 *      -ps     send a program stream (2 pids)
 *      -ts     send a transport stream (MAXPIDS pids, see below)
 *	-tsfile send a transport stream read from a .ts file
 */


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <ctype.h>

#if HAVE_DVB_API_VERSION < 3
    #include <ost/dmx.h>
    #define dmx_pes_filter_params dmxPesFilterParams
    #define pes_type pesType
    #ifdef HAVE_DREAMBOX_HARDWARE
	#define DMXDEV "/dev/dvb/card0/demux1"
	#define DVRDEV "/dev/dvb/card0/dvr1"
	#undef TRANSFORM
    #else
	#define DMXDEV "/dev/dvb/card0/demux0"
	#define DVRDEV "/dev/dvb/card0/dvr0"
    #endif
#else
    #define DMXDEV "/dev/dvb/adapter0/demux0"
    #define DVRDEV "/dev/dvb/adapter0/dvr0"
    #include <linux/dvb/dmx.h>
    #define TRANSFORM
#endif

#ifdef TRANSFORM
    #include <transform.h>
#else
    #include <string.h>
    #define TS_SIZE 188
#endif

/* conversion buffer sizes */
#define IN_SIZE		(TS_SIZE * 362)
#define IPACKS		2048

/* demux buffer size */
#define DMX_BUFFER_SIZE (256 * 1024)

/* maximum number of pes pids */
#define MAXPIDS		64

/* tcp packet data size */
#define PACKET_SIZE	1448

unsigned char * buf;

int dvrfd;
int demuxfd[MAXPIDS];

unsigned char demuxfd_count = 0;
unsigned char exit_flag = 0;
unsigned int writebuf_size = 0;
unsigned char writebuf[PACKET_SIZE];

#ifdef TRANSFORM
static int
sync_byte_offset (const unsigned char * buf, const unsigned int len) {

	unsigned int i;

	for (i = 0; i < len; i++)
		if (buf[i] == 0x47)
			return i;

	return -1;
}

static ssize_t
safe_read (const int fd, void * buf, const size_t count) {

	ssize_t neof = 0;
	size_t re = 0;

	while (neof >= 0 && re < count) {
		neof = read(fd, buf + re, count - re);
		if (neof > 0)
			re += neof;
	}

	return re;
}
#endif

void
packet_stdout (unsigned char * buf, int count, void * p) {

	unsigned int size;
	unsigned char * bp;
	ssize_t written;

	/*
	 * ensure, that there is always at least one complete
	 * packet inside of the send buffer
	 */
	while (writebuf_size + count >= PACKET_SIZE) {

		/*
		 * how many bytes are to be sent from the input buffer?
		 */
		size = PACKET_SIZE - writebuf_size;

		/*
		 * send buffer is not empty, so copy from
		 * input buffer to get a complete packet
		 */
		if (writebuf_size) {
			memcpy(writebuf + writebuf_size, buf, size);
			bp = writebuf;
		}

		/*
		 * if send buffer is empty, then do not memcopy,
		 * but send directly from input buffer
		 */
		else {
			bp = buf;
		}

		/*
		 * write the packet, count the amount of really
		 * written bytes
		 */
		written = write(STDOUT_FILENO, bp, PACKET_SIZE);

		/*
		 * exit on error
		 */
		if (written == -1) {
			exit_flag = 1;
			return;
		}

		/*
		 * if the packet could not be written completely, then
		 * how many bytes must be stored in the send buffer
		 * until the next packet is to be sent?
		 */
		writebuf_size = PACKET_SIZE - written;

		/*
		 * move all bytes of the packet which were not sent
		 * to the beginning of the send buffer
		 */
		if (writebuf_size)
			memmove(writebuf, bp + written, writebuf_size);

		/*
		 * advance in the input buffer
		 */
		buf += size;

		/*
		 * decrease the todo size
		 */
		count -= size;
	}

	/*
	 * if there are still some bytes left in the input buffer,
	 * then store them in the send buffer and increase send
	 * buffer size
	 */
	if (count) {
		memcpy(writebuf + writebuf_size, buf, count);
		writebuf_size += count;
	}
}


#ifdef TRANSFORM
static int
dvr_to_ps (const int dvr_fd, const unsigned short audio_pid, const unsigned short video_pid, const unsigned char ps) {

	unsigned char buf[IN_SIZE];
	unsigned char mbuf[TS_SIZE];
	unsigned char mod = 0;
	int i;
        int count;
	unsigned short pid;
	ipack pa, pv;
	ipack *p;

	init_ipack(&pa, IPACKS, packet_stdout, ps);
	init_ipack(&pv, IPACKS, packet_stdout, ps);

	/* read 188 bytes */
	if (safe_read(dvr_fd, mbuf, TS_SIZE) < 0)
		return -1;

	/* find beginning of transport stream */
	i = sync_byte_offset(mbuf, TS_SIZE);

	if (i == -1)
		return -1;

	/* store first part of ts packet */
	memcpy(buf, mbuf + i, TS_SIZE - i);

	if (i != 0) {
		/* read missing bytes of ts packet */
		if (safe_read(dvr_fd, mbuf, i) < 0)
			return -1;

		/* store second part of ts packet */
		memcpy(buf + TS_SIZE - i, mbuf, i);
	}

	i = TS_SIZE;

	while (!exit_flag) {

		if (mod) {
			memcpy(buf, mbuf, mod);
			i = mod;
		}

		count = safe_read(dvr_fd, buf + i, IN_SIZE - i) + i;

		mod = count % TS_SIZE;

		if (mod) {
			memcpy(mbuf, buf + count - mod, mod);
			count -= mod;
		}

		for (i = 0; i < count; i += TS_SIZE) {
			unsigned char off = 0;

			i += sync_byte_offset(buf, TS_SIZE);

			if (i == -1)
				continue;

			if (count - i < TS_SIZE)
				break;

			if (!(buf[i + 3] & 0x10)) // no payload?
				continue;

			pid = get_pid(buf + i + 1);

			if (pid == video_pid)
				p = &pv;
			else if (pid == audio_pid)
				p = &pa;
			else
				continue;

			if ((buf[i + 1] & 0x40) && (p->plength == MMAX_PLENGTH - 6)) {
				p->plength = p->found - 6;
				p->found = 0;
				send_ipack(p);
				reset_ipack(p);
			}

			if (buf[i + 3] & 0x20) // adaptation field?
				off = buf[i + 4] + 1;

			instant_repack(buf + 4 + off + i, TS_SIZE - 4 - off, p);
		}

		i = 0;
	}

	return 0;
}

#endif // TRANSFORM

static int
setPesFilter (const unsigned short pid)
{
	int fd;
	struct dmx_pes_filter_params flt; 

	if ((fd = open(DMXDEV, O_RDWR)) < 0)
		return -1;

	if (ioctl(fd, DMX_SET_BUFFER_SIZE, DMX_BUFFER_SIZE) < 0)
		return -1;

	flt.pid = pid;
	flt.input = DMX_IN_FRONTEND;
	flt.output = DMX_OUT_TS_TAP;
	flt.pes_type = DMX_PES_OTHER;
	flt.flags = 0;

	if (ioctl(fd, DMX_SET_PES_FILTER, &flt) < 0)
		return -1;

	if (ioctl(fd, DMX_START, 0) < 0)
		return -1;

	return fd;
}


static void
unsetPesFilter (int fd) {
	ioctl(fd, DMX_STOP);
	close(fd);
}


int
main (int argc, char ** argv) {

	int pid;
	int pids[MAXPIDS];
	unsigned char *bp;
	unsigned char mode;
	unsigned char tsfile[IN_SIZE];
	int tsfilelen = 0;
	int fileslice = 0;
	int i = 0;
	
	if (argc != 2)
		return EXIT_FAILURE;
#ifdef TRANSFORM
	if (!strncmp(argv[1], "-pes", 4))
		mode = 0;
	else if (!strncmp(argv[1], "-ps", 3))
		mode = 1;
	else 
#endif	
	if (!strncmp(argv[1], "-tsfile", 7))
		mode = 3;
	else
	if (!strncmp(argv[1], "-ts", 3))
		mode = 2;
	else
		return EXIT_FAILURE;

	buf = (unsigned char *) malloc(IN_SIZE);

	if (buf == NULL) {
		perror("malloc");
		return EXIT_FAILURE;
	}

	fclose(stderr);

	bp = buf;

	/* read one line */
	while (bp - buf < IN_SIZE) {
		unsigned char c;
		read(STDIN_FILENO, &c, 1);
		if ((*bp++ = c) == '\n')
			break;
	}

	*bp++ = 0;
	bp = buf;

	/* send response to http client */
	if (!strncmp(buf, "GET /", 5)) {
		printf("HTTP/1.1 200 OK\r\nServer: streamts (%s)\r\n\r\n", &argv[1][1]);
		fflush(stdout);
		bp += 5;
	}

	if (mode != 3)
	{
		if ((dvrfd = open(DVRDEV, O_RDONLY)) < 0) {
			free(buf);
			return EXIT_FAILURE;
		}

		/* parse stdin / url path, start dmx filters */
		do {
			sscanf(bp, "%x", &pid);

			pids[demuxfd_count] = pid;

			if ((demuxfd[demuxfd_count] = setPesFilter(pid)) < 0)
				break;

			demuxfd_count++;
		}
		while ((bp = strchr(bp, ',')) && (bp++) && (demuxfd_count < MAXPIDS));
	}
	else
	{
		/* ts filename */
		int j = 0;
		i = 0;
		while (i < strlen(bp) - 3)
		{
			if ((bp[i] == '.') && (bp[i + 1] == 't') && (bp[i + 2] == 's'))
			{
				tsfile[j] = bp[i];
				tsfile[j + 1] = bp[i + 1];
				tsfile[j + 2] = bp[i + 2];
				tsfile[j + 3] = '\0';
				break;
			}
			else
			if ((bp[i] == '%') && (bp[i + 1] == '2') && (bp[i + 2] == '0'))
			{
				tsfile[j++] = ' ';
				i += 3;
			}
			else
				tsfile[j++] = bp[i++];
		}
		tsfilelen = strlen(tsfile);
		/* open ts file */
		if ((dvrfd = open(tsfile, O_RDONLY)) < 0) {
			free(buf);
			return EXIT_FAILURE;
		}
	}

#ifdef TRANSFORM
	/* convert transport stream and write to stdout */
	if (((mode == 0) || (mode == 1)) && (demuxfd_count == 2))
		dvr_to_ps(dvrfd, pids[0], pids[1], mode);

	/* write raw transport stream to stdout */
	else 
#endif	
	if (mode == 2 || mode == 3) {
		size_t pos;
		ssize_t r;

		while (!exit_flag) {
			/* always read IN_SIZE bytes */
			for (pos = 0; pos < IN_SIZE; pos += r) {
				r = read(dvrfd, buf + pos, IN_SIZE - pos);
				if (r == -1) {
					/* Error */
					exit_flag = 1;
					break;
				} else if (r == 0) {
					/* End of file */
					if (mode == 3) {
						close(dvrfd);
						sprintf(&tsfile[tsfilelen], ".%03d", ++fileslice);
						dvrfd = open(tsfile, O_RDONLY);
					}
					if ((dvrfd == -1) || (mode != 3)) {
						exit_flag = 1;
						break;
					}
				}
			}

			packet_stdout(buf, pos, NULL);
		}
	}

	if (mode != 3)
	{
		while (demuxfd_count > 0)
			unsetPesFilter(demuxfd[--demuxfd_count]);
	}

	close(dvrfd);
	free(buf);
	
	return EXIT_SUCCESS;
}

