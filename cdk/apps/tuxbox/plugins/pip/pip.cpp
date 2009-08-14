#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H

extern "C"
{
    #include <mpeg2.h>
}

#include <plugin.h>
#if HAVE_DVB_API_VERSION < 3
 #define dmx_pes_filter_params dmxPesFilterParams
 #define pes_type pesType
 #define dmx_sct_filter_params dmxSctFilterParams
 #include <ost/dmx.h>
 #define DMX0	"/dev/dvb/card0/demux0"
 #define DMX1	"/dev/dvb/card0/demux1"
#else
 #include <linux/dvb/dmx.h>
 #define DMX0	"/dev/dvb/adapter0/demux0"
 #define DMX1	"/dev/dvb/adapter0/demux1"
#endif

#include <pic_1.h>
#include <pic_2.h>
#include <pic_3.h>
#include <sys/time.h>

#define TRUE  1
#define FALSE 0

#define SUCCESS	1
#define FAILURE 0

#define BORDERCOLOR  0x80
#define FONTCOLOR_G1 0xC0
#define MENUCOLOR1   1
#define MENUCOLOR2   2
#define MENUCOLOR3   3
#define FONTCOLOR_C1 4
#define FONTCOLOR_C2 5
#define FONTCOLOR_C3 1
#define FONTCOLOR_C4 3
#define SELECTBAR1   3
#define SELECTBAR2   2

#define RECVBUFFER_STEPSIZE 1024
#define INBUF_SIZE	    64*1024

#define VERSION	"0.7b"
#define BUILD	"21.05.2007"

#define CFGFILE "/var/tuxbox/config/pip.conf"
#define LOGFILE	"/tmp/pip.log"
#define PMTFILE "/tmp/pmt.pip"

#define FONT	"/share/fonts/pakenham.ttf"

#define FB	"/dev/fb/0"
#define RC	"/dev/rawir2"

#define	RC_0		0x00
#define	RC_1		0x01
#define	RC_2		0x02
#define	RC_3		0x03
#define	RC_4		0x04
#define	RC_5		0x05
#define	RC_6		0x06
#define	RC_7		0x07
#define	RC_8		0x08
#define	RC_9		0x09
#define RC_VOL_UP	0x0A	// 0x24
#define RC_VOL_DOWN	0x0B	// 0x23
#define	RC_TV		0x0C
#define RC_BQE_UP	0x0D
#define	RC_BQE_DOWN	0x0E
#define	RC_STANDBY	0x0F
#define RC_MENU		0x20
#define RC_UP		0x21
#define RC_DOWN		0x22
#define RC_LEFT		0x23
#define RC_RIGHT	0x24
#define RC_OK		0x25
#define RC_AUDIO	0x26
#define RC_VIDEO	0x27
#define RC_INFO		0x28
#define RC_RED		0x40
#define RC_GREEN	0x41
#define RC_YELLOW	0x42
#define RC_BLUE		0x43
#define RC_MUTE		0x44	// 0x0C
#define RC_TEXT		0x45
#define RC_FORWARD	0x50
#define RC_BACKWARD	0x51
#define RC_EXIT		0x52	// 0x54
#define RC_RADIO	0x53
#define RC_HELP		0x54

#define CRC_LEN		4
#define PAT_LEN		5
#define PMT_LEN		9

/* PAT - Program Association Table (ISO/IEC 13818-1, S.67) */

struct PATHEADER
{
	unsigned table_id			: 8;

	unsigned section_syntax_indicator	: 1;
	unsigned zero				: 1;
	unsigned reserved_1			: 2;
	unsigned section_length			: 12;

	unsigned transport_stream_id		: 16;

	unsigned reserved_2			: 2;
	unsigned version_number			: 5;
	unsigned current_next_indicator		: 1;

	unsigned section_number			: 8;

	unsigned last_section_number		: 8;

	/*
		N x PATDATA

		CRC_32
	*/

}__attribute__((__packed__));

struct PATDATA
{
	unsigned program_number			: 16;

	unsigned reserved			: 3;
	unsigned program_map_PID		: 13;

}__attribute__((__packed__));

/* PMT - Program Map Table (ISO/IEC 13818-1, S.70) */

struct PMTHEADER
{
	unsigned table_id			: 8;

	unsigned section_syntax_indicator	: 1;
	unsigned zero				: 1;
	unsigned reserved_1			: 2;
	unsigned section_length			: 12;

	unsigned program_number			: 16;

	unsigned reserved_2			: 2;
	unsigned version_number			: 5;
	unsigned current_next_indicator		: 1;

	unsigned section_number			: 8;

	unsigned last_section_number		: 8;

	unsigned reserved_3			: 3;
	unsigned PCR_PID			: 13;

	unsigned reserved_4			: 4;
	unsigned program_info_length		: 12;

	/*
		N x PMTDATA

		CRC_32
	*/

}__attribute__((__packed__));

struct PMTDATA
{
	unsigned stream_type			: 8;

	unsigned reserved_1			: 3;
	unsigned elementary_PID			: 13;

	unsigned reserved_2			: 4;
	unsigned ES_info_length			: 12;

}__attribute__((__packed__));

extern "C" void plugin_exec(PluginParam *Parameter);

enum {NOERROR, NETWORK, DENIED, NOSERVICE, BOXTYPE, THREAD, ABOUT};

enum {LEFT_TOP = 1, RIGHT_TOP = 3, CENTER = 5, LEFT_BOTTOM = 7, RIGHT_BOTTOM = 9};
enum {HALF = 2, THIRD = 3, QUAD = 4};

enum {GET_TRANSPONDERSERVICES, GET_STATUS, GET_PIDS, GET_VOLUME, SET_VOLUME, SET_MUTE, SET_CHANNEL};

enum {NOICON, SCRAMBLED, TIMEOUT, UNAVAILABLE};

enum {LEFT, MIDDLE, RIGHT};

enum {UNKNOWN, DM500, DM600, DM5600, DM5620, TR272, DM7000, DM7020};
char *BoxTypes[] = {"UNKNOWN", "DM500", "DM600PVR", "DM5600", "DM5620", "TR272", "DM7000", "DM7020"};
int BoxType;

struct SOURCE
{
    bool valid;

    char addr[16];
    char user[16];
    char pass[16];
    int  port;

    int services;
    int watched;
    int tv_index, tv_index_tmp;
    int pip_index, pip_index_tmp;

    struct
    {
	char  name[64];
	char  ref[64];
	short pid;
	unsigned short sid;
	int   len;
	unsigned char pmt[512];

    }service[100];

}source_int, source_ext;

struct SOURCE *source = &source_int;
bool pip_source = 0;

int volume;

int web;

unsigned short Key;

FILE *LogFile;

int rc = -1;
int fb = -1;
int dmx;

int sx = 0, sy = 0;
int xofs = 0, yofs = 0;

int scramble_check;

int PiPPosition = RIGHT_TOP, PiPSize = THIRD, PiPIcon;

int transp_value;

int demuxer = 0;
int DMXBufferSize = 1024;
int language = 0;
int debug = TRUE;
int transparency = FALSE;
int swap_cursor = FALSE;
int pmt_pip = FALSE;
int skip_p_frame = FALSE;

bool scaler_reset;
bool lock_framebuffer;
bool close_thread;
pthread_t thread_id;

unsigned char *lfb = 0;
struct fb_fix_screeninfo fix_screeninfo;
struct fb_var_screeninfo var_screeninfo;
unsigned short rd1[256];
unsigned short gn1[256];
unsigned short bl1[256];
unsigned short tr1[256];
struct fb_cmap graymap = {0, 256, rd1, gn1, bl1, tr1};
unsigned short rd2[] = {0x0000, 0x2500, 0x4A00, 0x9700, 0xE000, 0xFF00};
unsigned short gn2[] = {0x0000, 0x3A00, 0x6300, 0xAC00, 0xE000, 0xA000};
unsigned short bl2[] = {0x0000, 0x4D00, 0x7700, 0xC100, 0xE000, 0x0000};
unsigned short tr2[6];
struct fb_cmap colormap = {0, 6, rd2, gn2, bl2, tr2};

FT_Library	library = 0;
FTC_Manager	manager = 0;
FTC_SBitCache	cache;
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
FTC_ImageTypeRec desc;
#else
FTC_Image_Desc	desc;
#endif
FT_Face		face;
FT_UInt		prev_glyphindex;
FT_Bool		use_kerning;

char TempBuffer[1024];

void *DecodeThread(void *arg);
void ShowMessage(int message);

FILE *fd_conf;

/***** Log2File ***************************************************************/

void Log2File(char *string)
{
    if(LogFile)
    {
	fprintf(LogFile, "%s\n", string);

	fflush(LogFile);
    }
}

void Log2File(char *formatstring, void *val1)
{
    if(LogFile)
    {
	snprintf(TempBuffer, sizeof(TempBuffer), "%s\n", formatstring);

	fprintf(LogFile, TempBuffer, val1);

	fflush(LogFile);
    }
}

void Log2File(char *formatstring, void *val1, void *val2)
{
    if(LogFile)
    {
	snprintf(TempBuffer, sizeof(TempBuffer), "%s\n", formatstring);

	fprintf(LogFile, TempBuffer, val1, val2);

	fflush(LogFile);
    }
}

void Log2File(char *formatstring, void *val1, void *val2, void *val3)
{
    if(LogFile)
    {
	snprintf(TempBuffer, sizeof(TempBuffer), "%s\n", formatstring);

	fprintf(LogFile, TempBuffer, val1, val2, val3);

	fflush(LogFile);
    }
}

void Log2File(char *formatstring, void *val1, void *val2, void *val3, void *val4)
{
    if(LogFile)
    {
	snprintf(TempBuffer, sizeof(TempBuffer), "%s\n", formatstring);

	fprintf(LogFile, TempBuffer, val1, val2, val3, val4);

	fflush(LogFile);
    }
}

/***** ConvertString **********************************************************/

