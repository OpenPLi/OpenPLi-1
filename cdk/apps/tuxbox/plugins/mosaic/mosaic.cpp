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
    #include <mpeg2convert.h>
}

#include <plugin.h>
#if HAVE_DVB_API_VERSION < 3
 #define dmx_pes_filter_params dmxPesFilterParams
 #define pes_type pesType
 #define dmx_sct_filter_params dmxSctFilterParams
 #include <ost/dmx.h>
 #define DMX	"/dev/dvb/card0/demux0"
#else
 #include <linux/dvb/dmx.h>
 #define DMX	"/dev/dvb/adapter0/demux0"
#endif
#include <dbox/avia_gt_pig.h>

#include <pic_1.h>
#include <pic_2.h>
#include <pic_3.h>

#define TRUE  1
#define FALSE 0

#define SUCCESS	1
#define FAILURE 0

#define A 0
#define R 1
#define G 2
#define B 3

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
#define INBUF_SIZE	    16*1024

#define VERSION	"0.4b"
#define BUILD	"21.05.2007"

#define CFGFILE "/var/tuxbox/config/mosaic.conf"
#define LOGFILE	"/tmp/mosaic.log"

#define FONT	"/share/fonts/pakenham.ttf"

#define FB	"/dev/fb/0"
#define RC	"/dev/rawir2"
#define PIG	"/dev/dbox/pig0"

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

extern "C" void plugin_exec(PluginParam *Parameter);

enum {NOERROR, NETWORK, DENIED, NOSERVICE, BOXTYPE, THREAD, ABOUT};

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

    int sequence[9];

    struct
    {
	char  name[64];
	char  ref[64];
	short pid;
	int  state;

    }service[100];

}source_int, source_ext;

int x_pos[3] = {65, 270, 475};
int y_pos[3] = {50, 216, 382};

bool next_pip = FALSE;
int pip;

struct SOURCE *source = &source_int;
bool pip_source = 0;

int last_channel = -1, from_channel;

int volume;

int web;

unsigned short Key;

FILE *LogFile;

int rc = -1;
int fb = -1;
int dmx = -1;
int pig = -1;

int x_ofs = 0, y_ofs = 0;

int scramble_check;

int transp_value;

int DMXBufferSize = 1024;
int language = 0;
int debug = TRUE;
int transparency = FALSE;
int colormode = FALSE;

bool fullscreen = FALSE;

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
void DrawBox(int sx, int sy, int width, int height, int boxcolor, int bordercolor);

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

			source_int.watched = 0;

			for(int i = 0; i <= source_int.services; i++)
			{
			    if(!strcmp(current_sref, source_int.service[i].ref))
			    {
				source_int.watched = i;

				break;
			    }
			}

			Log2File("PiG = %s\n", source_int.service[source_int.watched].name);
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

/***** SetPalette  ************************************************************/

