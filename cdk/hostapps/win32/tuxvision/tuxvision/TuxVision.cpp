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
#include <windowsx.h>
#include <winbase.h>
#include <commctrl.h>
#include <shlobj.h>
#include <prsht.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vfw.h>
#include <wingdi.h>

#include <malloc.h>
#include <mmsystem.h>

#include <streams.h>
#include <strmif.h>

#include <initguid.h>

#include "guids.h"
#include "Dshow.h"
#include "TuxVision.h"
#include "resource.h"
#include "options.h"
#include "AX_Helper.h"
#include "WinHelper.h"
#include "Registry.h"
#include "..\\capture\\interface.h"
#include "..\\render\\interface.h"
#include "TCPServer.h"
#include "logger.h"
#include "debug.h"


char STREAM_TEST_DATA[]="<?xml version=\"1.0\" encoding=\"iso-8859-1\"?> \
                                            \
<neutrino commandversion=\"1\">               \
    <record command=\"record\">               \
        <channelname>PREMIERE 1</channelname>   \
        <epgtitle>Live Golf: British Open</epgtitle>    \
        <onidsid>8716305</onidsid>  \
        <epgid>571231782548</epgid> \
        <videopid>255</videopid>    \
        <audiopids selected=\"256\">  \
            <audio pid=\"256\" name=\"deutsch\"/>   \
            <audio pid=\"258\" name=\"englisch\"/>  \
        </audiopids>    \
    </record>   \
</neutrino> \
";

// ------------------------------------------------------------------------
// Global Stuff
// ------------------------------------------------------------------------
HINSTANCE	  ghInstApp=NULL;
HWND		  ghWndApp=NULL;
HWND		  ghWndVideo=NULL;
HMENU         ghPopUpMenu=NULL;
long          gMuted=FALSE;
long          glAppTop=0;
long          glAppLeft=0;
long          glAppWidth=0;
long          glAppHeight=0;
RecorderState gState=StateUninitialized;
RECT		  gRestoreRect={0,0,0,0};
long		  gCurrentChannel=0;
long		  gNumChannels=0;
TCHAR         gszDestinationFolder[264]="";
TCHAR         gszDestinationFile[264]="";
long          gUseDeInterlacer=0;
long          gIs16By9=FALSE;
long          gRecNoPreview=FALSE;
__int64       gFilteredVideoBitrate=0;
__int64       gFilteredAudioBitrate=0;
__int64       gDSoundVoume=100;
long          gApplicationPriority=NORMAL_PRIORITY_CLASS;
long          gAlwaysOnTop=FALSE;
long          gAutomaticAspectRatio=TRUE;
long          gLastPropertyPage=0;
long          gSetVideoToWindow=FALSE;
long          gCaptureAudioOnly=FALSE;
long          gTranscodeAudio=FALSE;
long          gTranscodeAudioFormat=AUDIO_MPEG1L2;
long          gTranscodeAudioBitRate=192000;
long          gTranscodeAudioSampleRate=44100;
long          gEnableTCPServer=FALSE;
long          gHTTPPort=8080;
long          gSTREAMPort=4000;

// ------------------------------------------------------------------------
// Basic Defines
// ------------------------------------------------------------------------
#define APPNAME		"TuxVision"  // dependency to CLASS statement in resource !!!
#define MUTEXNAME	"TuxVision_MUTEX"
// ------------------------------------------------------------------------
// Forward Declarations
// ------------------------------------------------------------------------
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProcVideo (HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam);
int		OnWM_Command(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam);
BOOL	DoesInstanceExist(void);
int		UpdateWindowState(HWND hWnd);
int     CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
void    CreateChannelList(HWND hwnd);
HRESULT UpdateChannelInfo(IDBOXIICapture *pIDBOXIICapture, __int64 currentChannel);
void    ComposeCaptureFileName(LPSTR szFileName);
void    LoadParameter(void);
void    SaveParameter(void);

// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------
BOOL  DoesInstanceExist(void)
{
    HANDLE hMutex;
    if ( hMutex = OpenMutex (MUTEX_ALL_ACCESS, FALSE, MUTEXNAME) )
        {
        CloseHandle(hMutex);
        return TRUE;
        }
    else      // if it does not exist, we create the mutex
       hMutex = CreateMutex (NULL, FALSE, MUTEXNAME);
    return FALSE;
}

// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	MSG msg;
	HWND hwnd=NULL;
	WNDCLASSEX wndclassex;
	RECT rect1={0,0,0,0};
	int topV=0,leftV=0,widthV=0,heightV=0;
	int retval=0;
	HRESULT hr=NOERROR;
    HBRUSH hbr=CreateSolidBrush(0x000000);


// ------------------------------------------------------------------------
// !!BS:TESTING only 
/*
    RecordingData   rdata;
    hr=AnalyzeXMLRequest(STREAM_TEST_DATA, &rdata);
    return(0);
*/
// !!BS:TESTING only 
// ------------------------------------------------------------------------
	if (DoesInstanceExist())
		return(-1);
// ------------------------------------------------------------------------
    LogClear();
// ------------------------------------------------------------------------
    // OLE subsystem requires applications to initialize things first!
    CoInitialize(NULL);
