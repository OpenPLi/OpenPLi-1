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
#include "filter.h"

#define VIDEO_BUFFER_SIZE       (32*1024)
#define AUDIO_BUFFER_SIZE       (768*16)  
#define BUFFER_COUNT                 64

extern CDBOXIIRender  *gpFilter;

extern CCircularBuffer *CVideoBuffer;
extern CCircularBuffer *CAudioBuffer;
extern CCircularBuffer *CMultiplexBuffer;

extern HRESULT InitSockets(void);
extern void DeInitSockets(void);

extern HRESULT ControlPlaybackOnDBOX(const char *name, unsigned short port, int active);
extern HRESULT ExecuteCommand(char *cmd, char *login, char *passwd, char *name, unsigned short port, int doReset, int isENX);
extern HRESULT MuteAudio(const char *name, unsigned short port, BOOL flag);

extern HRESULT InitStreaming(char *IPAddress, int aport, int vport);

extern void DeInitStreaming(void);

extern BOOL  gIsVideoConnected;
extern BOOL  gIsAudioConnected;
extern BOOL  gIsMultiplexerConnected;
extern BOOL  gfThreadAborted;

extern BOOL  gEOS_0;
extern BOOL  gEOS_1;
extern BOOL  gEOS_2;

extern __int64 gDeliveredAudioData;
extern __int64 gDeliveredVideoData;

