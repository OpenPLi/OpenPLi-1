#ifndef __AX_HELPER_H__
#define __AX_HELPER_H__

#ifdef __cplusplus              //
extern "C" {                    // Assume C declarations for C++
#endif                          //

HRESULT __cdecl GetPinByNumber(IBaseFilter *pFilter, IPin **pPinResult, int number, int direction);

HRESULT __cdecl LoadFilterByName(IGraphBuilder *pgb, WCHAR *pszName, IBaseFilter **pFilter, WCHAR *pszFriendlyName);
HRESULT __cdecl FindPinByName(IBaseFilter *pFilter, WCHAR *pszName, IPin **ppPin);

HRESULT __cdecl GetInputPins(IBaseFilter *pFilter, WCHAR PinNames[264][64], int *count);
HRESULT __cdecl GetOutputPins(IBaseFilter *pFilter, WCHAR PinNames[264][64], int *count);
HRESULT __cdecl GetInputPin(IBaseFilter* filter, IPin** find_pin, int index = 0);
HRESULT __cdecl GetOutputPin(IBaseFilter* filter, IPin** find_pin, int index = 0);
HRESULT __cdecl GetPin(IBaseFilter* filter, PIN_DIRECTION direction, int index, IPin** find_pin);

HRESULT __cdecl EnumerateAVDevices(char *videodriver[], int *vcount, char *audiodriver[], int *acount);

//HRESULT __cdecl WrapSizeCode(int sizeCode, int isPALTiming, int *width, int *height);


HRESULT __cdecl TryConnectingFilters(IGraphBuilder *pgb, IBaseFilter *pSrc, int SrcPinNumber, IBaseFilter *pDst, int DstPinNumber, BOOL ConnectDirect);
HRESULT __cdecl TryConnectPinToFilter(IGraphBuilder *pgb, IPin *pPinSrc, IBaseFilter *pDst, int DstPinNumber, BOOL ConnectDirect);
HRESULT __cdecl TryConnectFilterToPin(IGraphBuilder *pgb, IBaseFilter *pSrc, int SrcPinNumber, IPin *pPinDst, BOOL ConnectDirect);

HRESULT __cdecl TryInsertFilterBeforeInputPin(IGraphBuilder *pgb, IBaseFilter *pFilter, int InputPinNumber, int OutputPinNumber, IPin *pInputPin, BOOL ConnectDirect);
HRESULT __cdecl TryInsertFilterAfterOutputPin(IGraphBuilder *pgb, IBaseFilter *pFilter, int InputPinNumber, int OutputPinNumber, IPin *pOutputPin, BOOL ConnectDirect);
HRESULT __cdecl TryInsertFilterAnywhere(IGraphBuilder *pgb, IBaseFilter *pFilter, int InputPinNumber, int OutputPinNumber, BOOL ConnectDirect);



HRESULT __cdecl TryDisconnectingFilters(IGraphBuilder *pgb, IBaseFilter *pSrc, int SrcPinNumber, IBaseFilter *pDst, int DstPinNumber);
HRESULT __cdecl LoadFilterByCLSID(IGraphBuilder *pgb, GUID clsid, IBaseFilter **pFilter, WCHAR *name);

HRESULT __cdecl DestroyGraph(IGraphBuilder *pgb);
HRESULT __cdecl RunGraph(IGraphBuilder *pgb);
HRESULT __cdecl StopGraph(IGraphBuilder *pgb);
HRESULT __cdecl PauseGraph(IGraphBuilder *pgb);


DWORD   __cdecl WaitForMultipleObjectsPump(DWORD nCount, LPHANDLE pHandles, bool bWaitAll, DWORD dwMillisec);
DWORD   __cdecl WaitForSingleObjectPump(HANDLE hHandle, DWORD dwMillisec);


HRESULT __cdecl AddGraphToRot(IGraphBuilder *pgb, DWORD id);
HRESULT __cdecl RemoveGraphFromRot(DWORD id);

HRESULT __cdecl FindVideoRenderer(IGraphBuilder *pGraph, IBaseFilter **pf);
HRESULT __cdecl FindFilterConnectedToPin(IGraphBuilder *pgb, IBaseFilter **pFilter, IPin *pInputPin);

HRESULT __cdecl DeCouplePin(IGraphBuilder *pgb, IPin *pPin, IPin **pPinResult);

HRESULT __cdecl RemoveFilterWithUnconnectedInput(IGraphBuilder *pgb, IBaseFilter *pFilter, int InputPinNumber);

#ifdef __cplusplus              //
        }                       // Assume C declarations for C++
#endif                          //

#endif


