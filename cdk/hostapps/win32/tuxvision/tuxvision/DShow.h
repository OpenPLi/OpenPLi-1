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

#ifndef __DSHOW_H__
#define __DSHOW_H__

#define RELEASE(x) { if (x) x->Release(); x = NULL; }
#define WM_GRAPHNOTIFY	(WM_USER+42)

HRESULT ValidateFileName(TCHAR *szFile);
HRESULT MakeUniqueFileName(TCHAR *szFile);

HRESULT OpenInterface(HWND hwnd, HINSTANCE hinst);
HRESULT CloseInterface(HWND hwnd);

extern IGraphBuilder        *gpIGraphBuilder;
extern ICaptureGraphBuilder *gpICaptureGraphBuilder;
extern IBaseFilter          *gpVCap;

HRESULT CreateAudioOnlyPreviewGraph();
HRESULT CreateAudioOnlyCaptureGraph();
HRESULT CreatePreviewGraph();
HRESULT CreateCaptureGraph();

HRESULT AdjustAudioFrequency(void);

HRESULT ConnectVideoWindow(IGraphBuilder *pFg, HWND hwnd, RECT *pRect, BOOL is16By9);
HRESULT RebuildGraph();
HRESULT SetDeInterlacerState(long useDeInterlacer);
HRESULT GetCaptureFileSize(__int64 *size);
HRESULT GetResyncCount(__int64 *val, __int64 *avStatus);
HRESULT GetCurrentBitrates(__int64 *val, __int64 *val2);

HRESULT GetDSoundVolume(__int64 *val);
HRESULT SetDSoundVolume(__int64 val);

#endif