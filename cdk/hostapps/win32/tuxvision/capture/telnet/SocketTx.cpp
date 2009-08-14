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

// SocketTx.cpp: implementation of the CSocketTx class.
//
//////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include <windows.h>
#include "conio.h"
#include "SocketTx.h"
#include "..\\debug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern HANDLE stdin1;
extern HANDLE stdout1;
extern HANDLE stderr1;

CSocketTx::CSocketTx(SOCKET hSocket,HANDLE &hThread)
{
}

CSocketTx::~CSocketTx()
{
	m_nExit = 1;
}

DWORD CSocketTx::SendTh(CSocketTx *pSocketTx)
{
  return 0;
}

