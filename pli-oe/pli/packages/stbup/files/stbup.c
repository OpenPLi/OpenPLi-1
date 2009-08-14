/******************************************************************************
 *                     <<< Embedded-Uptime-Project Daemon >>>                 *
 *            (c) @mechatron Jean Willrich 2007 (mechatron@gmx.net)           *
 ******************************************************************************/

#include <getopt.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdint.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <linux/route.h>

#define DEFAULTSERVER	"http://www.embedded-uptime-project.com"
#define DEFAULTCONFIG	"/etc/stbup.conf"
#define PROG_VERSION	"v0.1.7"

struct para
{
	char user[30];
	char pass[30];
	char server[100];
	char description[30];
	char mac_addr[20];
	char key[9];
	int interval;
	int group;
	char pidfile[100];
} m_para;

int debug=1;

int http_port;
int http_proxy_port;
char *http_server;
char *http_proxy_server;

static unsigned long int crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

void myDebug(const char* str, ...)
{
	if(debug)
	{
		char buf[1024];
		va_list ap;
		va_start(ap, str);
		vsnprintf(buf, 1024, str, ap);
		va_end(ap);

		printf("%s\n",buf);
		if(debug>1)
			syslog(LOG_USER | LOG_DEBUG, buf);
	}
}

static char *base64enc(unsigned char *p, char *buf, int len)
{

	char al[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char *s = buf;

	while(*p)
	{
		if (s >= buf+len-4)
			myDebug("[stbup] ERROR buffer overflow");
		*(s++) = al[(*p >> 2) & 0x3F];
		*(s++) = al[((*p << 4) & 0x30) | ((*(p+1) >> 4) & 0x0F)];
		*s = *(s+1) = '=';
		*(s+2) = 0;
		if (! *(++p)) break;
		*(s++) = al[((*p << 2) & 0x3C) | ((*(p+1) >> 6) & 0x03)];
		if (! *(++p)) break;
		*(s++) = al[*(p++) & 0x3F];
	}

	return buf;
}

int is_running()
{
	struct dirent *entry;
	int val=0;
	char *name;
	char buf[1024], cmdline[40];
	FILE *fp;
	DIR *dir;

	dir= opendir("/proc");
        if(dir)
        {
        	while(entry = readdir(dir))
        	{
			name = entry->d_name;

			if (!(*name >= '0' && *name <= '9'))
				continue;
			
			sprintf(cmdline, "/proc/%s/cmdline", name);
			if ((fp = fopen(cmdline, "r")) == NULL)
				continue;
			if ((fread(buf, 1, sizeof(buf) - 1, fp)) > 0)
				if (strstr(buf, "stbup") != 0)
					val +=1;
			fclose(fp);
		}
		closedir(dir);
	}
	return val;
}

void getCRC(char *user, char *pass, long uptime)
{
	int x;
	unsigned int crc32 = 0xFFFFFFFF;
	char str[256];
	snprintf(str,255,"%s%s%ld",user, pass, uptime);
	
	for (x=0; x<strlen(str); x++)
		crc32 = crc_32_tab[((crc32) ^ (str[x])) & 0xFF] ^ ((crc32) >> 8);

	sprintf(m_para.key,"%08x",-crc32-1);
	for(x=0; x<strlen(m_para.key);x++)
		m_para.key[x] = toupper(m_para.key[x]);
}

int getMac()
{
	struct ifconf ifc;
	struct ifreq *req;
	int x;
	uint8_t arp[6];
	int found=0;
	
	int fd=socket(AF_INET,SOCK_DGRAM,0);
	if(fd)
	{
		ifc.ifc_len = sizeof(struct ifreq) * 30;//warum auch immer
		ifc.ifc_buf = malloc(ifc.ifc_len);

		if(ioctl(fd, SIOCGIFCONF, &ifc) == 0)
		{
			req = ifc.ifc_req;
			for (x=0; x<ifc.ifc_len; x+=sizeof(struct ifreq))
			{
				if(ioctl(fd,SIOCGIFHWADDR,req) == 0 )
				{
					memcpy(arp, req->ifr_hwaddr.sa_data, 6);
					sprintf(m_para.mac_addr,"%02x%02x%02x%02x%02x%02x",arp[0],arp[1],arp[2],arp[3],arp[4],arp[5]);
					if(strstr(m_para.mac_addr, "000000000000") == 0)
					{
						found=1;
						break;
					}
				}
				else	perror("SIOCGIFHWADDR");
				req++;
			}
			
			if(!found)
			{
				memset(req,0,sizeof(req));
				strcpy(req->ifr_name, "eth0");
				if( ioctl(fd,SIOCGIFHWADDR,req) == 0 )
				{
					found=1;
					memcpy(arp, req->ifr_hwaddr.sa_data, 6);
					sprintf(m_para.mac_addr,"%02x%02x%02x%02x%02x%02x",arp[0],arp[1],arp[2],arp[3],arp[4],arp[5]);
				}
				else	perror("SIOCGIFHWADDR");
			}
		}
		else	perror("SIOCGIFCONF");
		
		if(ifc.ifc_buf)
			free(ifc.ifc_buf);
		close(fd);
	}
	else	perror("socket");
		
	if(found)
	{
		for(x=0; x<strlen(m_para.mac_addr);x++)
			m_para.mac_addr[x] = toupper(m_para.mac_addr[x]);
		myDebug("[stbup] MAC=%s by Interface=%s", m_para.mac_addr, req->ifr_name);
	}
	return found;
}

int http_parse_url(char *url)
{
	char *pc,c;
	char new_url[256];

	http_port=80;
	if(http_server)
	{
		free(http_server);
		http_server=NULL;
	}

	if (strncasecmp("http://",url,7))
	{
		myDebug("[stbup] ERROR invalid url (must start with 'http://')");
		return 0;
	}
	url+=7;
	int x=0;
	for (pc=url,c=*pc; (c && c!=':' && c!='/');x++)
	{
		if(*pc == '/' || *pc == ':'){}
		else
		{
			if(x<sizeof(new_url))
				new_url[x] = *pc;
		}
		c=*pc++;
	}

	if (*pc)
		pc-=0;

	if (c==':')
	{
		if (sscanf(pc,"%d",&http_port)!=1)
		{
			myDebug("[stbup] ERROR invalid port in url");
			return 0;
		}
	}

	http_server = strdup(new_url);
	myDebug("[stbup] Host='%s', Port=%d", http_server, http_port);
	return 1;
}

int send_to_server(char *path, char *buffer)
{
	int s;
	struct hostent *hp;
	struct sockaddr_in server;
	char header[1024];
	char recv_buf[1024];
	char mess[512];
	int ret;
	int proxy=(http_proxy_server!=NULL && http_proxy_port!=0);
	int port = proxy ? http_proxy_port : http_port ;
	
	if (hp = gethostbyname( proxy ? http_proxy_server : ( http_server ? http_server : "adonis" )))
	{
		memset((char *) &server,0, sizeof(server));
		memmove((char *) &server.sin_addr, hp->h_addr, hp->h_length);
		server.sin_family = hp->h_addrtype;
		server.sin_port = (unsigned short) htons( port );
	}
	else
	{
		myDebug("[stbup] ERROR Could not resolve hostname");
		return 0;
	}
		
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		myDebug("[stbup] ERROR Could not create socket");
		return 0;
	}

	if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		myDebug("[stbup] ERROR Could not connect to server");
		ret=0;
	}
	else
	{
		if (proxy)
			sprintf(header,"GET http://%.256s:%d/%s%.512s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n",  http_server, http_port, path, buffer, http_server, PROG_VERSION);
		else
			sprintf(header,"GET /%s%.512s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n", path, buffer, http_server, PROG_VERSION);

		if(send(s, header, strlen(header), 0) == -1)
			myDebug("[stbup] ERROR Could not send stats to server");
		if((ret = recv(s, recv_buf, sizeof(recv_buf), 0)) == -1)
			myDebug("[stbup] ERROR Could not receive data from server");
		
		if(ret>0)
		{
			int len;
			sscanf(recv_buf,"HTTP/1.%*d %03d",&ret);
			char *rec=strstr(recv_buf, "\r\n\r\n")+4;
			if(strlen(rec)<512)
			{
				sscanf(rec,"%x\r\n%[^\r]s",&len,mess);
				//printf("len=%d mess=%s\n",len,mess);
				if(len>0)
					myDebug("%s",mess);
			}
			else
				myDebug("[stbup] ERROR receive data from server to long");
		}
	}
	close(s);
	return ret;
}

