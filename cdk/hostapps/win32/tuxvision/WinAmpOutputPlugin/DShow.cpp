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
#include <streams.h>
#include <strmif.h>

#include <initguid.h>

#include "debug.h"
#include "..\\TuxVision\\AX_Helper.h"
#include "..\\TuxVision\\guids.h"
#include "..\\render\\interface.h"
#include "..\\capture\\ccircularbuffer.h"
#include "dshow.h"

//---------------------------------------------------------
// Collection of interfaces
//---------------------------------------------------------
IGraphBuilder *pGraphBuilder    = NULL;
IBaseFilter   *pPushSourceAudio = NULL;
IBaseFilter   *pAudioCompressor = NULL;
IBaseFilter   *pMultiplexor     = NULL;
IBaseFilter   *pRenderer        = NULL;
IBaseFilter   *pAudioResampler  = NULL;

CCircularBuffer *pCircularBuffer= NULL;

__int64 g_TotalByteTransmitted=0;
int gInterruptStreaming=0;
int gWaitForCompletion=0;
int gMaxTimeOut=0;

char    g_DBOXAddress[264];
char    g_DBOXLogin[264];
char    g_DBOXPassword[264];
int     g_IsENX;
int     g_DBOXStopPlayback;

extern "C" int __cdecl GetAudioData(BYTE *data, int *size);

//---------------------------------------------------------
//!!BS: EndOfStream ???
//
// CALLBACK for the AudioSourceFilter
//---------------------------------------------------------
int __cdecl GetAudioData(BYTE *data, int *dsize)
{
    if (gWaitForCompletion)
        return(E_FAIL);

    for(;;)
        {
        if (gInterruptStreaming)
            {
            *dsize=0;
            return(E_FAIL);
            }
        if (pCircularBuffer->canRead(0, AUDIO_CHUNK_SIZE))
            {
            pCircularBuffer->Read(0, data, AUDIO_CHUNK_SIZE);
            *dsize=AUDIO_CHUNK_SIZE;
            return(NOERROR);
            }
        else
            {
            if (gWaitForCompletion)
                {
                *dsize=0;
                return(E_FAIL);
                }
            }
        Sleep(0);
        }

	return(E_FAIL);
}

//---------------------------------------------------------
//
//---------------------------------------------------------
HRESULT InitGraph(int AudioSampleFrequency, int AudioChannels, int AudioBitsPerSample, int AudioBitrate)
{ 
    IAudioResampler *pIAudioResampler=NULL;

	HRESULT hr=NOERROR;

    if (pGraphBuilder!=NULL)
        return(NOERROR);

    CoInitialize(NULL);

    gInterruptStreaming=0;
    gWaitForCompletion=0;
    gMaxTimeOut=0;

    pCircularBuffer=new CCircularBuffer();
    pCircularBuffer->Initialize(16384, 1);
    pCircularBuffer->Clear();

    // Graphbuilder instance
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraphBuilder);

	if (SUCCEEDED(hr))
        hr=LoadFilterByCLSID(pGraphBuilder, CLSID_PushSource, &pPushSourceAudio, L"PushSource Audio");

	if (SUCCEEDED(hr))
		{
		IPushSource *pIPushSource=NULL;
		hr=pPushSourceAudio->QueryInterface(IID_PushSource,(void **)&pIPushSource);
		if (SUCCEEDED(hr))
			{
            int tmp;
			pIPushSource->setParameter(CMD_CALLBACK,(void *)GetAudioData);
            tmp=TYPE_AUDIO;pIPushSource->setParameter(CMD_TYPE,(void *)&tmp);
            tmp=AudioSampleFrequency;pIPushSource->setParameter(CMD_AUDIO_FREQUENCY,(void *)&tmp);
            tmp=AudioChannels;pIPushSource->setParameter(CMD_AUDIO_CHANNELS,(void *)&tmp);
            tmp=AudioBitsPerSample;pIPushSource->setParameter(CMD_AUDIO_BITSPERSAMPLE,(void *)&tmp);
			pIPushSource->Release();
			}
		}


    if (SUCCEEDED(hr))
        hr=LoadFilterByCLSID(pGraphBuilder, CLSID_AudioResampler, &pAudioResampler, L"AudioResampler");

    if (SUCCEEDED(hr))
        hr=pAudioResampler->QueryInterface(IID_AudioResampler, (void **)&pIAudioResampler);
    if (SUCCEEDED(hr))
        hr=pIAudioResampler->setParameter(CMD_DESTINATION_FREQUENCY, 48000);


    if (SUCCEEDED(hr))
        hr=LoadFilterByCLSID(pGraphBuilder, CLSID_WaveToMPA, &pAudioCompressor, L"AudioCompressor");
	if (SUCCEEDED(hr))
		{
        IWaveToMPA *pIWaveToMPA;
		hr=pAudioCompressor->QueryInterface(IID_WaveToMPA,(void **)&pIWaveToMPA);
		if (SUCCEEDED(hr))
			{
			pIWaveToMPA->setParameter(CMD_BITRATE,AudioBitrate);
			pIWaveToMPA->Release();
			}
		}

	if (SUCCEEDED(hr))
        hr=LoadFilterByCLSID(pGraphBuilder, CLSID_DBOXIIRender, &pRenderer, L"Renderer");
