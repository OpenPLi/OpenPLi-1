/* 
 * test_av.c - Test program for new API
 *
 * Copyright (C) 2000 Ralph  Metzler <ralph@convergence.de>
 *                  & Marcus Metzler <marcus@convergence.de>
                      for convergence integrated media GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <ost/dmx.h>
#include <ost/frontend.h>
#include <ost/sec.h>
#include <ost/video.h>
#include <ost/audio.h>
#include <sys/poll.h>

int audioStop(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_STOP,0) < 0)){
		perror("AUDIO STOP: ");
		return -1;
	}

	return 0;
}

int audioPlay(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_PLAY) < 0)){
		perror("AUDIO PLAY: ");
		return -1;
	}

	return 0;
}


int audioPause(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_PAUSE) < 0)){
		perror("AUDIO PAUSE: ");
		return -1;
	}

	return 0;
}


int audioContinue(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_CONTINUE) < 0)){
		perror("AUDIO CONTINUE: ");
		return -1;
	}

	return 0;
}

int audioSelectSource(int fd, audioStreamSource_t source)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_SELECT_SOURCE, source) < 0)){
		perror("AUDIO SELECT SOURCE: ");
		return -1;
	}

	return 0;
}



int audioSetMute(int fd, boolean state)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_SET_MUTE, state) < 0)){
		perror("AUDIO SET MUTE: ");
		return -1;
	}

	return 0;
}

int audioSetAVSync(int fd,boolean state)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_SET_AV_SYNC, state) < 0)){
		perror("AUDIO SET AV SYNC: ");
		return -1;
	}

	return 0;
}

int audioSetBypassMode(int fd,boolean mode)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_SET_BYPASS_MODE, mode) < 0)){
		perror("AUDIO SET BYPASS MODE: ");
		return -1;
	}

	return 0;
}


int audioChannelSelect(int fd, audioChannelSelect_t select)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_CHANNEL_SELECT, select) < 0)){
		perror("AUDIO CHANNEL SELECT: ");
		return -1;
	}

	return 0;
}

int audioGetStatus(int fd)
{
	struct audioStatus stat;
	int ans;

	if ( (ans = ioctl(fd,AUDIO_GET_STATUS, &stat) < 0)){
		perror("AUDIO GET STATUS: ");
		return -1;
	}

	printf("Audio Status:\n");
	printf("  Sync State          : %s\n",
	       (stat.AVSyncState ? "SYNC" : "NO SYNC"));
	printf("  Mute State          : %s\n",
	       (stat.muteState ? "muted" : "not muted"));
	printf("  Play State          : ");
	switch ((int)stat.playState){
	case AUDIO_STOPPED:
		printf("STOPPED (%d)\n",stat.playState);
		break;
	case AUDIO_PLAYING:
		printf("PLAYING (%d)\n",stat.playState);
		break;
	case AUDIO_PAUSED:
		printf("PAUSED (%d)\n",stat.playState);
		break;
	default:
		printf("unknown (%d)\n",stat.playState);
		break;
	}
	
	printf("  Stream Source       : ");
	switch((int)stat.streamSource){
	case AUDIO_SOURCE_DEMUX:
		printf("DEMUX (%d)\n",stat.streamSource);
		break;
	case AUDIO_SOURCE_MEMORY:
		printf("MEMORY (%d)\n",stat.streamSource);
		break;
	default:
		printf("unknown (%d)\n",stat.streamSource);
		break;
	}

	printf("  Channel Select      : ");
	switch((int)stat.channelSelect){
	case AUDIO_STEREO:
		printf("Stereo (%d)\n",stat.channelSelect);
		break;
	case AUDIO_MONO_LEFT:
		printf("Mono left(%d)\n",stat.channelSelect);
		break;
	case AUDIO_MONO_RIGHT:
		printf("Mono right (%d)\n",stat.channelSelect);
		break;
	default:
		printf("unknown (%d)\n",stat.channelSelect);
		break;
	}
	printf("  Bypass Mode         : %s\n",
	       (stat.bypassMode ? "ON" : "OFF"));

	return 0;

}

int videoStop(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_STOP,0) < 0)){
		perror("VIDEO STOP: ");
		return -1;
	}

	return 0;
}

int videoPlay(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_PLAY) < 0)){
		perror("VIDEO PLAY: ");
		return -1;
	}

	return 0;
}


int videoFreeze(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_FREEZE) < 0)){
		perror("VIDEO FREEZE: ");
		return -1;
	}

	return 0;
}


int videoContinue(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_CONTINUE) < 0)){
		perror("VIDEO CONTINUE: ");
		return -1;
	}

	return 0;
}

int videoSelectSource(int fd, videoStreamSource_t source)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_SELECT_SOURCE, source) < 0)){
		perror("VIDEO SELECT SOURCE: ");
		return -1;
	}

	return 0;
}



int videoSetBlank(int fd, boolean state)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_SET_BLANK, state) < 0)){
		perror("VIDEO SET BLANK: ");
		return -1;
	}

	return 0;
}

int videoFastForward(int fd,int nframes)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_FAST_FORWARD, nframes) < 0)){
		perror("VIDEO FAST FORWARD: ");
		return -1;
	}

	return 0;
}

int videoSlowMotion(int fd,int nframes)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_SLOWMOTION, nframes) < 0)){
		perror("VIDEO SLOWMOTION: ");
		return -1;
	}

	return 0;
}

int videoGetStatus(int fd)
{
	struct videoStatus stat;
	int ans;

	if ( (ans = ioctl(fd,VIDEO_GET_STATUS, &stat) < 0)){
		perror("VIDEO GET STATUS: ");
		return -1;
	}

	printf("Video Status:\n");
	printf("  Blank State          : %s\n",
	       (stat.videoBlank ? "BLANK" : "STILL"));
	printf("  Play State           : ");
	switch ((int)stat.playState){
	case VIDEO_STOPPED:
		printf("STOPPED (%d)\n",stat.playState);
		break;
	case VIDEO_PLAYING:
		printf("PLAYING (%d)\n",stat.playState);
		break;
	case VIDEO_FREEZED:
		printf("FREEZED (%d)\n",stat.playState);
		break;
	default:
		printf("unknown (%d)\n",stat.playState);
		break;
	}
	
	printf("  Stream Source        : ");
	switch((int)stat.streamSource){
	case VIDEO_SOURCE_DEMUX:
		printf("DEMUX (%d)\n",stat.streamSource);
		break;
	case VIDEO_SOURCE_MEMORY:
		printf("MEMORY (%d)\n",stat.streamSource);
		break;
	default:
		printf("unknown (%d)\n",stat.streamSource);
		break;
	}

	printf("  Format (Aspect Ratio): ");
	switch((int)stat.videoFormat){
	case VIDEO_FORMAT_4_3:
		printf("4:3 (%d)\n",stat.videoFormat);
		break;
	case VIDEO_FORMAT_16_9:
		printf("16:9 (%d)\n",stat.videoFormat);
		break;
	default:
		printf("unknown (%d)\n",stat.videoFormat);
		break;
	}

	printf("  Display Format       : ");
	switch((int)stat.displayFormat){
	case VIDEO_PAN_SCAN:
		printf("Pan&Scan (%d)\n",stat.displayFormat);
		break;
	case VIDEO_LETTER_BOX:
		printf("Letterbox (%d)\n",stat.displayFormat);
		break;
	case VIDEO_CENTER_CUT_OUT:
		printf("Center cutout (%d)\n",stat.displayFormat);
		break;
	default:
		printf("unknown (%d)\n",stat.displayFormat);
		break;
	}
	return 0;
}

int videoStillPicture(int fd, struct videoDisplayStillPicture *sp)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_STILLPICTURE, sp) < 0)){
		perror("VIDEO STILLPICTURE: ");
		return -1;
	}

	return 0;
}

#define BUFFY 32768
#define NFD   2
void play_file_av(int filefd, int fd, int fd2)
{
	char buf[BUFFY];
	int count;
	int written;
	struct pollfd pfd[NFD];
	int stopped = 0;
	int ch;

	pfd[0].fd = STDIN_FILENO;
	pfd[0].events = POLLIN;
	
	pfd[1].fd = fd;
	pfd[1].events = POLLOUT;
	
	pfd[2].fd = fd2;
	pfd[2].events = POLLOUT;
	
	videoSelectSource(fd,VIDEO_SOURCE_MEMORY);
	audioSelectSource(fd2,AUDIO_SOURCE_MEMORY);

	// FIXME: only seems to work if starting audio first!
	audioPlay(fd2);
	videoPlay(fd);
	
	
	count = read(filefd,buf,BUFFY);
	write(fd,buf,count);
	
	while ( (count = read(filefd,buf,BUFFY)) >= 0  ){
		written = 0;
		while(written < count){
			if (poll(pfd,NFD,1)){
				if (pfd[1].revents & POLLOUT){
					written += write(fd,buf+written,
							count-written);
				}
				if (pfd[0].revents & POLLIN){
					int c = getchar();
					switch(c){
					case 'z':
						videoFreeze(fd);
						printf("playback frozen\n");
						stopped = 1;
						break;

					case 's':
						videoStop(fd);
						printf("playback stopped\n");
						stopped = 1;
						break;
						
					case 'c':
						videoContinue(fd);
						printf("playback continued\n");
						stopped = 0;
						break;

					case 'p':
						videoPlay(fd);
						audioPlay(fd2);
					        audioSetAVSync(fd2, true);
						audioSetMute(fd2, false);
						printf("playback started\n");
						stopped = 0;
						break;

					case 'f':
					        audioSetAVSync(fd2, false);
						audioSetMute(fd2, true);
						videoFastForward(fd,0);
						printf("fastforward\n");
						stopped = 0;
						break;

					case 'm':
					        audioSetAVSync(fd2, false);
						audioSetMute(fd2, true);
						videoSlowMotion(fd,2);
						printf("slowmotion\n");
						stopped = 0;
						break;

					case 'q':
						videoContinue(fd);
						exit(0);
						break;
					}
				}
				
			}
		}
	}
}

void load_iframe(int filefd, int fd)
{
	struct stat st;
	struct videoDisplayStillPicture sp;

	fstat(filefd, &st);
	
	sp.iFrame = (char *) malloc(st.st_size);
	sp.size = st.st_size;
	printf("I-frame size: %d\n", sp.size);
	
	if(!sp.iFrame) {
		printf("No memory for I-Frame\n");
		return;
	}

	printf("read: %d bytes\n",read(filefd,sp.iFrame,sp.size));
	videoStillPicture(fd,&sp);

	sleep(3);
	videoPlay(fd);
}

main(int argc, char **argv)
{
	int fd, fd2;
	int filefd;

	if (argc < 2) return -1;

	if ( (filefd = open(argv[1],O_RDONLY)) < 0){
		perror("File open:");
		return -1;
	}
	if((fd = open("/dev/ost/video",O_RDWR|O_NONBLOCK)) < 0){
		perror("VIDEO DEVICE: ");
		return -1;
	}
	if((fd2 = open("/dev/ost/audio",O_RDWR|O_NONBLOCK)) < 0){
		perror("AUDIO DEVICE: ");
		return -1;
	}
	    
	


	videoSetBlank(fd,false);
	//videoPlay(fd);
	//sleep(4);
	//videoFreeze(fd);
	//sleep(3);
	//videoContinue(fd);
	//sleep(3);
	//videoStop(fd);
	videoGetStatus(fd);
	audioGetStatus(fd2);


	//load_iframe(filefd, fd);
	play_file_av(filefd, fd, fd2);
	close(fd);
	close(filefd);
	return 0;


}

