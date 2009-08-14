
#ifndef __MYTIMER_H__
#define __MYTIMER_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "libsig_comp.h"


class CTimer
{
	static void* ThreadTimer(void*);
public:
	static CTimer *getInstance();
	Signal0<void>selected;
	static void start(int val);
	~CTimer();
};

#endif /* __MYTIMER_H__ */
