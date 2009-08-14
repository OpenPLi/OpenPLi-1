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
#include <olectl.h>
#include <ks.h>
#include <ksmedia.h>
#include <dvdmedia.h>


#include "debug.h"
#include "interface.h"
#include "source.h"
#include "fsource.h"
#include "ccircularbuffer.h"
#include "Remuxer.h"
#include "grab.h"

// ----------------------------------------------------------------
//								IAMCrossbar
// ----------------------------------------------------------------
HRESULT CDBOXIICapture::CanRoute (long OutputPinIndex,long InputPinIndex)
{
    if ((OutputPinIndex==1) && (InputPinIndex==1))
        return(NOERROR);
    else
        return(E_FAIL);
}

HRESULT CDBOXIICapture::get_CrossbarPinInfo (BOOL IsInputPin, long PinIndex, long * PinIndexRelated, long * PhysicalType)
{
    if (PinIndex>=1)
        return(E_FAIL);

    *PinIndexRelated=0;
    *PhysicalType=PhysConn_Video_Tuner;

    return(NOERROR);
}

HRESULT CDBOXIICapture::get_IsRoutedTo (long OutputPinIndex, long * InputPinIndex)
{
    if (OutputPinIndex>=1)
        return(E_FAIL);
    
    *InputPinIndex=0;

    return(NOERROR);
}

HRESULT CDBOXIICapture::get_PinCounts(long * OutputPinCount, long * InputPinCount) 
{
    *OutputPinCount=1;
    *InputPinCount=1;
    return(E_FAIL);
}

HRESULT CDBOXIICapture::Route (long OutputPinIndex, long InputPinIndex)
{
    if ((OutputPinIndex==1) && (InputPinIndex==1))
        return(NOERROR);
    else
        return(E_FAIL);
}


// ----------------------------------------------------------------
//								IAMTVTuner
// ----------------------------------------------------------------
HRESULT CDBOXIICapture::AutoTune(long lChannel, long * plFoundSignal)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::ChannelMinMax(long *lChannelMin, long *lChannelMax)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_AudioFrequency(long *lFreq)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_AvailableTVFormats(long *lAnalogVideoStandard)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_ConnectInput (long *plIndex)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_Channel(long *lChannel, long *lVideoSubChannel, long *lAudioSubChannel)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_CountryCode (long * plCountryCode)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_InputType (long lIndex, TunerInputType * pInputType)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_NumInputConnections(long * plNumInputConnections)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_TuningSpace(long * plTuningSpace)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_TVFormat(long * plAnalogVideoStandard )
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_VideoFrequency(long *lFreq)
{
  return(E_FAIL);
}

HRESULT CDBOXIICapture::put_Channel(long lChannel, long lVideoSubChannel, long lAudioSubChannel)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::put_ConnectInput(long lIndex)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::put_CountryCode(long lCountryCode)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::put_InputType(long lIndex,TunerInputType InputType)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::put_TuningSpace(long lTuningSpace)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::StoreAutoTune()
{
    return(E_NOTIMPL);
}


// ----------------------------------------------------------------
//							   IAMTuner
// ----------------------------------------------------------------

