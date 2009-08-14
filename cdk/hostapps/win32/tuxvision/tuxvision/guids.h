#ifndef __GUIDS_H__
#define __GUIDS_H__

// --------------------------------------------------------------------------
// MPEGVideoDecoder
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_PCLE_MPEGVideoDecoder2, 
0x4fed19d5, 0xcd87, 0x459e, 0x97, 0xe6, 0x56, 0x6b, 0xa0, 0xe2, 0xc6, 0x7f);

// --------------------------------------------------------------------------
// VideoDeInterlacer
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_DeInterlace, 
0x64dad9c2, 0x7d8a, 0x478f, 0x9b, 0xdc, 0xa2, 0x9e, 0xce, 0xdf, 0xec, 0x19);

DEFINE_GUID(IID_IDeInterlace, 
0x86aa9ce5, 0x5d13, 0x4c7e, 0x81, 0x97, 0x30, 0xf2, 0x2c, 0x25, 0x63, 0x50);

DECLARE_INTERFACE_(IDeInterlace,IUnknown)
{
STDMETHOD(setParameter) (THIS_ int command, DWORD value1, DWORD value2, DWORD *value3) PURE;
STDMETHOD(getParameter) (THIS_ int command, DWORD value1, DWORD value2, DWORD *value3) PURE;
};

#define CMD_DEINTERLACE_RGB24BITMAP 1
#define CMD_DEINTERLACE             2
#define CMD_TEMPORALFILTER          3

// --------------------------------------------------------------------------
// MPEGAudioDecoder
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_MPADecoder,
0xefc5a84, 0x7f47, 0x4ee7, 0xbf, 0xfb, 0xe8, 0x12, 0x90, 0x54, 0x18, 0x44);

// --------------------------------------------------------------------------
// ESink Video
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_Dump, 
0xe1beba1, 0xf026, 0x11d2, 0x94, 0xcd, 0x0, 0x80, 0x48, 0x83, 0x45, 0xe8);

// --------------------------------------------------------------------------
// ESink Audio
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_DumpAudio, 
0x301bd8e1, 0x1ceb, 0x11d3, 0xa9, 0xca, 0x0, 0xaa, 0x0, 0xc7, 0xef, 0x4);


// --------------------------------------------------------------------------
// AudioResampler
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_AudioResampler,
0xac2ad0f4, 0x2211, 0x49f2, 0x99, 0x85, 0x43, 0x74, 0x2d, 0x5f, 0x7, 0xd0);

DEFINE_GUID(IID_AudioResampler, 
0x8c1b48a1, 0xef8e, 0x4a60, 0x8a, 0xf3, 0x92, 0x35, 0xfa, 0xcd, 0x9b, 0x11);

DECLARE_INTERFACE_(IAudioResampler,IUnknown)
{
STDMETHOD(setParameter) (THIS_ int cmd, int  val) PURE;
STDMETHOD(getParameter) (THIS_ int cmd, int *val) PURE;
};

#define CMD_DESTINATION_FREQUENCY   1
#define CMD_SOURCE_FREQUENCY        2
#define CMD_FRAME_NOTIFY            3
#define CMD_FRAMEDURATION_NOTIFY    4

// --------------------------------------------------------------------------
// VideoRenderer
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_PinnacleVideoRenderer,
0x11c012c1, 0x5563, 0x11d3, 0x8d, 0x66, 0x0, 0xaa, 0x0, 0xa0, 0x18, 0x93);

// --------------------------------------------------------------------------
// WaveFileWriter
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_WaveOut,
0xcc01b761, 0xa537, 0x11d0, 0x9c, 0x71, 0x0, 0xaa, 0x0, 0x58, 0xa7, 0x35);

// --------------------------------------------------------------------------
// AV RawFileWriter
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_RAWWriter,
0x7c069d00, 0x777a, 0x11d3, 0xae, 0xe3, 0x0, 0x60, 0x8, 0x57, 0xee, 0xd8);

