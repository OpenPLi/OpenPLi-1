/*
 * $Id: zapstream.c,v1.1 Sjaaky $
 * 
 * inetd style daemon for streaming ts
 * 
 * Copyright (C) 2007 Sjaaky
 *
 * based on code which is
 * Copyright (C) 2006 pieterg
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
 *	http://dreambox:<port>
 *	http://dreambox:<port>/lang=English
 *	http://dreambox:<port>/langpid=0x321
 * parameters:
 *	lang		the default audio language. U can use the same definitions as enigma does in its s:/extra/audiochannelspriority setting
 *	langpid 	the pid of the default audio language. Supports decimal (90) and hexadecimal(0x50) and probably even octal
 *
 * command line parameters:
 *	-ts     send the current channel according to enigma with http
 *	-tcp    send the current channel according to enigma with tcp
 *	-tsfile send a transport stream read from a .ts file	
 *
 * TODO 
 *	implement streaming from continous files .ts -> .001 -> .002 ->...
 *
 */

#include <config.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include <netinet/tcp.h>

//needed to translate language codes
#include "iso639.h"


#if HAVE_DVB_API_VERSION < 3
    #include <ost/dmx.h>
    #define dmx_pes_filter_params dmxPesFilterParams
    #define dmx_sct_filter_params dmxSctFilterParams
    #define pes_type pesType
    #ifdef HAVE_DREAMBOX_HARDWARE
	#define DMXDEV "/dev/dvb/card0/demux1"
	#define DVRDEV "/dev/dvb/card0/dvr1"
	#define TRANSFORM
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


#include <string.h>
#define TS_SIZE 188

#define PMT_LEN 12

char zapstream[] = "zapstream 1.2";
char configfile1[] = "/var/etc/zapstream.conf";
char configfile2[] = "/etc/zapstream.conf";

char enigmaconfigfile[101] = "/var/tuxbox/config/enigma/config";
char logfile[101] = "";
char pmtsocket[100] = "/tmp/.listen.camd.socket";

int useaudiopriority = 1;
char audiochannelspriority[301] = "";
char preferedaudiolanguage[101] = "";
int preferedaudiopid = 0;

int logwritespeed = 0;
int logreadspeed = 0;
int logaudiopriority = 0;
int logdiscontinuity = 0;

int defaultpriority = 1;
int streamingpriority = -1;
int immediatestart = 1;

int httpmajor = 0;
int httpminor = 0;

int so_sndbuf = 5020;
int so_rcvbuf = 4096;

int tcp_nodelay = 1; //nodelay, disable nagle's algorithm
int tcp_maxseg = 1500; //TCP_MAXSEG <= mtu

int delayafterzap = 1200000;

typedef unsigned char uchar;

typedef enum
{
	streamtype_pmt,
	streamtype_audio,
	streamtype_video,
	streamtype_pcr,
	streamtype_other
} streamtypes;

char str_init[] = "st_init";
char str_streaming[] = "st_streaming";
char str_waitforcapmt[] = "st_waitforcapmt";
char str_waitafterzap[] = "st_waitafterzap";
char str_waitforpmt[] = "st_waitforpmt";
char str_startstreaming[] = "st_startstreaming";
char str_stopstreaming[] = "st_stopstreaming";
char str_unknownstate[] = "unknown state";

typedef enum
{
	st_init,
	st_streaming,
	st_waitforcapmt,
	st_waitafterzap,
	st_waitforpmt,
	st_startstreaming,
	st_stopstreaming
} states;

states st = st_init;
int waitcounter = 0;

typedef struct
{
	int fd;
	int pid;
	int count;
	streamtypes type; 
	uchar esinfolength;
	uchar* esinfo;
	uchar streamtype;
} DemuxEntry;

#define IN_SIZE		(TS_SIZE * 500)
#define IPACKS		2048
int bufsize = IN_SIZE*10;

/* demux buffer size */
#define DMX_BUFFER_SIZE (TS_SIZE*1000) // 188000
int videodemuxbuffersize = DMX_BUFFER_SIZE;

/* maximum number of pes pids */
#define MAXPIDS		32

DemuxEntry demuxfd[MAXPIDS];
unsigned char demuxfd_count = 0;
unsigned char mode;

int staticpids[MAXPIDS];
unsigned char staticpidtype[MAXPIDS];
unsigned char staticpid_count = 0;

int dvrfd = -1;
int capmtfd = -1;

struct sigaction sighandler;

volatile sig_atomic_t exit_flag = 0;
FILE* fdlog;

unsigned char* readptr = NULL;
unsigned char* writeptr = NULL; 
unsigned char* buf = NULL;
unsigned char* endofbuffer = NULL;

unsigned char* smallbuf=NULL;
unsigned char* pmtbuf=NULL;
int pmtbuflen = 0;

long readtotal=0;
long writetotal=0;
int rKBpsPred=0;
int rKBps10=0;
int wKBps=0;

int startup = 1;

struct timeval timeout;
struct timeval *timeoutptr = NULL;

unsigned char *bp;
unsigned char tsfile[1024];
int i = 0;
ushort patversion = 0;
ushort pmtversion = 0;
ushort pmtpid = 0;
ushort pcrpid = 0;
ushort patCC = 0;
ushort pmtCC = 0;
unsigned short int program_number = 0;
int createPMTs = 0;

struct timeval lastreadTimestamp;
struct timeval nextReadTimestamp;
struct timeval lastwriteTimestamp;

fd_set rset;
fd_set wset;
int maxfd = 0;


//============================================ PMT PAT ================================================
static u_long crc_table[256] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};


void tvfix(struct timeval *t1)
{
    while (t1->tv_usec < 0) {
        t1->tv_sec--;
        t1->tv_usec += 1000000;
    }
    while (t1->tv_usec >= 1000000) {
        t1->tv_sec++;
        t1->tv_usec -= 1000000;
    }
}

/*
 * t1 -= t2
 */
void tvsub(struct timeval *t1, const struct timeval *t2)
{
    t1->tv_sec  -= t2->tv_sec;
    t1->tv_usec -= t2->tv_usec;
    tvfix(t1);
}

//difference in msec (t1-t2)
long inline tvdiff(const struct timeval *t1, const struct timeval *t2)
{
	long result = 0;
	result += t1->tv_sec<<10;
	result += t1->tv_usec>>10;
	result -= t2->tv_sec<<10;
	result -= t2->tv_usec>>10;
	
	return result;
}

//add msec to t1
struct timeval tvadd(struct timeval *t1, long msec)
{
	struct timeval result = *t1;
    result.tv_usec += msec<<10;
    result.tv_sec  += msec>>10;
	tvfix(t1);
	return result;
}

//============================================ LOGGING ================================================

int flog(FILE* fdlog, const char *format, ... )
{
	int Chars = 0;
	if (fdlog != NULL)
	{
		va_list Ap;
		va_start( Ap, format );
		struct timeval tv;
		struct tm* ptm;
		char time_string[40];

		/* Obtain the time of day, and convert it to a tm struct. */
		gettimeofday (&tv, NULL);
		ptm = localtime (&tv.tv_sec);
		/* Format the date and time, down to a single second. */
		strftime (time_string, sizeof (time_string), "%d-%m-%Y %H:%M:%S", ptm);
		/* Print the formatted time, in seconds, followed by a decimal point
		and the microseconds. */

		Chars += fprintf(fdlog, "%s.%06ld %d\t", time_string, tv.tv_usec, getpid());
		Chars += vfprintf(fdlog, format, Ap );

		va_end( Ap );
	}
	return Chars;
}

