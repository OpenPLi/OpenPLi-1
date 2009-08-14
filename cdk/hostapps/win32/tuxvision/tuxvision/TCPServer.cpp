//
//  TuxVision
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

#include <windows.h>       
#include <windowsx.h>       
#include <process.h>       
#include <stdio.h>         
#include <string.h>        
#include <memory.h>
#include <vfw.h>
#include <process.h>
#include <time.h>
#include <winbase.h>
#include <commctrl.h>
#include <string.h>

#include "wsasimpl.h"
#include "debug.h"
#include "TuxVision.h"
#include "TCPServer.h"

#define HTTP_CONTROL_REQUEST	"/control/?"

int      gOpenSocketCount=0;
HWND     ghWndTCP;

#define  MY_CONNECT_HTTP   (WM_USER+100)
#define  MY_READ_HTTP      (WM_USER+101)
#define  MY_CONNECT_STREAM (WM_USER+102)
#define  MY_READ_STREAM    (WM_USER+103)

int OnInitInstance(void);
int OnWM_Create(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam);
int OnMY_CONNECT_HTTP(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam);
int OnMY_READ_HTTP(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam);
int OnMY_CONNECT_STREAM(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam);
int OnMY_READ_STREAM(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam);

volatile    BOOL gEnable=TRUE;

char		gStartingPoint[264]="local";
char		gStartingPage[264]="index.htm";

SOCKADDR_IN gstSockNameServerHTTP;
SOCKET      gListenSocketHTTP=INVALID_SOCKET;
SOCKADDR_IN gstSockNameServerSTREAM;
SOCKET      gListenSocketSTREAM=INVALID_SOCKET;

#define TRANSFERBUFFERSIZE 512000 
#define MAX_IMAGE_SIZE	    98000

#define TX_PACK_DELAY   2		//1000:modem, 1:network, 2:
#define TX_PACK_TIMEOUT 60000	//40000
#define RX_PACK_TIMEOUT 60000	//40000

RecordingData   gRecordingData;

// -----------

LONG     APIENTRY WndProcTCP(HWND hWnd,UINT message,UINT wParam,LONG lParam);

LRESULT  APIENTRY GetHostName(HWND hDlg,UINT message,UINT wParam,LONG lParam);

void     __cdecl CreateHTTPResponse(void *rs);
void     __cdecl CreateSTREAMResponse(void *rs);
void     __cdecl TransferThread(void *t);

int      GetResponse(char *path, SOCKET sock, int *isVideoRequest);
int		 HeadResponse(char *path, SOCKET sock, BYTE **IData, DWORD *ILen, int *isVideoRequest);
int      PostResponse(char *path, SOCKET sock);
int      CheckCommand(char *InputBuffer, int Level, SOCKET sock, int *keepSocketOpen);	
int      GetContentType(char *src,char *dst);
int      SendHTTPHeaders(SOCKET sock);
int      GetGMTime(char *buf);
int		 SendFileTCP(char *newPath, SOCKET sock, BYTE *ImageData, DWORD ImageLen);
// ---------------------------------------------------------------------

#define MAX_IN_BUFFER MAX_MSG_LENGTH


void __cdecl CreateHTTPResponse(void *rs)
{
    SOCKET rSock;
    int nData;
    int retval;
    int keepSocketOpen=0;
    
    BYTE InBuffer[MAX_MSG_LENGTH];    // packet buffer

	rSock=(SOCKET)rs;

    memset(InBuffer,0,MAX_MSG_LENGTH);

    //dprintf("Enter Receive");
	nData=RecvDataTCP(rSock, (char *)InBuffer,MAX_MSG_LENGTH, -1);
    //dprintf("Exit Receive");

	if (nData==-1)
		  {
		  CancelConnection(rSock,0);
		  dprintf("!!! Invalid Data !!!");
		  gOpenSocketCount=CountConnections();
		  return;
		  }
    do
		{
        retval=CheckCommand((char *)InBuffer,nData,rSock,&keepSocketOpen);	
        if (retval==INVALID_SOCKET)
           {
           keepSocketOpen=0;
		   CancelConnection(rSock,0);
           return;
           }
        if (retval==0)
            break;
        if (keepSocketOpen)
            break;
        }while(TRUE);

    CancelConnection(rSock,1);
    dprintf("HTTP Thread exit");
//	gOpenSocketCount=CountConnections();
    return;
}

