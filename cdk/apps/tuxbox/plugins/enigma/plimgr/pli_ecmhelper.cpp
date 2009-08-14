/***************************************************************************
                          ecmhelper.cpp  -  send caid info form ecm.info to enigma
                                            using the Pli enigma cahandler interface
                             -------------------
    begin                : Feb 2006
    copyright            : (C) 2005 by pieterg, mirakels
    email                : 
 ***************************************************************************/
#include <iostream>
using namespace ::std;

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "wrappers.h"


const char* ECMINFO = "/tmp/ecm.info";
const char* ECMINFOSH = "/var/etc/plimgr/scripts/ecminfo";
const char* PMT_SOCKET = "/tmp/.listen.camd.socket";

void daemon_init()
{
	int i;
	pid_t pid;
	
	if ((pid = fork()) != 0)
		exit(0);
	
	setsid(); /* become session leader */
	
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	
	if ((pid = fork()) != 0)
		exit(0);
	
	for (i = 0; i < 10; i++)
		close(i);
	
	chdir("/");

	freopen("/dev/null", "a", stdin);
	freopen("/dev/null", "a", stdout);
	freopen("/dev/null", "a", stderr);
}

int ioPipe[2] = {-1};

void sigio_handler(int)
{
	if (ioPipe[1] >= 0) ::Write(ioPipe[1], "\x01", 1);
	signal(SIGIO, sigio_handler);
}

