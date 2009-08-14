#ifdef HAVE_DREAMBOX_HARDWARE
#include "my_rc.h"

RcInput::RcInput()
{
	char buf[32];

	fd_rc=open(RC_DEVICE, O_RDONLY);
	if (fd_rc<0)
	{
		perror(RC_DEVICE);
		exit(-1);
	}
	fcntl(fd_rc, F_SETFL, O_NONBLOCK );
	read( fd_rc, buf, 32 );
	
	if (pthread_create (&thrRc, NULL, ThreadRc, (void *) fd_rc) != 0 )
		perror("[RcInput] pthread_created error");

	//bu
	fd_bu=open(BUTTON_DEVICE, O_RDONLY);
	if (fd_bu<0)
	{
		perror(BUTTON_DEVICE);
		exit(-1);
	}
	if (pthread_create (&thrBu, NULL, ThreadBu, (void *) fd_bu) != 0 )
		perror("[ButtonInput] pthread_created error");
}

void* RcInput::ThreadBu(void * fd_bu)
{
	printf("[ButtonInput] available\n");
	static unsigned short bucode=0;
	unsigned short rccode=0;
	while (1)
	{
		if (read((int)fd_bu, &bucode, 2)==2)
			if(bucode != BUTTON_BREAK)
			{
				switch(bucode)
				{
					case BUTTON_EXIT:	rccode = RC_EXIT;	break;
					case BUTTON_UP:		rccode = RC_UP;		break;
					case BUTTON_DOWN:	rccode = RC_DOWN;	break;
				}
				RcInput::getInstance()->selected(rccode);
			}

		usleep(10000);//repeattimer
	}

	pthread_exit(0);
	return NULL;
}

void* RcInput::ThreadRc(void * fd_rc)
{
	printf("[RCInput] available\n");
	static unsigned short rccode=0;
	while (1)
	{
		if (read((int)fd_rc, &rccode, 2)==2)
			if (rccode != RC_BREAK && rccode < 0x8000)
				RcInput::getInstance()->selected(rccode);

		usleep(10000);//repeattimer
	}

	pthread_exit(0);
	return NULL;
}
RcInput::~RcInput()
{
	pthread_exit(0);
	if (fd_rc >= 0) close(fd_rc);
	if (fd_bu >= 0) close(fd_bu);
}

RcInput* RcInput::getInstance()
{
	static RcInput* rcinput = NULL;
	if(rcinput == NULL) { rcinput = new RcInput(); }
	return rcinput;
}
#endif