void SetPalette(bool color, bool clearscreen)
{
    if(clearscreen)
    {
	DrawBox(0, 0, 720, 576, 0, FALSE);
    }

    if(!colormode)
    {
	if(ioctl(fb, FBIOPUTCMAP, color ? &colormap : &graymap) == -1)
	{
    	Log2File("ioctl() \"FBIOPUTCMAP\" failed: %s\n", strerror(errno));
	}
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

colormode = FALSE;
Log2File("Option \"COLORMODE=1\" unsupported on this Hardware\n");
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
        tr1[i] = /*transparency ? transp_value :*/ 0x0000;

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

#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	if(FTC_SBitCache_Lookup(cache, &desc, glyphindex, &sbit, NULL))
#else
	if(FTC_SBit_Cache_Lookup(cache, &desc, glyphindex, &sbit))
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
			    if(colormode)
			    {
				*(lfb + (sx+x_ofs + sbit->left + kerning.x + x)*4 + (sy+y_ofs - sbit->top + y)*720*4 + A) = 0xFF;
				*(lfb + (sx+x_ofs + sbit->left + kerning.x + x)*4 + (sy+y_ofs - sbit->top + y)*720*4 + R) = rd2[color]>>8;
				*(lfb + (sx+x_ofs + sbit->left + kerning.x + x)*4 + (sy+y_ofs - sbit->top + y)*720*4 + G) = gn2[color]>>8;
				*(lfb + (sx+x_ofs + sbit->left + kerning.x + x)*4 + (sy+y_ofs - sbit->top + y)*720*4 + B) = bl2[color]>>8;
			    }
			    else
			    {
				*(lfb + (sx+x_ofs + sbit->left + kerning.x + x) + (sy+y_ofs - sbit->top + y)*720) = color;
			    }
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
	    DrawBox(sx, sy-14, maxwidth, 18, backcolor, FALSE);
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

/***** DrawBox  ***************************************************************/

void DrawBox(int sx, int sy, int width, int height, int boxcolor, int bordercolor)
{
    int x, y;

    // box

	for(y = 0; y < height; y++)
	{
	    if(colormode)
	    {
		if(width == 720 && height == 576)
		{
		    memset(lfb, boxcolor, 720*576*4);

		    break;
		}
		else
		{
		    for(x = 0; x < width; x++)
		    {
		        *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+y)*720*4 + A) = boxcolor ? 0xFF : 0;
		        *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+y)*720*4 + R) = rd2[boxcolor]>>8;
		        *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+y)*720*4 + G) = gn2[boxcolor]>>8;
		        *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+y)*720*4 + B) = bl2[boxcolor]>>8;
		    }
		}
	    }
	    else
	    {
		memset(lfb + sx+x_ofs + (sy+y_ofs+y)*720, boxcolor, width);
	    }
	}

    // border

	if(bordercolor)
	{
	    if(colormode)
	    {
		for(x = 0; x < width; x++)
		{
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs)*720*4 + A) = 0xFF;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs)*720*4 + R) = rd2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs)*720*4 + G) = gn2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs)*720*4 + B) = bl2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+1)*720*4 + A) = 0xFF;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+1)*720*4 + R) = rd2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+1)*720*4 + G) = gn2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+1)*720*4 + B) = bl2[bordercolor]>>8;

		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+height-2)*720*4 + A) = 0xFF;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+height-2)*720*4 + R) = rd2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+height-2)*720*4 + G) = gn2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+height-2)*720*4 + B) = bl2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+height-1)*720*4 + A) = 0xFF;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+height-1)*720*4 + R) = rd2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+height-1)*720*4 + G) = gn2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+x)*4 + (sy+y_ofs+height-1)*720*4 + B) = bl2[bordercolor]>>8;
		}

		for(y = 0; y < height; y++)
		{
		    *(lfb + (sx+x_ofs)*4 + (sy+y_ofs+y)*720*4 + A) = 0xFF;
		    *(lfb + (sx+x_ofs)*4 + (sy+y_ofs+y)*720*4 + R) = rd2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs)*4 + (sy+y_ofs+y)*720*4 + G) = gn2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs)*4 + (sy+y_ofs+y)*720*4 + B) = bl2[bordercolor]>>8;

		    *(lfb + (sx+x_ofs+width-1)*4 + (sy+y_ofs+y)*720*4 + A) = 0xFF;
		    *(lfb + (sx+x_ofs+width-1)*4 + (sy+y_ofs+y)*720*4 + R) = rd2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+width-1)*4 + (sy+y_ofs+y)*720*4 + G) = gn2[bordercolor]>>8;
		    *(lfb + (sx+x_ofs+width-1)*4 + (sy+y_ofs+y)*720*4 + B) = bl2[bordercolor]>>8;
		}
	    }
	    else
	    {
		memset(lfb + sx+x_ofs + (sy+y_ofs)*720, bordercolor, width);
		memset(lfb + sx+x_ofs + (sy+y_ofs+1)*720, bordercolor, width);

		for(y = 0; y < height; y++)
		{
		    *(lfb + sx+x_ofs + (sy+y_ofs+y)*720) = bordercolor;
		    *(lfb + sx+x_ofs+width-1 + (sy+y_ofs+y)*720) = bordercolor;
		}

		memset(lfb + sx+x_ofs + (sy+y_ofs+height-2)*720, bordercolor, width);
		memset(lfb + sx+x_ofs + (sy+y_ofs+height-1)*720, bordercolor, width);
	    }
	}
}

/***** DrawLayout  ************************************************************/

void DrawLayout()
{
    int x;

    char *msg[][1] =
    {
	{"unused"},
	{"unbenutzt"}
    };

    // clear screen

	DrawBox(0, 0, 720, 576, colormode ? 0 : 1, FALSE);

    // draw borders

	for(x = 0; x < 9; x++)
	{
	    DrawBox(x_pos[x%3]-1, y_pos[x/3]-2, 720/4+2, 576/4+4, colormode ? 0 : 1, colormode ? FONTCOLOR_C1 : BORDERCOLOR);
	}

    // cut pig hole

	DrawBox(x_pos[1], y_pos[1], 720/4, 576/4, 0, FALSE);

	avia_pig_hide(pig);
	avia_pig_set_pos(pig, x_pos[1]+x_ofs, y_pos[1]+y_ofs);
	avia_pig_show(pig);

    // draw channelnames

	for(x = 0; x < 9; x++)
	{
	    RenderString(source->sequence[x] == -1 ? msg[language][0] : source->service[source->sequence[x]].name, x_pos[x%3], y_pos[x/3] + 160, 720/4, MIDDLE, colormode ? FONTCOLOR_C1 : FONTCOLOR_G1, FALSE);
	}

	pip = 8;
	next_pip = TRUE;
}

/***** ShowMenu  **************************************************************/

