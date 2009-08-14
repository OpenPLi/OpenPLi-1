#include <map>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <libsig_comp.h>
#include <lib/base/ebase.h>
#include <lib/base/estring.h>

class eHotplug: public Object
{
	int listenfd, connfd, clilen;
	struct sockaddr_un servaddr;
	static eHotplug *instance;
	eSocketNotifier *sn;
	void dataAvail(int what);
	int paramsleft;
	std::map<eString,eString> params;
	void init_eHotplug();
public:
	eHotplug *getInstance() { return instance; }
	eHotplug();
	~eHotplug();
};

