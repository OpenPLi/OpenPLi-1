#ifndef __ENIGMA_MMISOCKET_H_
#define __ENIGMA_MMISOCKET_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <map>
#include <lib/dvb/dvbservice.h>
#include <enigma_mmi.h>
#include <enigma_main.h>
#include <enigma_setup.h>

class eSocketMMIHandler: public Object
{
	int listenfd, connfd, clilen;
	struct sockaddr_un servaddr;
	eSocketNotifier *listensn, *connsn;
	void listenDataAvail(int what);
	void connDataAvail(int what);
	void initiateMMI();
	void setupOpened( eSetupWindow *setup, int *entrynum );
	void closeConn();
	const char *sockname;
	char *name;
	void init_eSocketMMIHandler();
public:
	const char *getName() const { return name; }
	Signal2<void, const char*, int> mmi_progress;
	int send_to_mmisock( void *, size_t );
	eSocketMMIHandler();
	~eSocketMMIHandler();
};

class eSocketMMI : public enigmaMMI
{
	eSocketMMIHandler *handler;
	void beginExec();
	void sendAnswer( AnswerType ans, int param, unsigned char *data );
	static std::map<eSocketMMIHandler*,eSocketMMI*> exist;
	void init_eSocketMMI();
public:
	static eSocketMMI *getInstance( eSocketMMIHandler *handler );
	eSocketMMI(eSocketMMIHandler *handler);
};

#endif // __ENIGMA_MMISOCKET_H_
