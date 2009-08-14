#include <inttypes.h>
#include <semaphore.h>
#include <sys/time.h>
#include "PacketQueue.hpp"
#include "Debug.hpp"
#include "helpers.hpp"
#include "dvbsubtitle.h"

int dvbsub_running;
extern PacketQueue packet_queue;
extern pthread_cond_t packetCond;
extern pthread_mutex_t packetMutex;

cDvbSubtitleConverter *dvbSubtitleConverter;

#define get_time() 42;

void* dvbsub_thread(void* arg)
{
	struct timespec to;
	struct timespec restartWait;
	struct timeval now;

	debug.print(Debug::DEBUG, "%s started\n", __FUNCTION__);
	if (!dvbSubtitleConverter)
		dvbSubtitleConverter = new cDvbSubtitleConverter;

	while(dvbsub_running) {
		uint8_t* packet;
		int64_t pts;
		int pts_dts_flag;
		int dataoffset;
		int packlen;
//		int err = 0;
#if 0
		int64_t current_time = get_time();

		/* Check screen updates */
		if ((page.prev_page.cleartime > 0) &&
				(page.prev_page.cleartime < current_time)) {
			// clear_page(page.prev_page);
			page.prev_page.cleartime = -1;
		}
		if ((page.prev_page.cleartime > 0) && 
				(page.timestamp > 0) &&
				(page.timestamp < current_time)) {
			// draw_page(page);
			page.timestamp = -1;
		}

		if (page.timestamp == -1) {
#endif
			gettimeofday(&now, NULL);
			TIMEVAL_TO_TIMESPEC(&now, &restartWait);
			restartWait.tv_sec += 1;

			pthread_mutex_lock( &packetMutex );
			int ret = pthread_cond_timedwait( &packetCond, &packetMutex, &restartWait );
			pthread_mutex_unlock( &packetMutex );
			if (ret == ETIMEDOUT)
			{
				continue;
			}
			else if (ret == EINTR)
			{
				debug.print(Debug::DEBUG, "pthread_cond_timedwait fails with %s\n", strerror(errno));
			}
			packet = packet_queue.pop();
			if (!packet) {
				debug.print(Debug::DEBUG, "Error no packet found\n");
				continue;
			}
			packlen = (packet[4] << 8 | packet[5]) + 6;
			
			/* Get PTS */
			pts_dts_flag = getbits(packet, 7*8, 2);
			if ((pts_dts_flag == 2) || (pts_dts_flag == 3)) {
				pts = (uint64_t)getbits(packet, 9*8+4, 3) << 30;  /* PTS[32..30] */
				pts |= getbits(packet, 10*8, 15) << 15;           /* PTS[29..15] */
				pts |= getbits(packet, 12*8, 15);                 /* PTS[14..0] */
			} else {
				pts = 0;
			}
			
			dataoffset = packet[8] + 8 + 1;
			if (packet[dataoffset] != 0x20) {
				debug.print(Debug::DEBUG, "Not a dvb subtitle packet, discard it\n");
				goto next_round;
			}
			
			debug.print(Debug::DEBUG, "PES packet PTS=%Ld (%02d:%02d:%02d.%d)\n",
							pts, (int)(pts/324000000), (int)((pts/5400000)%60),
							(int)((pts/90000)%60), (int)(pts%90000));

			if (packlen <= dataoffset + 3) {
				debug.print(Debug::INFO, "Packet too short, discard\n");
				goto next_round;
			}

#if 0
			while (packet[dataoffset + 2] == 0x0f) {
				err = subtitling_segment(&packet[dataoffset + 2],
							 packlen - (dataoffset + 2), pts);
				if (err < 0) {
					break;
				}
				dataoffset += err;
			}
			if (err >= 0) {
				if (dataoffset + 2 >= packlen) {
					debug.print(Debug::INFO, "Packet too short, discard it\n",
											dataoffset, packlen);
				} else if (packet[dataoffset + 2] != 0xFF) {
					debug.print(Debug::INFO, "End_of_PES is missing\n");
				}
			}
#endif
			if (packet[dataoffset + 2] == 0x0f) {
				dvbSubtitleConverter->Convert(&packet[dataoffset + 2],
							 packlen - (dataoffset + 2), pts);
			}
#if 0
		} /* page.timestamp == -1 */
#endif
		dvbSubtitleConverter->Action();
		
	next_round:
		delete[] packet;
	}

	debug.print(Debug::DEBUG, "%s stopped\n", __FUNCTION__);
	pthread_exit(NULL);
}