//============================================ processutils (inspired by enigma) ===========================================
long *getProcessID(const char *procname)
{
	struct dirent *entry;
	long *pidList = (long *)malloc(sizeof(long));

	char *name;
	int pid = 0;
	int i = 0;
	char buf[1024], cmdline[40];
	FILE *fp;

	// eDebug("getPID: %s", procname);
	DIR *dir = opendir("/proc");
	if (dir)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			name = entry->d_name;
			if (*name >= '0' && *name <= '9')
			{
				pid = atoi(name);
				sprintf(cmdline, "/proc/%d/cmdline", pid);
	
				if ((fp = fopen(cmdline, "r")) != NULL)
				{
					if ((fread(buf, 1, sizeof(buf) - 1, fp)) > 0)
					{
						if (strstr(buf, procname) != 0)
						{
							pidList = (long *)realloc( pidList, sizeof(long) * (i + 2));
							pidList[i++] = pid;
							// eDebug("getPID:    %d", pid);
						}
					}
					fclose(fp);
				}
			}
		}
		closedir(dir);
	}
	pidList[i] = (i == 0) ? -1 : 0;
	return pidList;
}

void killPID(long pid)
{
	if (pid > 0)
	{
		if (kill(pid, SIGTERM) != 0)
			kill(pid, SIGKILL);
		waitpid(pid, 0, 0);
    }
}

int killOtherZapstreams()
{
	int result = 0;
	long *pidList = getProcessID("zapstream");

	if (*pidList > 0)
	{
		long *pid;
		for (pid = pidList; *pid != 0; pid++)
		{
			if (*pid != getpid())
			{
				killPID(*pid);
				flog(fdlog, "other instance of zapstream killed %d\n", *pid);
				result = 1;
			}
		}			
	}
	fflush(fdlog);
	return result;
}

//============================================ PAT / PMT  ================================================

u_long calc_crc32 (char *data, int len)
{
	register int i;
	u_long crc = 0xffffffff;

	for (i=0; i<len; i++)
		crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *data++) & 0xff];

	return crc;
}

int ts_header(unsigned char *dest, int pusi, int pid, int scrmbl, int adap, int cc)
{
	dest[0] = 0x47;
	dest[1] = (!!pusi << 6) | (pid >> 8);
	dest[2] = pid;
	dest[3] = (scrmbl << 6) | (adap << 4) | (cc & 0x0f);

	return 4;
}

//give transport_stream_id bogus value
//give version an increasing value, not sure if necessary
int createSimplePAT(unsigned char *data, ushort transport_stream_id, char version, ushort program_number, ushort pmtPID)
{
	int slen = 8;  //PATLEN 8
	data[0] = 0x00;                      // table ID;
	data[3] = (transport_stream_id >> 8);// tsid hi
	data[4] = transport_stream_id & 0xFF;// tsid lo
	data[5] = (version<<1)+0x1; 		         // version,cur/next
	data[6] = 0;                         // section no
	data[7] = 0;                         // last section no

	data[slen++] = program_number >> 8;
	data[slen++] = program_number & 0xFF;
	data[slen++] = 0xE0 | (pmtPID >> 8);
	data[slen++] = pmtPID & 0xFF;

	data[1] = 0xB0 | ((slen-3+4) >> 8);   // section length hi
	data[2] = (slen-3+4) & 0xFF;          // section length lo

	ulong crc32 = calc_crc32(data, slen);

	data[slen++] = crc32 >> 24;
	data[slen++] = crc32 >> 16;
	data[slen++] = crc32 >> 8;
	data[slen++] = crc32 & 0xFF;

	return slen;
}

int createPatTs(unsigned char* buf, ushort patCC, ushort patversion, ushort program_number, ushort pmtpid)
{
	int len = 0;
	len += ts_header(buf, 1, 0, 0, 1, patCC);
	
	buf[len++] = 0x00; 

	len += createSimplePAT(buf+len, 0x0451, patversion, program_number, pmtpid);
	for(; len < 188; len++) //fill ts
	{
		buf[len] = 0xff;
	}

	return len;
}

char* getISO639Description(char *iso)
{
	unsigned int i=0;
	for (; i<sizeof(iso639)/sizeof(*iso639); ++i)
	{
		if (!strncasecmp(iso639[i].iso639foreign, iso, 3))
				return iso639[i].description1;
		if (!strncasecmp(iso639[i].iso639int, iso, 3))
				return iso639[i].description1;
	}
	//HACK. return the inputparameter. This only works if *iso lives at least as long the return value
	return iso;
}

//Finds the part of the pmt where a language is defined. 
//Finds by language ie 'english', 'Tonoption 2'
unsigned char* matchPreferedLanguage(unsigned char* pmt, int pmtlen, int preferedpid, char* audiochannel)
{
	char isolanguage[4];
	int infloopprotector = 100;
	int preferedchannelnr = -1;
	sscanf(audiochannel, "Tonoption %d", &preferedchannelnr);
	int channelnr = 1;
	
	if (logaudiopriority>1) flog(fdlog, "check language %s ...\n", audiochannel);
	
	int src = 11;
	//flog(fdlog, "skip %d\n", pmt[src]);
	src += pmt[src]+1; //skip program info length
	
	//loop over audio channels in pmt
	while(src < pmtlen-4 && infloopprotector > 0)
	{

		int streamtype = pmt[src];
		int pid = ((pmt[src+1]&0x1f)<<8)+pmt[src+2];
		int esinfolen = ((pmt[src+3]&0x0f)<<8)+pmt[src+4];
		
		//Handle "http://dreambox:31344/langpid=0x60" 
		if (preferedpid > 0)
		{
			if (pid == preferedpid)
			{
				if (logaudiopriority>0) flog(fdlog, "prefer audiopid=%d\n", preferedpid);
				return pmt+src; 
			}
			else
			{
				if (logaudiopriority>0) flog(fdlog, "no equal pids 0x%x != 0x%x\n", pid, preferedpid);
			}
		}
		else
		{
			//handle "deutsch" "english"
			if (streamtype == 3 || streamtype == 4)
			{
				int i = 5;
				while((src+i < pmtlen-4) && i < esinfolen && infloopprotector > 0)
				{
					if(pmt[src+i] == 10) //ISO_639_language_descriptor
					{
						isolanguage[0] = pmt[src+i+2];
						isolanguage[1] = pmt[src+i+3];
						isolanguage[2] = pmt[src+i+4];
						isolanguage[3] = 0;
						
						//Handle enigma config  audiochannelspriority=english#deutsch#...
						//Handle "Tonoption 2", but only when we're in .
						//note: pw has channels with 3 audiotracks with iso 'deutsch' => 2nd channels is the original sound track
						//mtv has channels with 2 audiotracks without iso descriptor => leave pmt intact, 
						if (channelnr == preferedchannelnr)
						{
							if (logaudiopriority>0) flog(fdlog, "prefer Tonoption %d\n", preferedchannelnr);
							return pmt+src;
						}
						
						if (strncasecmp(isolanguage, audiochannel, strlen(audiochannel)) == 0)
						{
							if (logaudiopriority>0) flog(fdlog, "match on isolanguage %s (%s) \n", isolanguage, audiochannel);
							return pmt+src;
						}

						char * iso639description = getISO639Description(isolanguage);
						if (logaudiopriority>1) flog(fdlog, "... %s (%s)\n", iso639description, isolanguage);
						
						if (strncasecmp(iso639description, audiochannel, strlen(audiochannel)) == 0)
						{
							if (logaudiopriority>0) flog(fdlog, "match on isodescription %s (%s) \n", iso639description, audiochannel);
							return pmt+src;
						}
					}
					
					i += pmt[src+i+1]+2;
					infloopprotector--;
				}
				channelnr++;
			}

		}
		src += 5 + esinfolen; //skip esinfo_length
		infloopprotector--;
	}
	if (infloopprotector == 0)
	{
		if (logaudiopriority>0) flog(fdlog, "error parsing pmt\n");
	}
	return NULL;
}

