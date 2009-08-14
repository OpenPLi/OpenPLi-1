#include "hotplug.h"

#include <unistd.h>
#include <lib/base/eerror.h>

eHotplug *eHotplug::instance = 0;

eHotplug::eHotplug()
	:paramsleft(0)
{
	init_eHotplug();
}
void eHotplug::init_eHotplug()
{
	if (!instance)
		instance=this;
	else
		eWarning("MORE THAN ONE INSTANCES OF eHotplug created!!");
	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	servaddr.sun_family = AF_UNIX;
	unlink("/tmp/hotplug.socket");
	strcpy(servaddr.sun_path, "/tmp/hotplug.socket");
	clilen = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);
	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("[hotplug] socket");
		return;
	}
	if (bind(listenfd, (struct sockaddr *) &servaddr, clilen) < 0)
	{
		perror("[hotplug] bind");
		return;
	}
	if (listen(listenfd, 5) < 0)
	{
		perror("[hotplug] listen");
		return;
	}
	sn = new eSocketNotifier( eApp, listenfd, 17 ); // POLLIN/POLLPRI/POLLHUP
	sn->start();
	CONNECT( sn->activated, eHotplug::dataAvail );
	eDebug("[eHotplug] created successfully");
}

void eHotplug::dataAvail(int what)
{
	switch(what)
	{
		case POLLIN:
		case POLLPRI:
		{
			unsigned char msgbuffer[1024];
			int connfd =
				accept(listenfd, (struct sockaddr *) &servaddr, (socklen_t *) &clilen);
			ssize_t length = read(connfd, msgbuffer, sizeof(msgbuffer));
			if ( !length )
				break;
			eString tmp((char*)msgbuffer, length);
			if ( tmp.find("LENGTH = ") != eString::npos )
			{
				paramsleft=tmp[9];
				params.clear();
			}
			else
			{
				unsigned int pos = tmp.find("=");
				if ( pos == eString::npos )
					break;
				params[tmp.left(pos)]=tmp.mid(pos+1, tmp.length()-pos+1);
				paramsleft--;
			}
			close(connfd);
			if ( !paramsleft )
			{
				for (std::map<eString,eString>::iterator it( params.begin() ); it != params.end(); it++ )
					eDebug("%s = %s", it->first.c_str(), it->second.c_str() );
			}
			break;
		}
		default:
			break;
	}
}

eHotplug::~eHotplug()
{
	delete sn;
	unlink("/tmp/hotplug.socket");
}
