/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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


#ifndef __flashtool__
#define __flashtool__

#include <string>
#include <vector>


#include <gui/widget/progressstatus.h>


using namespace std;

class CFlashTool
{
	private:
	
		int		fd_fp;
		CProgress_StatusViewer* statusViewer;
		string mtdDevice;
		string ErrorMessage;

		bool erase(int globalProgressEnd=-1);

	public:
		CFlashTool();
		~CFlashTool();

		string getErrorMessage();

		void setMTDDevice( string mtddevice );
		void setStatusViewer( CProgress_StatusViewer* statusview );

		bool program( string filename, int globalProgressEndErase=-1, int globalProgressEndFlash=-1 );
		bool readFromMTD( string filename, int globalProgressEnd=-1 );

		bool check_cramfs( string filename );

		void reboot();
};


class CFlashVersionInfo
{
	private:

		string	date;
		string	time;
		string	baseImageVersion;
		char	snapshot;

	public:
		CFlashVersionInfo(string versionString);

		string getDate();
		string getTime();
		string getBaseImageVersion();
		string getType();
};


class CMTDInfo
{
	private:

		struct SMTDPartition
		{
			int size;
			int erasesize;
			string name;
			string filename;
		};

		vector<SMTDPartition*> mtdData;
		
		void getPartitionInfo();

		CMTDInfo();
		~CMTDInfo();

	public: 
		static CMTDInfo* getInstance();
	
		int getMTDCount();

		//mtdinfos abfragen (nach mtdnummer)
		string getMTDName( int pos );
		string getMTDFileName( int pos );
		int getMTDSize( int pos );
		int getMTDEraseSize( int pos );

		//mtdinfos abfragen (nach mtd-filename)
		string getMTDName( string filename );
		string getMTDFileName( string filename );
		int getMTDSize( string filename );
		int getMTDEraseSize( string filename );

		int findMTDNumber( string filename );

};


#endif
