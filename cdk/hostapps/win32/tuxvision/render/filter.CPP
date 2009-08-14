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
#include <initguid.h>
#include <process.h>
#include <dvdmedia.h>
#include <ks.h>
#include <ksmedia.h>

#include <stdio.h>
#include <io.h>
#include <stdlib.h>

#include "interface.h"
#include "filter.h"
#include "..\\capture\\ccircularbuffer.h"
#include "render.h"
#include "..\\capture\\debug.h"


// Setup data

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_NULL,       // Major type
    &MEDIASUBTYPE_NULL     // Minor type
};

const AMOVIESETUP_PIN sudPins[] =
{
    { 
        L"Input",               // strName
        FALSE,                  // bRendered
        FALSE,                  // bOutput
        FALSE,                  // bZero
        FALSE,                  // bMany
        &CLSID_NULL,            // clsConnectsToFilter
        NULL,                   // strConnectsToPin
        1,                      // nTypes
        &sudOpPinTypes          // lpTypes
    },
    { 
        L"Input",               // strName
        FALSE,                  // bRendered
        FALSE,                  // bOutput
        FALSE,                  // bZero
        FALSE,                  // bMany
        &CLSID_NULL,            // clsConnectsToFilter
        NULL,                   // strConnectsToPin
        1,                      // nTypes
        &sudOpPinTypes          // lpTypes
    }
};       

const AMOVIESETUP_FILTER sudFilter =
{
    &CLSID_DBOXIIRender,       // Filter CLSID
    L"DBOXII Render",          // String name
    MERIT_DO_NOT_USE,          // Filter merit
    NUMELMS(sudPins),          // Number pins
    sudPins                    // Pin details
};


// COM global table of objects in this dll

CFactoryTemplate g_Templates[2] = {
  { L"DBOXII Render"
  , &CLSID_DBOXIIRender
  , CDBOXIIRender::CreateInstance
  , NULL
  , &sudFilter },


  { L"DBOXII Render Property Page"
  , &CLSID_DBOXIIRenderProperty
  , CProperties::CreateInstance
  , NULL
  , NULL }

};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);
int g_Instance=0;


// register as video render filter
STDAPI DllRegisterServer( void )
{
    HRESULT hr=NOERROR;
    CDBOXIIRender *pTempFilter=new CDBOXIIRender(NULL, &hr);

    if (pTempFilter!=NULL)
        {
        lstrcpy(pTempFilter->m_DBOXAddress,"192.168.0.13");
        lstrcpy(pTempFilter->m_DBOXLogin,"root");
        lstrcpy(pTempFilter->m_DBOXPassword,"dbox2");
        pTempFilter->m_DBOXStopPlayback=0;
        pTempFilter->m_IsENX=0;
        pTempFilter->LoadInternalData();
        }

    // register the server
    hr = AMovieDllRegisterServer2( TRUE );

    if (pTempFilter!=NULL)
        {
        pTempFilter->SaveInternalData();
        delete pTempFilter;
        }

    return hr;
}

// unregister as video render filter
STDAPI DllUnregisterServer( void )
{
    HRESULT hr = AMovieDllRegisterServer2( FALSE );
    return hr;
}

//
// CreateInstance
//
CUnknown * WINAPI CDBOXIIRender::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CDBOXIIRender(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;

} // CreateInstance

// -----------------------------------------------------------
//
// -----------------------------------------------------------

//
// NonDelegatingQueryInterface
//
STDMETHODIMP CDBOXIIRender::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);
    if (riid == IID_IDBOXIIRender) 
        return GetInterface((IDBOXIIRender *) this, ppv);
    else 
    if (riid == IID_ISpecifyPropertyPages) 
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
	else
        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CDBOXIIRender::GetPages(CAUUID * pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_DBOXIIRenderProperty;
    return NOERROR;
} // GetPages

