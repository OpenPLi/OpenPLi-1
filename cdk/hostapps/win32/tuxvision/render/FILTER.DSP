# Microsoft Developer Studio Project File - Name="filter" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=filter - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FILTER.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FILTER.MAK" CFG="filter - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "filter - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "filter - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "" ", "
# PROP Scc_LocalPath "."
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "filter - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Gz /MD /W3 /O2 /D "NDEBUG" /D "INC_OLE2" /D "STRICT" /D "WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D try=__try /D except=__except /D leave=__leave /D finally=__finally /U "DBG" /U "DEBUG" /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 strmbase.lib quartz.lib vfw32.lib winmm.lib kernel32.lib advapi32.lib version.lib largeint.lib user32.lib gdi32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcns4.lib rpcrt4.lib ws2_32.lib /nologo /base:"0x1c400000" /version:4.0 /entry:"DllEntryPoint@12" /dll /pdb:none /machine:I386 /out:"DBOXIIRender.ax" /subsystem:windows,4.0
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "filter - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Gz /MDd /W3 /GR /GX /Zi /D "INC_OLE2" /D "STRICT" /D "WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /FR /Oid /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 strmbasd.lib ddraw.lib quartz.lib vfw32.lib winmm.lib kernel32.lib advapi32.lib version.lib largeint.lib user32.lib gdi32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcns4.lib rpcrt4.lib ws2_32.lib /nologo /base:"0x1c400000" /entry:"DllEntryPoint@12" /dll /pdb:none /debug /machine:I386 /out:"DBOXIIRender.ax" /subsystem:windows,4.0
# SUBTRACT LINK32 /profile /nodefaultlib

!ENDIF 

# Begin Target

# Name "filter - Win32 Release"
# Name "filter - Win32 Debug"
# Begin Group "Filter"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\capture\CCIRCULARBUFFER.CPP
DEP_CPP_CCIRC=\
	"..\capture\ccircularbuffer.h"\
	"..\capture\debug.h"\
	{$(INCLUDE)}"amextra.h"\
	{$(INCLUDE)}"amfilter.h"\
	{$(INCLUDE)}"audevcod.h"\
	{$(INCLUDE)}"cache.h"\
	{$(INCLUDE)}"combase.h"\
	{$(INCLUDE)}"cprop.h"\
	{$(INCLUDE)}"ctlutil.h"\
	{$(INCLUDE)}"dllsetup.h"\
	{$(INCLUDE)}"fourcc.h"\
	{$(INCLUDE)}"measure.h"\
	{$(INCLUDE)}"msgthrd.h"\
	{$(INCLUDE)}"mtype.h"\
	{$(INCLUDE)}"outputq.h"\
	{$(INCLUDE)}"pstream.h"\
	{$(INCLUDE)}"refclock.h"\
	{$(INCLUDE)}"reftime.h"\
	{$(INCLUDE)}"renbase.h"\
	{$(INCLUDE)}"Schedule.h"\
	{$(INCLUDE)}"source.h"\
	{$(INCLUDE)}"streams.h"\
	{$(INCLUDE)}"strmctl.h"\
	{$(INCLUDE)}"sysclock.h"\
	{$(INCLUDE)}"transfrm.h"\
	{$(INCLUDE)}"transip.h"\
	{$(INCLUDE)}"videoctl.h"\
	{$(INCLUDE)}"vtrans.h"\
	{$(INCLUDE)}"winctrl.h"\
	{$(INCLUDE)}"winutil.h"\
	{$(INCLUDE)}"wxdebug.h"\
	{$(INCLUDE)}"wxlist.h"\
	{$(INCLUDE)}"wxutil.h"\
	
# End Source File
# Begin Source File

SOURCE=..\capture\ccircularbuffer.h
# End Source File
# Begin Source File

SOURCE=.\DEBUG.C
DEP_CPP_DEBUG=\
	".\DEBUG.H"\
	
# End Source File
# Begin Source File

SOURCE=.\DEBUG.H
# End Source File
# Begin Source File

SOURCE=.\dump.def
# End Source File
# Begin Source File

SOURCE=.\dump.rc
# End Source File
# Begin Source File

