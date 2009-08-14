/*
 *  main.h - Main program
 *
 *  Frodo (C) 1994-1997 Christian Bauer
 */

#ifndef _MAIN_H
#define _MAIN_H


class C64;

// Global variables
extern char AppDirPath[1024];	// Path of application directory
extern char d64image[512];	// fname of application
extern char kbdname[512];	// fname of application


/*
 *  X specific stuff
 */

class Prefs;

class Frodo {
public:
	Frodo();
	void ArgvReceived(int argc, char **argv);
	void ReadyToRun(void);
	static Prefs *reload_prefs(void);

	C64 *TheC64;

private:
	bool load_rom_files(void);

	static char prefs_path[256];	// Pathname of current preferences file
};


inline void Debug(const char *format, ...)
{
}

#define DebugResult(message, val) \
	Debug("%s: 0x%x (%d)\n", message, val, HRESULT_CODE(val))


#endif
