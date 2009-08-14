//
//  DBOXII WinAmp Plugin
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

void config(HWND hwnd);
void about(HWND hwnd);
void init();
void quit();
int  open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms);
void close();
int  write(char *buf, int len);
int  canwrite();
int  isplaying();
int  pause(int pause);
void setvolume(int volume);
void setpan(int pan);
void flush(int t);
int  getoutputtime();
int  getwrittentime();


#define OUT_VER 0x10

extern "C"
    {
    typedef struct 
        {
	    int version;				// module version (OUT_VER)
	    char *description;			// description of module, with version string
	    int id;						// module id. each input module gets its own. non-nullsoft modules should
								    // be >= 65536. 

	    HWND hMainWindow;			// winamp's main window (filled in by winamp)
	    HINSTANCE hDllInstance;		// DLL instance handle (filled in by winamp)

	    void (*Config)(HWND hwndParent); // configuration dialog 
	    void (*About)(HWND hwndParent);  // about dialog

	    void (*Init)();				// called when loaded
	    void (*Quit)();				// called when unloaded

	    int (*Open)(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms); 
					    // returns >=0 on success, <0 on failure
					    // NOTENOTENOTE: bufferlenms and prebufferms are ignored in most if not all output plug-ins. 
					    //    ... so don't expect the max latency returned to be what you asked for.
					    // returns max latency in ms (0 for diskwriters, etc)
					    // bufferlenms and prebufferms must be in ms. 0 to use defaults. 
					    // prebufferms must be <= bufferlenms

	    void (*Close)();	// close the ol' output device.

	    int (*Write)(char *buf, int len);
					    // 0 on success. Len == bytes to write (<= 8192 always). buf is straight audio data. 
					    // 1 returns not able to write (yet). Non-blocking, always.

	    int (*CanWrite)();	// returns number of bytes possible to write at a given time. 
						    // Never will decrease unless you call Write (or Close, heh)

	    int (*IsPlaying)(); // non0 if output is still going or if data in buffers waiting to be
						    // written (i.e. closing while IsPlaying() returns 1 would truncate the song

	    int (*Pause)(int pause); // returns previous pause state

	    void (*SetVolume)(int volume); // volume is 0-255
	    void (*SetPan)(int pan); // pan is -128 to 128

	    void (*Flush)(int t);	// flushes buffers and restarts output at time t (in ms) 
							    // (used for seeking)

	    int (*GetOutputTime)(); // returns played time in MS
	    int (*GetWrittenTime)(); // returns time written in MS (used for synching up vis stuff)
        } Out_Module;
    }