//
// Constructor
//
// Initialise a CPushStream object so that we have a pin.
//
CDBOXIIRender::CDBOXIIRender(LPUNKNOWN lpunk,  HRESULT *phr) :
    CBaseFilter(NAME("DBOXII Render"), lpunk, &m_Lock, CLSID_DBOXIIRender)
{
    m_pPinVideoIn=NULL;
    m_pPinAudioIn=NULL;
 
    m_pPinVideoIn = new CDBOXIIRenderInputPin(L"VPES",
                               this,
                               GetOwner(),
                               this,
                               &m_Lock,
                               phr,
                               0);
    if (m_pPinVideoIn == NULL) 
        {
        *phr = E_OUTOFMEMORY;
        return;
        }
    m_pPinAudioIn = new CDBOXIIRenderInputPin(L"APES",
                               this,
                               GetOwner(),
                               this,
                               &m_Lock,
                               phr,
                               1);
    if (m_pPinAudioIn == NULL) 
        {
        *phr = E_OUTOFMEMORY;
        return;
        }

    m_pPinMultiplexIn = new CDBOXIIRenderInputPin(L"PS",
                               this,
                               GetOwner(),
                               this,
                               &m_Lock,
                               phr,
                               2);
    if (m_pPinMultiplexIn == NULL) 
        {
        *phr = E_OUTOFMEMORY;
        return;
        }


    g_Instance++;

    m_LastDeliveredAudio=0;
    m_LastDeliveredVideo=0;

    m_MyState=0;
    
    InitSockets();

    lstrcpy(m_DBOXAddress,"192.168.0.13");
    lstrcpy(m_DBOXLogin,"root");
    lstrcpy(m_DBOXPassword,"dbox2");
    m_DBOXStopPlayback=0;
    m_IsENX=0;

    m_DBOXVPort=40000;
    m_DBOXAPort=40001;
    m_DBOXHTTPPort=80;
    m_DBOXTelnetPort=23;

    gpFilter=this;

    LoadInternalData();
/*
{
FILE *fp=fopen("e:\\audio.mpg","rb");
int i=0, j=0;
for(j=0;j<4;j++)
    {
    for (i=0;i<16;i++)
        {
        char dtxt[264];
        char c=0;
        fread(&c, 1, 1, fp);
        sprintf(dtxt,"0x%8.8X, ", c);
        OutputDebugString(dtxt);
        }
    OutputDebugString("\r\n");
    }
}
    dprintf("...");
*/


} // (Constructor)

CDBOXIIRender::~CDBOXIIRender()
{
    SaveInternalData();

    if (m_pPinVideoIn)
        delete (m_pPinVideoIn); 
    if (m_pPinAudioIn)
        delete (m_pPinAudioIn); 
    if (m_pPinMultiplexIn)
        delete (m_pPinMultiplexIn); 

    DeInitSockets();

	g_Instance--;
}

STDMETHODIMP CDBOXIIRender::setParameter(__int64 command, __int64 data1)
{
    HRESULT hr=E_FAIL;
    switch(command)
        {
        case CMD_IPADDRESS:
            if (data1==0)
                return(E_POINTER);
            lstrcpy(m_DBOXAddress, (LPSTR)data1);
            hr=NOERROR;
            break;
        case CMD_LOGIN:
            if (data1==0)
                return(E_POINTER);
            lstrcpy(m_DBOXLogin, (LPSTR)data1);
            hr=NOERROR;
            break;
        case CMD_PASSWORD:
            if (data1==0)
                return(E_POINTER);
            lstrcpy(m_DBOXPassword, (LPSTR)data1);
            hr=NOERROR;
            break;
        case CMD_STOPPLAYBACK:
            m_DBOXStopPlayback=(int)data1;
            hr=NOERROR;
            break;
        case CMD_DECODERTYPE:
            m_IsENX=(int)data1;
            hr=NOERROR;
            break;
        }
	return(hr);
}

