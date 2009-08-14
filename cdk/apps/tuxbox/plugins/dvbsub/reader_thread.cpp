#include <cerrno>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>

#include "Debug.hpp"
#include "PacketQueue.hpp"
#include "helpers.hpp"

#include "reader_thread.hpp"

int reader_running = 1;
extern PacketQueue packet_queue;
pthread_cond_t packetCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t packetMutex = PTHREAD_MUTEX_INITIALIZER;

void* reader_thread(void *arg)
{
	uint8_t tmp[16];  /* actually 6 should be enough */
	int count;
	int len;
	uint16_t packlen;
	uint8_t* buf;
	int fd = *(int*)arg;
	struct timeval tvselect;
	fd_set rfds;

	debug.print(Debug::DEBUG, "%s started\n", __FUNCTION__);

	while (reader_running) {
		/* Wait for pes sync */
		len = 0;
		count = 0;

		tvselect.tv_sec = 1;
		tvselect.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		int status =  select(fd+1, &rfds, NULL, NULL, &tvselect);

		if ( status == -1 )
		{
			perror("[reader_thread]: select returned ");
			// in case of an error return timeout...?!
			goto error;
		}
		else if ( status == 0 ) // Timeout!
		{
			continue;
		}

		if(FD_ISSET(fd, &rfds))
		{
			while (reader_running) {
				len = read(fd, &tmp[count], 3-count);
//printf("reader: fd = %d, len = %d\n", fd, len);
				if (len == (3-count)) {
					if (	(tmp[0] == 0x00) &&
						(tmp[1] == 0x00) &&
						(tmp[2] == 0x01)) {
						count = 3;
						break;
					}
				} else if (len > 0) {
					count += len;
					continue;
				} else if (len == 0) {
					/* EOF */
					debug.print(Debug::INFO, "EOF while reading stream\n");
					goto error;
				} else if (len < 0) {
					if (errno == EAGAIN || errno == EOVERFLOW) {
						sleep(1);
						continue;
					} else if (errno != EINTR) {
						/* ERROR */
						debug.print(Debug::ERROR, "'%s' while reading stream (1-%d)\n", strerror(errno));
						goto error;
					} else {
						debug.print(Debug::DEBUG, "Got EINTR while reading stream\n");
						continue;
					}
				}
				tmp[0] = tmp[1];
				tmp[1] = tmp[2];
				count = 2;
			}

			/* read stream id & length */
			while ((count < 6) && reader_running) {
//printf("try to read 6-count = %d\n", 6-count);
				len = read(fd, &tmp[count], 6-count);
				if (unlikely(len < 0)) {
					if (errno == EAGAIN || errno == EOVERFLOW) {
						sleep(1);
					} else if (errno != EINTR) { /* ERROR */
						debug.print(Debug::ERROR, "'%s' while reading stream (2-%d)\n", strerror(errno), errno);
						goto error;
					}
				} else if (unlikely(len == 0)) {
					/* EOF */
					debug.print(Debug::INFO, "EOF while reading stream\n");
					goto error;
				} else {
					count += len;
				}
			}

			packlen =  getbits(tmp, 4*8, 16) + 6;	
	
			buf = new uint8_t[packlen];

			/* TODO: Throws an exception on out of memory */

			/* copy tmp[0..5] => buf[0..5] */
			*(uint32_t*)buf = *(uint32_t*)tmp;
			((uint16_t*)buf)[2] = ((uint16_t*)tmp)[2];
	
			/* read rest of the packet */
			while((count < packlen) && reader_running) {
				len = read(fd, buf+count, packlen-count);
				if (unlikely(len < 0)) {
					if (errno == EAGAIN || errno == EOVERFLOW) {
						sleep(1);
					} else if (errno != EINTR) { /* ERROR */
						debug.print(Debug::ERROR, "'%s' while reading stream\n",
												strerror(errno));
						delete[] buf;
						goto error;
					}
				} else if (unlikely(len == 0)) {
					/* EOF */
					debug.print(Debug::INFO, "EOF while reading stream\n");
					delete[] buf;
					goto error;
				} else {
					count += len;
				}
			}
	
			/* Packet now in memory */
			packet_queue.push(buf);
			/* TODO: allocation exception */
			// wake up dvb thread
			pthread_mutex_lock(&packetMutex);
			pthread_cond_broadcast(&packetCond);
			pthread_mutex_unlock(&packetMutex);
		}
	}

 error:
	debug.print(Debug::DEBUG, "%s stopped\n", __FUNCTION__);
	pthread_exit(NULL);
}
