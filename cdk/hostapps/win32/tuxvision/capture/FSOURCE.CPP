//
//  DBOXII Capture Filter
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
#include <uuids.h>
#include <olectl.h>
#include <initguid.h>
#include <ks.h>
#include <ksmedia.h>
#include <mmreg.h>

#include "debug.h"
#include "interface.h"
#include "source.h"
#include "fsource.h"
#include "ccircularbuffer.h"
#include "Remuxer.h"
#include "grab.h"

#define USE_DBOX    1

//#define ERROR_NOTIFICATION EC_STREAM_ERROR_STILLPLAYING
#define ERROR_NOTIFICATION EC_ERRORABORT

const static UCHAR ColorBars100Amp100SatYUV [4][8] = 
{
//  Whi Yel Cya Grn Mag Red Blu Blk
    128, 16,166, 54,202, 90,240,128,    // U
    235,211,170,145,106, 81, 41, 16,    // Y
    128,146, 16, 34,222,240,109,128,    // V
    235,211,170,145,106, 81, 41, 16     // Y
};


void CreateTestPattern(BYTE *data, int w, int h, int param)
{
    int i,x;
    UINT  col; 
    PUCHAR pT;

    for(i=0;i<h;i++)
        {
        pT=data+(w*2)*i;
#if 0
        for (x = 0; x < (w / 2); x++) 
            {
            col = (x * 8) / (w / 2);
            *pT++ = ColorBars100Amp100SatYUV[1][col]; // U
            *pT++ = ColorBars100Amp100SatYUV[0][col]; // Y
            *pT++ = ColorBars100Amp100SatYUV[3][col]; // V
            *pT++ = ColorBars100Amp100SatYUV[2][col]; // Y
            }   
#else
        int ww=(int)(((double)w*(double)param) / 255.0);
        for (x = 0; x < (ww / 2); x++) 
            {
            col = (x * 8) / (ww / 2);
            *pT++ = ColorBars100Amp100SatYUV[1][col]; // U
            *pT++ = ColorBars100Amp100SatYUV[0][col]; // Y
            *pT++ = ColorBars100Amp100SatYUV[3][col]; // V
            *pT++ = ColorBars100Amp100SatYUV[2][col]; // Y
            }   
#endif
        }
} 


// Setup data
const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN sudOpPin[] =
{
    {
    L"Video",               // Pin string name
    FALSE,                  // Is it rendered
    TRUE,                   // Is it an output
    FALSE,                  // Can we have none
    FALSE,                  // Can we have many
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of types
    &sudOpPinTypes          // Pin details
    },

    {
    L"Audio",               // Pin string name
    FALSE,                  // Is it rendered
    TRUE,                   // Is it an output
    FALSE,                  // Can we have none
    FALSE,                  // Can we have many
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of types
    &sudOpPinTypes          // Pin details
    },

    {
    L"Output PS",           // Pin string name
    FALSE,                  // Is it rendered
    TRUE,                   // Is it an output
    FALSE,                  // Can we have none
    FALSE,                  // Can we have many
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of types
    &sudOpPinTypes          // Pin details
    },

    {
    L"Output VPES",         // Pin string name
    FALSE,                  // Is it rendered
    TRUE,                   // Is it an output
    FALSE,                  // Can we have none
    FALSE,                  // Can we have many
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of types
    &sudOpPinTypes          // Pin details
    },

    {
    L"Output APES",         // Pin string name
    FALSE,                  // Is it rendered
    TRUE,                   // Is it an output
    FALSE,                  // Can we have none
    FALSE,                  // Can we have many
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of types
    &sudOpPinTypes          // Pin details
    },
   
};       

const AMOVIESETUP_FILTER sudFilter =
{
    &CLSID_DBOXIICapture,   // Filter CLSID
    L"DBOXII Capture",      // String name
    MERIT_DO_NOT_USE,       // Filter merit
    5,                      // Number pins
    sudOpPin                // Pin details
};


// COM global table of objects in this dll

CFactoryTemplate g_Templates[2] = {
  { L"DBOXII Capture"
  , &CLSID_DBOXIICapture
  , CDBOXIICapture::CreateInstance
  , NULL
  , &sudFilter },


  { L"DBOXII Capture Property Page"
  , &CLSID_DBOXIICaptureProperty
  , CProperties::CreateInstance
  , NULL
  , NULL }

};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

int g_Instance=0;


