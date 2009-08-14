//
//  DBOXII Capture Filter
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

#include "debug.h"
#include "ccircularbuffer.h"
#include "Remuxer.h"
#include "grab.h"

// -------------------------------------------
char         *gChannelNameList[MAX_LIST_ITEM];
__int64       gChannelIDList[MAX_LIST_ITEM];
__int64       gTotalChannelCount=-1;
// -------------------------------------------
BOOL          gfTerminateThread=FALSE;
BOOL          gfThreadAborted=FALSE;
unsigned long ghAVReadThread=0;
void  __cdecl AVReadThread(void *thread_arg);

int gSocketVideoPES=0;
int gSocketAudioPES=0;

CCircularBuffer *CMultiplexBuffer=NULL;
CCircularBuffer *pHTMLCircularBuffer=NULL;

Remuxer         *CRemuxer=NULL;
BOOL            gIsVideoConnected=FALSE;
BOOL            gIsAudioConnected=FALSE;
BOOL            gIsPSPinConnected=FALSE;

volatile __int64         gTotalVideoDataCount=0;
volatile __int64         gTotalAudioDataCount=0;
__int64         gLastVideoDataCount=0;
__int64         gLastAudioDataCount=0;
long            gLastAVBitrateRequest=0;

BOOL            gFakeisDataAvailable=FALSE;

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

HRESULT ReadCompleteDataFromSocket(SOCKET s)
{
    HRESULT hr=NOERROR;
    unsigned long avail=0;
    int ret=0;
    int i=0;

    if (!pHTMLCircularBuffer)
        return(E_UNEXPECTED);
    pHTMLCircularBuffer->Clear();

    hr=WaitForSocketData(s, &avail, 1000);
    if (FAILED(hr)||(avail==0))
        return(hr);

    while(TRUE)  
        {
        if ((ret==0)&&(avail>0))
            {
            char rbuffer[1024];
            ZeroMemory(rbuffer,sizeof(rbuffer));
            ret=recv(s,rbuffer,sizeof(rbuffer),0);
            if (ret>0)
                {
                if (!pHTMLCircularBuffer->canWrite(ret))
                    return(E_UNEXPECTED);
                pHTMLCircularBuffer->Write((BYTE *)rbuffer, ret);
                i=0;
                }
            }
        ret=WaitForSocketData(s, &avail, 50);
        if (ret<0)
            break;
        if (i++>10)
            break;
        }

    return(NOERROR);
}

HRESULT WaitForSocketData(SOCKET sock, unsigned long *avail, long tim)
{
    int ret=0;
    int count=tim/10;
    int i=0;

    if (avail==NULL)
        return(E_POINTER);
    *avail=0;

    for(i=0;i<count;i++)
        {
        Sleep(10);
        ret=ioctlsocket(sock, FIONREAD, avail);
        if (ret<0)
            return(E_FAIL);
        if (*avail>0)
            break;
        }

    return(NOERROR);
}


int isDataAvailable(int socket, unsigned long size)
{
    int ret=0;
    unsigned long avail=0;

    if (gFakeisDataAvailable)
        return(TRUE);

    ret=ioctlsocket(socket, FIONREAD, &avail);

    if (ret!=0)
        return(FALSE);

    if (avail>=size)
        return(TRUE);
    return(FALSE);
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

int openPES(const char * name, unsigned short port, int pid, int bsize)
{
    int ret=0;
	dprintf("opening PES %s:%d PID %d", name, (int)port, pid);
	
	struct hostent * hp = gethostbyname(name);
		
	struct sockaddr_in adr;
	memset ((char *)&adr, 0, sizeof(struct sockaddr_in));

    if (hp==NULL)
        return(SOCKET_ERROR);
				
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	adr.sin_port = htons(port);
		
    if (adr.sin_addr.s_addr == 0) 
        {
		dprintf("unable to lookup hostname !");
		return(SOCKET_ERROR);
	    }
		         
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (SOCKET_ERROR == connect(sock, (sockaddr*)&adr, sizeof(struct sockaddr_in))) 
        {
		dprintf("connect failed !");
		//EmptySocket(sock);
        closesocket(sock);
		return(SOCKET_ERROR);
	    }
	
	char buffer[264];		
	wsprintf(buffer, "GET /%x HTTP/1.0\r\n\r\n", pid);
	
    ret=GetBufTCP (sock, bsize, SO_RCVBUF);
    dprintf("Requested Buffer:%lu, granted Buffer:%lu",bsize,ret);

    ret=send(sock, buffer, strlen(buffer),0);

	return sock;
}

int openPS(const char * name, unsigned short port, int vpid, int apid, int bsize)
{
    int ret=0;
	dprintf("opening PS %s:%d APID %d, VPID %d", name, (int)port, apid, vpid);
	
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
		//EmptySocket(sock);
		closesocket(sock);
		return(SOCKET_ERROR);
	    }
	
	char buffer[264];		
	wsprintf(buffer, "GET /%x %x HTTP/1.0\r\n\r\n", apid,vpid);
	
    ret=GetBufTCP (sock, bsize, SO_RCVBUF);
    dprintf("Requested Buffer:%lu, granted Buffer:%lu",bsize,ret);

    ret=send(sock, buffer, strlen(buffer),0);

	return sock;
}