// ------------------------------------------------------------------------
    ghInstApp=hInstance;

	wndclassex.style		= /*CS_HREDRAW|CS_VREDRAW|*/CS_DBLCLKS ;
	wndclassex.lpfnWndProc	= WndProc;
	wndclassex.cbClsExtra	= 0;
	wndclassex.cbWndExtra	= DLGWINDOWEXTRA;
	wndclassex.hInstance	= hInstance;
	wndclassex.hIcon		= LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON));
	wndclassex.hIconSm		= LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON+1));
	wndclassex.hCursor		= LoadCursor(hInstance,IDC_ARROW);
	wndclassex.hbrBackground= (HBRUSH)COLOR_WINDOW;
	wndclassex.lpszMenuName	= NULL;
	wndclassex.lpszClassName= APPNAME;
	wndclassex.cbSize		= sizeof(WNDCLASSEX);
	retval=RegisterClassEx(&wndclassex);

	wndclassex.style		= /*CS_HREDRAW|CS_VREDRAW|*/CS_DBLCLKS ;
	wndclassex.lpfnWndProc	= WndProcVideo;
	wndclassex.cbClsExtra	= 0;
	wndclassex.cbWndExtra	= 0;
	wndclassex.hInstance	= hInstance;
	wndclassex.hIcon		= NULL;
	wndclassex.hIconSm		= NULL;
	wndclassex.hCursor		= NULL;
	wndclassex.hbrBackground= (HBRUSH)hbr; 
	wndclassex.lpszMenuName	= NULL;
	wndclassex.lpszClassName= "VideoWindow";
	wndclassex.cbSize		= sizeof(WNDCLASSEX);
	retval=RegisterClassEx(&wndclassex);

// ------------------------------------------------------------------------
    InitCommonControls();
// ------------------------------------------------------------------------
	LoadParameter();
// ------------------------------------------------------------------------
    hwnd=CreateDialog(hInstance,MAKEINTRESOURCE(MAIN),0,(DLGPROC)NULL);
    ghWndApp=hwnd;
    long stime=timeGetTime();
// ------------------------------------------------------------------------
    DisplaySplash();
// ------------------------------------------------------------------------
	hr=OpenInterface(ghWndVideo, ghInstApp);
// ------------------------------------------------------------------------
    long dtime=(SPLASHTIME+stime-timeGetTime());
    if (dtime<0) dtime=0;
    //dprintf("Additional SplashDelay:%d ms",dtime);
    {
    int i;
    for(i=0;i<50;i++)
        {
        if (PeekMessage(&msg, (HWND) NULL, 0, 0, PM_REMOVE))
            {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            }
        Sleep(dtime/50);
        }
    }
    if (ghwndSplash)
        DestroyWindow(ghwndSplash);
// ------------------------------------------------------------------------
	GetWindowRect(GetDlgItem(hwnd,IDC_VIDEO_FRAME), &rect1);
	topV=rect1.top+GetSystemMetrics(SM_CYFRAME)-GetSystemMetrics(SM_CYCAPTION);
	leftV=rect1.left-GetSystemMetrics(SM_CXEDGE);
	widthV=rect1.right-rect1.left - GetSystemMetrics(SM_CXEDGE);
	heightV=rect1.bottom-rect1.top - 2*GetSystemMetrics(SM_CYFRAME);
	ghWndVideo=CreateWindow("VideoWindow",
							NULL,
							WS_CHILD/*|WS_CLIPCHILDREN*/|WS_VISIBLE, 
							leftV,
							topV,
							widthV,
							heightV,
							ghWndApp,
							NULL,
							hInstance,
							NULL);
    
    gRestoreRect.left=leftV;
    gRestoreRect.top=topV;
    gRestoreRect.right=leftV+widthV;
    gRestoreRect.bottom=topV+heightV;

    CenterWindow(ghWndApp);
	ShowWindow(ghWndApp,SW_SHOW);
	ShowWindow(ghWndVideo,SW_SHOW);
    SetWindowPos(ghWndVideo,ghWndApp,leftV,topV,0,0,SWP_NOSIZE);
    {
    RECT rc;
    GetWindowRect(ghWndApp,&rc);
    glAppWidth=Width(rc);
    glAppHeight=Height(rc);
    }

// ------------------------------------------------------------------------
    LogPrintf("Application started");
    LogFlush(hwnd);