unsigned char* findPreferedLanguage(unsigned char* pmt, int pmtlen, int preferedpid)
{
	unsigned char* preferedLanguage = NULL;
	char* last = NULL;
	char tempaudiochannelspriority[301];
	strncpy(tempaudiochannelspriority, audiochannelspriority, 300);
	
	if (strlen(tempaudiochannelspriority)>0)
	{
		//check preferences from url
		if (preferedpid > 0 || strlen(preferedaudiolanguage) > 0)
		{
			preferedLanguage = matchPreferedLanguage(pmt, pmtlen, preferedpid, preferedaudiolanguage);
		}
		
		//loop over preferences in configfile
		char *audiochannel = strtok_r(tempaudiochannelspriority, "#", &last);
		while (preferedLanguage == NULL && audiochannel != NULL)
		{
			preferedLanguage = matchPreferedLanguage(pmt, pmtlen, 0, audiochannel);
			audiochannel = strtok_r(NULL, "#", &last);
		}
	}
	return preferedLanguage; //no preference found
}

unsigned char tempbuf[20000];
void reorderAudioStreamsInPmtPes(unsigned char* pmt, int pmtlen)
{
	int infloopprotector = 100;
	
	if (pmtlen > 20000) return;

	int dst = 0;
	int src = 0;
	int done = 0;
	int error = 0;
	
	src += 11;
	src += pmt[src]+1; //skip program info length
	memcpy(tempbuf, pmt, src);
	dst = src;

	unsigned char* preferedLanguage = findPreferedLanguage(pmt, pmtlen, preferedaudiopid); //todo fill in preferedPid
	
	while(src < pmtlen-4 ) //walk through streams in pmt
	{
		int streamtype = pmt[src];
		int len = (pmt+src)[4]+5;
		if (src+len > pmtlen-4) 
		{
			if (logaudiopriority>0) flog(fdlog, "error reordering audio channels 1 %d > %d+1\n", src+len, pmtlen); 
			//error = 1;
			break;
		}
				
		if (done==0&&(streamtype == 3 || streamtype == 4))
		{
			if (preferedLanguage != NULL)
			{
				if (logaudiopriority>0) flog(fdlog, "insert prefered language\n");
				int desclen = ((preferedLanguage[3]&0x0f)<<8)+preferedLanguage[4] + 5;
				if (desclen+dst > pmtlen) 
				{
					if (logaudiopriority>0) flog(fdlog, "error reordering audio channels 2\n"); 
					error = 1;
					break;
				}
				memcpy(tempbuf+dst, preferedLanguage, desclen);
				dst += desclen;
			}
			done = 1;
		}
		
		if (pmt+src == preferedLanguage)
		{
			src += len;
			if (logaudiopriority>1) flog(fdlog, "skip prefered language\n");
		}
		else
		{
			if (logaudiopriority>1) flog(fdlog, "insert other part\n");
			memcpy(tempbuf+dst, pmt+src, len);
			src += len;
			dst += len;
		}
		if (--infloopprotector < 0)
		{
			if (logaudiopriority>0) flog(fdlog, "inf loop protector\n");
			break;
		}
	}

	if (pmtlen - src > 0)
	{
		memcpy(tempbuf+dst, pmt+src, pmtlen-src);
	}
	//copy back to pmt
	if (error == 0)
	{
		memcpy(pmt, tempbuf, pmtlen);

		ulong crc32 = calc_crc32 (pmt, pmtlen-4);
		pmt[pmtlen-4] = (crc32 & 0xff000000) >> 24;
		pmt[pmtlen-3] = (crc32 & 0x00ff0000) >> 16;
		pmt[pmtlen-2] = (crc32 & 0x0000ff00) >>  8;
		pmt[pmtlen-1] = (crc32 & 0x000000ff) >>  0;	
		
	}
}

int createPmtTsFromPmtPes(unsigned char* buf, DemuxEntry* de, unsigned char* pmt, int pmtlen)
{
	int dst = 0;
	int src = 0;
	int todo = pmtlen;
	int leftInTs = 0;
	
	int len = 0;
	int c = 0;
	if (pmtversion != ((pmt[5]>>1)&0x001f))
	{
		pmtversion = (pmt[5]>>1)&0x001f;
		if (logreadspeed>0)
		{
			flog(fdlog, "new pmt version = %d\n", pmtversion); //version
		}
	}

	if (useaudiopriority>0)
	{
		reorderAudioStreamsInPmtPes(pmt, pmtlen);
	}
	
	while (src < pmtlen && c < 50)
	{
		dst += ts_header(buf+dst, (c==0)?1:0, de->pid, 0, 1, de->count);
		de->count = (de->count+1)%16;
		
		if (c==0)
		{
			buf[dst] = 0x00;
			dst++;
		}

		todo = pmtlen-src;
		leftInTs = 188-(dst%188);
		len = (todo > leftInTs)?leftInTs:todo;
		
		if (src+len > pmtlen) {flog(fdlog, "error reconstructing pmt ts\n"); break;}

		memcpy(buf+dst, pmt+src, len);

		src += len;
		dst += len;

		c++;
	}
	int pad = 188-(dst%188);
	if ( pad < 188)
	{
		memset(buf+dst, 0xff, pad);
		dst += pad;
	}

	return dst;
}

//============================================ DEMUX CODE ================================================
void setInputOptions(int fd, int blocking )
{
	int res = 0;
	res = fcntl (fd, F_GETFL, NULL);
	if ( res < 0) return;
	
	if (blocking == 1)
	{
		res &= ~O_NONBLOCK;	
	}
	else
	{
		res |= O_NONBLOCK;	
	}

	res = fcntl (fd, F_SETFL, res);
	if (res == -1){perror("set (non) blocking failed");}
}

void InitDemuxEntry(DemuxEntry* de, int pid, streamtypes type, uchar streamtype, uchar esinfolength, uchar* esinfo)
{
	de->pid = pid;
	de->fd = 0;
	de->count = -1;
	de->type = type;
	de->streamtype = streamtype;
	de->esinfolength = esinfolength;
	de->esinfo = esinfo;
	flog(fdlog, "demuxentry inited %x of type %d\n", de->pid, de->type);
}

int OpenPesFilter(DemuxEntry* de)
{
	struct dmx_sct_filter_params sct;
	de->count = 0;
	
	if (de->fd > 0) return de->fd;
	
	flog(fdlog, "open %spes pid %x\n", immediatestart?"and start ":"", de->pid);

	if ((de->fd = open(DMXDEV, O_RDWR|O_NONBLOCK)) < 0)
		goto error;	

	if (de->type == streamtype_pmt)
	{
		sct.pid = de->pid;
#if HAVE_DVB_API_VERSION < 3
        	memset(sct.filter.filter, 0, DMX_FILTER_SIZE);
		memset(sct.filter.mask, 0, DMX_FILTER_SIZE);
#else
		memset(&sct.filter, 0, sizeof(struct dmx_filter));
#endif

		sct.timeout = 0;
		//sct.output = DMX_OUT_TS_TAP;
		sct.flags = immediatestart?DMX_IMMEDIATE_START:0;

		if (ioctl(de->fd, DMX_SET_FILTER, &sct) < 0)
			goto error;	
	}
	else
	{
		struct dmx_pes_filter_params flt;
		flt.pid = de->pid;
		flt.input = DMX_IN_FRONTEND;
		flt.output = DMX_OUT_TS_TAP;
		flt.pes_type = DMX_PES_OTHER; 
		flt.flags = immediatestart?DMX_IMMEDIATE_START:0;
		if (de->type == streamtype_video)
		{
			flog(fdlog, "set videodemuxbuffersize %d\n", videodemuxbuffersize);
			if (ioctl(de->fd, DMX_SET_BUFFER_SIZE, videodemuxbuffersize) < 0)
				goto error;	
		}

		if (ioctl(de->fd, DMX_SET_PES_FILTER, &flt) < 0)
			goto error;	
	}
	
	flog(fdlog, "open pes pid %x => %d\n", de->pid, de->fd);
	fflush(fdlog);
	return de->fd;
	
error:
	flog(fdlog, "open pes pid %d failed (%d) %s\n", de->pid, de->fd, strerror(errno));
	de->fd = 0;
	fflush(fdlog);
	return -1;		
}

