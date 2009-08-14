#ifndef __LIB_BASE_CONSOLE_H__
#define __LIB_BASE_CONSOLE_H__

#include <lib/base/ebase.h>
#include <queue>

class eString;

struct queue_data
{
	queue_data( char *data, int len )
		:data(data), len(len)
	{
	}
	char *data;
	int len;
};

class eConsoleAppContainer: public Object
{
	int fd[3];
	int pid;
	int killstate;
	std::queue<struct queue_data> outbuf;
	eSocketNotifier *in, *out, *err;
	void readyRead(int what);
	void readyErrRead(int what);
	void readyWrite(int what);
	void closePipes();
	void init_eConsoleAppContainer( const eString &cmd );
public:
	eConsoleAppContainer( const eString &str );
	~eConsoleAppContainer();
	int getPID() { return pid; }
	void kill();
	void sendCtrlC();
	void write( const char *data, int len );
	bool running() { return (fd[0]!=-1) && (fd[1]!=-1) && (fd[2]!=-1); }
	Signal1<void, eString> dataAvail;
	Signal1<void,int> dataSent;
	Signal1<void,int> appClosed;
};

#endif // __LIB_BASE_CONSOLE_H__
