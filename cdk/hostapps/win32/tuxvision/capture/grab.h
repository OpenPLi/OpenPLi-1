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
#define MAX_LIST_ITEM       (16384)
#define MTU_DBOX            ( 1500)

#if 1
#define VIDEO_BUFFER_SIZE   (6*16*1024)
#define AUDIO_BUFFER_SIZE   (1*16*1024)
#define BUFFER_COUNT               32
#else
#define VIDEO_BUFFER_SIZE   (64*MTU_DBOX) 
#define AUDIO_BUFFER_SIZE   ( 8*MTU_DBOX) 
#define BUFFER_COUNT               32
#endif
 
#define USE_REMUX                   1

extern char   *gChannelNameList[MAX_LIST_ITEM];
extern __int64 gChannelIDList[MAX_LIST_ITEM];
extern __int64 gTotalChannelCount;

extern Remuxer         *CRemuxer;

extern CCircularBuffer *CVideoBuffer;
extern CCircularBuffer *CAudioBuffer;
extern CCircularBuffer *CMultiplexBuffer;
extern CCircularBuffer *pHTMLCircularBuffer;

extern volatile __int64         gTotalVideoDataCount;
extern volatile __int64         gTotalAudioDataCount;
extern __int64         gLastVideoDataCount;
extern __int64         gLastAudioDataCount;
extern long            gLastAVBitrateRequest;


extern HRESULT InitSockets(void);
extern void DeInitSockets(void);
extern HRESULT EmptySocket(SOCKET s);
extern HRESULT WaitForSocketData(SOCKET sock, unsigned long *avail, long tim);

extern HRESULT RetrievePIDs(int *vpid, int *apid, const char * name, unsigned short port);
extern HRESULT RetrieveChannelList(const char *name, unsigned short port, char *szName, __int64 *count);
extern HRESULT SetChannel(const char *name, unsigned short port, unsigned long channel);
extern HRESULT GetChannel(const char *name, unsigned short port, unsigned long *channel);
extern HRESULT GetChannelInfo(const char *name, unsigned short port, unsigned long channel, char *info);
extern HRESULT GetEPGInfo(const char *name, unsigned short port, char *eventid, char *info);

extern HRESULT ControlPlaybackOnDBOX(const char *name, unsigned short port, int active);
extern HRESULT ExecuteCommand(char *cmd, char *login, char *passwd, char *name, unsigned short port, int doReset, int isENX);
extern HRESULT RetrieveStreamInfo(int *width, int *height,  int *bitrate, int *is4By3,const char *name, unsigned short port);
extern HRESULT CheckBoxStatus(const char *name, unsigned short port);



extern HRESULT InitStreaming(int vpid, int apid, char *IPAddress, int Port);
extern HRESULT InitPSStreaming(int vpid, int apid, char *IPAddress, int Port);

extern void DeInitStreaming(void);

extern BOOL  gIsVideoConnected;
extern BOOL  gIsAudioConnected;
extern BOOL  gIsPSPinConnected;
extern BOOL  gfThreadAborted;

extern DWORD GetWindowsVersion(void);
