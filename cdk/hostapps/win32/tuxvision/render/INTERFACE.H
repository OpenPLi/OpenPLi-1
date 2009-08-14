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


// {D1B18D2E-966C-491b-830F-4367DA77F309}
DEFINE_GUID(CLSID_DBOXIIRender, 
0xd1b18d2e, 0x966c, 0x491b, 0x83, 0xf, 0x43, 0x67, 0xda, 0x77, 0xf3, 0x9);

// ----------------------------------------------------------------------------

// {35707CE2-88B1-4b9f-A255-F59B0177C3B4}
DEFINE_GUID(CLSID_DBOXIIRenderProperty, 
0x35707ce2, 0x88b1, 0x4b9f, 0xa2, 0x55, 0xf5, 0x9b, 0x1, 0x77, 0xc3, 0xb4);

// {292B9187-C7C8-4529-9793-63EA9EBDA300}
DEFINE_GUID(IID_IDBOXIIRender, 
0x292b9187, 0xc7c8, 0x4529, 0x97, 0x93, 0x63, 0xea, 0x9e, 0xbd, 0xa3, 0x0);

// ----------------------------------------------------------------------------

DECLARE_INTERFACE_(IDBOXIIRender,IUnknown)
{
STDMETHOD(setParameter) (THIS_ __int64 command, __int64  data1) PURE;
STDMETHOD(getParameter) (THIS_ __int64 command, __int64 *data1, __int64 *data2) PURE;
};

// ----------------------------------------------------------------------------

#define CMD_IPADDRESS       1   // set/get char* in data1, pass IP (or resolvable name)
#define CMD_LOGIN           2   // set/get char* in data1, usually default "root" should be fine
#define CMD_PASSWORD        3   // set/get char* in data1, usually default "dbox2" should be fine
#define CMD_STOPPLAYBACK    4   // set/get val in data1 (0: playback is NOT stopped during capture, 1: playback IS stopped
#define CMD_DECODERTYPE     5   // set/get int in data1 (0:GTX, 1:ENX)
#define CMD_DELIVEREDDATA   6   //     get val in data1, amount of delivered data
//#define CMD_VOLUME          7   // set/get val in data1, range 0(mute) .. 100