SOURCE=.\filter.CPP
DEP_CPP_FILTE=\
	"..\capture\ccircularbuffer.h"\
	"..\capture\debug.h"\
	".\filter.H"\
	".\interface.h"\
	".\render.h"\
	{$(INCLUDE)}"amextra.h"\
	{$(INCLUDE)}"amfilter.h"\
	{$(INCLUDE)}"audevcod.h"\
	{$(INCLUDE)}"cache.h"\
	{$(INCLUDE)}"combase.h"\
	{$(INCLUDE)}"cprop.h"\
	{$(INCLUDE)}"ctlutil.h"\
	{$(INCLUDE)}"dllsetup.h"\
	{$(INCLUDE)}"fourcc.h"\
	{$(INCLUDE)}"ksmedia.h"\
	{$(INCLUDE)}"measure.h"\
	{$(INCLUDE)}"msgthrd.h"\
	{$(INCLUDE)}"mtype.h"\
	{$(INCLUDE)}"outputq.h"\
	{$(INCLUDE)}"pstream.h"\
	{$(INCLUDE)}"refclock.h"\
	{$(INCLUDE)}"reftime.h"\
	{$(INCLUDE)}"renbase.h"\
	{$(INCLUDE)}"Schedule.h"\
	{$(INCLUDE)}"source.h"\
	{$(INCLUDE)}"streams.h"\
	{$(INCLUDE)}"strmctl.h"\
	{$(INCLUDE)}"sysclock.h"\
	{$(INCLUDE)}"transfrm.h"\
	{$(INCLUDE)}"transip.h"\
	{$(INCLUDE)}"videoctl.h"\
	{$(INCLUDE)}"vtrans.h"\
	{$(INCLUDE)}"winctrl.h"\
	{$(INCLUDE)}"winutil.h"\
	{$(INCLUDE)}"wxdebug.h"\
	{$(INCLUDE)}"wxlist.h"\
	{$(INCLUDE)}"wxutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\filter.H
# End Source File
# Begin Source File

SOURCE=.\interface.h
# End Source File
# Begin Source File

SOURCE=.\PERSISTBAG.CPP
DEP_CPP_PERSI=\
	"..\capture\ccircularbuffer.h"\
	".\DEBUG.H"\
	".\filter.H"\
	".\interface.h"\
	".\render.h"\
	{$(INCLUDE)}"amextra.h"\
	{$(INCLUDE)}"amfilter.h"\
	{$(INCLUDE)}"audevcod.h"\
	{$(INCLUDE)}"cache.h"\
	{$(INCLUDE)}"combase.h"\
	{$(INCLUDE)}"cprop.h"\
	{$(INCLUDE)}"ctlutil.h"\
	{$(INCLUDE)}"dllsetup.h"\
	{$(INCLUDE)}"fourcc.h"\
	{$(INCLUDE)}"measure.h"\
	{$(INCLUDE)}"msgthrd.h"\
	{$(INCLUDE)}"mtype.h"\
	{$(INCLUDE)}"outputq.h"\
	{$(INCLUDE)}"pstream.h"\
	{$(INCLUDE)}"refclock.h"\
	{$(INCLUDE)}"reftime.h"\
	{$(INCLUDE)}"renbase.h"\
	{$(INCLUDE)}"Schedule.h"\
	{$(INCLUDE)}"source.h"\
	{$(INCLUDE)}"streams.h"\
	{$(INCLUDE)}"strmctl.h"\
	{$(INCLUDE)}"sysclock.h"\
	{$(INCLUDE)}"transfrm.h"\
	{$(INCLUDE)}"transip.h"\
	{$(INCLUDE)}"videoctl.h"\
	{$(INCLUDE)}"vtrans.h"\
	{$(INCLUDE)}"winctrl.h"\
	{$(INCLUDE)}"winutil.h"\
	{$(INCLUDE)}"wxdebug.h"\
	{$(INCLUDE)}"wxlist.h"\
	{$(INCLUDE)}"wxutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\PROPERTY.CPP
DEP_CPP_PROPE=\
	"..\capture\ccircularbuffer.h"\
	".\DEBUG.H"\
	".\filter.H"\
	".\interface.h"\
	".\render.h"\
	{$(INCLUDE)}"amextra.h"\
	{$(INCLUDE)}"amfilter.h"\
	{$(INCLUDE)}"audevcod.h"\
	{$(INCLUDE)}"cache.h"\
	{$(INCLUDE)}"combase.h"\
	{$(INCLUDE)}"cprop.h"\
	{$(INCLUDE)}"ctlutil.h"\
	{$(INCLUDE)}"dllsetup.h"\
	{$(INCLUDE)}"fourcc.h"\
	{$(INCLUDE)}"measure.h"\
	{$(INCLUDE)}"msgthrd.h"\
	{$(INCLUDE)}"mtype.h"\
	{$(INCLUDE)}"outputq.h"\
	{$(INCLUDE)}"pstream.h"\
	{$(INCLUDE)}"refclock.h"\
	{$(INCLUDE)}"reftime.h"\
	{$(INCLUDE)}"renbase.h"\
	{$(INCLUDE)}"Schedule.h"\
	{$(INCLUDE)}"source.h"\
	{$(INCLUDE)}"streams.h"\
	{$(INCLUDE)}"strmctl.h"\
	{$(INCLUDE)}"sysclock.h"\
	{$(INCLUDE)}"transfrm.h"\
	{$(INCLUDE)}"transip.h"\
	{$(INCLUDE)}"videoctl.h"\
	{$(INCLUDE)}"vtrans.h"\
	{$(INCLUDE)}"winctrl.h"\
	{$(INCLUDE)}"winutil.h"\
	{$(INCLUDE)}"wxdebug.h"\
	{$(INCLUDE)}"wxlist.h"\
	{$(INCLUDE)}"wxutil.h"\
	
