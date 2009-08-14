/*
 * $Id: grab.cpp,v 1.2 2002/08/31 01:06:19 obi Exp $
 *
 * Audio and video stream recorder for use with streampes
 *
 * Copyright (C) 2001 Peter Niemayer et al.
 * See AUTHORS for details.
 * See README for details on who-wrote-what within this software.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 

#include "AVBuf.h"
#include "Remuxer.h"
#include "SigFlags.h"
#include "StopWatch.h"

#include <iostream>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <sys/poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sched.h>
#include <sys/mman.h>
#include <pthread.h>

// ####################################################################

#define ALIGN4(x) ( (((long)(x)) +2) & (~0x3) )

// ####################################################################
// data shared between main and reader thread:

unsigned long num_avbufs;
AVBuf * avbufs;

volatile long new_read_size_audio = AUDIO_BUF_SIZE/2; // full for AC3, half for MPEG audio...

int nice_increment;
bool no_rt_prio;

volatile bool thread_started;

volatile bool terminate_avread_thread;

volatile bool avread_thread_failed;
volatile bool avread_thread_ended;

volatile bool new_display_available = false;

pthread_t avrthread;

// ####################################################################
// network reading functions for Linux on dbox2 by Felix Domke,
// some modifications by Peter Niemayer

int dvb_video_pes;
int dvb_audio_pes;

int openPES(const char * name, unsigned short port, int pid)
{
	fprintf(stderr,"opening %s:%d PID %d\n", name, (int)port, pid);
	
	struct hostent * hp = gethostbyname(name);
		
	struct sockaddr_in adr;
	memset ((char *)&adr, 0, sizeof(struct sockaddr_in));
				
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	adr.sin_port = htons(port);
		
   if (adr.sin_addr.s_addr == 0) {
		fprintf(stderr, "unable to lookup hostname");
		return -1;
	}
		         
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (-1 == connect(sock, (sockaddr*)&adr, sizeof(struct sockaddr_in))) {
		perror("connect");
		close(sock);
		return -1;
	}
	
	char buffer[100];		
	sprintf(buffer, "GET /%x HTTP/1.0\r\n\r\n", pid);
	write(sock, buffer, strlen(buffer));
	
	return sock;
}

int readPES(int fd, void *buffer, int len)
{
// Note from Peter to Felix: I better don't touch your code here,
// but it looks dirty: You better check errno==EINTR and try
// again when this was the cause for read() returning early.
// Furthermore: Wouldn't a non-block read be better, here?
#if 1
	int left=len;
	char *buf=(char*)buffer;
	//int tries=3;
	while (left>0)
	{
		int r=read(fd, buf, left);
		if (r<=0)
			return r;
		buf+=r;
		left-=r;
#if 0
		if (!--tries)
			break;
#else
		if (r<1000)
			break;
#endif
	}
	return len-left;
#else
	return read(fd, buffer, len);
#endif
}

// ####################################################################
// AV reader thread for dbox2 / EtherNet

void * AVReadThreadDBox2(void * thread_arg) {
	
	{
		
		if (mlockall(MCL_CURRENT | MCL_FUTURE)) {
			fprintf(stderr, "WARNING: unable to lock memory. Swapping may disturb the avread_thread\n");			
		}
		
		bool rt_enabled = false;
		if (!no_rt_prio) {
			struct sched_param sp;
			
			
			sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
			if (pthread_setschedparam(avrthread, SCHED_FIFO, &sp) < 0) {
			//if (sched_setscheduler(0, SCHED_FIFO, &sp) < 0) {
				fprintf(stderr, "WARNING: cannot enable real-time scheduling - will try to renice\n");
			} else {
				rt_enabled = true;
			}
		}
		if (!rt_enabled) {
			if (::nice(nice_increment)) {
				fprintf(stderr,"WARNING: avread_thread cannot change nice level - continuing\n");
			}
		}
	}

	// ---------------------------
	
	sigset_t newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGINT);
	sigaddset(&newmask, SIGTERM);
	sigaddset(&newmask, SIGHUP);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);
	
	if (pthread_sigmask(SIG_BLOCK, &newmask, 0)) {
		fprintf(stderr,"cannot block signals\n");
		avread_thread_failed = true;
		avread_thread_ended = true;
		return NULL;
	}
	
	// ---------------------------
	
	// ---------------------------
	
	struct pollfd pollfds[2];
	pollfds[0].fd = dvb_video_pes;
	pollfds[0].events = POLLIN;
	pollfds[1].fd = dvb_audio_pes;
	pollfds[1].events = POLLIN;
	
	long read_size_audio = new_read_size_audio;
	long read_size_video = VIDEO_BUF_SIZE;	
	
	
	// ---------------------------
	
	unsigned long rec_idx = 0; // avbuf to use next
	
	pthread_mutex_lock(& avbufs[rec_idx].mutex); // lock mutex for avbuf to use next
	
	thread_started = true;
	
	avbufs[rec_idx].clear();
	avbufs[rec_idx].resync = true; // always a new start at the beginning..

	// ##############################################

	
	unsigned long poll_counter_video_succ = 0;
	unsigned long poll_counter_audio_succ = 0;
	
	time_t last_second = 0;
	
	bool up_to_current = false;
	
	for (;;) {
				
		if (terminate_avread_thread) {
			break;
		}
		
		StopWatch poll_timer;
		int pres = ::poll(pollfds, 2, 100);
		if (pres < 0) {
			if (errno == EINTR) {
				continue;
			}
			fprintf(stderr,"poll failed\n");
			avread_thread_failed = true;
			avread_thread_ended = true;
			return NULL;			
		}
		if (poll_timer.stop() > 10) {
			// if it took us some time to get data, we know that we're
			// up to date with polling. This is important to assure the
			// PTS in the aux data reflects sort of the current video time... 
			up_to_current = true;
		}
		
		if (pollfds[0].revents & POLLIN) {
			
			read_size_video = readPES(dvb_video_pes, avbufs[rec_idx].video, VIDEO_BUF_SIZE);
			
			if (read_size_video <= 0) {
				fprintf(stderr, "receive video failed\n");
				avread_thread_failed = true;
				avread_thread_ended = true;
				return NULL;
			}
				
			if (up_to_current) {
				poll_counter_video_succ += 1;

				avbufs[rec_idx].video_valid = read_size_video;
			}
			
		}

		if (pollfds[1].revents & POLLIN) {
			
			read_size_audio = readPES(dvb_audio_pes, avbufs[rec_idx].audio, new_read_size_audio);
			if (read_size_audio <= 0) {
				fprintf(stderr, "receive audio failed\n");
				avread_thread_failed = true;
				avread_thread_ended = true;
				return NULL;
			}
			
			if (up_to_current) {
				poll_counter_audio_succ += 1;
				
				avbufs[rec_idx].audio_valid = read_size_audio;
			}

			if (up_to_current) {
				poll_counter_audio_succ += 1;
				
				avbufs[rec_idx].audio_valid = read_size_audio;
			}
		}


		// polls done, look for avbufs to pass

		if (   avbufs[rec_idx].video_valid
		    || avbufs[rec_idx].audio_valid
			) {

			// ok, get the next avbuf and release the current one (that is filled)

			unsigned long next_idx = rec_idx + 1;
			if (next_idx >= num_avbufs) {
				next_idx = 0;
			}

			if (pthread_mutex_trylock(& avbufs[next_idx].mutex)) {
				fprintf(stderr,
				        "WARNING: avbuf overrun, lost data: %ld video %ld audio\n",
				        avbufs[rec_idx].video_valid,
						  avbufs[rec_idx].audio_valid);

				// we have to stay with the current buffer :-(
				avbufs[rec_idx].clear();
				avbufs[rec_idx].resync = true;

				//video_started = false;

			} else {

				// we could get the last buffer - check whether it is unused

				if (avbufs[next_idx].is_used()) {

					pthread_mutex_unlock(& avbufs[next_idx].mutex);

					fprintf(stderr,
				      	  "WARNING: avbuf overrun, lost data: %ld video %ld aux %ld audio\n",
				      	  avbufs[rec_idx].video_valid,
							  avbufs[rec_idx].aux_valid,
							  avbufs[rec_idx].audio_valid);

					// we have to stay with the current buffer :-(
					avbufs[rec_idx].clear();
					avbufs[rec_idx].resync = true;

				} else {					

					// let's release our last one and use the new one, now...

					pthread_mutex_unlock(& avbufs[rec_idx].mutex);

					rec_idx = next_idx;
				}

			}

		} else {

			// none of the polls were successful

			//usleep(10 * 1000);
		}
		
		
		time_t now = time(0);
		if (now != last_second) {
			last_second = now;
			/*
			fprintf(stderr, "polls video %ld   audio %ld   aux %ld \r",
			        poll_counter_video_succ,
			        poll_counter_audio_succ,
				poll_counter_aux_succ
					);
			*/
		}
		
				
		
		// end of thread main loop, here
	}
	
	// waiting for the last requests to be answered isn't sensible for dbox2 network read...		
	
	pthread_mutex_unlock(& avbufs[rec_idx].mutex);
	// fprintf(stderr, "AVReadThread terminating normally\n");
		
	avread_thread_ended = true;
	
	return 0;
}

