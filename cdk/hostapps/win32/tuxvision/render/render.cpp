//
//  DBOXII Render Filter
//  
//  Rev.0.0 Bernd Scharping 
//  bernd@transputer.escape.de
//
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <winsock2.h>
#include <streams.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <math.h>
#include <process.h>
#include <time.h>

#include "ccircularbuffer.h"
#include "interface.h"
#include "render.h"

#include "debug.h"

BYTE gSilentMPAFrame[768 + 54]=
{
0x00, 0x00, 0x01, 0xBA, 0x44, 0x00, 0x04, 0x00, 0xA4, 0x01, 0x00, 0x0A, 0x67, 0xF8, 0x00, 0x00, 
0x01, 0xBB, 0x00, 0x0C, 0x80, 0x05, 0x33, 0x06, 0xE0, 0xFF, 0xC0, 0xC0, 0x80, 0xE0, 0xE0, 0x00, 
0x00, 0x00, 0x01, 0xC0, 0x03, 0x10, 0x81, 0x81, 0x0D, 0x21, 0x00, 0x01, 0x21, 0xC1, 0x1E, 0x40, 
0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  //0xFF, 0xFD, 0xC4, 0x00, 0x99, 0x66, 0x44, 0x55, 0x44, 0x33, 

0xFF, 0xFD, 0xC4, 0x00, 0x99, 0x66, 0x44, 0x55, 0x44, 0x33, 0x33, 0x33, 0x33, 0x22, 0x22, 0x49, 
0x24, 0x92, 0x49, 0x12, 0x49, 0x24, 0x92, 0x49, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAF, 0xBE, 0xFB, 0xEF, 0xBE, 0xFB, 0xEF, 0xBE, 0xFB, 
0xEF, 0xBE, 0xFB, 0xEF, 0xBE, 0xFB, 0xEF, 0xBE, 0xFB, 0xEF, 0xBE, 0xFB, 0xEF, 0xBE, 0xFB, 0xEF, 
0xBE, 0xFB, 0xEF, 0xBE, 0xFB, 0xEF, 0xBE, 0xFB, 0xEF, 0xBE, 0xFB, 0xEF, 0xBE, 0xFB, 0xEF, 0xBE, 
0x7F, 0xDF, 0xF7, 0xFD, 0xFF, 0x7F, 0xDF, 0xF7, 0xEF, 0xDF, 0xBF, 0x7E, 0xFD, 0xEF, 0x7B, 0xDE, 
0xF7, 0x77, 0x77, 0x75, 0xB1, 0x6C, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x7C, 
0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x35, 0xAD, 0x6B, 0x5A, 
0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5F, 0xF7, 0xFD, 0xFF, 0x7F, 0xDF, 0xF7, 
0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7B, 0xDE, 0xF7, 0xBD, 0xDD, 0xDD, 0xDD, 0x6C, 0x5B, 0x1B, 
0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDF, 0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 
0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 
0x6B, 0x5A, 0xD7, 0xFD, 0xFF, 0x7F, 0xDF, 0xF7, 0xFD, 0xFF, 0x7E, 0xFD, 0xFB, 0xF7, 0xEF, 0xDE, 
0xF7, 0xBD, 0xEF, 0x77, 0x77, 0x77, 0x5B, 0x16, 0xC6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 
0x6D, 0xB7, 0xCF, 0x9F, 0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7C, 0xF9, 0xF3, 0x5A, 
0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xFF, 0x7F, 0xDF, 0xF7, 
0xFD, 0xFF, 0x7F, 0xDF, 0xBF, 0x7E, 0xFD, 0xFB, 0xF7, 0xBD, 0xEF, 0x7B, 0xDD, 0xDD, 0xDD, 0xD6, 
0xC5, 0xB1, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 
0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7C, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 
0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x7F, 0xDF, 0xF7, 0xFD, 0xFF, 0x7F, 0xDF, 0xF7, 0xEF, 0xDF, 0xBF, 
0x7E, 0xFD, 0xEF, 0x7B, 0xDE, 0xF7, 0x77, 0x77, 0x75, 0xB1, 0x6C, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 
0xDB, 0x6D, 0xB6, 0xDB, 0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 
0x9F, 0x35, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5F, 0xF7, 
0xFD, 0xFF, 0x7F, 0xDF, 0xF7, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7B, 0xDE, 0xF7, 0xBD, 0xDD, 
0xDD, 0xDD, 0x6C, 0x5B, 0x1B, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDF, 0x3E, 0x7C, 
0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 
0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD7, 0xFD, 0xFF, 0x7F, 0xDF, 0xF7, 0xFD, 0xFF, 0x7E, 
0xFD, 0xFB, 0xF7, 0xEF, 0xDE, 0xF7, 0xBD, 0xEF, 0x77, 0x77, 0x77, 0x5B, 0x16, 0xC6, 0xDB, 0x6D, 
0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xB7, 0xCF, 0x9F, 0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 
0x3E, 0x7C, 0xF9, 0xF3, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 
0xB5, 0xFF, 0x7F, 0xDF, 0xF7, 0xFD, 0xFF, 0x7F, 0xDF, 0xBF, 0x7E, 0xFD, 0xFB, 0xF7, 0xBD, 0xEF, 
0x7B, 0xDD, 0xDD, 0xDD, 0xD6, 0xC5, 0xB1, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 
0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7C, 0xD6, 0xB5, 0xAD, 
0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x7F, 0xDF, 0xF7, 0xFD, 0xFF, 0x7F, 
0xDF, 0xF7, 0xEF, 0xDF, 0xBF, 0x7E, 0xFD, 0xEF, 0x7B, 0xDE, 0xF7, 0x77, 0x77, 0x75, 0xB1, 0x6C, 
0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 
0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x35, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 
0xB5, 0xAD, 0x6B, 0x5F, 0xF7, 0xFD, 0xFF, 0x7F, 0xDF, 0xF7, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 
0x7B, 0xDE, 0xF7, 0xBD, 0xDD, 0xDD, 0xDD, 0x6C, 0x5B, 0x1B, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 
0x6D, 0xB6, 0xDF, 0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCD, 
0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD7, 0xFD, 0xFF, 0x7F, 
0xDF, 0xF7, 0xFD, 0xFF, 0x7E, 0xFD, 0xFB, 0xF7, 0xEF, 0xDE, 0xF7, 0xBD, 0xEF, 0x77, 0x77, 0x77, 
0x5B, 0x16, 0xC6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xB7, 0xCF, 0x9F, 0x3E, 0x7C, 
0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7C, 0xF9, 0xF3, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 
0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xFF, 0x7F, 0xDF, 0xF7, 0xFD, 0xFF, 0x7F, 0xDF, 0xBF, 0x7E, 
0xFD, 0xFB, 0xF7, 0xBD, 0xEF, 0x7B, 0xDD, 0xDD, 0xDD, 0xD6, 0xC5, 0xB1, 0xB6, 0xDB, 0x6D, 0xB6, 
0xDB, 0x6D, 0xB6, 0xDB, 0x6D, 0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7C, 0xF9, 0xF3, 0xE7, 0xCF, 0x9F, 
0x3E, 0x7C, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x6B, 0x5A, 0xD6, 0xB5, 0xAD, 0x00};