// ------------------------------------------------------------------------

	if (SUCCEEDED(hr))
		{
        gState=StateStopped;
		UpdateWindowState(hwnd);
        SetWindowText(GetDlgItem(ghWndApp,IDC_DESTINATION_NAME), gszDestinationFolder);
        SendMessage( GetDlgItem(hwnd,IDC_DEINTERLACE), BM_SETCHECK, gUseDeInterlacer, 0 );
        SendMessage( GetDlgItem(hwnd,IDC_16BY9), BM_SETCHECK, gIs16By9, 0 );
        SendMessage( GetDlgItem(hwnd,IDC_NOPREVIEW), BM_SETCHECK, gRecNoPreview, 0 );

		SendMessage(GetDlgItem(hwnd,IDC_VOLUME), TBM_SETRANGE, TRUE, MAKELONG(0, 100) );
        SendMessage(GetDlgItem(hwnd,IDC_VOLUME), TBM_SETPOS, TRUE, (long)gDSoundVoume);

        SetPriorityClass(GetCurrentProcess(),gApplicationPriority);

        if (gAlwaysOnTop)
            SetWindowPos(ghWndApp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
        else
            SetWindowPos(ghWndApp, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

        SetTimer(ghWndApp,1,1000,NULL);


        CreateChannelList(ghWndApp);

   
        ghPopUpMenu=CreatePopupMenu();
        AppendMenu(ghPopUpMenu,	 MF_STRING, ID_ALWAYSONTOP, "Allways on top");
        AppendMenu(ghPopUpMenu,	 MF_SEPARATOR, 0,         NULL);
        AppendMenu(ghPopUpMenu,	 MF_STRING, ID_FULLSCREEN, "Fullscreen");
        AppendMenu(ghPopUpMenu,	 MF_STRING, ID_WINDOW, "Window");
        AppendMenu(ghPopUpMenu,	 MF_STRING, ID_NORMAL, "Normal");
        AppendMenu(ghPopUpMenu,	 MF_SEPARATOR, 0,         NULL);
        AppendMenu(ghPopUpMenu,	 MF_STRING, ID_MUTE, "Mute");
        AppendMenu(ghPopUpMenu,	 MF_SEPARATOR, 0,         NULL);
        AppendMenu(ghPopUpMenu,	 MF_STRING, ID_EXIT, "Exit");

        if (gEnableTCPServer)
            {
            HTTPInit();
            HTTPRun();
            }

    	while(GetMessage(&msg,NULL,0,0))
			{
			TranslateMessage((LPMSG)&msg);
			DispatchMessage((LPMSG)&msg);
			}
		}
    else
        {
        MessageBox(NULL,"Unable to open capture driver, program will exit !","TuxVision has a severe problem ...",MB_OK|MB_ICONSTOP);
        }
// ------------------------------------------------------------------------
    HTTPStop();
    HTTPDeInit();
// ------------------------------------------------------------------------
	CloseInterface(ghWndVideo);
// ------------------------------------------------------------------------
	SaveParameter();
// ------------------------------------------------------------------------
    // Finished with OLE subsystem
    CoUninitialize();
// ------------------------------------------------------------------------
	return (msg.wParam);
// ------------------------------------------------------------------------
}

// ------------------------------------------------------------------------
// main-window handler
// ------------------------------------------------------------------------
LRESULT CALLBACK WndProc (HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam)
{

	switch(message)
		{
//		case WM_CREATE:
//			return (0);

		case WM_COMMAND:
			OnWM_Command(hwnd,message,wParam,lParam);
			break;
		case WM_KEYDOWN:
			//dprintf("KeyDown: WParam: %ld, LPARAM:%ld",wParam,lParam);
			switch(wParam)
				{
                case 27: //!!BS Escape Key
					SendMessage(ghWndVideo,message,wParam,lParam);
                    return(0);
				case 35:
				case 36:
				case 38:
				case 40:
					SendMessage(GetDlgItem(hwnd,IDC_CHANNEL),message,wParam,lParam);
					return(0);
				}
			break;

        case WM_SYSCOMMAND:
            if (wParam==SC_MAXIMIZE)
                {
                //!!BSTEST
                gSetVideoToWindow=FALSE;    
                //!!BSTEST
                
                SendMessage(ghWndVideo, message, wParam, lParam);
                return(0);
                }
            break;

		case WM_LBUTTONDOWN:
		    if ((!gFullscreen) || (gFullscreen && gSetVideoToWindow) )
				{
				RECT rc;
				int xPos, yPos;
				GetWindowRect (hwnd, &rc);
				xPos = (rc.left+rc.right)/2;
				yPos = rc.top+1;
      			//dprintf("MainWindow LButtonDown");
   				return (DefWindowProc (hwnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(xPos, yPos)));
				}
            else
                {
      			//dprintf("Forwarding LButtonDown");
                SendMessage(ghWndVideo,WM_KEYDOWN,27,0);
                }
		  break;
		case WM_USER:
			//OnWM_User(hwnd,message,wParam,lParam);
			break;

        case WM_STREAMNOTIFY:
            {
            RecordingData   *rdata=(RecordingData*)wParam;
            dprintf("WM_STREAMNOTIFY: CMD:%ld, ONIDSID:%ld, APID:%ld, VPID:%ld", (int)rdata->cmd, (int)rdata->onidsid, (int)rdata->apid, (int)rdata->vpid);
            int i=0;
            if (rdata->cmd==CMD_VCR_RECORD)
                {
				//SendMessage(ghWndApp,WM_COMMAND,IDC_STOP,0);
                //SetFullscreen(ghWndApp, hwnd, &gRestoreRect, FALSE);
                for(;;)
                    {
                    char buffer[264];
				    int channel=SendMessage( GetDlgItem(hwnd,IDC_CHANNEL), CB_GETLBTEXT, i, (LPARAM)buffer );
                    if (channel==CB_ERR)
                        break;
                    if (!lstrcmp(buffer, rdata->channelname))
                        {
    				    SendMessage( GetDlgItem(hwnd,IDC_CHANNEL), CB_SETCURSEL, i, 0 );
                        SetTVChannel(hwnd, rdata->onidsid, rdata->apid, rdata->vpid);
				        gCurrentChannel=i;
						PostMessage(ghWndApp,WM_COMMAND,IDC_RECORD,0);
                        break;
                        }
                    i++;
                    }
                }
            else
            if (rdata->cmd==CMD_VCR_STOP)
                {
				PostMessage(ghWndApp,WM_COMMAND,IDC_STOP,0);
                //SetFullscreen(ghWndApp, hwnd, &gRestoreRect, FALSE);
                }
            }

            break;

        case WM_NCLBUTTONDBLCLK:
            //dprintf("WM_NCLBUTTONDBLCLK: %ld",wParam);
            return(0);

        case WM_ERASEBKGND:
   		    if (gFullscreen)
                return(1);
            break;

        case WM_SIZE:
   		    if (gFullscreen && gSetVideoToWindow)
                {
                MoveVideoWindow();
                return(0);
                }
            break;

        case WM_HSCROLL:
			switch(LOWORD(wParam))
				{
				case TB_THUMBTRACK:
				case TB_THUMBPOSITION:
				case TB_ENDTRACK:
					if ((HWND)lParam==GetDlgItem(hwnd,IDC_VOLUME))
						{
						gDSoundVoume=SendMessage((HWND)lParam, TBM_GETPOS, 0, 0L);
                        SetDSoundVolume(gDSoundVoume);
                        gMuted=FALSE;
						}
					break;
                }
            break;

        case WM_TIMER:
            if (!gFullscreen)
                {
                BOOL aStatus=TRUE;
                BOOL vStatus=TRUE;
                //dprintf("WM_TIMER");

                char szString[264];
                char szString2[264];
                lstrcpy(szString,"");
                if ((wParam==1) && (gState==StateRecord))
                    {
                    __int64 val=0;
                    HRESULT hr=GetCaptureFileSize(&val);
                    if (SUCCEEDED(hr))
                        {
                        val=val/(1024*1024);
                        ltoa((long)val, szString, 10);
                        lstrcat(szString," MByte");
                        }
                    }
                SetDlgItemText(ghWndApp,IDC_CAPFILESIZE, szString);

                lstrcpy(szString,"0");
                if ((wParam==1) && ((gState==StateRecord)||(gState==StatePreview)))
                    {
                    __int64 val=0;
                    __int64 avStatus=0;
                    HRESULT hr=GetResyncCount(&val, &avStatus);
                    if (SUCCEEDED(hr))
                        {
                        ltoa((long)val, szString, 10);
                        if (avStatus&0x80)
                            {
                            if (gCaptureAudioOnly)
                                SetDlgItemText(ghWndApp,IDC_VIDEOBITRATE, "-----");
                            else
                                {
                                SetDlgItemText(ghWndApp,IDC_VIDEOBITRATE, "fail !");
                                dprintf("Video failed");
                                }
                            vStatus=FALSE;
                            }
                        if ((avStatus&0x40)&&(!gCaptureAudioOnly))
                            {
                            SetDlgItemText(ghWndApp,IDC_AUDIOBITRATE, "fail !");
                            aStatus=FALSE;
                            dprintf("Audio failed");
                            }
                        if (avStatus&0x01)
                            {
                            if (gCaptureAudioOnly)
                                lstrcpy(szString,"-----");
                            else
                                {
                                lstrcpy(szString,"search...");
                                dprintf("still synching ....");
                                }
                            }
                        }
                    }
                SetDlgItemText(ghWndApp,IDC_RESYNCCOUNT, szString);

                lstrcpy(szString,"0 kBit/s");
                lstrcpy(szString2,"0 kBit/s");
                if ((wParam==1) && ((gState==StateRecord)||(gState==StatePreview)))
                    {
                    __int64 val =0;
                    __int64 val2=0;
                    HRESULT hr=GetCurrentBitrates(&val, &val2);
                    if (SUCCEEDED(hr))
                        {
                        gFilteredAudioBitrate=(gFilteredAudioBitrate*8+val*2)/10;
                        gFilteredVideoBitrate=(gFilteredVideoBitrate*8+val2*2)/10;

                        val=gFilteredAudioBitrate/(1024);
                        ltoa((long)val, szString, 10);
                        lstrcat(szString," kBit/s");

                        val2=gFilteredVideoBitrate/(1024);
                        ltoa((long)val2, szString2, 10);
                        lstrcat(szString2," kBit/s");
                        }
                    }
                else
                    {
                    gFilteredVideoBitrate=0;
                    gFilteredAudioBitrate=0;
                    }
                if (aStatus)
                    SetDlgItemText(ghWndApp,IDC_AUDIOBITRATE, szString);
                if (vStatus)
                    SetDlgItemText(ghWndApp,IDC_VIDEOBITRATE, szString2);
                }

            if ( (wParam==1) && ((gState==StateRecord)||(gState==StatePreview)) && (gCaptureAudioOnly) )
                {
                AdjustAudioFrequency();
                }

            break;

		case WM_GRAPHNOTIFY:
			IMediaEvent *pMediaEvent;
			if (gpIGraphBuilder!=NULL)
				{
				HRESULT hr;
                BOOL graphStoppedMsg=FALSE;
				hr = gpIGraphBuilder->QueryInterface(IID_IMediaEvent, (void**) &pMediaEvent);
				if (SUCCEEDED(hr)&&(pMediaEvent!=NULL))
					{
					long lEventCode,  lParam1,  lParam2;  
					pMediaEvent->GetEvent(&lEventCode,&lParam1,&lParam2,1000);
					dprintf("Graph signaled event. EventCode:%lx, Param1:%lx, Param2:%lx",lEventCode,lParam1,lParam2);
					if ((lEventCode==EC_COMPLETE)||
						(lEventCode==EC_USERABORT)||
						(lEventCode==EC_ERRORABORT))
						{
						if (gFullscreen)
							{
							//dprintf("Restore from Fullscreen");
							SetFullscreen(ghWndApp, hwnd, &gRestoreRect, FALSE);
							}
						//StopPlayback(ghWndVideo, &gState);
						PostMessage(ghWndApp,WM_COMMAND,IDC_STOP,0);
						}
					pMediaEvent->FreeEventParams(lEventCode,lParam1,lParam2);
					RELEASE(pMediaEvent);
					}
				}
			break;

		case WM_CLOSE:
            LogPrintf("Application stopped");
            LogFlush(hwnd);
            KillTimer(hwnd,1);
			PostQuitMessage(0);
			return (0);
		}
	return (DefWindowProc(hwnd, message, wParam, lParam));
}

// ------------------------------------------------------------------------
// video-window handler
// ------------------------------------------------------------------------
LRESULT CALLBACK WndProcVideo (HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam)
{
	switch(message)
		{
//		case WM_CREATE:
//			return (0);

//		case WM_COMMAND:
//			break;

		case WM_KEYDOWN:
			//dprintf("VideoWindow KeyDown");
            if ((gFullscreen)&&(wParam==27))
                {
                //dprintf("Restore from Fullscreen");
                SetFullscreen(ghWndApp, hwnd, &gRestoreRect, FALSE);
                return(0);
                }
			break;

        case WM_RBUTTONDOWN:
            {
            int ret;
            POINT pt;
            GetCursorPos(&pt);
            // ------------------------------------------------------------------------------------------------
            if (gAlwaysOnTop)
                CheckMenuItem(ghPopUpMenu, ID_ALWAYSONTOP, MF_CHECKED);
            else
                CheckMenuItem(ghPopUpMenu, ID_ALWAYSONTOP, MF_UNCHECKED);
            if (gFullscreen&&!gSetVideoToWindow)
                {
                CheckMenuItem(ghPopUpMenu, ID_FULLSCREEN, MF_CHECKED);
                CheckMenuItem(ghPopUpMenu, ID_WINDOW, MF_UNCHECKED);
                CheckMenuItem(ghPopUpMenu, ID_NORMAL, MF_UNCHECKED);
                }
            else
            if (gFullscreen&&gSetVideoToWindow)
                {
                CheckMenuItem(ghPopUpMenu, ID_FULLSCREEN, MF_UNCHECKED);
                CheckMenuItem(ghPopUpMenu, ID_WINDOW, MF_CHECKED);
                CheckMenuItem(ghPopUpMenu, ID_NORMAL, MF_UNCHECKED);
                }
            else
                {
                CheckMenuItem(ghPopUpMenu, ID_FULLSCREEN, MF_UNCHECKED);
                CheckMenuItem(ghPopUpMenu, ID_WINDOW, MF_UNCHECKED);
                CheckMenuItem(ghPopUpMenu, ID_NORMAL, MF_CHECKED);
                }
            if (gMuted)
                CheckMenuItem(ghPopUpMenu, ID_MUTE, MF_CHECKED);
            else
                CheckMenuItem(ghPopUpMenu, ID_MUTE, MF_UNCHECKED);

            // ------------------------------------------------------------------------------------------------
            ret=TrackPopupMenu(ghPopUpMenu,TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD,pt.x,pt.y,0,hwnd,NULL);
            // ------------------------------------------------------------------------------------------------
            switch (ret)
                {
                case ID_ALWAYSONTOP:
                    if (gAlwaysOnTop)
                        {
                        gAlwaysOnTop=FALSE;
                        SetWindowPos(ghWndApp, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
                        }
                    else
                        {
                        gAlwaysOnTop=TRUE;
                        SetWindowPos(ghWndApp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
                        }
                    break;
                case ID_FULLSCREEN: 
                    SetFullscreen(ghWndApp, hwnd, &gRestoreRect, FALSE);
                    gSetVideoToWindow=FALSE;
                    SetFullscreen(NULL, hwnd, &gRestoreRect, TRUE);
                    break;
                case ID_WINDOW:    
                    SetFullscreen(ghWndApp, hwnd, &gRestoreRect, FALSE);
                    gSetVideoToWindow=TRUE;
                    SetFullscreen(NULL, hwnd, &gRestoreRect, TRUE);
                    break;
                case ID_NORMAL:     
                    SetFullscreen(ghWndApp, hwnd, &gRestoreRect, FALSE);
                    break;
                case ID_MUTE:       
                    if (gMuted)
                        {
                        SetDSoundVolume(gDSoundVoume);
                        gMuted=FALSE;
                        }
                    else
                        {
                        SetDSoundVolume(0);
                        gMuted=TRUE;
                        }
                    break;
                case ID_EXIT:       
                    SendMessage(ghWndApp,WM_COMMAND, IDC_EXIT, 0);
                    break;
                }
            // ------------------------------------------------------------------------------------------------
            }
            return(0);

		case WM_LBUTTONDOWN:
		  if ((!gFullscreen)||gSetVideoToWindow)
            {
  			//dprintf("VideoWindow LButtonDown");
			SendMessage(ghWndApp,message,wParam,lParam);
            }
          else  
            {
            //dprintf("Restore from Fullscreen");
            SetFullscreen(ghWndApp, hwnd, &gRestoreRect, FALSE);
            }
		  break;

        case WM_SYSCOMMAND:
            if (wParam==SC_MAXIMIZE)
                {
                if (!gFullscreen)
                    {
                    //dprintf("Switching to Fullscreen");
                    SetFullscreen(NULL, hwnd, &gRestoreRect, TRUE);
                    }
                return(0);
                }
            else 
            if (wParam==SC_RESTORE)
                {
                if (gFullscreen)
                    {
                    //dprintf("Restore from Fullscreen");
                    SetFullscreen(ghWndApp, hwnd, &gRestoreRect, FALSE);
                    }
                return(0);
                }
            break;

        case WM_LBUTTONDBLCLK:
            //if (gState==StatePlayback)
                {
                if (gFullscreen)
                    {
                    //dprintf("Restore from Fullscreen");
                    SetFullscreen(ghWndApp, hwnd, &gRestoreRect, FALSE);
                    }
                else
                    {
                    gSetVideoToWindow=TRUE;    
                    //dprintf("Switching to Fullscreen");
                    SetFullscreen(NULL, hwnd, &gRestoreRect, TRUE);
                    }
                }
            break;
		case WM_USER:
			break;
		case WM_CLOSE:
			PostQuitMessage(0);
			return (0);
		}
	return (DefWindowProc(hwnd, message, wParam, lParam));
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch(uMsg) 
		{
		case BFFM_INITIALIZED: 
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)gszDestinationFolder);
			break;
		}
	return(0);
}

int OnWM_Command(HWND hwnd, UINT message , WPARAM wParam, LPARAM lParam)
{
	switch(GET_WM_COMMAND_ID (wParam, lParam))
		{
        case IDC_GETCHANNELLIST:
            CreateChannelList(hwnd);
            break;

        case IDC_RESET_NHTTPD:
            if (gpVCap!=NULL)
                {
                IDBOXIICapture *pIDBOXIICapture=NULL;
                HRESULT hr=gpVCap->QueryInterface(IID_IDBOXIICapture, (void **)&pIDBOXIICapture);
                if (SUCCEEDED(hr))
                    pIDBOXIICapture->setParameter(CMD_RESTARTNHTTPD, NULL);
                RELEASE(pIDBOXIICapture);
                CreateChannelList(hwnd);
                }
            break;

        case IDC_DESTINATION:
			{
            TCHAR buffer[_MAX_PATH];
            BOOL ret;
            LPITEMIDLIST pil;
            BROWSEINFO bi;

	        lstrcpy(buffer, gszDestinationFolder);

            bi.hwndOwner=hwnd; 
            bi.pidlRoot=NULL; 
            bi.pszDisplayName=buffer; 
            bi.lpszTitle=""; 
            bi.ulFlags=0;
            bi.lpfn=BrowseCallbackProc; //NULL; 
            bi.lParam=0; 
            bi.iImage=0; 

	        LPMALLOC pMalloc;

            if (SHGetMalloc(&pMalloc) == NOERROR)
                {
                pil=SHBrowseForFolder(&bi);
                if (pil!=NULL)
                    {
                    ret=SHGetPathFromIDList(pil,gszDestinationFolder);
		            pMalloc->Free(pil);
                    }
		        pMalloc->Release();
                }
            SetWindowText(GetDlgItem(hwnd,IDC_DESTINATION_NAME), gszDestinationFolder);
            }
            break;

		case IDC_RECORD:
            {
            __int64 val=0;
            CreateCaptureGraph();
            gState=StateRecord;
            SetDSoundVolume(gDSoundVoume);
            RunGraph(gpIGraphBuilder);
			UpdateWindowState(hwnd);
            LogPrintf("RECORD");
            LogFlush(hwnd);
			}
            break;

		case IDC_STOP:
            StopGraph(gpIGraphBuilder);
            DestroyGraph(gpIGraphBuilder);
            gState=StateStopped;
			UpdateWindowState(hwnd);
            LogPrintf("STOP");
            LogFlush(hwnd);
			break;

		case IDC_PLAY:
            gState=StatePlayback;
			UpdateWindowState(hwnd);
            LogPrintf("PLAY");
            LogFlush(hwnd);
			break;

		case IDC_PREVIEW:
            {
            __int64 val=0;
            CreatePreviewGraph();
            gState=StatePreview;
            SetDSoundVolume(gDSoundVoume);
            RunGraph(gpIGraphBuilder);
            UpdateWindowState(hwnd);
            LogPrintf("PREVIEW");
            LogFlush(hwnd);
			}
            break;

		case IDC_OPTIONS:
            HTTPStop();
            HTTPDeInit();

			CreatePropertySheet(ghWndApp, ghInstApp, gLastPropertyPage);

            if (gEnableTCPServer)
                {
                HTTPInit();
                HTTPRun();
                }

            CreateChannelList(ghWndApp);
			UpdateWindowState(hwnd);
			break;

		case IDC_DEINTERLACE:
            gUseDeInterlacer=SendMessage( GetDlgItem(hwnd,IDC_DEINTERLACE), BM_GETCHECK, 0, 0 );
            SetDeInterlacerState(gUseDeInterlacer);
			break;

		case IDC_16BY9:
            {
            RECT rc;
            gIs16By9=SendMessage( GetDlgItem(hwnd,IDC_16BY9), BM_GETCHECK, 0, 0 );
            GetClientRect(ghWndVideo, &rc);
            ConnectVideoWindow(gpIGraphBuilder, ghWndVideo, &rc, gIs16By9);
            }
			break;

		case IDC_NOPREVIEW:
            gRecNoPreview=SendMessage( GetDlgItem(hwnd,IDC_NOPREVIEW), BM_GETCHECK, 0, 0 );
			break;

		case IDC_EXIT:
            StopGraph(gpIGraphBuilder);
            DestroyGraph(gpIGraphBuilder);
			SendMessage(hwnd,WM_CLOSE,0,0);
			break;
// --------------------------------------------------------------------------
		case IDC_CHANNEL:
			if (GET_WM_COMMAND_CMD (wParam, lParam)==CBN_SELCHANGE)
				{
				unsigned long channel=0;
				int sel=0;
				sel=SendMessage( GetDlgItem(hwnd,IDC_CHANNEL), CB_GETCURSEL, 0, 0 );
				channel=SendMessage( GetDlgItem(hwnd,IDC_CHANNEL), CB_GETITEMDATA, sel, 0 );
				SetTVChannel(hwnd, channel, 0, 0);
				gCurrentChannel=sel;
				}
			break;
// --------------------------------------------------------------------------
		}
	return(0);
}


int UpdateWindowState(HWND hWnd)
{
	switch(gState)
		{
		case StateStopped:
			EnableWindow(GetDlgItem(hWnd,IDC_PREVIEW),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_STOP),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_OPTIONS),TRUE);
            EnableWindow(GetDlgItem(hWnd,IDC_DESTINATION),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_CHANNEL),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_GETCHANNELLIST),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_RECORD),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_NOPREVIEW),TRUE);