void __cdecl CreateSTREAMResponse(void *rs)
{
    SOCKET rSock;
    int nData;
    int keepSocketOpen=0;
    __int64 cmd=0;
    __int64 onidsid=0;
    int apid=0;
    int vpid=0;
    HRESULT hr=NOERROR;

    BYTE InBuffer[MAX_MSG_LENGTH];    // packet buffer

	rSock=(SOCKET)rs;

    memset(InBuffer,0,MAX_MSG_LENGTH);

    //dprintf("Enter Receive");
	nData=RecvDataTCP(rSock, (char *)InBuffer,MAX_MSG_LENGTH, -1);
    //dprintf("Exit Receive");

    dprintf("STREAM received %ld byte.",nData);
    dprintf("--------------------------\r\n",(char *)InBuffer);
    dprintf("%s", InBuffer);
    dprintf("--------------------------\r\n",(char *)InBuffer);

	if (nData==-1)
		  {
		  CancelConnection(rSock,0);
		  dprintf("!!! Invalid Data !!!");
		  gOpenSocketCount=CountConnections();
		  return;
		  }

    hr=AnalyzeXMLRequest((char *)InBuffer, &gRecordingData);
    
    if (SUCCEEDED(hr))
        {
        dprintf("Parsing succeeded: CMD:%ld, ONIDSID:%ld, APID:%ld, VPID:%ld", (int)gRecordingData.cmd, (int)gRecordingData.onidsid, (int)gRecordingData.apid, (int)gRecordingData.vpid);
        PostMessage(ghWndApp, WM_STREAMNOTIFY, (WPARAM)&gRecordingData, 0);
        }

/*
    do
		{
        retval=CheckCommand((char *)InBuffer,nData,rSock,&keepSocketOpen);	
        if (retval==INVALID_SOCKET)
           {
           keepSocketOpen=0;
		   CancelConnection(rSock,0);
           return;
           }
        if (retval==0)
            break;
        if (keepSocketOpen)
            break;
        }while(TRUE);
*/


    CancelConnection(rSock,1);
    dprintf("STREAM Thread exit");
//	gOpenSocketCount=CountConnections();
    return;
}

int CheckCommand(char *InputBuffer, int Level, SOCKET sock, int *keepSocketOpen)
{
    int i;
    int retval=0;
    int marker=-1;

    BYTE tBuffer [MAX_IN_BUFFER+2];
	BYTE sBuffer1[MAX_IN_BUFFER+2];
	BYTE sBuffer2[MAX_IN_BUFFER+2];
	BYTE sBuffer3[MAX_IN_BUFFER+2];

	for (i=0;i<Level;i++)
		{
		if (InputBuffer[i]=='\n')
			{
			marker=i;
			break;
			}
		}
	if (marker>0)
		{
		memset(sBuffer1,0,MAX_IN_BUFFER+2);
		memset(sBuffer2,0,MAX_IN_BUFFER+2);
		memset(sBuffer3,0,MAX_IN_BUFFER+2);

		memcpy(tBuffer,InputBuffer,marker+1);
		tBuffer[marker+1]=0;
		memcpy(InputBuffer,InputBuffer+marker+1,MAX_IN_BUFFER-marker-1);
		Level=Level-marker-1;	

		sscanf((char *)tBuffer,"%s %s %s",sBuffer1,sBuffer2,sBuffer3);

		if (strlen((char *)sBuffer2)>1)
			{
			int l=strlen((char *)sBuffer2);
			strrev((char *)sBuffer2);
			strrev((char *)gStartingPoint);
			strcat((char *)sBuffer2,gStartingPoint);
			strrev((char *)gStartingPoint);
			strrev((char *)sBuffer2);
			}

		if (strnicmp((char *)sBuffer1,"GET",3)==0)
			{
	//		dprintf("Creating GET: response ...");

			if (!lstrcmp((char *)sBuffer2,"/"))
				{
				lstrcpy((char *)sBuffer2,gStartingPoint);
				lstrcat((char *)sBuffer2,"/");
				lstrcat((char *)sBuffer2,gStartingPage);
				}
			retval=GetResponse((char *)sBuffer2,sock,keepSocketOpen);
			}
		else
		if (strnicmp((char *)sBuffer1,"HEAD",4)==0)
			{
	//		dprintf("Creating HEAD: response ...");
			retval=HeadResponse((char *)sBuffer2,sock,NULL,NULL,keepSocketOpen);
			}
		else
		if (strnicmp((char *)sBuffer1,"POST",4)==0)
			{
	//		dprintf("Creating POST: response ...");
			retval=PostResponse((char *)sBuffer2,sock);
			}
		retval=1;
		}

    return(retval);
}