void ConvertString(char *utf8, char *ansi)
{
    unsigned char *ptr_utf8 = (unsigned char*)utf8;
    unsigned char *ptr_ansi = (unsigned char*)ansi;

    while(*ptr_utf8 != '\0' && *ptr_utf8 != '\n' && *ptr_utf8 != ';')
    {
        if(*ptr_utf8 >= 192 && *ptr_utf8 < 224)
        {
	    if(*ptr_utf8 == 0xC3)
	    {
		*ptr_ansi = 0x40 + *(ptr_utf8 + 1);
	    }
	    else
	    {
		*ptr_ansi = '?';
	    }

            ptr_utf8 += 2;
        }
        else if(*ptr_utf8 >= 224 && *ptr_utf8 < 240)
        {
	    *ptr_ansi = '?';

            ptr_utf8 += 3;
        }
        else if(*ptr_utf8 >= 240)
        {
	    *ptr_ansi = '?';

            ptr_utf8 += 4;
        }
        else
        {
            *ptr_ansi = *ptr_utf8;

            ptr_utf8++;
        }

        ptr_ansi++;
    }

    *ptr_ansi = '\0';
}

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
    char *ptrS, *ptrE;
    struct timeval tv;
    fd_set readfd;

    // prepare connection

	if((websrv_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
	    Log2File("socket() failed: %s\n", strerror(errno));

	    return NETWORK;
	}

	websrv_data.sin_family = AF_INET;
	websrv_data.sin_addr.s_addr = inet_addr(pip_source && command == GET_TRANSPONDERSERVICES ? source_ext.addr : source_int.addr);
	websrv_data.sin_port = htons(pip_source && command == GET_TRANSPONDERSERVICES ? source_ext.port : source_int.port);

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

	strncpy(decoded_string, pip_source && command == GET_TRANSPONDERSERVICES ? source_ext.user : source_int.user, sizeof(decoded_string));
	strcat(decoded_string, ":");
	strcat(decoded_string, pip_source && command == GET_TRANSPONDERSERVICES ? source_ext.pass : source_int.pass);

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
	    case GET_TRANSPONDERSERVICES:

		snprintf(sendbuffer, sizeof(sendbuffer), "GET /cgi-bin/currentTransponderServices HTTP/1.0\nAuthorization: Basic %s\n\n", encoded_string);

		break;

	    case GET_STATUS:

		snprintf(sendbuffer, sizeof(sendbuffer), "GET /cgi-bin/status HTTP/1.0\nAuthorization: Basic %s\n\n", encoded_string);

		break;

	    case GET_PIDS:

		snprintf(sendbuffer, sizeof(sendbuffer), "GET /control/zapto?getpids HTTP/1.0\nAuthorization: Basic %s\n\n", encoded_string);

		break;

	    case GET_VOLUME:

		snprintf(sendbuffer, sizeof(sendbuffer), "GET /cgi-bin/audio HTTP/1.0\nAuthorization: Basic %s\n\n", encoded_string);

		break;

	    case SET_VOLUME:

		snprintf(sendbuffer, sizeof(sendbuffer), "GET /cgi-bin/audio?volume=%d HTTP/1.0\nAuthorization: Basic %s\n\n", parameter_val, encoded_string);

		break;

	    case SET_MUTE:

		snprintf(sendbuffer, sizeof(sendbuffer), "GET /cgi-bin/audio?mute=%d HTTP/1.0\nAuthorization: Basic %s\n\n", parameter_val, encoded_string);

		break;

	    case SET_CHANNEL:

		snprintf(sendbuffer, sizeof(sendbuffer), "GET /cgi-bin/zapTo?path=%s HTTP/1.0\nAuthorization: Basic %s\n\n", parameter_str, encoded_string);

		break;
	}

    // send request

	done = FALSE;

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
		done = TRUE;
	    }
	    else
	    {
		Log2File("send() incomplete, continue...\n");

		buffer_offset += bytes;
	    }
	}

    // recv answer

	done = FALSE;

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

		done = TRUE;
	    }
	}

	close(websrv_socket);

    // check answer

	if(strstr(recvbuffer, "401 Unauthorized"))
	{
	    free(recvbuffer);

	    Log2File("Access to Web-Server [\"%s\":\"%s\"@%s:%d] denied! Check User/Password...\n", pip_source && command == GET_TRANSPONDERSERVICES ? source_ext.user : source_int.user, pip_source && command == GET_TRANSPONDERSERVICES ? source_ext.pass : source_int.pass, pip_source && command == GET_TRANSPONDERSERVICES ? source_ext.addr : source_int.addr, (void*)(pip_source && command == GET_TRANSPONDERSERVICES ? source_ext.port : source_int.port));

	    return DENIED;
	}

	switch(command)
	{
	    case GET_TRANSPONDERSERVICES:

		ptrS = recvbuffer;

		source->services = 0;

		while(*ptrS != '\0')
		{
		    if(*(ptrS + 4) == '1')		// tv-service
		    {
			source->service[source->services].sid = strtol(ptrS + 6, NULL, 16);
//Log2File("SID 0x%.4X", (void*)source->service[source->services].sid);

			if((ptrE = strchr(ptrS, ';')))
			{
			    strncpy(source->service[source->services].ref, ptrS, ptrE - ptrS);
			    source->service[source->services].ref[ptrE - ptrS] = '\0';

			    Log2File("Service-Ref  : %s", source->service[source->services].ref);

			    ptrS = ptrE + 1;

			    if((ptrE = strpbrk(ptrS, ";\n")))
			    {
				ConvertString(ptrS, source->service[source->services].name);

				Log2File("Service-Name : %s", source->service[source->services].name);
			    }

			    if(*ptrE == ';')
			    {
				ptrS = ptrE + 1;

				if(*(ptrS + 1) == '0')	// video-pid
				{
				    source->service[source->services].pid = strtol(ptrS + 2, NULL, 16);

				    Log2File("Service-PID  : 0x%.4X", (void*)source->service[source->services].pid);
				}
				else
				{
				    source->service[source->services].pid = -1;

				    Log2File("Service-PID  : not found!");
				}
			    }
			    else
			    {
				source->service[source->services].pid = -1;

				Log2File("Service-PID  : not found!");
			    }

			    source->services++;
			}
		    }

		    if((ptrS = strchr(ptrS, '\n')))
		    {
			ptrS++;
		    }
		    else
		    {
			break;
		    }
		}

		if(!source->services)
		{
		    free(recvbuffer);

		    close(websrv_socket);

		    return NOSERVICE;
		}
		else
		{
		    Log2File("");
		}

		break;

	    case GET_STATUS:

		char current_sref[64];

		if((ptrS = strstr(recvbuffer, "Current service reference:")))
		{
		    ptrS += 35;

		    if((ptrE = strchr(ptrS, '<')))
		    {
			strncpy(current_sref, ptrS, ptrE - ptrS);

			current_sref[ptrE - ptrS] = '\0';

			source_int.tv_index = source_int.pip_index = source_int.watched = 0;

			for(int i = 0; i <= source_int.services; i++)
			{
			    if(!strcmp(current_sref, source_int.service[i].ref))
			    {
				source_int.tv_index = source_int.pip_index = source_int.watched = i;

				break;
			    }
			}

			Log2File("PiP = %s\n", source_int.service[source_int.tv_index].name);
		    }
		}

		break;

	    case GET_PIDS:

		if((ptrS = strstr(recvbuffer, "\r\n\r\n")))
		{
		    source_int.service[parameter_val].pid = atoi(ptrS + 4);
		}

		break;

	    case GET_VOLUME:

		if((ptrS = strstr(recvbuffer, "volume: ")))
		{
		    volume = atoi(ptrS + 8);
		}

		break;
	}

    // cleanup

	free(recvbuffer);

	return NOERROR;
}

/*** GetPMT *******************************************************************/

void GetPMT()
{
	int PMTPidsFound = 0;
	struct dmxSctFilterParams dmx_flt;
	unsigned short PMTPids[100];
	unsigned char RecvBuffer[8192];

	// 

	    if(!pmt_pip)
	    {
		return;
	    }

	    Log2File("MultiChannelDecoding enabled! Set PMT=0 if your CAMD doesn't support /tmp/pmt.pip...\n");

	// recv PAT

		if((dmx = open(demuxer ? DMX1 : DMX0, O_RDONLY)) == -1)
		{
			Log2File((char*)(demuxer ? "open "DMX1" failed: %s" : "open "DMX0" failed: %s"), strerror(errno));

			return;
		}

		memset(dmx_flt.filter.filter, 0, DMX_FILTER_SIZE);
		memset(dmx_flt.filter.mask, 0, DMX_FILTER_SIZE);

		dmx_flt.pid              = 0x0000;
		dmx_flt.filter.filter[0] = 0x00;
		dmx_flt.filter.mask[0]   = 0xFF;
		dmx_flt.timeout          = 3000;
		dmx_flt.flags            = DMX_ONESHOT | DMX_CHECK_CRC | DMX_IMMEDIATE_START;

		if(ioctl(dmx, DMX_SET_FILTER, &dmx_flt) == -1)
		{
			Log2File("ioctl() \"DMX_SET_FILTER\" for PAT failed: %s", strerror(errno));

			close(dmx);

			return;
		}

		if(read(dmx, RecvBuffer, sizeof(RecvBuffer)) == -1)
		{
			Log2File("reading PAT failed: %s", strerror(errno));

			close(dmx);

			return;
		}

		close(dmx);

	// scan all PMTs

		struct PATHEADER *PATHeader = (struct PATHEADER*)RecvBuffer;

		for(int N = 0; N*sizeof(struct PATDATA) < PATHeader->section_length - PAT_LEN - CRC_LEN; N++)
		{
			struct PATDATA *PATData = (struct PATDATA*)(RecvBuffer + sizeof(struct PATHEADER) + N*sizeof(struct PATDATA));

			if(PATData->program_number)
			{
//Log2File("PMT-PID 0x%.4X found", (void*)PATData->program_map_PID);

				PMTPids[PMTPidsFound++] = PATData->program_map_PID;
			}
		}

	// recv PMT

		for(int i = 0; i < PMTPidsFound; i++)
		{
			if((dmx = open(demuxer ? DMX1 : DMX0, O_RDONLY)) == -1)
			{
				Log2File((char*)(demuxer ? "open "DMX1" failed: %s" : "open "DMX0" failed: %s"), strerror(errno));

				return;
			}

			memset(dmx_flt.filter.filter, 0, DMX_FILTER_SIZE);
			memset(dmx_flt.filter.mask, 0, DMX_FILTER_SIZE);

			dmx_flt.pid              = PMTPids[i];
			dmx_flt.filter.filter[0] = 0x02;
			dmx_flt.filter.mask[0]   = 0xFF;
			dmx_flt.timeout          = 3000;
			dmx_flt.flags            = DMX_ONESHOT | DMX_CHECK_CRC | DMX_IMMEDIATE_START;

			if(ioctl(dmx, DMX_SET_FILTER, &dmx_flt) == -1)
			{
				Log2File("ioctl() \"DMX_SET_FILTER\" for PMT-PID 0x%.4X skipped: %s", (void*)PMTPids[i], strerror(errno));

				close(dmx);

				continue;
			}

			if(read(dmx, RecvBuffer, sizeof(RecvBuffer)) == -1)
			{
				Log2File("reading PMT 0x%.4X skipped: %s", (void*)PMTPids[i], strerror(errno));

				close(dmx);

				continue;
			}

			close(dmx);

	// scan all SIDs

			struct PMTHEADER *PMTHeader = (struct PMTHEADER*)RecvBuffer;

			for(int i = 0; i < source->services ;i++)
			{
//Log2File("scanning #%.2d: 0x%.4X == 0x%.4X", (void*)i, (void*)PMTHeader->program_number, (void*)source->service[i].sid);

				if(PMTHeader->program_number == source->service[i].sid)
				{
					Log2File("PMT for SID 0x%.4X / %s found", (void*)source->service[i].sid, source->service[i].name);

					source->service[i].len = PMTHeader->section_length + 3;

					if(source->service[i].len <= (int)sizeof(source->service[i].pmt))
					{
						memcpy(source->service[i].pmt, RecvBuffer, source->service[i].len);
					}
					else
					{
						Log2File("PMT-Data Overflow (%d Bytes), please report!", (void*)source->service[i].len);

						source->service[i].len = 0;
					}

					break;
				}
			}
		}

    Log2File("");
}

