-------------------------------------------------------------------------------------------------

This is version 0.0.0.16 of TuxVision/Capture/Render 
Date: 08.07 2002

This text describes usage of application and direct show filters running on a win32 OS
which are written to give you preview, capture and playback capabilities for a lan
connected DBOXII running linux.

-------------------------------------------------------------------------------------------------

*** IMPORTANT ***

Before you start recompiling the filter
make shure you have the latest DirectX SDK installed.
Please ensure you recompiled strmbase.lib and strmbasd.lib from the SDK.
Don´t forget to setup correct path to the include and lib directory of the SDK.
You may ignore this step if you simply want to use the filter and application binary as it is !

(The software/hardware configuration)
Currently I´m using M$ VC6 compiler and the DirectX8.1 SDK
running on a P4, 1.8GHz, 512MB RDRAM, MX2, Win2k
DBOX Nokia Cable, AVIA500, GTX, 2xIntel, connected through 10MBit HUB
running neutrino based on various images (current: 07.04.2002)

*** IMPORTANT ***

-------------------------------------------------------------------------------------------------

*** IMPORTANT ***

Before fiddling around with the filters or even trying to start the TuxVision app it is 
recommanded that you load and install the latest version of TRex 
(Generic DShow Based MediaConverter) ! This gives you a core set of absolutely needed
filter components.

*** IMPORTANT ***

-------------------------------------------------------------------------------------------------

*** GENERAL ***

To register a direct show filter (that is nothing less or more than a COM object)
you can use the regsvr32.exe program that can be found in your system or system32
directory.

Simply call: regsvr32 <filter.ax> to register the filter. If you want to unregister
a component call: regsvr32 /u <filter.ax>
Usually it doesn´t matter where you install the components.

-------------------------------------------------------------------------------------------------

*** TUXVISION ***

TODO:
- get HTTP and CTRL (streaming) Port alive
- create graphbuilding for Audio-Only mode
- implement basic playback graph building

-------------------------------------------------------------------------------------------------

*** WinAmp - Plugin ***

Installing the WinAmp Output plugin should be quite simple.
If you already installed WinAmp to its default directory "\Program Files\WinAmp" the exporter
plugin (out_DBOXII.dll) should be copied to the Plugin directory. If you installed WinAmp to
a different location or encounter some problems (e.g. you don´t find the output plugin in the 
options/preferences/Plug-ins/output section (it is called BSE DBOXII OutputPlugin Rev.x.x) you
should locate your TuxVision install directory and copy the out_DBOXII.dll manually to the plugin
directory.

Once installation is done open options/preferences/Plug-ins/output select the DBOXII plugin and
hit configure to setup your user specific parameters (like IP-Address).

-------------------------------------------------------------------------------------------------

*** CAPTURE ***

TODO:
- write a better multiplexer or wait for a PES-Mux ?
- get raw video/audio alive (internal graph building)
- implement interface for PID setup
- make HTTP transfers faster

1.) (Rebuild the source if you like :-) and then) register the capture filter (DBOXIISource.ax)
using regsvr32.exe (which usually can be found in your windows system or system32 
directory. (I prefere a link on my desktop where I can simply drag and drop
filters to register them).
2.) Take a look at the graphedit.gif image which shows a simple preview graph that makes use
of the source filter.
You´ll see a MPEG2Demultiplexer, a MPEG2VideoDecoder and a VideoRenderFilter.
Of cause you are free to use the existing M$ Demultiplexer but due to lack of appropriate 
bufferhandling this beast is not very smart. If you want to go with the shown filters
go an download the latest TRex installation, which will give you (beneath a nice conversion
utility) the MPEG2Demultiplexer.
3.) Open grpahedit and add the source filter first. Once loaded you should open
the properties of the capture filter (right click) and modify parameters like
IPAddress. (Login and Password should be ok in most cases)
4.) Load the Demultiplexer and connect it to the PS Output pin.
5.) From here it highly depends wich MPEGDecoder you are willing to use.
If you don´t care you might give render on the Video Output a try, but I would
strongly recommend (once it is available through TRex) to use the filter that is 
shown in the graphedit_preview_video.gif image.
6.) Hit run. Now your currently selected program should come up in a window.
If it stutters try hitting pause and then run again (That fills up some
buffers internally)
7.) Connecting the audio looks trivial BUT: currently the audio output defaults to 44.1 kHz
and then performs a inband change to 48kHz. This is a completely valid behaviour but
it looks like M$ DSound renderer has some problems ...
So I think you wanna use the PinnaleDemuxII that has a nice switch for 48kHz Audio ...

