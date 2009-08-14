#include "ir.h"

ir::ir()
{

}

void ir::writeCommand(std::string cmd)
{
	write(fd, cmd.c_str(), cmd.length());
}

void ir::sendCommand(std::string cmd)
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
	std::stringstream ostr;

	ostr << "SEND_ONCE " << device << " " << cmd << std::endl << std::ends;
	writeCommand(ostr.str());

	close(fd);
}