CDBOXIIRender  *gpFilter=NULL;
BOOL  gEOS_0=FALSE;
BOOL  gEOS_1=FALSE;
BOOL  gEOS_2=FALSE;

BOOL          gfTerminateThread=FALSE;
BOOL          gfThreadAborted=FALSE;

unsigned long ghAWriteThread=0;
unsigned long ghVWriteThread=0;
unsigned long ghAVWriteThread=0;

void  __cdecl AVWriteThread(void *thread_arg);
void  __cdecl AWriteThread(void *thread_arg);
void  __cdecl VWriteThread(void *thread_arg);

int gSocketVideoPES=0;
int gSocketAudioPES=0;

CCircularBuffer *CVideoBuffer=NULL;
CCircularBuffer *CAudioBuffer=NULL;
CCircularBuffer *CMultiplexBuffer=NULL;

BOOL gIsVideoConnected=FALSE;
BOOL gIsAudioConnected=FALSE;
BOOL gIsMultiplexerConnected=FALSE;

__int64 gDeliveredAudioData=0;
__int64 gDeliveredVideoData=0;

HRESULT InitSockets(void)
{
    int retval=-1;
    WSADATA WSAData;
    retval = WSAStartup(MAKEWORD(1,1), &WSAData);
    return(retval);
}


