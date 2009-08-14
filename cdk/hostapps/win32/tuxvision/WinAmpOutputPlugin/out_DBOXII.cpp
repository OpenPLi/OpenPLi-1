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

#include <windows.h>
#include <windowsx.h>
#include <winbase.h>
#include <commctrl.h>
#include <shlobj.h>

#include "debug.h"
#include "out_DBOXII.h"
#include "resource.h"
#include "dshow.h"

#define PI_VER "Rev.0.0"

HWND gDialogHandle=NULL;

__int64 g_PlayTime=0;
__int64 g_SampleRate=0;
__int64 g_NumChannels=0;
__int64 g_BytePerSample=0;
__int64 g_TotalByteReceived=0;
__int64 g_pause=0;

BOOL WINAPI DllMain (HINSTANCE hDllInst, DWORD fdwReason, LPVOID lpvReserved)
{
    BOOL bResult = TRUE;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // The DLL is being loaded for the first time by a given process.
            // Perform per-process initialization here. If the initialization
            // is successful, return TRUE; if unsuccessful, return FALSE.
            break;

        case DLL_PROCESS_DETACH:
            // The DLL is being unloaded by a given process.  Do any
            // per-process clean up here, such as undoing what was done in
            // DLL_PROCESS_ATTACH.  The return value is ignored.
            break;

        case DLL_THREAD_ATTACH:
            // A thread is being created in a process that has already loaded
            // this DLL.  Perform any per-thread initialization here.  The
            // return value is ignored.
            break;

        case DLL_THREAD_DETACH:
            // A thread is exiting cleanly in a process that has already
            // loaded this DLL.  Perform any per-thread clean up here.  The
            // return value is ignored.
            break;

        default:
            break;
    }

    return (bResult);
}

Out_Module out2 = 
    {
	OUT_VER,
	"BSE DBOXII Output Plugin" PI_VER,
	65536,
	0, // hMainWindow
	0, // hDLLInstance
	config,
	about,
	init,
	quit,
	open,
	close,
	write,
	canwrite,
	isplaying,
	pause,
	setvolume,
	setpan,
	flush,
	getoutputtime,
    //getoutputtime
	getwrittentime
    };

Out_Module *winampGetOutModule()
{
	return &out2;
}

BOOL CALLBACK DialogWindowProc(
                                HWND hwndDlg,  // handle to dialog box
                                UINT uMsg,     // message
                                WPARAM wParam, // first message parameter
                                LPARAM lParam  // second message parameter
                              )
{

	switch(uMsg)
		{
		case WM_INITDIALOG:
			{

            gDialogHandle=hwndDlg;
			SendMessage( GetDlgItem(hwndDlg,IDC_STOPPLAYBACK), BM_SETCHECK, (unsigned int)g_DBOXStopPlayback, 0 );
            if (g_IsENX)
			    SendMessage( GetDlgItem(hwndDlg,IDC_ENX), BM_SETCHECK, 1, 0 );
            else            
			    SendMessage( GetDlgItem(hwndDlg,IDC_GTX), BM_SETCHECK, 1, 0 );
            SetWindowText(GetDlgItem(hwndDlg,IDC_IPADDRESS), g_DBOXAddress);
            SetWindowText(GetDlgItem(hwndDlg,IDC_LOGIN), g_DBOXLogin);
            SetWindowText(GetDlgItem(hwndDlg,IDC_PASSWORD), g_DBOXPassword);
            }
			return (0);

		case WM_COMMAND:
	        switch(GET_WM_COMMAND_ID (wParam, lParam))
		        {
                case IDOK:
                    {
                    g_DBOXStopPlayback=SendMessage( GetDlgItem(hwndDlg,IDC_STOPPLAYBACK), BM_GETCHECK, 0, 0 );
                    g_IsENX=SendMessage( GetDlgItem(hwndDlg,IDC_ENX), BM_GETCHECK, 0, 0 );
                    GetWindowText(GetDlgItem(hwndDlg,IDC_IPADDRESS), g_DBOXAddress, 264);
                    GetWindowText(GetDlgItem(hwndDlg,IDC_LOGIN), g_DBOXLogin, 264);
                    GetWindowText(GetDlgItem(hwndDlg,IDC_PASSWORD), g_DBOXPassword, 264);
                    }
                    EndDialog(hwndDlg, 0);
                    break;
                }
			break;

		case WM_CLOSE:
            EndDialog(hwndDlg, 0);
			return (0);
		}

    return(FALSE);
}


