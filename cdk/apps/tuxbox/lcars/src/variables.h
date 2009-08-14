#ifndef VARIABLES_H
#define VARIABLES_H

#include <pthread.h>
#include <map>
#include <queue>
#include <string>
#include <sstream>
#include <pthread.h>

class variables
{
	pthread_mutex_t mutex;
	pthread_mutex_t events_mutex;
	pthread_mutex_t event_wait_mutex;

	std::map<std::string, std::string> vars;
	std::queue<std::string> events;
public:
	variables();
	std::string getvalue(std::string name);
	void setvalue(std::string name, std::string value);
	void setvalue(std::string name, int value);
	bool isavailable(std::string name);

	void addEvent(std::string event);
	std::string waitForEvent();
};

#endif
