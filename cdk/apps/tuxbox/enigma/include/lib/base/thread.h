#ifndef __lib_base_thread_h
#define __lib_base_thread_h

#include <pthread.h>

class eThread
{
	pthread_t the_thread;
	static void *wrapper(void *ptr);
	int alive;
	static void thread_completed(void *p);
public:
	bool thread_running() { return alive; }
	eThread();
	virtual ~eThread();

	void run(int prio=0, int policy=0);

	virtual void thread()=0;
	virtual void thread_finished() { }

	void kill(bool sendcancel=false);
};

#endif
