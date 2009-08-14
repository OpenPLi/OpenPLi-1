#ifndef DISABLE_LIRC

#include <elirc.h>

#include <sys/socket.h>
#include <sstream>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/service.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/epgcache.h>
#include <lib/gui/enumber.h>
#include <lib/gui/statusbar.h>

ELirc::ELirc()
	:timeout(eApp)
{
}

ELirc::~ELirc()
{
}
 
void ELirc::sendcommand(eString cmd)
{
	std::stringstream ostr;

	ostr << "SEND_ONCE " << device << " " << cmd << std::endl << std::ends;
	write(fd, ostr.str().c_str(), ostr.str().length());
	std::cout << "[elirc.cpp]Sending: " << ostr.str() << std::endl;
}

void ELirc::sendcommandlist(eString filename)
{
	struct sockaddr_un addr;

	addr.sun_family=AF_UNIX;
	strcpy(addr.sun_path, "/dev/lircd");
	fd=socket(AF_UNIX,SOCK_STREAM,0);
	if(fd==-1)
	{
		perror("could not open lircd-socket\n");
		return;
	};

	if(connect(fd,(struct sockaddr *)&addr,sizeof(addr))==-1)
	{
		perror("could not connect to lircd-socket\n");
		return;
	};
	std::ifstream inFile;

	inFile.open(filename.c_str());
	if (!inFile)
	{
		perror(filename.c_str());
		// TODO: Handly errors here
	}
	eString tmp_string;
	getline(inFile, device);
	while(getline(inFile, tmp_string))
	{
		if (tmp_string != "SLEEP_A_WHILE")
		{
			std::cout << "Sending Command" << std::endl;
			sendcommand(tmp_string);
		}
		else
		{
			std::cout << "Sleeping" << std::endl;

			sleep(1);
		}
	}
	inFile.close();

	close(fd);
}

void ELirc::sendstart()
{
	eDebug("lirc sendstart requested");

	sendcommandlist(CONFIGDIR"/ir_vcr_start.lirc");
}

void ELirc::sendstop()
{
	eDebug("lirc sendstop requested");

	sendcommandlist(CONFIGDIR"/ir_vcr_stop.lirc");
}


#endif // DISABLE_LIRC
