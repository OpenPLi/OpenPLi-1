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
#include "filter.h"
#include "..\\capture\\ccircularbuffer.h"
#include "render.h"
#include "debug.h"
#include "resource.h"
//#include "resrc1.h"

// ------------------------------------------------------------------------------
// PropertyPages ...
// ------------------------------------------------------------------------------

CUnknown * WINAPI CProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{

    CUnknown *punk = new CProperties(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
} // CreateInstance

//
CProperties::CProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage( NAME("DBOXII Capture Property Page")
                       , lpunk, IDD_DIALOG, IDS_NAME)
    , m_pIDBOXIIRender(NULL)
{

} // (constructor)



//
// OnReceiveMessage
//
// Override CBasePropertyPage method.
// Handle windows messages for the dialog of the property sheet.
//
BOOL CProperties::OnReceiveMessage
                            (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg) {

        case WM_INITDIALOG:
			{
			__int64 val;
            char szStr[264];
			m_pIDBOXIIRender->getParameter(CMD_STOPPLAYBACK, &val, NULL);
			SendMessage( GetDlgItem(hwnd,IDC_STOPPLAYBACK), BM_SETCHECK, (unsigned int)val, 0 );


			m_pIDBOXIIRender->getParameter(CMD_DECODERTYPE, &val, NULL);
            if (val)
			    SendMessage( GetDlgItem(hwnd,IDC_ENX), BM_SETCHECK, 1, 0 );
            else            
			    SendMessage( GetDlgItem(hwnd,IDC_GTX), BM_SETCHECK, 1, 0 );

            m_pIDBOXIIRender->getParameter(CMD_IPADDRESS, (__int64 *)szStr, NULL);
            SetWindowText(GetDlgItem(hwnd,IDC_IPADDRESS), szStr);

            m_pIDBOXIIRender->getParameter(CMD_LOGIN, (__int64 *)szStr, NULL);
            SetWindowText(GetDlgItem(hwnd,IDC_LOGIN), szStr);

            m_pIDBOXIIRender->getParameter(CMD_PASSWORD, (__int64 *)szStr, NULL);
            SetWindowText(GetDlgItem(hwnd,IDC_PASSWORD), szStr);
            }
			return TRUE;              

        case WM_COMMAND:
            //switch(GET_WM_COMMAND_ID (wParam, lParam))
			//	{
			//	}
            return TRUE;

        case WM_DESTROY:
            {
            char szStr[264];
            int val=SendMessage( GetDlgItem(hwnd,IDC_STOPPLAYBACK), BM_GETCHECK, 0, 0 );
            m_pIDBOXIIRender->setParameter(CMD_STOPPLAYBACK, (__int64)val);

            val=SendMessage( GetDlgItem(hwnd,IDC_ENX), BM_GETCHECK, 0, 0 );
            m_pIDBOXIIRender->setParameter(CMD_DECODERTYPE, (__int64)val);

            GetWindowText(GetDlgItem(hwnd,IDC_IPADDRESS), szStr, 264);
            m_pIDBOXIIRender->setParameter(CMD_IPADDRESS, (__int64)szStr);

            GetWindowText(GetDlgItem(hwnd,IDC_LOGIN), szStr, 264);
            m_pIDBOXIIRender->setParameter(CMD_LOGIN, (__int64)szStr);

            GetWindowText(GetDlgItem(hwnd,IDC_PASSWORD), szStr, 264);
            m_pIDBOXIIRender->setParameter(CMD_PASSWORD, (__int64)szStr);
            }
            return TRUE;

        default:
            return FALSE;

    } // switch

	return FALSE;
} // OnReceiveMessage

//
// OnConnect
//
// Override CBasePropertyPage method.
// Get the interface to the filter.
//
HRESULT CProperties::OnConnect(IUnknown * punk)
{
    if (punk == NULL) {
        return(E_POINTER);
    }

    HRESULT hr = punk->QueryInterface(IID_IDBOXIIRender, (void **) &m_pIDBOXIIRender);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    return NOERROR;

} // OnConnect


//
// OnDisconnect
//
// Override CBasePropertyPage method.
// Release the private interface.
//
HRESULT CProperties::OnDisconnect()
{
    //
    // Release the interface
    //
    if (m_pIDBOXIIRender == NULL) {
        return( E_UNEXPECTED);
    }

    m_pIDBOXIIRender->Release();
    m_pIDBOXIIRender = NULL;

    return(NOERROR);

} // OnDisconnect

//
// OnDeactivate
//
// Destroy the dialog.
//
HRESULT CProperties::OnDeactivate(void)
{
    return NOERROR;
} // OnDeactivate

// ------------------------------------------------------------------------------
// PropertyPages ...
// ------------------------------------------------------------------------------