//
// DllRegisterServer
//
STDAPI DllRegisterServer( void )
{
    HRESULT hr=NOERROR;
    CDBOXIICapture *pTempFilter=new CDBOXIICapture(NULL, &hr);

    if (pTempFilter!=NULL)
        {
        lstrcpy(pTempFilter->m_DBOXAddress,"192.168.0.13");
        lstrcpy(pTempFilter->m_DBOXLogin,"root");
        lstrcpy(pTempFilter->m_DBOXPassword,"dbox2");
        pTempFilter->LoadInternalData();
        }


    // register the server
    hr = AMovieDllRegisterServer2( TRUE );
    if( FAILED(hr) )
        return hr;

    IFilterMapper2 *pFm2 = NULL;
    hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void **)&pFm2);
    
    if(FAILED(hr))
        return hr;

    REGFILTER2 rf2;
    rf2.dwVersion = 1;
    rf2.dwMerit = MERIT_UNLIKELY;
    rf2.cPins = NUMELMS(sudOpPin);
    rf2.rgPins = sudOpPin;

    hr = pFm2->RegisterFilter(
        CLSID_DBOXIICapture,
        L"DBOXII Capture",              // name shown to the user
        0,                              // device moniker
        &CLSID_VideoInputDeviceCategory,
        L"DBOXII Capture",              // unique instance name
        &rf2);

    hr = pFm2->RegisterFilter(
        CLSID_DBOXIICapture,
        L"DBOXII Capture",              // name shown to the user
        0,                              // device moniker
        &KSCATEGORY_CAPTURE,
        L"DBOXII Capture",              // unique instance name
        &rf2);



    pFm2->Release();
 
    if (pTempFilter!=NULL)
        {
        pTempFilter->SaveInternalData();
        delete pTempFilter;
        }


    return hr;

}

//
// DllUnregisterServer
//
STDAPI DllUnregisterServer( void )
{

    HRESULT hr = AMovieDllRegisterServer2( FALSE );
    if( FAILED(hr) )
        return hr;

    IFilterMapper2 *pFm2 = NULL;

    hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void **)&pFm2);
    
    if(FAILED(hr))
        return hr;

    hr = pFm2->UnregisterFilter(
        &CLSID_VideoInputDeviceCategory,
        L"DBOXII Capture",
        CLSID_DBOXIICapture);

    hr = pFm2->UnregisterFilter(
        &KSCATEGORY_CAPTURE,
        L"DBOXII Capture",
        CLSID_DBOXIICapture);

    pFm2->Release();
 
    return hr;
}


//
// CreateInstance
//
CUnknown * WINAPI CDBOXIICapture::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CDBOXIICapture(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;

} // CreateInstance