//			EnableWindow(GetDlgItem(hWnd,IDC_PLAY),TRUE);

			break;
		case StatePreview:
			EnableWindow(GetDlgItem(hWnd,IDC_PREVIEW),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_STOP),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_OPTIONS),FALSE);
            EnableWindow(GetDlgItem(hWnd,IDC_DESTINATION),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_CHANNEL),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_GETCHANNELLIST),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_RECORD),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_NOPREVIEW), FALSE);
//			EnableWindow(GetDlgItem(hWnd,IDC_PLAY),FALSE);
			break;
		case StateRecord:
			EnableWindow(GetDlgItem(hWnd,IDC_PREVIEW),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_STOP),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_OPTIONS),FALSE);
            EnableWindow(GetDlgItem(hWnd,IDC_DESTINATION),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_CHANNEL),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_GETCHANNELLIST),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_RECORD),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_NOPREVIEW), FALSE);
//			EnableWindow(GetDlgItem(hWnd,IDC_PLAY),FALSE);
			break;
		case StatePlayback:
			EnableWindow(GetDlgItem(hWnd,IDC_PREVIEW),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_STOP),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_OPTIONS),FALSE);
            EnableWindow(GetDlgItem(hWnd,IDC_DESTINATION),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_CHANNEL),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_GETCHANNELLIST),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_RECORD),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_NOPREVIEW), FALSE);
