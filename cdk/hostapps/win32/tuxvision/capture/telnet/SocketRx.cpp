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


// SocketRx.cpp: implementation of the CSocketRx class.
//
//////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include <windows.h>
#include "ProtocolRx.h"
#include "SocketRx.h"
#include "..\\debug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


char gpTelnetInputBuffer[4096];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSocketRx::CSocketRx()
{

}

CSocketRx::CSocketRx(SOCKET hSocket,HANDLE &hThread)
{
DWORD dwRet;

	m_nExit = 0;
	m_hThread = NULL;
	m_hSocket = hSocket;

    ZeroMemory(gpTelnetInputBuffer,sizeof(gpTelnetInputBuffer));

	m_hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) RdTh,(LPVOID)this,0,&dwRet);
	if ( m_hThread == NULL ) return;
	hThread = m_hThread;
}

CSocketRx::~CSocketRx()
{
	m_nExit = 1;
}

DWORD CSocketRx::RdTh(CSocketRx *pSocketRx)
{
char* scan;
int nRet;
char m_pInputBuffer[4096];

  dprintf("Rx Thread started");
  while(1)
  {
	if ( pSocketRx->m_nExit == 1 ) 
        { 
        break; 
        }
    ZeroMemory(m_pInputBuffer,sizeof(m_pInputBuffer));
    nRet = recv(pSocketRx->m_hSocket,m_pInputBuffer,sizeof(m_pInputBuffer),0);
	if ( nRet == SOCKET_ERROR ) 
        { 
        pSocketRx->m_nExit = 1; 
        continue;
        }
	if ( nRet == 0) 
        Sleep(10);
	else
        {
        lstrcpy(gpTelnetInputBuffer,m_pInputBuffer);
        scan = m_pInputBuffer;
        #ifdef DEBUG
            OutputDebugString(gpTelnetInputBuffer);
        #endif
	    while(nRet--)
    	    {
		    pSocketRx->m_Protocol.TelnetProtcol(pSocketRx->m_hSocket,*scan++);
	        }
        }
  }

  dprintf("Rx Thread terminated");
  return 0;
}
