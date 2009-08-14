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

// {B0380267-8077-48e1-AAA7-ABA456C200C5}
DEFINE_GUID(CLSID_DBOXIICapture, 
0xb0380267, 0x8077, 0x48e1, 0xaa, 0xa7, 0xab, 0xa4, 0x56, 0xc2, 0x0, 0xc5);

// ----------------------------------------------------------------------------
// {53FABF59-50FF-4f5c-A794-95E6CFCC57B4}
DEFINE_GUID(CLSID_DBOXIICaptureProperty, 
0x53fabf59, 0x50ff, 0x4f5c, 0xa7, 0x94, 0x95, 0xe6, 0xcf, 0xcc, 0x57, 0xb4);


// {3FCE0451-B93C-4d90-88D7-64EAEFAB41B3}
DEFINE_GUID(IID_IDBOXIICapture, 
0x3fce0451, 0xb93c, 0x4d90, 0x88, 0xd7, 0x64, 0xea, 0xef, 0xab, 0x41, 0xb3);

// ----------------------------------------------------------------------------

DECLARE_INTERFACE_(IDBOXIICapture,IUnknown)
{
STDMETHOD(setParameter) (THIS_ __int64 command, __int64  data1) PURE;
STDMETHOD(getParameter) (THIS_ __int64 command, __int64 *data1, __int64 *data2) PURE;
};

// ----------------------------------------------------------------------------

#define CMD_IPADDRESS       1   // set/get char* in data1, pass IP (or resolvable name)
#define CMD_LOGIN           2   // set/get char* in data1, usually default "root" should be fine
#define CMD_PASSWORD        3   // set/get char* in data1, usually default "dbox2" should be fine
#define CMD_STOPPLAYBACK    4   // set/get val in data1 (0: playback is NOT stopped during capture, 1: playback IS stopped)
#define CMD_STATUS          5   //     get val in data1 (0: box can not be reached, 1: box connection is ok)
#define CMD_GETCHANNELLIST  6   //     get char* in data1, *__int64 itemcount, if itemcount==-1 get is triggered and totalcount is passed back !
#define CMD_SETCHANNEL      7   // set     data1 contains ChannelID to set
#define CMD_GETCHANNEL      8   //     get *__int64 in data1 contains current ChannelID
#define CMD_GETCHANNELINFO  9   //     get *__int64 in data2 contains ChannelInfo string
#define CMD_GETTIME         10
#define CMD_GETDATE         11
#define CMD_GETSYNCCOUNT    12  //     get *__int64 in data1 contains current resyc counter
#define CMD_GETASPECTRATIO  13  //     get *__int64 in data1 contains current aspect ratio, 1: for 4/3, 0: otherwise
#define CMD_GETBITRATE      14  //     get *__int64 in data1, data2 contains current audio,video bitrate in bit per second
#define CMD_GETWIDTHEIGHT   15  //     get *__int64 in data1 width and in data2 height
#define CMD_GETAUDIOFREQ    16  //     get *__int64 in data1 audio sample frequency at the start of the streaming
#define CMD_GETEPG          17  //     get *__int64 in data2 contains EPGInfo string
#define CMD_SKIPPREPARATION 18  // set/get *__int64 in data1 TRUE/FALSE to skip DBox Preparation or not
#define CMD_AUDIOVIDEOPID   19  //     get *__int64 in data1 Audio PID and in data2 Video PID, set lower 32 Bit Audio, upper 32 Bit Video PID (default:0,0 for automatic selection)
#define CMD_RESTARTNHTTPD   20  // set (data ignored)        simply triggers restart of nhttpd (the pure evil ...)