char *MYstrstr(char *str, char *token)
{
    int i,j;

    if (token==NULL)
        return(NULL);

    if (str==NULL)
        return(NULL);

    int tlen=lstrlen(token);
    if (tlen==0)
        return(NULL);

    int slen=lstrlen(str);
    if (slen==0)
        return(NULL);

    if (slen<tlen)
        return(NULL);

    for(i=0;i<=(slen-tlen);i++)
        {
        int found=1;
        for(j=0;j<tlen;j++)
            {
            if (str[i+j]!=token[j])
                {
                found=0;
                break;
                }
            }
        if (found)
            return(str+i+tlen);
        }

    return(NULL);
}

/*
HRESULT EmptySocket(SOCKET s)
{
    #define MAXCLOOP    50
    int count=MAXCLOOP;
    int ret=0;
    unsigned long avail=0;
// -------------------------------------------------------------------
    {
	LINGER ling;
	ling.l_onoff=1;
	ling.l_linger=5;
	setsockopt(s,SOL_SOCKET,SO_LINGER,(LPSTR)&ling,sizeof(ling));
    }

    GetBufTCP (s, 4096, SO_RCVBUF);
    
    while(TRUE)
        {
        if ((ret>=0) && (avail>0))
            {
            char *rbuffer=(char *)malloc(avail);
            ret=recv(s,rbuffer,avail,0);
            free(rbuffer);
            count=MAXCLOOP;
            }
        else 
            {
            count--;
            Sleep(10);
            }

        ret=ioctlsocket(s, FIONREAD, &avail);

        if ((ret<0) || (count<0))
            break;
        }
// -------------------------------------------------------------------
    return(ret);
}
*/

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

HRESULT SetChannel(const char *name, unsigned short port, unsigned long channel)
{
    HRESULT hr=NOERROR;
    int ret=0;

    dprintf("SetChannel from %s:%d to channel:%lu", name, (int)port, channel);
    	
	int sock = OpenSocket(name, port);
    if (sock==SOCKET_ERROR)
        return(E_FAIL);
	
	char wbuffer[1024];		
	char wbody[1024];		

    //!!BS attention data is delivered in SIGNED format !! (who the hack brought up this idea ...)
    //!!BS internally we stay with unsigned format (of course)
    wsprintf(wbuffer, "GET /control/zapto?%ld HTTP/1.0\r\n", channel);
    wsprintf(wbody,   "User-Agent: BS\r\n"
                      "Host: %s\r\n"
                      "Pragma: no-cache\r\n"
                      "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\r\n"
                      "\r\n", name);
	

    lstrcat(wbuffer,wbody);
    ret=send(sock, wbuffer, strlen(wbuffer),0);

    hr=ReadCompleteDataFromSocket(sock);
    if (SUCCEEDED(hr))
        {
        long len=0;
        pHTMLCircularBuffer->Remain(0, &len);
        if (len>0)
            {
            char *rbuffer=(char *)malloc(len+2);
            ZeroMemory(rbuffer, len+2);
            pHTMLCircularBuffer->Read(0, (BYTE *)rbuffer, len);
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
            char *p1=NULL;
            char *p2=NULL;
            hr=E_FAIL;
            if (!strncmp(rbuffer,"HTTP",4))
                {
                p1=MYstrstr(rbuffer,"\n\n");
                if (p1==NULL)
                    p1=MYstrstr(rbuffer,"\r\n\r\n");
                if (p1!=NULL)
                    {
                    if (lstrcmpi(p1,"ok")==0)
                        {
                        hr=NOERROR;
                        }
                    }
                }
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
                            
            free(rbuffer);
            }
        }

    if (FAILED(hr))
        {
        dprintf("SetChannel FAILED !!!");
        }

    closesocket(sock);

    return(hr);
}

