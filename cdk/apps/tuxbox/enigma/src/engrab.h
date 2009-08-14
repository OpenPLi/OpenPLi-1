#ifndef DISABLE_NETWORK

#ifndef __engrab_h
#define __engrab_h

#include <lib/gui/listbox.h>
#include <lib/gui/ewindow.h>
#include <lib/socket/socket.h>

class ENgrab: public Object
{
	eString sendStr;
	eString startxml( const char * descr=0 );
	eString stopxml();
	void sending();
	void connected();
	void connectionClosed();
	void connectionTimeouted();
	void dataWritten( int );
	eSocket *sd;
	eTimer timeout;
	ENgrab();
	~ENgrab();
public:
	static ENgrab *getNew() { return new ENgrab(); }
	void sendstart( const char* descr=0 );
	void sendstop();
};

#endif

#endif // DISABLE_NETWORK
