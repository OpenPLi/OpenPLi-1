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

#define SAMPLE_TYPE_PS      0
#define SAMPLE_TYPE_VPES    1
#define SAMPLE_TYPE_APES    2
#define SAMPLE_TYPE_VRAW    3
#define SAMPLE_TYPE_ARAW    4

// The class managing the output pin
class CPushStream;

class CProperties : public CBasePropertyPage
{
public:
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    // Overrides from CBasePropertyPage
    HRESULT OnConnect(IUnknown * punk);
    HRESULT OnDisconnect(void);
    HRESULT OnDeactivate(void);
    CProperties(LPUNKNOWN lpunk, HRESULT *phr);

private:
    BOOL OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    IDBOXIICapture   *m_pIDBOXIICapture;       
};  


// Main object for filter
class CDBOXIICapture : public CSource,
  				    public ISpecifyPropertyPages,
                    public IDBOXIICapture,
                    public IAMTVTuner, 
                    public IAMCrossbar,
                  	public IMediaSeeking
{
public:
	DECLARE_IUNKNOWN
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP GetPages(CAUUID * pPages);

    STDMETHODIMP setParameter(__int64 command, __int64  data1);
    STDMETHODIMP getParameter(__int64 command, __int64 *data1, __int64 *data2);

    STDMETHODIMP Stop(void);
    STDMETHODIMP Pause(void);
    STDMETHODIMP Run(REFERENCE_TIME tStart);


	// IAMCrossbar
	STDMETHODIMP CanRoute (long OutputPinIndex,long InputPinIndex); 
	STDMETHODIMP get_CrossbarPinInfo (BOOL IsInputPin, long PinIndex, long * PinIndexRelated, long * PhysicalType); 
	STDMETHODIMP get_IsRoutedTo (long OutputPinIndex, long * InputPinIndex);
	STDMETHODIMP get_PinCounts(long * OutputPinCount, long * InputPinCount); 
	STDMETHODIMP Route (long OutputPinIndex, long InputPinIndex); 

    // IAMTVTuner
    STDMETHODIMP AutoTune(long lChannel, long * plFoundSignal);
    STDMETHODIMP ChannelMinMax(long *lChannelMin, long *lChannelMax);
    STDMETHODIMP get_AudioFrequency(long *lFreq);
    STDMETHODIMP get_AvailableTVFormats(long *lAnalogVideoStandard);
    STDMETHODIMP get_ConnectInput (long *plIndex);
    STDMETHODIMP get_Channel(long *lChannel, long *lVideoSubChannel, long *lAudioSubChannel);
    STDMETHODIMP get_CountryCode (long * plCountryCode);
    STDMETHODIMP get_InputType (long lIndex, TunerInputType * pInputType);
    STDMETHODIMP get_NumInputConnections(long * plNumInputConnections);
    STDMETHODIMP get_TuningSpace(long * plTuningSpace);
    STDMETHODIMP get_TVFormat(long * plAnalogVideoStandard );
    STDMETHODIMP get_VideoFrequency(long *lFreq);
    STDMETHODIMP put_Channel(long lChannel, long lVideoSubChannel, long lAudioSubChannel);
    STDMETHODIMP put_ConnectInput(long lIndex);
    STDMETHODIMP put_CountryCode(long lCountryCode);
    STDMETHODIMP put_InputType(long lIndex,TunerInputType InputType);
    STDMETHODIMP put_TuningSpace(long lTuningSpace);
    STDMETHODIMP StoreAutoTune();

    // IAMTuner
    STDMETHODIMP Logon(void *pLog);
    STDMETHODIMP Logout(void);
    STDMETHODIMP SignalPresent(long *plSignal);
    STDMETHODIMP put_Mode(enum tagAMTunerModeType Mode);
    STDMETHODIMP get_Mode(enum tagAMTunerModeType *pMode);
    STDMETHODIMP GetAvailableModes(long *lModes);
    STDMETHODIMP RegisterNotificationCallBack(struct IAMTunerNotification *pNotify,long Param);
    STDMETHODIMP UnRegisterNotificationCallBack(struct IAMTunerNotification *pNotify);
    
    // IMediaSeeking
	STDMETHODIMP GetCapabilities(unsigned long *);
	STDMETHODIMP CheckCapabilities(unsigned long *);
	STDMETHODIMP IsFormatSupported(const struct _GUID *);
	STDMETHODIMP QueryPreferredFormat(struct _GUID *);
	STDMETHODIMP GetTimeFormatA(struct _GUID *);
	STDMETHODIMP IsUsingTimeFormat(const struct _GUID *);
	STDMETHODIMP SetTimeFormat(const struct _GUID *);
	STDMETHODIMP GetDuration(__int64 *);
	STDMETHODIMP GetStopPosition(__int64 *);
	STDMETHODIMP GetCurrentPosition(__int64 *);
	STDMETHODIMP ConvertTimeFormat(__int64 *,const struct _GUID *,__int64,const struct _GUID *);
	STDMETHODIMP SetPositions(__int64 *,unsigned long,__int64 *,unsigned long);
	STDMETHODIMP GetPositions(__int64 *,__int64 *);
	STDMETHODIMP GetAvailable(__int64 *,__int64 *);
	STDMETHODIMP SetRate(double);
	STDMETHODIMP GetRate(double *);
	STDMETHODIMP GetPreroll(__int64 *);


    char    m_DBOXAddress[264];
    char    m_DBOXLogin[264];
    char    m_DBOXPassword[264];
    int     m_DBOXPESStreamingPort;
    int     m_DBOXPSStreamingPort;
    int     m_DBOXHTTPPort;
    int     m_DBOXTelnetPort;
    int     m_DBOXStopPlayback;

    int     m_VideoWidth;
    int     m_VideoHeight;
    int     m_VideoBitrate;
    int     m_IsAspectRatio4By3;

    int     m_AudioSampleFrequency;

    int     m_StartSucceeded;

    __int64 m_VPid;
    __int64 m_APid;

    // It should (is) only allowed to to create these objects with CreateInstance
    CDBOXIICapture(LPUNKNOWN lpunk, HRESULT *phr);
	~CDBOXIICapture();
    HRESULT SaveInternalData(void);
    HRESULT LoadInternalData(void);

private:
    
    CPushStream *pPushStreamVideoRAW;
    CPushStream *pPushStreamAudioRAW;
	CPushStream *pPushStreamVideo;
	CPushStream *pPushStreamAudio;
    CPushStream *pPushStreamMultiplex;


}; // CDBOXIICapture


