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
#include <streams.h>
#include <strmif.h>
#include <math.h>

#include "WinHelper.h"
#include "TuxVision.h"
#include "resource.h"
#include "DShow.h"
#include "debug.h"

HWND   ghwndSplash=NULL;
HFONT  ghfontApp=NULL;
BOOL   gFullscreen=FALSE;

int CALLBACK SplashProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) 
        {
       	case WM_INITDIALOG:
    		return 0;
	    case WM_ERASEBKGND:
		    return(0);
        }
    return (0);
}

HFONT EzCreateFont (HDC hdc, char * szFaceName, int iDeciPtHeight,
                    int iDeciPtWidth, int iAttributes, BOOL fLogRes)
{
     FLOAT      cxDpi, cyDpi ;
     HFONT      hFont ;
     LOGFONT    lf ;
     POINT      pt ;
     TEXTMETRIC tm ;

     SaveDC (hdc) ;

     SetGraphicsMode (hdc, GM_ADVANCED) ;
     ModifyWorldTransform (hdc, NULL, MWT_IDENTITY) ;
     SetViewportOrgEx (hdc, 0, 0, NULL) ;
     SetWindowOrgEx   (hdc, 0, 0, NULL) ;

     if (fLogRes)
          {
          cxDpi = (FLOAT) GetDeviceCaps (hdc, LOGPIXELSX) ;
          cyDpi = (FLOAT) GetDeviceCaps (hdc, LOGPIXELSY) ;
          }
     else
          {
          cxDpi = (FLOAT) (25.4 * GetDeviceCaps (hdc, HORZRES) /
                                  GetDeviceCaps (hdc, HORZSIZE)) ;

          cyDpi = (FLOAT) (25.4 * GetDeviceCaps (hdc, VERTRES) /
                                  GetDeviceCaps (hdc, VERTSIZE)) ;
          }

     pt.x = (int) (iDeciPtWidth  * cxDpi / 72) ;
     pt.y = (int) (iDeciPtHeight * cyDpi / 72) ;

     DPtoLP (hdc, &pt, 1) ;

     lf.lfHeight         = - (int) (fabs (pt.y) / 10.0 + 0.5) ;
     lf.lfWidth          = 0 ;
     lf.lfEscapement     = 0 ;
     lf.lfOrientation    = 0 ;
     lf.lfWeight         = iAttributes & EZ_ATTR_BOLD      ? 700 : 0 ;
     lf.lfItalic         = iAttributes & EZ_ATTR_ITALIC    ?   1 : 0 ;
     lf.lfUnderline      = iAttributes & EZ_ATTR_UNDERLINE ?   1 : 0 ;
     lf.lfStrikeOut      = iAttributes & EZ_ATTR_STRIKEOUT ?   1 : 0 ;
     lf.lfCharSet        = 0 ;
     lf.lfOutPrecision   = 0 ;
     lf.lfClipPrecision  = 0 ;
     lf.lfQuality        = 0 ;
     lf.lfPitchAndFamily = 0 ;

     lstrcpy (lf.lfFaceName, szFaceName) ;
     hFont = CreateFontIndirect (&lf) ;
     if (iDeciPtWidth != 0)
          {
          hFont = (HFONT) SelectObject (hdc, hFont) ;
          GetTextMetrics (hdc, &tm) ;
          DeleteObject (SelectObject (hdc, hFont)) ;
          lf.lfWidth = (int) (tm.tmAveCharWidth * fabs (pt.x) / fabs (pt.y) + 0.5) ;
          hFont = CreateFontIndirect (&lf) ;
          }
     RestoreDC (hdc, -1) ;

     return hFont ;
}

void CenterWindow(HWND hwnd)
{
    RECT rc;
    int left, top;
    int width =GetSystemMetrics(SM_CXSCREEN);
    int height=GetSystemMetrics(SM_CYSCREEN);
    
    GetWindowRect(hwnd, &rc);
    left=(width -Width(rc))/2;
    top =(height-Height(rc))/2;

    SetWindowPos(hwnd,HWND_TOP,left,top,0,0,SWP_NOSIZE);
}

