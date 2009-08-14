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

#define MAX_BUFFER_ITEMS    16

typedef struct 
    {
    DWORD    isUsed;
    LONGLONG llStartTime;
    BYTE    *BufferStartAddress;
    DWORD    BufferSize;
    } BufferItemExtension;

class CCircularBuffer 
{
public:
    CCircularBuffer();
   ~CCircularBuffer();

    HRESULT Initialize(int dwBufferSize, int numReadPointer);
    HRESULT DeInitialize();
    void    Interrupt();

    BOOL    canWrite(long len);
    void    Write(unsigned char *src, long len);
    BOOL    canRead(int which, long len);
    int     Read(int which, unsigned char *dest, long len);
    BOOL    Clear();
    int     Skip(int which, long len);
    int     Remain(int which, long *len);

private:


    int     m_BufferSize;
    BYTE   *m_Buffer;
    int    *m_RPtr;
    int     m_WPtr;
    int     m_numReadPointer;

    int     m_BufferExtensionRIndex;
    int     m_BufferExtensionWIndex;

    BufferItemExtension* m_BufferItemArray[MAX_BUFFER_ITEMS];

    CRITICAL_SECTION m_CS; 
    HANDLE           m_Event;
    BOOL             m_Interrupt;
};