STDMETHODIMP CDBOXIIRender::getParameter(__int64 command, __int64 *data1, __int64 *data2)
{
    HRESULT hr=E_FAIL;
    if (data1==NULL)
        return(E_POINTER);

    switch(command)
        {
        case CMD_IPADDRESS:
            lstrcpy((LPSTR)data1, m_DBOXAddress);
            hr=NOERROR;
            break;
        case CMD_LOGIN:
            lstrcpy((LPSTR)data1, m_DBOXLogin);
            hr=NOERROR;
            break;
        case CMD_PASSWORD:
            lstrcpy((LPSTR)data1, m_DBOXPassword);
            hr=NOERROR;
            break;
        case CMD_STOPPLAYBACK:
            *data1=(__int64)m_DBOXStopPlayback;
            hr=NOERROR;
            break;
        case CMD_DECODERTYPE:
            *data1=(__int64)m_IsENX;
            hr=NOERROR;
            break;
        case CMD_DELIVEREDDATA:
            *data1=gDeliveredAudioData;
            if (data2!=NULL)
                *data2=gDeliveredVideoData;
            break;
        }
	return(hr);
}

STDMETHODIMP CDBOXIIRender::Stop()
{
    CAutoLock cAutoLock(&m_Lock);

    HRESULT hr;
    m_MyState=-1;
    m_LastDeliveredAudio=0;
    m_LastDeliveredVideo=0;
    DeInitStreaming();
    ControlPlaybackOnDBOX(m_DBOXAddress,m_DBOXHTTPPort, 0);

    hr=CBaseFilter::Stop();

    return(hr);
}

STDMETHODIMP CDBOXIIRender::Pause()
{
    HRESULT hr;
    if (m_State==State_Stopped)
        {
        int vp=0;
        int ap=0;


        gEOS_0=FALSE;
        gEOS_1=FALSE;
        gEOS_2=FALSE;

        if (m_pPinVideoIn->IsConnected())
            gIsVideoConnected=TRUE;
        else
            gIsVideoConnected=FALSE;

        if (m_pPinAudioIn->IsConnected())
            gIsAudioConnected=TRUE;
        else
            gIsAudioConnected=FALSE;

        if (m_pPinMultiplexIn->IsConnected())
            gIsMultiplexerConnected=TRUE;
        else
            gIsMultiplexerConnected=FALSE;

        ControlPlaybackOnDBOX(m_DBOXAddress,m_DBOXHTTPPort, 2);
        ExecuteCommand("killall -HUP controld", m_DBOXLogin, m_DBOXPassword, m_DBOXAddress, m_DBOXTelnetPort, 1, m_IsENX);

        if (m_pPinVideoIn->IsConnected())
            vp=m_DBOXVPort;
        if (m_pPinAudioIn->IsConnected())
            ap=m_DBOXAPort;
        if (m_pPinMultiplexIn->IsConnected())
            {
            vp=m_DBOXVPort;
            ap=m_DBOXAPort;
            }

        MuteAudio(m_DBOXAddress, m_DBOXHTTPPort, TRUE);
        InitStreaming(m_DBOXAddress, ap, vp);
        m_MyState=0;

        Sleep(1100);
        MuteAudio(m_DBOXAddress, m_DBOXHTTPPort, FALSE);

        }
    hr=CBaseFilter::Pause();
    return(hr);
}

STDMETHODIMP CDBOXIIRender::Run(REFERENCE_TIME tStart)
{
    HRESULT hr;
    hr=CBaseFilter::Run(tStart);
    return(hr);
}

int CDBOXIIRender::GetPinCount(void)
{
    return(2);  //3
}

CBasePin* CDBOXIIRender::GetPin(int n)
{
    switch(n)
        {
        case 0: return(m_pPinVideoIn);
        case 1: return(m_pPinAudioIn);
        //case 2: return(m_pPinMultiplexIn);
        }
    return(NULL);
}

HRESULT CDBOXIIRender::GetCurrentState()
{
    return(m_MyState);
}

// -----------------------------------------------------------
//
// -----------------------------------------------------------

CDBOXIIRenderInputPin::CDBOXIIRenderInputPin(LPCWSTR pName,
                             CDBOXIIRender *pXSync,
                             LPUNKNOWN pUnk,
                             CBaseFilter *pFilter,
                             CCritSec *pLock,
                             HRESULT *phr,
                             int type) :

    CBaseInputPin(NAME("Input"),
                  pFilter,     
                  pLock,
                  phr,  
                  pName)
{
    m_pILock=pLock;
    m_Type=type;
    m_pFilter=pXSync;
}