# End Source File
# End Group
# Begin Group "DBOXII"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\render.cpp
DEP_CPP_RENDE=\
	".\CCIRCULARBUFFER.H"\
	".\DEBUG.H"\
	".\filter.H"\
	".\interface.h"\
	".\render.h"\
	{$(INCLUDE)}"amextra.h"\
	{$(INCLUDE)}"amfilter.h"\
	{$(INCLUDE)}"audevcod.h"\
	{$(INCLUDE)}"cache.h"\
	{$(INCLUDE)}"combase.h"\
	{$(INCLUDE)}"cprop.h"\
	{$(INCLUDE)}"ctlutil.h"\
	{$(INCLUDE)}"dllsetup.h"\
	{$(INCLUDE)}"fourcc.h"\
	{$(INCLUDE)}"measure.h"\
	{$(INCLUDE)}"msgthrd.h"\
	{$(INCLUDE)}"mtype.h"\
	{$(INCLUDE)}"outputq.h"\
	{$(INCLUDE)}"pstream.h"\
	{$(INCLUDE)}"refclock.h"\
	{$(INCLUDE)}"reftime.h"\
	{$(INCLUDE)}"renbase.h"\
	{$(INCLUDE)}"Schedule.h"\
	{$(INCLUDE)}"source.h"\
	{$(INCLUDE)}"streams.h"\
	{$(INCLUDE)}"strmctl.h"\
	{$(INCLUDE)}"sysclock.h"\
	{$(INCLUDE)}"transfrm.h"\
	{$(INCLUDE)}"transip.h"\
	{$(INCLUDE)}"videoctl.h"\
	{$(INCLUDE)}"vtrans.h"\
	{$(INCLUDE)}"winctrl.h"\
	{$(INCLUDE)}"winutil.h"\
	{$(INCLUDE)}"wxdebug.h"\
	{$(INCLUDE)}"wxlist.h"\
	{$(INCLUDE)}"wxutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# End Group
# Begin Group "Telnet"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\capture\telnet\ProtocolRx.cpp
DEP_CPP_PROTO=\
	"..\capture\telnet\ProtocolRx.h"\
	
# End Source File
# Begin Source File

SOURCE=..\capture\telnet\ProtocolRx.h
# End Source File
# Begin Source File

SOURCE=..\capture\telnet\SocketDx.cpp
DEP_CPP_SOCKE=\
	"..\capture\debug.h"\
	"..\capture\telnet\SocketDx.h"\
	
# End Source File
# Begin Source File

SOURCE=..\capture\telnet\SocketDx.h
# End Source File
# Begin Source File

SOURCE=..\capture\telnet\SocketRx.cpp
DEP_CPP_SOCKET=\
	"..\capture\debug.h"\
	"..\capture\telnet\ProtocolRx.h"\
	"..\capture\telnet\SocketRx.h"\
	
# End Source File
# Begin Source File

SOURCE=..\capture\telnet\SocketRx.h
# End Source File
# Begin Source File

SOURCE=..\capture\telnet\SocketTx.cpp
DEP_CPP_SOCKETT=\
	"..\capture\debug.h"\
	"..\capture\telnet\SocketTx.h"\
	
# End Source File
# Begin Source File

SOURCE=..\capture\telnet\SocketTx.h
# End Source File
# Begin Source File

SOURCE=..\capture\telnet\Telnet.cpp
DEP_CPP_TELNE=\
	"..\capture\ccircularbuffer.h"\
	"..\capture\debug.h"\
	"..\capture\grab.h"\
	"..\capture\telnet\ProtocolRx.h"\
	"..\capture\telnet\SocketDx.h"\
	"..\capture\telnet\SocketRx.h"\
	"..\capture\telnet\SocketTx.h"\
	
# End Source File
# End Group
# Begin Source File

SOURCE=.\AUTHORS.TXT
# End Source File
# Begin Source File

SOURCE=.\COPYING.TXT
# End Source File
# Begin Source File

SOURCE=.\README.TXT
# End Source File
# End Target
# End Project