void DeInitSockets(void)
{
    WSACleanup();
}

int canTransmitData(int socket)
{
#pragma warning (disable : 4018)
    int ret;
    fd_set fd_write;
    struct timeval tv;

    FD_ZERO(&fd_write);
    FD_SET(socket, &fd_write);
    tv.tv_sec =0;
    tv.tv_usec=0;

    ret=select (0, NULL, &fd_write, NULL, &tv);

    if (ret<=0)
        return(FALSE);

    if  (FD_ISSET(socket, &fd_write))
        return(TRUE);


    return(FALSE);
#pragma warning (default : 4018)
}

//-----------------------------------------------------------
// Function: GetBuf()
//
// nOptval: SO_SNDBUF, SO_RCVBUF
//-----------------------------------------------------------
#define MAX_MTU 1460

int  GetBufTCP (SOCKET hSock, int nBigBufSize, int nOptval)
{
    int nRet, nTrySize, nFinalSize = 0;
    
    for (nTrySize=nBigBufSize; nTrySize > MAX_MTU; nTrySize >>= 1) 
    {
        nRet = setsockopt (hSock, SOL_SOCKET, nOptval, (char FAR*) &nTrySize, sizeof (int));
        if (nRet == SOCKET_ERROR) 
        {
            int WSAErr = WSAGetLastError();
            if ((WSAErr==WSAENOPROTOOPT) || (WSAErr==WSAEINVAL))
                break;
        } 
        else 
        {
            nRet = sizeof (int);
            getsockopt (hSock, SOL_SOCKET, nOptval, (char FAR *) &nFinalSize, &nRet);
            break;
        }
    }
    return (nFinalSize);
} /* end GetBuf() */

int openPES(const char * name, unsigned short port, int bsize)
{
    int ret=0;

	struct hostent * hp = gethostbyname(name);
		
	struct sockaddr_in adr;
	memset ((char *)&adr, 0, sizeof(struct sockaddr_in));

    if (hp==NULL)
        return(SOCKET_ERROR);
				
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	adr.sin_port = htons(port);
		
   if (adr.sin_addr.s_addr == 0) {
		dprintf("unable to lookup hostname !");
		return(SOCKET_ERROR);
	}
		         
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (SOCKET_ERROR == connect(sock, (sockaddr*)&adr, sizeof(struct sockaddr_in))) 
        {
		dprintf("connect failed !");
		closesocket(sock);
		return(SOCKET_ERROR);
	    }
	
	
    ret=GetBufTCP (sock, bsize, SO_SNDBUF);
    dprintf("Requested Buffer:%ld, granted Buffer:%ld",bsize,ret);

	return sock;
}

int OpenSocket(const char *name, unsigned short port)
{
    HRESULT hr=NOERROR;
    int ret=0;
	
	struct hostent * hp = gethostbyname(name);
		
	struct sockaddr_in adr;
	memset ((char *)&adr, 0, sizeof(struct sockaddr_in));

    if (hp==NULL)
        return(SOCKET_ERROR);
				
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	adr.sin_port = htons(port);
		
   if (adr.sin_addr.s_addr == 0) {
		dprintf("unable to lookup hostname !");
		return(SOCKET_ERROR);
	}
		         
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (SOCKET_ERROR == connect(sock, (sockaddr*)&adr, sizeof(struct sockaddr_in))) 
        {
		dprintf("connect failed !");
		closesocket(sock);
		return(SOCKET_ERROR);
	    }
    return(sock);
}