//
// Constructor
//
CDBOXIICapture::CDBOXIICapture(LPUNKNOWN lpunk, HRESULT *phr) :
    CSource(NAME("DBOXII Capture"),
            lpunk,
            CLSID_DBOXIICapture)
{
    CAutoLock cAutoLock(&m_cStateLock);
    HRESULT hr=NOERROR;
    __int64 val=0;
    char szData[264]="";

    m_paStreams    = (CSourceStream **) new CPushStream*[5];
    if (m_paStreams == NULL) {
        *phr = E_OUTOFMEMORY;
	return;
    }



    m_paStreams[0] = new CPushStream(phr, this, L"Video", SAMPLE_TYPE_VRAW);
    if (m_paStreams[0] == NULL) 
        {
        *phr = E_OUTOFMEMORY;
	    return;
        }

    m_paStreams[1] = new CPushStream(phr, this, L"Audio", SAMPLE_TYPE_ARAW);
    if (m_paStreams[1] == NULL) 
        {
        *phr = E_OUTOFMEMORY;
	    return;
        }

    m_paStreams[2] = new CPushStream(phr, this, L"PS", SAMPLE_TYPE_PS);
    if (m_paStreams[2] == NULL) 
        {
        *phr = E_OUTOFMEMORY;
	    return;
        }

    m_paStreams[3] = new CPushStream(phr, this, L"V_ES", SAMPLE_TYPE_VPES);
    if (m_paStreams[3] == NULL) 
        {
        *phr = E_OUTOFMEMORY;
	    return;
        }

    m_paStreams[4] = new CPushStream(phr, this, L"A_ES", SAMPLE_TYPE_APES);
    if (m_paStreams[4] == NULL) 
        {
        *phr = E_OUTOFMEMORY;
	    return;
        }

	g_Instance++;

	pPushStreamVideoRAW =(CPushStream*)m_paStreams[0];
	pPushStreamAudioRAW =(CPushStream*)m_paStreams[1];
	pPushStreamMultiplex=(CPushStream*)m_paStreams[2];
	pPushStreamVideo    =(CPushStream*)m_paStreams[3];
	pPushStreamAudio    =(CPushStream*)m_paStreams[4];

    InitSockets();

    gTotalChannelCount=-1;
    for(int i=0;i<MAX_LIST_ITEM;i++)
        {
        gChannelNameList[i]=NULL;
        gChannelIDList[i]=0;
        }

    lstrcpy(m_DBOXAddress,"192.168.0.13");
    lstrcpy(m_DBOXLogin,"root");
    lstrcpy(m_DBOXPassword,"dbox2");
    m_DBOXStopPlayback=1;

    m_DBOXPESStreamingPort=31338;
    m_DBOXPSStreamingPort=31337;
    m_DBOXHTTPPort=80;
    m_DBOXTelnetPort=23;

    m_StartSucceeded=TRUE;

    m_VPid=0;
    m_APid=0;


    LoadInternalData();
    val=-1;

    pHTMLCircularBuffer=new CCircularBuffer();
    pHTMLCircularBuffer->Initialize(512*1024, 1);

//    hr=RetrieveChannelList(m_DBOXAddress, m_DBOXHTTPPort, NULL, &val);
//    hr=GetChannel(m_DBOXAddress, m_DBOXHTTPPort, (unsigned long *)&val);

//    hr=SetChannel(m_DBOXAddress, m_DBOXHTTPPort, 8716309);
//    hr=RetrieveStreamInfo(&m_VideoWidth, &m_VideoHeight,&m_VideoBitrate, &m_IsAspectRatio4By3, m_DBOXAddress, m_DBOXHTTPPort);
//    hr=CheckBoxStatus(m_DBOXAddress, m_DBOXTelnetPort);

//    hr=GetChannel(m_DBOXAddress, m_DBOXHTTPPort, (unsigned long *)&val);
//    hr=GetChannelInfo(m_DBOXAddress, m_DBOXHTTPPort, (unsigned long)val, szData);
//    dprintf("ChannelInfo: %s",szData);

//    m_VideoWidth=352;
//    m_VideoHeight=288;

//!!BS nhttpd is really a bloody bastard ...
#if 0
    {
    HRESULT hr=CheckBoxStatus(m_DBOXAddress, m_DBOXTelnetPort);
    if (SUCCEEDED(hr))
        hr=ExecuteCommand("killall -HUP nhttpd;killall -9 inetd;sleep 2;/bin/nhttpd/;sleep 2;/sbin/inetd", m_DBOXLogin, m_DBOXPassword, m_DBOXAddress, m_DBOXTelnetPort, 0, -1);
//        hr=ExecuteCommand("killall -SIGQUIT nhttpd;sleep 1;kill -SIGQUIT `ps | grep inetd | sed 's/[a-zA-Z]//g'`;sleep 2;/bin/nhttpd", m_DBOXLogin, m_DBOXPassword, m_DBOXAddress, m_DBOXTelnetPort, 0, -1);
    }
#endif
//!!BS nhttpd is really a bloody bastard ...


    m_VideoWidth=0;
    m_VideoHeight=0;
    m_VideoBitrate=0;
    m_IsAspectRatio4By3=1;


} // (Constructor)

CDBOXIICapture::~CDBOXIICapture()
{
    if (pHTMLCircularBuffer)
        delete pHTMLCircularBuffer;

    SaveInternalData();
    DeInitSockets();
	g_Instance--;
}

//
// NonDelegatingQueryInterface
//
STDMETHODIMP CDBOXIICapture::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    // ----------------------------------------------------------
    if (riid == IID_IAMTVTuner)
        return GetInterface((IAMTVTuner *) this, ppv);
    else 
    if (riid == IID_IAMCrossbar)
        return GetInterface((IAMCrossbar *) this, ppv);
    else 
    if (riid == IID_IMediaSeeking)
        return GetInterface((IMediaSeeking *) this, ppv);
    else 
    // ----------------------------------------------------------
    if (riid == IID_IDBOXIICapture) 
        return GetInterface((IDBOXIICapture *) this, ppv);
    else 
    if (riid == IID_ISpecifyPropertyPages) 
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
	else
        return CSource::NonDelegatingQueryInterface(riid, ppv);
    // ----------------------------------------------------------
} 

STDMETHODIMP CDBOXIICapture::GetPages(CAUUID * pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_DBOXIICaptureProperty;
    return NOERROR;
} // GetPages

STDMETHODIMP CDBOXIICapture::setParameter(__int64 command, __int64 data1)
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
        case CMD_SETCHANNEL:
            m_VideoWidth=0;
            m_VideoHeight=0;
            m_VideoBitrate=0;
            m_IsAspectRatio4By3=1;
            hr=SetChannel(m_DBOXAddress, m_DBOXHTTPPort, (unsigned long)data1);
            if (SUCCEEDED(hr))
                hr=RetrieveStreamInfo(&m_VideoWidth, &m_VideoHeight,&m_VideoBitrate, &m_IsAspectRatio4By3, m_DBOXAddress, m_DBOXHTTPPort);
            break;

        case CMD_RESTARTNHTTPD:
            {
            HRESULT hr=CheckBoxStatus(m_DBOXAddress, m_DBOXTelnetPort);
            if (SUCCEEDED(hr))
                hr=ExecuteCommand("killall -9 thttpd;sleep 1;/bin/thttpd -C /etc/thttpd.conf", m_DBOXLogin, m_DBOXPassword, m_DBOXAddress, m_DBOXTelnetPort, 0, -1);
            break;
            }

        case CMD_AUDIOVIDEOPID:
            m_VPid=((data1&0xFFFFFFFF00000000)>>32);
            m_APid=(data1&0x00000000FFFFFFFF);
            hr=NOERROR;
            break;

        }
	return(hr);
}