void ClearPesFilter(DemuxEntry* de) 
{
	if (de->fd == 0 ) return;
	if (de->esinfo != NULL)
	{
		free(de->esinfo);
	}
	flog(fdlog, "close pes fd %d\n", de->fd);
	ioctl(de->fd, DMX_STOP);
	close(de->fd);
	de->fd = 0;
	de->count = 0;
}

int StartPesFilter(DemuxEntry* de)
{
	if (!immediatestart)
	{
		flog(fdlog, "starting demuxentry %x\n", de->pid);
		if (de->fd == 0) return -1;

		if (ioctl(de->fd, DMX_START) < 0) {
			flog(fdlog, "can't start demux: %s\n", strerror(errno));
			return -1;
		}
	}
	return 0;
}

int StopPesFilter(DemuxEntry* de)
{
	if (de->fd == 0) return -1;
	flog(fdlog, "stopping demuxentry %x\n", de->pid);
	if (ioctl(de->fd, DMX_STOP) < 0) {
		flog(fdlog, "can't stop demux: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}	
void ClosePesFilter(DemuxEntry* de)
{
	if (de->fd == 0 ) return;
	flog(fdlog, "close pes fd %d\n", de->fd);
	ioctl(de->fd, DMX_STOP);
	close(de->fd);
	de->fd = 0;
	return;
}

void setSocketOptions(int fd)
{
	int res = 0;
	
	res = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &so_sndbuf, sizeof(so_sndbuf));
		flog(fdlog, "consumer) so_sndbuf <= %i = %i\n", so_sndbuf, res);

	res = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf, sizeof(so_rcvbuf));
		flog(fdlog, "consumer) so_rcvbuf <= %i =%i \n", so_rcvbuf, res);

 	res = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&tcp_nodelay, (socklen_t)sizeof(tcp_nodelay));
	//	flog(fdlog, "consumer) so_nodelay <= %i = %i\n", sockopt, res);

	res = setsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, (char *)&tcp_maxseg, (socklen_t)sizeof(tcp_maxseg));
		//flog(fdlog, "consumer) so_maxseg <= %i = %i\n", sockopt, res);

	res = fcntl (fd, F_SETFL, O_NONBLOCK);
	//if (res == -1){perror("set non blocking failed");}

}

int togo(unsigned char* readptr, unsigned char* writeptr)
{
	int len = readptr-writeptr;
	
	if (len < 0)
	{
		len = endofbuffer-writeptr;
	}	
	return len;
}

int bytesinbuffer(unsigned char* readptr, unsigned char* writeptr)
{
	int len = readptr-writeptr;
	
	if (len < 0)
	{
		len = (endofbuffer-writeptr)+(readptr-buf);
	}	
	return len;
}

void cleanup()
{
	if (mode != 3)
	{
		while (demuxfd_count > 0)
			ClearPesFilter(&demuxfd[--demuxfd_count]);
	}

	if (capmtfd >= 0)
	{
		close(capmtfd);
		capmtfd = -1;
	}
	close(dvrfd);
	free(buf);
	free(smallbuf);
	free(pmtbuf);
	fclose(fdlog);	
}

void signalHandler(int d, siginfo_t * info, void * r)
{
	flog(fdlog, "sighandler\n");
	exit_flag = 1;
	
	cleanup();
	//exit (implicit)
}


void addStaticPids()
{
	int i=0;
	for(i = 0; i < staticpid_count; i++)
	{
		switch(staticpidtype[i])
		{
			case 0:
				InitDemuxEntry(&demuxfd[demuxfd_count++], staticpids[i], streamtype_pmt, 0, 0, NULL);
				break;
			case 2:
				InitDemuxEntry(&demuxfd[demuxfd_count++], staticpids[i], streamtype_video, 2, 0, NULL);
				break;
			case 4:
				InitDemuxEntry(&demuxfd[demuxfd_count++], staticpids[i], streamtype_audio, 4, 0, NULL);
				break;
			default:
				InitDemuxEntry(&demuxfd[demuxfd_count++], staticpids[i], streamtype_other, 0, 0, NULL);
				break;
		}
	}
}

char * stateToString(states state)
{
	switch(state)
	{
		case st_init: return str_init;
		case st_streaming: return str_streaming;
		case st_waitforcapmt: return str_waitforcapmt;
		case st_waitforpmt: return str_waitforpmt;
		case st_waitafterzap: return str_waitafterzap;
		case st_startstreaming: return str_startstreaming;
		case st_stopstreaming: return str_stopstreaming;
	}
	return str_unknownstate;
}

void setState(states state)
{
	flog(fdlog, "setState %s => %s\n", stateToString(st), stateToString(state));
	st = state;
}

int getPID(unsigned char* buf)
{
	if (buf[0] == 0x47)
	{
		return ((buf[1]& 0x1F)<<8) | buf[2];	
	}
	return -1;
}

int getCC(unsigned char* buf)
{
	if (buf[0] == 0x47)
	{
		return (buf[3]& 0xF);	
	}
	return -1;
}

int getDiscontinuityIndicator(unsigned char* buf)
{
	if (buf[0] == 0x47)
	{
		if (buf[3] & 0x20) {
			/* adaption field present, check for discontinuity_indicator */
			if ((buf[4] > 0) && (buf[5] & 0x80))
				return 1;
		}
	}
	return 0;
}

unsigned char payload(const char*tsp)
{
	if (!(tsp[3] & 0x10))	// no payload?
		return 0;

	if (tsp[3] & 0x20) {	// adaptation field?
		if (tsp[4] > 183)	// corrupted data?
			return 0;
		else
			return 184 - 1 - tsp[4];
	}

	return 184;
}

void checkContinuity(unsigned char* writeptr, int len)
{
	unsigned char* p = writeptr;
	//sync
	unsigned char* scanTo = writeptr+len;
	int synced = 0;
	while (p < scanTo)
	{
		if (*p == 0x47)
		{
			int packets = 4;
			unsigned char* i = p;
			while(i < scanTo && packets > 0)
			{
				if (*i == 0x47)
				{
					packets--;
				}
				i += 188;
			}
			if (packets == 0 || i >= scanTo)
			{
				synced = 1;
				break;
			}
		}
		p++;
	}
	if (synced==1 && p > writeptr+188)
	{
		flog(fdlog, "lost and found sync %x %d\n", writeptr, len);
	}
	if (synced==0 && p < scanTo)
	{
		flog(fdlog, "lost sync %x %d\n", writeptr, len);
	}
	else
	{
		int pid = 0;
		int cc = 0;
		while( p < writeptr+len && (pid = getPID(p)) >= 0 && *p == 0x47)
		{
			int count = payload(p);

			if (count > 0)		/* count == 0 if no payload or out of range */
			{
				cc = getCC(p);
				int discontinuity = getDiscontinuityIndicator(p);

				//flog(fdlog, "disc pid=%x, cc=%d, disc=%d\n", pid, cc, discontinuity);
				
				int j = 0;
				for(; j < demuxfd_count; j++)
				{
					if (demuxfd[j].pid == pid && demuxfd[j].type != streamtype_pcr)
					{
						int ccok = ((demuxfd[j].count + 1) & 0x0f) == cc;
						
						if ((!ccok || discontinuity))
						{
							flog(fdlog, "discontinuity received, %d instead of %d, %d bytes lost, di=%d (pid=%x) i=%d\n", cc, demuxfd[j].count+1, count, discontinuity, pid, p-writeptr);
						}
						demuxfd[j].count = cc;
	
					}
				}
			
			}
			p+=188;
		}
	}
}


