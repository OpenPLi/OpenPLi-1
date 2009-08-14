#ifndef IR_H
#define IR_H

#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>

class ir
{
	int fd;
	std::string device;
	void writeCommand(std::string);
public:
	ir();
	void setDevice(std::string dev) { device = dev; }
	void sendCommand(std::string cmd);
};

#endif
