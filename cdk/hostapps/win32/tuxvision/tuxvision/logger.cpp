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
#include "logger.h"

TCHAR gLoggerArray[SIZE_LOGGER_ARRAY];
int   gLoggerPointer=0;

HRESULT cdecl LogClear()
{
    int i;
    for(i=0;i<SIZE_LOGGER_ARRAY;i++)
        gLoggerArray[i]=0;
    gLoggerPointer=0;
    return(NOERROR);
}

HRESULT cdecl LogFlush(HWND hwnd)
{
    int i;
    HGLOBAL hGlobalMemory=NULL;
    TCHAR  *lpGlobalMemory=NULL;

    OpenClipboard(hwnd);
    EmptyClipboard();

    hGlobalMemory=GlobalAlloc(GHND, (DWORD) (gLoggerPointer + 1)*sizeof(TCHAR));
    lpGlobalMemory=(TCHAR *)GlobalLock(hGlobalMemory);

    for(i=0;i<gLoggerPointer;i++)
        *lpGlobalMemory++=gLoggerArray[i];

    SetClipboardData(CF_TEXT,hGlobalMemory);
    CloseClipboard();
    return(NOERROR);
}


HRESULT cdecl LogPrintf(LPSTR szFormat, ...)
{
    int i=0,c=0;
    char ach[4096];
    static BOOL fDebug = -1;
    lstrcpy(ach, "TuxVision: ");
    wvsprintf(ach+lstrlen(ach),szFormat,(char *)(&szFormat+1));
    lstrcat(ach, "\r\n");

    if (gLoggerPointer+lstrlen(ach)<SIZE_LOGGER_ARRAY)
        {
        for(i=gLoggerPointer;i<gLoggerPointer+lstrlen(ach);i++)
            gLoggerArray[i]=ach[c++];

        gLoggerPointer+=lstrlen(ach);
        }

    return(NOERROR);
}