STDMETHODIMP CDBOXIICapture::getParameter(__int64 command, __int64 *data1, __int64 *data2)
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
        case CMD_STATUS:
            {
            hr=CheckBoxStatus(m_DBOXAddress, m_DBOXTelnetPort);
            if (SUCCEEDED(hr))
                *data1=1;
            else
                *data1=0;
            }
            break;
        case CMD_GETCHANNELLIST:
            if (data2==NULL)
                return(E_POINTER);
            lstrcpy((LPTSTR)data1, "");
            if (*data2<0)
                {
                if (FAILED(CheckBoxStatus(m_DBOXAddress, m_DBOXTelnetPort)))
                    {
                    *data2=-1;
                    hr=E_FAIL;
                    break;
                    }
                }
            hr=RetrieveChannelList(m_DBOXAddress, m_DBOXHTTPPort, (char *)data1, data2);
            break;
        case CMD_GETCHANNEL:
            hr=GetChannel(m_DBOXAddress, m_DBOXHTTPPort, (unsigned long*)data1);
            break;

        case CMD_GETCHANNELINFO:
            if (data2==NULL)
                return(E_POINTER);
            hr=GetChannelInfo(m_DBOXAddress, m_DBOXHTTPPort, (unsigned long)(*data1), (char*)data2);
            break;

        case CMD_GETTIME:        
            break;

        case CMD_GETDATE:        
            break;

        case CMD_GETSYNCCOUNT:   
                
            if (data1==NULL)
                return(E_POINTER);
            if (CRemuxer==NULL)
                {
                *data1=0;
                return(NOERROR);
                }

            if (data2!=NULL)
                {
                if (CRemuxer->resync)
                    {
                    *data2=1;
                    if (!CRemuxer->gotVideo)
                        *data2|=0x80;
                    if (!CRemuxer->gotAudio)
                        *data2|=0x40;
                    }
                else
                    *data2=0;
                }

            if (m_State!=State_Running)
                {
                *data1=0;
                }
            else
                {
                //dprintf("Resyncs: %ld",CRemuxer->resyncs);
                *data1=(CRemuxer->resyncs - 1);
                if (*data1<0)
                    *data1=0;
                }

            hr=NOERROR;
            break;

        case CMD_GETBITRATE:
            if ((data1==NULL)||(data2==NULL))
                return(E_POINTER);
            if (CRemuxer==NULL)
                {
                *data1=0;
                *data2=0;
                return(NOERROR);
                }
            if (m_State!=State_Running)
                {
                *data1=0;
                *data2=0;
                }
            else
                {
                long currentTime=timeGetTime();
                long tDelta=currentTime-gLastAVBitrateRequest;
                __int64 deltaAudio=gTotalAudioDataCount-gLastAudioDataCount;
                __int64 deltaVideo=gTotalVideoDataCount-gLastVideoDataCount;
                if (deltaVideo<0)
                    deltaVideo=0;
                if (deltaAudio<0)
                    deltaAudio=0;
                gLastVideoDataCount=gTotalVideoDataCount;
                gLastAudioDataCount=gTotalAudioDataCount;
                gLastAVBitrateRequest=currentTime;
                if (tDelta>0)
                    {
                    __int64 brv=(deltaVideo*1000*8)/tDelta;
                    __int64 bra=(deltaAudio*1000*8)/tDelta;

                    *data1=bra;
                    *data2=brv;
                    hr=NOERROR;
                    }
                }
            hr=NOERROR;
            break;

        case CMD_GETASPECTRATIO: 
            if (data1==NULL)
                return(E_POINTER);
            hr=RetrieveStreamInfo(&m_VideoWidth, &m_VideoHeight,&m_VideoBitrate, &m_IsAspectRatio4By3, m_DBOXAddress, m_DBOXHTTPPort);
            if (SUCCEEDED(hr))
                *data1=m_IsAspectRatio4By3;
            else
                *data1=-1;
            hr=NOERROR;
            break;

        case CMD_GETWIDTHEIGHT:
            if ((data1==NULL)||(data2==NULL))
                return(E_POINTER);
            hr=RetrieveStreamInfo(&m_VideoWidth, &m_VideoHeight,&m_VideoBitrate, &m_IsAspectRatio4By3, m_DBOXAddress, m_DBOXHTTPPort);
            if (SUCCEEDED(hr))
                {
                *data1=m_VideoWidth;
                *data2=m_VideoHeight;
                }
            else
                {
                *data1=0;
                *data2=0;
                }
            hr=NOERROR;
            break;

        case CMD_GETAUDIOFREQ:  
            if (data1==NULL)
                return(E_POINTER);
            if (CRemuxer==NULL)
                {
                *data1=0;
                return(NOERROR);
                }
            if (m_State!=State_Running)
                {
                *data1=0;
                return(NOERROR);
                }
            *data1=CRemuxer->audio_frequency;
            hr=NOERROR;
            break;

         case CMD_GETEPG:
            if (data2==NULL)
                return(E_POINTER);
            hr=GetEPGInfo(m_DBOXAddress, m_DBOXHTTPPort, (char*)data1, (char*)data2);
            break;

        case CMD_AUDIOVIDEOPID:
            if ( (data1==NULL) || (data2==NULL) )
                return(E_POINTER);
            *data1=m_APid;
            *data2=m_VPid;
            break;

        }
	return(hr);
}

