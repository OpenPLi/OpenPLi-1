/*
 * $Id: streamvideo.c,v 1.8 2002/02/23 18:34:53 Toerli Exp $
 * 
 * TCP Video/Audio - PES Streamer
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
 * $Log: streamvideo.c,v $
 * Revision 1.8  2002/02/23 18:34:53  Toerli
 * vi
 *
 * Revision 1.7  2002/02/10 20:12:23  Toerli
 * first working version
 *
 * Revision 1.6  2002/02/09 01:46:09  obi
 * header redesign ;)
 *
 * Revision 1.5  2002/02/06 21:39:12  Toerli
 * fixes
 * 
 * Revision 1.4  2002/02/06 21:22:24  Toerli
 * *** empty log message ***
 * 
 * Revision 1.3  2002/02/06 21:15:35  Toerli
 * updates..
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <netdb.h>


#define DEST_PORT_AUDIO 40001
#define DEST_PORT_VIDEO 40000

#define BLOCKSIZE_AUDIO 1000
#define BLOCKSIZE_VIDEO 1400

int main(int argc, char *argv[])
{

  char *aname=NULL, *vname=NULL, *host=NULL;  //Also das mit NULL is irgendwie kacke aber na ja.... 
  int audiofd, videofd;
  fd_set writefds;
  int sockfda,sockfdv;
  struct sockaddr_in dest_addra, dest_addrv;
  struct hostent *he;
  unsigned char audiobuf[BLOCKSIZE_AUDIO], videobuf[BLOCKSIZE_VIDEO];
  int rbaudio;  //  Read Bytes Audio
  int rbvideo;  //         and Video
  int bytes_senta;
  int bytes_sentv;
  int i;


/* ------------------ cmdline ---------------------- */

 if (argc==1) { printf("streamvideo\n\n");
                printf("valid options:\n");
                printf("-a <filename>  audio-pes\n");
                printf("-v <filename>  video-pes\n");
                printf("-h <host>      ip/hostname of the box\n");
                return 0;
              }
  i = 0;
  while (++i < argc) {
  if      (!strcmp (argv[i],"-a")) aname = argv[++i];
  else if (!strcmp (argv[i],"-v")) vname = argv[++i];
  else if (!strcmp (argv[i],"-h")) host  = argv[++i];
                        }
/* --------------------------------------------------- */

if (host != NULL ) { if ((he=gethostbyname(host)) == NULL) {  // get the host info
            perror("gethostbyname");
            exit(1); } }
       else { printf("specify hostname\n"); return 0; }

if (aname != NULL)  audiofd = open(aname, O_RDONLY, S_IRWXU);
if (vname != NULL)  videofd = open(vname, O_RDONLY, S_IRWXU);
if (audiofd == -1) { printf("could not open audiopes:  %s \n", aname); return 0 ; }
if (videofd == -1) { printf("could not open videopes:  %s \n", vname); return 0 ; }

if (aname != NULL) {
        sockfda = socket(AF_INET, SOCK_STREAM, 0);
        dest_addra.sin_family = AF_INET;
        dest_addra.sin_port = htons(DEST_PORT_AUDIO);
        dest_addra.sin_addr = *((struct in_addr *)he->h_addr);
        memset(&(dest_addra.sin_zero), '\0', 8);
        fcntl(sockfda, F_SETFL, O_NONBLOCK );
        connect(sockfda, (struct sockaddr *)&dest_addra, sizeof(struct sockaddr));
		}

if (vname != NULL) {
        sockfdv = socket(AF_INET, SOCK_STREAM, 0);
        dest_addrv.sin_family = AF_INET;
        dest_addrv.sin_port = htons(DEST_PORT_VIDEO);
        dest_addrv.sin_addr = *((struct in_addr *)he->h_addr);
        memset(&(dest_addrv.sin_zero), '\0', 8);
        fcntl(sockfdv, F_SETFL, O_NONBLOCK );
        connect(sockfdv, (struct sockaddr *)&dest_addrv, sizeof(struct sockaddr));
   		}

//Create FD_SET's for select
FD_ZERO(&writefds);
if (aname != NULL) FD_SET(sockfda,&writefds);
if (vname != NULL) FD_SET(sockfdv,&writefds);

// 500ms muss der Audio vor Video kommen
if (aname != NULL) {
rbaudio=read(audiofd, audiobuf, BLOCKSIZE_AUDIO);
bytes_senta = send(sockfda, audiobuf, rbaudio, 0);
usleep(500000); }


 while(1) {
   if (select(7, 0, &writefds, 0 ,NULL)>0)
     {
       if  ( (vname!=NULL) && (FD_ISSET(sockfdv, &writefds)) ) {
         rbvideo=read(videofd, videobuf, BLOCKSIZE_VIDEO);
         if(!(bytes_sentv = send(sockfdv, videobuf, rbvideo, 0))) {
           printf("Video: EOF reached\n");
           break;
         }
       }
       if ( (aname!=NULL) && (FD_ISSET(sockfda, &writefds)) ) {
         rbaudio=read(audiofd, audiobuf, BLOCKSIZE_AUDIO);
         if(!(bytes_senta = send(sockfda, audiobuf, rbaudio, 0))) {
          printf("Audio: EOF reached\n");
          break;
         }
       }

     }
if (aname != NULL)  FD_SET(sockfda,&writefds);
if (vname != NULL)  FD_SET(sockfdv,&writefds);
 }
if (vname != NULL) close(videofd);
if (aname != NULL) close(audiofd);
if (vname != NULL) close(sockfdv);
if (aname != NULL) close(sockfda);
return 0;
}