void processCAPMT()
{
	flog(fdlog, "data from capmt\n");
	fflush(fdlog);			
	int length = 0;
	int programinfo_length = 0;
	int lengthdatasize = 0;
	int readcount;
	unsigned char buffer[1024];
	
	readcount = 0;
	if (read(capmtfd, &buffer[readcount], 4) <= 0)
	{
		exit_flag=1;
	}
	readcount += 4;
	if (buffer[3] & 0x80)
	{
		/* multibyte length field */
		int i;
		lengthdatasize = buffer[3] & 0x7f;
		if (read(capmtfd, &buffer[readcount], lengthdatasize) <= 0) return;
		readcount += lengthdatasize;
		for (i = 0; i < lengthdatasize; i++)
		{
			length = (length << 8) | buffer[i + 4];
		}
	}
	else
	{
		/* singlebyte length field */
		length = buffer[3] & 0x7f;
	}
	if (read(capmtfd, &buffer[readcount], length) <= 0) return;
	readcount += length;
	if (!memcmp(buffer, "\x9F\x80\x32", 3)) /* CAPMT object */
	{
		int i;
		int descriptor_length = 0;
		unsigned char *data = &buffer[4 + lengthdatasize];
		if (data[6] == 0x01 && (data[0] == 0x01 || data[0] == 0x03 || data[0] == 0x05)) /* ok_descramble command, first or only object in the list, or an update */
		{
			//unsigned char pmtdata[2048];
			int slen = PMT_LEN;   // 12

			program_number = *(unsigned short *)&data[1];
			//unsigned short int provider = 0;
			//unsigned char version = data[3];
			unsigned short int pid = 0;
			int prog_info_len = 0;
			int es_info_length = 0;
			//unsigned int crc32;

			/* remove previous filters */
			while (demuxfd_count > 0)
			{ 
				ClearPesFilter(&demuxfd[--demuxfd_count]);
			}
			
			if (mode != 3)
			{
				if (dvrfd >= 0) close(dvrfd);
				if ((dvrfd = open(DVRDEV, O_RDONLY)) < 0)
				{
					exit_flag = 1;
					return;
				}
			}
			flog(fdlog, "zap detected\n");
			fflush(fdlog);

			demuxfd_count = 0;
			pmtbuflen = 0;

			programinfo_length = *(unsigned short *)&data[4] & 0xfff;

			for (i = 0; i < programinfo_length - 1; i += descriptor_length + 2)
			{
				descriptor_length = data[i + 8];
				if (data[i + 7] == 0x81)
				{
					/* private descr: dvb namespace */
				}
				else if (data[i + 7] == 0x82)
				{
				}
				else if (data[i + 7] == 0x84)
				{
					/* pmt pid */
					pid = ((data[i + 9] & 0xff ) << 8) + (data[i + 10] & 0xff);
					
					flog(fdlog, "PMT = %x\n", pid);
					pmtpid = pid;
				}
				else
				{
					prog_info_len += descriptor_length + 2;
				}
			}

			InitDemuxEntry(&demuxfd[demuxfd_count++], pmtpid, streamtype_pmt, 0, 0, NULL);
			
			if (pmtpid != 0)
			{
				for (i = programinfo_length + 6; i < length; i += es_info_length + 5)
				{
					int j;
					uchar stream_type = data[i];
					es_info_length = *(unsigned short *)&data[i + 3] & 0xfff;
					if (stream_type == 1 || stream_type == 2)
					{
						/* video */
						pid = *(unsigned short*)&data[i + 1];
						InitDemuxEntry(&demuxfd[demuxfd_count++], pid, streamtype_video, stream_type, 0, NULL);
					}
					else if (stream_type == 3 || stream_type == 4)
					{
						/* audio */
						pid = *(unsigned short*)&data[i + 1];
						InitDemuxEntry(&demuxfd[demuxfd_count++], pid, streamtype_audio, stream_type, 0, NULL);
					}
					else if (stream_type == 6 && *(unsigned short*)&data[i + 1] > 0x50)
					{
						/* audio */
						pid = *(unsigned short*)&data[i + 1];
						InitDemuxEntry(&demuxfd[demuxfd_count++], pid, streamtype_audio, stream_type, 0, NULL);
					}
					for (j = 0; j < es_info_length - 1; j += descriptor_length + 2)
					{
						descriptor_length = data[i + j + 7];
						slen += descriptor_length + 2;
					}
				}
				addStaticPids();
				
				//arbitrary delay till enigma and emu are done
				setState(st_waitafterzap);
				waitcounter = 0;
				if (startup == 1)
				{
					timeout.tv_sec = 0;
					timeout.tv_usec = 50; //no delay on startup
				}
				else
				{
					timeout.tv_sec = 0;
					timeout.tv_usec = delayafterzap;
				}
				tvfix(&timeout);
				timeoutptr = &timeout;
				startup = 0;

				flog(fdlog, "pmtpid = %d %x, waiting to initialise\n", pmtpid, pmtpid);
				fflush(fdlog);

				//read stale data
				int res;
				res = fcntl (dvrfd, F_SETFL, O_NONBLOCK);					
				do
				{
					res = read(dvrfd, buf, IN_SIZE);
				} while (res > 0);
				
			}
			else
			{
				flog(fdlog, "pmt = 0\n"); //weird!
				fflush(fdlog);						
			}
		}
		
		//display capmt
		// for(i = 0; i < readcount; i++)
		// {
			// if ((i%8) == 0 && (i != 0)) fprintf(fdlog, "\n");
			// fprintf(fdlog, "0x%x ", buffer[i]);
		// }
		flog(fdlog, "we're waiting..\n");
		fflush(fdlog);						
	}
	return;
}


void processDVR()
{
	int r = 0;
	int wrappedaround = 0;
	readtotal = 0;
	int oneread = 188*500;
	
	//To play a recording in vlc a pat and pmt must be present
	if (readptr == buf && st==st_streaming && mode!=3 && *(endofbuffer-188) == 0x47)
	{
		readptr += createPatTs(readptr, patCC++, patversion, program_number, pmtpid);
		if (pmtbuflen > 0)
		{
			int s = createPmtTsFromPmtPes(readptr, &demuxfd[i], pmtbuf, pmtbuflen);
			readptr+=s;
		}
		if (((readptr-buf) %188)>0)
		{
			flog(fdlog, "error!!!!!!!!!!!! %s!=0 %x %x\n", readptr-buf, readptr, buf);
		}
	}
	
	int todo = oneread;
	if (readptr + todo > buf+bufsize)
	{
		todo = (buf+bufsize) - readptr; 
	}
	
	//make sure we only do reads of a multiple of 188
	if (todo > 752 && (todo % 752 > 0))
	{
		todo = todo - (todo%752);
	}

	r = read(dvrfd, readptr, todo);
	if (st != st_streaming)
	{
		flog(fdlog, "read %d bytes from dvr r(%x) w(%x) state %d\n", r, readptr, writeptr, st);
	}

	if (r < 0)
	{
		if (errno == EOVERFLOW ) 
		{
			flog(fdlog, "demux overflow\n");
		}
	}
	else
	{
		if (logdiscontinuity > 0)
		{
			checkContinuity(readptr, r);
		}
		readptr += r;
		readtotal += r;
			
		if (readptr > buf+bufsize-20000) // don't fill the buffer entirely, prevents non 188x reads and small reads
		{
			endofbuffer = readptr;
		}

		//buffer overflow detection
		if ((writeptr > readptr-r) && (writeptr < readptr))
		{
			flog(fdlog, "buffer overflow\n");
			//fflush(fdlog);
			writeptr = readptr-r;
			//break;
		}

		if (readptr >= endofbuffer)
		{
			if (readptr >= endofbuffer+1) //just checking
			{
				flog(fdlog, "readptr beyond end of buf\n");
			}
			readptr = buf;
			if (logreadspeed == 2)
			{
				flog(fdlog, "buffer wrapped around\n");
			}
			wrappedaround = 1;
		}
	}

		
	if (st == st_streaming)
	{
		struct timeval prevreadTimestamp = lastreadTimestamp;
		gettimeofday (&lastreadTimestamp, NULL);

		long msec = tvdiff(&lastreadTimestamp, &prevreadTimestamp);
		if (msec > 0)
		{
			int newsample = (readtotal / msec);
			if (newsample > rKBpsPred)
			{
				rKBpsPred = newsample + (newsample-rKBpsPred); //fast rise, should probably check the mpeg2 specs for dynamic buffer specs
			}
			else
			{
				rKBpsPred = (2*rKBpsPred + newsample)/3; //slow decay
			}
		}
		else
		{
			msec = 1;
		}
		rKBps10 = (9*rKBps10 + (readtotal / msec))/10; 

		if ((logreadspeed == 1 && wrappedaround) || logreadspeed == 2)
		{
			flog(fdlog, "[speedsample] read %ld bytes in %ld msec = %d kB/s todo=%dKB\n", 
				readtotal, msec, rKBps10, bytesinbuffer(readptr, writeptr)>10);
		}
	}
	if (st != st_streaming)
	{
		usleep(20000);
	}
}

