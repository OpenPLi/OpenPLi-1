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
#include "registry.h"

BOOL GetRegStringValue (HKEY hKey, LPCSTR SubKey, LPCSTR SubSubKey,
                        LPCSTR RegVal, LPBYTE pValue, DWORD Size)
{
    HKEY   hkOpenKey;
    HKEY   hkSubOpenKey;
    DWORD  ValueType = REG_SZ;
    DWORD  cb = Size;
    LONG   Result = ERROR_SUCCESS;

    if (RegOpenKey(hKey, SubKey, &hkOpenKey) != ERROR_SUCCESS)
        return FALSE;

    if (RegOpenKey(hkOpenKey, SubSubKey, &hkSubOpenKey) != ERROR_SUCCESS)
        {
        RegCloseKey(hkOpenKey);
        return FALSE;
        }

    pValue[0]=0;
    Result = RegQueryValueEx(hkSubOpenKey, RegVal, NULL , &ValueType, pValue, &cb);

    RegCloseKey(hkSubOpenKey);

    if (hkOpenKey!=hkSubOpenKey)
        RegCloseKey(hkOpenKey);

    return (Result == ERROR_SUCCESS ? TRUE : FALSE);
}


BOOL SetRegStringValue (HKEY hKey, LPCSTR SubKey, LPCSTR SubSubKey,
                               LPCSTR RegVal, LPBYTE pValue, DWORD Size)
{
    HKEY  hkOpenKey;
    HKEY  hkSubOpenKey;

    if (RegOpenKey(hKey, SubKey, &hkOpenKey) != ERROR_SUCCESS)
        return FALSE;

    if (RegOpenKey(hkOpenKey, SubSubKey, &hkSubOpenKey) != ERROR_SUCCESS)
        {
        RegCloseKey(hkOpenKey);
        return FALSE;
        }

    RegSetValueEx (hkSubOpenKey, RegVal, 0, REG_SZ, pValue, Size);

    //  RegFlushKey(hkSubOpenKey);
    RegCloseKey(hkSubOpenKey);

    if (hkOpenKey!=hkSubOpenKey)
        {
        //      RegFlushKey(hkOpenKey);
        RegCloseKey(hkOpenKey);
        }

    return TRUE;
}


BOOL CreateRegKey  (HKEY hKey, LPCSTR SubKey, LPCSTR SubSubKey)
{
    HKEY  hkOpenKey;
    HKEY  hkSubOpenKey;

    if (RegCreateKey(hKey, SubKey, &hkOpenKey) != ERROR_SUCCESS)
        return FALSE;

    if (SubSubKey[0])
        {
        if (RegCreateKey(hkOpenKey, SubSubKey, &hkSubOpenKey) != ERROR_SUCCESS)
            {
            RegCloseKey(hkOpenKey);
            return FALSE;
            }
        RegCloseKey(hkSubOpenKey);
        }

    RegCloseKey(hkOpenKey);

    return TRUE;
}