//			EnableWindow(GetDlgItem(hWnd,IDC_PLAY),FALSE);
			break;
		}
	UpdateWindow(hWnd);
	return(0);
}

void CreateChannelList(HWND hwnd)
{
    long index=0;
    HRESULT hr=E_FAIL;
    SendMessage( GetDlgItem(hwnd,IDC_CHANNEL), CB_RESETCONTENT , 0, 0 );
    if (gpVCap!=NULL)
        {
        IDBOXIICapture *pIDBOXIICapture=NULL;
        hr=gpVCap->QueryInterface(IID_IDBOXIICapture, (void **)&pIDBOXIICapture);
        if (SUCCEEDED(hr))
            {
            int i;
            char buf[264];
            __int64 count=-1;
            __int64 currentChannel=0;

            SetDlgItemText(ghWndApp,IDC_CHANNELINFO, "");

            hr=pIDBOXIICapture->getParameter(CMD_GETCHANNELLIST, (__int64*)buf, &count);
            if (SUCCEEDED(hr))
                hr=pIDBOXIICapture->getParameter(CMD_GETCHANNEL, (__int64*)&currentChannel,NULL);
            else
                {
                RELEASE(pIDBOXIICapture);
                return;
                }

            if (gAutomaticAspectRatio)
                {
                __int64 val=0;
                hr=pIDBOXIICapture->getParameter(CMD_GETASPECTRATIO, (__int64*)&val,NULL);
                if (SUCCEEDED(hr))
                    {
                    if (val)
                        gIs16By9=0;
                    else
                        gIs16By9=1;
                    }
                EnableWindow(GetDlgItem(hwnd,IDC_16BY9), FALSE);
                SendMessage( GetDlgItem(hwnd,IDC_16BY9), BM_SETCHECK, gIs16By9, 0 );
                }
            else
                {
                EnableWindow(GetDlgItem(hwnd,IDC_16BY9), TRUE);
                SendMessage( GetDlgItem(hwnd,IDC_16BY9), BM_SETCHECK, gIs16By9, 0 );
                }

            if (SUCCEEDED(hr)&&(count>=0))
                {
                //!!BS moved inside loop to avoid stall on subchannel
                //hr=UpdateChannelInfo(pIDBOXIICapture, currentChannel);

                for(i=0;i<=count;i++)
                    {
                    __int64 cid=i;
                    hr=pIDBOXIICapture->getParameter(CMD_GETCHANNELLIST, (__int64*)buf, &cid);
                    if (cid<0)
                        break;
                    index=SendMessage( GetDlgItem(hwnd,IDC_CHANNEL), CB_ADDSTRING, 0, (LONG)(LPSTR)(buf) );
                    SendMessage( GetDlgItem(hwnd,IDC_CHANNEL), CB_SETITEMDATA, index, (unsigned long)cid );
                    if (cid==currentChannel)
                        {
                        SendMessage( GetDlgItem(hwnd,IDC_CHANNEL), CB_SETCURSEL, index, 0 );
                        hr=UpdateChannelInfo(pIDBOXIICapture, currentChannel);
                        }
                        
                    //dprintf("Update Channel: <%ld> <%s> <%ld>",i,buf,(unsigned long)cid);
                    }
                }
            RELEASE(pIDBOXIICapture); 
            }
        }
}