void DisplaySplash(void)
{
    ghwndSplash=CreateDialog(ghInstApp, MAKEINTRESOURCE(SPLASH), NULL, SplashProc);
    if (ghwndSplash)
        {
        SetWindowPos(ghwndSplash,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
        UpdateWindow(ghwndSplash);
        }

    if (ghwndSplash)
        {
        HFONT hfontSplash=NULL;
        HDC hdc=GetDC(ghwndSplash);
        HBRUSH hBrush=CreateSolidBrush(0);
        HRGN hRgn=CreateRoundRectRgn(0,0,352,280,16,16);
        SetWindowRgn(ghwndSplash,hRgn,TRUE);
        SelectObject(hdc,GetStockObject(NULL_BRUSH));
        SelectObject(hdc, CreatePen(PS_SOLID,1,0));
        RoundRect(hdc,0,0,352,280,16,16);

#if ALPHA_VERSION
	    hfontSplash =EzCreateFont (hdc,"Arial",320, 0, EZ_ATTR_BOLD, FALSE);
        SelectObject(hdc, hfontSplash);
        SetTextColor(hdc,0x20A020);
        SetBkMode(hdc,TRANSPARENT);
        TextOut(hdc,60,180,ALPHA_STRING,lstrlen(ALPHA_STRING));
	    SelectObject(hdc, ghfontApp);
        DeleteObject(hfontSplash);
#endif
	    hfontSplash =EzCreateFont (hdc,"Arial",120, 0, EZ_ATTR_BOLD, FALSE);
        SelectObject(hdc, hfontSplash);
        SetTextColor(hdc,0x505050);
        SetBkMode(hdc,TRANSPARENT);
        TextOut(hdc,240,258,REVISION,lstrlen(REVISION));
	    SelectObject(hdc, ghfontApp);
        DeleteObject(hfontSplash);


        ReleaseDC(ghwndSplash,hdc);
        UpdateWindow(ghwndSplash);
        } 
}

HRESULT SetFullscreen(HWND hWndParent, HWND hWnd, RECT *restore, BOOL flag)
{
    HRESULT hr;
    HWND style=HWND_TOP;

// ---------------------------------------------------------------------------------    
//                          FULLSCREEN - MODE
// ---------------------------------------------------------------------------------    
    if (!gSetVideoToWindow)
        {
        if (flag)
            { 
            RECT rc;
            DWORD val;

            if (gAlwaysOnTop)
                style=HWND_TOPMOST;

            GetWindowRect(ghWndApp, &rc);
            glAppTop=rc.top;
            glAppLeft=rc.left;

            rc.left=-GetSystemMetrics(SM_CXDLGFRAME);
            rc.top=-GetSystemMetrics(SM_CYDLGFRAME);
            rc.right =GetSystemMetrics(SM_CXSCREEN)+2*GetSystemMetrics(SM_CXDLGFRAME);
            rc.bottom=GetSystemMetrics(SM_CYSCREEN)+2*GetSystemMetrics(SM_CYDLGFRAME);

            val=GetWindowLong(ghWndApp, GWL_STYLE);
            
            SetWindowLong(ghWndApp, GWL_STYLE, (val) & (~(WS_CAPTION|WS_CLIPSIBLINGS)) );
            
            SetWindowPos(ghWndApp, style ,rc.left, rc.top, Width(rc), Height(rc), 0);
            gFullscreen=TRUE;
            MoveVideoWindow();
            InvalidateRect(ghWndApp, NULL, TRUE);
            UpdateWindow(ghWndApp);
            }
        else
            { 
            DWORD val;
            RECT rc;
            val=GetWindowLong(ghWndApp,GWL_STYLE);
            SetWindowLong(ghWndApp,GWL_STYLE, (val) | (WS_CAPTION|WS_CLIPSIBLINGS) );

            if (gAlwaysOnTop)
                style=HWND_TOPMOST;

            SetWindowPos(hWndParent, style, glAppLeft, glAppTop,  glAppWidth, glAppHeight, 0);

            SetWindowPos(hWnd,NULL,
                        restore->left, 
                        restore->top , 
                        pWidth(restore), 
                        pHeight(restore),
                        SWP_SHOWWINDOW|SWP_NOZORDER);
            CopyRect(&rc, restore);
            rc.top=0;
            rc.left=0;
            hr=ConnectVideoWindow(gpIGraphBuilder, hWnd, &rc, gIs16By9);
		    gFullscreen=FALSE;
            InvalidateRect(ghWndApp, NULL, TRUE);
            UpdateWindow(ghWndApp);
            }

        }
    else
// ---------------------------------------------------------------------------------    
//                          WINDOW - MODE
// ---------------------------------------------------------------------------------    
        {
        if (flag)
            { 
            RECT rc;
            DWORD val;

            if (gAlwaysOnTop)
                style=HWND_TOPMOST;

            GetWindowRect(ghWndApp, &rc);
            val=GetWindowLong(ghWndApp, GWL_STYLE);
            SetWindowLong(ghWndApp, GWL_STYLE, (val|WS_THICKFRAME) & (~(WS_CAPTION|WS_CLIPSIBLINGS)) );
            SetWindowPos(ghWndApp, style ,rc.left, rc.top, 352, 288, 0);
            gFullscreen=TRUE;
            MoveVideoWindow();
            InvalidateRect(ghWndApp, NULL, TRUE);
            UpdateWindow(ghWndApp);
            }
        else
            { 
            DWORD val;
            RECT rc;
            val=GetWindowLong(ghWndApp,GWL_STYLE);
            SetWindowLong(ghWndApp,GWL_STYLE, (val & (~(WS_THICKFRAME)) ) | (WS_CAPTION|WS_CLIPSIBLINGS) );

            if (gAlwaysOnTop)
                style=HWND_TOPMOST;

            SetWindowPos(hWndParent, style, 0, 0, glAppWidth, glAppHeight, SWP_NOMOVE);

            SetWindowPos(hWnd,NULL,
                        restore->left, 
                        restore->top , 
                        pWidth(restore), 
                        pHeight(restore),
                        SWP_SHOWWINDOW|SWP_NOZORDER);
            CopyRect(&rc, restore);
            rc.top=0;
            rc.left=0;
            hr=ConnectVideoWindow(gpIGraphBuilder, hWnd, &rc, gIs16By9);
		    gFullscreen=FALSE;
            InvalidateRect(ghWndApp, NULL, TRUE);
            UpdateWindow(ghWndApp);
            }
        }                    
    return(hr);
}


HRESULT MoveVideoWindow()
{ 
    RECT rc={0,0,0,0};

    if (!gFullscreen)
        return(NOERROR);

    GetClientRect(ghWndApp, &rc);
    SetWindowPos(ghWndVideo, NULL ,0 ,0, Width(rc), Height(rc), SWP_NOZORDER);
    HRESULT hr=ConnectVideoWindow(gpIGraphBuilder, ghWndVideo, &rc, gIs16By9);

    return(hr);
}

 