// ####################################################################

void usage (char * filename) {

	fprintf(stderr, "grab by Peter Niemayer and the dbox2 on linux team - see README for details\n\n"
		"usage: %s <options>\n\n" 
		"------- mandatory options: ---------\n"
		"-p <vpid> <apid> video and audio pids to receive [none]\n"
		"\n"
		"------- basic options: -------------\n"
		"-host <host>   hostname or ip address [dbox]\n"
		"-port <port>   port number [31338]\n"
		"-o <filename>  basename of output files [vts_01_]\n"
		"-m <minutes>   number of minutes to record [infinite]\n"
		"-s <megabyte>  maximum size per recorded file [1000]\n"
		"-q             be quiet\n"
		"\n"
		"------- advanced options: ----------\n"
		"-b <megabyte>  size of AV buffer memory [8]\n"
		"-nort          do not try to enable realtime-scheduling for the AV reader thread\n"
		"-n <niceinc>   relative nice-level of AV reader thread if -nort (default: -10)\n"
		"-nonfos        do not open a new file for writing on resync\n"
		"-nomux         do not multiplex, write 2 seperate files with a/v ES\n"
		"-raw           do not multiplex, write 3 seperate files with a/v/pts\n"
		"-loop          record forever (two files named *1.* *2.*, starting at PTS 0)\n"
		"-1ptspergop    expect only 1 PTS per GOP (needed for some channels)\n"
		"\n"
		"------- handled signals: -----------\n"
		"SIGUSR1        force immediate resync\n"
		"SIGUSR2        force write to next file\n"
		"\n"
		"values in square brackets indicate default settings if available\n", filename);
}

