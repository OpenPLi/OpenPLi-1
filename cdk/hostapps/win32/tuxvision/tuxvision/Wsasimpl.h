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
#include <winsock.h>

#ifdef __cplusplus
extern "C" {    // Assume C declarations for C++
#endif          // __cplusplus

// TCP functions
extern __declspec (dllexport) int    __cdecl DisconnectTCP(SOCKET *ReadWriteSocket, SOCKET *ListenSocket);
extern __declspec (dllexport) SOCKET __cdecl ConnectTCP(LPSTR host, LPSTR service);
extern __declspec (dllexport) int	 __cdecl SendDataTCP(SOCKET, LPSTR data, int size, int sdelay, int timeout);
extern __declspec (dllexport) int	 __cdecl RecvDataTCP(SOCKET, LPSTR data, int size, int timeout);
extern __declspec (dllexport) int	 __cdecl CloseTCP(void);
extern __declspec (dllexport) int	 __cdecl OpenTCP(HWND hwnd);
extern __declspec (dllexport) SOCKET __cdecl InitLstnSockTCP(int iLstnPort, PSOCKADDR_IN pstSockName, HWND hWnd, u_int nAsyncMsg);
extern __declspec (dllexport) SOCKET __cdecl AcceptConnTCP(SOCKET hLstnSock, PSOCKADDR_IN pstName, HWND hWnd, u_int nAsyncMsg);
extern __declspec (dllexport) int    __cdecl SendDataBulkTCP(SOCKET sock,LPSTR databuffer,int buffersize,int datatype, int sdelay, int timeout);
extern __declspec (dllexport) int    __cdecl RecvDataBulkTCP(LPSTR gBufferData,int numBytes,LPSTR gOutBufferData,int outSize);
extern __declspec (dllexport) int    __cdecl GetBufTCP (SOCKET hSock, int nBigBufSize, int nOptval);
extern __declspec (dllexport) void	 __cdecl WaitmsTCP(DWORD dwDelay);
extern __declspec (dllexport) BOOL   __cdecl IsMutexSetTCP(void);
extern __declspec (dllexport) BOOL   __cdecl SetMutexTCP(void);
extern __declspec (dllexport) BOOL   __cdecl ResetMutexTCP(void);
extern __declspec (dllexport) void   __cdecl SetSockOptionsTCP(SOCKET sock);
extern __declspec (dllexport) int    __cdecl NewConnectionTCP(SOCKET hLstnSock, PSOCKADDR_IN pstName,HWND hWnd, u_int nAsyncMsg, SOCKET newSock);
extern __declspec (dllexport) int	 __cdecl AddConnection(SOCKET rwsock);

// UDP functions
// ...
extern __declspec (dllexport) int    __cdecl OpenUDP(HWND hwnd);
extern __declspec (dllexport) int    __cdecl CloseUDP(HWND hwnd);
extern __declspec (dllexport) SOCKET __cdecl ConnectUDP(LPSTR szDestination, LPSTR szService);
extern __declspec (dllexport) int    __cdecl SendDataUDP(SOCKET hSock, LPSTR lpOutBuf, int cbTotalToSend, int sdelay, int nTimeout);

// ICMP functions
// ...
extern __declspec (dllexport) SOCKET __cdecl icmp_open(void);
extern __declspec (dllexport) unsigned short __cdecl cksum (unsigned short FAR*, int);
extern __declspec (dllexport) int    __cdecl icmp_close(SOCKET);
extern __declspec (dllexport) int    __cdecl set_ttl (SOCKET, int);
extern __declspec (dllexport) int    __cdecl icmp_sendto (SOCKET, HWND, LPSOCKADDR_IN, int, int, int);
extern __declspec (dllexport) unsigned long __cdecl icmp_recvfrom(SOCKET,LPINT,LPINT,LPSOCKADDR_IN);


// MULTICAST functions
// ...

// GENERIC functions
extern __declspec (dllexport) unsigned long  __cdecl GetAddr (LPSTR szHost); 
extern __declspec (dllexport) unsigned short __cdecl GetPort (LPSTR szService);

extern __declspec (dllexport) int    __cdecl CancelConnection(SOCKET rwsock, int howlong);
extern __declspec (dllexport) int    __cdecl CancelAllConnections(void);
extern __declspec (dllexport) int    __cdecl FindFreeConnection(void);
extern __declspec (dllexport) int    __cdecl FindSocket(SOCKET sock);
extern __declspec (dllexport) int    __cdecl DeleteConnection(SOCKET rwsock);
extern __declspec (dllexport) int    __cdecl CountConnections(void);

// Useless functions ..
extern __declspec (dllexport) void   __cdecl DisplayConnectionData(HWND hWnd);

#define MAX_CLIENTS              128       // max. number of open sockets ...


#define MAX_MSG_LENGTH           1460	   // this is exactly MTU !
#define MAX_MSG_DATA		     (MAX_MSG_LENGTH-6*sizeof(unsigned long))

#define WSLIB_READ_MSG		     0x00000000
#define WSLIB_READ_FAILED	     0x00000001
#define WSLIB_CONNECTION_LOST    0x00000002
#define WSLIB_ACCEPT_MSG	     0x00000003
#define WSLIB_ACCEPT_FAILED      0x00000004
#define WSLIB_ACCEPT_OK_MSG      0x00000005
#define WSLIB_REQUEST_MSG        0x00000006

#define WSLIB_REQUEST_TRANSMIT   0x00000100
#define WSLIB_REQUEST_RETRANSMIT 0x00000200
#define WSLIB_REQUEST_WAIT		 0x00000300
#define WSLIB_REQUEST_ABORT		 0x00000400
#define WSLIB_REQUEST_DISCONNECT 0x00000500

struct WSLibMsg
			{
			unsigned long ServerIP;
			unsigned long ClientIP;
			unsigned long MaxBlock;		  
			unsigned long BlockCount;         // 0xFFFFFFFF for last Block, 0xFFFFFFF0 for one Block
			unsigned long DataType;		      // 
			unsigned long NumOfDataBytes;	  // total number for transmitted bytes inside a sequence
			unsigned char Data[MAX_MSG_DATA]; // 0x00,0x00,0x00,0x00 for REQUEST-Message
			};

#ifdef __cplusplus
	}    // Assume C declarations for C++
#endif          // __cplusplus