/***** SetPMT *****************************************************************/

void SetPMT(int index)
{
	FILE *PMTFile;

        if(!pmt_pip)
        {
	    return;
	}

	if(index <= source->services && source->service[index].len)
	{
		if((PMTFile = fopen(PMTFILE, "wb")))
		{
			fwrite(source->service[index].pmt, 1, source->service[index].len, PMTFile);

			fclose(PMTFile);

			Log2File("set PMT for SID 0x%.4X / %s", (void*)source->service[index].sid, source->service[index].name);
		}
		else
		{
			Log2File("set PMT for SID 0x%.4X / %s failed: could not open "PMTFILE, (void*)source->service[index].sid, source->service[index].name);
		}
	}
	else
	{
		Log2File("set PMT for SID 0x%.4X / %s failed: %s", (void*)source->service[index].sid, (void*)(index <= source->services ? source->service[index].name : "?"), (void*)(index <= source->services ? "no Data" : "out of Range"));
	}
}

/***** SetPalette  ************************************************************/

void SetPalette(bool color, bool clearscreen)
{
    if(clearscreen)
    {
//	memset(lfb, 0, 720*576);
	memset(lfb, 0,var_screeninfo.yres*var_screeninfo.xres*2);
    }

    if(ioctl(fb, FBIOPUTCMAP, color ? &colormap : &graymap) == -1)
    {
        Log2File("ioctl() \"FBIOPUTCMAP\" failed: %s\n", strerror(errno));
    }
}

/***** DetectBoxType **********************************************************/

bool DetectBoxType()
{
    FILE *BoxInfo;

    BoxType = UNKNOWN;

    if((BoxInfo = fopen("/proc/bus/dreambox", "r")))
    {
	while(fgets(TempBuffer, sizeof(TempBuffer), BoxInfo))
	{
	    if(!strncmp(TempBuffer, "type=", 5))
	    {
		if(!strncmp(TempBuffer + 5, "DM500", 5))
		{
		    BoxType = DM500;
		}
		else if(!strncmp(TempBuffer + 5, "DM600PVR", 8))
		{
		    BoxType = DM600;
		}
		else if(!strncmp(TempBuffer + 5, "DM5600", 6))
		{
		    BoxType = DM5600;
		}
		else if(!strncmp(TempBuffer + 5, "DM5620", 6))
		{
		    BoxType = DM5620;
		}
		else if(!strncmp(TempBuffer + 5, "TR_DVB272S", 10))
		{
		    BoxType = TR272;
		}
		else if(!strncmp(TempBuffer + 5, "DM7000", 6))
		{
		    BoxType = DM7000;
		}
		else if(!strncmp(TempBuffer + 5, "DM7020", 6))
		{
		    BoxType = DM7020;
		}
	    }
	}

	fclose(BoxInfo);
    }

    if(BoxType == DM500 || BoxType == DM600 || BoxType == DM5600 || BoxType == DM5620 || BoxType == TR272)
    {
	transp_value = 0x0900;
    }
    else
    {
	transp_value = 0x3000;
    }

    for(int i = 0; i < 256; i++)
    {
        rd1[i] = i<<8;
        gn1[i] = i<<8;
        bl1[i] = i<<8;
        tr1[i] = transparency ? transp_value : 0x0000;

	if(i < 6)
	{
    	    tr2[i] = transparency ? transp_value : 0x0000;
	}
    }

    tr1[0] = tr2[0] = 0xFFFF;

    SetPalette(FALSE, FALSE);

    Log2File("BoxType = %s\n", BoxTypes[BoxType]);

    return BoxType ? SUCCESS : FAILURE;
}

/***** GetRCCode **************************************************************/

int GetRCCode()
{
	unsigned short rccode = 0xDEAD;

	if(read(rc, &rccode, sizeof(rccode)) != -1)
	{
		if((rccode & 0xFF00) != 0x8000 && (rccode & 0x00FF) != 0x00FF)
		{
			rccode &= 0x00FF;
		}
		else
		{
			rccode = 0xDEAD;
		}
	}

	if(BoxType == DM500 || BoxType == DM5600 || BoxType == DM5620 || BoxType == TR272)
	{
	    switch(rccode)
	    {
		case RC_STANDBY:

		    rccode = RC_HELP;

		    break;

		case RC_HELP:

		    rccode = RC_EXIT;

		    break;

		case RC_TV:

		    rccode = RC_MUTE;

		    break;

		case RC_FORWARD:

		    rccode = RC_BQE_UP;

		    break;

		case RC_BACKWARD:

		    rccode = RC_BQE_DOWN;

		    break;

		case RC_GREEN:

		    rccode = RC_VOL_DOWN;

		    break;

		case RC_YELLOW:

		    rccode = RC_VOL_UP;

		    break;
	    }
	}

	return rccode;
}

/***** FaceRequester **********************************************************/

FT_Error FaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
    FT_Error Result;

    if((Result = FT_New_Face(library, (const char*)face_id, 0, aface)))
    {
        Log2File("FT_New_Face() for Font \"%s\" failed\n", (char*)face_id);
    }

    return Result;
}

/***** RenderChar *************************************************************/

int RenderChar(FT_ULong currentchar, int sx, int sy, int ex, int color)
{
    int row, pitch, bit, x = 0, y = 0;
    FT_UInt glyphindex;
    FTC_SBit sbit;
    FT_Vector kerning;

    // load char

	if(!(glyphindex = FT_Get_Char_Index(face, currentchar)))
	{
	    return FAILURE;
	}

#if FREETYPE_MAJOR  == 2 && FREETYPE_MINOR == 0
	if(FTC_SBit_Cache_Lookup(cache, &desc, glyphindex, &sbit))
#else
	if(FTC_SBitCache_Lookup(cache, &desc, glyphindex, &sbit, NULL))
#endif
	{
	    return FAILURE;
	}

	if(use_kerning)
	{
	    FT_Get_Kerning(face, prev_glyphindex, glyphindex, ft_kerning_default, &kerning);

	    prev_glyphindex = glyphindex;

	    kerning.x >>= 6;
	}
	else
	{
	    kerning.x = 0;
	}

    // render char

	if(color != -1)	 			/* don't render char, return charwidth only */
	{
	    if(sx + sbit->xadvance >= ex)
	    {
		return -1; 			/* limit to maxwidth */
	    }

	    for(row = 0; row < sbit->height; row++)
	    {
		for(pitch = 0; pitch < sbit->pitch; pitch++)
		{
		    for(bit = 7; bit >= 0; bit--)
		    {
			if(pitch*8 + 7-bit >= sbit->width)
			{
			    break;		/* render needed bits only */
			}

			if((sbit->buffer[row * sbit->pitch + pitch]) & 1<<bit)
			{
			    *(lfb + (sx + sbit->left + kerning.x + x) + (sy - sbit->top + y)*720) = color;
			}

			x++;
		    }
		}

		x = 0;
		y++;
	    }
	}

    // return charwidth

	return sbit->xadvance + kerning.x;
}

/***** GetStringLen ***********************************************************/

int GetStringLen(char *string)
{
    int stringlen = 0;

    prev_glyphindex = 0;

    while(*string != '\0')
    {
	stringlen += RenderChar(*string, -1, -1, -1, -1);

	string++;
    }

    return stringlen;
}

/***** RenderString ***********************************************************/

void RenderString(char *string, int sx, int sy, int maxwidth, int alignment, int color, int backcolor)
{
    int stringlen, ex, charwidth;

    // clear area

	if(backcolor)
	{
	    for(int y = 0; y < 18; y++)
	    {
		memset(lfb + sx + (sy - 14 + y)*720, backcolor, maxwidth);
	    }
	}

    // set size
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	desc.width = desc.height = 20;
#else
	desc.font.pix_width = desc.font.pix_height = 20;
#endif

    // alignment

	if(alignment != LEFT)
	{
	    stringlen = GetStringLen(string);

	    if(stringlen < maxwidth)
	    {
		if(alignment == MIDDLE)
		{
		    sx += (maxwidth - stringlen)/2;
		}
		else
		{
		    sx += maxwidth - stringlen;
		}
	    }
	}

    // reset kerning

	prev_glyphindex = 0;

    // render string

	ex = sx + maxwidth;

	while(*string != '\0')
	{
	    if((charwidth = RenderChar(*string, sx, sy, ex, color)) == -1)
	    {
		return; /* string > maxwidth */
	    }

	    sx += charwidth;

	    string++;
	}
}

/***** DrawPiP ****************************************************************/

void DrawPiPpart(int ypos)
{
    int y;
    unsigned char *pic_ptr;


    // set position

	switch(PiPPosition)
	{
	    case LEFT_TOP:

		sx = 50;
		sy = 50;

		break;

	    case RIGHT_TOP:

		sx = 720 - 720/PiPSize - 50;
		sy = 50;

		break;

	    case CENTER:

		sx = (720 - 720/PiPSize)/2;
		sy = (576 - 576/PiPSize)/2;

		break;

	    case LEFT_BOTTOM:

		sx = 50;
		sy = 576 - 576/PiPSize - 50;

		break;

	    case RIGHT_BOTTOM:

		sx = 720 - 720/PiPSize - 50;
		sy = 576 - 576/PiPSize - 50;

		break;
	}

    // draw frame
	sy+=ypos;
	memset(lfb + sx+xofs + (sy+yofs-2)*720, BORDERCOLOR, 720/PiPSize);
	memset(lfb + sx+xofs + (sy+yofs-1)*720, BORDERCOLOR, 720/PiPSize);

	for(y = 0; y < 576/PiPSize; y++)
	{
	    *(lfb + sx+xofs-1 + (sy+yofs + y)*720) = BORDERCOLOR;

	    memset(lfb + sx+xofs  + (sy+yofs + y)*720, 0x01, 720/PiPSize);

	    *(lfb + sx+xofs + 720/PiPSize + (sy+yofs + y)*720) = BORDERCOLOR;
	}

	memset(lfb + sx+xofs + (576/PiPSize + sy+yofs)*720, BORDERCOLOR, 720/PiPSize);
	memset(lfb + sx+xofs + (576/PiPSize + sy+yofs + 1)*720, BORDERCOLOR, 720/PiPSize);

	for(y = 0; y < 20; y++)
	{
	    *(lfb + sx+xofs-1 + (576/PiPSize + sy+yofs + y)*720) = BORDERCOLOR;
	    *(lfb + sx+xofs + 720/PiPSize + (576/PiPSize + sy+yofs + y)*720) = BORDERCOLOR;
	}

	memset(lfb + sx+xofs + (576/PiPSize + sy+yofs + 20)*720, BORDERCOLOR, 720/PiPSize);
	memset(lfb + sx+xofs + (576/PiPSize + sy+yofs + 21)*720, BORDERCOLOR, 720/PiPSize);

    // channelname

	RenderString(source->service[source->pip_index].name, sx+xofs, sy+yofs + 576/PiPSize + 16, 720/PiPSize, MIDDLE, FONTCOLOR_G1, TRUE);

    // icons

	switch(PiPIcon)
	{
	    case SCRAMBLED:

		pic_ptr = pic_1;

		break;

	    case TIMEOUT:

		pic_ptr = pic_2;

		break;

	    case UNAVAILABLE:

		pic_ptr = pic_3;

		break;

	    default:

		pic_ptr = NULL;
	}

	if(pic_ptr)
	{
	    for(int y = 0; y < 48; y++)
	    {
		for(int x = 0; x < 48; x++)
		{
    		    *(lfb + sx+xofs + x + ((720/PiPSize)-48)/2 + (sy+yofs + y + ((576/PiPSize)-48)/2)*720) = pic_ptr[x + y*48];
		}
	    }
	}
	sy-=ypos;

}
void DrawPiP(bool clearscreen)
{
    lock_framebuffer = TRUE;
    usleep(100000);

    // clear screen
	if(clearscreen)
	{
//	    memset(lfb, 0, 720*576);
		memset(lfb, 0,var_screeninfo.yres*var_screeninfo.xres*2);

	}
	DrawPiPpart(0);
	DrawPiPpart(var_screeninfo.yres);
    lock_framebuffer = FALSE;
}
/***** ShowMenu  **************************************************************/