STDMETHODIMP CDBOXIICapture::Stop()
{
    HRESULT hr=NOERROR;

    hr=CSource::Stop();
    DeInitStreaming();
    if (m_StartSucceeded)
        ControlPlaybackOnDBOX(m_DBOXAddress,m_DBOXHTTPPort, 0);
//    hr=ExecuteCommand("killall -9 streampes", m_DBOXLogin, m_DBOXPassword, m_DBOXAddress, m_DBOXTelnetPort, 0, -1);

    m_VPid=0;
    m_APid=0;

    return(hr);
}

STDMETHODIMP CDBOXIICapture::Pause()
{
    int mMode=0;    // 0:PES streaming (internal mux), 1:PS streaming (dbox delivers multiplexed stream, nyi ...)
    HRESULT hr=NOERROR;

    if (m_State==State_Stopped)
        {
        int vpid=0;
        int apid=0;

        m_StartSucceeded=TRUE;

        if (pPushStreamVideo->IsConnected())
            gIsVideoConnected=TRUE;
        else
            gIsVideoConnected=FALSE;

        if (pPushStreamAudio->IsConnected())
            gIsAudioConnected=TRUE;
        else
            gIsAudioConnected=FALSE;

        if (pPushStreamMultiplex->IsConnected())
            gIsPSPinConnected=TRUE;
        else
            gIsPSPinConnected=FALSE;

        #if USE_DBOX
            {
            char dtxt[264];

            if ( (m_VPid!=0) && (m_APid!=0) )
                {
                hr=NOERROR;
                vpid=(int)m_VPid;
                apid=(int)m_APid;
                }
            else
                hr=RetrievePIDs( &vpid, &apid, m_DBOXAddress, m_DBOXHTTPPort);

            if (FAILED(hr))
                return(E_FAIL);

            if (mMode==0)
                {
                if (!gIsPSPinConnected)
                    {
                    if (!gIsVideoConnected)
                        vpid=0;
                    if (!gIsAudioConnected)
                        apid=0;
                    }
                }

            //!!BS oops, there must be something wrong ...
            if ((vpid==0)&&(apid==0))
                {
                OutputDebugString("No valid pid(s), ABORTED !\n");
                m_StartSucceeded=FALSE;
                return(E_FAIL);
                }

#if 0
            if (m_DBOXStopPlayback)
                hr=ControlPlaybackOnDBOX(m_DBOXAddress,m_DBOXHTTPPort, 2);
            else
                hr=ControlPlaybackOnDBOX(m_DBOXAddress,m_DBOXHTTPPort, 1);
            if (FAILED(hr))
                return(E_FAIL);
#endif

            hr=ExecuteCommand("killall -HUP controld;killall -HUP sectionsd", m_DBOXLogin, m_DBOXPassword, m_DBOXAddress, m_DBOXTelnetPort, 0, -1);
            if (FAILED(hr))
                return(E_FAIL);

            sprintf(dtxt,"VPID:%ld, APID:%ld\n",vpid,apid);
            OutputDebugString(dtxt);

            if (mMode==0)
                hr=InitStreaming(vpid, apid, m_DBOXAddress, m_DBOXPESStreamingPort);
            else
                hr=InitPSStreaming(vpid, apid, m_DBOXAddress, m_DBOXPSStreamingPort);
            if (FAILED(hr))
                return(E_FAIL);
            }
        #endif
        }

    hr=CSource::Pause();
    return(hr);
}