HRESULT SetTVChannel(HWND hwnd, __int64 channel, __int64 apid, __int64 vpid)
{
    HRESULT hr=NOERROR;
    if (gpVCap!=NULL)
        {
        __int64 val=0;
        IDBOXIICapture *pIDBOXIICapture=NULL;
        hr=gpVCap->QueryInterface(IID_IDBOXIICapture, (void **)&pIDBOXIICapture);

        val|=(apid&0x00000000FFFFFFFF)|((vpid&0x00000000FFFFFFFF)<<32);
        if (SUCCEEDED(hr))
            hr=pIDBOXIICapture->setParameter(CMD_AUDIOVIDEOPID, (__int64)val);

        if (SUCCEEDED(hr))
            hr=pIDBOXIICapture->setParameter(CMD_SETCHANNEL, (__int64)channel);

        if (SUCCEEDED(hr))
            hr=UpdateChannelInfo(pIDBOXIICapture, channel);
        else
            SetDlgItemText(ghWndApp,IDC_CHANNELINFO, "");

        if (gAutomaticAspectRatio)
            {
            __int64 val=0;
            hr=pIDBOXIICapture->getParameter(CMD_GETASPECTRATIO, (__int64*)&val,NULL);
            if (SUCCEEDED(hr))
                {
                if (val)
                    gIs16By9=0;
                else
                    gIs16By9=1;
                }
            EnableWindow(GetDlgItem(hwnd,IDC_16BY9), FALSE);
            SendMessage( GetDlgItem(hwnd,IDC_16BY9), BM_SETCHECK, gIs16By9, 0 );
            }
        else
            {
            EnableWindow(GetDlgItem(hwnd,IDC_16BY9), TRUE);
            SendMessage( GetDlgItem(hwnd,IDC_16BY9), BM_SETCHECK, gIs16By9, 0 );
            }

        RELEASE(pIDBOXIICapture);
        }
    return(hr);
}