void ShowMenu()
{
#define MENU_WIDTH 200
#define MENU_START 180

    int y;
    int selected_entry = 4;
    int selected_channel[9];

    char *title[] = {"Service Sequence", "Senderanordnung"};
    char *msg[][1] =
    {
	{"unused"},
	{"unbenutzt"}
    };

    if(fullscreen)
    {
	return;
    }

    lock_framebuffer = TRUE;
    usleep(100000);

    SetPalette(TRUE, TRUE);

    avia_pig_hide(pig);

    for(y = 0; y < 9; y++)
    {
	selected_channel[y] = source->sequence[y];
    }

    DrawBox((720-MENU_WIDTH)/2, MENU_START, 200, 25, MENUCOLOR1, MENUCOLOR3);
    DrawBox((720-MENU_WIDTH)/2, MENU_START+23, 200, 190, MENUCOLOR2, MENUCOLOR3);

    RenderString(title[language], (720-MENU_WIDTH)/2, MENU_START+18, MENU_WIDTH, MIDDLE, FONTCOLOR_C2, FALSE);
    RenderString(selected_channel[0] == -1 ? msg[language][0] : source->service[selected_channel[0]].name, (720-MENU_WIDTH)/2, MENU_START+45, MENU_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);
    RenderString(selected_channel[1] == -1 ? msg[language][0] : source->service[selected_channel[1]].name, (720-MENU_WIDTH)/2, MENU_START+65, MENU_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);
    RenderString(selected_channel[2] == -1 ? msg[language][0] : source->service[selected_channel[2]].name, (720-MENU_WIDTH)/2, MENU_START+85, MENU_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);
    RenderString(selected_channel[3] == -1 ? msg[language][0] : source->service[selected_channel[3]].name, (720-MENU_WIDTH)/2, MENU_START+105, MENU_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);
    RenderString(selected_channel[4] == -1 ? msg[language][0] : source->service[selected_channel[4]].name, (720-MENU_WIDTH)/2, MENU_START+125, MENU_WIDTH, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
    RenderString(selected_channel[5] == -1 ? msg[language][0] : source->service[selected_channel[5]].name, (720-MENU_WIDTH)/2, MENU_START+145, MENU_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);
    RenderString(selected_channel[6] == -1 ? msg[language][0] : source->service[selected_channel[6]].name, (720-MENU_WIDTH)/2, MENU_START+165, MENU_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);
    RenderString(selected_channel[7] == -1 ? msg[language][0] : source->service[selected_channel[7]].name, (720-MENU_WIDTH)/2, MENU_START+185, MENU_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);
    RenderString(selected_channel[8] == -1 ? msg[language][0] : source->service[selected_channel[8]].name, (720-MENU_WIDTH)/2, MENU_START+205, MENU_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);

    do
    {
	Key = GetRCCode();

	switch(Key)
	{
	    case RC_UP:

		if(selected_entry)
		{
		    RenderString(selected_channel[selected_entry] == -1 ? msg[language][0] : source->service[selected_channel[selected_entry]].name, (720-MENU_WIDTH)/2+1, MENU_START+45+selected_entry*20, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);
		    selected_entry--;
		    RenderString(selected_channel[selected_entry] == -1 ? msg[language][0] : source->service[selected_channel[selected_entry]].name, (720-MENU_WIDTH)/2+1, MENU_START+45+selected_entry*20, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
		}
		else
		{
		    RenderString(selected_channel[selected_entry] == -1 ? msg[language][0] : source->service[selected_channel[selected_entry]].name, (720-MENU_WIDTH)/2+1, MENU_START+45+selected_entry*20, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);
		    selected_entry = 8;
		    RenderString(selected_channel[selected_entry] == -1 ? msg[language][0] : source->service[selected_channel[selected_entry]].name, (720-MENU_WIDTH)/2+1, MENU_START+45+selected_entry*20, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
		}

		break;

	    case RC_DOWN:

		if(selected_entry < 8)
		{
		    RenderString(selected_channel[selected_entry] == -1 ? msg[language][0] : source->service[selected_channel[selected_entry]].name, (720-MENU_WIDTH)/2+1, MENU_START+45+selected_entry*20, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);
		    selected_entry++;
		    RenderString(selected_channel[selected_entry] == -1 ? msg[language][0] : source->service[selected_channel[selected_entry]].name, (720-MENU_WIDTH)/2+1, MENU_START+45+selected_entry*20, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
		}
		else
		{
		    RenderString(selected_channel[selected_entry] == -1 ? msg[language][0] : source->service[selected_channel[selected_entry]].name, (720-MENU_WIDTH)/2+1, MENU_START+45+selected_entry*20, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C4, SELECTBAR2);
		    selected_entry = 0;
		    RenderString(selected_channel[selected_entry] == -1 ? msg[language][0] : source->service[selected_channel[selected_entry]].name, (720-MENU_WIDTH)/2+1, MENU_START+45+selected_entry*20, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);
		}

		break;

	    case RC_LEFT:

		if(selected_channel[selected_entry] > 0)
		{
		    selected_channel[selected_entry]--;
		}
		else if(selected_channel[selected_entry] == 0)
		{
		    selected_channel[selected_entry] = -1;
		}
		else
		{
		    selected_channel[selected_entry] = source->services - 1;
		}

		RenderString(selected_channel[selected_entry] == -1 ? msg[language][0] : source->service[selected_channel[selected_entry]].name, (720-MENU_WIDTH)/2+1, MENU_START+45+selected_entry*20, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);

		break;

	    case RC_RIGHT:

		if(selected_channel[selected_entry] < source->services - 1)
		{
		    selected_channel[selected_entry]++;
		}
		else if(selected_channel[selected_entry] == source->services - 1)
		{
		    selected_channel[selected_entry] = -1;
		}
		else
		{
		    selected_channel[selected_entry] = 0;
		}

		RenderString(selected_channel[selected_entry] == -1 ? msg[language][0] : source->service[selected_channel[selected_entry]].name, (720-MENU_WIDTH)/2+1, MENU_START+45+selected_entry*20, MENU_WIDTH-2, MIDDLE, FONTCOLOR_C3, SELECTBAR1);

		break;

	    case RC_OK:

		if(selected_channel[4] != -1)
		{
		    if(selected_channel[4] != source->sequence[4])
		    {
			ioctl(dmx, DMX_STOP);

			WebServerRequest(SET_CHANNEL, source->service[selected_channel[4]].ref, 0);

			ioctl(dmx, DMX_START);
		    }

		    for(y = 0; y < 9; y++)
		    {
			source->sequence[y] = selected_channel[y];
		    }

		    last_channel = -1;
		}
		else
		{
		    Key = 0xDEAD;
		}

		break;
	}
    }
    while(Key != RC_EXIT && Key != RC_OK && Key != RC_MENU);

    SetPalette(FALSE, TRUE);

    DrawLayout();

    lock_framebuffer = FALSE;

    Key = 0xDEAD;
}

/***** ShowMessage  ***********************************************************/

void ShowMessage(int message)
{
#define MSGBOX_WIDTH 260
#define MSGBOX_START 233

    char *title[] = {"Program Information", "Programminformation"};

    char *msg[][8] =
    {
	{"Box-Type unknown!",	"Access to WebServer failed!",			"Check Network / Port",		"Access to WebServer denied!",		"Check User / Password",		"No Transponder-Services found!",	"Decode-Thread failed!",		"Mosaic-Plugin Version "VERSION" / "BUILD},
	{"Box-Typ unbekannt!",	"Zugriff auf WebServer fehlgeschlagen!",	"Netzwerk / Port überprüfen",	"Zugriff auf WebServer verweigert!",	"Benutzer / Passwort überprüfen",	"Keine Transponder-Kanäle gefunden!",	"Decoder-Thread fehlgeschlagen!",	"Mosaik-Plugin Version "VERSION" / "BUILD}
    };

    if(fullscreen)
    {
	return;
    }

    lock_framebuffer = TRUE;
    usleep(100000);

    SetPalette(TRUE, TRUE);

    avia_pig_hide(pig);

    DrawBox((720-MSGBOX_WIDTH)/2, MSGBOX_START, MSGBOX_WIDTH, 25, MENUCOLOR1, MENUCOLOR3);
    DrawBox((720-MSGBOX_WIDTH)/2, MSGBOX_START+23, MSGBOX_WIDTH, 90, MENUCOLOR2, MENUCOLOR3);
    DrawBox((720-30)/2, MSGBOX_START+85, 30, 19, MENUCOLOR1, MENUCOLOR3);

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

	DrawLayout();
    }

    lock_framebuffer = FALSE;

    Key = 0xDEAD;
}

/***** ShowHelp ***************************************************************/

void ShowHelp()
{
#define HELP_WIDTH 470
#define HELP_START 130

    char *title[] = {"Help", "Hilfe"};

    char *msg[][14] =
    {
	{"Livestream :",	"last Channel :",	"Fullscreen :",	"Offsets :",		"reset Offsets :",		"Mute on/off :",		"Volume :",	"Plugin-Information :",	"Menu :",	"Exit :",	"complete Documentation available at",		"CURSOR UP/DOWN & VOLUME +/-",		"GREEN/YELLOW",	"CURSOR UP/DOWN/LEFT/RIGHT"},
	{"Livebild :",		"letzter Kanal :",	"Vollbild :",	"Verschiebung :",	"Verschiebung rücksetzen :",	"Stummschaltung ein/aus :",	"Lautstärke :",	"Plugin-Information :",	"Menü :",	"Ende :",	"komplette Dokumentation verfügbar unter",	"CURSOR HOCH/RUNTER & VOLUME +/-",	"GRÜN/GELB",	"CURSOR HOCH/RUNTER/LINKS/RECHTS"}
    };

    if(fullscreen)
    {
	return;
    }

    lock_framebuffer = TRUE;
    usleep(100000);

    SetPalette(TRUE, TRUE);

    avia_pig_hide(pig);

    DrawBox((720-HELP_WIDTH)/2, HELP_START, HELP_WIDTH, 25, MENUCOLOR1, MENUCOLOR3);
    DrawBox((720-HELP_WIDTH)/2, HELP_START+23, HELP_WIDTH, 290, MENUCOLOR2, MENUCOLOR3);
    DrawBox((720-30)/2, HELP_START+285, 30, 19, MENUCOLOR1, MENUCOLOR3);

    RenderString(title[language], (720-HELP_WIDTH)/2, HELP_START+18, HELP_WIDTH, MIDDLE, FONTCOLOR_C2, FALSE);

    RenderString(msg[language][0], (720-HELP_WIDTH)/2, HELP_START+50, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("1 2 3 4 6 7 8 9", 720/2+4, HELP_START+50, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][1], (720-HELP_WIDTH)/2, HELP_START+70, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("5", 720/2+4, HELP_START+70, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][2], (720-HELP_WIDTH)/2, HELP_START+90, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("OK", 720/2+4, HELP_START+90, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][3], (720-HELP_WIDTH)/2, HELP_START+110, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString((char*)(BoxType == DM7000 || BoxType == DM7020 ? msg[language][13] : msg[language][11]), 720/2+4, HELP_START+110, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][4], (720-HELP_WIDTH)/2, HELP_START+130, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("0", 720/2+4, HELP_START+130, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][5], (720-HELP_WIDTH)/2, HELP_START+150, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("MUTE", 720/2+4, HELP_START+150, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][6], (720-HELP_WIDTH)/2, HELP_START+170, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString((char*)(BoxType == DM7000 || BoxType == DM7020 ? "VOLUME +/-" : msg[language][12]), 720/2+4, HELP_START+170, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][7], (720-HELP_WIDTH)/2, HELP_START+190, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("INFO", 720/2+4, HELP_START+190, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][8], (720-HELP_WIDTH)/2, HELP_START+210, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("MENU", 720/2+4, HELP_START+210, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][9], (720-HELP_WIDTH)/2, HELP_START+230, HELP_WIDTH/2, RIGHT, FONTCOLOR_C1, FALSE);
    RenderString("EXIT/LAME", 720/2+4, HELP_START+230, HELP_WIDTH/2, LEFT, FONTCOLOR_C1, FALSE);

    RenderString(msg[language][10], (720-HELP_WIDTH)/2, HELP_START+255, HELP_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);
    RenderString("www.dream-multimedia-tv.de/board/thread.php?threadid=933", (720-HELP_WIDTH)/2, HELP_START+275, HELP_WIDTH, MIDDLE, FONTCOLOR_C2, FALSE);

    RenderString("OK", (720-HELP_WIDTH)/2, HELP_START+300, HELP_WIDTH, MIDDLE, FONTCOLOR_C4, FALSE);

    do
    {
	Key = GetRCCode();
    }
    while(Key != RC_EXIT && Key != RC_OK && Key != RC_HELP);

    SetPalette(FALSE, TRUE);

    DrawLayout();

    lock_framebuffer = FALSE;

    Key = 0xDEAD;
}

/***** SwitchPiG  *************************************************************/

void SwitchPiG(int position)
{
    if(source->sequence[position - 1] != -1)
    {
	last_channel = source->sequence[4];
	from_channel = position - 1;

	source->sequence[4] = source->sequence[position - 1];
	source->sequence[position - 1] = last_channel;

	DrawLayout();

	ioctl(dmx, DMX_STOP);

	WebServerRequest(SET_CHANNEL, source->service[source->sequence[4]].ref, 0);

	ioctl(dmx, DMX_START);
    }
}

/***** DrawIcon  ***************************************************************/

void DrawIcon(int icon)
{
    unsigned char *pic_ptr = NULL;

	switch(icon)
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
	}

	if(!lock_framebuffer)
	{
	    for(int y = 0; y < 48; y++)
	    {
		for(int x = 0; x < 48; x++)
		{
		    if(colormode)
		    {
			*(lfb + (x_pos[pip%3]+x_ofs + x + ((720/4)-48)/2)*4 + (y_pos[pip/3]+y_ofs + y + ((576/4)-48)/2)*720*4 + A) = 0xFF;
			*(lfb + (x_pos[pip%3]+x_ofs + x + ((720/4)-48)/2)*4 + (y_pos[pip/3]+y_ofs + y + ((576/4)-48)/2)*720*4 + R) = pic_ptr[x + y*48];
			*(lfb + (x_pos[pip%3]+x_ofs + x + ((720/4)-48)/2)*4 + (y_pos[pip/3]+y_ofs + y + ((576/4)-48)/2)*720*4 + G) = pic_ptr[x + y*48];
			*(lfb + (x_pos[pip%3]+x_ofs + x + ((720/4)-48)/2)*4 + (y_pos[pip/3]+y_ofs + y + ((576/4)-48)/2)*720*4 + B) = pic_ptr[x + y*48];
		    }
		    else
		    {
    			*(lfb + x_pos[pip%3]+x_ofs + x + ((720/4)-48)/2 + (y_pos[pip/3]+y_ofs + y + ((576/4)-48)/2)*720) = pic_ptr[x + y*48];
		    }
		}
	    }
	}
}

/***** SaveConfig  ************************************************************/

void SaveConfig()
{
    if((fd_conf = fopen(CFGFILE, "w")))
    {
	fprintf(fd_conf, "OFFSET_X=%d\n", x_ofs);
	fprintf(fd_conf, "OFFSET_Y=%d\n\n", y_ofs);

	fprintf(fd_conf, "TRANSPARENCY=%d\n", transparency);
	fprintf(fd_conf, "COLORMODE=%d\n\n", colormode);

	fprintf(fd_conf, "WEBPORT_INT=%d\n", source_int.port);
	fprintf(fd_conf, "WEBUSER_INT=%s\n", source_int.user);
	fprintf(fd_conf, "WEBPASS_INT=%s\n\n", source_int.pass);

	fprintf(fd_conf, "LANGUAGE=%d\n\n", language);

	fprintf(fd_conf, "DEBUG=%d\n\n", debug);

	fprintf(fd_conf, "DMXBUFFER=%d\n", DMXBufferSize);

	fclose(fd_conf);
    }
}

/***** CleanUp ****************************************************************/

void CleanUp()
{
    if(LogFile)
    {
        Log2File("Mosaic-Plugin "VERSION" by LazyT cleaning up");

	fclose(LogFile);
    }

    // framebuffer

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

    // pig

	if(pig != -1)
	{
	    avia_pig_hide(pig);

	    close(pig);
	}

    // remote control

	if(rc != -1)
	{
	    close(rc);
	}
}

/***** DecodeThread  **********************************************************/

void *DecodeThread(void *arg)
{
    int scale_720[180], scale_704[180], scale_640[180], scale_544[180], scale_528[180], scale_480[180], scale_352[180];
    int s1, s2, s3, s4, s5, s6, s7;

    mpeg2dec_t *decoder= NULL;
    const mpeg2_info_t *info = NULL;

    struct dmxPesFilterParams dmx_flt;
    struct timeval tv;
    fd_set readfd;

    unsigned char buffer[INBUF_SIZE];
    int bytes;
    int stream_x = 0, stream_y = 0;
    bool skip_frame = FALSE;

int last = -1;

    // init

        Log2File("Decode-Thread starting");

	s1 = s2 = s3 = s4 = s5 = s6 = s7 = 0;

	for(int i = 0; i < 180; s1 += 7200/180, s2 += 7040/180, s3 += 6400/180, s4 += 5440/180, s5 += 5280/180, s6 += 4800/180, s7 += 3520/180, i++)
	{
	    scale_720[i] = (int)(s1 + 5)/10;
	    scale_704[i] = (int)(s2 + 5)/10;
	    scale_640[i] = (int)(s3 + 5)/10;
	    scale_544[i] = (int)(s4 + 5)/10;
	    scale_528[i] = (int)(s5 + 5)/10;
	    scale_480[i] = (int)(s6 + 5)/10;
	    scale_352[i] = (int)(s7 + 5)/10;
	}

	for(pip = 0; pip < 9; pip++)
	{
	    source->service[pip].state = NOICON;
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

    // demuxer

	if((dmx = open(DMX, O_RDONLY | O_NONBLOCK)) == -1)
	{
	    Log2File("open() \""DMX"\" failed: %s\n", strerror(errno));

	    goto cleanup;
	}

	if(ioctl(dmx, DMX_SET_BUFFER_SIZE, DMXBufferSize*1024) == -1)
	{
	    Log2File("ioctl() \"DMX_SET_BUFFER_SIZE\" failed: %s\n", strerror(errno));
	}
	else
	{
	    Log2File("Demux-Buffersize = %dKB", (void*)DMXBufferSize);
	}

    // loop

        while(!close_thread)
	{
    	    for(pip = 0; pip < 9; pip++)
	    {
		if(pip != 4)
		{
		    if(source->sequence[pip] == -1)
		    {
			DrawIcon(UNAVAILABLE);

			continue;
		    }
		    else if(source->service[source->sequence[pip]].state == SCRAMBLED)
		    {
			DrawIcon(SCRAMBLED);

			continue;
		    }
		    else if(source->service[source->sequence[pip]].state == TIMEOUT)
		    {
			DrawIcon(TIMEOUT);

			continue;
		    }

		    next_pip = FALSE;
		    scramble_check = 0;

		    if(last != source->service[source->sequence[pip]].pid)
		    {
		        last = source->service[source->sequence[pip]].pid;

			ioctl(dmx, DMX_STOP);

			dmx_flt.pid = source->service[source->sequence[pip]].pid;
			dmx_flt.input = DMX_IN_FRONTEND;
			dmx_flt.output = DMX_OUT_TAP;
			dmx_flt.pesType = DMX_PES_OTHER;
			dmx_flt.flags = DMX_IMMEDIATE_START;

			if(ioctl(dmx, DMX_SET_PES_FILTER, &dmx_flt) == -1)
			{
			    Log2File("ioctl() \"DMX_SET_PES_FILTER\" failed: %s", strerror(errno));

			    continue;
			}

			mpeg2_reset(decoder, TRUE);
		    }

		    while(!next_pip)
		    {
			if(close_thread)
			{
			    break;
			}

			switch(mpeg2_parse(decoder))
			{
			    case STATE_BUFFER:

				tv.tv_sec = 2;
				tv.tv_usec = 0;

				FD_ZERO(&readfd);
				FD_SET(dmx, &readfd);

				select(dmx + 1, &readfd, NULL, NULL, &tv);

				if(!FD_ISSET(dmx, &readfd))
				{
				    DrawIcon(TIMEOUT);

				    source->service[source->sequence[pip]].state = TIMEOUT;

				    Log2File("%s timed out", source->service[source->sequence[pip]].name);

				    next_pip = TRUE;

				    break;
				}

				bytes = read(dmx, buffer, sizeof(buffer));

				mpeg2_buffer(decoder, buffer, buffer + bytes);

				scramble_check += bytes;

				if(scramble_check > INBUF_SIZE*50)
				{
				    DrawIcon(SCRAMBLED);

				    source->service[source->sequence[pip]].state = SCRAMBLED;

				    Log2File("%s can't be decoded", source->service[source->sequence[pip]].name);

				    next_pip = TRUE;

				    break;
				}

				break;

			    case STATE_PICTURE:

				scramble_check = 0;

				switch(info->current_picture->flags & PIC_MASK_CODING_TYPE)
				{
				    case PIC_FLAG_CODING_TYPE_B:

					skip_frame = TRUE;

					break;

				    case PIC_FLAG_CODING_TYPE_P:

					skip_frame = FALSE;

					break;

				    case PIC_FLAG_CODING_TYPE_I:

					skip_frame = FALSE;

					break;
				}

				mpeg2_skip(decoder, skip_frame);

				break;

			    case STATE_SEQUENCE:

				stream_x = info->sequence->width;
				stream_y = info->sequence->height;

				if(colormode)
				{
				    mpeg2_convert(decoder, mpeg2convert_rgb32, NULL);
				}

				break;

			    case STATE_SLICE:

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

					    Log2File("Streamtype %dx%d (%s) unsupported, please report!", (void*)stream_x, (void*)stream_y, source->service[source->sequence[pip]].name);

					    next_pip = TRUE;

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

					    Log2File("Streamtype %dx%d (%s) unsupported, please report!", (void*)stream_x, (void*)stream_y, source->service[source->sequence[pip]].name);

					    next_pip = TRUE;

					    continue;
				    }

				    if(!lock_framebuffer)
				    {
					for(y = 0; y < stream_y/4*y_scaler; y++)
					{
					    for(x = 0; x < 720/4; x++)
					    {
						if(colormode)
						{
						    *(lfb + (x_pos[pip%3]+x_ofs+x)*4 + (y_pos[pip/3]+y_ofs+y)*720*4 + A) = 0xFF;
						    *(lfb + (x_pos[pip%3]+x_ofs+x)*4 + (y_pos[pip/3]+y_ofs+y)*720*4 + R) = *(info->display_fbuf->buf[0] + R + x_scaler[x]*4 + y*4/y_scaler*stream_x*4) ? *(info->display_fbuf->buf[0] + R + x_scaler[x]*4 + y*4/y_scaler*stream_x*4) : 0x01;
						    *(lfb + (x_pos[pip%3]+x_ofs+x)*4 + (y_pos[pip/3]+y_ofs+y)*720*4 + G) = *(info->display_fbuf->buf[0] + G + x_scaler[x]*4 + y*4/y_scaler*stream_x*4) ? *(info->display_fbuf->buf[0] + G + x_scaler[x]*4 + y*4/y_scaler*stream_x*4) : 0x01;
						    *(lfb + (x_pos[pip%3]+x_ofs+x)*4 + (y_pos[pip/3]+y_ofs+y)*720*4 + B) = *(info->display_fbuf->buf[0] + B + x_scaler[x]*4 + y*4/y_scaler*stream_x*4) ? *(info->display_fbuf->buf[0] + B + x_scaler[x]*4 + y*4/y_scaler*stream_x*4) : 0x01;
						}
						else
						{
						    *(lfb + x_pos[pip%3]+x_ofs+x + (y_pos[pip/3]+y_ofs+y)*720) = *(info->display_fbuf->buf[0] + x_scaler[x] + y*4/y_scaler*stream_x) ? *(info->display_fbuf->buf[0] + x_scaler[x] + y*4/y_scaler*stream_x) : 0x01;
						}
					    }
					}
				    }

				    next_pip = TRUE;
				}

				break;

			    case STATE_SEQUENCE_REPEATED:
			    case STATE_GOP:
			    case STATE_SLICE_1ST:
			    case STATE_PICTURE_2ND:
			    case STATE_END:
			    case STATE_INVALID:
			    case STATE_INVALID_END:

				break;
			}
		    }
		}
	    }
	}

    // exit

cleanup:

	if(dmx != -1)
	{
	    if(ioctl(dmx, DMX_STOP) == -1)
	    {
		Log2File("ioctl() \"DMX_STOP\" failed: %s", strerror(errno));
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

/***** plugin_exec ************************************************************/

void plugin_exec(PluginParam *Parameter)
{
    bool rezap = FALSE;
    int max_retries;
    int old_xofs, old_yofs;
    int i;

    // read config

	strncpy(source_int.addr, "127.0.0.1", sizeof(source_int.addr));
	strncpy(source_int.user, "root", sizeof(source_int.user));
	strncpy(source_int.pass, "dreambox", sizeof(source_int.pass));
	source_int.port = 80;

	if((fd_conf = fopen(CFGFILE, "r")))
	{
	    while(fgets(TempBuffer, sizeof(TempBuffer), fd_conf))
	    {
		if(strstr(TempBuffer, "OFFSET_X="))
		{
		    sscanf(TempBuffer + 9, "%d", &x_ofs);

		    if(x_ofs < -50 || x_ofs > 50)
		    {
			x_ofs = 0;
		    }
		}
		else if(strstr(TempBuffer, "OFFSET_Y="))
		{
		    sscanf(TempBuffer + 9, "%d", &y_ofs);

		    if(y_ofs < -50 || y_ofs > 50)
		    {
			y_ofs = 0;
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
		else if(strstr(TempBuffer, "COLORMODE="))
		{
		    sscanf(TempBuffer + 10, "%d", &colormode);

		    if(colormode < 0 || colormode > 1)
		    {
			colormode = 0;
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
		else if(strstr(TempBuffer, "DMXBUFFER="))
		{
		    sscanf(TempBuffer + 10, "%d", &DMXBufferSize);

		    if(DMXBufferSize < 512 || DMXBufferSize > 1024*10)
		    {
			DMXBufferSize = 1024;
		    }
		}
	    }

	    fclose(fd_conf);
	}
	else
	{
	    SaveConfig();
	}

	old_xofs = x_ofs;
	old_yofs = y_ofs;

    // debug

        if(debug)
	{
	    LogFile = fopen(LOGFILE, "w");
	}

	Log2File("Mosaic-Plugin "VERSION" by LazyT starting up\n");

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

	if((lfb = (unsigned char*)mmap(0, fix_screeninfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0)) == MAP_FAILED)
	{
		Log2File("mmap() failed: %s\n", strerror(errno));

		return;
	}

	if(colormode)
	{
	    if(ioctl(fb, FBIOGET_VSCREENINFO, &var_screeninfo) == -1)
	    {
		Log2File("ioctl() \"FBIOGET_VSCREENINFO\" failed: %s\n", strerror(errno));

		return;
	    }

	    var_screeninfo.bits_per_pixel = 32;

	    if(ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo) == -1)
	    {
		Log2File("ioctl() \"FBIOPUT_VSCREENINFO\" failed: %s\n", strerror(errno));

		return;
	    }
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
	desc.flags = FT_LOAD_MONOCHROME;
	desc.face_id = (char*)FONT;
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
    // pig

	if((pig = open(PIG, O_RDWR)) == -1)
	{
	    Log2File("open() "PIG" failed: %s\n", strerror(errno));

	    return;
	}

	avia_pig_set_pos(pig, x_pos[1]+x_ofs, y_pos[1]+y_ofs);
	avia_pig_set_size(pig, 720/4, 576/4);
	avia_pig_set_stack(pig, 2);
	avia_pig_show(pig);

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

	for(i = 0; i < 9; i++)
	{
	    source->sequence[i] = i < source->services ? i : -1;
	}

	source->sequence[4] = source->watched;

	for(i = 0; i < 9; i++)
	{
	    if(source->sequence[i] == source->watched && i != 4)
	    {
		source->sequence[i] = source->services >= 5 ? 4 : -1;
	    }
	}

    // decode thread

lock_framebuffer = TRUE;

	if(pthread_create(&thread_id, NULL, DecodeThread, NULL))
	{
	    Log2File("pthread_create() failed!");

	    ShowMessage(THREAD);

	    return;
	}

    // layout

	DrawLayout();

lock_framebuffer = FALSE;

    // control loop

	do
	{
	    Key = GetRCCode();

	    switch(Key)
	    {
		case RC_1:

		    SwitchPiG(1);

		    break;

		case RC_2:

		    SwitchPiG(2);

		    break;

		case RC_3:

		    SwitchPiG(3);

		    break;

		case RC_4:

		    SwitchPiG(4);

		    break;

		case RC_5:

		    if(last_channel != -1)
		    {
			int temp = last_channel;

			last_channel = source->sequence[4];

			source->sequence[4] = temp;
			source->sequence[from_channel] = last_channel;

			DrawLayout();

			ioctl(dmx, DMX_STOP);

			WebServerRequest(SET_CHANNEL, source->service[temp].ref, 0);

			ioctl(dmx, DMX_START);
		    }

		    break;

		case RC_6:

		    SwitchPiG(6);

		    break;

		case RC_7:

		    SwitchPiG(7);

		    break;

		case RC_8:

		    SwitchPiG(8);

		    break;

		case RC_9:

		    SwitchPiG(9);

		    break;

		case RC_0:

		    x_ofs = y_ofs = 0;

		    DrawLayout();

		    break;

		case RC_LEFT:

		    if(x_ofs >= -45)
		    {
			x_ofs -= 5;

			DrawLayout();
		    }

		    break;

		case RC_RIGHT:

		    if(x_ofs <= 45)
		    {
			x_ofs += 5;

			DrawLayout();
		    }

		    break;

		case RC_UP:

		    if(y_ofs >= -45)
		    {
			y_ofs -= 5;

			DrawLayout();
		    }

		    break;

		case RC_DOWN:

		    if(y_ofs <= 45)
		    {
			y_ofs += 5;

			DrawLayout();
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

		case RC_OK:

		    fullscreen ^= 1;

		    if(fullscreen)
		    {
			lock_framebuffer = TRUE;
			usleep(100000);

			avia_pig_hide(pig);

			DrawBox(0, 0, 720, 576, 0, FALSE);
		    }
		    else
		    {
			DrawLayout();

			lock_framebuffer = FALSE;
		    }

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

	if(source_int.sequence[4] != source_int.watched)
	{
	    WebServerRequest(SET_CHANNEL, source_int.service[source_int.watched].ref, 0);
	}

    // update setup

	if(old_xofs != x_ofs || old_yofs != y_ofs)
	{
	    SaveConfig();
	}

    return;
}
