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


// SocketDx.cpp: implementation of the CSocketDx class.
//
//////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include <windows.h>
#include "SocketDx.h"
#include "..\\debug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSocketDx::CSocketDx(char *strIP,int nPort)
{
unsigned long ip;

  if((*strIP <= '9') && (*strIP >= '0'))
  {
     if((ip = inet_addr(strIP)) == INADDR_NONE)
       {
       dprintf("invalid host ip given");
       }
  }
  else
  {
    hostent* ent = gethostbyname(strIP);
    if(!ent) 
        {
        dprintf("\nError\n");
        }
    ip = *(unsigned long*)(ent->h_addr);
  }

	m_sockaddr_in.sin_family = AF_INET;
	m_sockaddr_in.sin_port = htons(nPort);
	m_sockaddr_in.sin_addr = *(in_addr*)&ip;
}
CSocketDx::~CSocketDx()
{

}
int CSocketDx::Create()
{
  m_hSocket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
  if ( m_hSocket == INVALID_SOCKET) return -1;
  return 0;
}
int CSocketDx::Connect()
{
int nRet;

  nRet = connect(m_hSocket,(sockaddr*)&m_sockaddr_in,sizeof(sockaddr));
  if ( nRet == SOCKET_ERROR ) return -1;
  return 0;
}
SOCKET CSocketDx::TelnetConnect()
{
int nRet;
	
	nRet = Create();
	if ( nRet < 0 ) return NULL;

	nRet = Connect();
	if ( nRet < 0 ) return NULL;

	return m_hSocket;
}