HRESULT UpdateChannelInfo(IDBOXIICapture *pIDBOXIICapture, __int64 currentChannel)
{
    char buf[1024]="";
    char szEPGID[264]="";
    char szEPGDate[264]="";
    char szEPGTime[264]="";
    char szEPGLen[264]="";
    char szEPGTitle[264]="";
    char szTmp[1024]="";
    int len=0;
    HRESULT hr=NOERROR;

    if (pIDBOXIICapture==NULL)
        return(E_POINTER);

    SetDlgItemText(ghWndApp,IDC_CHANNELINFO, "");

    hr=pIDBOXIICapture->getParameter(CMD_GETCHANNELINFO, &currentChannel, (__int64*)buf);
    dprintf("ChannelInfo: <%s>",buf);
//    if (lstrlen(buf)==0)
//        {
//        Sleep(2000);
//        hr=pIDBOXIICapture->getParameter(CMD_GETCHANNELINFO, &currentChannel, (__int64*)buf);
//        }
    sscanf(buf,"%s %s %s %s",szEPGID, szEPGDate, szEPGTime, szEPGLen);
    lstrcpy(szTmp,szEPGID);lstrcat(szTmp," ");
    lstrcat(szTmp,szEPGDate);lstrcat(szTmp," ");
    lstrcat(szTmp,szEPGTime);lstrcat(szTmp," ");

    if (strstr(szEPGDate,".")!=NULL)
        {
        lstrcat(szTmp,szEPGLen);
        lstrcat(szTmp," ");
        }
    
    len=strlen(szTmp);
    if (len<lstrlen(buf))
        {
        lstrcpy(szEPGTitle, buf+len);
        }
    else
        lstrcpy(szEPGTitle, "");

//    if (lstrlen(szEPGID)>0)
//        hr=pIDBOXIICapture->getParameter(CMD_GETEPG, (__int64*)szEPGID, (__int64*)buf);

    SetDlgItemText(ghWndApp,IDC_CHANNELINFO, szEPGTitle);
    lstrcpy(gszDestinationFile, szEPGTitle);
 
    return(hr);
}