HRESULT CDBOXIICapture::Logon(void *pLog)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::Logout(void)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::SignalPresent(long *plSignal)
{		
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::put_Mode(enum tagAMTunerModeType Mode)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::get_Mode(enum tagAMTunerModeType *pMode)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::GetAvailableModes(long *lModes)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::RegisterNotificationCallBack(struct IAMTunerNotification *pNotify,long Param)
{
    return(E_NOTIMPL);
}

HRESULT CDBOXIICapture::UnRegisterNotificationCallBack(struct IAMTunerNotification *pNotify)
{
    return(E_NOTIMPL);
}

// ----------------------------------------------------------------
//							   IMediaSeeking
// ----------------------------------------------------------------

HRESULT CDBOXIICapture::GetCapabilities(unsigned long *cap)
{
	dprintf("GetCapabilities"); 
	*cap=AM_SEEKING_CanGetCurrentPos;
	return(S_OK);  
}

HRESULT CDBOXIICapture::CheckCapabilities(unsigned long *cap)
{
	dprintf("CheckCapabilities"); 
	if (*cap==AM_SEEKING_CanGetCurrentPos)
		return(S_OK);  
	return(S_FALSE);  
}

HRESULT CDBOXIICapture::IsFormatSupported(const struct _GUID *fmt)
{
	dprintf("IsFormatSupported"); 
	if (IsEqualGUID(*fmt,TIME_FORMAT_MEDIA_TIME))
		return(S_OK);
	return(S_FALSE);  
}

HRESULT CDBOXIICapture::QueryPreferredFormat(struct _GUID *fmt)
{
	*fmt=TIME_FORMAT_MEDIA_TIME;
	return(S_OK);  
}

HRESULT CDBOXIICapture::GetTimeFormatA(struct _GUID *fmt)
{
	*fmt=TIME_FORMAT_MEDIA_TIME;
	return(S_OK);  
}

HRESULT CDBOXIICapture::IsUsingTimeFormat(const struct _GUID *fmt)
{
	if (IsEqualGUID(*fmt,TIME_FORMAT_MEDIA_TIME))
		return(S_OK);
	return(S_FALSE);  
}

HRESULT CDBOXIICapture::SetTimeFormat(const struct _GUID *fmt)
{
	if (IsEqualGUID(*fmt,TIME_FORMAT_MEDIA_TIME))
		return(S_OK);
	return(S_FALSE);  
}

HRESULT CDBOXIICapture::GetDuration(__int64 *duration)
{
	return(E_NOTIMPL);  
}

HRESULT CDBOXIICapture::GetStopPosition(__int64 *pos)
{
	*pos=pPushStreamVideoRAW->m_rtSampleTime;
	return(S_OK);  
}

HRESULT CDBOXIICapture::GetCurrentPosition(__int64 *pos)
{
	dprintf("GetCurrentPosition"); 
	*pos=pPushStreamVideoRAW->m_rtSampleTime;
	return(S_OK);
}

HRESULT CDBOXIICapture::ConvertTimeFormat(__int64 *,const struct _GUID *,__int64,const struct _GUID *)
{
	return(E_NOTIMPL);  
}

HRESULT CDBOXIICapture::SetPositions(__int64 *,unsigned long,__int64 *,unsigned long)
{
	return(E_NOTIMPL);  
}

HRESULT CDBOXIICapture::GetPositions(__int64 *cur,__int64 *stop)
{
	*cur=pPushStreamVideoRAW->m_rtSampleTime;
	*stop=pPushStreamVideoRAW->m_rtSampleTime;
	dprintf("WaveOut::GetPositions"); 
	return(S_OK);  
}

HRESULT CDBOXIICapture::GetAvailable(__int64 *etime,__int64 *ltime)
{
	*etime=0;
	*ltime=0;
	return(S_OK);  
}

HRESULT CDBOXIICapture::SetRate(double)
{
	return(S_OK);  
}

HRESULT CDBOXIICapture::GetRate(double *rate)
{
	*rate=1;
	return(S_OK);  
}

HRESULT CDBOXIICapture::GetPreroll(__int64 *pre)
{
	*pre=0;
	return(S_OK);  
}

// ----------------------------------------------------------------
//							   IAMStreamConfig
// ----------------------------------------------------------------

HRESULT CPushStream::SetFormat(AM_MEDIA_TYPE *pmt)
{
    dprintf("IAMStreamConfig::SetFormat");

	if( pmt->majortype != MEDIATYPE_Video )
		return E_INVALIDARG;

	if( pmt->subtype != MEDIASUBTYPE_YUY2 )
		return E_INVALIDARG;

	if( pmt->formattype != FORMAT_VideoInfo ) 
		return E_INVALIDARG;

	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pmt->pbFormat;

    if (
        (pvi->bmiHeader.biWidth  == pCDBOXIICapture->m_VideoWidth) &&
        (pvi->bmiHeader.biHeight == pCDBOXIICapture->m_VideoHeight)
       )
    return(NOERROR);                

    return(E_INVALIDARG);
}


// What format is the capture card capturing right now?
// The caller must free it with DeleteMediaType(*ppmt)
//
HRESULT CPushStream::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    dprintf("IAMStreamConfig::GetFormat");

    if (ppmt == NULL)
	return E_POINTER;

    *ppmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (*ppmt == NULL)
	return E_OUTOFMEMORY;
    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));
    HRESULT hr = GetMediaType(0,(CMediaType *)*ppmt);
    if (hr != NOERROR) {
	CoTaskMemFree(*ppmt);
	*ppmt = NULL;
	return hr;
    }
    return NOERROR;
}


