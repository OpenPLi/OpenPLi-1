//
//  DBOXII Capture/Render Filter
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
#include <windows.h>
#include <process.h>

#include "SocketRx.h"
#include "SocketDx.h"
#include "SocketTx.h"
#include "..\\debug.h"

extern char gpTelnetInputBuffer[4096];

#define CMD_EXIT    "exit\r\n"
#define CMD_PROMPT  "> "


HRESULT WriteTerminal(SOCKET sock, char *waitfor, char *str)
{
    int nRet=0;
    char tBuffer[4096]={0};
    char rBuffer[4096]={0};
    int retry=150;

    if (lstrlen(waitfor)>0)
        {
        while(lstrcmp(rBuffer,waitfor))
            {
            lstrcpy(rBuffer, gpTelnetInputBuffer);
            strrev(rBuffer);
            rBuffer[lstrlen(waitfor)]=0;
            strrev(rBuffer);

            retry--;
            if (retry<=0)
                return(E_FAIL);
            Sleep(100);
            }
        }
    else
        Sleep(250);

    lstrcpy(tBuffer,str);
    lstrcat(tBuffer,"\r\n");
	nRet = send(sock,tBuffer,strlen(tBuffer),0);
    if (nRet>0)
        {
        if (lstrlen(waitfor)==0)
            Sleep(250);
        return(NOERROR);
        }
    return(E_FAIL);
}

HRESULT ExecuteCommand(char *cmd, char *login, char *passwd, char *name, unsigned short port, int doReset, int isENX)
{
    HRESULT hr=NOERROR;
    SOCKET hSocket;
    HANDLE hThread;

	CSocketDx	SocketDx(name,port);
	hSocket = SocketDx.TelnetConnect();
	if ( hSocket == NULL ) 
        { 
        dprintf("Unable To Connect\n");
        return(E_FAIL); 
        }

	CSocketRx	SocketRx(hSocket,hThread); 
	
    SetThreadPriority((HANDLE)hThread, THREAD_PRIORITY_ABOVE_NORMAL);

    hr=WriteTerminal(hSocket, "ogin: ", login);
    if (SUCCEEDED(hr))
        hr=WriteTerminal(hSocket, "sword: ", passwd);

    if (SUCCEEDED(hr))
        hr=WriteTerminal(hSocket, "", "export PS1='> '");

    if (SUCCEEDED(hr))
        hr=WriteTerminal(hSocket, CMD_PROMPT, cmd);

    if (doReset)
        {
        if (SUCCEEDED(hr))
            hr=WriteTerminal(hSocket, CMD_PROMPT, "killall -9 dvrv");
        if (SUCCEEDED(hr))
            hr=WriteTerminal(hSocket, CMD_PROMPT, "killall -9 dvra");
        if (SUCCEEDED(hr))
            hr=WriteTerminal(hSocket, CMD_PROMPT, "ps -ax");
        if (SUCCEEDED(hr))
            hr=WriteTerminal(hSocket, CMD_PROMPT, "/var/tuxbox/fbclear");

        if (SUCCEEDED(hr))
            hr=WriteTerminal(hSocket, CMD_PROMPT, "rmmod -r avia_gt_dvr");
        if (SUCCEEDED(hr))
            hr=WriteTerminal(hSocket, CMD_PROMPT, "find /lib/modules -name avia_gt_dvr.o > BSE");
        if (SUCCEEDED(hr))
            hr=WriteTerminal(hSocket, CMD_PROMPT, "insmod -q `cat BSE`");

#if 1
        if (isENX)
            {
            if (SUCCEEDED(hr))
                hr=WriteTerminal(hSocket, CMD_PROMPT, "rmmod -r enx_dvr");
            #if 1
            if (SUCCEEDED(hr))
                hr=WriteTerminal(hSocket, CMD_PROMPT, "find /lib/modules -name enx_dvr.o > BSE");
            if (SUCCEEDED(hr))
                hr=WriteTerminal(hSocket, CMD_PROMPT, "insmod -q `cat BSE`");
            #else
            if (SUCCEEDED(hr))
                hr=WriteTerminal(hSocket, CMD_PROMPT, "insmod -q /var/tuxbox/enx_dvr.o");
            #endif
            }
        else
            {
            if (SUCCEEDED(hr))
                hr=WriteTerminal(hSocket, CMD_PROMPT, "rmmod -r gtx_dvr");
            #if 1
            if (SUCCEEDED(hr))
                hr=WriteTerminal(hSocket, CMD_PROMPT, "find /lib/modules -name gtx_dvr.o > BSE");
            if (SUCCEEDED(hr))
                hr=WriteTerminal(hSocket, CMD_PROMPT, "insmod -q `cat BSE`");
            #else
            if (SUCCEEDED(hr))
                hr=WriteTerminal(hSocket, CMD_PROMPT, "insmod -q /var/tuxbox/gtx_dvr.o");
            #endif
            }
#endif
        }
    if (SUCCEEDED(hr))
        hr=WriteTerminal(hSocket, CMD_PROMPT, "echo done");
    if (SUCCEEDED(hr))
        hr=WriteTerminal(hSocket, CMD_PROMPT, CMD_EXIT);

    SocketRx.m_nExit=1;
	WaitForSingleObject(hThread,5000);

    closesocket(hSocket);
	return 0;
}