// CPushStream manages the data flow from the output pin.
class CPushStream : public CSourceStream,
					public IAMStreamConfig,
					public IKsPropertySet,
                    public IAMDroppedFrames,
                    public IAMStreamControl,
                    public IAMVideoCompression

{
public:
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    DECLARE_IUNKNOWN

    CPushStream(HRESULT *phr, CDBOXIICapture *pParent, LPCWSTR pPinName, int sampleType);
    ~CPushStream();

    // plots a ball into the supplied video frame
    HRESULT FillBuffer(IMediaSample *pms);

    // Ask for buffers of the size appropriate to the agreed media type
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

    // Set the agreed media type, and set up the necessary parameters
    HRESULT SetMediaType(const CMediaType *pMediaType);

    // Because we calculate the ball there is no reason why we
    // can't calculate it in any one of a set of formats...
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);

    // Resets the stream time to zero
    HRESULT OnThreadCreate(void);
    HRESULT OnThreadDestroy(void);

    // Quality control notifications sent to us
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    // IAMStreamConfig
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE **pmt);
    STDMETHODIMP GetNumberOfCapabilities(int *piCount,  int *piSize);
    STDMETHODIMP GetStreamCaps(int iIndex,  AM_MEDIA_TYPE **pmt,  BYTE *pSCC); 
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt); 

    // IKSPropertySet
    STDMETHODIMP Get(REFGUID guidPropSet,  DWORD dwPropID,  LPVOID pInstanceData,  DWORD cbInstanceData,  LPVOID pPropData,  DWORD cbPropData,  DWORD *pcbReturned);
    STDMETHODIMP QuerySupported(REFGUID guidPropSet,  DWORD dwPropID,  DWORD *pTypeSupport);
    STDMETHODIMP Set(REFGUID guidPropSet,  DWORD dwPropID,  LPVOID pInstanceData,  DWORD cbInstanceData,  LPVOID pPropData,  DWORD cbPropData);

    // IAMDroppedFrames
    STDMETHODIMP GetAverageFrameSize(long *plAverageSize);
    STDMETHODIMP GetDroppedInfo(long lSize, long *plArray, long *plNumCopied);
    STDMETHODIMP GetNumDropped(long *plDropped);
    STDMETHODIMP GetNumNotDropped(long *plNotDropped);

    // IAMStreamControl
    STDMETHODIMP GetInfo(AM_STREAM_INFO *pInfo);
    STDMETHODIMP StartAt(const REFERENCE_TIME *ptStart, DWORD dwCookie);
    STDMETHODIMP StopAt(const REFERENCE_TIME *ptStop, BOOL bSendExtra, DWORD dwCookie);

	// IAMVideoCompression 
    STDMETHODIMP put_KeyFrameRate(long KeyFrameRate);
    STDMETHODIMP get_KeyFrameRate(long *pKeyFrameRate);
    STDMETHODIMP put_PFramesPerKeyFrame(long PFramesPerKeyFrame);
    STDMETHODIMP get_PFramesPerKeyFrame(long *pPFramesPerKeyFrame);
    STDMETHODIMP put_Quality(double Quality);
    STDMETHODIMP get_Quality(double *pQuality);
    STDMETHODIMP put_WindowSize(DWORDLONG WindowSize);
    STDMETHODIMP get_WindowSize(DWORDLONG *pWindowSize);
    STDMETHODIMP GetInfo(WCHAR *pszVersion, int *pcbVersion, LPWSTR pszDescription,int *pcbDescription, long *pDefaultKeyFrameRate, long *pDefaultPFramesPerKey, double *pDefaultQuality, long *pCapabilities);
    STDMETHODIMP OverrideKeyFrame(long FrameNumber);
    STDMETHODIMP OverrideFrameSize(long FrameNumber, long Size);




    int                 m_SampleType;
    LONGLONG            m_rtSampleTime;	        // The time stamp for each sample

private:

    BOOL IsValidMediaType(const CMediaType *pMediaType);

    BOOL     m_Abort;

    CDBOXIICapture  *pCDBOXIICapture;

}; // CPushStream
	