HRESULT GetChannel(const char *name, unsigned short port, unsigned long *channel)
{
    HRESULT hr=NOERROR;
    int ret=0;
	
    *channel=0;
    dprintf("GetChannel from %s:%d ", name, (int)port);

	int sock = OpenSocket(name, port);
    if (sock==SOCKET_ERROR)
        return(E_FAIL);
	
	char wbuffer[1024];		
	char wbody[1024];		

    wsprintf(wbuffer, "GET /control/zapto HTTP/1.0\r\n");
    wsprintf(wbody,   "User-Agent: BS\r\n"
                      "Host: %s\r\n"
                      "Pragma: no-cache\r\n"
                      "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\r\n"
                      "\r\n", name);
	

    lstrcat(wbuffer,wbody);
    ret=send(sock, wbuffer, strlen(wbuffer),0);


    hr=ReadCompleteDataFromSocket(sock);
    if (SUCCEEDED(hr))
        {
        long len=0;
        pHTMLCircularBuffer->Remain(0, &len);
        if (len>0)
            {
            char *rbuffer=(char *)malloc(len+2);
            ZeroMemory(rbuffer, len+2);
            pHTMLCircularBuffer->Read(0, (BYTE *)rbuffer, len);
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
            char *p1=NULL;
            char *p2=NULL;
            hr=E_FAIL;
            if (!strncmp(rbuffer,"HTTP",4))
                {
                p1=MYstrstr(rbuffer,"\n\n");
                if (p1==NULL)
                    p1=MYstrstr(rbuffer,"\r\n\r\n");
                if (p1!=NULL)
                    {
                    int pos=strlen(p1)-1;
                    if (pos>0)
                        p1[pos]=0;
                    *channel=atol(p1);
                    //dprintf(p1);
                    }
                }
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
                            
            free(rbuffer);
            }
        }

    closesocket(sock);

    if (*channel==0)
        return(E_FAIL);
    else
        return(NOERROR);
}

HRESULT GetChannelInfo(const char *name, unsigned short port, unsigned long channel, char *info)
{
    HRESULT hr=NOERROR;
    int ret=0;
    int i=0;
    unsigned long avail=0;
    time_t tsec=0;
    char firstInfo[264];
    
    lstrcpy(firstInfo,"");
    _tzset();
    time(&tsec);
	
    lstrcpy(info,"");

    dprintf("GetChannelInfo from %s:%d ", name, (int)port);

	int sock = OpenSocket(name, port);
    if (sock==SOCKET_ERROR)
        return(E_FAIL);
	
	char wbuffer[1024];		
	char wbody[1024];		

    wsprintf(wbuffer, "GET /control/epg?%lu HTTP/1.0\r\n", channel);
    wsprintf(wbody,   "User-Agent: BS\r\n"
                      "Host: %s\r\n"
                      "Pragma: no-cache\r\n"
                      "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\r\n"
                      "\r\n", name);
	

    lstrcat(wbuffer,wbody);
    ret=send(sock, wbuffer, strlen(wbuffer),0);

    hr=ReadCompleteDataFromSocket(sock);

    if (SUCCEEDED(hr))
        {
        long len=0;
        pHTMLCircularBuffer->Remain(0, &len);
        if (len>0)
            {
            char *rbuffer=(char *)malloc(len+2);
            ZeroMemory(rbuffer, len+2);
            pHTMLCircularBuffer->Read(0, (BYTE *)rbuffer, len);
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
            char *p1=NULL;
            char *p2=NULL;
            int found = 0;
            if (!strncmp(rbuffer,"HTTP",4))
                {
                p1=MYstrstr(rbuffer,"\n\n");
                if (p1==NULL)
                    p1=MYstrstr(rbuffer,"\r\n\r\n");
                if (p1!=NULL)
                    p2=MYstrstr(p1,"\n");
            
                while(!found)
                    {
                    if (p2!=NULL)
                        {
                        if (lstrlen(p2)>1)  //ignore trailing cr or lf
                            {
                            char szEPGID[264]="";
                            char szEPGDate[264]="";
                            char szEPGTime[264]="";
                            char szEPGTitle[264]="";
                            unsigned long ltim=0;
                            p2--;
                            *p2=0;
                            lstrcpyn(info, p1, 264);
                            if (lstrlen(firstInfo)==0)
                                lstrcpy(firstInfo,info);
                            sscanf(info,"%s %s %s %s",szEPGID, szEPGDate, szEPGTime, szEPGTitle);
                            //dprintf(p1);
                            ltim=atol(szEPGDate)+atol(szEPGTime);
                            if (ltim>=(unsigned long)tsec)
                                found=1;
                            else
                                lstrcpy(info,"");
                            }
                        p1=p2+1;
                        p2=MYstrstr(p1,"\n");
                        }
                    else
                        break;
                    }
                }
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
                            
            free(rbuffer);
            }
        }
    closesocket(sock);

    if (lstrlen(info)==0)
        lstrcpy(info, firstInfo);
    
    if (lstrlen(info)==0)
        return(E_FAIL);
    else
        return(NOERROR);
}