int writePidFile(char *pidfile)
{
	FILE *pf = fopen(pidfile, "w");
	if (NULL == pf)
		return 0;
	else
	{
		fprintf(pf, "%d\n", (int)getpid());
		fclose(pf);
		return 1;
	}
}
int readConfFile(char *name)
{
	char line_buf[500]; *line_buf=0;
	char *key, *value;
	char tokens[] = "=\n\r";
	FILE *fp;

	if((fp = fopen(name, "r")) == NULL)
	{
		myDebug("[stbup] ERROR open File=%s",name);
		return 0;
	}
	
	while(fgets(line_buf, sizeof(line_buf), fp) != NULL)
	{
		if((key = strtok(line_buf, tokens)) != NULL && (value = strtok(NULL, tokens)) != NULL)
		{
			if(strstr(key, "STBUP_ON") != NULL)
			{
				if(value[0]=='0')
				{
					myDebug("[stbup] stopped from %s",name);
					return 0;
				}
			}
			else if(strstr(key, "GROUPID") != NULL)
			{
				m_para.group = atoi(value);
			}
			else if(strstr(key, "INTERVAL") != NULL)
			{
				m_para.interval = atoi(value) * 60;
			}
			else if(strstr(key, "DEBUGMODE") != NULL)
			{
				debug = atoi(value);
			}
			else if(strstr(key, "USERNAME") != NULL)
			{
				strncpy(m_para.user, value, sizeof(m_para.user)-1);
			}
			else if(strstr(key, "PASSNAME") != NULL)
			{
				strncpy(m_para.pass, value, sizeof(m_para.pass)-1);
			}
			else if(strstr(key, "SERVERNAME") != NULL)
			{
				strncpy(m_para.server, value, sizeof(m_para.server)-1);
			}
			else if(strstr(key, "PIDFILE") != NULL)
			{
				strncpy(m_para.pidfile, value, sizeof(m_para.pidfile)-1);
			}
			else if(strstr(key, "DESCRIPTION") != NULL)
			{
				strncpy(m_para.description, value, sizeof(m_para.description)-1);
				int x;
				for(x=0; x<strlen(m_para.description)-1;x++)
					if(m_para.description[x]>'z')
						m_para.description[x] = '_';
			}
		}
	}

	fclose(fp);
	return 1;
}

