#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/input.h>

#define HAVE_DVB_API_VERSION 3

#if HAVE_DVB_API_VERSION < 3
#include <ost/dmx.h>
#define dmx_output_t		dmxOutput_t
#define dmx_pes_filter_params	dmxPesFilterParams
#define pes_type		pesType
#else
#include <linux/dvb/dmx.h>
#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>
#define DEMUX_DEV "/dev/dvb/adapter0/demux0"
#endif

#include "Debug.hpp"
#include "PacketQueue.hpp"
#include "semaphore.h"
#include "reader_thread.hpp"
#include "dvbsub_thread.hpp"

#ifdef AS_PLUGIN
#include <plugin.h>
extern "C" void plugin_exec(PluginParam *Parameter);

#define RC_MUTE		0x71
#define RC_VOL_DOWN	0x72
#define RC_VOL_UP	0x73
#define RC_EXIT		0x66

#define Log2File	printf
#define RECVBUFFER_STEPSIZE 1024

enum {NOERROR, NETWORK, DENIED, NOSERVICE, BOXTYPE, THREAD, ABOUT};
enum {GET_VOLUME, SET_VOLUME, SET_MUTE, SET_CHANNEL};

int volume;
unsigned short Key;

struct SOURCE
{
    bool valid;

    char addr[16];
    char user[16];
    char pass[16];
    int  port;

} source_int;
#endif

Debug debug;
PacketQueue packet_queue;
sem_t event_semaphore;

extern int reader_running;
extern int dvbsub_running;
pthread_t threadReader;
pthread_t threadDvbsub;
int rc_fd, fb_fd;

int init(int pid) {
	int fd;
	int trc;
	struct dmx_pes_filter_params f;

	debug.set_level(42);

	if (pid <= 0)
	{
		printf("error invalid pid 0x%d\n", pid);
		return -1;
	}

	fd = open(DEMUX_DEV, O_RDWR|O_NONBLOCK);
	if (fd == -1)
	{
		printf("failed to open " DEMUX_DEV);
		return -1;
	}
	
	printf("DEMUX_DEV: fd = %d, pid = 0x%x\n", fd, pid);
	
	f.pid = pid;
	f.input = DMX_IN_FRONTEND;
	f.output = DMX_OUT_TAP;
	f.pes_type = DMX_PES_OTHER;
	f.flags = DMX_IMMEDIATE_START;
	if (ioctl(fd, DMX_SET_PES_FILTER, &f) == -1)
		printf("error DMX_SET_PES_FILTER:  (subtitling)");
	else
		printf("started subtitling filter..\n");

	reader_running = true;
	// reader-Thread starten
	trc = pthread_create(&threadReader, 0, reader_thread, &fd);
	if (trc) {
		fprintf(stderr, "[dvb-sub] failed to create reader-thread (rc=%d)\n", trc);
		return EXIT_FAILURE;
	}

	dvbsub_running = true;
	// subtitle decoder-Thread starten
	trc = pthread_create(&threadDvbsub, 0, dvbsub_thread, &fb_fd);
	if (trc) {
		fprintf(stderr, "[dvb-sub] failed to create dvbsub-thread (rc=%d)\n", trc);
		return EXIT_FAILURE;
	}

	return(0);
}