int main( int argc, char *argv[] ) {

	// set various default-values

	char * fname_output = "vts_01_";      // default output filename stem

	time_t seconds_to_record = -1;     // default number of seconds to record

	long av_bufmem = 8 * 1024 * 1024;  // default size for av-reception buffer

	nice_increment = -10;              // default nice-level adjustment for receiving thread

	unsigned long long max_bytes_per_file = 1000*1024*1024;

	bool quiet = false;

	bool nomux = false;

	bool raw_data = false;

	long checkpoint = 10*60;
	
	bool new_file_on_sync = true;
	
	int dbox2port = 31338;
	
	const char * dbox2name = 0;
	dbox2name = "dbox";
	
	int apid = -1;
	
	int vpid = -1;
	
	bool one_pts_per_gop = false;
	
	no_rt_prio = false;
	
	bool start_new_files_with_pts_0 = false;
	
	bool record_loop = false;

	if (argc == 1) {
		usage(argv[0]);
		return -1;
	}

	for (int i = 1; i < argc; i++) {
		
		if (!strcmp("-o", argv[i])) {
			i++; if (i >= argc) { fprintf(stderr, "need filename for -o\n"); return -1; }
			fname_output = argv[i];
		
		} else if (!strcmp("-m", argv[i])) {
			i++; if (i >= argc) { fprintf(stderr, "need number of minutes for -m\n"); return -1; }
			
			seconds_to_record = ((time_t)atol(argv[i]))*60;
		
		} else if (!strcmp("-b", argv[i])) {
			i++; if (i >= argc) { fprintf(stderr, "need argument for -b\n"); return -1; }
			
			av_bufmem = atol(argv[i]) * 1024 * 1024;
		
		} else if (!strcmp("-n", argv[i])) {
			i++; if (i >= argc) { fprintf(stderr, "need argument for -n\n"); return -1; }
			
			nice_increment = atoi(argv[i]);
		
		} else if (!strcmp("-s", argv[i])) {
			i++; if (i >= argc) { fprintf(stderr, "need max size in MB argument for -s\n"); return -1; }
			
			max_bytes_per_file = 1024ULL*1024ULL*((unsigned long long)atoi(argv[i]));
		
		} else if (!strcmp("-port", argv[i])) {
			i++; if (i >= argc) { fprintf(stderr, "need argument for -port\n"); return -1; }
			
			dbox2port = atoi(argv[i]);
			
		} else if (!strcmp("-p", argv[i])) {
			
			i++; if (i >= argc) { fprintf(stderr, "need two arguments for -p\n"); return -1; }
			sscanf(argv[i], "%x", &vpid);

			i++; if (i >= argc) { fprintf(stderr, "need two arguments for -p\n"); return -1; }
			sscanf(argv[i], "%x", &apid);
			
		} else if (!strcmp("-q", argv[i])) {
			quiet = true;
		
		} else if (!strcmp("-nomux", argv[i])) {
			nomux = true;

		} else if (!strcmp("-raw", argv[i])) {
			raw_data = true;

		} else if (!strcmp("-loop", argv[i])) {
			record_loop = true;
			start_new_files_with_pts_0 = true;
		
		} else if (!strcmp("-1ptspergop", argv[i])) {
			one_pts_per_gop = true;
		
		} else if (!strcmp("-host", argv[i])) {
			i++; if (i >= argc) { fprintf(stderr, "need argument for -host\n"); return -1; }
			dbox2name = argv[i];

		} else if (!strcmp("-nonfos", argv[i])) {
			new_file_on_sync = false;
			
		} else if (!strcmp("-nort", argv[i])) {
			no_rt_prio = true;
		
		} else {
			usage(argv[0]);
			return -1;
		}
	}
	
	/* we need at leat an audio and video pid to continue */
	if ((vpid <= 0) || (apid <= 0)) {
		usage(argv[0]);
		return -1;
	}

	// ----------------------------------------
	// open the files to be written
	
	int fname_mpg_counter = 1;
	
	char fname_mpg[1024]; fname_mpg[0] = 0;
	char fname_mpp[1024]; fname_mpp[0] = 0;
	char fname_mpv[1024]; fname_mpv[0] = 0;
	char fname_aux[1024]; fname_aux[0] = 0;
	
	char * fname_extension = ".mpg";
	if (!strcmp(fname_output, "vts_01_")) {
		fname_extension = ".vob";
	}
	
	strncat(fname_mpg, fname_output, 1000);
	char fname_mpg_num[10];
	sprintf(fname_mpg_num, "%d", fname_mpg_counter);
	strcat(fname_mpg, fname_mpg_num);
	strcat(fname_mpg, fname_extension);
	
	strncat(fname_mpp, fname_output, 1000);
	strcat(fname_mpp, ".mpp");
	
	strncat(fname_mpv, fname_output, 1000);
	strcat(fname_mpv, ".mpv");
	
	strncat(fname_aux, fname_output, 1000);
	strcat(fname_aux, ".pts");
	
	FILE * file_mpg = 0;
	FILE * file_mpp = 0;
	FILE * file_mpv = 0;
	FILE * file_aux = 0;
	
	if (nomux || raw_data) {
		if ( (file_mpp = fopen(fname_mpp,"w")) == NULL ) {
			fprintf(stderr,"unable to open %s\n",fname_mpp);
			return -1;
		}

		if ( (file_mpv = fopen(fname_mpv,"w")) == NULL ) {
			fprintf(stderr,"unable to open %s\n",fname_mpv);
			return -1;
		}

		if ((!nomux) && ( (file_aux = fopen(fname_aux,"w")) == NULL )) {
			fprintf(stderr,"unable to open %s\n",fname_aux);
			return -1;
		}

	} else {
		if (!strcmp(fname_output, "-")) {
		
			file_mpg = stdout;
			max_bytes_per_file = ~0ULL;
			new_file_on_sync = false;
		
		} else {
			if ( (file_mpg = fopen(fname_mpg,"w")) == NULL ) {
				fprintf(stderr,"unable to open %s\n",fname_mpg);
				return -1;
			}
		}	
	}
	
	// -------------------------------------------
	// setup remuxer
	
	Remuxer remuxer;
	remuxer.one_pts_per_gop = one_pts_per_gop;
	
	// -------------------------------------------
	// find / open / initialize the device
	
	char dbuf2[1000]; // buffer for dvb2k display

	dvb_video_pes = openPES(dbox2name, dbox2port, vpid);
	if (dvb_video_pes < 0) return -1;

	dvb_audio_pes = openPES(dbox2name, dbox2port, apid);
	if (dvb_audio_pes < 0) return -1;
		
	
	// -----------------------------------------------------------------------
	// start the AV reading thread

	num_avbufs = av_bufmem / sizeof(AVBuf);

	avbufs = new AVBuf[num_avbufs];
	if (!avbufs) {
		fprintf(stderr,"unable to allocate %ld bytes for avbufs\n",
		        ((long)sizeof(AVBuf)*num_avbufs)
				 );
		return -1;
	}

	avrthread = 0;
	
	terminate_avread_thread = false;
	avread_thread_failed = false;
	avread_thread_ended = false;
	thread_started = false;

	pthread_create( &avrthread, 0, AVReadThreadDBox2 , 0);

	SigFlags sigflags;

	if (SigFlags::handlers[SIGUSR1].activate()) {
		fprintf(stderr,"unable to handle SIGUSR1\n");
	}

	if (SigFlags::handlers[SIGUSR2].activate()) {
		fprintf(stderr,"unable to handle SIGUSR2\n");
	}

	// ----------------------------------------

	fprintf(stderr,"waiting for first data to become available\n");

	// first busy-wait until thread_started is signaled
	for (;;) {
		if (thread_started) {
			break;
		}
		
		if (SigFlags::flags[SIGINT] || SigFlags::flags[SIGTERM]) {
			
			SigFlags::flags[SIGINT] = 0;
			SigFlags::flags[SIGTERM] = 0;

			fprintf(stderr, "terminating on signal from user\n");
			
			terminate_avread_thread = true;			
		}
		
		if (avread_thread_ended) {
			fprintf(stderr, "avread_thread ended unexpectedly to main program\n");
			break;
		}
		
		usleep(10 * 1000); // wait 10ms
	}
	
	// ----------------------------------------
	
	if (!terminate_avread_thread && !avread_thread_ended) {
		
		//fprintf(stderr,"entering main loop\n");
		
		unsigned long long received_avbufs = 0;
		unsigned long long received_video = 0;
		unsigned long long received_audio = 0;
		unsigned long long received_aux = 0;

		unsigned long long last_received_video = 0;
		unsigned long long last_received_audio = 0;
		
		//unsigned long long grabbed_audio_packets = 0;
		//unsigned long long grabbed_video_packets = 0;
		
		unsigned long read_idx = 0;
		
		
		time_t start_second = time(0);
		time_t last_second = start_second+1;
		
		// main loop
		
		for (;;) {
			
			if (avread_thread_ended) {
				fprintf(stderr, "\navread_thread ended unexpectedly to main program\n");
				break;
			}
			
			if (SigFlags::flags[SIGINT] || SigFlags::flags[SIGTERM]) {

				SigFlags::flags[SIGINT] = 0;
				SigFlags::flags[SIGTERM] = 0;

				//fprintf(stderr, "\nterminating avread on users request\n");
				
				break;			
			}

			if (SigFlags::flags[SIGUSR1]) {
				SigFlags::flags[SIGUSR1] = 0;
				
				fprintf(stderr,"\nperforming resync on users request (SIGUSR1)          \n");
				remuxer.perform_resync();
			}
			
			// try to lock a filled avbuf with incoming data
			AVBuf & avbuf = avbufs[read_idx];
			
			if (pthread_mutex_trylock(& avbuf.mutex)) {
				
				// we cannot lock it yet - so no new data - ok, we'll wait.			
				usleep(10 * 1000); // wait 10ms
				
			} else {
				
				// great, new data available!
				
				received_avbufs += 1;
				
				if (avbuf.resync) {
					fprintf(stderr,"resyncing at request of reader thread\n");
					remuxer.perform_resync();
				}

				if (avbuf.aux_valid) {
					received_aux   += avbuf.aux_valid;
					
					unsigned long p1 =   (avbuf.aux[2] << 24)
					                   | (avbuf.aux[3] << 16)
					                   | (avbuf.aux[4] << 8)
					                   | (avbuf.aux[5])
											 ;
					char p1buf[32];
					strcpy(p1buf, pts_to_hms((double)p1));
					
				   /*	
					unsigned long p2 =   (avbuf.aux[12] << 24)
					                   | (avbuf.aux[13] << 16)
					                   | (avbuf.aux[14] << 8)
					                   | (avbuf.aux[15])
											 ;
					*/
					/*										 
					fprintf(stdout,"aux   at %s + %ld\n",
					        p1buf,
							  (((long)p2)-((long)(p1)))
							 );
					*/
					
					
					if (raw_data) {
						if (1 != fwrite(avbuf.aux, avbuf.aux_valid, 1, file_aux)) {
							fprintf(stderr, "\nerror while writing to %s\n", fname_aux);
							break;
						}
						
					} else {
						remuxer.supply_aux_data(avbuf.aux, avbuf.aux_valid);
					}
					
				}
				
				if (avbuf.video_valid) {
					received_video += avbuf.video_valid;
					
					if (raw_data) {
						if (1 != fwrite(avbuf.video, avbuf.video_valid, 1, file_mpv)) {
							fprintf(stderr, "\nerror while writing to %s\n", fname_mpv);
							break;
						}

					} else if (nomux) {

						remuxer.supply_video_data(avbuf.video, avbuf.video_valid);

						if (remuxer.write_mpv(file_mpv)) {
							fprintf(stderr, "\nfailed to write .mpv file\n");
							break;
						}
						remuxer.remove_video_packets(remuxer.video_packets_avail);

					} else {
												
						remuxer.supply_video_data(avbuf.video, avbuf.video_valid);
					}
					
				}
				
				if (avbuf.audio_valid) {
					received_audio += avbuf.audio_valid;
					
					if (raw_data) {
						if (1 != fwrite(avbuf.audio, avbuf.audio_valid, 1, file_mpp)) {
							fprintf(stderr, "\nerror while writing to %s\n", fname_mpp);
							break;
						}

					} else if (nomux) {

						remuxer.supply_audio_data(avbuf.audio, avbuf.audio_valid);
						if (remuxer.write_mpp(file_mpp)) {
							fprintf(stderr, "\nfailed to write .mpp file\n");
							break;
						}
						remuxer.remove_audio_packets(remuxer.audio_packets_avail);
						
					} else {
						
						remuxer.supply_audio_data(avbuf.audio, avbuf.audio_valid);					
					}
						
					// need this awful trick to make both MPEG and AC3 sound work... how ugly... :-(
					if (remuxer.wanted_audio_stream == STREAM_PRIVATE_1) {
						new_read_size_audio = AUDIO_BUF_SIZE;
					} else {
						new_read_size_audio = AUDIO_BUF_SIZE / 2;							
					}
				}

				
				// release the buffer, we had it long enough
				avbuf.clear();
				
				pthread_mutex_unlock(& avbuf.mutex);
				
				// up for the next avbuf!
				read_idx += 1;
				if (read_idx >= num_avbufs) {
					read_idx = 0;
				}
				
				if (!nomux) {
					
					static bool was_synced_before = false;
					
					// open a new file?
					if (   SigFlags::flags[SIGUSR2]
					    || remuxer.total_bytes_written > max_bytes_per_file
						 || (new_file_on_sync && was_synced_before && remuxer.resync && fname_mpg_counter < 200)
						) {
						
						if (SigFlags::flags[SIGUSR2]) {
							SigFlags::flags[SIGUSR2] = 0;
							fprintf(stderr,"starting new file on users request (SIGUSR2)        \n");
						}
						
						if (remuxer.flush_pp(file_mpg)) {
							fprintf(stderr, "\nerror while writing to %s\n", fname_mpg);
							break;
						}
						
						if (start_new_files_with_pts_0) {
							remuxer.playtime_offset = 0.0;
							remuxer.system_clock_ref_start = remuxer.system_clock_ref;
						}
						
						fname_mpg_counter++;
						if (record_loop) {
							if (fname_mpg_counter >= 3) fname_mpg_counter = 1;
							start_second = time(0);
							received_video = 0;
							received_audio = 0;
						}
						
						remuxer.total_bytes_written = 0;
						
						fname_mpg[0] = 0;
						strncat(fname_mpg, fname_output, 1000);
						sprintf(fname_mpg_num, "%d", fname_mpg_counter);
						strcat(fname_mpg, fname_mpg_num);
						strcat(fname_mpg, fname_extension);
						
						fclose(file_mpg);
						if ( (file_mpg = fopen(fname_mpg,"w")) == NULL ) {
							fprintf(stderr,"unable to open %s\n",fname_mpg);
							return -1;
						}
					}
					
					if (remuxer.resync) was_synced_before = false;
					
					// let the remuxer try to emit MPEG-2 data...
					if (remuxer.write_mpg(file_mpg)) {
						fprintf(stderr, "\nerror while writing to %s\n", fname_mpg);
						break;					
					}
					
					if (!remuxer.resync) was_synced_before = true;
				}
			}
			
			// do some things once per second

			
			time_t now = time(0);
			if (now != last_second) {
				last_second = now;
				
				//double bandw_video = ((received_video-last_received_video)*8)/1000.0;
				//double bandw_audio = ((received_audio-last_received_audio)*8)/1000.0;
				double tbandw_video = (received_video*8ULL)/((now-start_second)*1000.0);
				double tbandw_audio = (received_audio*8ULL)/((now-start_second)*1000.0);
				
				last_received_video = received_video;
				last_received_audio = received_audio;
				
				long seconds_spent = now - start_second;
				int minutes_spent = seconds_spent / 60;
				int seconds_rem   = seconds_spent % 60;
				
				double seconds_dropped = ((double)seconds_spent) - (remuxer.get_playtime()/90000.0);
				if (seconds_dropped < 0.0) seconds_dropped = 0.0;
				
				if (received_video && received_audio) {
					if (!quiet || (seconds_spent % checkpoint) == 0) {
						fprintf(stderr, "%02d:%02d  vid %ld kbit/s  aud %ld kbit/s  syn %lld  drop %lds        \r",
						        minutes_spent, seconds_rem,
								  (long)tbandw_video, 
								  (long)tbandw_audio,
								  remuxer.resyncs,
								  (long)seconds_dropped
								 );
					}
					
					if ((seconds_spent % checkpoint) == 0) {
						fprintf(stderr,"\n");
					}
					
					unsigned long togo = 0;
					if (seconds_to_record > 0) togo = seconds_to_record - seconds_spent;
					sprintf(dbuf2, "REC %d:%02d  REM %d:%02d  SYN %lld DROP %lds ",
					        minutes_spent, seconds_rem,
					        (int)(togo/60), (int)(togo%60),
					        remuxer.resyncs,
						(long)seconds_dropped
					       );


				}
				if (seconds_to_record > 0 && now - start_second >= seconds_to_record) {
					fprintf(stderr, "\nfinished recording after %ld seconds\n", now - start_second);
					break;
				}
			}
			
			// end of main loop
		}

		// ----------------------------------------
		// emit statistics

		fprintf(stderr, "\n"); 

	}
		
	
	// ----------------------------------------

	terminate_avread_thread = true;

	if (!nomux && file_mpg) {
		if (remuxer.flush_pp(file_mpg)) {
			fprintf(stderr, "\nerror while writing to %s\n", fname_mpg);
		}
	}

	
	if (file_mpg) fclose(file_mpg);
	if (file_mpv) fclose(file_mpv);
	if (file_mpp) fclose(file_mpp);
	if (file_aux) fclose(file_aux);
	
	usleep(10 * 1000); // wait 10ms
	SigFlags::flags[SIGINT] = 0;
	SigFlags::flags[SIGTERM] = 0;
	
	
	// busy-wait until avread_thread has ended
	for (;;) {
		if (avread_thread_ended) {
			break;
		}
		
		if (SigFlags::flags[SIGINT] || SigFlags::flags[SIGTERM]) {
			
			SigFlags::flags[SIGINT] = 0;
			SigFlags::flags[SIGTERM] = 0;

			fprintf(stderr,"terminating program - but avread_thread is still alive!\n");
			break;
		}
		
		usleep(10 * 1000); // wait 10ms
	}
	
	// ----------------------------------------
	
	// avthread is gone or ignored...

	//fprintf(stderr,"stopping stream and closing device\n");
	

	if (dvb_video_pes >= 0) close(dvb_video_pes);
	if (dvb_audio_pes >= 0) close(dvb_audio_pes);

	//fprintf(stderr,"main program is finished\n");
  
	return 0;
}