void processOut()
{
	int r = 1;
	int len = togo(readptr, writeptr);
	int startlen = len;
	int writecount = 0;
	writetotal = 0;
	int wrappedaround = 0;
	struct timeval now;

	gettimeofday (&now, NULL);
	lastwriteTimestamp = now;

	//while the socket is accepting data and ( we have more than 4k to write or we're at the end of the buffer)
	while ((len > 20*188 || writeptr >= endofbuffer-20*188 || st != st_streaming) && r > 0) 
	{
			
		if (len > 250*188)
		{
			len = 250*188;
		}
		r = write(STDOUT_FILENO, writeptr, len);

		//flog(fdlog, "write %d %d\n", r, len);
		
		if (r < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				r = 0;
			} 
			else
			{
//						perror("write stdout");
				exit_flag = 1;
				break;
			}
		}
		
		writeptr += r;
		writetotal += r;
		if (writeptr >= endofbuffer)
		{
			writeptr = buf;
			wrappedaround = 1;
		}
		len = togo(readptr, writeptr);

		writecount++;

		//don't starve the reading, hmm might be better to just use threads after all...
		if( writecount > 3)
		{
			gettimeofday (&now, NULL);
			
			//if the dvr has data waiting stop writing
			long msec = tvdiff(&nextReadTimestamp, &now);
			if (msec < 0)
			{
				break;
			}
		}
	}
	if (st == st_streaming)
	{
		if ((logwritespeed == 1 && wrappedaround) || logwritespeed >= 2)
		{
			struct timeval prevwriteTimestamp = lastwriteTimestamp;
			lastwriteTimestamp = now;
			
			long msec = tvdiff(&lastwriteTimestamp, &prevwriteTimestamp);
			if (msec > 0)
			{
				wKBps = (2*wKBps+ (writetotal / msec))/3; //avg KBps
			}
	
			flog(fdlog, "[speedsample] written %ld in %ld msec = %d kB/s start=%dKB todo=%dKB\n", 
				writetotal, msec, wKBps, startlen>>10, togo(readptr, writeptr)>>10);
		}
	}			
}

unsigned char* openConfig(unsigned char* configfilename)
{
	struct stat sb;
	FILE* etcconfig;
	int bsize;
	unsigned char* configbuf;
	
	if (stat(configfilename, &sb) != 0)
	{
		printf("no configfile %s\n", configfilename);
		return NULL;
	}
	etcconfig = fopen(configfilename, "r");
	if (etcconfig == NULL)
	{
		printf("open configfile %s failed (%s)\n", configfilename, strerror(errno));
		return NULL;
	}
	
	bsize = sb.st_size;
	configbuf = malloc(bsize+1);

	fread(configbuf, bsize, 1, etcconfig);
	fclose(etcconfig);

	configbuf[bsize] = 0;
	
	return configbuf;
}

void readConfig(unsigned char* configfilename)
{
	unsigned char* ptr;
	unsigned char* configbuf = openConfig(configfilename);

	if (configbuf == NULL) return;
	ptr = configbuf;
	
	while(ptr != NULL)
	{
		int extrapid = 0;
		sscanf(ptr, " logfile = %100s ", logfile);
		sscanf(ptr, " pmtsocket = %100s ", pmtsocket);
		sscanf(ptr, " enigmaconfigfile = %100s ", enigmaconfigfile);
		sscanf(ptr, " logreadspeed = %i ", &logreadspeed);
		sscanf(ptr, " logwritespeed = %i ", &logwritespeed);
		sscanf(ptr, " logaudiopriority = %i ", &logaudiopriority);
		sscanf(ptr, " delayafterzap = %i ", &delayafterzap);
		sscanf(ptr, " extrapid = %i ", &extrapid);
		if (extrapid != 0)
		{
			staticpids[staticpid_count] = extrapid;
			staticpidtype[staticpid_count] = -1;		
			staticpid_count++;
		}
		sscanf(ptr, " buffersize = %i ", &bufsize);
		sscanf(ptr, " videodemuxbuffersize = %i ", &videodemuxbuffersize);
		sscanf(ptr, " defaultpriority = %i ", &defaultpriority);
		sscanf(ptr, " streamingpriority = %i ", &streamingpriority);
		sscanf(ptr, " so_sndbuf = %i ", &so_sndbuf);
		sscanf(ptr, " so_rcvbuf = %i ", &so_rcvbuf);
		sscanf(ptr, " tcp_nodelay = %i ", &tcp_nodelay);
		sscanf(ptr, " tcp_maxseg = %i ", &tcp_maxseg);
		sscanf(ptr, " useaudiopriority = %i ", &useaudiopriority);
		sscanf(ptr, " logdiscontinuity = %i ", &logdiscontinuity);
		sscanf(ptr, " audiochannelspriority = %300[a-zA-Z0-9# ,()_-/\\] ", audiochannelspriority);
		sscanf(ptr, " immediatestart = %i ", &immediatestart);	

		if (bufsize < 100000) 
		{
			bufsize = 100000;
		}
		
		ptr = strstr(ptr, "\n");
		if (ptr!=NULL) ptr++;
	}
	free(configbuf);
}

void readEnigmaConfig(unsigned char* configfilename)
{
	unsigned char* ptr;
	unsigned char* configbuf = openConfig(configfilename);
	
	if (configbuf == NULL) return;
	ptr = configbuf;
	
	while(ptr != NULL)
	{
		//only read this setting from the enigmaconfig if it isn't overriden in zapstream.conf
		if (strlen(audiochannelspriority) == 0)
		{
			sscanf(ptr, " s:/extras/audiochannelspriority = %300[a-zA-Z0-9# _-] ", audiochannelspriority);
		}
		
		ptr = strstr(ptr, "\n");
		if (ptr!=NULL) ptr++;
	}
	free(configbuf);
}

void parseTsUrl(unsigned char* bp)
{
	char* last = NULL;
	/* parse stdin / url path, start dmx filters */
	char *param = strtok_r(bp, "&", &last);
	
	int pid;
	while (param != NULL)
	{
		if (sscanf(param, "pmt=%i", &pid))
		{
			flog(fdlog, "match pmt=0x%x\n", pid);
			staticpids[staticpid_count] = pid;
			staticpidtype[staticpid_count] = 0;
			staticpid_count++;			
		}
		if (sscanf(param, "apid=%i", &pid))
		{
			flog(fdlog, "match apid=0x%x\n", pid);
			staticpids[staticpid_count] = pid;
			staticpidtype[staticpid_count] = 4;
			staticpid_count++;
		}
		if (sscanf(param, "vpid=%i", &pid))
		{
			flog(fdlog, "match vpid=0x%x\n", pid);
			staticpids[staticpid_count] = pid;
			staticpidtype[staticpid_count] = 2;
			staticpid_count++;
		}
		if (sscanf(param, "lang=%s", preferedaudiolanguage))
		{
			flog(fdlog, "match lang=%100s\n", preferedaudiolanguage);
		}
		if (sscanf(param, "langpid=%i", &pid))
		{
			flog(fdlog, "match langpid=0x%x\n", pid);
			preferedaudiopid = pid;
		}
		sscanf(param, " HTTP/%i.%i", &httpmajor, &httpminor);
		
		param = strtok_r(NULL, "&", &last);
	}
	if (httpmajor == 0 && httpminor == 0)
	{
		httpmajor = 0;
		httpminor = 9;
	}
	flog(fdlog, "http protocol version %i.%i\n", httpmajor, httpminor);
	
	return;
}