int FindStartCode(BYTE *buffer, int size, DWORD code)
{
    int i=0;
    int c0= code     &0x000000FF;
    int c1=(code>>8) &0x000000FF;
    int c2=(code>>16)&0x000000FF;
    int c3=(code>>24)&0x000000FF;
    for(i=0;i<size-3;i++)
        {
        if (buffer[i]==c3)
            if (buffer[i+1]==c2)
                if (buffer[i+2]==c1)
                    if (buffer[i+3]==c0)
                        return(i);
        }
    return(-1);
}

int CheckPackType(BYTE *buffer, int size)
{
    int result=-1;
    int off1=0;
    int off2=0;
    off1=FindStartCode(buffer,size,0x000001BA);
    if (off1<0)
        return(-1);
    size=size-off1;
    if (size<32)
        return(-1);
    off2=FindStartCode(buffer+off1,32,0x000001E0);
    if (off2<0)
        {
        off2=FindStartCode(buffer+off1,32,0x000001C0);
        if (off2>=0)
            return(1);
        }
    else
        return(0);

    return(result);
}

HRESULT ControlPlaybackOnDBOX(const char *name, unsigned short port, int active)
{
    HRESULT hr=NOERROR;
    int ret=0;
	
	int sock = OpenSocket(name, port);
    if (sock==SOCKET_ERROR)
        return(E_FAIL);
	
	char wbuffer[1024];		
	char wbody[1024];		


    if (active==0)
        {
	    wsprintf(wbuffer, "GET /control/zapto?startplayback HTTP/1.0\r\n");
        }
    else
    if (active==1)
	    wsprintf(wbuffer, "GET /control/zapto?stopsectionsd HTTP/1.0\r\n");
    else
	    wsprintf(wbuffer, "GET /control/zapto?stopplayback HTTP/1.0\r\n");


    wsprintf(wbody,   "User-Agent: BS\r\n"
                      "Host: %s\r\n"
                      "Pragma: no-cache\r\n"
                      "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\r\n"
                      "\r\n", name);
	

    lstrcat(wbuffer,wbody);
    ret=send(sock, wbuffer, strlen(wbuffer),0);
    closesocket(sock);

    if (ret>0)
        return(NOERROR);
    else
        return(E_FAIL);

}

HRESULT MuteAudio(const char *name, unsigned short port, BOOL flag)
{
    HRESULT hr=NOERROR;
    int ret=0;
	
	int sock = OpenSocket(name, port);
    if (sock==SOCKET_ERROR)
        return(E_FAIL);
	
	char wbuffer[1024];		
	char wbody[1024];		


    if (flag==0)
	    wsprintf(wbuffer, "GET /control/volume?unmute HTTP/1.0\r\n");
    else
	    wsprintf(wbuffer, "GET /control/volume?mute HTTP/1.0\r\n");


    wsprintf(wbody,   "User-Agent: BS\r\n"
                      "Host: %s\r\n"
                      "Pragma: no-cache\r\n"
                      "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\r\n"
                      "\r\n", name);
	

    lstrcat(wbuffer,wbody);
    ret=send(sock, wbuffer, strlen(wbuffer),0);
    closesocket(sock);

    if (ret>0)
        return(NOERROR);
    else
        return(E_FAIL);
}


#define AUDIO_LEVEL 3