int GetResponse(char *path, SOCKET sock, int *isVideoRequest)
{
int ret;
char szCommandString[264];
char newPath[264];
char gserver_url[264];
BYTE *IData;
DWORD ILen;

    *isVideoRequest=0;

	// default ...
	strcpy(newPath,path);

	strcpy(szCommandString,gStartingPoint);
	strcat(szCommandString,"/jctrl"); // _$$$_XXXXXXXX_
	if (strnicmp(path,szCommandString,strlen(szCommandString))==0)
		{
		int val=0,rval=0,data=0,cmd=0;
		char szCmd[9];
		char szRCmd[24];
		int l=strlen(szCommandString)+9;
		memcpy(szCmd,path+strlen(szCommandString),8);
		szCmd[8]=0;

		sscanf(szCmd,"%X",&val);
		data=-1;
		cmd=(val&0x0FFFFFFF)>>16;

		if ((val&0x10000000L)==0x10000000L)	// READ Request
			{
			//if (HTTP_IF_GetValue(cmd,&rval)==NOERROR)
				{
				data=rval;
				}
			}
		else
			{
			data=(val&0x0000FFFF);
			//if (HTTP_IF_PutValue(cmd,data)==NOERROR)
				{

				}
			}
				
		// ---------------------------------

		strcpy(gserver_url, "HTTP/1.0 200 OK\r\n");
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);

		SendHTTPHeaders(sock);

		sprintf(gserver_url,"Content-Length: %ld\r\n",8);
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);

		strcpy(gserver_url,"Content-Type: ");
		strcat(gserver_url,"application/octet-stream");

		strcat(gserver_url,"\r\n\r\n");
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);

		sprintf(szRCmd,"%8.8X\r\n",rval);
		strcpy(gserver_url,szRCmd);
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);

		return(0);
		}

	if (HeadResponse(newPath, sock, &IData, &ILen, isVideoRequest)!=SOCKET_ERROR)
		{
		ret=SendFileTCP(newPath,sock, IData, ILen);
		return(ret);
		}

	return(SOCKET_ERROR);

}

int HeadResponse(char *path, SOCKET sock, BYTE **IData, DWORD *ILen, int *isVideoRequest)
{
FILE *fp;
unsigned char ct[264];
int len;
int ret;
unsigned char tmp[264];
char gserver_url[264];

	lstrcpy((char *)tmp,path);
	strrev((char *)tmp);
	tmp[strlen(HTTP_CONTROL_REQUEST)]=0;
	strrev((char *)tmp);

    *isVideoRequest=0;

	if (!lstrcmp((char *)tmp,HTTP_CONTROL_REQUEST))
		{
        *isVideoRequest=1;
        return(0);
        }

	fp=fopen(path,"rb");
	if (fp!=NULL)   // file is existing !!!
		{
		fseek(fp,0,SEEK_END);
		len=ftell(fp);
		fclose(fp);
		strcpy(gserver_url, "HTTP/1.0 200 OK\r\n");
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);

		if ((ret=SendHTTPHeaders(sock))==SOCKET_ERROR)
			return(ret);

		sprintf(gserver_url,"Content-Length: %ld\r\n",len);
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);

		//sprintf(gserver_url,"Last-Modified: %s\r\n", get_rfc822_time(req->last_modified));

		GetContentType(path,(char *)ct);
		strcpy(gserver_url, "Content-Type: ");
		strcat(gserver_url,(char *)ct);
        
		strcat(gserver_url,"\r\n\r\n");
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);
		}
	else
		{
		strcpy(gserver_url, "HTTP/1.0 404 Not Found\r\n");
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);

		if ((ret=SendHTTPHeaders(sock))==SOCKET_ERROR)
			return(ret);

		strcpy(gserver_url, "Content-Type: text/html\r\n\r\n");
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);
		strcpy(gserver_url,"<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\n"
						   "<BODY><H1>404 Not Found</H1>\nThe requested URL ");
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);
		strcpy(gserver_url,path);
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);
		strcpy(gserver_url," was not found on this server.\n</BODY></HTML>\n");
		if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
			return(ret);
		dprintf("Opening the file <%s> failed",path);
		return(SOCKET_ERROR);
		}

	return(0);

}

