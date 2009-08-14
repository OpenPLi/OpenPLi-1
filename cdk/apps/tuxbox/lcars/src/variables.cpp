#include "variables.h"

#include <iostream>
#include <unistd.h>



variables::variables()
{
	vars.clear();
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&events_mutex, NULL);
	pthread_mutex_init(&event_wait_mutex, NULL);
	pthread_mutex_lock(&event_wait_mutex);
}

std::string variables::getvalue(std::string name)
{
	std::string value;
	pthread_mutex_lock(&mutex);
	if (vars.count(name) < 1)
		value = "NONE FOUND";
	else
		value = vars.find(name)->second;
	pthread_mutex_unlock(&mutex);
	return value;
}

void variables::setvalue(std::string name, int value)
{
	std::string val;
	std::stringstream ostr;

	ostr << value << std::ends;
	val = ostr.str();
	setvalue(name, val);
}

void variables::setvalue(std::string name, std::string value)
{
	pthread_mutex_lock(&mutex);
	vars[name] = value;
	pthread_mutex_unlock(&mutex);
}

bool variables::isavailable(std::string name)
{
	pthread_mutex_lock(&mutex);
	bool tmp = (vars.count(name) > 0);
	pthread_mutex_unlock(&mutex);
	return tmp;
}

void variables::addEvent(std::string event)
{
	pthread_mutex_lock(&events_mutex);
	events.push(event);
	pthread_mutex_unlock(&events_mutex);
	pthread_mutex_unlock(&event_wait_mutex);
}

std::string variables::waitForEvent()
{
	std::string return_string;
	bool doit = true;

	while(doit)
	{
		pthread_mutex_lock(&events_mutex);
		doit = (events.size() == 0);
		pthread_mutex_unlock(&events_mutex);
		usleep(1000);
	}
	if (doit)
		pthread_mutex_lock(&event_wait_mutex);
	pthread_mutex_lock(&events_mutex);
	return_string = events.front();
	events.pop();
	pthread_mutex_unlock(&events_mutex);
	return return_string;
}