void config(HWND hwnd)
{
    if (gDialogHandle==NULL)
        {
        getConfiguration();
        DialogBox(out2.hDllInstance, MAKEINTRESOURCE(IDD_DIALOG), out2.hMainWindow, DialogWindowProc);
        setConfiguration();
        gDialogHandle=NULL;
        dprintf("config");
        }
}

void about(HWND hwnd)
{
    dprintf("about");
	MessageBox(hwnd,"BSE DBOXII Output Plugin " PI_VER "\n"
					"Copyright (c) 2002 BSE\n\n"
					"Compiled on " __DATE__ "\n","About",MB_OK);
}

void init()
{
    dprintf("init");
}

void quit()
{
    dprintf("quit");
}

int open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{
    dprintf("open: samplerate:%ld, channels:%ld, bps:%ld",samplerate, numchannels, bitspersamp);
    g_SampleRate=samplerate;
    g_NumChannels=numchannels;
    g_BytePerSample=(bitspersamp+7)/8;
    g_pause=0;
	//return theApp.Open(samplerate, numchannels, bitspersamp, bufferlenms, prebufferms);
    InitGraph((int)g_SampleRate, (int)g_NumChannels, (int)bitspersamp, 384000 /*256000*/);
    return(0); // -1
}

void close()
{
    dprintf("close");
    g_TotalByteReceived=0;
    g_PlayTime=0;
    g_pause=0;
    DeInitGraph();
}

int write(char *buf, int len)
{
    //dprintf("write %ld byte", len);
    g_TotalByteReceived+=(__int64)len;
    InjectData((BYTE *)buf, len);
    //Sleep(24);
	return(0);
}

int canwrite()
{
    if (g_pause)
        return(0);
    //dprintf("can write");
	return(8192);
}

int isplaying()
{
    int state=0;
    dprintf("isplaying");
    CheckIfStillPlaying(&state);
	return(state);
}

int pause(int pause)
{
    __int64 val=g_pause;
    dprintf("pause :%ld", pause);
    g_pause=pause;
	return((int)val);
}

// 0..255
void setvolume(int volume)
{
    if (volume>=0)
        {
        dprintf("volume: %ld", volume);
        }
}

// -128..0..127
void setpan(int pan)
{
    dprintf("pan: %ld", pan);
}

void flush(int t)
{
    dprintf("flush: %ld", t);
    g_TotalByteReceived=0;
}

int getoutputtime()
{
    __int64 val=g_SampleRate*g_NumChannels/**g_BytePerSample*/;
    if (val>0)
        g_PlayTime=(g_TotalByteReceived*1000)/(val);
    else
        g_PlayTime=0;
    //dprintf("getoutputtime: %ld", (int)g_PlayTime);
	return((int)g_PlayTime);
}

int getwrittentime()
{
	return((int)g_PlayTime/*+18000*/);  // due to buffering, box is approx 18 sec. behind 
}
/*
int getwrittentime()
{
    GetDeliveredData(&g_TotalByteTransmitted);
    if (g_TotalByteTransmitted<0)
        g_TotalByteTransmitted=0;
    __int64 wTime=0;
    __int64 val=g_SampleRate*g_NumChannels*g_BytePerSample;
    if (val>0)
        wTime=(g_TotalByteTransmitted*1000)/(val);
    else
        wTime=0;
    dprintf("getwrittentime: %ld", (int)wTime);
	return((int)wTime);
}
*/