int PostResponse(char *path, SOCKET sock)
{
char gserver_url[264];
int ret;

	strcpy(gserver_url, "HTTP/1.0 204 No Content\r\n\r\n");
	if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
        {
        return(ret);
        }

	return(0);

}


int GetContentType(char *src,char *dst)
{
FILE *fp;
char *ret;
char tstr[264];
char cont[264];
char ext[264];
char tmp[264];
char* ptrTemp;

	// set as default ...
	strcpy(dst,"text/plain");

	strcpy(tmp,src);
	strrev(tmp);
	ptrTemp = strchr(tmp,'.');    

	if(ptrTemp)
		{
		int l=(int)ptrTemp-(int)tmp;
		strncpy(ext,tmp,l);
		ext[l]=0;
		strrev(ext);
		strcpy(tmp,ext);
		dprintf("Extension <%s> found.",tmp);    
		fp=fopen("mimetypes.ini","rt");
		if (fp==NULL)
			return(FALSE);

			do
				{
				ret=fgets(tstr,264,fp);
				sscanf(tstr,"%s %s",cont,ext);
				if (!stricmp(ext,tmp))
					{
					strcpy(dst,cont);
					dprintf("ContentType <%s>",dst);    
					fclose(fp);
					return(TRUE);
					}
				}while(ret!=NULL);
		}

	fclose(fp);
	return(TRUE);
}