int main(int argc, char *argv[])
{
	int capmtfd = -1;
	struct stat fst;
	time_t last_st = 0;
	int daemon = 1;
	int i;
	FILE * fp;
	char tlvbuf[16];
	char * progname;
	struct timeval to;

	progname = argv[0];
	if (strchr(progname, '/'))
	{
		progname = strrchr(progname, '/');
		progname++;
	}
	if (strlen(progname) != 13)
	{
		exit(1);
	}
	if (
		progname[6] + 0x1b != 0x88  || // 6d m
		progname[0] - 0x4d != 0x23  || // 70 p
		progname[5] - 0x3d != 0x26  || // 63 c
		progname[1] - 0x47 != 0x25  || // 6c l
		progname[3] + 0x87 != 0xe6  || // 5f _
		progname[4] - 0x51 != 0x14  || // 65 e
		progname[10] - 0x39 != 0x37  || // 70 p
		progname[8] + 0x8f != 0xf4  || // 65 e
		progname[12] - 0x51 != 0x21  || // 72 r
		progname[2] - 0x09 != 0x60  || // 69 i
		progname[7] - 0x53 != 0x15  || // 68 h
		progname[9] - 0x59 != 0x13  || // 6c l
		progname[11] - 0x50 != 0x15  || // 65 e
		progname[13] != 0 )
	{
		exit(2);
	}

	for (i = 1; i < argc; i++)
	{
		if (strstr(argv[i], "-d"))
			daemon = 0;
	}

	if (access(ECMINFOSH, X_OK) == -1)
	{
		printf("%s: %s not available/executable\n", progname, ECMINFOSH);
		exit(1);
	}
	
	if (daemon)
		daemon_init();

	pipe(ioPipe);
	signal(SIGIO, sigio_handler);
	DIR *tmpdir = opendir("/tmp");
	if (tmpdir) fcntl(dirfd(tmpdir), F_NOTIFY , DN_CREATE | DN_MODIFY | DN_DELETE | DN_MULTISHOT);

	/* initial trigger */
	if (ioPipe[1] >= 0) ::Write(ioPipe[1], "\x01", 1);

	while (1)
	{
		if (capmtfd >= 0)
		{
			close(capmtfd);
			capmtfd = -1;
		}

		while (1)
		{
			struct sockaddr_un pmtaddr;
			bzero(&pmtaddr, sizeof(pmtaddr));
			pmtaddr.sun_family = AF_LOCAL;
			strcpy(pmtaddr.sun_path, PMT_SOCKET);
			if (capmtfd >= 0)
				close(capmtfd);
			if ((capmtfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
				exit(1);
			if (!daemon)
				printf("connect\n");
			if (Connect(capmtfd, (struct sockaddr *)&pmtaddr, sizeof(pmtaddr)) >= 0)
				break;
			sleep(1);
		}

		int32_t oldcaid = -1;

		while (1)
		{
			fd_set rset;
			int maxfd = -1;

			FD_ZERO(&rset);
			if (capmtfd >= 0)
			{
				FD_SET(capmtfd, &rset);
				if (capmtfd > maxfd) maxfd = capmtfd;
			}
			if (ioPipe[0] >= 0)
			{
				FD_SET(ioPipe[0], &rset);
				if (ioPipe[0] > maxfd) maxfd = ioPipe[0];
			}
			if (Select(maxfd + 1, &rset, NULL, NULL, NULL) <= 0)
				break;
			if (capmtfd >= 0 && FD_ISSET(capmtfd, &rset))
			{
				int length = 0;
				int lengthdatasize = 0;
				int readcount;
				unsigned char buffer[1024];
				readcount = 0;
				if (Read(capmtfd, &buffer[readcount], 4) <= 0)
					break;
				readcount += 4;
				if (buffer[3] & 0x80)
				{
					/* multibyte length field */
					int i;
					lengthdatasize = buffer[3] & 0x7f;
					if (Read(capmtfd, &buffer[readcount], lengthdatasize) <= 0)
						break;
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
				if (Read(capmtfd, &buffer[readcount], length) <= 0)
					break;
			}
			if (ioPipe[0] >= 0 && FD_ISSET(ioPipe[0], &rset))
			{
				unsigned char byte;
				if (::Read(ioPipe[0], &byte, 1) < 1) break;
				char caidbuff[512] = {0};
				char infobuff[512] = {0};
				char fullinfobuff[512] = {0};
				char ecmtimebuff[512] = {0};
				char nodebuff[512] = {0};
				int32_t caid = 0;
				double ecmtime = 0.0;
				bool change = false;
				if (stat(ECMINFO, &fst) == 0)
				{
					if (fst.st_mtime != last_st)
					{
						change = true;
						last_st = fst.st_mtime;
					}
				}
				else
				{
					if (last_st)
					{
						change = true;
						last_st = 0;
					}
				}
				if (change)
				{
					if (!daemon)
						printf("%s: ecminfo changed. try: %d size=%ld modtime=%s", progname, i + 1, fst.st_size, ctime(&fst.st_mtime));
					fp = popen(ECMINFOSH, "r");
					if (fp)
					{
						fgets(caidbuff, sizeof(caidbuff), fp);
						fgets(infobuff, sizeof(infobuff), fp);
						fgets(fullinfobuff, sizeof(fullinfobuff), fp);
						fgets(ecmtimebuff, sizeof(ecmtimebuff), fp);
						fgets(nodebuff, sizeof(nodebuff), fp);
						pclose(fp);

						sscanf(caidbuff, "%x", &caid);
						sscanf(caidbuff, "%lf", &ecmtime);
						ecmtime *= 1000;
					}
					tlvbuf[0]=0x9F;
					tlvbuf[1]=0x70;
					tlvbuf[2]=0x20; /* client info */
					tlvbuf[3]=strlen(infobuff)+1;
					memcpy(tlvbuf+4, infobuff, strlen(infobuff)+1);
					if (::Write(capmtfd, tlvbuf, 5+strlen(infobuff)) < 0)
					{
						if (!daemon)
							printf("%s: write to enigma failed %s\n", progname, strerror(errno));
					}
					if (caid != oldcaid)
					{
						oldcaid = caid;
						if (!daemon)
							printf("%s: sending caid\n>%x<\nto enigma\n", progname, caid);
						caid = htonl(caid);
						tlvbuf[0] = 0x9F;
						tlvbuf[1] = 0x70;
						tlvbuf[2] = 0x21; /* used caid */
						tlvbuf[3] = sizeof(caid);
						memcpy(tlvbuf+4, (char*)&caid, sizeof(caid));
						if (::Write(capmtfd, tlvbuf, 4+sizeof(caid)) < 0)
						{
							if (!daemon)
								printf("%s: write to enigma failed %s\n", progname, strerror(errno));
						}
					}
					tlvbuf[0]=0x9F;
					tlvbuf[1]=0x70;
					tlvbuf[2]=0x22; /* verbose info */
					tlvbuf[3]=strlen(fullinfobuff)+1;
					memcpy(tlvbuf+4, fullinfobuff, strlen(fullinfobuff)+1);
					if (::Write(capmtfd, tlvbuf, 5+strlen(fullinfobuff)) < 0)
					{
						if (!daemon)
							printf("%s: write to enigma failed %s\n", progname, strerror(errno));
					}

					int32_t time = htonl((int)ecmtime);
					tlvbuf[0] = 0x9F;
					tlvbuf[1] = 0x70;
					tlvbuf[2] = 0x23; /* decode time (ms) */
					tlvbuf[3] = sizeof(time);
					memcpy(tlvbuf+4, (char*)&time, sizeof(time));
					if (::Write(capmtfd, tlvbuf, 4+sizeof(time)) < 0)
					{
						if (!daemon)
							printf("%s: write to enigma failed %s\n", progname, strerror(errno));
					}

					tlvbuf[0] = 0x9F;
					tlvbuf[1] = 0x70;
					tlvbuf[2] = 0x24; /* nodeid */
					tlvbuf[3] = strlen(nodebuff)+1;
					memcpy(tlvbuf+4, nodebuff, strlen(nodebuff)+1);
					if (::Write(capmtfd, tlvbuf, 4+strlen(nodebuff)+1) < 0)
					{
						if (!daemon)
							printf("%s: write to enigma failed %s\n", progname, strerror(errno));
					}
				}
			}
		}
	}
}