/*
		if (SUCCEEDED(hr))
			{
			IDBOXIIRender *pSink=NULL;
			hr=pRenderer->QueryInterface(IID_IDBOXIIRender,(void **)&pSink);
			if (SUCCEEDED(hr))
				{
				pSink->setParameter(CMD_CALLBACK,(void *)PutData);
				pSink->Release();
				}
			}
*/

    if (SUCCEEDED(hr))
        hr=LoadFilterByCLSID(pGraphBuilder, CLSID_DBOXIIMUX, &pMultiplexor, L"Multiplexor");


    if (SUCCEEDED(hr))
        hr=TryConnectingFilters(pGraphBuilder, pPushSourceAudio, 0, pAudioResampler, 0, FALSE);
    if (SUCCEEDED(hr))
        hr=TryConnectingFilters(pGraphBuilder, pAudioResampler, 0, pAudioCompressor, 0, TRUE);
    if (SUCCEEDED(hr))
        hr=TryConnectingFilters(pGraphBuilder, pAudioCompressor, 0, pMultiplexor, 1, TRUE);
    if (SUCCEEDED(hr))
        hr=TryConnectingFilters(pGraphBuilder, pMultiplexor, 0, pRenderer, 1, TRUE);


    {
	IMediaFilter    *pMediaFilter   = NULL;
    if (pGraphBuilder != NULL) 
		hr = pGraphBuilder->QueryInterface(IID_IMediaFilter, (void**) &pMediaFilter);
	if (SUCCEEDED(hr)&&(pMediaFilter!=NULL))
		pMediaFilter->SetSyncSource(NULL);
	RELEASE(pMediaFilter);
    }


    if (SUCCEEDED(hr))
        hr=RunGraph(pGraphBuilder);

    RELEASE(pIAudioResampler);

	return(hr);
}

HRESULT GetDeliveredData(__int64 *data)
{
    HRESULT hr=NOERROR;
    IDBOXIIRender *pSink=NULL;

    if (pRenderer==NULL)
        return(E_POINTER);

    hr=pRenderer->QueryInterface(IID_IDBOXIIRender, (void **)&pSink);

    if (SUCCEEDED(hr))
        hr=pSink->getParameter(CMD_DELIVEREDDATA, data, NULL);

    RELEASE(pSink);

    return(NOERROR);
}


HRESULT InjectData(BYTE *data, int size)
{
    gWaitForCompletion=0;
    for(;;)
        {
        if (gInterruptStreaming)
            return(E_FAIL);
        if (pCircularBuffer->canWrite(size))
            {
            pCircularBuffer->Write(data, size);
            return(NOERROR);
            }
        Sleep(2);
        }
}