void parseTsFileUrl(unsigned char* bp)
{
	/* ts filename */
	strcpy(tsfile, bp);
	for (i = 0; i < strlen(bp) - 3; i++) 
	{
		if ((tsfile[i] == '.') && (tsfile[i + 1] == 't') && (tsfile[i + 2] == 's'))
		{
			tsfile[i + 3] = '\0';
			break;
		}
	}
	
	if (bp == NULL || !sscanf(bp, "HTTP/%i.%i", &httpmajor, &httpminor))
	{
		httpmajor = 0;
		httpminor = 9;
	}	
}

int parseRequest(char** argv)
{
	unsigned char* bp = buf;

	/* read one line */
	while (bp - buf < IN_SIZE) {
		unsigned char c;
		read(STDIN_FILENO, &c, 1);
		if ((*bp++ = c) == '\n')
			break;
	}
	
	*bp++ = 0;
	flog(fdlog, "request %s\n", buf);
	bp = buf;

	/* send response to http client */
	if (!strncmp(buf, "GET /", 5)) {
		
		bp += 5;

		if (mode == 2)
		{
			parseTsUrl(bp);
		}
		if (mode == 3)
		{
			parseTsFileUrl(bp);
		}
		if (httpmajor >= 1 && httpminor >= 0)
		{
			printf("HTTP/%i.%i 200 OK\r\n", httpmajor, httpminor);
			printf("Content-Type: video/x-mpeg2-ts\r\n");
			printf("Server: %s (%s)\r\n", zapstream, &argv[1][1]); 
			printf("Connection: Keep-Alive\r\n");
			printf("\r\n");
			
		}
		fflush(stdout);
				
		return 1;
	}
	else
	{
		printf("HTTP/1.1 400 Bad request\r\nServer: %s (%s)\r\n\r\n", zapstream, &argv[1][1]);
		fflush(stdout);
		return 0;
	}	
}

void setupSignalHandler()
{
	sigemptyset(&(sighandler.sa_mask));
	sighandler.sa_sigaction = signalHandler;
	sighandler.sa_flags = SA_RESETHAND | SA_SIGINFO;//SA_ONESHOT;

	sigaction( SIGPIPE, &sighandler, NULL);
	sigaction( SIGTERM, &sighandler, NULL);
}

void readConfigFiles()
{
	struct stat sb;
	if (stat(configfile1, &sb) == 0)
	{
		readConfig(configfile1);
	}
	else
	if (stat(configfile2, &sb) == 0)
	{
		readConfig(configfile2);
	}
	
	if (stat(enigmaconfigfile, &sb) == 0)
	{
		readEnigmaConfig(enigmaconfigfile);
	}
}

