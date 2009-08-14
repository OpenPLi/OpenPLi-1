#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <config.h>

#define BSIZE		4096

#ifdef HAVE_OST_DMX_H
	#include <ost/dmx.h>
	#ifdef HAVE_DREAMBOX_HARDWARE
		#define DMXDEV  "/dev/dvb/card0/demux1"
	#else
		#define DMXDEV	"/dev/dvb/card0/demux0"
	#endif
	#define dmx_sct_filter_params dmxSctFilterParams
#else
	#include <linux/dvb/dmx.h>
	#define DMXDEV	"/dev/dvb/adapter0/demux0"
#endif

int main(void)
{
	int fd, pid, filter, mask;
	struct dmx_sct_filter_params flt;
	char buffer[BSIZE], *bp;
	unsigned char c;
	ssize_t r, w;

	bp = buffer;

	while (bp - buffer < BSIZE) {
		if (read(STDIN_FILENO, &c, 1) == 1)
			if ((*bp++ = c) == '\n')
				break;
	}

	*bp = '\0';

	bp = buffer;

	if (!strncmp(buffer, "GET /", 5)) {
		printf("HTTP/1.1 200 OK\r\nServer: d-Box network\r\n\r\n");
		fflush(stdout);
		bp += 5;
	}

	if ((fd = open(DMXDEV, O_RDONLY)) == -1) {
		perror(DMXDEV);
		return EXIT_FAILURE;
	}

	if (sscanf(bp, "%4x,%2x,%2x", &pid, &filter, &mask) != 3)
		return EXIT_FAILURE;

	printf("pid: %x\n", pid);
	printf("filter: %x\n", filter);
	printf("mask: %x\n", mask);

	memset(&flt, 0, sizeof(struct dmx_sct_filter_params));
	flt.pid = pid;
	flt.filter.filter[0] = filter;
	flt.filter.mask[0] = mask;
	flt.flags = DMX_IMMEDIATE_START;

	if (ioctl(fd, DMX_SET_FILTER, &flt) == -1) {
		perror("DMX_SET_FILTER");
		close(fd);
		return EXIT_FAILURE;
	}

	for (;;) {
		if ((r = read(fd, buffer, BSIZE)) == -1) {
			perror("read");
			continue;
		}

		bp = buffer;

		while (r != 0) {
			if ((w = write(STDOUT_FILENO, bp, r)) == -1) {
				perror("write");
				continue;
			}

			bp += w;
			r -= w;
		}
	}

	close(fd);

	return EXIT_SUCCESS;
}