void print_help()
{
	printf("\nUsage: stbup [OPTION]...\n");
	printf("\t -u (username)\n");
	printf("\t -p (password)\n");
	printf("\t -g (groupID)\n");
	printf("\t -s (uptime server, default=%s)\n",DEFAULTSERVER);
	printf("\t -i (interval in min, default=%d)\n",m_para.interval/60);
	printf("\t -d (debuglevel 0=none, 1=console, 2=console+syslog, default=1)\n");
	printf("\t -C (configuration file, default=%s)\n",DEFAULTCONFIG);
	printf("\t -D (description, default=none)\n");
	printf("\t -P (write pid to file, default=off)\n\n");
}

int main(int argc, char **argv)
{
	myDebug("[stbup] %s started...",PROG_VERSION);

	if(is_running()>1)
	{
		myDebug("[stbup] only one instance");
		return -1;
	}
	
	struct sysinfo info;
	struct utsname uinfo;
	
	memset(&m_para, 0, sizeof(struct para));
	m_para.interval=10*60;
	strncpy(m_para.server, DEFAULTSERVER, sizeof(m_para.server)-1);
	strncpy(m_para.description, "none", sizeof(m_para.description)-1);
	
	char encodedstring[512];
	char command[256];
	char ConfFile[256]; *ConfFile=0;
	char *proxy;
	
	unsigned char c;
	int count=0;
	int ret;
	
	if(argc==1)
		strncpy(ConfFile, DEFAULTCONFIG, sizeof(ConfFile)-1);
	
	while (1)
	{
		c = getopt(argc, argv, "i:u:p:g:s:d:D:C:n:P:h");
		if ( c == 255 )	 break;
		
		switch( c )
		{
			case 'u':
				strncpy(m_para.user, optarg, sizeof(m_para.user)-1);
				break;
			case 'p':
				strncpy(m_para.pass, optarg, sizeof(m_para.pass)-1);
				break;
			case 'g':
				m_para.group = atoi(optarg);
				break;
			case 's':
				strncpy(m_para.server, optarg, sizeof(m_para.server)-1);
				break;
			case 'P':
				strncpy(m_para.pidfile, optarg, sizeof(m_para.pidfile)-1);
				break;
			case 'd':
				debug = atoi(optarg);
				break;
			case 'i':
				m_para.interval = atoi(optarg) * 60;
				break;
			case 'D':
				strncpy(m_para.description, optarg, sizeof(m_para.description)-1);
				int x;
				for(x=0; x<strlen(m_para.description)-1;x++)
					if(m_para.description[x]>'z')
						m_para.description[x] = '_';
				break;
			case 'C':
				strncpy(ConfFile, optarg, sizeof(ConfFile)-1);
				break;
			case 'h':
				print_help();
				return 0;
				break;
			default:
				break;
		}
		
	}
	
	if(!getMac())
	{
		myDebug("[stbup] ERROR MAC not found");
		return 0;
	}

	if(*ConfFile!=0)
	{
		if(! readConfFile(ConfFile))
			return 0;
	}
	if(*m_para.pidfile != 0)
	{
		if(! writePidFile(m_para.pidfile))
                	myDebug("[stbup] ERROR Could not write to Pidfile");
	}

	if(m_para.interval==0 || *m_para.user==0 || *m_para.pass==0 || m_para.group==0 || *m_para.server==0)
	{
		if(m_para.interval==0)
			myDebug("[stbup] ERROR Interval not found");
		if(*m_para.user==0)
			myDebug("[stbup] ERROR Username not found");
		if(*m_para.pass==0)
			myDebug("[stbup] ERROR Password not found");
		if(m_para.group==0)
			myDebug("[stbup] ERROR GroupID not found");
		if(*m_para.server==0)
			myDebug("[stbup] ERROR Uptime Server not found");

		print_help();
		return -1;
	}
	
	if(m_para.interval < 600)
		m_para.interval=600;

	myDebug("[stbup] Username='%s'", m_para.user);
	//myDebug("[stbup] Password='%s'", m_para.pass);
	myDebug("[stbup] GroupID=%d", m_para.group);
	myDebug("[stbup] Uptime Server='%s'", m_para.server);
	myDebug("[stbup] Interval=%dsec", m_para.interval);
	if(*m_para.pidfile != 0)
		myDebug("[stbup] Pidfile='%s'", m_para.pidfile);
	if(*m_para.description != 0)
		myDebug("[stbup] Description='%s'", m_para.description);

	uname(&uinfo);

	
	if(proxy=getenv("http_proxy"))
	{
		ret=http_parse_url(proxy);
		if (ret<0)	return ret;
		http_proxy_server=http_server;
		http_server=NULL;
		http_proxy_port=http_port;
		myDebug("[stbup] Proxy=%s", http_proxy_server);
	}
	ret=http_parse_url(m_para.server);
	
	while(1)
	{
		if(count <= 0)
		{
			count = m_para.interval;
			sysinfo(&info);
			
			getCRC(m_para.user, m_para.pass, info.uptime);
			
			memset(command, 0, sizeof(command));
			sprintf(command,"mac=%s&user=%s&pass=%s&group=%d&uptime=%ld&sys=%s-%s&des=%s&key=%s", m_para.mac_addr, m_para.user, m_para.pass, m_para.group, info.uptime, uinfo.sysname, uinfo.release, m_para.description, m_para.key);

			base64enc(command, encodedstring, sizeof(encodedstring));
			
			ret = send_to_server("data/stbup_in.pl?", encodedstring);
			if (ret==200 || ret==300 || ret==301 || ret==302 || ret==303)
				myDebug("[stbup] next Update in %d min", count/60);
			else
				myDebug("[stbup] Transfer not successful code=%d",ret);
		}
		count -= 1;
		sleep(1);
	}

	return 0;
}