STDMETHODIMP CDBOXIICapture::Run(REFERENCE_TIME tStart)
{
    HRESULT hr=NOERROR;

    hr=CSource::Run(tStart);
    return(hr);
}

//
// Constructor
//
CPushStream::CPushStream(HRESULT *phr,
                         CDBOXIICapture *pParent,
                         LPCWSTR pPinName,
                         int sampleType) :
    CSourceStream(NAME("DBOXII Capture"),phr, pParent, pPinName)
{
    pCDBOXIICapture=pParent;
    m_SampleType=sampleType;
} // (Constructor)


//
// Destructor
//
CPushStream::~CPushStream()
{
} // (Destructor)


//
// NonDelegatingQueryInterface
//
STDMETHODIMP CPushStream::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (m_SampleType!=SAMPLE_TYPE_VRAW)
        return CSourceStream::NonDelegatingQueryInterface(riid, ppv);

    // ----------------------------------------------------------
    if (riid == IID_IAMStreamConfig)
        return GetInterface((IAMStreamConfig *) this, ppv);
	else
    if (riid == IID_IKsPropertySet)
        return GetInterface((IKsPropertySet *) this, ppv);
    else
    if (riid == IID_IAMDroppedFrames)
        return GetInterface((IAMDroppedFrames *) this, ppv);
    else
    if (riid == IID_IAMStreamControl)
        return GetInterface((IAMStreamControl *) this, ppv);
    else
    if (riid == IID_IAMVideoCompression)
        return GetInterface((IAMVideoCompression *) this, ppv);
    // ----------------------------------------------------------
	else
        return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
    // ----------------------------------------------------------
} 


//
// GetMediaType
//
HRESULT CPushStream::GetMediaType(int iPosition, CMediaType *pmt)
{
    GUID SubTypeGUID = MEDIASUBTYPE_None;

    if (iPosition < 0) 
        return E_INVALIDARG;

    if (iPosition > 0) 
        return VFW_S_NO_MORE_ITEMS;

// --------------------------------------------------------------------
    if (m_SampleType==SAMPLE_TYPE_VRAW)
	    {
        VIDEOINFO *pvo = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
        if (NULL == pvo) 
		    {
		    return(E_OUTOFMEMORY);
		    }

        ZeroMemory(pvo, sizeof(VIDEOINFO));

        switch(iPosition)
            {
            case 0:
                pvo->bmiHeader.biCompression = MAKEFOURCC('Y','U','Y','2');
                pvo->bmiHeader.biBitCount    = 16;
                SubTypeGUID = MEDIASUBTYPE_YUY2;
                break;
            default:
                return E_UNEXPECTED;
            }

        pvo->AvgTimePerFrame=400000;

        pvo->bmiHeader.biSize		 = sizeof(BITMAPINFOHEADER);
        pvo->bmiHeader.biWidth		 = pCDBOXIICapture->m_VideoWidth;
        pvo->bmiHeader.biHeight		 = pCDBOXIICapture->m_VideoHeight;
        pvo->bmiHeader.biPlanes		 = 1;
        pvo->bmiHeader.biSizeImage	 = GetBitmapSize(&pvo->bmiHeader);
        pvo->bmiHeader.biClrImportant= 0;

        pmt->SetType(&MEDIATYPE_Video);
        pmt->SetFormatType(&FORMAT_VideoInfo);
        pmt->SetTemporalCompression(FALSE);

        // Work out the GUID for the subtype from the header info.
        pmt->SetSubtype(&SubTypeGUID);
        pmt->SetSampleSize(pvo->bmiHeader.biSizeImage);
        return NOERROR;
	    }
// ----------------------------------------------



    if (m_SampleType==SAMPLE_TYPE_PS)
        {
        pmt->SetType(&MEDIATYPE_Stream);
        pmt->SetFormatType(&FORMAT_MPEG2_VIDEO);
        pmt->SetSubtype(&MEDIASUBTYPE_MPEG2_PROGRAM);
        }
    else
    if (m_SampleType==SAMPLE_TYPE_VPES)
        {
        pmt->SetType(&MEDIATYPE_Video);
        pmt->SetFormatType(&FORMAT_MPEG2_VIDEO);
        pmt->SetSubtype(&MEDIASUBTYPE_MPEG2_VIDEO);
        }
    else
    if (m_SampleType==SAMPLE_TYPE_APES)
        {
        MPEG1WAVEFORMAT *pwf = (MPEG1WAVEFORMAT *)pmt->AllocFormatBuffer(sizeof(MPEG1WAVEFORMAT));

	    pwf->wfx.wFormatTag=WAVE_FORMAT_MPEG;
	    pwf->wfx.nChannels=2;
	    pwf->wfx.nSamplesPerSec =48000;
	    pwf->wfx.wBitsPerSample=16;
	    pwf->wfx.nBlockAlign=1;
	    pwf->wfx.nAvgBytesPerSec=192000/8;
	    pwf->wfx.cbSize=0;

	    pwf->fwHeadLayer=ACM_MPEG_LAYER2;
        pwf->dwHeadBitrate=192000;
        pwf->fwHeadMode=0;
        pwf->fwHeadModeExt=0;
        pwf->wHeadEmphasis=0;
        pwf->fwHeadFlags=0;

        pmt->SetType(&MEDIATYPE_Audio);
        pmt->SetFormatType(&FORMAT_MPEG2Audio);
        pmt->SetSubtype(&MEDIASUBTYPE_MPEG2_AUDIO);
        }

    pmt->SetTemporalCompression(FALSE);
    
    return NOERROR;
} // GetMediaType