HRESULT GetEPGInfo(const char *name, unsigned short port, char *eventid, char *info)
{
    HRESULT hr=NOERROR;
    int ret=0;
	
    lstrcpy(info,"");

    dprintf("GetEPGInfo from %s:%d ", name, (int)port);

	int sock = OpenSocket(name, port);
    if (sock==SOCKET_ERROR)
        return(E_FAIL);
	
	char wbuffer[1024];		
	char wbody[1024];		

    wsprintf(wbuffer, "GET /control/epg?eventid=%s HTTP/1.0\r\n", eventid);
    wsprintf(wbody,   "User-Agent: BS\r\n"
                      "Host: %s\r\n"
                      "Pragma: no-cache\r\n"
                      "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\r\n"
                      "\r\n", name);
	

    lstrcat(wbuffer,wbody);
    ret=send(sock, wbuffer, strlen(wbuffer),0);


    hr=ReadCompleteDataFromSocket(sock);

    if (SUCCEEDED(hr))
        {
        long len=0;
        pHTMLCircularBuffer->Remain(0, &len);
        if (len>0)
            {
            char *rbuffer=(char *)malloc(len+2);
            ZeroMemory(rbuffer, len+2);
            pHTMLCircularBuffer->Read(0, (BYTE *)rbuffer, len);
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
            char *p1=NULL;
            char *p2=NULL;
            if (!strncmp(rbuffer,"HTTP",4))
                {
                p1=MYstrstr(rbuffer,"\n\n");
                if (p1==NULL)
                    p1=MYstrstr(rbuffer,"\r\n\r\n");

                if (p1!=NULL)
                    {
                    if (lstrlen(p1)>1)
                        {
                        lstrcpyn(info, p1, 1024);
                        }
                    }
                }
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
            free(rbuffer);
            }
        }

    closesocket(sock);

    if (lstrlen(info)==0)
        return(E_FAIL);
    else
        return(NOERROR);
}