int SendHTTPHeaders(SOCKET sock)
{
int ret;
unsigned char tbuf[264];
char gserver_url[264];

    strcpy(gserver_url, "Server: BS HTTPSink Rev.1.0\r\n");
	if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
		return(ret);

	strcpy(gserver_url, "MIME-Version: 1.0\r\n");
	if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
		return(ret);

	GetGMTime((char *)tbuf);
	sprintf((char *)gserver_url, "Date: %s\r\n",tbuf);
	if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
		return(ret);

	//strcpy(gserver_url, "Connection: Keep-Alive\r\n");
	strcpy(gserver_url, "Connection: close\r\n");
	if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
		return(ret);

	strcpy(gserver_url, "Keep-Alive: timeout=5, max=50\r\n");
	if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
		return(ret);

	strcpy(gserver_url, "Pragma:  no-cache\r\n");
	if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
		return(ret);

	strcpy(gserver_url, "Cache-Control:  no-cache\r\n");
	if ((ret=SendDataTCP(sock,gserver_url,strlen(gserver_url), TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
		return(ret);

	return(0); 

}



int GetGMTime(char *buf)
{
    struct tm *newtime;
    time_t aclock;

	time( &aclock );              
	newtime = gmtime( &aclock );  
	sprintf(buf,"%s", asctime( newtime ) );
	buf[strlen(buf)-1]=0;
	strcat(buf," GMT");
	return(TRUE);
}

int SendFileTCP(char *newPath, SOCKET sock, BYTE *ImageData, DWORD ImageLen)
{
FILE *fp;
int size;
int ret;
unsigned char tmp[264];

	lstrcpy((char *)tmp,newPath);
	strrev((char *)tmp);
	tmp[strlen(HTTP_CONTROL_REQUEST)]=0;
	strrev((char *)tmp);

	if (!lstrcmp((char *)tmp,HTTP_CONTROL_REQUEST))
		{
		return(0);
		}

	fp=fopen(newPath,"rb");
	if (fp!=NULL)
		{
		BYTE *gTransferBuffer=(BYTE *)malloc(TRANSFERBUFFERSIZE);
        if (NULL==gTransferBuffer)
            return(SOCKET_ERROR);
		do
			{
			size=fread(gTransferBuffer,1,TRANSFERBUFFERSIZE,fp);
			if ((ret=SendDataTCP(sock,(char *)gTransferBuffer,size, TX_PACK_DELAY, TX_PACK_TIMEOUT))==SOCKET_ERROR)
				{
				fclose(fp);
                free(gTransferBuffer);
				return(ret);
				}
			}while(size==TRANSFERBUFFERSIZE);
		fclose(fp);
        free(gTransferBuffer);
		return(0);
		}

	return(SOCKET_ERROR);
}


int OnWM_Create(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam)
{
	OpenTCP(hwnd);
	CancelAllConnections();
	gListenSocketHTTP=InitLstnSockTCP(gHTTPPort,&gstSockNameServerHTTP,hwnd,MY_CONNECT_HTTP);
	gListenSocketSTREAM=InitLstnSockTCP(gSTREAMPort,&gstSockNameServerSTREAM,hwnd,MY_CONNECT_STREAM);
	return(0);
}

int OnMY_CONNECT_HTTP(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam)
{
	if (!gEnable)
		return(0);

	if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
		NewConnectionTCP(gListenSocketHTTP,&gstSockNameServerHTTP,hwnd,MY_READ_HTTP,(SOCKET)wParam);
		gOpenSocketCount=CountConnections();

		}
	if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
	    CancelConnection((SOCKET)wParam,0 /*1*/ /*0*/);
	return(0);
}

int OnMY_CONNECT_STREAM(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam)
{
	if (!gEnable)
		return(0);

	if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
		NewConnectionTCP(gListenSocketSTREAM,&gstSockNameServerSTREAM,hwnd,MY_READ_STREAM,(SOCKET)wParam);
		gOpenSocketCount=CountConnections();

		}
	if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
	    CancelConnection((SOCKET)wParam,0 /*1*/ /*0*/);
	return(0);
}

int OnMY_READ_HTTP(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam)
{
	if (!gEnable)
		return(0);

	if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
		unsigned long  dThread;
		SOCKET rSock=(SOCKET)wParam;

		if (FindSocket(rSock)<0)
			{
			dprintf("unknown socket ???\n");
			return(0);
			}

		dThread=_beginthread(CreateHTTPResponse,1000000,(void *)rSock);
		
        SetThreadPriority((HANDLE)dThread, THREAD_PRIORITY_ABOVE_NORMAL);
		//SetThreadPriority((HANDLE)dThread, THREAD_PRIORITY_BELOW_NORMAL);
		return(0);
		}

	if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
        int count=10;
        int ret;
        unsigned long avail;

        while(count-- > 0)
            {
            ret=ioctlsocket((SOCKET)wParam, FIONREAD, &avail);
            if (ret==0)
                if (avail>0)
                    {
                    Sleep(100);
                    }
            }

		dprintf("Close Request from Client\n");
		DeleteConnection((SOCKET)wParam);
		closesocket((SOCKET)wParam);	
		gOpenSocketCount=CountConnections();
		Sleep(100);
		return(0);
		}

	return(0);

}

int OnMY_READ_STREAM(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam)
{
	if (!gEnable)
		return(0);

	if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
		unsigned long  dThread;
		SOCKET rSock=(SOCKET)wParam;

		if (FindSocket(rSock)<0)
			{
			dprintf("unknown socket ???\n");
			return(0);
			}

#if 1
		dThread=_beginthread(CreateSTREAMResponse,1000000,(void *)rSock);
#else
        {
        int nData=0;
        BYTE InBuffer[MAX_MSG_LENGTH];    // packet buffer
        memset(InBuffer,0,MAX_MSG_LENGTH);
        //dprintf("Enter Receive");
        nData=RecvDataTCP(rSock, (char *)InBuffer,MAX_MSG_LENGTH, -1);
        //dprintf("Exit Receive");

        dprintf("STREAM received %ld byte.",nData);
        dprintf("--------------------------\r\n%s",(char *)InBuffer);
        }
#endif
		SetThreadPriority((HANDLE)dThread, THREAD_PRIORITY_ABOVE_NORMAL);
		//SetThreadPriority((HANDLE)dThread, THREAD_PRIORITY_BELOW_NORMAL);
		return(0);
		}

	if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
        int count=10;
        int ret;
        unsigned long avail;

        while(count-- > 0)
            {
            ret=ioctlsocket((SOCKET)wParam, FIONREAD, &avail);
            if (ret==0)
                if (avail>0)
                    {
                    Sleep(100);
                    }
            }

		dprintf("Close Request from Client\n");
		DeleteConnection((SOCKET)wParam);
		closesocket((SOCKET)wParam);	
		gOpenSocketCount=CountConnections();
		Sleep(100);
		return(0);
		}

	return(0);
}
HRESULT HTTPRun(void)
{
	CancelAllConnections();
    return(NOERROR);
}