HRESULT CDBOXIIRenderInputPin::CheckMediaType(const CMediaType *cmt)
{
    return(NOERROR);

    if (m_Type==0)
        {
        if (IsEqualGUID(*cmt->Type(), MEDIATYPE_Video)) 
    	    return NOERROR;
        }
    else
    if (m_Type==1)
        {
        if (IsEqualGUID(*cmt->Type(), MEDIATYPE_Audio)) 
    	    return NOERROR;
        }
    else
    if (m_Type==2)
        {
  	    return NOERROR;
        }
    return(E_FAIL);
}

HRESULT CDBOXIIRenderInputPin::SetMediaType(const CMediaType *cmt)
{
    HRESULT hr=NOERROR;
    hr=CBaseInputPin::SetMediaType(cmt);
    return(NOERROR);
}

STDMETHODIMP CDBOXIIRenderInputPin::EndOfStream(void)
{
    HRESULT hr=NOERROR;
    switch(m_Type)
        {
        case 0:gEOS_0=TRUE;break;
        case 1:gEOS_1=TRUE;break;
        case 2:gEOS_2=TRUE;break;
        }
    hr=CBaseInputPin::EndOfStream();   
    return(hr);
}

STDMETHODIMP CDBOXIIRenderInputPin::BeginFlush(void)
{
    m_pFilter->m_MyState=-1;
    return(CBaseInputPin::BeginFlush());   
}

STDMETHODIMP CDBOXIIRenderInputPin::EndFlush(void)
{
    m_pFilter->m_MyState=0;
    return(CBaseInputPin::EndFlush());   
}

STDMETHODIMP CDBOXIIRenderInputPin::ReceiveCanBlock()
{
    return(S_OK);
}

STDMETHODIMP CDBOXIIRenderInputPin::Receive(IMediaSample *pSample)
{
    HRESULT hr=NOERROR;
	BYTE *pSrc;
	DWORD len;
    REFERENCE_TIME pStart=0, pEnd=0;

//    CAutoLock cAutoLock(m_pILock);

    hr=pSample->GetTime(&pStart,&pEnd);

    if (gfThreadAborted)
        {
        m_pFilter->NotifyEvent(EC_USERABORT, S_OK, 0);
        m_pFilter->m_MyState=-1;
        dprintf("EC_USERABORT notification");
        }

    if (m_pFilter->m_MyState<0)
        return(NOERROR);

	pSample->GetPointer(&pSrc);     
	len=pSample->GetActualDataLength();


    if (m_Type==0)
        {
        //dprintf("IN V: %ld, Time: %ld",len, pStart/10000);
        if (CVideoBuffer)
            {
            while (!CVideoBuffer->canWrite(len))
                {
                Sleep(2);
                //dprintf("V cycle ...");
                if (m_pFilter->m_MyState<0)
                    return(NOERROR);
                }
            CVideoBuffer->Write( pSrc, len);
            }
        }
    else
    if (m_Type==1)
        {
        //dprintf("IN A: %ld, Time: %ld",len, pStart/10000);
        if (CAudioBuffer)
            {
            while (!CAudioBuffer->canWrite(len))
                {
                Sleep(2);
                //dprintf("A cycle ...");
                if (m_pFilter->m_MyState<0)
                    return(NOERROR);
                }
            CAudioBuffer->Write( pSrc, len);
            }
        }
    else
    if (m_Type==2)
        {
        //dprintf("IN A/V: %ld",len);
        if (CMultiplexBuffer)
            {
            while (!CMultiplexBuffer->canWrite(len))
                {
                Sleep(2);
                //dprintf("A cycle ...");
                if (m_pFilter->m_MyState<0)
                    return(NOERROR);
                }
            CMultiplexBuffer->Write( pSrc, len);
            }
        }

    return(hr);
}


