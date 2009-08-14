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

#include <streams.h>
#include <olectl.h>

#include "interface.h"
#include "..\\capture\\ccircularbuffer.h"
#include "render.h"
#include "filter.h"
#include "debug.h"

HRESULT CDBOXIIRender::SaveInternalData(void)
{
HRESULT hr;
ICreateDevEnum *pCreateDevEnum;
hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		  IID_ICreateDevEnum, (void**)&pCreateDevEnum);
if (hr == NOERROR)
	{
	IEnumMoniker *pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_LegacyAmFilterCategory,&pEm, 0);
	pCreateDevEnum->Release();
	if (hr == NOERROR)
		{
		pEm->Reset();
		ULONG cFetched;
		IMoniker *pM=NULL;
		while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
			{
			IPropertyBag *pBag;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr)) 
				{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL);
				if (hr == NOERROR) 
					{
					char achName[264];
					WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, achName, 260, NULL, NULL);
					if (!lstrcmp(achName,"DBOXII Render"))
						{
                        WCHAR wBuffer[264];
                        char  pStr[264];
                        __int64 i64val;
					    VARIANT var;

					    var.vt = VT_I4;
					
                        getParameter(CMD_STOPPLAYBACK, &i64val, NULL); 
                        var.lVal=(long)i64val;
					    hr = pBag->Write(L"StopPlayback", &var);

                        getParameter(CMD_DECODERTYPE, &i64val, NULL); 
                        var.lVal=(long)i64val;
					    hr = pBag->Write(L"DecoderType", &var);

					    var.vt = VT_BSTR;
                        var.bstrVal=wBuffer;

						getParameter(CMD_IPADDRESS, (__int64 *)pStr, NULL); 
                        memset(wBuffer,0,sizeof(wBuffer));
       					MultiByteToWideChar(CP_ACP, 0, pStr, lstrlen(pStr), wBuffer, 264);
					    hr = pBag->Write(L"IPAddress", &var);

						getParameter(CMD_LOGIN, (__int64 *)pStr, NULL); 
                        memset(wBuffer,0,sizeof(wBuffer));
       					MultiByteToWideChar(CP_ACP, 0, pStr, lstrlen(pStr), wBuffer, 264);
					    hr = pBag->Write(L"Login", &var);

						getParameter(CMD_PASSWORD, (__int64 *)pStr, NULL); 
                        memset(wBuffer,0,sizeof(wBuffer));
       					MultiByteToWideChar(CP_ACP, 0, pStr, lstrlen(pStr), wBuffer, 264);
					    hr = pBag->Write(L"Password", &var);

						}
					SysFreeString(var.bstrVal);
					}
				pBag->Release();
				}
			}
		if (pM!=NULL)
			pM->Release();
		pEm->Release();
		}
	}
return(NOERROR);
}

HRESULT CDBOXIIRender::LoadInternalData(void)
{
HRESULT hr;
ICreateDevEnum *pCreateDevEnum;
hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		  IID_ICreateDevEnum, (void**)&pCreateDevEnum);
if (hr == NOERROR)
	{
	IEnumMoniker *pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_LegacyAmFilterCategory,&pEm, 0);
	pCreateDevEnum->Release();
	if (hr == NOERROR)
		{
		pEm->Reset();
		ULONG cFetched;
		IMoniker *pM=NULL;
		while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
			{
			IPropertyBag *pBag;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr)) 
				{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL);
				if (hr == NOERROR) 
					{
					char achName[264];
					WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, achName, 260, NULL, NULL);
					if (!lstrcmp(achName,"DBOXII Render"))
						{
                        WCHAR wBuffer[264];
                        char  pStr[264];
					    VARIANT var;

					    var.vt = VT_I4;
					    hr = pBag->Read(L"StopPlayback", &var, NULL);
						if (SUCCEEDED(hr))
							setParameter(CMD_STOPPLAYBACK, var.lVal);

					    hr = pBag->Read(L"DecoderType", &var, NULL);
						if (SUCCEEDED(hr))
							setParameter(CMD_DECODERTYPE, var.lVal);

					    var.vt = VT_BSTR;
                        var.bstrVal=wBuffer;

                        hr = pBag->Read(L"IPAddress", &var, NULL);
                        if (SUCCEEDED(hr))
                            {
                            WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, pStr, 264, NULL, NULL);
                            setParameter(CMD_IPADDRESS, (__int64)pStr);
                            }

                        hr = pBag->Read(L"Login", &var, NULL);
                        if (SUCCEEDED(hr))
                            {
                            WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, pStr, 264, NULL, NULL);
                            setParameter(CMD_LOGIN, (__int64)pStr);
                            }

                        hr = pBag->Read(L"Password", &var, NULL);
                        if (SUCCEEDED(hr))
                            {
                            WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, pStr, 264, NULL, NULL);
                            setParameter(CMD_PASSWORD, (__int64)pStr);
                            }
						}
					SysFreeString(var.bstrVal);
					}
				pBag->Release();
				}
			}
		if (pM!=NULL)
			pM->Release();
		pEm->Release();
		}
	}
return(NOERROR);
}