-------------------------------------------------------------------------------------------------

*** RENDER *** (also called clipmode)

TODO: 
- path for binaries are fixed to /var/tuxbox, maybe I´ll find a smarter way ...
- Basic Video and Audio works, BUT it looks like streaming on both pins parallel confuses my GTX + dvrv/dvra !
- avoid annoying "queeek" at the very beginning when streaming audio
- AC3 support
- bottom field first order support
- setup more reasonable Types/Subtypes for the pins
- implement all the needed interfaces that a REAL Render filter should expose
- pray for a nice guy to improve GTX playback performance (resyncing ...)
- prevent using PS and V_ES/A_ES pins at the same time

Rev.0.0.0.16: 
- Support for WinAmp Audiorendering implemented


Neutrino from 07.04.2002 with the actual versions of GTX_DVR.O and ENX_DVR.O
(grabbed from http://www.elitedvb.de/files/enx_dvr.o and http://www.elitedvb.de/files/gtx_dvr.o)
For your conveniance I placed a copy on my server http://transputer.dyn.escape.de/src/dboxII/
Also the fbclear utility can be found there. 
These modules do NOT work with current images (e.g. alexW 1.5 !)
You should update your box only if you plan to use the render (clip) mode. These files are NOT
necessary for the capture mode.
To work with the (render)filters you should place these 3 files in your /var/tuxbux directory on
your dbox. (Please use THIS and not another directory because the path is hardcoded in the filter ;-))
(You might use any ftp program for this step) Please don´t forget to make the
fbclear program executable (chmod +x fbclear), otherwise your screen will stay quite dark
during video playback :-))

In general the clipmode is still under heavy development (this is what I hope !) so you may not 
expect tooooo much. Video OR Audio should work but all together ...
VideoCapture is in a muuuuch better shape.
This is true for the neutrino version I´m testing with. I checked a few later revs and found
render completetly broken (xxx_dvr.o is not loadable anymore) and capture only works with preview
turned off. Sorry to solve these problems is out of my reach.

Video should work fine with 352x288@25fps MPEG1/MPEG2, 480x576@25fps MPEG2, 704x576@25fps MPEG2
and 720x576@25fps MPEG2. (I´ll have to check if also 352x576@25fps MPEG2 is supported)
graphedit_render_dv_video.gif shows how things are connected up for a video path.
Ofcause DV is BottomField first, samples are displayed in a wrong field order, but overall
picture quality (I had no probs with 6Mbit) was really nice. If you want to display MPEG1
VCD simply connect the splitter video output to VPES input of the renderer. 
If you need the MPEGEncoderEngine (is include in TRex install) take a look at 
graphedit_MPEGEncoder_Properties.gif. Depending on your decoder it might be neccesary to switch 
to Offline mode to avoid stalling the graph.

On the Audio side things get only a little bit more complicated. It looks like the decoder simply
needs packetized data. Therefore a ES to PES translation is neccesary. 
(I´ll attach this special Mutiplexer in binary only, sorry for that but I´ll think this is a thing
you won´t touch anyway, believe me ...)
Take a look at graphedit_render_audio.gif. I took a MP3 only as an example, in practice you might 
convert ANY audio stream you have a DSHow reader/splitter combination (WAVE, MP3, WMA, MPA, ...).
Please take care of setting the AudioResampler to 48kHz destination frequency. On the AudioMPEG-
Encoder side I decided to go for 256kBit (always Layer II).

As an input filter I used for all tests the FileSource(Async).

-------------------------------------------------------------------------------------------------

Bernd Scharping (BSE) bernd@transputer.escape.de

-------------------------------------------------------------------------------------------------