HRESULT CheckIfStillPlaying(int *state)
{
	HRESULT	hr = E_FAIL;
    __int64 curVal = 0;
	IMediaFilter  *pMediaFilter  = NULL;
	IMediaControl *pMediaControl = NULL;
	OAFilterState pfs;


    *state=0;

    if (pGraphBuilder != NULL) 
	    hr=pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&pMediaControl);
	if (SUCCEEDED(hr)&&(pMediaControl!=NULL))
	    hr=pMediaControl->GetState(10000,&pfs);


	if (SUCCEEDED(hr))
		{
		if (pfs==State_Running)
            {
            *state=1;
            gWaitForCompletion=1;
            }
		}

    GetDeliveredData(&curVal);
    if (curVal>g_TotalByteTransmitted)
        gMaxTimeOut=0;
    else
        gMaxTimeOut++;
    g_TotalByteTransmitted=curVal;

    if (gMaxTimeOut>10)
        {
        *state=0;
        gWaitForCompletion=1;
        }

    if (pMediaControl!=NULL)
	    pMediaControl->Release();

    Sleep(100);

	return(hr);
}

//---------------------------------------------------------
//
//---------------------------------------------------------
HRESULT DeInitGraph(void)
{
    if (pGraphBuilder==NULL)
        return(E_POINTER);
    
    gInterruptStreaming=TRUE;
    Sleep(250);

    StopGraph(pGraphBuilder);

    //carefully release all filter
    RELEASE(pAudioCompressor);
    RELEASE(pMultiplexor);
	RELEASE(pRenderer);
	RELEASE(pPushSourceAudio);
	RELEASE(pGraphBuilder);

    DestroyGraph(pGraphBuilder);

    delete pCircularBuffer;
    pCircularBuffer=NULL;

    Sleep(5000);

	return(NOERROR);
}

HRESULT setConfiguration(void)
{
    __int64 val=0;
    HRESULT hr=NOERROR;
    IDBOXIIRender *pIDBOXIIRender=NULL;

    // Graphbuilder instance
	hr = CoCreateInstance(CLSID_DBOXIIRender, NULL, CLSCTX_INPROC_SERVER, IID_IDBOXIIRender, (void **)&pIDBOXIIRender);

    if (SUCCEEDED(hr))
        {
        val=(__int64)g_DBOXStopPlayback;
	    pIDBOXIIRender->setParameter(CMD_STOPPLAYBACK, val);
        val=(__int64)g_IsENX;
	    pIDBOXIIRender->setParameter(CMD_DECODERTYPE, val);
        pIDBOXIIRender->setParameter(CMD_IPADDRESS, (__int64)g_DBOXAddress);
        pIDBOXIIRender->setParameter(CMD_LOGIN, (__int64)g_DBOXLogin);
        pIDBOXIIRender->setParameter(CMD_PASSWORD, (__int64)g_DBOXPassword);
        }

    RELEASE(pIDBOXIIRender);

    return(hr);
}

HRESULT getConfiguration(void)
{
    __int64 val=0;
    HRESULT hr=NOERROR;
    IDBOXIIRender *pIDBOXIIRender=NULL;

    CoInitialize(NULL);

    // Graphbuilder instance
	hr = CoCreateInstance(CLSID_DBOXIIRender, NULL, CLSCTX_INPROC_SERVER, IID_IDBOXIIRender, (void **)&pIDBOXIIRender);

    if (SUCCEEDED(hr))
        {
	    pIDBOXIIRender->getParameter(CMD_STOPPLAYBACK, &val, NULL);
        g_DBOXStopPlayback=(int)val;
	    pIDBOXIIRender->getParameter(CMD_DECODERTYPE, &val, NULL);
        g_IsENX=(int)val;
        pIDBOXIIRender->getParameter(CMD_IPADDRESS, (__int64 *)g_DBOXAddress, NULL);
        pIDBOXIIRender->getParameter(CMD_LOGIN, (__int64 *)g_DBOXLogin, NULL);
        pIDBOXIIRender->getParameter(CMD_PASSWORD, (__int64 *)g_DBOXPassword, NULL);
        }

    RELEASE(pIDBOXIIRender);

    return(hr);
}