int main (int argc, char ** argv)
{
	lastreadTimestamp.tv_sec = 0;
	lastreadTimestamp.tv_usec = 0;
	gettimeofday (&lastreadTimestamp, NULL);

	lastwriteTimestamp = lastreadTimestamp;
	nextReadTimestamp = lastreadTimestamp;
	
	setState(st_init);
	waitcounter = 0;
	
	setupSignalHandler();
	readConfigFiles();
	
	if (strlen(logfile) > 0)
	{
		fdlog = fopen(logfile, "a");
	}

	flog(fdlog, " Log opened (bufsize=%d)\n", bufsize);
	fflush(fdlog);

	killOtherZapstreams();	

	setpriority( PRIO_PROCESS, 0, defaultpriority);

	if (argc < 2)
	{
		printf("%s\n", zapstream);
		printf("logfile=%s\n", logfile);
		printf("audiochannelspriority=%s\n", audiochannelspriority);
		printf("no arguments\n");
		return EXIT_FAILURE;
	}
	if (!strncmp(argv[1], "-tsfile", 7))
		mode = 3;
	else
	if (!strncmp(argv[1], "-ts", 3))
		mode = 2;
	else
	if (!strncmp(argv[1], "-tcp", 4))
		mode = 4;
	else
		return EXIT_FAILURE;

	buf = (unsigned char *)malloc(bufsize);
	smallbuf = (unsigned char *)malloc(8192);
	pmtbuf = (unsigned char *)malloc(8192);

	if (buf == NULL || smallbuf == NULL || pmtbuf == NULL ) {
		perror("malloc");
		cleanup();
		return EXIT_FAILURE;
	}	
	
	if (mode != 4)
	{
		if (!parseRequest(argv))
		{
			cleanup();
			return EXIT_FAILURE;
		}
	}

	if (mode != 3)
	{
		if ((dvrfd = open(DVRDEV, O_RDONLY)) < 0) {
			cleanup();
			return EXIT_FAILURE;
		}

		int res = 0;
		res = fcntl (dvrfd, F_GETFL, O_NONBLOCK);
		if (res && O_NONBLOCK > 0)
		{
			flog(fdlog, "dvr O_NONBLOCK %d\n", res);
		}
		else
		{
			flog(fdlog, "dvr O_BLOCK %d\n", res);
		}
		
		int retrycounter=5;
		while (!exit_flag && retrycounter > 0)
		{
			flog(fdlog, "connecting to capmt\n");
			fflush(fdlog);
			struct sockaddr_un pmtaddr;
			bzero(&pmtaddr, sizeof(pmtaddr));
			pmtaddr.sun_family = AF_LOCAL;
			strcpy(pmtaddr.sun_path, pmtsocket);
			if (capmtfd >= 0) close(capmtfd);
			if ((capmtfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) exit(1);
			int res = connect(capmtfd, (struct sockaddr *)&pmtaddr, sizeof(pmtaddr));
			if (res >= 0) break;
			flog(fdlog, "error reading capmt from %s: %s\n", pmtsocket, strerror(errno));
			sleep(1);
			retrycounter--;
		}
		if (retrycounter == 0)
		{
			flog(fdlog, "EXIT could not connect to %s\n", pmtsocket);
			cleanup();
			exit(-1);
		}		

		setState(st_waitforcapmt);

	}
	else
	{
		/* open ts file */
		if ((dvrfd = open(tsfile, O_RDONLY)) < 0) {
			cleanup();
			return EXIT_FAILURE;
		}
		setState(st_streaming);
	}

	readptr=buf;
	writeptr=buf;
	endofbuffer=buf+bufsize;

	setSocketOptions(STDOUT_FILENO);
	setInputOptions(dvrfd, 0);		

  		
	flog(fdlog, "enter loop\n");
	fflush(fdlog);
	
	//int state = -1;
	
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;				
	timeoutptr = &timeout; 	

	waitcounter = 0;
	
	while (!exit_flag)
	{
		FD_ZERO(&rset);
		FD_ZERO(&wset);

		if (capmtfd >= 0)
		{
			FD_SET(capmtfd, &rset);
			if (capmtfd > maxfd) maxfd = capmtfd;
		}

		//only read from dvr when we don't wait after a zap
		if (st == st_streaming || st == st_waitforpmt || st == st_startstreaming)
		{
			if ( dvrfd >= 0 && demuxfd_count > 0)
			{
				struct timeval now;
				gettimeofday (&now, NULL);
				
				long usecUntilRead = tvdiff(&nextReadTimestamp, &now)<<10;

				if (logreadspeed > 2) flog(fdlog, "wait usec until read %ld \n", usecUntilRead);
				
				if (usecUntilRead <= 0 || st != st_streaming)
				{
					if (rKBpsPred > 0)
					{
						//we want to read +/- 50KB per read
						usecUntilRead = (50*1000000 / rKBpsPred);
					}
					if (usecUntilRead > 100000)	usecUntilRead = 100000;
					if (usecUntilRead < 0)	usecUntilRead = 0;
					
					FD_SET(dvrfd, &rset);
					if (dvrfd > maxfd) maxfd = dvrfd;

					if (logreadspeed > 1) flog(fdlog, "lastreadTimestamp %ld %ld nextreadTimestamp %ld %ld usecUntilRead = %ld\n", 
						lastreadTimestamp.tv_sec, lastreadTimestamp.tv_usec, nextReadTimestamp.tv_sec, nextReadTimestamp.tv_usec, usecUntilRead);
					nextReadTimestamp = tvadd(&lastreadTimestamp, usecUntilRead>>10);
					if (logreadspeed > 2) flog(fdlog, "nextreadTimestamp %ld %ld\n", nextReadTimestamp.tv_sec, nextReadTimestamp.tv_usec);
				}
				else
				{
					timeout.tv_usec = usecUntilRead;
					tvfix(&timeout);

					timeoutptr = &timeout;
				}

				int i = 0;
				for(; i < demuxfd_count; i++)
				{
					if (demuxfd[i].type == streamtype_pmt)
					{
						FD_SET(demuxfd[i].fd, &rset);
						if (demuxfd[i].fd > maxfd) maxfd = demuxfd[i].fd;
					}
				}
			}
			if (mode==3) //streamfile
			{
				if (logreadspeed == 2) { flog(fdlog, "buffer left %d\n", bufsize - bytesinbuffer(readptr, writeptr)); }
				if (bufsize - bytesinbuffer(readptr, writeptr) > 200000)
				{
					//flog(fdlog, "adding dvrfd %d\n", dvrfd);
					FD_SET(dvrfd, &rset);
					if (dvrfd > maxfd) maxfd = dvrfd;
				}
			}
		}

		int len = bytesinbuffer(readptr, writeptr);
		if (st == st_streaming)
		{
			if (len > 20000)
			{
				FD_SET(STDOUT_FILENO, &wset);
				if ( STDOUT_FILENO > maxfd) maxfd = STDOUT_FILENO;
			}
		}
		else
		{
			flog(fdlog, "nonstream len = %d\n", len);
			if (len > 0)
			{
				FD_SET(STDOUT_FILENO, &wset);
				if ( STDOUT_FILENO > maxfd) maxfd = STDOUT_FILENO;
			}
		}

		fflush(fdlog);

/*========================================== SELECT =============================================================== */
		int retval = select(maxfd + 1, &rset, &wset, NULL, timeoutptr);

		//flog(fdlog, "select = %x %d\n", retval, retval);
		//fflush(fdlog);

		// error
		if (retval < 0)
		{
			flog(fdlog, "select = %x %d\n", retval, retval);
			fflush(fdlog);
			break;
		}
		// timeout => initialize the demux
		if (retval == 0)
		{
			if (logreadspeed > 2) flog(fdlog,"timeout\n");
			timeoutptr = NULL;
			if (st == st_waitforcapmt)
			{
				flog(fdlog, "timeout on waitforcapmt, box probably in standby => exit\n");
				fflush(fdlog);
				exit_flag = 1;
			}
			else
			if (st == st_waitafterzap)
			{
				flog(fdlog, "timeout after zap => init demux\n");
				fflush(fdlog);			
				//read stale data
				int res;
				res = fcntl (dvrfd, F_SETFL, O_NONBLOCK);					
				do
				{
					res = read(dvrfd, buf, IN_SIZE);
				} while (res > 0);				

				readptr = buf;
				writeptr = buf;
				
				patversion++;
				pmtversion++;
				//4 times, so vlc syncs on the data.
				readptr += createPatTs(readptr, patCC++, patversion, program_number, pmtpid);
				readptr += createPatTs(readptr, patCC++, patversion, program_number, pmtpid);
				readptr += createPatTs(readptr, patCC++, patversion, program_number, pmtpid);
				readptr += createPatTs(readptr, patCC++, patversion, program_number, pmtpid);

				setState(st_waitforpmt);
				int i = 0;
				for(; i < demuxfd_count; i++)
				{
					if (demuxfd[i].type == streamtype_pmt)
					{
						OpenPesFilter(&demuxfd[i]);
						StartPesFilter(&demuxfd[i]);
					}
				}
				
				timeout.tv_sec = 0;
				timeout.tv_usec = 500000;
				timeoutptr = &timeout; 
				continue;
			}
			else
			if (st == st_waitforpmt) // timeout on pcr / pmt
			{
				if (waitcounter > 5)
				{
					//no pmt? well ehh, stream anyway
					setState(st_startstreaming);
					waitcounter = 0;
				}
				else
				{
					timeout.tv_sec = 0;
					timeout.tv_usec = 500000;
					timeoutptr = &timeout; 					
					waitcounter++;		
				}
			}
			continue;
		}
		if (st == st_waitforpmt && waitcounter > 4)
		{
			flog(fdlog, "state > 4, insert generated pmt\n");
			waitcounter = 0;
			setState(st_startstreaming); //fall through
		}

		if (capmtfd >= 0 && FD_ISSET(capmtfd, &rset))
		{
			//flog(fdlog, "cam\n");
			setpriority( PRIO_PROCESS, 0, defaultpriority);
			processCAPMT();
		}
		
		int i = 0;
		if (st == st_waitforpmt || st == st_streaming)
		{
			for(; i < demuxfd_count; i++)
			{
				if (demuxfd[i].fd >= 0 && FD_ISSET(demuxfd[i].fd, &rset))
				{
					//flog(fdlog, "pes\n");
					if (demuxfd[i].type == streamtype_pmt)
					{
						//process pmt
						int r = read(demuxfd[i].fd, smallbuf, 8192);
						if (r > 0)				
						{
							int programnumber_pmt = 0;
							
							programnumber_pmt  = (smallbuf[3]&0xff)<<8;
							programnumber_pmt |= (smallbuf[4]&0x00ff);

							int current = (smallbuf[5]&0x0001);

							// only send the pmt of the correct program
							if (program_number == programnumber_pmt && current == 1) 
							{
								//hack, setting global var pcrpid
								pcrpid  = 0;
								pcrpid  = (smallbuf[8]&0x1f)<<8; // pcrpid
								pcrpid |= (smallbuf[9]&0x00ff);  // pcrpid
								
								pmtbuflen = r;
								memcpy(pmtbuf, smallbuf, pmtbuflen);
	
								//copy directly to main buffer if the stream has to start.
								if (st == st_waitforpmt)
								{
									int s = createPmtTsFromPmtPes(readptr, &demuxfd[i], pmtbuf, pmtbuflen);
									readptr+=s;
									setState(st_startstreaming);
								}
							}
						}
					}
				}
			}
		}
		
		if (st == st_startstreaming)
		{
			readptr += createPatTs(readptr, patCC++, patversion, program_number, pmtpid);
			flog(fdlog, "extra pat\n");

			int i = 0;
			for(;i < demuxfd_count; i++)
			{
				if (pcrpid == demuxfd[i].pid)
				{
					pcrpid = 0; //don't  duplicate this pid (if it's the same as the vpid)
				}
			}
			if (pcrpid != 0)
			{
				InitDemuxEntry(&demuxfd[demuxfd_count++], pcrpid, streamtype_pcr, 0, 0, NULL);
			}
			
			i = 0;
			for(;i < demuxfd_count; i++)
			{
				if (demuxfd[i].type != streamtype_pmt)
				{
					OpenPesFilter(&demuxfd[i]);
					StartPesFilter(&demuxfd[i]);
				}
			}
			
			setpriority( PRIO_PROCESS, 0, streamingpriority);
			timeoutptr = NULL;
			setState(st_streaming);
		}		

		if (FD_ISSET(STDOUT_FILENO, &wset))
		{
			//flog(fdlog, "out\n");
			processOut();
		}

		if (dvrfd >= 0 && FD_ISSET(dvrfd, &rset))
		{
			//flog(fdlog, "dvr\n");
			processDVR();
		}
	}
	
	flog(fdlog, "EXIT\n");
	fflush(fdlog);		

	cleanup();
	return EXIT_SUCCESS;
}