HRESULT ControlPlaybackOnDBOX(const char *name, unsigned short port, int active)
{
    HRESULT hr=NOERROR;
    int ret=0;
    unsigned long avail=0;
	
    dprintf("ControlPlaybackOnDBOX from %s:%d ", name, (int)port);

	int sock = OpenSocket(name, port);
    if (sock==SOCKET_ERROR)
        return(E_FAIL);
	
	char wbuffer[1024];		
	char wbody[1024];		


    if (active==0)
        {
	    //wsprintf(wbuffer, "GET /control/zapto?startsectionsd HTTP/1.0\r\n");
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

    WaitForSocketData(sock, &avail, 5000);

//	EmptySocket(sock);
    closesocket(sock);

    if (ret>0)
        return(NOERROR);
    else
        return(E_FAIL);

}

HRESULT RetrievePIDs(int *vpid, int *apid, const char *name, unsigned short port)
{
    HRESULT hr=NOERROR;
    int ret=0;
    int i=0;
    int gotAudio=0;
    int gotVideo=0;
	
    *apid=0;
    *vpid=0;
    
    dprintf("retrieving PIDs from %s:%d ", name, (int)port);
	
	int sock = OpenSocket(name, port);
    if (sock==SOCKET_ERROR)
        return(E_FAIL);
	
	char wbuffer[1024];		
	char wbody[1024];		
	wsprintf(wbuffer, "GET /control/zapto?getpids HTTP/1.0\r\n");

    wsprintf(wbody,   "User-Agent: BS\r\n"
                      "Host: %s\r\n"
                      "Pragma: no-cache\r\n"
                      "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\r\n"
                      "\r\n", name);
	

    lstrcat(wbuffer,wbody);
    ret=send(sock, wbuffer, strlen(wbuffer),0);

    hr=ReadCompleteDataFromSocket(sock);

    if (SUCCEEDED(hr))
        {
        long len=0;
        pHTMLCircularBuffer->Remain(0, &len);
        if (len>0)
            {
            char *rbuffer=(char *)malloc(len+2);
            ZeroMemory(rbuffer, len+2);
            pHTMLCircularBuffer->Read(0, (BYTE *)rbuffer, len);
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
            char *p1=NULL;
            char *p2=NULL;
            if (!strncmp(rbuffer,"HTTP",4))
                {
                p1=MYstrstr(rbuffer,"\n\n");
                if (p1==NULL)
                    p1=MYstrstr(rbuffer,"\r\n\r\n");

                if (p1!=NULL)
                    p2=MYstrstr(p1,"\n");
                if (p1!=NULL)
                    {
                    for(i=0;i<lstrlen(p1);i++)
                        {
                        if (p1[i]=='\n') 
                            {
                            p1[i]=0;
                            break;
                            }
                        }
                    *vpid=atoi(p1);
                    gotVideo=1;
                    }
                if (p2!=NULL)
                    {
                    for(i=0;i<lstrlen(p2);i++)
                        {
                        if (p2[i]=='\n') 
                            {
                            p2[i]=0;
                            break;
                            }
                        }
                    *apid=atoi(p2);
                    gotAudio=1;
                    }
                }
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
            free(rbuffer);
            }
        }


    dprintf("APID: %ld, VPID:%ld",*apid, *vpid);
    closesocket(sock);
	return(hr);
}

HRESULT CheckBoxStatus(const char *name, unsigned short port)
{
#pragma warning (disable : 4018)

    fd_set fd_write, fd_except;
    struct timeval tv;
    HRESULT hr=NOERROR;
    int ret=0;

    dprintf("CheckBoxStatus from %s:%d ", name, (int)port);
	
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

    u_long val=1;
    //put socket to nonblocking mode
    ret=ioctlsocket (sock, FIONBIO, &val);

	ret=connect(sock, (sockaddr*)&adr, sizeof(struct sockaddr_in));

    FD_ZERO(&fd_write);
    FD_SET((int)sock, &fd_write);
    FD_ZERO(&fd_except);
    FD_SET((int)sock, &fd_except);
    tv.tv_sec =5;
    tv.tv_usec=0;
    ret=select (0, NULL, &fd_write, &fd_except, &tv);

// 	EmptySocket(sock);
    closesocket(sock);

    if (ret<=0)
        return(SOCKET_ERROR);
    return(NOERROR);

#pragma warning (default : 4018)
}

HRESULT RetrieveStreamInfo(int *width, int *height, int *bitrate, int *is4By3, const char *name, unsigned short port)
{
    #define RCV_BUFFER_SIZE 1024 //(1024*1024)

    HRESULT hr=NOERROR;
    int ret=0;
    unsigned long avail=0;
	
    *width=0;
    *height=0;
    *bitrate=0;
    *is4By3=1;
    int i=0;
    
    dprintf("RetrieveStreamInfo from %s:%d ", name, (int)port);

	int sock = OpenSocket(name, port);
    if (sock==SOCKET_ERROR)
        return(E_FAIL);
	
    ret=GetBufTCP (sock, RCV_BUFFER_SIZE, SO_RCVBUF);

	char wbuffer[1024];		
	char wbody[1024];		
	wsprintf(wbuffer, "GET /control/info?streaminfo HTTP/1.0\r\n");

    wsprintf(wbody,   "User-Agent: BS\r\n"
                      "Host: %s\r\n"
                      "Pragma: no-cache\r\n"
                      "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\r\n"
                      "\r\n", name);
	

    lstrcat(wbuffer,wbody);
    ret=send(sock, wbuffer, strlen(wbuffer),0);

    hr=ReadCompleteDataFromSocket(sock);

    if (SUCCEEDED(hr))
        {
        long len=0;
        pHTMLCircularBuffer->Remain(0, &len);
        if (len>0)
            {
            char *rbuffer=(char *)malloc(len+2);
            ZeroMemory(rbuffer, len+2);
            pHTMLCircularBuffer->Read(0, (BYTE *)rbuffer, len);
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
            char *p1=NULL;
            char *p2=NULL;
            char *p3=NULL;
            char *p4=NULL;
            if (!strncmp(rbuffer,"HTTP",4))
                {
                p1=MYstrstr(rbuffer,"\n\n");
                if (p1==NULL)
                    p1=MYstrstr(rbuffer,"\r\n\r\n");


                if (p1!=NULL)
                    p2=MYstrstr(p1,"\n");
                if (p1!=NULL)
                    {
                    for(i=0;i<lstrlen(p1);i++)
                        if (p1[i]=='\n') {p1[i]=0;break;}
                    *width=atoi(p1);
                    }
                if (p2!=NULL)
                    p3=MYstrstr(p2,"\n");
                if (p2!=NULL)
                    {
                    for(i=0;i<lstrlen(p2);i++)
                        if (p2[i]=='\n') {p2[i]=0;break;}
                    *height=atoi(p2);
                    }
                if (p3!=NULL)
                    p4=MYstrstr(p3,"\n");
                if (p3!=NULL)
                    {
                    for(i=0;i<lstrlen(p3);i++)
                        if (p3[i]=='\n') {p3[i]=0;break;}
                    *bitrate=atoi(p3);
                    }
                if (p4!=NULL)
                    {
                    for(i=0;i<lstrlen(p4);i++)
                        if (p4[i]=='\n') {p4[i]=0;break;}
                    if (lstrcmp(p4,"4:3"))
                        *is4By3=0;
                    }

                }
            // ---------------------------------------------------    
            //
            // ---------------------------------------------------    
            free(rbuffer);
            }
        }


    closesocket(sock);
	return(hr);
}

HRESULT RetrieveChannelList(const char *name, unsigned short port, char *szName, __int64 *count)
{
    #define RCV_BUFFER_SIZE 1024 //(1024*1024)

    HRESULT hr=NOERROR;
    int ret=0;
    char rem_str[1024];
	
    lstrcpy(rem_str,"");
    if (*count<0)
        {
        dprintf("RetrieveChannelList from %s:%d ", name, (int)port);
    
        gTotalChannelCount=-1;

        for(int i=0;i<MAX_LIST_ITEM;i++)
            {
            if (gChannelNameList[i]!=NULL)
                free(gChannelNameList[i]);
            gChannelNameList[i]=NULL;
            gChannelIDList[i]=0;
            }

	    int sock = OpenSocket(name, port);
        if (sock==SOCKET_ERROR)
            return(E_FAIL);
	    
        ret=GetBufTCP (sock, RCV_BUFFER_SIZE, SO_RCVBUF);

	    char wbuffer[1024];		
	    char wbody[1024];		
	    wsprintf(wbuffer, "GET /control/channellist HTTP/1.0\r\n");

        wsprintf(wbody,   "User-Agent: BS\r\n"
                          "Host: %s\r\n"
                          "Pragma: no-cache\r\n"
                          "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\r\n"
                          "\r\n", name);
	    

        lstrcat(wbuffer,wbody);
        ret=send(sock, wbuffer, strlen(wbuffer),0);

        hr=ReadCompleteDataFromSocket(sock);

        if (SUCCEEDED(hr))
            {
            long len=0;
            pHTMLCircularBuffer->Remain(0, &len);
            if (len>0)
                {
                char *rbuffer=(char *)malloc(len+2);
                ZeroMemory(rbuffer, len+2);
                pHTMLCircularBuffer->Read(0, (BYTE *)rbuffer, len);
                // ---------------------------------------------------    
                //
                // ---------------------------------------------------    
                char *p1=NULL;
                char *p2=NULL;
                if (!strncmp(rbuffer,"HTTP",4))
                    {
                    p1=MYstrstr(rbuffer,"\n\n");
                    if (p1==NULL)
                        p1=MYstrstr(rbuffer,"\r\n\r\n");

                    while (p1!=NULL)
                        {
                        if (*p1!=0)
                            {
                            int nlfound=0;
                            unsigned int k;
                            unsigned long lval=0;
                            char *sval=NULL;
                            if (gTotalChannelCount>=(MAX_LIST_ITEM-1))
                                break;
                            gTotalChannelCount++;
                            for(i=0;i<lstrlen(p1);i++)
                                {
                                if (p1[i]=='\n') 
                                    {
                                    p2=p1+i+1;p1[i]=0;
                                    nlfound=1;
                                    break;
                                    }
                                }
                            if (!nlfound)
                                {
                                if (lstrlen(p1)<1024)
                                    lstrcpy(rem_str, p1);
                                gTotalChannelCount--;
                                break;
                                }
                            dprintf(p1);
                            sscanf(p1,"%lu", &lval);
                            sval=p1;
                            for(k=0;k<strlen(p1)-1;k++)   
                                {
                                if (p1[k]==' ')
                                    {
                                    sval=p1+k+1;
                                    break;
                                    }
                                }
                            gChannelNameList[gTotalChannelCount]=(char *)malloc(264);
                            lstrcpyn(gChannelNameList[gTotalChannelCount], sval, 264);
                            gChannelIDList[gTotalChannelCount]=lval;
                            p1=p2;
                            }
                        else
                            break;
                        }


                    }
                // ---------------------------------------------------    
                //
                // ---------------------------------------------------    
                free(rbuffer);
                }
            }

        closesocket(sock);
        *count=gTotalChannelCount;
        return(hr);
        }

    if (*count>gTotalChannelCount)
        {
        *count=-1;
        return(E_FAIL);
        }
    else
        {
        __int64 index=*count;
        if (gChannelNameList[index]!=NULL)
            {
            lstrcpyn(szName, gChannelNameList[index], 264);
            *count=gChannelIDList[index];
            }
        else
            {
            dprintf("ooooohhh");
            }
        }

	return(hr);
}

/*
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

int FindStartCodesVideo(BYTE *buffer, int size)
{
    int off1=0;
    off1=FindStartCode(buffer+off1,size,0x000001B3);
//    off1=FindStartCode(buffer+off1,size,0x000001E0);
    if (off1>=0)
        return(off1);

    return(-1);
}

int FindStartCodesAudio(BYTE *buffer, int size)
{
    int off1=0;
    off1=FindStartCode(buffer+off1,size,0x000001C0);
    if (off1>=0)
        return(off1);

    return(-1);
}
*/

void __cdecl AVReadThread(void *thread_arg)
{	
    BOOL wait=TRUE;
	int ret=0;
    int off=0;
    unsigned char *bufferVideo=NULL;
    int  bufferlenVideo=VIDEO_BUFFER_SIZE;
    unsigned char *bufferAudio=NULL;
    int  bufferlenAudio=AUDIO_BUFFER_SIZE;
    BOOL firstAudio=FALSE;
    BOOL firstVideo=FALSE;
    DWORD smode = 1;
    

    bufferVideo=(unsigned char *)malloc(bufferlenVideo);
    bufferAudio=(unsigned char *)malloc(bufferlenAudio);

    dprintf("AVReadThread started ...");

    if (!GetWindowsVersion())
        {
        OutputDebugString("ioctlsocket faking enabled\n");
        gFakeisDataAvailable=TRUE;
        }
    else
        {
        OutputDebugString("ioctlsocket faking disabled\n");
        gFakeisDataAvailable=FALSE;
        }

    //!!BS: to be straight foreward I think its fine to drive Win2k and Win9x in one
    //!!BS: (non-blocking) mode ...
    gFakeisDataAvailable=TRUE;

    if (gFakeisDataAvailable)
        {
        if (gSocketVideoPES>0)
            ioctlsocket(gSocketVideoPES, FIONBIO , &smode ) ;     // set nonblocking mode
        if (gSocketAudioPES>0)
            ioctlsocket(gSocketAudioPES, FIONBIO , &smode ) ;     // set nonblocking mode
        }


    for (;;) 
        {
		if (gfTerminateThread) 
			break;

        wait=TRUE;
        ret=0;

        if (gSocketVideoPES>0)
            {
            if (CRemuxer==NULL)
                {
                dprintf("Thread Video error");
                break;
                }

            while (isDataAvailable(gSocketVideoPES, bufferlenVideo))
                {
                ret=recv(gSocketVideoPES, (char *)bufferVideo, bufferlenVideo, 0);
                if (ret>0)
                    {
                    //dprintf("Receive Video: %ld",ret);
                    gTotalVideoDataCount+=ret;
                    CRemuxer->supply_video_data(bufferVideo, ret);
                    wait=FALSE;
                    firstVideo=TRUE;
                    }
                else
                    break;
                }
            }
        else
            firstVideo=TRUE;

        if (gSocketAudioPES>0)
            {
            if (CRemuxer==NULL)
                {
                dprintf("Thread Audio error");
                break;
                }

            while (isDataAvailable(gSocketAudioPES, bufferlenAudio))
                {
                ret=recv(gSocketAudioPES, (char *)bufferAudio, bufferlenAudio, 0);
                if (ret>0)
                    {
                    //dprintf("Receive Audio: %ld",ret);
                    gTotalAudioDataCount+=ret;
                    CRemuxer->supply_audio_data(bufferAudio, ret);
                    wait=FALSE;
                    firstAudio=TRUE;
                    }
                else
                    break;
                }
            }
        else
            firstAudio=TRUE;
        
        
        #if USE_REMUX
        //if (!wait)
        if (firstAudio && firstVideo)
            {
            if ((gSocketVideoPES>0)&&(gSocketAudioPES>0))
                {
                if (gIsPSPinConnected)
	                CRemuxer->write_mpg(NULL);
                }
            else
            if (gSocketVideoPES>0)
                {
	                CRemuxer->write_mpv(NULL);
                }
            else
            if (gSocketAudioPES>0)
                {
	                CRemuxer->write_mpp(NULL);
                }
            }
        #endif
        
        if (wait)
            Sleep(2);
        }

    gfThreadAborted=TRUE;

    free(bufferVideo);
    free(bufferAudio);
}

void DeInitStreaming()
{
    gfTerminateThread=TRUE;
    Sleep(500);
    
    if (gSocketVideoPES>0)
        {
        closesocket(gSocketVideoPES);
        }
    gSocketVideoPES=0;

    if (gSocketAudioPES>0)
        {
        closesocket(gSocketAudioPES);
        }
    gSocketAudioPES=0;

  
    if (CMultiplexBuffer)
        {
        CMultiplexBuffer->Interrupt();
        CMultiplexBuffer->DeInitialize();
        delete CMultiplexBuffer;
        CMultiplexBuffer=NULL;
        }

    if (CRemuxer)
        {
        delete CRemuxer;
        CRemuxer=NULL;
        }
}

HRESULT InitPSStreaming(int vpid, int apid, char *IPAddress, int Port) 
{
    if ((vpid>0)&&(apid>0))
        {
	    gSocketVideoPES = openPS(IPAddress, Port, vpid, apid, VIDEO_BUFFER_SIZE);
	    if (gSocketVideoPES < 0) 
            return(E_FAIL);
        CMultiplexBuffer=NULL;
        }
    else
        return(E_FAIL);

	
    gfTerminateThread=FALSE;
	ghAVReadThread=_beginthread(AVReadThread , 0, NULL);
    SetThreadPriority((HANDLE)ghAVReadThread, THREAD_PRIORITY_ABOVE_NORMAL);
//    SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);


	return(NOERROR);
}

HRESULT InitStreaming(int vpid, int apid, char *IPAddress, int Port) 
{

    if (vpid>0)
        {
	    gSocketVideoPES = openPES(IPAddress, Port, vpid, VIDEO_BUFFER_SIZE);
	    if (gSocketVideoPES < 0) 
            return(E_FAIL);
        }
    else
        {
        gSocketVideoPES=0;
        }

    if (apid>0)
        {
	    gSocketAudioPES = openPES(IPAddress, Port, apid, AUDIO_BUFFER_SIZE);
	    if (gSocketAudioPES < 0) 
            return(E_FAIL);
        }
    else
        {
        gSocketAudioPES=0;
        }


    if ((apid>0)&&(vpid>0))
        {
        CMultiplexBuffer=new CCircularBuffer();
        CMultiplexBuffer->Initialize( (AUDIO_BUFFER_SIZE+VIDEO_BUFFER_SIZE) * BUFFER_COUNT, 1);
        CMultiplexBuffer->Clear();
        }
    else
    if (vpid>0)
        {
        CMultiplexBuffer=new CCircularBuffer();
        CMultiplexBuffer->Initialize( (VIDEO_BUFFER_SIZE) * BUFFER_COUNT, 1);
        CMultiplexBuffer->Clear();
        }
    else
    if (apid>0)
        {
        CMultiplexBuffer=new CCircularBuffer();
        CMultiplexBuffer->Initialize( (AUDIO_BUFFER_SIZE) * BUFFER_COUNT, 1);
        CMultiplexBuffer->Clear();
        }
    else
        {
        CMultiplexBuffer=NULL;
        }

#if USE_REMUX
	CRemuxer= new Remuxer();
	CRemuxer->one_pts_per_gop = FALSE;
	CRemuxer->playtime_offset = 0.0;
    CRemuxer->system_clock_ref_start = CRemuxer->system_clock_ref;
    CRemuxer->total_bytes_written = 0;
    CRemuxer->m_framePTS=0;
    if (vpid>0)
        CRemuxer->perform_resync();		
#endif
	
    gLastAVBitrateRequest=timeGetTime();
    gLastVideoDataCount=0;
    gLastAudioDataCount=0;
    gTotalVideoDataCount=0;
    gTotalAudioDataCount=0;

    gfThreadAborted=FALSE;
    gfTerminateThread=FALSE;
	ghAVReadThread=_beginthread(AVReadThread , 0, NULL);

//    SetThreadPriority((HANDLE)ghAVReadThread, THREAD_PRIORITY_ABOVE_NORMAL);
//    SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);


	return(NOERROR);
}

//
// returns 1 for real OS (Win2k, XP, ...) and 0 for all the old ones
//
DWORD GetWindowsVersion(void)
{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

    GetVersionEx (&osvi);

    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
        return(1);
    else
        return(0);
}
