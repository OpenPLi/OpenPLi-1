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

#ifndef __RENDER_FILTER_H__
#define __RENDER_FILTER_H__

class CDBOXIIRenderInputPin;
class CDBOXIIRenderOutputPin;

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
    IDBOXIIRender   *m_pIDBOXIIRender;       
};  

class CDBOXIIRender : public CBaseFilter,
		       public ISpecifyPropertyPages,
               public IDBOXIIRender

{
    CCritSec m_Lock;                    // Main renderer critical section

public:
    CDBOXIIRenderInputPin  *m_pPinVideoIn;         
    CDBOXIIRenderInputPin  *m_pPinAudioIn;      
    CDBOXIIRenderInputPin  *m_pPinMultiplexIn;

public:
	DECLARE_IUNKNOWN
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP GetPages(CAUUID * pPages);

    STDMETHODIMP setParameter(__int64 command, __int64  data1);
    STDMETHODIMP getParameter(__int64 command, __int64 *data1, __int64 *data2);

    int GetPinCount(void);
    CBasePin *GetPin(int n);

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);

    HRESULT SaveInternalData(void);
    HRESULT LoadInternalData(void);


    CDBOXIIRender(LPUNKNOWN lpunk, HRESULT *phr);
   ~CDBOXIIRender();


    HRESULT  GetCurrentState();
    LONGLONG m_LastDeliveredAudio;
    LONGLONG m_LastDeliveredVideo;

    int      m_MyState;

    char                m_DBOXAddress[264];
    char                m_DBOXLogin[264];
    char                m_DBOXPassword[264];
    int                 m_IsENX;
    int                 m_DBOXAPort;
    int                 m_DBOXVPort;
    int                 m_DBOXHTTPPort;
    int                 m_DBOXTelnetPort;
    int                 m_DBOXStopPlayback;

}; 


class CDBOXIIRenderInputPin : public CBaseInputPin
{
    int m_Type;
    CDBOXIIRender  *m_pFilter;
public:
    CDBOXIIRenderInputPin(LPCWSTR pName, CDBOXIIRender *pXSync, LPUNKNOWN pUnk, CBaseFilter *pFilter, CCritSec *pLock, HRESULT *phr, int type);

    CCritSec *m_pILock;

    HRESULT CheckMediaType(const CMediaType *cmt);
    HRESULT SetMediaType(const CMediaType *cmt);
    
    STDMETHODIMP EndOfStream(void);
    STDMETHODIMP BeginFlush(void);
    STDMETHODIMP EndFlush(void);


    STDMETHODIMP Receive(IMediaSample *pSample);
    STDMETHODIMP ReceiveCanBlock();
}; 
	
#endif