#ifdef AS_PLUGIN
/***** WebServerRequest *******************************************************/
int WebServerRequest(int command, char *parameter_str, int parameter_val)
{
    int websrv_socket;
    int bytes;
    int recvbuffer_size;
    int buffer_offset;
    bool done;
    struct sockaddr_in websrv_data;
    char sendbuffer[256];
    char *recvbuffer;
    struct timeval tv;
    fd_set readfd;

	// prepare connection
	if((websrv_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
	    Log2File("socket() failed: %s\n", strerror(errno));

	    return NETWORK;
	}

	websrv_data.sin_family = AF_INET;
	websrv_data.sin_addr.s_addr = inet_addr(source_int.addr);
	websrv_data.sin_port = htons(source_int.port);

	if(connect(websrv_socket, (sockaddr*)&websrv_data, sizeof(sockaddr)) == -1)
	{
	    close(websrv_socket);

	    Log2File("connect() failed: %s\n", strerror(errno));

	    return NETWORK;
	}

	// build base64 string
	char encoding_table[64] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
	char decoded_string[64], encoded_string[64];
	int src_index, dst_index;

	strncpy(decoded_string, source_int.user, sizeof(decoded_string));
	strcat(decoded_string, ":");
	strcat(decoded_string, source_int.pass);

	for(src_index = dst_index = 0; src_index < (int)(strlen(decoded_string)); src_index += 3, dst_index += 4)
	{
	    encoded_string[0 + dst_index] = encoding_table[decoded_string[0 + src_index] >> 2];
	    encoded_string[1 + dst_index] = encoding_table[(decoded_string[0 + src_index] & 3) << 4 | decoded_string[1 + src_index] >> 4];
	    encoded_string[2 + dst_index] = encoding_table[(decoded_string[1 + src_index] & 15) << 2 | decoded_string[2 + src_index] >> 6];
	    encoded_string[3 + dst_index] = encoding_table[decoded_string[2 + src_index] & 63];
	}

	encoded_string[dst_index] = '\0';

	if(strlen(decoded_string) % 3)
	{
	    for(src_index = 0; src_index < (int)(3 - (strlen(decoded_string) % 3)); src_index++, dst_index--)
	    {
	        encoded_string[dst_index - 1] = '=';
	    }
	}

	// build url
	switch(command)
	{
	    case GET_VOLUME:
		snprintf(sendbuffer, sizeof(sendbuffer), "GET /control/volume HTTP/1.0\nAuthorization: Basic %s\n\n", encoded_string);
		break;

	    case SET_VOLUME:
		snprintf(sendbuffer, sizeof(sendbuffer), "GET /control/volume?%d HTTP/1.0\nAuthorization: Basic %s\n\n", parameter_val, encoded_string);
		break;

	    case SET_MUTE:
		snprintf(sendbuffer, sizeof(sendbuffer), "GET /control/volume?%s HTTP/1.0\nAuthorization: Basic %s\n\n", parameter_val ? "mute" : "unmute", encoded_string);
		break;

	    case SET_CHANNEL:
		snprintf(sendbuffer, sizeof(sendbuffer), "GET /control/zapTo?%s HTTP/1.0\nAuthorization: Basic %s\n\n", parameter_str, encoded_string);
		break;
	}

	// send request
	done = false;

	buffer_offset = 0;

	while(!done)
	{
	    if((bytes = send(websrv_socket, &sendbuffer + buffer_offset, strlen(sendbuffer) - buffer_offset, 0)) == -1)
	    {
		close(websrv_socket);

		Log2File("send() failed: %s\n", strerror(errno));

		return NETWORK;
	    }

	    if(bytes == (int)strlen(sendbuffer))
	    {
		done = true;
	    }
	    else
	    {
		Log2File("send() incomplete, continue...\n");

		buffer_offset += bytes;
	    }
	}

	// recv answer
	done = false;

	recvbuffer = NULL;
	recvbuffer_size = 0;
	buffer_offset = 0;

	while(!done)
	{
	    recvbuffer_size += RECVBUFFER_STEPSIZE;

	    if(!(recvbuffer = (char*)realloc(recvbuffer, recvbuffer_size)))
	    {
		free(recvbuffer);

		close(websrv_socket);

		Log2File("realloc() failed!\n");

		return NETWORK;
	    }

	    tv.tv_sec = 3;
	    tv.tv_usec = 0;

	    FD_ZERO(&readfd);
	    FD_SET(websrv_socket, &readfd);

	    select(websrv_socket + 1, &readfd, NULL, NULL, &tv);

	    if(!FD_ISSET(websrv_socket, &readfd))
	    {
	        free(recvbuffer);

	        close(websrv_socket);

	        Log2File("recv() timed out\n");

		return NETWORK;
	    }

	    if((bytes = recv(websrv_socket, recvbuffer + buffer_offset, RECVBUFFER_STEPSIZE, 0)) == -1)
	    {
		free(recvbuffer);

		close(websrv_socket);

		Log2File("recv() failed: %s\n", strerror(errno));

		return NETWORK;
	    }
	    else if(bytes)
	    {
		buffer_offset += bytes;
	    }
	    else
	    {
		recvbuffer[buffer_offset] = '\0';

		done = true;
	    }
	}

	close(websrv_socket);

	// check answer
	if(strstr(recvbuffer, "401 Unauthorized"))
	{
	    free(recvbuffer);

	    Log2File("Access to Web-Server [\"%s\":\"%s\"@%s:%d] denied! Check User/Password...\n", source_int.user, source_int.pass, source_int.addr, source_int.port);

	    return DENIED;
	}

	switch(command)
	{
	    case GET_VOLUME:
		// find last CR/LF before value
		char *ptr = &recvbuffer[strlen(recvbuffer)-1];
		while ((ptr > recvbuffer) && (*(ptr-1) != '\r'))
			ptr--;

		volume = atoi(ptr);
		break;
	}

	// cleanup
	free(recvbuffer);
	return NOERROR;
}

/***** GetRCCode **************************************************************/

int GetRCCode()
{
	unsigned short rccode = 0xDEAD;
	struct input_event ev;

	if(read(rc_fd, &ev, sizeof(ev)) == sizeof(ev))
	{
		if(ev.value)
		{
//			printf("get rc event = 0x%x\n", ev.code);
			rccode = ev.code;
		}

		if((rccode & 0xFF00) != 0x8000 && (rccode & 0x00FF) != 0x00FF)
		{
			rccode &= 0x00FF;
		}
		else
		{
			rccode = 0xDEAD;
		}
	}

	return rccode;
}
#endif

int loop()
{
#ifdef AS_PLUGIN
	strcpy(source_int.user, "root");
	strcpy(source_int.pass, "dbox2");
	strcpy(source_int.addr, "127.0.0.1");
	source_int.port = 80;

	// control loop
	do
	{
		Key = GetRCCode();

		switch(Key)
		{
			case RC_VOL_DOWN:
				WebServerRequest(GET_VOLUME, NULL, volume);
				if(volume >= 2)
				{
					volume -= 2;
					WebServerRequest(SET_VOLUME, NULL, volume);
				}
				break;

			case RC_VOL_UP:
				WebServerRequest(GET_VOLUME, NULL, volume);
				if(volume <= 61)
				{
					volume += 2;
					WebServerRequest(SET_VOLUME, NULL, volume);
				}
				break;

			case RC_MUTE:
				static bool mute = false;
				mute ^= 1;
				WebServerRequest(SET_MUTE, NULL, mute);
				break;

			default:
				break;
		}
	}
	while(Key != RC_EXIT);
#else

	do
	{
		sleep(1);
	} while(1);
#endif
	// reader-Thread stoppen
	reader_running = false;
	pthread_join(threadReader, NULL);

	// reader-Thread stoppen
	dvbsub_running = false;
	pthread_join(threadDvbsub, NULL);

	return 0;
}

#ifdef AS_PLUGIN
/******************************************************************************
 * plugin_exec                                                                *
 ******************************************************************************/

char versioninfo[32];

void plugin_exec(PluginParam *par)
{
	int subpid, sx, ex, sy, ey;
	int ret;
	char cvs_revision[] = "$Revision: 1.1 $";

	/* show versioninfo */
	sscanf(cvs_revision, "%*s %s", versioninfo);
	printf("DBOX2 DVB subtitle plugin %s\n", versioninfo);

	/* get params */
	subpid = fb_fd = rc_fd = sx = ex = sy = ey = -1;

	for (; par; par = par->next)
	{
		if (!strcmp(par->id, P_ID_SUBPID))
			subpid = atoi(par->val);
		else if (!strcmp(par->id, P_ID_FBUFFER))
			fb_fd = atoi(par->val);
		else if (!strcmp(par->id, P_ID_RCINPUT))
			rc_fd = atoi(par->val);
		else if (!strcmp(par->id, P_ID_OFF_X))
			sx = atoi(par->val);
		else if (!strcmp(par->id, P_ID_END_X))
			ex = atoi(par->val);
		else if (!strcmp(par->id, P_ID_OFF_Y))
			sy = atoi(par->val);
		else if (!strcmp(par->id, P_ID_END_Y))
			ey = atoi(par->val);
	}

	if (subpid == -1 || fb_fd == -1 || rc_fd == -1 || sx == -1 || ex == -1 || sy == -1 || ey == -1)
	{
		printf("DvbSub <Invalid Param(s)>\n");
		return;
	}

	ret = init(subpid);
	if (!ret)
		ret = loop();
}

#else

int main(int argc, char **argv) {
	int subpid;
	int ret;

	if (argc != 2) { 
		printf("usage: dvb-sub <subtitle pid>\n");
		return -1;
	}

	sscanf(argv[1], "%x", &subpid);

	ret = init(subpid);

	if (!ret)
		ret = loop();

	return ret;
}

#endif
