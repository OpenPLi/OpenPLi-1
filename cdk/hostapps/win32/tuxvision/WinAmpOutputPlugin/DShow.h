//
//  DBOXII WinAmp Plugin
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

#ifndef __DSHOW_H__
#define __DSHOW_H__

#define AUDIO_CHUNK_SIZE    2048

#define RELEASE(x) { if (x) x->Release(); x = NULL; }

extern int gInterruptStreaming;
extern __int64 g_TotalByteTransmitted;

HRESULT InitGraph(int AudioSampleFrequency, int AudioChannels, int AudioBitsPerSample, int AudioBitrate);
HRESULT InjectData(BYTE *data, int size);
HRESULT DeInitGraph(void);

HRESULT GetDeliveredData(__int64 *data);
HRESULT CheckIfStillPlaying(int *state);

HRESULT getConfiguration(void);
HRESULT setConfiguration(void);

extern char g_DBOXAddress[264];
extern char g_DBOXLogin[264];
extern char g_DBOXPassword[264];
extern int  g_IsENX;
extern int  g_DBOXStopPlayback;


#endif