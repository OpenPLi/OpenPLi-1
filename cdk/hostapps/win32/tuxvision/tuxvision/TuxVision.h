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

#ifndef __TUXVISION_H__
#define __TUXVISION_H__ 

#define ALPHA_VERSION       1
#define ALPHA_STRING        "TuxVision"
#define SPLASHTIME          3000
#define REGISTRY_SUBKEY		"Software\\TuxVision"
#define REVISION            "Rev.0.0.1.0"
typedef enum{StateStopped, StatePreview, StateRecord, StatePlayback, StateUninitialized} RecorderState;
#define AUDIO_PCM       0
#define AUDIO_MPEG1L2   1
#define AUDIO_MPEG1L3   2

#define WM_STREAMNOTIFY	(WM_USER+4000)

extern HINSTANCE	ghInstApp;
extern HWND			ghWndApp;
extern HWND			ghWndVideo;
extern long         glAppTop;
extern long         glAppLeft;
extern long         glAppWidth;
extern long         glAppHeight;
extern TCHAR        gszDestinationFolder[264];
extern TCHAR        gszDestinationFile[264];
extern long         gUseDeInterlacer;
extern long         gIs16By9;
extern long         gRecNoPreview;
extern long         gApplicationPriority;
extern long         gAlwaysOnTop;
extern long         gAutomaticAspectRatio;
extern long         gLastPropertyPage;
extern long         gSetVideoToWindow;
extern long         gCaptureAudioOnly;
extern long         gTranscodeAudio;
extern long         gTranscodeAudioFormat;
extern long         gTranscodeAudioBitRate;
extern long         gTranscodeAudioSampleRate;
extern long         gEnableTCPServer;
extern long         gHTTPPort;
extern long         gSTREAMPort;
                    
typedef struct 
    {    
    __int64 cmd; 
    __int64 onidsid;
    __int64 apid;
    __int64 vpid;
    char    channelname[264];
    }RecordingData;

#define ID_ALWAYSONTOP  (40000)
#define ID_FULLSCREEN   (40001)
#define ID_WINDOW       (40002)
#define ID_NORMAL       (40003)
#define ID_MUTE         (40004)
#define ID_EXIT         (40005)


#define Width(x)    (x.right-x.left)
#define Height(x)   (x.bottom-x.top)
#define pWidth(x)   (x->right-x->left)
#define pHeight(x)  (x->bottom-x->top)


HRESULT SetupParameter(HWND hWnd, RecorderState *state);

HRESULT StartPreview(HWND hWnd, RecorderState *state);
HRESULT StopPreview(HWND hWnd, RecorderState *state);
HRESULT StartRecord(HWND hWnd, RecorderState *state);
HRESULT StopRecord(HWND hWnd, RecorderState *state);
HRESULT StartPlayback(HWND hWnd, RecorderState *state);
HRESULT StopPlayback(HWND hWnd, RecorderState *state);


HRESULT SetInput(long input);
HRESULT SetTVChannel(HWND hwnd, __int64 channel, __int64 apid, __int64 vpid);
HRESULT SetTVStandard(long std);

HRESULT SetFullscreen(HWND hWndParent, HWND hWnd, RECT *restore, BOOL flag);
HRESULT RedrawVideoWindow(HWND hWnd);

HRESULT GetChannelList(long *CurrentChannel,
                       long *NumChannels,
                       HWND hWnd,
                       int  idListBox);

HRESULT LoadSettings();
HRESULT SaveSettings();


#endif