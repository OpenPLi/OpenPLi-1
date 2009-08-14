/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __controldclient__
#define __controldclient__

#include <string>

#include <zapit/client/basicclient.h>

using namespace std;


#define VCR_STATUS_OFF 0
#define VCR_STATUS_ON 1
#define VCR_STATUS_16_9 2

class CControldClient:private CBasicClient
{
		void send(const unsigned char command, char* data, const unsigned int size);

	public:

		enum events
		{
			EVT_VOLUMECHANGED,
			EVT_MUTECHANGED,
			EVT_MODECHANGED,
			EVT_VCRCHANGED
		};

		//VideoFormat
		static const char VIDEOFORMAT_AUTO = 0;
		static const char VIDEOFORMAT_16_9 = 1;
		static const char VIDEOFORMAT_4_3  = 2;

		//VideoOutput
		static const char VIDEOOUTPUT_COMPOSITE = 0;
		static const char VIDEOOUTPUT_SVIDEO    = 2;
		static const char VIDEOOUTPUT_RGB       = 1;

		//mute
		static const bool VOLUME_MUTE = true;
		static const bool VOLUME_UNMUTE = false;

		//BoxType
		static const char BOXTYPE_NOKIA   = 1;
		static const char BOXTYPE_SAGEM   = 2;
		static const char BOXTYPE_PHILIPS = 3;

		//scartmode
		static const char SCARTMODE_ON  = 1;
		static const char SCARTMODE_OFF = 0;

		/*
			setVolume(char) : Setzten der Lautstärke
			Parameter: 0..100 - 0=leise 100=laut
			           avs == true : mute avs device
			           avs == false: mute audio device
		*/
		void setVolume(const char volume, const bool avs = true);
		char getVolume(const bool avs = true);

		/*
			setMute(bool, bool) : setzen von Mute
			Parameter: VOLUME_MUTE   = ton aus
			           VOLUME_UNMUTE = ton an
			           avs == true : mute avs device
			           avs == false: mute audio device
		*/
		void setMute(const bool mute, const bool avs = true);
		bool getMute(const bool avs = true);

		/*
			Mute(bool) : Ton ausschalten
			Parameter: avs == true : mute avs device
			           avs == false: mute audio device
		*/
		void Mute(const bool avs = true);

		/*
			UnMute(bool) : Ton wieder einschalten
			Parameter: avs == true : mute avs device
			           avs == false: mute audio device
		*/
		void UnMute(const bool avs = true);

		/*
			Setzen des AnalogOutputs (0 = stereo, 1 = mono left, 2 = mono right)
		*/
		void setAnalogOutput(int mode);

		/*
			setVideoFormat(char) : Setzten des Bildformates ( 4:3 / 16:9 )
			Parameter: VIDEOFORMAT_AUTO = auto
			           VIDEOFORMAT_4_3  = 4:3
			           VIDEOFORMAT_16_9 = 16:9
		*/
		void setVideoFormat(char);
		char getVideoFormat();
		/*
			getAspectRatio : Aktueller Wert aus dem Bitstream
					2: 4:3
					3: 16:9
					4: 2:2.1
		*/
		char getAspectRatio();

		/*
			setVideoOutput(char) : Setzten des Videooutputs ( composite / svhs / rgb )
			Parameter: VIDEOOUTPUT_COMPOSITE = composite video
			           VIDEOOUTPUT_SVIDEO    = svhs video
			           VIDEOOUTPUT_RGB       = rgb
		*/
		void setVideoOutput(char);
		char getVideoOutput();

		/*
			setVideoOutput(char) : Setzten des Boxentyps ( nokia / sagem / philips )
			Parameter: BOXTYPE_NOKIA   = nokia dbox
			           BOXTYPE_SAGEM   = sagem
			           BOXTYPE_PHILIPS = philips

		*/
		void setBoxType(char);
		char getBoxType();

		/*
			setVideoOutput(char) : Scartmode ( an / aus )
			Parameter: SCARTMODE_ON  = auf scartinput schalten
			           SCARTMODE_OFF = wieder dvb anzeigen


		*/
		void setScartMode(bool);


		/*
			die Dbox herunterfahren
		*/
		void videoPowerDown(bool);

		/*
			die Dbox herunterfahren
		*/
		void shutdown();


		/*
			ein beliebiges Event anmelden
		*/
		void registerEvent(unsigned int eventID, unsigned int clientID, string udsName);

		/*
			ein beliebiges Event abmelden
		*/
		void unRegisterEvent(unsigned int eventID, unsigned int clientID);

		void saveSettings();

};

#endif
