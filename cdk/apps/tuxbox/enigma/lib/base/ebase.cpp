#include <lib/base/ebase.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <lib/base/eerror.h>
#include <lib/system/elock.h>

eSocketNotifier::eSocketNotifier(eMainloop *context, int fd, int requested, bool startnow): context(*context), fd(fd), state(0), requested(requested)
{
	if (startnow)	
		start();
}

eSocketNotifier::~eSocketNotifier()
{
	stop();
}

void eSocketNotifier::start()
{
	if (state)
		stop();

	context.addSocketNotifier(this);
	state=2;  // running but not in poll yet
}

void eSocketNotifier::stop()
{
	if (state)
		context.removeSocketNotifier(this);

	state=0;
}

					// timer
void eTimer::start(long msek, bool singleShot)
{
	if (bActive)
		stop();

	bActive = true;
	bSingleShot = singleShot;
	interval = msek;
	gettimeofday(&nextActivation, 0);
	nextActivation.tv_sec -= context.getTimerOffset();
	nextActivation += (msek<0 ? 0 : msek);
//	eDebug("this = %p\nnow sec = %d, usec = %d\nadd %d msec", this, nextActivation.tv_sec, nextActivation.tv_usec, msek);
//	eDebug("next Activation sec = %d, usec = %d", nextActivation.tv_sec, nextActivation.tv_usec );
	context.addTimer(this);
}

void eTimer::restart()
{
	if (bActive)
		stop();

	bActive = true;

	gettimeofday(&nextActivation, 0);
	nextActivation.tv_sec -= context.getTimerOffset();
	nextActivation += (interval<0 ? 0 : interval);
	context.addTimer(this);
}

void eTimer::startLongTimer( int seconds )
{
	if (bActive)
		stop();

	bActive = bSingleShot = true;
	interval = 0;
	gettimeofday(&nextActivation, 0);
	nextActivation.tv_sec -= context.getTimerOffset();
//	eDebug("this = %p\nnow sec = %d, usec = %d\nadd %d msec", this, nextActivation.tv_sec, nextActivation.tv_usec, msek);
	if ( seconds > 0 )
		nextActivation.tv_sec += seconds;
//	eDebug("next Activation sec = %d, usec = %d", nextActivation.tv_sec, nextActivation.tv_usec );
	context.addTimer(this);
}

void eTimer::stop()
{
	if (bActive)
	{
		bActive=false;
		context.removeTimer(this);
	}
}

void eTimer::changeInterval(long msek)
{
	if (bActive)  // Timer is running?
	{
		context.removeTimer(this);	 // then stop
		nextActivation -= interval;  // sub old interval
	}
	else
		bActive=true; // then activate Timer

	interval = msek;   			 			// set new Interval
	nextActivation += interval;		// calc nextActivation

	context.addTimer(this);				// add Timer to context TimerList
}

void eTimer::activate()   // Internal Funktion... called from eApplication
{
#if 0
	timeval now;
	gettimeofday(&now, 0);
	eDebug("this = %p\nnow sec = %d, usec = %d\nnextActivation sec = %d, usec = %d",
	this,
	now.tv_sec,
	now.tv_usec,
	nextActivation.tv_sec,
	nextActivation.tv_usec );
	eDebug("Timer emitted");
#endif
	context.removeTimer(this);

	if (!bSingleShot)
	{
		nextActivation += interval;
		context.addTimer(this);
	}
	else
		bActive=false;

	/*emit*/ timeout();
}

inline void eTimer::recalc( int offset )
{
	nextActivation.tv_sec += offset;
}

// mainloop

ePtrList<eMainloop> eMainloop::existing_loops;

void eMainloop::setTimerOffset( int difference )
{
	singleLock s(recalcLock);
	if (!TimerList)
		timer_offset=0;
	else
	{
		if ( timer_offset )
			eDebug("time_offset %d avail.. add new offset %d than new is %d",
			timer_offset, difference, timer_offset+difference);
		timer_offset+=difference;
	}
}

void eMainloop::addSocketNotifier(eSocketNotifier *sn)
{
	notifiers.insert(std::pair<int,eSocketNotifier*> (sn->getFD(), sn));
}

void eMainloop::removeSocketNotifier(eSocketNotifier *sn)
{
	notifiers.erase(sn->getFD());
}

void eMainloop::processOneEvent()
{
	int ret;

	/* process pending timers... */
	long usec = 0;

	if (TimerList)
		doRecalcTimers();
	while (TimerList && (usec = timeout_usec( TimerList.begin()->getNextActivation() ) ) <= 0 )
	{
		TimerList.begin()->activate();
		doRecalcTimers();
	}

	int fdAnz = notifiers.size();
	pollfd pfd[fdAnz];

// fill pfd array
	std::map<int,eSocketNotifier*>::iterator it(notifiers.begin());
	for (int i=0; i < fdAnz; i++, it++)
	{
		it->second->state = 1; // set state to in poll (2 is running but not in poll)
		pfd[i].fd = it->first;
		pfd[i].events = it->second->getRequested();
	}

	ret = poll(pfd, fdAnz, TimerList ? usec / 1000 : -1);  // milli .. not micro seks

	if (ret < 0 && errno != EINTR)
		eDebug("poll made error");
	else if (ret > 0) // leaved poll with revent fds
	{
	//		eDebug("bin aussem poll raus und da war was");
		for (int i=0; i < fdAnz && ret; i++)
		{
			if (pfd[i].revents)
			{
				it = notifiers.find(pfd[i].fd);
				--ret;
				if (it != notifiers.end()
					&& it->second->state == 1) // added and in poll
				{
					int req = it->second->getRequested();
					if (pfd[i].revents & req)
						it->second->activate(pfd[i].revents & req);
					pfd[i].revents &= ~req;
				}
				if (pfd[i].revents & (POLLERR|POLLHUP|POLLNVAL))
					eDebug("poll: unhandled POLLERR/HUP/NVAL for fd %d(%d)", pfd[i].fd, pfd[i].revents);
			}
		}
	}
	else // timeouted leave poll
	{
		// here we do nothing...
		// timers are checked on next call of processOneEvent
	}
}

int eMainloop::exec()
{
	if (!loop_level)
	{
		app_quit_now = false;
		app_exit_loop = false;
		enter_loop();
	}
	return retval;
}

void eMainloop::enter_loop()
{
	loop_level++;
	// Status der vorhandenen Loop merken
	bool old_exit_loop = app_exit_loop;

	app_exit_loop = false;

	while (!app_exit_loop && !app_quit_now)
		processOneEvent();

	// wiederherstellen der vorherigen app_exit_loop
	app_exit_loop = old_exit_loop;

	--loop_level;

	if (!loop_level)
	{
		// do something here on exit the last loop
	}
}

void eMainloop::exit_loop()  // call this to leave the current loop
{
	app_exit_loop = true;
}

void eMainloop::quit( int ret )   // call this to leave all loops
{
	retval=ret;
	app_quit_now = true;
}

inline void eMainloop::doRecalcTimers()
{
	singleLock s(recalcLock);
	if ( timer_offset )
	{
		for (ePtrList<eTimer>::iterator it(TimerList); it != TimerList.end(); ++it )
			it->recalc( timer_offset );
		timer_offset=0;
	}
}

eApplication* eApp = 0;