void __cdecl AVWriteThread(void *thread_arg)
{	
    
    BOOL wait=TRUE;
	int ret=0;
    int off=0;
    unsigned char *bufferVideo=NULL;
    int  bufferlenVideo=VIDEO_BUFFER_SIZE;
    unsigned char *bufferAudio=NULL;
    int  bufferlenAudio=AUDIO_BUFFER_SIZE;
    unsigned char *bufferMultiplex=NULL;
    int  bufferlenMultiplex=2048;
    int nprecount=1;

    BOOL firstAudio=TRUE;
    BOOL firstVideo=TRUE;
    unsigned long start_time=0; 
    //unsigned long start_time=timeGetTime();
    int vdelay=0;

    bufferVideo    =(unsigned char *)malloc(bufferlenVideo);
    bufferAudio    =(unsigned char *)malloc(bufferlenAudio);
    bufferMultiplex=(unsigned char *)malloc(bufferlenMultiplex);

#if 1
    if ((gSocketAudioPES>0)&&(gSocketVideoPES==0)&&(CMultiplexBuffer==NULL))
        {
        
        for(int i=0;i<200;i++)
            {
            ret=send(gSocketAudioPES, (char *)gSilentMPAFrame, 768+54, 0);
            Sleep(0);
            }
        
        }
#endif

//    Sleep(1000);

    for (;;) 
        {
		if (gfTerminateThread) 
			break;

        wait=TRUE;
        ret=0;

        if ((gSocketAudioPES>0)&&(CMultiplexBuffer==NULL))
            {
            if (CAudioBuffer==NULL)
                {
                dprintf("Thread Audio error");
                break;
                }

            if (canTransmitData(gSocketAudioPES))
                {
                if (CAudioBuffer->canRead(0, bufferlenAudio))
                    {
                    CAudioBuffer->Read(0, bufferAudio, bufferlenAudio);
                    ret=send(gSocketAudioPES, (char *)(bufferAudio), (bufferlenAudio), 0);

                    gDeliveredAudioData+=(__int64)bufferlenAudio;
                    //dprintf("OUT A: %ld",ret);
                    wait=FALSE;
                    if (start_time==0)
                        start_time=timeGetTime();
                    if (ret!=bufferlenAudio)
                        {
                        dprintf("audio socket write error");
                        }
                    if (vdelay==0)
                        {
                        //Sleep(500);
                        vdelay=1;
                        }
                    }
                else
                    {
                    if (gEOS_1)
                        break;
                    }
                }
            }
        else
            vdelay=1;            

        if ((gSocketVideoPES>0)&&(CMultiplexBuffer==NULL) & vdelay & wait)
            {
            if (CVideoBuffer==NULL)
                {
                dprintf("Thread Video error");
                break;
                }

            if (canTransmitData(gSocketVideoPES))
                {
                if (CVideoBuffer->canRead(0, bufferlenVideo))
                    {
                    CVideoBuffer->Read(0, bufferVideo, bufferlenVideo);
                    if (vdelay)
                        {
                        gDeliveredVideoData+=(__int64)bufferlenVideo;
                        ret=send(gSocketVideoPES, (char *)bufferVideo, bufferlenVideo, 0);
                        }
                    //dprintf("OUT V: %ld",ret);
                    wait=FALSE;
                    /*
                    if (ret!=bufferlenVideo)
                        {
                        dprintf("video socket write error");
                        }
                    */
                    }
                else
                    {
                    if (gEOS_0)
                        break;
                    }
                }
            }

        if ((gSocketVideoPES>0) && (gSocketAudioPES>0))
            {
            if (CMultiplexBuffer!=NULL)
                {
                if (CMultiplexBuffer->canRead(0, bufferlenMultiplex))
                    {
                    int type=-1;
                    wait=FALSE;
                    CMultiplexBuffer->Read(0, bufferMultiplex, bufferlenMultiplex);
                    type=CheckPackType(bufferMultiplex, bufferlenMultiplex);
                    ret=2048;
                    if (type==0)
                        {
                        
                        while(!canTransmitData(gSocketVideoPES))
                            {
                            Sleep(2);

		                    if (gfTerminateThread) 
			                    break;
                            }
                        
                        if (nprecount==0)
                            {
                            gDeliveredVideoData+=(__int64)bufferlenMultiplex;
                            ret=send(gSocketVideoPES, (char *)bufferMultiplex, bufferlenMultiplex, 0);
                            dprintf("OUT V: %ld",ret);
                            }
                        }
                    else
                    if (type==1)
                        {
                        
                        while(!canTransmitData(gSocketAudioPES))
                            {
                            Sleep(2);

		                    if (gfTerminateThread) 
			                    break; 
                            }
                        
                        gDeliveredAudioData+=(__int64)bufferlenMultiplex;
                        ret=send(gSocketAudioPES, (char *)bufferMultiplex, bufferlenMultiplex, 0);
                        if (nprecount>0)
                            nprecount--;
                        dprintf("OUT A: %ld",ret);
                        }
                    else
                        {
                        dprintf("unknown packet");
                        }
                    if (ret!=2048)
                        {
                        dprintf("mux write error");
                        }
                    }
                else
                    {
                    if (gEOS_2)
                        break;
                    }
                }
            }

        if (ret<0)
            break;

        if (wait)
            Sleep(2);
        }

    gfThreadAborted=TRUE;

    if (gpFilter!=NULL)
        if (gEOS_1)
            gpFilter->NotifyEvent(EC_USERABORT, S_OK, 0);

    free(bufferVideo);
    free(bufferAudio);
    free(bufferMultiplex);
}