HRESULT HTTPStop(void)
{
	CancelAllConnections();
    return(NOERROR);
}


HRESULT HTTPDeInit(void)
{
    gEnable=FALSE;
	CancelAllConnections();
	closesocket(gListenSocketHTTP);
	closesocket(gListenSocketSTREAM);
	CloseTCP();
    Sleep(500);
    DestroyWindow(ghWndTCP);
    return(NOERROR);
}


HRESULT HTTPInit(void)
{
	int result;
	WNDCLASS wndClass;
    gEnable=TRUE;

    wndClass.style          = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc    = WndProcTCP;
    wndClass.cbClsExtra     = 0;
    wndClass.cbWndExtra     = 0;
    wndClass.hInstance      = (HINSTANCE)0;
    wndClass.hIcon          = NULL;
    wndClass.hCursor        = NULL;
    wndClass.hbrBackground  = (HBRUSH)GetStockObject( BLACK_BRUSH );
    wndClass.lpszMenuName   = NULL;
    wndClass.lpszClassName  = "WndProcTCP";


	//we ignore any errors if the class is already registered !!!
    result=RegisterClass( &wndClass );

    ghWndTCP=CreateWindow(  "WndProcTCP",		  // Our class name
                            "WndProcTCP",         // Window title
                            WS_OVERLAPPED,        // It's styles
                            0,		              // No x position
                            0,		              // And no y either
                            0,0,                  // Initial sizes
                            NULL,                 // parent window ?
                            NULL,                 // And no menu
                            (HINSTANCE)0,         // App instance
                            (LPVOID) NULL);       // Creation data

    SetTimer(ghWndTCP,0,1000,NULL);

	if(ghWndTCP!=NULL)
        return(NOERROR);
    else
        return(E_FAIL);
}
// ------------------------------------------------------------------------
// main-window handler
// ------------------------------------------------------------------------
LRESULT CALLBACK WndProcTCP (HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam)
{
	switch(message)
		{
		case WM_CREATE:
            dprintf("MY_CREATE");
			OnWM_Create(hwnd,message,wParam,lParam);
			return (0);

		case MY_CONNECT_HTTP:
            dprintf("MY_CONNECT_HTTP");
			OnMY_CONNECT_HTTP(hwnd,message,wParam,lParam);
			break;

		case MY_READ_HTTP:
            dprintf("MY_READ_HTTP");
			OnMY_READ_HTTP(hwnd,message,wParam,lParam);
			break;

		case MY_CONNECT_STREAM:
            dprintf("MY_CONNECT_STREAM");
			OnMY_CONNECT_STREAM(hwnd,message,wParam,lParam);
			break;

		case MY_READ_STREAM:
            dprintf("MY_READ_STREAM");
			OnMY_READ_STREAM(hwnd,message,wParam,lParam);
			break;

        case WM_TIMER:
            //dprintf("OpenSockets:%ld",gOpenSocketCount);
            break;
		}
	return (DefWindowProc(hwnd, message, wParam, lParam));
}


char* ParseForString(char *szStr, char *szSearch, int ptrToEnd)
{
    char *p=NULL;
    p=strstr(szStr, szSearch);
    if (p==NULL)
        return(NULL);
    if (!ptrToEnd)
        return(p);
    else
        return(p+strlen(szSearch));
    
}