void LoadParameter(void)
{
    char regval[264];

    dprintf("Loading settings ...");

    lstrcpy(gszDestinationFolder,"C:\\");
    gUseDeInterlacer=0;

    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "DestinationPath", (unsigned char *)regval, sizeof(regval)))
        lstrcpy(gszDestinationFolder,regval);
    
    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "DeInterlace", (unsigned char *)regval, sizeof(regval)))
        gUseDeInterlacer=atoi(regval);
    
    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "16By9", (unsigned char *)regval, sizeof(regval)))
        gIs16By9=atoi(regval);
    
    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "RecNoPreview", (unsigned char *)regval, sizeof(regval)))
        gRecNoPreview=atoi(regval);
    
    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "DSoundVolume", (unsigned char *)regval, sizeof(regval)))
        gDSoundVoume=atoi(regval);
    
    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "ApplicationPriority", (unsigned char *)regval, sizeof(regval)))
        gApplicationPriority=atoi(regval);
    
    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "AlwaysOnTop", (unsigned char *)regval, sizeof(regval)))
        gAlwaysOnTop=atoi(regval);
    
    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "AutomaticAspectRatio", (unsigned char *)regval, sizeof(regval)))
        gAutomaticAspectRatio=atoi(regval);

    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "SetVideoToWindow", (unsigned char *)regval, sizeof(regval)))
        gSetVideoToWindow=atoi(regval);


    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "CaptureAudioOnly", (unsigned char *)regval, sizeof(regval)))
        gCaptureAudioOnly=atoi(regval);

    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "TranscodeAudio", (unsigned char *)regval, sizeof(regval)))
        gTranscodeAudio=atoi(regval);

    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "TranscodeAudioFormat", (unsigned char *)regval, sizeof(regval)))
        gTranscodeAudioFormat=atoi(regval);

    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "TranscodeAudioBitRate", (unsigned char *)regval, sizeof(regval)))
        gTranscodeAudioBitRate=atoi(regval);

    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "TranscodeAudioSampleRate", (unsigned char *)regval, sizeof(regval)))
        gTranscodeAudioSampleRate=atoi(regval);

    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "EnableTCPServer", (unsigned char *)regval, sizeof(regval)))
        gEnableTCPServer=atoi(regval);

    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "HTTPPort", (unsigned char *)regval, sizeof(regval)))
        gHTTPPort=atoi(regval);

    if (GetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "STREAMPort", (unsigned char *)regval, sizeof(regval)))
        gSTREAMPort=atoi(regval);

}

void SaveParameter(void)
{
    char regval[264];

    dprintf("Saving settings ...");
	CreateRegKey(HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "");

	wsprintf((char *)regval,"%s",gszDestinationFolder);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "DestinationPath", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gUseDeInterlacer);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "DeInterlace", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gIs16By9);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "16By9", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gRecNoPreview);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "RecNoPreview", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gDSoundVoume);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "DSoundVolume", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gApplicationPriority);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "ApplicationPriority", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gAlwaysOnTop);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "AlwaysOnTop", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gAutomaticAspectRatio);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "AutomaticAspectRatio", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gSetVideoToWindow);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "SetVideoToWindow", (unsigned char *)regval, lstrlen(regval));


	wsprintf((char *)regval,"%ld",gCaptureAudioOnly);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "CaptureAudioOnly", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gTranscodeAudio);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "TranscodeAudio", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gTranscodeAudioFormat);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "TranscodeAudioFormat", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gTranscodeAudioBitRate);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "TranscodeAudioBitRate", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gTranscodeAudioSampleRate);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "TranscodeAudioSampleRate", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gEnableTCPServer);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "EnableTCPServer", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gHTTPPort);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "HTTPPort", (unsigned char *)regval, lstrlen(regval));

	wsprintf((char *)regval,"%ld",gSTREAMPort);
    SetRegStringValue (HKEY_LOCAL_MACHINE, REGISTRY_SUBKEY, "", "STREAMPort", (unsigned char *)regval, lstrlen(regval));

}