void __cdecl AWriteThread(void *thread_arg)
{	
    
    BOOL wait=TRUE;
	int ret=0;
    int off=0;
    unsigned char *bufferAudio=NULL;
    int  bufferlenAudio=AUDIO_BUFFER_SIZE;

    bufferAudio    =(unsigned char *)malloc(bufferlenAudio);
    
    Sleep(1000);

    for (;;) 
        {
		if (gfTerminateThread) 
			break;

        wait=TRUE;
        ret=0;

        if (gSocketAudioPES>0)
            {
            if (CAudioBuffer==NULL)
                {
                dprintf("Thread Audio error");
                break;
                }

            if (canTransmitData(gSocketAudioPES))
                {
                if (CAudioBuffer->canRead(0, bufferlenAudio))
                    {
                    CAudioBuffer->Read(0, bufferAudio, bufferlenAudio);
                    ret=bufferlenAudio;
                    ret=send(gSocketAudioPES, (char *)bufferAudio, bufferlenAudio, 0);
                    //dprintf("OUT A: %ld",ret);
                    wait=FALSE;
                    if (ret!=bufferlenAudio)
                        {
                        dprintf("audio socket write error");
                        }
                    }
                else
                    {
                    if (gEOS_1)
                        break;
                    }
                }
            }
        else
            break;            

        if (ret<0)
            break;

        if (wait)
            Sleep(2);
        }

    gfThreadAborted=TRUE;

    if (gpFilter!=NULL)
        if (gEOS_1)
            gpFilter->NotifyEvent(EC_USERABORT, S_OK, 0);

    free(bufferAudio);
}

void __cdecl VWriteThread(void *thread_arg)
{	
    
    BOOL wait=TRUE;
	int ret=0;
    int off=0;
    unsigned char *bufferVideo=NULL;
    int  bufferlenVideo=VIDEO_BUFFER_SIZE;

    bufferVideo    =(unsigned char *)malloc(bufferlenVideo);

    
    Sleep(0);

    for (;;) 
        {
		if (gfTerminateThread) 
			break;

        wait=TRUE;
        ret=0;


        if (gSocketVideoPES>0)
            {
            if (CVideoBuffer==NULL)
                {
                dprintf("Thread Video error");
                break;
                }

            if (canTransmitData(gSocketVideoPES))
                {
                if (CVideoBuffer->canRead(0, bufferlenVideo))
                    {
                    CVideoBuffer->Read(0, bufferVideo, bufferlenVideo);
                    ret=send(gSocketVideoPES, (char *)bufferVideo, bufferlenVideo, 0);
                    //dprintf("OUT V: %ld",ret);
                    wait=FALSE;
                    if (ret!=bufferlenVideo)
                        {
                        dprintf("video socket write error");
                        }
                    }
                else
                    {
                    if (gEOS_0)
                        break;
                    }
                }
            }

        if (ret<0)
            break;

        if (wait)
            Sleep(2);
        }

    gfThreadAborted=TRUE;

    if (gpFilter!=NULL)
        if (gEOS_0)
            gpFilter->NotifyEvent(EC_USERABORT, S_OK, 0);

    free(bufferVideo);
}