DEFINE_GUID(CLSID_SimpleSink,0x82febb53, 
0xbed7, 0x4e94, 0xb2, 0xa4, 0xc3, 0xa3, 0x47, 0x94, 0x95, 0x1b);

DEFINE_GUID(IID_ISimpleSink,
0x50a3b39a, 0x5b2c, 0x4684, 0xae, 0x9d, 0x6c, 0x5a, 0x77, 0x96, 0x88, 0xb6);

DECLARE_INTERFACE_(ISimpleSink,IUnknown)
{
    STDMETHOD_(ULONG,setSegmentSize) (ULONG ulSegmentSize) PURE;
    STDMETHOD(setDiskFreeArea) (LONGLONG lMinFreeSize) PURE;
    STDMETHOD_(ULONG,getSegmentSize) (void) PURE;
    STDMETHOD(getSegmentList) (void** pSegmentList) PURE;   //!!BS: slightly modified ;-)
    STDMETHOD(writeReferenceFile) (LPCTSTR pszFileName, REFERENCE_TIME rtRecStart, REFERENCE_TIME rtRecStop) PURE;
    STDMETHOD(setMessageHandler) (DWORD dwThreadId) PURE;
    STDMETHOD(createSegmentList) (void** pSegList) PURE;    //!!BS: slightly modified ;-)

    STDMETHOD(setMaxFileSize)  (THIS_ __int64 size, DWORD event) PURE;
    STDMETHOD(getCurrentFileSize)  (THIS_ __int64 *size) PURE;
};

// --------------------------------------------------------------------------
// MPEG1 LayerII AudioEncoder
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_WaveToMPA,
0xfc41cfc1, 0x4927, 0x11d3, 0xae, 0xe3, 0x0, 0x60, 0x8, 0x57, 0xee, 0xd8);

// {FC41CFC2-4927-11d3-AEE3-00600857EED8}
DEFINE_GUID(IID_WaveToMPA, 
0xfc41cfc2, 0x4927, 0x11d3, 0xae, 0xe3, 0x0, 0x60, 0x8, 0x57, 0xee, 0xd8);

DECLARE_INTERFACE_(IWaveToMPA,IUnknown)
{
STDMETHOD(setParameter) (THIS_ int command, int  val) PURE;
STDMETHOD(getParameter) (THIS_ int command, int *val) PURE;
};

#define CMD_BITRATE		1   // set/get bitrate in bit/sec
							// allowed parameters: (stereo allowed !)
							//  96000, 112000, 128000, 160000, 192000,
							// 224000, 256000, 320000, 384000
#define CMD_CRC         2   // 0:CRC off (default), 1:CRC on



// --------------------------------------------------------------------------
// MPEG1 LayerIII AudioEncoder 
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_WaveToMP3,
0x9b6f2536, 0x6a50, 0x4c16, 0xba, 0xf9, 0x87, 0x8b, 0x7d, 0xd7, 0x5d, 0x5a);

DEFINE_GUID(IID_WaveToMP3, 
0x8317535d, 0xa297, 0x4f59, 0x97, 0xaa, 0x59, 0x69, 0x10, 0x17, 0xcb, 0x2b);

DECLARE_INTERFACE_(IWaveToMP3,IUnknown)
{
STDMETHOD(setParameter) (THIS_ int command, int  val) PURE;
STDMETHOD(getParameter) (THIS_ int command, int *val) PURE;
};

#define CMD_BITRATE		1   // set/get bitrate in bit/sec
							// allowed parameters: (stereo allowed !)
							//  96000, 112000, 128000, 160000, 192000,
							// 224000, 256000, 320000, 384000

// --------------------------------------------------------------------------
// Multiplexer
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_DBOXIIMUX,
0xfad5b8a7, 0x1114, 0x4474, 0xa0, 0x3a, 0x12, 0xb3, 0x9b, 0xe8, 0xd1, 0x40);

// --------------------------------------------------------------------------
// DeMultiplexer
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_PCLE_DEMUX2, 
0xad119551, 0xe651, 0x4587, 0x82, 0x3e, 0xb0, 0xac, 0x99, 0x79, 0x84, 0xad);

