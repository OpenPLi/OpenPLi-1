#ifndef DISABLE_FILE

#ifndef __lib_dvb_serviceexternal_h
#define __lib_dvb_serviceexternal_h

#include <lib/dvb/service.h>
#include <lib/base/thread.h>
#include <lib/base/message.h>

class eServiceHandlerExternal: public eServiceHandler
{
	static eServiceHandlerExternal *instance;

	struct FileExtensionScriptInfo
	{
		int needfb;
		int needrc;
		int needlcd;
		eString pattern;
		eString command;
	};

	std::vector<FileExtensionScriptInfo> extensionFileList, extensionDirectoryList;

	void addFile(void *node, const eString &filename);
	void addDirectory(void *node, const eString &filename);

public:
	eServiceHandlerExternal();
	~eServiceHandlerExternal();

	int play(const eServiceReference &service, int workaround = 0);
	int stop(int workaround = 0);

	eService *addRef(const eServiceReference &service);
	void removeRef(const eServiceReference &service);

	void addFileHandler(const eString &pattern, const eString &command, int needfb, int needrc, int needlcd);
	void addDirectoryHandler(const eString &pattern, const eString &command, int needfb, int needrc, int needlcd);

	static eServiceHandlerExternal *getInstance() { return instance; }
};

class ePlayerThread: public eThread, public Object
{
	eFixedMessagePump<int> message;
	eString command;
	bool needfb, needrc, needlcd;
	void thread();
	void thread_finished();
	void finalize_player();
	void recv_msg(const int &);
public:
	ePlayerThread(eString command, int needfb, int needrc, int needlcd)
		:message(eApp, 1), command(command), needfb(needfb), needrc(needrc), needlcd(needlcd)
	{
		CONNECT(message.recv_msg, ePlayerThread::recv_msg);
	}
	~ePlayerThread()
	{
	}
	void start();
};

#endif

#endif //DISABLE_FILE
