//
//  DBOXII Capture/Render Filter
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

#include "CCircularBuffer.h"
#include "debug.h"


#define WAIT_DELAY  2

CCircularBuffer::CCircularBuffer()
{
    InitializeCriticalSection(&m_CS);
    m_Event=CreateEvent(NULL,FALSE,FALSE,NULL);
    m_Buffer=NULL;
    m_RPtr=NULL;
    m_numReadPointer=0;
}

CCircularBuffer::~CCircularBuffer()
{
    DeInitialize();
    CloseHandle(m_Event);
}

void CCircularBuffer::Interrupt()
{
    m_Interrupt=TRUE;
    Sleep(WAIT_DELAY*10);
}


HRESULT CCircularBuffer::Initialize(int dwBufferSize, int numReadPointer)
{
    int i;

    if (numReadPointer<0)
        return(E_FAIL);
    m_RPtr=(int *)malloc(numReadPointer * sizeof(int));
    if (m_RPtr==NULL)
        return(E_FAIL);
    m_numReadPointer=numReadPointer;

    m_BufferSize=dwBufferSize;
    if (m_Buffer!=NULL)
        free(m_Buffer);
    m_Buffer=(BYTE *)malloc(dwBufferSize);
    if (m_Buffer==NULL)
        return(E_FAIL);
    
    for(i=0;i<m_numReadPointer;i++)
        m_RPtr[i]=0;

    m_WPtr=0;
    m_Interrupt=FALSE;


    for (i=0;i<MAX_BUFFER_ITEMS;i++)
        {
        m_BufferItemArray[i]=(BufferItemExtension*)malloc(sizeof(BufferItemExtension));
        memset(m_BufferItemArray[i], 0, sizeof(BufferItemExtension));
        }

    m_BufferExtensionRIndex=0;
    m_BufferExtensionWIndex=0;

    return(NOERROR);
}

BOOL CCircularBuffer::Clear()
{
    int i=0;
    m_Interrupt=TRUE;
    EnterCriticalSection(&m_CS);
    for(i=0;i<m_numReadPointer;i++)
        m_RPtr[i]=0;
    for (i=0;i<MAX_BUFFER_ITEMS;i++)
        memset(m_BufferItemArray[i], 0, sizeof(BufferItemExtension));
    m_WPtr=0;
    m_Interrupt=FALSE;
    LeaveCriticalSection(&m_CS);
    return(TRUE);
}

HRESULT CCircularBuffer::DeInitialize()
{
    if (m_Buffer!=NULL)
        free(m_Buffer);
    m_Buffer=NULL;
    if (m_RPtr!=NULL)
        free(m_RPtr);
    m_RPtr=NULL;
    m_numReadPointer=0;

    return(NOERROR);
}

BOOL CCircularBuffer::canWrite(long len)
{
    int i;

    EnterCriticalSection(&m_CS);
    BOOL result=TRUE;

    for(i=0;i<m_numReadPointer;i++)
        {
        int delta=m_WPtr-m_RPtr[i];
        if (delta<0)
            delta+=m_BufferSize;
        if (len>(m_BufferSize-delta-1))
            {
            result=FALSE;
            break;
            }
        }

    LeaveCriticalSection(&m_CS);
    return(result);
}

void CCircularBuffer::Write(unsigned char *src, long len)
{
    EnterCriticalSection(&m_CS);

    while (!canWrite(len) && !m_Interrupt)
        {
        LeaveCriticalSection(&m_CS);
        WaitForSingleObject(m_Event, WAIT_DELAY);
        EnterCriticalSection(&m_CS);
        }

    if (m_Interrupt)
        {
        LeaveCriticalSection(&m_CS);
        return;
        }

    if ((m_WPtr+len)>=m_BufferSize)
        {
        int first=m_BufferSize-m_WPtr;
        memcpy(m_Buffer+m_WPtr,src      ,       first);
        memcpy(m_Buffer       ,src+first, len - first);

        m_WPtr=len-first;
        }
    else
        {
        memcpy(m_Buffer+m_WPtr,src,len);
        m_WPtr+=len;
        }


    LeaveCriticalSection(&m_CS);
    SetEvent(m_Event);
}


BOOL CCircularBuffer::canRead(int which, long len)
{
    EnterCriticalSection(&m_CS);
    BOOL result=TRUE;

    if ((which<0)||(which>=m_numReadPointer))
        return(FALSE);

    int delta=m_WPtr-m_RPtr[which];
    if (delta<0) 
        delta+=m_BufferSize;

    if (len>delta)
        result=FALSE;

    LeaveCriticalSection(&m_CS);
    return(result);
}

int CCircularBuffer::Skip(int which, long len)
{
    if ((which<0)||(which>=m_numReadPointer))
        return(FALSE);

    EnterCriticalSection(&m_CS);

    if (!canRead(which, len))
        {
        LeaveCriticalSection(&m_CS);
        return(FALSE);
        }

    if ((m_RPtr[which]+len)>=m_BufferSize)
        {
        int first=m_BufferSize-m_RPtr[which];
        m_RPtr[which]=len-first;
        }
    else
        {
        m_RPtr[which]+=len;
        }
    
    LeaveCriticalSection(&m_CS);
    SetEvent(m_Event);
    return(len);
}

int CCircularBuffer::Remain(int which, long *len)
{
    if ((which<0)||(which>=m_numReadPointer))
        return(FALSE);

    EnterCriticalSection(&m_CS);

    *len=m_WPtr-m_RPtr[which];
    if (*len<0)
        *len+=m_BufferSize;

//    *len=m_BufferSize-*len;
    
    LeaveCriticalSection(&m_CS);
    SetEvent(m_Event);

    return(*len);
}



int CCircularBuffer::Read(int which, unsigned char *dest, long len)
{
    if ((which<0)||(which>=m_numReadPointer))
        return(FALSE);

    EnterCriticalSection(&m_CS);

    while (!canRead(which, len) && !m_Interrupt)
        {
        LeaveCriticalSection(&m_CS);
        WaitForSingleObject(m_Event, WAIT_DELAY);
        EnterCriticalSection(&m_CS);
        }

    if (m_Interrupt)
        {
        long diff = m_WPtr - m_RPtr[which];
        if (diff < 0) 
            diff += m_BufferSize;
        if (diff == 0) 
            {
            LeaveCriticalSection(&m_CS);
            return 0;
            }
        if (len>diff)
            {
            LeaveCriticalSection(&m_CS);
            return Read(which, dest, diff);
            }
        }


    if ((m_RPtr[which]+len)>=m_BufferSize)
        {
        int first=m_BufferSize-m_RPtr[which];
        memcpy(dest      , m_Buffer+m_RPtr[which], first);
        memcpy(dest+first, m_Buffer              , len - first);
        m_RPtr[which]=len-first;
        }
    else
        {
        memcpy(dest, m_Buffer+m_RPtr[which], len);
        m_RPtr[which]+=len;
        }
    
    LeaveCriticalSection(&m_CS);
    SetEvent(m_Event);
    return(len);
}

