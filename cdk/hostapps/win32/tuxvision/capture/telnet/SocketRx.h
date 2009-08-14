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


#ifndef __SOCKETRX_H__
#define __SOCKETRX_H__

#include "ProtocolRx.h"
class CProtocolRx;
class CSocketRx  
{
public:
	CSocketRx();
	CSocketRx(SOCKET,HANDLE&);
	virtual ~CSocketRx();
	static DWORD RdTh(CSocketRx *);

	SOCKET m_hSocket;
	HANDLE m_hThread;
	int m_nExit;
	CProtocolRx m_Protocol;
};

#endif