//
// CheckMediaType
//
HRESULT CPushStream::CheckMediaType(const CMediaType *pMediaType)
{
    if (!IsValidMediaType(pMediaType))
        return(E_FAIL);

    return S_OK;  // This format is acceptable.
} // CheckMediaType

//
// SetMediaType
//
// Called when a media type is agreed between filters
//
HRESULT CPushStream::SetMediaType(const CMediaType *pMediaType)
{
    if (!IsValidMediaType(pMediaType))
        return(E_FAIL);

    HRESULT hr = CSourceStream::SetMediaType(pMediaType);
    return hr;
} // SetMediaType


BOOL CPushStream::IsValidMediaType(const CMediaType *pMediaType)
{

    if (m_SampleType==SAMPLE_TYPE_VRAW)
        {
        if (IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video)) 
            if (*pMediaType->FormatType() == FORMAT_VideoInfo)
                if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_YUY2)) 
                    {
                    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pMediaType->Format();
                    if (
                        (pvi->bmiHeader.biWidth  == pCDBOXIICapture->m_VideoWidth) &&
                        (pvi->bmiHeader.biHeight == pCDBOXIICapture->m_VideoHeight)
                       )
                    return(TRUE);                
                    }
        return(FALSE);
        }

    if (IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Stream)) 
		{
        if (m_SampleType==SAMPLE_TYPE_PS)
            {
            if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_MPEG2_PROGRAM)) 
                return(TRUE);
            }
        }
    else
    if (IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video)) 
		{
        if (m_SampleType==SAMPLE_TYPE_VPES)
            {
            if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_MPEG2_VIDEO)) 
                return(TRUE);
            }
        }
    else
    if (IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Audio)) 
		{
        if (m_SampleType==SAMPLE_TYPE_APES)
            {
            if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_MPEG2_AUDIO)) 
                return(TRUE);
            }

		}
    return(FALSE);
} 


//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//
HRESULT CPushStream::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cbAlign  = 1;
    pProperties->cbPrefix = 0;
    //pProperties->cBuffers = BUFFER_COUNT; 
    //!!BS of cause we assume here larger VIDEO BufferSize ...
    //!!BS factor of 2 just for safety ...

    if (m_SampleType==SAMPLE_TYPE_VRAW)
        {
        pProperties->cBuffers = 1;
        pProperties->cbBuffer = m_mt.GetSampleSize();
        }
    else
        {
        pProperties->cBuffers = BUFFER_COUNT;
        pProperties->cbBuffer = (AUDIO_BUFFER_SIZE+VIDEO_BUFFER_SIZE);
        } 

    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted
    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

    // Is this allocator unsuitable

    if (Actual.cbBuffer < pProperties->cbBuffer) {
        return E_FAIL;
    }

    return NOERROR;

} // DecideBufferSize


//
// OnThreadCreate
//
// As we go active reset the stream time to zero
//
HRESULT CPushStream::OnThreadCreate()
{
    m_rtSampleTime = 0;
    m_Abort=FALSE;
    //!!BS just wait for a while to acuumulate some incomming data ...
#if USE_DBOX
    Sleep(3500);
#endif
    return NOERROR;
} // OnThreadCreate

HRESULT CPushStream::OnThreadDestroy()
{
    m_Abort=TRUE;
    return NOERROR;
}

STDMETHODIMP CPushStream::Notify(IBaseFilter * pSender, Quality q)
{
    return NOERROR;
} // Notify

