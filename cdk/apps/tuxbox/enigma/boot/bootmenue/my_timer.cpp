#ifdef HAVE_DREAMBOX_HARDWARE
#include "my_timer.h"

pthread_t thrTimer = 0;

void CTimer::start(int val)
{
	if(thrTimer)
	{
		pthread_cancel(thrTimer);
		pthread_join(thrTimer,NULL);
	}
	if (pthread_create (&thrTimer, NULL, ThreadTimer, (void *)val) != 0 )
		perror("[TIMER] pthread_created error");
}

void *CTimer::ThreadTimer(void *val)
{
	sleep((int) val);
	CTimer::getInstance()->selected();
	pthread_exit(0);
	return NULL;
}

CTimer::~CTimer()
{
	pthread_exit(0);
}

CTimer *CTimer::getInstance()
{
	static CTimer* mytimer = NULL;
	if (mytimer == NULL) { mytimer = new CTimer(); }
	return mytimer;
}
#endif