void DeInitStreaming()
{
    gfTerminateThread=TRUE;
    Sleep(500);
    
    if (gSocketVideoPES>0)
        closesocket(gSocketVideoPES);
    gSocketVideoPES=0;

    if (gSocketAudioPES>0)
        closesocket(gSocketAudioPES);
    gSocketAudioPES=0;

    if (CVideoBuffer)
        {
        CVideoBuffer->Interrupt();
        CVideoBuffer->DeInitialize();
        delete CVideoBuffer;
        CVideoBuffer=NULL;
        }

    if (CAudioBuffer)
        {
        CAudioBuffer->Interrupt();
        CAudioBuffer->DeInitialize();
        delete CAudioBuffer;
        CAudioBuffer=NULL;
        }
    
    if (CMultiplexBuffer)
        {
        CMultiplexBuffer->Interrupt();
        CMultiplexBuffer->DeInitialize();
        delete CMultiplexBuffer;
        CMultiplexBuffer=NULL;
        }
}


HRESULT InitStreaming(char *IPAddress, int aport, int vport) 
{
    CMultiplexBuffer=NULL;
    CVideoBuffer=NULL;
    CAudioBuffer=NULL;

/*
    if (gIsMultiplexerConnected && (vport>0) && (aport>0))
        {
        CMultiplexBuffer=new CCircularBuffer();

        CMultiplexBuffer->Initialize((VIDEO_BUFFER_SIZE+AUDIO_BUFFER_SIZE) * BUFFER_COUNT, 1);
        CMultiplexBuffer->Clear();

	    gSocketVideoPES = openPES(IPAddress, vport, 2048);
	    if (gSocketVideoPES < 0) 
            return(E_FAIL);

	    gSocketAudioPES = openPES(IPAddress, aport, 2048);
	    if (gSocketAudioPES < 0) 
            return(E_FAIL);
        }
    else
*/
        {
        if (vport>0)
            {
            CVideoBuffer=new CCircularBuffer();
            CVideoBuffer->Initialize(VIDEO_BUFFER_SIZE * BUFFER_COUNT, 1);
            CVideoBuffer->Clear();

	        gSocketVideoPES = openPES(IPAddress, vport, VIDEO_BUFFER_SIZE);
	        if (gSocketVideoPES < 0) 
                return(E_FAIL); 
            }
        else
            {
            CVideoBuffer=NULL;
            gSocketVideoPES=0;
            }

        if (aport>0)
            {
            CAudioBuffer=new CCircularBuffer();
            CAudioBuffer->Initialize(AUDIO_BUFFER_SIZE * BUFFER_COUNT, 1);
            CAudioBuffer->Clear();
            
            gSocketAudioPES = openPES(IPAddress, aport, AUDIO_BUFFER_SIZE);
	        if (gSocketAudioPES < 0) 
                return(E_FAIL);
            }
        else
            {
            CAudioBuffer=NULL;
            gSocketAudioPES=0;
            }
        }
    
    gfThreadAborted=FALSE;
    gfTerminateThread=FALSE;

    gDeliveredAudioData=0;
    gDeliveredVideoData=0;

#if 0
    if (gSocketAudioPES)
	    ghAWriteThread=_beginthread(AWriteThread , 0, NULL);
    if (gSocketVideoPES)
	    ghVWriteThread=_beginthread(VWriteThread , 0, NULL);

    SetThreadPriority((HANDLE)ghAWriteThread, THREAD_PRIORITY_ABOVE_NORMAL);
    SetThreadPriority((HANDLE)ghVWriteThread, THREAD_PRIORITY_ABOVE_NORMAL);
//    SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
#else
	ghAVWriteThread=_beginthread(AVWriteThread , 0, NULL);
    SetThreadPriority((HANDLE)ghAVWriteThread, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

	return(NOERROR);
}