DEFINE_GUID(IID_IPCLECommands, 
0xd9fb2240, 0x687a, 0x11d4, 0xbf, 0xb1, 0x0, 0x10, 0x5a, 0xa8, 0x5, 0xc4);

typedef enum 
{
    Cmd_GetVersion,
    Cmd_SetGrabFrame,
    Cmd_GetGrabFrame, 
    Cmd_EnableAdvancedFeatures,
    // Enable additional output pins.
    // Parameter:
    //       lparam1: BOOL bEnable
    //       lparam2: -
    //       pllparam3: -
    Cmd_GetPosition,
    // Set available seek range (needed for timeshift).
    // Parameter:
    //       lparam1: -
    //       lparam2: -
    //       pllparam3:  struct { LONGLONG llTimeStart, llTimeEnd; }
    Cmd_SetAvailable,
    // Set first available input file offset 
    // Parameter:
    //       pllparam3:  LONGLONG offset
    Cmd_SetFileStart,
    Cmd_SetSCRCorrection,
    Cmd_DebugOut,
    Cmd_GetPTSLateness,
    Cmd_SetNotifyThreadId,
    Cmd_TSSelectServiceType,
    Cmd_TSResetProgramInfo,
    Cmd_TSEnableTSOutput,
    Cmd_TSEnableMpegPSOutput,
    Cmd_GetOutputQueueLevels,
    Cmd_EnableGOPTimeFixNTSC,
    Cmd_SetDefaultAudioSampleFrequency,
    Cmd_GetDefaultAudioSampleFrequency,
} ECommand;

DECLARE_INTERFACE_(IPCLECommands, IUnknown)
{
   STDMETHOD(Command)  (ECommand cmd, LPARAM lParam1, LPARAM lParam2, LONGLONG* param3) PURE;     
};

// --------------------------------------------------------------------------
// Push Source/Sink
// --------------------------------------------------------------------------
DEFINE_GUID(CLSID_PushSink, 
0xadb57f8, 0x1c, 0x4166, 0x8c, 0x4b, 0x52, 0x81, 0x40, 0x6c, 0x43, 0xeb);

DEFINE_GUID(IID_PushSink, 
0x9552bddc, 0xec2c, 0x4ce2, 0x81, 0x8c, 0x1e, 0x44, 0x10, 0x26, 0x79, 0xc);

DECLARE_INTERFACE_(IPushSink,IUnknown)
{
STDMETHOD(setParameter) (THIS_ int command, void *data) PURE;
STDMETHOD(getParameter) (THIS_ int command, void *data) PURE;
};


DEFINE_GUID(CLSID_PushSource, 
0xc709a2a1, 0x343a, 0x11d1, 0xae, 0xea, 0x0, 0x60, 0x8, 0x57, 0xee, 0xd8);

DEFINE_GUID(IID_PushSource, 
0xe50f9ca1, 0x3462, 0x11d1, 0xae, 0xea, 0x0, 0x60, 0x8, 0x57, 0xee, 0xd8);

DECLARE_INTERFACE_(IPushSource,IUnknown)
{
STDMETHOD(setParameter) (THIS_ int command, void *data) PURE;
STDMETHOD(getParameter) (THIS_ int command, void *data) PURE;
};


#define TYPE_VIDEO               1
#define TYPE_AUDIO               2

#define CMD_CALLBACK	         1
#define CMD_TYPE                 2

#define CMD_VIDEO_WIDTH         10
#define CMD_VIDEO_HEIGHT        11
#define CMD_VIDEO_FOURCC        12
#define CMD_VIDEO_BPP           13
#define CMD_VIDEO_TIMEPERFRAME  14

#define CMD_AUDIO_FREQUENCY     20
#define CMD_AUDIO_CHANNELS      21
#define CMD_AUDIO_BITSPERSAMPLE 22

// --------------------------------------------------------------------------

#endif