char* ExtractQuotedString(char *szStr, char *szResult, int ptrToEnd)
{
    int i=0;
    int len=lstrlen(szStr);
    lstrcpy(szResult,"");
    for(i=0;i<len;i++)
        {
        if (szStr[i]=='\"')
            {
            char *pe=NULL;
            char *ps=szStr+i+1;      
            pe=ParseForString(ps, "\"", 1);
            if (pe==NULL)
                return(NULL);

            memcpy(szResult,ps, pe-ps-1);
            szResult[pe-ps-1]=0;                       
            if (ptrToEnd)
                return(pe);
            else
                return(ps);
            }
        }
    return(NULL);
}

HRESULT AnalyzeXMLRequest(char *szXML, RecordingData   *rdata)
//__int64 *iCMD, __int64 *iONIDSID, int *apid, int *vpid)
{
    char *p1=NULL;
    char *p2=NULL;
    char *p3=NULL;
    char szcommand[264]="";
    char szonidsid[264]="";
    char szapid[264]="";
    char szvpid[264]="";
    char szchannelname[264]="";
    HRESULT hr=E_FAIL;

    if ( (szXML==NULL) || (rdata==NULL) )
        return(E_POINTER);

    rdata->apid=0;
    rdata->vpid=0;
    rdata->cmd=CMD_VCR_UNKNOWN;
    rdata->onidsid=0;
    lstrcpy(rdata->channelname,"");

    p1=ParseForString(szXML,"<record ", 1);
    if (p1!=NULL)
        {
        p2=ParseForString(p1,"command=", 1);
        p1=NULL;
        if (p2!=NULL)
            p3=ExtractQuotedString(p2, szcommand, 1);
        if (p3!=NULL)
            p1=ParseForString(p3,">", 1);
        }

    if (p1!=NULL)
        {
        p2=ParseForString(p1,"<channelname>", 1);
        p3=p2;
        p1=NULL;
        if (p2!=NULL)
            {
            p1=ParseForString(p2,"</channelname>", 0);
            if (p1!=NULL)
                {
                memcpy(szchannelname,p3,p1-p3);
                szchannelname[p1-p3]=0;
                hr=NOERROR;
                }
            }
        }



    if (p1!=NULL)
        {
        p2=ParseForString(p1,"<onidsid>", 1);
        p3=p2;
        p1=NULL;
        if (p2!=NULL)
            {
            p1=ParseForString(p2,"</onidsid>", 0);
            if (p1!=NULL)
                {
                memcpy(szonidsid,p3,p1-p3);
                szonidsid[p1-p3]=0;
                hr=NOERROR;
                }
            }
        }
    
    p2=ParseForString(szXML,"<videopid>", 1);
    if (p2!=NULL)
        {
        p3=p2;
        p1=NULL;
        p1=ParseForString(p2,"</videopid>", 0);
        if (p1!=NULL)
            {
            memcpy(szvpid,p3,p1-p3);
            szvpid[p1-p3]=0;
            hr=NOERROR;
            }
        }

    p2=ParseForString(szXML,"<audiopids selected=", 1);
    if (p2!=NULL)
        {
        p3=ExtractQuotedString(p2, szapid, 1);
        if (p3!=NULL)
            p1=ParseForString(p3,">", 1);
        }

    if (FAILED(hr))
        return(hr);

    lstrcpy(rdata->channelname, szchannelname);

    if (lstrlen(szvpid)>0)
        rdata->vpid=_atoi64(szvpid);

    if (lstrlen(szapid)>0)
        rdata->apid=_atoi64(szapid);

    if (!lstrcmp(szcommand,"record"))
        rdata->cmd=CMD_VCR_RECORD;
    if (!lstrcmp(szcommand,"stop"))
        rdata->cmd=CMD_VCR_STOP;
    if (!lstrcmp(szcommand,"pause"))
        rdata->cmd=CMD_VCR_PAUSE;
    if (!lstrcmp(szcommand,"resume"))
        rdata->cmd=CMD_VCR_RESUME;
    if (!lstrcmp(szcommand,"available"))
        rdata->cmd=CMD_VCR_AVAILABLE;

    rdata->onidsid=atol(szonidsid);

    return(hr);
}