void ShowMenu()
{
#define MENU_WIDTH 200
#define MENU_START 205

    int y;
    bool tmp_pip_source = pip_source;
    bool tuner_changed = FALSE;
    int selected_entry = 0;

    char *title[] = {"Service Selection", "Senderauswahl"};
    char *msg[][5] =
    {
	{"PiP-Channel",	"TV-Channel",	"PiP-Source",	"Internal Tuner",	"External Tuner"},
	{"PiP-Kanal",	"TV-Kanal",	"PiP-Quelle",	"Interner Tuner",	"Externer Tuner"}
    };

    lock_framebuffer = TRUE;
    usleep(100000);

    SetPalette(TRUE, TRUE);
	if (var_screeninfo.yoffset)
	{
		var_screeninfo.yoffset= 0;
		if (ioctl(fb, FBIOPAN_DISPLAY, &var_screeninfo)  == -1)
			printf("ioctl() \"FBIOPAN_DISPLAY\" failed: %s", strerror(errno));
	}

    for(y = 0; y < 25; y++)
    {
	memset(lfb + (720-MENU_WIDTH)/2 + (MENU_START+y)*720, MENUCOLOR1, 200);

	*(lfb + (720-MENU_WIDTH)/2 + (MENU_START+y)*720) = MENUCOLOR3;
	*(lfb + (720-MENU_WIDTH)/2 + 199 + (MENU_START+y)*720) = MENUCOLOR3;
    }

    for(y = 0; y < 140; y++)
    {
	memset(lfb + (720-MENU_WIDTH)/2 + (MENU_START+25+y)*720, MENUCOLOR2, 200);

	*(lfb + (720-MENU_WIDTH)/2 + (MENU_START+25+y)*720) = MENUCOLOR3;
	*(lfb + (720-MENU_WIDTH)/2 + 199 + (MENU_START+25+y)*720) = MENUCOLOR3;
    }

    memset(lfb + (720-MENU_WIDTH)/2 + (MENU_START-1)*720, MENUCOLOR3, 200);
    memset(lfb + (720-MENU_WIDTH)/2 + MENU_START*720, MENUCOLOR3, 200);
    memset(lfb + (720-MENU_WIDTH)/2 + (MENU_START+25)*720, MENUCOLOR3, 200);
    memset(lfb + (720-MENU_WIDTH)/2 + (MENU_START+165)*720, MENUCOLOR3, 200);
    memset(lfb + (720-MENU_WIDTH)/2 + (MENU_START+166)*720, MENUCOLOR3, 200);

    RenderString(title[language], (720-MENU_WIDTH)/2, MENU_START+18, MENU_WIDTH, MIDDLE, FONTCOLOR_C2, FALSE);
    RenderString(msg[language][0], (720-MENU_WIDTH)/2, MENU_START+45, MENU_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);
    RenderString(source->service[source->pip_index].name, (720-MENU_WIDTH)/2 + 1, MENU_START+65, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
    RenderString(msg[language][1], (720-MENU_WIDTH)/2, MENU_START+90, MENU_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);
    RenderString(source_int.service[source_int.tv_index].name, (720-MENU_WIDTH)/2, MENU_START+110, MENU_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);
    RenderString(msg[language][2], (720-MENU_WIDTH)/2, MENU_START+135, MENU_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);
    RenderString(msg[language][tmp_pip_source + 3], (720-MENU_WIDTH)/2, MENU_START+155, MENU_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);

    source->pip_index_tmp = source->pip_index;
    source->tv_index_tmp = source->tv_index;

    do
    {
	Key = GetRCCode();

	switch(Key)
	{
	    case RC_UP:

		if(selected_entry)
		{
		    selected_entry--;

		    switch(selected_entry)
		    {
			case 0:

			    RenderString(source->service[source->pip_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+65, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
			    RenderString(source_int.service[source_int.tv_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+110, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);

			    break;

			case 1:

			    RenderString(source_int.service[source_int.tv_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+110, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
			    RenderString(msg[language][tmp_pip_source + 3], (720-MENU_WIDTH)/2 + 1, MENU_START+155, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);

			    break;
		    }
		}
		else
		{
		    selected_entry = 2;

		    RenderString(source->service[source->pip_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+65, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);
		    RenderString(msg[language][tmp_pip_source + 3], (720-MENU_WIDTH)/2 + 1, MENU_START+155, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
		}

		break;

	    case RC_DOWN:

		if(selected_entry < 2)
		{
		    selected_entry++;

		    switch(selected_entry)
		    {
			case 1:

			    RenderString(source->service[source->pip_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+65, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);
			    RenderString(source_int.service[source_int.tv_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+110, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);

			    break;

			case 2:

			    RenderString(source_int.service[source_int.tv_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+110, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);
			    RenderString(msg[language][tmp_pip_source + 3], (720-MENU_WIDTH)/2 + 1, MENU_START+155, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);

			    break;
		    }
		}
		else
		{
		    selected_entry = 0;

		    RenderString(source->service[source->pip_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+65, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
		    RenderString(msg[language][tmp_pip_source + 3], (720-MENU_WIDTH)/2 + 1, MENU_START+155, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);
		}

		break;

	    case RC_LEFT:

		switch(selected_entry)
		{
		    case 0:

			if(source->pip_index_tmp)
			{
			    source->pip_index_tmp--;
			}
			else
			{
			    source->pip_index_tmp = source->services - 1;
			}

			RenderString(source->service[source->pip_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+65, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
			
			break;

		    case 1:

			if(source_int.tv_index_tmp)
			{
			    source_int.tv_index_tmp--;
			}
			else
			{
			    source_int.tv_index_tmp = source_int.services - 1;
			}

			RenderString(source_int.service[source_int.tv_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+110, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);

			break;

		    case 2:

			if(source_ext.valid)
			{
			    tmp_pip_source ^= 1;

			    RenderString(msg[language][tmp_pip_source + 3], (720-MENU_WIDTH)/2 + 1, MENU_START+155, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
			}

			break;
		}

		break;

	    case RC_RIGHT:

		switch(selected_entry)
		{
		    case 0:

			if(source->pip_index_tmp < source->services - 1)
			{
			    source->pip_index_tmp++;
			}
			else
			{
			    source->pip_index_tmp = 0;
			}

			RenderString(source->service[source->pip_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+65, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
			
			break;

		    case 1:

			if(source_int.tv_index_tmp < source_int.services - 1)
			{
			    source_int.tv_index_tmp++;
			}
			else
			{
			    source_int.tv_index_tmp = 0;
			}

			RenderString(source_int.service[source_int.tv_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+110, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);

			break;

		    case 2:

			if(source_ext.valid)
			{
			    tmp_pip_source ^= 1;

			    RenderString(msg[language][tmp_pip_source + 3], (720-MENU_WIDTH)/2 + 1, MENU_START+155, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
			}

			break;
		}

		break;

	    case RC_OK:

		switch(selected_entry)
		{
		    case 0:

			if(tuner_changed || source->pip_index != source->pip_index_tmp)
			{
			    close_thread = TRUE;

			    pthread_join(thread_id, NULL);

			    source->pip_index = source->pip_index_tmp;

			    if(pthread_create(&thread_id, NULL, DecodeThread, NULL))
			    {
			        Log2File("pthread_create() failed!");
			    }
			}
			else
			{
			    Key = 0xDEAD;
			}

			break;

		    case 1:

			if(source_int.tv_index != source_int.tv_index_tmp)
			{
			    source_int.tv_index = source_int.tv_index_tmp;

			    ioctl(dmx, DMX_STOP);

			    WebServerRequest(SET_CHANNEL, source_int.service[source_int.tv_index].ref, 0);

			    ioctl(dmx, DMX_START);
			}
			else
			{
			    Key = 0xDEAD;
			}

			break;

		    case 2:

			if(pip_source != tmp_pip_source)
			{
			    tuner_changed ^= 1;

			    pip_source = tmp_pip_source;
			    source = pip_source ? &source_ext : &source_int;

			    selected_entry = 0;

			    RenderString(source->service[source->pip_index_tmp].name, (720-MENU_WIDTH)/2 + 1, MENU_START+65, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
			    RenderString(msg[language][tmp_pip_source + 3], (720-MENU_WIDTH)/2 + 1, MENU_START+155, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);
			}
			else
			{
			    Key = 0xDEAD;
			}

			Key = 0xDEAD;

			break;
		}

		break;
	}
    }
    while(Key != RC_EXIT && Key != RC_OK && Key != RC_MENU);

    if(tuner_changed && (Key != RC_OK || (Key == RC_OK && selected_entry)))
    {
	pip_source ^= 1;

	source = pip_source ? &source_ext : &source_int;
    }

    SetPalette(FALSE, TRUE);

    if(Key != RC_OK || selected_entry)
    {
	DrawPiP(TRUE);
    }

    lock_framebuffer = FALSE;

    Key = 0xDEAD;
}

/***** ShowMessage  ***********************************************************/

void ShowMessage(int message)
{
#define MSGBOX_WIDTH 250
#define MSGBOX_START 233

    int y;

    char *title[] = {"Program Information", "Programminformation"};

    char *msg[][8] =
    {
	{"Box-Type unknown!",	"Access to WebServer failed!",			"Check Network / Port",		"Access to WebServer denied!",		"Check User / Password",		"No Transponder-Services found!",	"Decode-Thread failed!",		"PiP-Plugin Version "VERSION" / "BUILD},
	{"Box-Typ unbekannt!",	"Zugriff auf WebServer fehlgeschlagen!",	"Netzwerk / Port überprüfen",	"Zugriff auf WebServer verweigert!",	"Benutzer / Passwort überprüfen",	"Keine Transponder-Kanäle gefunden!",	"Decoder-Thread fehlgeschlagen!",	"PiP-Plugin Version "VERSION" / "BUILD}
    };

    lock_framebuffer = TRUE;
    usleep(100000);

    SetPalette(TRUE, TRUE);
	if (var_screeninfo.yoffset)
	{
		var_screeninfo.yoffset= 0;
		if (ioctl(fb, FBIOPAN_DISPLAY, &var_screeninfo)  == -1)
			printf("ioctl() \"FBIOPAN_DISPLAY\" failed: %s", strerror(errno));
	}

    for(y = 0; y < 25; y++)
    {
	memset(lfb + (720-MSGBOX_WIDTH)/2 + (MSGBOX_START+y)*720, MENUCOLOR1, MSGBOX_WIDTH);

	*(lfb + (720-MSGBOX_WIDTH)/2 + (MSGBOX_START+y)*720) = MENUCOLOR3;
	*(lfb + (720-MSGBOX_WIDTH)/2 + MSGBOX_WIDTH-1 + (MSGBOX_START+y)*720) = MENUCOLOR3;
    }

    for(y = 0; y < 85; y++)
    {
	memset(lfb + (720-MSGBOX_WIDTH)/2 + (MSGBOX_START+25+y)*720, MENUCOLOR2, MSGBOX_WIDTH);

	*(lfb + (720-MSGBOX_WIDTH)/2 + (MSGBOX_START+25+y)*720) = MENUCOLOR3;
	*(lfb + (720-MSGBOX_WIDTH)/2 + MSGBOX_WIDTH-1 + (MSGBOX_START+25+y)*720) = MENUCOLOR3;
    }

    for(y = 0; y < 19; y++)
    {
	memset(lfb + (720-30)/2 + (MSGBOX_START+85+y)*720, MENUCOLOR1, 30);
    }

    memset(lfb + (720-MSGBOX_WIDTH)/2 + (MSGBOX_START-1)*720, MENUCOLOR3, MSGBOX_WIDTH);
    memset(lfb + (720-MSGBOX_WIDTH)/2 + MSGBOX_START*720, MENUCOLOR3, MSGBOX_WIDTH);
    memset(lfb + (720-MSGBOX_WIDTH)/2 + (MSGBOX_START+25)*720, MENUCOLOR3, MSGBOX_WIDTH);
    memset(lfb + (720-MSGBOX_WIDTH)/2 + (MSGBOX_START+110)*720, MENUCOLOR3, MSGBOX_WIDTH);
    memset(lfb + (720-MSGBOX_WIDTH)/2 + (MSGBOX_START+111)*720, MENUCOLOR3, MSGBOX_WIDTH);

    RenderString(title[language], (720-MSGBOX_WIDTH)/2, MSGBOX_START+18, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C2, FALSE);

    switch(message)
    {
	case BOXTYPE:

	    RenderString(msg[language][0], (720-MSGBOX_WIDTH)/2, MSGBOX_START+60, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);

	    break;

	case NETWORK:

	    RenderString(msg[language][1], (720-MSGBOX_WIDTH)/2, MSGBOX_START+50, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);
	    RenderString(msg[language][2], (720-MSGBOX_WIDTH)/2, MSGBOX_START+75, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);

	    break;

	case DENIED:

	    RenderString(msg[language][3], (720-MSGBOX_WIDTH)/2, MSGBOX_START+50, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);
	    RenderString(msg[language][4], (720-MSGBOX_WIDTH)/2, MSGBOX_START+75, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);

	    break;

	case NOSERVICE:

	    RenderString(msg[language][5], (720-MSGBOX_WIDTH)/2, MSGBOX_START+60, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);

	    break;

	case THREAD:

	    RenderString(msg[language][6], (720-MSGBOX_WIDTH)/2, MSGBOX_START+60, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);

	    break;

	case ABOUT:

	    RenderString(msg[language][7], (720-MSGBOX_WIDTH)/2, MSGBOX_START+50, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);
	    RenderString("(c) Thomas \"LazyT\" Löwe", (720-MSGBOX_WIDTH)/2, MSGBOX_START+75, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C1, FALSE);

	    break;
    }

    RenderString("OK", (720-MSGBOX_WIDTH)/2, MSGBOX_START+100, MSGBOX_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);

    do
    {
	Key = GetRCCode();
    }
    while(Key != RC_EXIT && Key != RC_OK && Key != RC_INFO);

    if(message == ABOUT)
    {
	SetPalette(FALSE, TRUE);

	DrawPiP(TRUE);
    }

    lock_framebuffer = FALSE;

    Key = 0xDEAD;
}

/***** ShowHelp ***************************************************************/

void ShowHelp()
{
#define HELP_WIDTH 410
#define HELP_START 90

    int y;

    char *title[] = {"Help", "Hilfe"};

    char *msg[][20] =
    {
	{"PiP-Position :",	"PiP-Offsets :",	"reset Offsets :",		"PiP-Channel :",	"TV-Channel :",	"swap internal PiP with TV :",		"Mute on/off :",		"Volume :",	"PiP-Size :",	"Transparency on/off :",	"P-Frames on/off :",	"Plugin-Information :",	"Channel-Menu :",	"Exit :",	"CURSOR UP/DOWN",	"CURSOR LEFT/RIGHT",	"RED",	"BLUE",	"GREEN/YELLOW",	"complete Documentation available at"},
	{"PiP-Position :",	"PiP-Verschiebung :",	"Verschiebung rï¿½cksetzen :",	"PiP-Kanal :",		"TV-Kanal :",	"internes PiP mit TV vertauschen :",	"Stummschaltung ein/aus :",	"Lautstï¿½rke :",	"PiP-Grï¿½ï¿½e :",	"Transparenz ein/aus :",	"P-Frames ein/aus :",	"Plugin-Information :",	"Kanal-Menï¿½ :",		"Ende :",	"CURSOR HOCH/RUNTER",	"CURSOR LINKS/RECHTS",	"ROT",	"BLAU",	"GRï¿½N/GELB",	"komplette Dokumentation verfï¿½gbar unter"}
    };

    lock_framebuffer = TRUE;
    usleep(100000);

    SetPalette(TRUE, TRUE);
	if (var_screeninfo.yoffset)
	{
		var_screeninfo.yoffset= 0;
		if (ioctl(fb, FBIOPAN_DISPLAY, &var_screeninfo)  == -1)
			printf("ioctl() \"FBIOPAN_DISPLAY\" failed: %s", strerror(errno));
	}

    for(y = 0; y < 25; y++)
    {
	memset(lfb + (720-HELP_WIDTH)/2 + (HELP_START+y)*720, MENUCOLOR1, HELP_WIDTH);

	*(lfb + (720-HELP_WIDTH)/2 + (HELP_START+y)*720) = MENUCOLOR3;
	*(lfb + (720-HELP_WIDTH)/2 + HELP_WIDTH-1 + (HELP_START+y)*720) = MENUCOLOR3;
    }

    for(y = 0; y < 370; y++)
    {
	memset(lfb + (720-HELP_WIDTH)/2 + (HELP_START+25+y)*720, MENUCOLOR2, HELP_WIDTH);

	*(lfb + (720-HELP_WIDTH)/2 + (HELP_START+25+y)*720) = MENUCOLOR3;
	*(lfb + (720-HELP_WIDTH)/2 + HELP_WIDTH-1 + (HELP_START+25+y)*720) = MENUCOLOR3;
    }

    for(y = 0; y < 19; y++)
    {
	memset(lfb + (720-30)/2 + (HELP_START+365+y)*720, MENUCOLOR1, 30);
    }

    memset(lfb + (720-HELP_WIDTH)/2 + (HELP_START-1)*720, MENUCOLOR3, HELP_WIDTH);
    memset(lfb + (720-HELP_WIDTH)/2 + HELP_START*720, MENUCOLOR3, HELP_WIDTH);
    memset(lfb + (720-HELP_WIDTH)/2 + (HELP_START+25)*720, MENUCOLOR3, HELP_WIDTH);
    memset(lfb + (720-HELP_WIDTH)/2 + (HELP_START+395)*720, MENUCOLOR3, HELP_WIDTH);
    memset(lfb + (720-HELP_WIDTH)/2 + (HELP_START+396)*720, MENUCOLOR3, HELP_WIDTH);

    RenderString(title[language], (720-HELP_WIDTH)/2, HELP_START+18, HELP_WIDTH, MIDDLE, FONTCOLOR_C2, FALSE);

    RenderString(msg[language][0], (720-HELP_WIDTH)/2, HELP_START+50, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("1 3 5 7 9", 720/2+4, HELP_START+50, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][1], (720-HELP_WIDTH)/2, HELP_START+70, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("2 4 6 8", 720/2+4, HELP_START+70, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][2], (720-HELP_WIDTH)/2, HELP_START+90, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("0", 720/2+4, HELP_START+90, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][3], (720-HELP_WIDTH)/2, HELP_START+110, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString(msg[language][4], (720-HELP_WIDTH)/2, HELP_START+130, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);

    if(BoxType == DM7000 || BoxType == DM7020)
    {
	RenderString(swap_cursor ? msg[language][15] : msg[language][14], 720/2+4, HELP_START+110, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);
	RenderString(swap_cursor ? msg[language][14] : msg[language][15], 720/2+4, HELP_START+130, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);
    }
    else
    {
	RenderString((char*)(swap_cursor ? "VOLUME +/-" : msg[language][14]), 720/2+4, HELP_START+110, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);
	RenderString((char*)(swap_cursor ? msg[language][14] : "VOLUME +/-"), 720/2+4, HELP_START+130, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);
    }

    RenderString(msg[language][5], (720-HELP_WIDTH)/2, HELP_START+150, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("OK", 720/2+4, HELP_START+150, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][6], (720-HELP_WIDTH)/2, HELP_START+170, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("MUTE", 720/2+4, HELP_START+170, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][7], (720-HELP_WIDTH)/2, HELP_START+190, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString((char*)(BoxType == DM7000 || BoxType == DM7020 ? "VOLUME +/-" : msg[language][18]), 720/2+4, HELP_START+190, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][8], (720-HELP_WIDTH)/2, HELP_START+210, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString((char*)(BoxType == DM7000 || BoxType == DM7020 ? "BOUQUET +/-" : "</>"), 720/2+4, HELP_START+210, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][9], (720-HELP_WIDTH)/2, HELP_START+230, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString(msg[language][16], 720/2+4, HELP_START+230, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][10], (720-HELP_WIDTH)/2, HELP_START+250, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString(msg[language][17], 720/2+4, HELP_START+250, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][11], (720-HELP_WIDTH)/2, HELP_START+270, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("INFO", 720/2+4, HELP_START+270, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][12], (720-HELP_WIDTH)/2, HELP_START+290, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("MENU", 720/2+4, HELP_START+290, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][13], (720-HELP_WIDTH)/2, HELP_START+310, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("EXIT/LAME", 720/2+4, HELP_START+310, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][19], (720-HELP_WIDTH)/2, HELP_START+335, HELP_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);
    RenderString("www.dream-multimedia-tv.de/board/thread.php?threadid=133", (720-HELP_WIDTH)/2, HELP_START+355, HELP_WIDTH, MIDDLE, FONTCOLOR_C2, FALSE);

    RenderString("OK", (720-HELP_WIDTH)/2, HELP_START+380, HELP_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);

    do
    {
	Key = GetRCCode();
    }
    while(Key != RC_EXIT && Key != RC_OK && Key != RC_HELP);

    SetPalette(FALSE, TRUE);

    DrawPiP(TRUE);

    lock_framebuffer = FALSE;

    Key = 0xDEAD;
}

/***** SaveConfig  ************************************************************/

void SaveConfig()
{
    if((fd_conf = fopen(CFGFILE, "w")))
    {
	fprintf(fd_conf, "SIZE=%d\n\n", PiPSize);

	fprintf(fd_conf, "POSITION=%d\n", PiPPosition);
	fprintf(fd_conf, "OFFSET_X=%d\n", xofs);
	fprintf(fd_conf, "OFFSET_Y=%d\n\n", yofs);

	fprintf(fd_conf, "TRANSPARENCY=%d\n\n", transparency);

	fprintf(fd_conf, "SWAPCURSOR=%d\n\n", swap_cursor);

	fprintf(fd_conf, "WEBPORT_INT=%d\n", source_int.port);
	fprintf(fd_conf, "WEBUSER_INT=%s\n", source_int.user);
	fprintf(fd_conf, "WEBPASS_INT=%s\n\n", source_int.pass);

	fprintf(fd_conf, "WEBADDR_EXT=%s\n", source_ext.addr);
	fprintf(fd_conf, "WEBPORT_EXT=%d\n", source_ext.port);
	fprintf(fd_conf, "WEBUSER_EXT=%s\n", source_ext.user);
	fprintf(fd_conf, "WEBPASS_EXT=%s\n\n", source_ext.pass);

	fprintf(fd_conf, "LANGUAGE=%d\n\n", language);

	fprintf(fd_conf, "DEBUG=%d\n\n", debug);

	fprintf(fd_conf, "DMX=%d\n", demuxer);
	fprintf(fd_conf, "DMXBUFFER=%d\n\n", DMXBufferSize);

	fprintf(fd_conf, "SKIP_P_FRAME=%d\n\n", skip_p_frame);

	fprintf(fd_conf, "PMT=%d\n", pmt_pip);

	fclose(fd_conf);
    }
}

/***** CleanUp ****************************************************************/

void CleanUp()
{
    if(LogFile)
    {
        Log2File("PiP-Plugin "VERSION" by LazyT cleaning up");

	fclose(LogFile);
    }

    // framebuffer

	if (var_screeninfo.yoffset)
	{
		var_screeninfo.yoffset = 0;

		ioctl(fb, FBIOPAN_DISPLAY, &var_screeninfo);
	}
        if(fb != -1)
        {
	    close(fb);
	}

	if(lfb != MAP_FAILED)
	{
	    munmap(lfb, fix_screeninfo.smem_len);
	}

    // freetype

	if(manager)
	{
	    FTC_Manager_Done(manager);
	}

	if(library)
	{
	    FT_Done_FreeType(library);
	}

    // remote control

	if(rc != -1)
	{
	    close(rc);
	}

    // delete pmt.pip

	unlink(PMTFILE);
}

/***** DecodeThread  **********************************************************/

void *DecodeThread(void *arg)
{
    struct dmxPesFilterParams dmx_flt;
    struct timeval tv;
    fd_set readfd;
    const mpeg2_info_t *info = NULL;
    mpeg2dec_t *decoder= NULL;
//#define ALIGN_16(p) ((void *)(((uintptr_t)(p) + 15) & ~((uintptr_t)15)))
    uint8_t buffer[INBUF_SIZE];
    size_t bytes;
    int stream_x = 0, stream_y = 0;
    bool skip_frame = FALSE;
    int scale_720[360], scale_704[360], scale_640[360], scale_544[360], scale_528[360], scale_480[360], scale_352[360];
    int frame_width, s1, s2, s3, s4, s5, s6, s7;
    bool is_internal;

    // init

	Log2File("Decode-Thread for \"%s\" on PID 0x%.4X starting", source->service[source->pip_index].name, (void*)(source->service[source->pip_index].pid));

	if(!pip_source)
	{
	    SetPMT(source_int.pip_index);
	}

	close_thread = FALSE;
	scaler_reset = TRUE;
	lock_framebuffer = FALSE;
	scramble_check = 0;
	dmx = 0;
	is_internal = pip_source ? FALSE : TRUE;
	PiPIcon = source->service[source->pip_index].pid == -1 ? UNAVAILABLE : NOICON;

        DrawPiP(FALSE);

	if(source->service[source->pip_index].pid == -1)
	{
	    Log2File("not available...");

	    goto cleanup;
	}

    // libmpeg2

	if(!(decoder = mpeg2_init()))
	{
	    Log2File("mpeg2_init() failed!\n");

	    goto cleanup;
	}

	if(!(info = mpeg2_info(decoder)))
	{
	    Log2File("mpeg2_info() failed!\n");

	    goto cleanup;
	}

    // demuxer or streampes

if(!pip_source)
{
	if((dmx = open(demuxer ? DMX1 : DMX0, O_RDONLY | O_NONBLOCK)) == -1)
	{
	    Log2File((char*)(demuxer ? "open() \""DMX1"\" failed: %s\n" : "open() \""DMX0"\" failed: %s\n"), strerror(errno));

	    goto cleanup;
	}

	if(ioctl(dmx, DMX_SET_BUFFER_SIZE, DMXBufferSize*1024) == -1)
	{
	    Log2File("ioctl() \"DMX_SET_BUFFER_SIZE\" failed: %s\n", strerror(errno));
	}
	else
	{
	    Log2File("Demuxer = %s with %dKB Buffer", (char*)(demuxer ? DMX1 : DMX0), (void*)DMXBufferSize);
	}

	dmx_flt.pid = source_int.service[source_int.pip_index].pid;
	dmx_flt.input = DMX_IN_FRONTEND;
	dmx_flt.output = DMX_OUT_TAP;
	dmx_flt.pesType = DMX_PES_OTHER;
	dmx_flt.flags = DMX_IMMEDIATE_START;

	if(ioctl(dmx, DMX_SET_PES_FILTER, &dmx_flt) == -1)
	{
	    Log2File("ioctl() \"DMX_SET_PES_FILTER\" failed: %s", strerror(errno));

	    goto cleanup;
	}
}
else
{
	struct sockaddr_in websrv_data;

	if((dmx = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
	    Log2File("socket() failed: %s\n", strerror(errno));

	    goto cleanup;
	}

	websrv_data.sin_family = AF_INET;
	websrv_data.sin_addr.s_addr = inet_addr(source_ext.addr);
	websrv_data.sin_port = htons(31338);

	if(connect(dmx, (sockaddr*)&websrv_data, sizeof(sockaddr)) == -1)
	{
	    Log2File("connect() failed: %s\n", strerror(errno));

	    goto cleanup;
	}

	snprintf((char*)buffer, sizeof(buffer), "GET /%.4X\n\n", source_ext.service[source_ext.pip_index].pid);

	if(send(dmx, buffer, strlen((char*)buffer), 0) == -1)
	{
	    Log2File("send() failed: %s\n", strerror(errno));

	    goto cleanup;
	}
}

    // decoder loop
	while(!close_thread)
	{
	    if(scaler_reset)
	    {
		scaler_reset = FALSE;

		frame_width = 720/PiPSize;

		s1 = s2 = s3 = s4 = s5 = s6 = s7 = 0;

		for(int i = 0; i < frame_width; s1 += 7200/frame_width, s2 += 7040/frame_width, s3 += 6400/frame_width, s4 += 5440/frame_width, s5 += 5280/frame_width, s6 += 4800/frame_width, s7 += 3520/frame_width, i++)
		{
		    scale_720[i] = (int)(s1 + 5)/10;
		    scale_704[i] = (int)(s2 + 5)/10;
		    scale_640[i] = (int)(s3 + 5)/10;
		    scale_544[i] = (int)(s4 + 5)/10;
		    scale_528[i] = (int)(s5 + 5)/10;
		    scale_480[i] = (int)(s6 + 5)/10;
		    scale_352[i] = (int)(s7 + 5)/10;
		}
	    }

	    switch(mpeg2_parse(decoder))
	    {
		case STATE_BUFFER:
//Log2File("STATE_BUFFER");

		    tv.tv_sec = 2;
		    tv.tv_usec = 0;

		    FD_ZERO(&readfd);
		    FD_SET(dmx, &readfd);

		    select(dmx + 1, &readfd, NULL, NULL, &tv);

		    if(!FD_ISSET(dmx, &readfd))
		    {
			PiPIcon = TIMEOUT;
			DrawPiP(FALSE);

			Log2File("timed out...");

			close_thread = TRUE;

			break;
		    }

		    bytes = read(dmx, buffer, sizeof(buffer));

		    mpeg2_buffer(decoder, buffer, buffer + bytes);

		    scramble_check += bytes;

		    if(scramble_check > INBUF_SIZE*50)
		    {
			PiPIcon = SCRAMBLED;
			DrawPiP(FALSE);

			Log2File("can't decode...");

			close_thread = TRUE;
		    }

		    break;

		case STATE_PICTURE:
//Log2File("STATE_PICTURE");

		    scramble_check = 0;

		    switch(info->current_picture->flags & PIC_MASK_CODING_TYPE)
		    {
			case PIC_FLAG_CODING_TYPE_B:

			    skip_frame = TRUE;

			    break;

			case PIC_FLAG_CODING_TYPE_P:

			    skip_frame = skip_p_frame;

			    break;

			case PIC_FLAG_CODING_TYPE_I:

			    skip_frame = FALSE;

			    break;
		    }

		    mpeg2_skip(decoder, skip_frame);

		    break;

		case STATE_SEQUENCE:
//Log2File("STATE_SEQUENCE");

		    stream_x = info->sequence->width;

		    stream_y = info->sequence->height;

		    break;

		case STATE_SLICE:
//Log2File("STATE_SLICE");

		    if(info->display_fbuf && !skip_frame)
		    {
			int x, y;
			int *x_scaler;
			int y_scaler;

			switch(stream_x)
			{
			    case 720:

				x_scaler = scale_720;

				break;

			    case 704:

				x_scaler = scale_704;

				break;

			    case 640:

				x_scaler = scale_640;

				break;

			    case 544:

				x_scaler = scale_544;

				break;

			    case 528:

				x_scaler = scale_528;

				break;

			    case 480:

				x_scaler = scale_480;

				break;

			    case 352:

				x_scaler = scale_352;

				break;

			    default:

				Log2File("Streamtype %dx%d unsupported, please report!", (void*)stream_x, (void*)stream_y);

				close_thread = TRUE;

				continue;
			}

			switch(stream_y)
			{
			    case 576:

				y_scaler = 1;

				break;

			    case 288:

				y_scaler = 2;

				break;

			    default:

				Log2File("Streamtype %dx%d unsupported, please report!", (void*)stream_x, (void*)stream_y);

				close_thread = TRUE;

				continue;
			}

			if(!lock_framebuffer)
			{

				int xmax = 720/PiPSize-1;
				int ymax = stream_y/PiPSize*y_scaler-1;
				int yfactor =PiPSize/y_scaler*stream_x;
			    unsigned char* bufp = info->display_fbuf->buf[0];
			    unsigned char* pfb = lfb + (sy+yofs)*720+sx+xofs + ((var_screeninfo.yres - var_screeninfo.yoffset) * 720);
/*
			    for(y = 0; y < stream_y/PiPSize*y_scaler; y++)
			    {
				for(x = 0; x < 720/PiPSize; x++)
				{
				    *(lfb + sx+xofs+x + (sy+yofs+y)*720) = *(info->display_fbuf->buf[0] + x_scaler[x] + y*PiPSize/y_scaler*stream_x) ? *(info->display_fbuf->buf[0] + x_scaler[x] + y*PiPSize/y_scaler*stream_x) : 0x01;
				}
			    }
*/
			    for(y = ymax; --y;)
			    {
				bufp += yfactor;
				for(x = xmax; --x;)
				{
				    *(pfb +x) = *(bufp + x_scaler[x]) ? *(bufp + x_scaler[x]) : 0x01;
				}
				pfb += 720;
			    }
				var_screeninfo.yoffset= var_screeninfo.yres - var_screeninfo.yoffset;
				if (ioctl(fb, FBIOPAN_DISPLAY, &var_screeninfo)  == -1)
					printf("ioctl() \"FBIOPAN_DISPLAY\" failed: %s", strerror(errno));
			}
		    }

		    break;

		case STATE_SEQUENCE_REPEATED:
//Log2File("STATE_SEQUENCE_REPEATED");

		    break;

		case STATE_GOP:
//Log2File("STATE_GOP");

		    break;

		case STATE_SLICE_1ST:
//Log2File("STATE_SLICE_1ST");

		    break;

		case STATE_PICTURE_2ND:
//Log2File("STATE_PICTURE_2ND");

		    break;

		case STATE_END:
//Log2File("STATE_END");

		    break;

		case STATE_INVALID:
//Log2File("STATE_INVALID");

		    break;

		case STATE_INVALID_END:
//Log2File("STATE_INVALID_END");

		    break;
	    }
	}

    // cleanup

cleanup:

	if(dmx)
	{
	    if(is_internal)
	    {
		if(ioctl(dmx, DMX_STOP) == -1)
		{
    		    Log2File("ioctl() \"DMX_STOP\" failed: %s", strerror(errno));
		}
	    }

	    close(dmx);
	}

	if(decoder)
	{
	    mpeg2_close(decoder);
	}

    Log2File("Decode-Thread closing\n");

    return 0;
}

/***** SwitchTV ***************************************************************/

void SwitchTV(bool prev_channel)
{
    if(prev_channel)
    {
	if(source_int.tv_index > 0)
	{
	    source_int.tv_index--;
	}
	else
	{
	    source_int.tv_index = source_int.services - 1;
	}
    }
    else
    {
	if(source_int.tv_index < source_int.services - 1)
	{
	    source_int.tv_index++;
	}
	else
	{
	    source_int.tv_index = 0;
	}
    }

    ioctl(dmx, DMX_STOP);

    WebServerRequest(SET_CHANNEL, source_int.service[source_int.tv_index].ref, 0);

    ioctl(dmx, DMX_START);

}

/***** SwitchPiP **************************************************************/

void SwitchPiP(bool prev_channel)
{
    close_thread = TRUE;

    pthread_join(thread_id, NULL);

    if(prev_channel)
    {
	if(source->pip_index)
	{
	    source->pip_index--;
	}
	else
	{
	    source->pip_index = source->services - 1;
	}
    }
    else
    {
	if(source->pip_index < source->services - 1)
	{
	    source->pip_index++;
	}
	else
	{
	    source->pip_index = 0;
	}
    }

    if(pthread_create(&thread_id, NULL, DecodeThread, NULL))
    {
	Log2File("pthread_create() failed!");
    }
}

/***** plugin_exec ************************************************************/

void plugin_exec(PluginParam *Parameter)
{
    bool rezap = FALSE;
    int max_retries;
    int old_pipsize, old_pippos, old_xofs, old_yofs, old_transparency, old_skip_p_frame;
    int i;

    // read config

	strncpy(source_int.addr, "127.0.0.1", sizeof(source_int.addr));
	strncpy(source_int.user, "root", sizeof(source_int.user));
	strncpy(source_int.pass, "dreambox", sizeof(source_int.pass));
	source_int.port = 80;

	strncpy(source_ext.addr, "?.?.?.?", sizeof(source_ext.addr));
	strncpy(source_ext.user, "root", sizeof(source_ext.user));
	strncpy(source_ext.pass, "dreambox", sizeof(source_ext.pass));
	source_ext.port = 80;

	if((fd_conf = fopen(CFGFILE, "r")))
	{
	    while(fgets(TempBuffer, sizeof(TempBuffer), fd_conf))
	    {
		if(strstr(TempBuffer, "SIZE="))
		{
		    sscanf(TempBuffer + 5, "%d", &PiPSize);

		    if(PiPSize < HALF || PiPSize > QUAD)
		    {
			PiPSize = THIRD;
		    }
		}
		else if(strstr(TempBuffer, "POSITION="))
		{
		    sscanf(TempBuffer + 9, "%d", &PiPPosition);

		    if(PiPPosition != LEFT_TOP && PiPPosition != RIGHT_TOP && PiPPosition != CENTER && PiPPosition != LEFT_BOTTOM && PiPPosition != RIGHT_BOTTOM)
		    {
			PiPPosition = RIGHT_TOP;
		    }
		}
		else if(strstr(TempBuffer, "OFFSET_X="))
		{
		    sscanf(TempBuffer + 9, "%d", &xofs);

		    if(xofs < -50 || xofs > 50)
		    {
			xofs = 0;
		    }
		}
		else if(strstr(TempBuffer, "OFFSET_Y="))
		{
		    sscanf(TempBuffer + 9, "%d", &yofs);

		    if(yofs < -50 || yofs > 50)
		    {
			yofs = 0;
		    }
		}
		else if(strstr(TempBuffer, "TRANSPARENCY="))
		{
		    sscanf(TempBuffer + 13, "%d", &transparency);

		    if(transparency < 0 || transparency > 1)
		    {
			transparency = 0;
		    }
		}
		else if(strstr(TempBuffer, "SWAPCURSOR="))
		{
		    sscanf(TempBuffer + 11, "%d", &swap_cursor);

		    if(swap_cursor < 0 || swap_cursor > 1)
		    {
			swap_cursor = 0;
		    }
		}
		else if(strstr(TempBuffer, "WEBPORT_INT="))
		{
		    sscanf(TempBuffer + 12, "%d", &source_int.port);

		    if(source_int.port < 1 || source_int.port > 65535)
		    {
			source_int.port = 80;
		    }
		}
		else if(strstr(TempBuffer, "WEBUSER_INT="))
		{
		    sscanf(TempBuffer + 12, "%15s", source_int.user);
		}
		else if(strstr(TempBuffer, "WEBPASS_INT="))
		{
		    sscanf(TempBuffer + 12, "%15s", source_int.pass);
		}
		else if(strstr(TempBuffer, "WEBADDR_EXT="))
		{
		    sscanf(TempBuffer + 12, "%15s", source_ext.addr);

		    if(inet_addr(source_ext.addr) == INADDR_NONE)
		    {
			strncpy(source_ext.addr, "?.?.?.?", 16);
		    }
		    else
		    {
			source_ext.valid = TRUE;
		    }
		}
		else if(strstr(TempBuffer, "WEBPORT_EXT="))
		{
		    sscanf(TempBuffer + 12, "%d", &source_ext.port);

		    if(source_ext.port < 1 || source_ext.port > 65535)
		    {
			source_ext.port = 80;
		    }
		}
		else if(strstr(TempBuffer, "WEBUSER_EXT="))
		{
		    sscanf(TempBuffer + 12, "%15s", source_ext.user);
		}
		else if(strstr(TempBuffer, "WEBPASS_EXT="))
		{
		    sscanf(TempBuffer + 12, "%15s", source_ext.pass);
		}
		else if(strstr(TempBuffer, "LANGUAGE="))
		{
		    sscanf(TempBuffer + 9, "%d", &language);

		    if(language < 0 || language > 1)
		    {
			language = 0;
		    }
		}
		else if(strstr(TempBuffer, "DEBUG="))
		{
		    sscanf(TempBuffer + 6, "%d", &debug);

		    if(debug < 0 || debug > 1)
		    {
			debug = 0;
		    }
		}
		else if(strstr(TempBuffer, "DMX="))
		{
		    sscanf(TempBuffer + 4, "%d", &demuxer);

		    if(demuxer < 0 || demuxer > 1)
		    {
			demuxer = 0;
		    }
		}
		else if(strstr(TempBuffer, "DMXBUFFER="))
		{
		    sscanf(TempBuffer + 10, "%d", &DMXBufferSize);

		    if(DMXBufferSize < 512 || DMXBufferSize > 1024*10)
		    {
			DMXBufferSize = 1024;
		    }
		}
		else if(strstr(TempBuffer, "SKIP_P_FRAME="))
		{
		    sscanf(TempBuffer + 13, "%d", &skip_p_frame);

		    if(skip_p_frame < 0 || skip_p_frame > 1)
		    {
			skip_p_frame = 0;
		    }
		}
		else if(strstr(TempBuffer, "PMT="))
		{
		    sscanf(TempBuffer + 4, "%d", &pmt_pip);

		    if(pmt_pip < 0 || pmt_pip > 1)
		    {
			pmt_pip = 0;
		    }
		}
	    }

	    fclose(fd_conf);
	}
	else
	{
	    SaveConfig();
	}

	old_pipsize = PiPSize;
	old_pippos = PiPPosition;
	old_xofs = xofs;
	old_yofs = yofs;
	old_transparency = transparency;
	old_skip_p_frame = skip_p_frame;

    // debug

        if(debug)
	{
	    LogFile = fopen(LOGFILE, "w");
	}

	Log2File("PiP-Plugin "VERSION" by LazyT starting up\n");

    // exit handler

        atexit(CleanUp);

    // framebuffer

	if((fb = open(FB, O_RDWR)) == -1)
	{
		Log2File("open() \""FB"\" failed: %s\n", strerror(errno));

		return;
	}

	if(ioctl(fb, FBIOGET_FSCREENINFO, &fix_screeninfo) == -1)
	{
		Log2File("ioctl() \"FBIOGET_FSCREENINFO\" failed: %s\n", strerror(errno));

		return;
	}

	/* get variable screeninfo */
	if (ioctl(fb, FBIOGET_VSCREENINFO, &var_screeninfo) == -1)
	{
		Log2File("ioctl() \"FBIOGET_VSCREENINFO\" failed: %s\n", strerror(errno));

		return;
	}

	/* set variable screeninfo for double buffering */
	var_screeninfo.yres_virtual = 2*var_screeninfo.yres;
	var_screeninfo.xres_virtual = var_screeninfo.xres;
	var_screeninfo.yoffset      = 0;

	if (ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo) == -1)
	{
		Log2File("ioctl() \"FBIOPUT_VSCREENINFO\" failed: %s\n", strerror(errno));

		return;
	}

	if((lfb = (unsigned char*)mmap(0, fix_screeninfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0)) == MAP_FAILED)
	{
		Log2File("mmap() failed: %s\n", strerror(errno));

		return;
	}

    // freetype

	if(FT_Init_FreeType(&library))
	{
	    Log2File("FT_Init_FreeType() failed\n");

	    return;
	}

	if(FTC_Manager_New(library, 1, 1, 0, &FaceRequester, NULL, &manager))
	{
	    Log2File("FTC_Manager_New() failed\n");

	    return;
	}

	if(FTC_SBitCache_New(manager, &cache))
	{
	    Log2File("FTC_SBitCache_New() failed\n");

	    return;
	}

	if(FTC_Manager_Lookup_Face(manager, (char*)FONT, &face))
	{
	    Log2File("FTC_Manager_Lookup_Face() failed\n");

	    return;
	}

#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	desc.face_id = FONT;
	desc.flags = FT_LOAD_MONOCHROME;
#else
	desc.font.face_id = (char*)FONT;
	desc.image_type = ftc_image_mono;
#endif

	use_kerning = FALSE;//FT_HAS_KERNING(face);

    // remote control

	if((rc = open(RC, O_RDONLY)) == -1)
	{
	    Log2File("open() \""RC"\" failed: %s\n", strerror(errno));

	    return;
	}

    // hw detection

	if(DetectBoxType() == FAILURE)
	{
	    ShowMessage(BOXTYPE);

	    return;
	}
/*
ShowMessage(NETWORK);
ShowMessage(DENIED);
ShowMessage(NOSERVICE);
ShowMessage(BOXTYPE);
ShowMessage(THREAD);
ShowMessage(ABOUT);
return;
*/
    // transponder check

	Log2File("Transponder-Scan (internal Tuner)...\n");

	if((web = WebServerRequest(GET_TRANSPONDERSERVICES, NULL, 0)) != NOERROR)
	{
	    switch(web)
	    {
		case NETWORK:

		    Log2File("GET_TRANSPONDERSERVICES failed: NETWORK\n");

		    ShowMessage(NETWORK);

		    break;

		case DENIED:

		    Log2File("GET_TRANSPONDERSERVICES failed: DENIED\n");

		    ShowMessage(DENIED);

		    break;

		case NOSERVICE:

		    Log2File("GET_TRANSPONDERSERVICES failed: NOSERVICE\n");

		    ShowMessage(NOSERVICE);

		    break;
	    }

	    return;
	}

	if((web = WebServerRequest(GET_STATUS, NULL, 0)) != NOERROR)
	{
	    switch(web)
	    {
		case NETWORK:

		    Log2File("GET_STATUS failed: NETWORK\n");

		    ShowMessage(NETWORK);

		    break;

		case DENIED:

		    Log2File("GET_STATUS failed: DENIED\n");

		    ShowMessage(DENIED);

		    break;
	    }

	    return;
	}

	for(i = 0; i < source_int.services; i++)
	{
	    if(source_int.service[i].pid == -1)
	    {
		rezap = TRUE;

		WebServerRequest(SET_CHANNEL, source_int.service[i].ref, 0);

		max_retries = 10;

		while(source_int.service[i].pid == -1 && max_retries)
		{
		    max_retries--;

		    usleep(100000);

		    WebServerRequest(GET_PIDS, NULL, i);
		}

		if(max_retries)
		{
		    Log2File("Video-PID 0x%.4X for \"%s\" detected", (void*)source_int.service[i].pid, source_int.service[i].name);
		}
		else
		{
		    Log2File("Video-PID for \"%s\" not available", source_int.service[i].name);
		}
	    }
	}

	if(rezap)
	{
	    WebServerRequest(SET_CHANNEL, source_int.service[source_int.watched].ref, 0);

	    Log2File("");
	}

	if(source_ext.valid)
	{
	    pip_source = 1;
	    source = &source_ext;

	    Log2File("Transponder-Scan (external Tuner)...\n");

	    if((web = WebServerRequest(GET_TRANSPONDERSERVICES, NULL, 0)) != NOERROR)
	    {
		source_ext.valid = FALSE;

		switch(web)
		{
		    case NETWORK:

			Log2File("GET_TRANSPONDERSERVICES failed: NETWORK\n");

			break;

		    case DENIED:

			Log2File("GET_TRANSPONDERSERVICES failed: DENIED\n");

			break;

		    case NOSERVICE:

			Log2File("GET_TRANSPONDERSERVICES failed: NOSERVICE\n");

			break;
		}
	    }

	    pip_source = 0;
	    source = &source_int;
	}
	else
	{
	    Log2File("Transponder-Scan (external Tuner) skipped...\n");
	}

	if(pmt_pip)
	{
	    GetPMT();
	}
	else
	{
	    Log2File("MultiChannelDecoding disabled! Set PMT=1 if your CAMD supports /tmp/pmt.pip...\n");
	}

    // decode thread

	if(pthread_create(&thread_id, NULL, DecodeThread, NULL))
	{
	    Log2File("pthread_create() failed!");

	    ShowMessage(THREAD);

	    return;
	}

    // control loop

	do
	{
	    Key = GetRCCode();

	    switch(Key)
	    {
		case RC_1:

		    PiPPosition = LEFT_TOP;

		    DrawPiP(TRUE);

		    break;

		case RC_2:

		    if(yofs >= -45)
		    {
			yofs -= 5;

			DrawPiP(TRUE);
		    }

		    break;

		case RC_3:

		    PiPPosition = RIGHT_TOP;

		    DrawPiP(TRUE);

		    break;

		case RC_4:

		    if(xofs >= -45)
		    {
			xofs -= 5;

			DrawPiP(TRUE);
		    }

		    break;

		case RC_5:

		    PiPPosition = CENTER;

		    DrawPiP(TRUE);

		    break;

		case RC_6:

		    if(xofs <= 45)
		    {
			xofs += 5;

			DrawPiP(TRUE);
		    }

		    break;

		case RC_7:

		    PiPPosition = LEFT_BOTTOM;

		    DrawPiP(TRUE);

		    break;

		case RC_8:

		    if(yofs <= 45)
		    {
			yofs += 5;

			DrawPiP(TRUE);
		    }

		    break;

		case RC_9:

		    PiPPosition = RIGHT_BOTTOM;

		    DrawPiP(TRUE);

		    break;

		case RC_0:

		    xofs = yofs = 0;

		    DrawPiP(TRUE);

		    break;

		case RC_LEFT:

		    if(swap_cursor)
		    {
			SwitchPiP(TRUE);
		    }
		    else
		    {
			SwitchTV(TRUE);
		    }

		    break;

		case RC_RIGHT:

		    if(swap_cursor)
		    {
			SwitchPiP(FALSE);
		    }
		    else
		    {
			SwitchTV(FALSE);
		    }

		    break;

		case RC_UP:

		    if(swap_cursor)
		    {
			SwitchTV(TRUE);
		    }
		    else
		    {
			SwitchPiP(TRUE);
		    }

		    break;

		case RC_DOWN:

		    if(swap_cursor)
		    {
			SwitchTV(FALSE);
		    }
		    else
		    {
			SwitchPiP(FALSE);
		    }

		    break;

		case RC_VOL_UP:

		    WebServerRequest(GET_VOLUME, NULL, volume);

		    if(volume >= 2)
		    {
			volume -= 2;

			WebServerRequest(SET_VOLUME, NULL, volume);
		    }

		    break;

		case RC_VOL_DOWN:

		    WebServerRequest(GET_VOLUME, NULL, volume);

		    if(volume <= 61)
		    {
			volume += 2;

			WebServerRequest(SET_VOLUME, NULL, volume);
		    }

		    break;

		case RC_MUTE:

		    static bool mute = FALSE;

		    mute ^= 1;

		    WebServerRequest(SET_MUTE, NULL, mute);

		    break;

		case RC_BQE_UP:

		    if(PiPSize > 2)
		    {
			PiPSize--;

			DrawPiP(TRUE);

			scaler_reset = TRUE;
		    }

		    break;

		case RC_BQE_DOWN:

		    if(PiPSize < 4)
		    {
			PiPSize++;

			DrawPiP(TRUE);

			scaler_reset = TRUE;
		    }

		    break;

		case RC_OK:

		    int tmp_index;

		    if(!pip_source && source_int.tv_index != source_int.pip_index)
		    {
			tmp_index = source_int.tv_index;
			source_int.tv_index = source_int.pip_index;
			source_int.pip_index = tmp_index;

			ioctl(dmx, DMX_STOP);

			WebServerRequest(SET_CHANNEL, source_int.service[source_int.tv_index].ref, 0);

			ioctl(dmx, DMX_START);

			close_thread = TRUE;

			pthread_join(thread_id, NULL);

			if(pthread_create(&thread_id, NULL, DecodeThread, NULL))
			{
			    Log2File("pthread_create() failed!");
			}
		    }

		    break;

		case RC_RED:

		    transparency ^= 1;

		    for(i = 0; i < 256; i++)
		    {
			tr1[i] = transparency ? transp_value : 0x0000;

			if(i < 6)
			{
    			    tr2[i] = transparency ? transp_value : 0x0000;
			}
		    }

		    tr1[0] = tr2[0] = 0xFFFF;

		    SetPalette(FALSE, FALSE);

		    break;

		case RC_BLUE:

		    skip_p_frame ^= 1;

		    break;

		case RC_MENU:

		    ShowMenu();

		    break;

		case RC_INFO:

		    ShowMessage(ABOUT);

		    break;

		case RC_HELP:

		    ShowHelp();

		    break;
	    }
	}
	while(Key != RC_EXIT);

    // close decode-thread

	close_thread = TRUE;

	pthread_join(thread_id, NULL);

    // restore channel

	if(source_int.tv_index != source_int.watched)
	{
	    WebServerRequest(SET_CHANNEL, source_int.service[source_int.watched].ref, 0);
	}

    // update setup

	if(old_pipsize != PiPSize || old_pippos != PiPPosition || old_xofs != xofs || old_yofs != yofs || old_transparency != transparency || old_skip_p_frame != skip_p_frame)
	{
	    SaveConfig();
	}

    return;
}