//
//
HRESULT CPushStream::GetNumberOfCapabilities(int *piCount, int *piSize)
{
    dprintf("IAMStreamConfig::GetNumberOfCapabilities");

    if (piCount == NULL || piSize == NULL)
	return E_POINTER;

    *piCount = 0;
    *piSize = 0;

    return NOERROR;
}


// find out some capabilities of this capture device
//
HRESULT CPushStream::GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt, LPBYTE pSCC)
{
    dprintf("IAMStreamConfig::GetStreamCaps");
    return E_NOTIMPL;
}

// ----------------------------------------------------------------
//							   IKSPropertySet
// ----------------------------------------------------------------
HRESULT CPushStream::Set(REFGUID guidPropSet,  DWORD dwPropID,  LPVOID pInstanceData,  DWORD cbInstanceData,  LPVOID pPropData,  DWORD cbPropData)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::Get(REFGUID guidPropSet,  DWORD dwPropID,  LPVOID pInstanceData,  DWORD cbInstanceData,  LPVOID pPropData,  DWORD cbPropData,  DWORD *pcbReturned)
{
    if (AMPROPSETID_Pin==guidPropSet)
        {
        if( dwPropID != AMPROPERTY_PIN_CATEGORY )
            return E_PROP_ID_UNSUPPORTED;
        if( pPropData == NULL && pcbReturned == NULL )
            return E_POINTER;
        if( pcbReturned )
            *pcbReturned = sizeof(GUID);
        if( pPropData == NULL )
            return S_OK;
        if( cbPropData < sizeof(GUID) )
            return E_UNEXPECTED;
        *(GUID *)pPropData = PIN_CATEGORY_CAPTURE;

        return(NOERROR);
        }

    return(E_FAIL);
}

HRESULT CPushStream::QuerySupported(REFGUID guidPropSet,  DWORD dwPropID,  DWORD *pTypeSupport)
{
	if( pTypeSupport )
        {
		*pTypeSupport = KSPROPERTY_SUPPORT_GET;
        return(NOERROR);
        }

    return(E_FAIL); 
}

// ----------------------------------------------------------------
//							   IAMDroppedFrames
// ----------------------------------------------------------------
HRESULT CPushStream::GetAverageFrameSize(long *plAverageSize)
{
    if (plAverageSize==NULL)
        return(E_POINTER);
    *plAverageSize=pCDBOXIICapture->m_VideoWidth*pCDBOXIICapture->m_VideoHeight*2;
    return(NOERROR);
}

HRESULT CPushStream::GetDroppedInfo(long lSize, long *plArray, long *plNumCopied)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::GetNumDropped(long *plDropped)
{
    if (plDropped==NULL)
        return(E_POINTER);
    *plDropped=0;
    return(NOERROR);
}

HRESULT CPushStream::GetNumNotDropped(long *plNotDropped)
{
    if (plNotDropped==NULL)
        return(E_POINTER);
    *plNotDropped=(long)(m_rtSampleTime/400000);
    return(NOERROR);
}


// ----------------------------------------------------------------
//							   IAMStreamControl
// ----------------------------------------------------------------
HRESULT CPushStream::GetInfo(AM_STREAM_INFO *pInfo)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::StartAt(const REFERENCE_TIME *ptStart, DWORD dwCookie)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::StopAt(const REFERENCE_TIME *ptStop, BOOL bSendExtra, DWORD dwCookie)
{
    return(E_NOTIMPL);
}


// ----------------------------------------------------------------
//							   IAMVideoCompression
// ----------------------------------------------------------------
HRESULT CPushStream::put_KeyFrameRate(long KeyFrameRate)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::get_KeyFrameRate(long *pKeyFrameRate)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::put_PFramesPerKeyFrame(long PFramesPerKeyFrame)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::get_PFramesPerKeyFrame(long *pPFramesPerKeyFrame)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::put_Quality(double Quality)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::get_Quality(double *pQuality)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::put_WindowSize(DWORDLONG WindowSize)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::get_WindowSize(DWORDLONG *pWindowSize)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::GetInfo(WCHAR *pszVersion, int *pcbVersion, LPWSTR pszDescription,int *pcbDescription, long *pDefaultKeyFrameRate, long *pDefaultPFramesPerKey, double *pDefaultQuality, long *pCapabilities)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::OverrideKeyFrame(long FrameNumber)
{
    return(E_NOTIMPL);
}

HRESULT CPushStream::OverrideFrameSize(long FrameNumber, long Size)
{
    return(E_NOTIMPL);
}