//
// FillBuffer
//
HRESULT CPushStream::FillBuffer(IMediaSample *pms)
{
    BYTE *pData=NULL;
    long lMaxDataLen=0;
    HRESULT hr=E_FAIL;
    int errorCount=2000;
    long tStart=timeGetTime();
    long tEnd=tStart;

    if (gfThreadAborted||(!pCDBOXIICapture->m_StartSucceeded))
        {
        m_pFilter->NotifyEvent(ERROR_NOTIFICATION, E_FAIL, 0);
        m_Abort=TRUE;
        }


    if (m_Abort)
        {
        pms->SetActualDataLength(0);
        Sleep(5);
        return(E_FAIL);
        }

    pms->GetPointer(&pData);
    lMaxDataLen = pms->GetSize();

    if (m_SampleType==SAMPLE_TYPE_VRAW)
        {
        CreateTestPattern(pData, pCDBOXIICapture->m_VideoWidth, pCDBOXIICapture->m_VideoHeight,(int)((m_rtSampleTime/400000)%256i64));

        LONGLONG rmstart=0;
        LONGLONG rmstop =0;
        LONGLONG rtStart=m_rtSampleTime;

        pms->SetActualDataLength(pCDBOXIICapture->m_VideoWidth * pCDBOXIICapture->m_VideoHeight * 2);

        m_rtSampleTime+=400000;

        pms->SetTime(&rtStart,&m_rtSampleTime);
        rmstart=rtStart/400000;
        rmstop =rmstart+1; //2*rmstart+1;
        //rmstart=0;
        //rmstop =0;
        pms->SetMediaTime(&rmstart,&rmstop);
        pms->SetDiscontinuity(FALSE);
        pms->SetPreroll(FALSE);
        pms->SetSyncPoint(TRUE);
        //dprintf("delivered: %ld",rtStart/10000);
        /*
        tEnd=timeGetTime();
        if ((tEnd-tStart)<40)
            Sleep(40-(tEnd-tStart));
        */
        return(NOERROR);
        }


    pms->SetActualDataLength(0);

    //!!BS this should always be true, otherwise DecideBufferSize would throw an error
    //!!BS of cause we assume here larger VIDEO BufferSize ...
    if (lMaxDataLen>=(AUDIO_BUFFER_SIZE+VIDEO_BUFFER_SIZE)) 
        {
        if (m_SampleType==SAMPLE_TYPE_PS)
            {
            if (CMultiplexBuffer!=NULL)
                {
                while (!CMultiplexBuffer->canRead(0, (AUDIO_BUFFER_SIZE+VIDEO_BUFFER_SIZE)))
                    {
                    Sleep(5);
                    if ((errorCount--<=0)||gfThreadAborted)
                        m_Abort=TRUE;
                    if (m_Abort)
                        {
                        m_pFilter->NotifyEvent(ERROR_NOTIFICATION, E_FAIL, 0);
                        return(NOERROR);
                        }
                    }

                {
                LONGLONG ts=0, te=0;
                LONGLONG ms=0, me=0;
                ts=CRemuxer->m_framePTS;
                te=ts;
                pms->SetTime(&ts,&te);
                pms->SetMediaTime(&ms, &me);
                }

                CMultiplexBuffer->Read(0, pData, (AUDIO_BUFFER_SIZE+VIDEO_BUFFER_SIZE));
			    pms->SetActualDataLength((AUDIO_BUFFER_SIZE+VIDEO_BUFFER_SIZE));
                pms->SetSyncPoint(TRUE);
                }
            }
        else
        if (m_SampleType==SAMPLE_TYPE_VPES)
            {
            if (CMultiplexBuffer!=NULL)
                {
                while (!CMultiplexBuffer->canRead(0, VIDEO_BUFFER_SIZE))
                    {
                    Sleep(5);
                    if ((errorCount--<=0)||gfThreadAborted)
                        m_Abort=TRUE;
                    if (m_Abort)
                        {
                        m_pFilter->NotifyEvent(ERROR_NOTIFICATION, E_FAIL, 0);
                        return(NOERROR);
                        }
                    }
                CMultiplexBuffer->Read(0, pData, VIDEO_BUFFER_SIZE);
			    pms->SetActualDataLength(VIDEO_BUFFER_SIZE);
                pms->SetSyncPoint(TRUE);
                }
            }
        else
        if (m_SampleType==SAMPLE_TYPE_APES)
            {
            if (CMultiplexBuffer!=NULL)
                {
                while (!CMultiplexBuffer->canRead(0, AUDIO_BUFFER_SIZE))
                    {
                    Sleep(5);
                    if ((errorCount--<=0)||gfThreadAborted)
                        m_Abort=TRUE;
                    if (m_Abort)
                        {
                        m_pFilter->NotifyEvent(ERROR_NOTIFICATION, E_FAIL, 0);
                        return(NOERROR);
                        }
                    }
                CMultiplexBuffer->Read(0, pData, AUDIO_BUFFER_SIZE);
			    pms->SetActualDataLength(AUDIO_BUFFER_SIZE);
                pms->SetSyncPoint(TRUE);
                }
            }
        }
    
    return(NOERROR);

} // FillBuffer