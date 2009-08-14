/***************************************************************************
    copyright            : (C) 2001 by TheDOC
    email                : thedoc@chatville.de
	homepage			 : www.chatville.de
	modified by			 : -
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
$Log: rc.cpp,v $
Revision 1.10.2.1.2.1  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.10.2.1  2003/02/28 14:58:13  thedoc
Fix for compiler-warning in rel

Revision 1.10  2003/01/05 20:49:55  TheDOC
and the old rc-devices

Revision 1.9  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.8  2003/01/05 02:41:53  TheDOC
lcars supports inputdev now

Revision 1.7  2002/10/20 02:03:37  TheDOC
Some fixes and stuff

Revision 1.6  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.5  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.4  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.5  2001/12/17 16:54:47  tux
Settings halb komplett

Revision 1.4  2001/12/17 14:00:41  tux
Another commit

Revision 1.3  2001/12/17 03:52:42  tux
Netzwerkfernbedienung fertig

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/

#include <stdio.h>
#include "rc.h"
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#ifdef HAVE_DREAMBOX_HARDWARE
#include <dbox/fp.h>
#endif
#ifdef HAVE_LINUX_DVB_VERSION_H

rc::rc(hardware *h, settings *s)
{
	setting = s;
	rcstop = false;
	hardware_obj = h;

	fp = open("/dev/input/event0", O_RDONLY);
	if (fp < 0)
	{
		perror("Could not open input device");
		exit(1);
	}

	int rcs[NUMBER_RCS][25] =
	    {
	        {
	            RC_0,
	            RC_1,
	            RC_2,
	            RC_3,
	            RC_4,
	            RC_5,
	            RC_6,
	            RC_7,
	            RC_8,
	            RC_9,
	            RC_STANDBY,
	            RC_HOME,
	            RC_MENU,
	            RC_RED,
	            RC_GREEN,
	            RC_YELLOW,
	            RC_BLUE,
	            RC_OK,
	            RC_VOLPLUS,
	            RC_MUTE,
	            RC_HELP,
	            RC_UP,
	            RC_DOWN,
	            RC_RIGHT,
	            RC_LEFT
	        }
	    };
	//rc_codes = rcs;
	for (int i = 0; i < NUMBER_RCS; i++)
	{
		for (int j = 0; j < 25; j++)
		{
			rc_codes[i][j] = rcs[i][j];
		}
	}
}

rc::~rc()
{
	close(fp);
}

int rc::start_thread()
{

	int status;

	pthread_mutex_init(&mutex, NULL);
	status = pthread_create( &rcThread,
	                         NULL,
	                         start_rcqueue,
	                         (void *)this );

	return status;

}

void* rc::start_rcqueue( void * this_ptr )
{
	rc *r = (rc *) this_ptr;

	while(1)
	{
		if (!r->rcstop)
			pthread_mutex_unlock( &r->blockingmutex );
		else
			sleep(1);
		pthread_mutex_lock( &r->blockingmutex );

		r->key = r->read_from_rc2();
		//std::cout << "Key: " << r->key << std::endl;
	}
	return 0;
}

int rc::parseKey(std::string key)
{
	if (key == "1")
	{
		return RC_1;
	}
	else if (key == "2")
	{
		return RC_2;
	}
	else if (key == "3")
	{
		return RC_3;
	}
	else if (key == "4")
	{
		return RC_4;
	}
	else if (key == "5")
	{
		return RC_5;
	}
	else if (key == "6")
	{
		return RC_6;
	}
	else if (key == "7")
	{
		return RC_7;
	}
	else if (key == "8")
	{
		return RC_8;
	}
	else if (key == "9")
	{
		return RC_9;
	}
	else if (key == "0")
	{
		return RC_0;
	}
	else if (key == "STANDBY")
	{
		return RC_STANDBY;
	}
	else if (key == "HOME")
	{
		return RC_HOME;
	}
	else if (key == "MENU")
	{
		return RC_MENU;
	}
	else if (key == "RED")
	{
		return RC_RED;
	}
	else if (key == "GREEN")
	{
		return RC_GREEN;
	}
	else if (key == "YELLOW")
	{
		return RC_YELLOW;
	}
	else if (key == "BLUE")
	{
		return RC_BLUE;
	}
	else if (key == "OK")
	{
		return RC_OK;
	}
	else if (key == "VOLPLUS")
	{
		return RC_VOLPLUS;
	}
	else if (key == "VOLMINUS")
	{
		return RC_VOLMINUS;
	}
	else if (key == "MUTE")
	{
		return RC_MUTE;
	}
	else if (key == "HELP")
	{
		return RC_HELP;
	}
	else if (key == "UP")
	{
		return RC_UP;
	}
	else if (key == "DOWN")
	{
		return RC_DOWN;
	}
	else if (key == "RIGHT")
	{
		return RC_RIGHT;
	}
	else if (key == "LEFT")
	{
		return RC_LEFT;
	}
	return -1;
}

void rc::cheat_command(unsigned short cmd)
{
	key = cmd;
	last_read = cmd;
	//std::cout << "Command: " << cmd << std::endl;
	pthread_mutex_unlock( &blockingmutex );
	usleep(100);
	pthread_mutex_lock( &blockingmutex );
}

void rc::stoprc()
{
	rcstop = true;
	pthread_mutex_unlock( &blockingmutex );
	pthread_mutex_lock( &blockingmutex );
}

void rc::startrc()
{
	rcstop = false;
	pthread_mutex_unlock( &blockingmutex );
}

void rc::restart()
{
	close(fp);
	fp = open("/dev/input/event0", O_RDONLY);
}

unsigned short rc::get_last()
{
	return last_read;
}

unsigned short rc::read_from_rc()
{
	usleep(100);
	if (key == -1)
	{
		pthread_mutex_lock( &blockingmutex );
		pthread_mutex_unlock( &blockingmutex );
	}
	//std::cout << "KEY: " << key << std::endl;
	int returnkey = key;
	key = -1;
	return returnkey;
}

unsigned short rc::read_from_rc2()
{
	struct input_event read_code;
	int rd;

	pthread_mutex_lock( &mutex );

	do
	{
		rd = read(fp, &read_code, sizeof(struct input_event));
	} while(read_code.value != 1);

	if (rd < (int) sizeof(struct input_event))
	{
		perror("[rc.cpp] Error reading input-event");
		return 0;
	}
	last_read = read_code.code;

	pthread_mutex_unlock( &mutex );

	return read_code.code;
}

int rc::get_number()
{
	int i, codes, number = -1;

	for (i = 0; i < NUMBER_RCS; i++)
	{
		for (codes = 0; codes < 10; codes++)
		{
			if (last_read == rc_codes[i][codes])
				number = codes;
		}
	}
	return number;
}


int rc::command_available()
{
	return (key != -1);
}

#elif HAVE_OST_DMX_H

rc::rc(hardware *h, settings *s)
{
	setting = s;
	rcstop = false;
	hardware_obj = h;

	fp = open("/dev/dbox/rc0", O_RDONLY);

	int rcs[NUMBER_RCS][25] =
	    {
	        {
	            RC1_0,
	            RC1_1,
	            RC1_2,
	            RC1_3,
	            RC1_4,
	            RC1_5,
	            RC1_6,
	            RC1_7,
	            RC1_8,
	            RC1_9,
	            RC1_STANDBY,
	            RC1_HOME,
	            RC1_MENU,
	            RC1_RED,
	            RC1_GREEN,
	            RC1_YELLOW,
	            RC1_BLUE,
	            RC1_OK,
	            RC1_VOLPLUS,
	            RC1_MUTE,
	            RC1_HELP,
	            RC1_UP,
	            RC1_DOWN,
	            RC1_RIGHT,
	            RC1_LEFT
	        },
	        {
	            RC2_0,
	            RC2_1,
	            RC2_2,
	            RC2_3,
	            RC2_4,
	            RC2_5,
	            RC2_6,
	            RC2_7,
	            RC2_8,
	            RC2_9,
	            RC2_STANDBY,
	            RC2_HOME,
	            RC2_MENU,
	            RC2_RED,
	            RC2_GREEN,
	            RC2_YELLOW,
	            RC2_BLUE,
	            RC2_OK,
	            RC2_VOLPLUS,
	            RC2_MUTE,
	            RC2_HELP,
	            RC2_UP,
	            RC2_DOWN,
	            RC2_RIGHT,
	            RC2_LEFT
	        }
	    };
	//rc_codes = rcs;
	for (int i = 0; i < NUMBER_RCS; i++)
	{
		for (int j = 0; j < 25; j++)
		{
			rc_codes[i][j] = rcs[i][j];
		}
	}
}

rc::~rc()
{
	close(fp);
}

int rc::start_thread(bool withoutkeyboard)
{

	int status;

	pthread_mutex_init(&mutex, NULL);
	status = pthread_create( &rcThread,
	                         NULL,
	                         start_rcqueue,
	                         (void *)this );

	if (!withoutkeyboard)
		status = pthread_create( &keyboardThread,
		                         NULL,
		                         start_keyboardqueue,
		                         (void *)this );

	return status;

}

void* rc::start_rcqueue( void * this_ptr )
{
	rc *r = (rc *) this_ptr;

	while(1)
	{
		if (!r->rcstop)
			pthread_mutex_unlock( &r->blockingmutex );
		else
			sleep(1);
		pthread_mutex_lock( &r->blockingmutex );

		r->key = r->read_from_rc2();
		//std::cout << "Key: " << r->key << std::endl;
	}
}

int rc::parseKey(std::string key)
{
	if (key == "1")
	{
		return RC1_1;
	}
	else if (key == "2")
	{
		return RC1_2;
	}
	else if (key == "3")
	{
		return RC1_3;
	}
	else if (key == "4")
	{
		return RC1_4;
	}
	else if (key == "5")
	{
		return RC1_5;
	}
	else if (key == "6")
	{
		return RC1_6;
	}
	else if (key == "7")
	{
		return RC1_7;
	}
	else if (key == "8")
	{
		return RC1_8;
	}
	else if (key == "9")
	{
		return RC1_9;
	}
	else if (key == "0")
	{
		return RC1_0;
	}
	else if (key == "STANDBY")
	{
		return RC1_STANDBY;
	}
	else if (key == "HOME")
	{
		return RC1_HOME;
	}
	else if (key == "MENU")
	{
		return RC1_MENU;
	}
	else if (key == "RED")
	{
		return RC1_RED;
	}
	else if (key == "GREEN")
	{
		return RC1_GREEN;
	}
	else if (key == "YELLOW")
	{
		return RC1_YELLOW;
	}
	else if (key == "BLUE")
	{
		return RC1_BLUE;
	}
	else if (key == "OK")
	{
		return RC1_OK;
	}
	else if (key == "VOLPLUS")
	{
		return RC1_VOLPLUS;
	}
	else if (key == "VOLMINUS")
	{
		return RC1_VOLMINUS;
	}
	else if (key == "MUTE")
	{
		return RC1_MUTE;
	}
	else if (key == "HELP")
	{
		return RC1_HELP;
	}
	else if (key == "UP")
	{
		return RC1_UP;
	}
	else if (key == "DOWN")
	{
		return RC1_DOWN;
	}
	else if (key == "RIGHT")
	{
		return RC1_RIGHT;
	}
	else if (key == "LEFT")
	{
		return RC1_LEFT;
	}
	return -1;
}

void* rc::start_keyboardqueue( void * this_ptr )
{
	rc *r = (rc *) this_ptr;
	struct termios t,ot;

	tcgetattr(0,&t);
	t.c_lflag &= ~(ECHO | ICANON | ECHOK | ECHOE | ECHONL);
	ot = t;
	tcsetattr(0,TCSANOW,&t);
	while(1)
	{
		char character;

		std::cin.get(character);

		if (character > 47 && character < 58)
			r->cheat_command(character - 48);
		switch(character)
		{
		case 10:
			r->cheat_command(RC1_OK);
			break;
		case 65:
			r->cheat_command(RC1_UP);
			break;
		case 66:
			r->cheat_command(RC1_DOWN);
			break;
		case 67:
			r->cheat_command(RC1_RIGHT);
			break;
		case 68:
			r->cheat_command(RC1_LEFT);
			break;
		case 113: // q
			r->cheat_command(RC1_RED);
			break;
		case 119: // w
			r->cheat_command(RC1_GREEN);
			break;
		case 101: // e
			r->cheat_command(RC1_YELLOW);
			break;
		case 114: // r
			r->cheat_command(RC1_BLUE);
			break;
		case 32: // SPACE
			r->cheat_command(RC1_MUTE);
			break;
		case 8: // BACKSPACE
			r->cheat_command(RC1_HOME);
			break;


		}
		//printf("%d\n", character);
	}
}

void rc::cheat_command(unsigned short cmd)
{
	key = cmd;
	last_read = cmd;
	//std::cout << "Command: " << cmd << std::endl;
	pthread_mutex_unlock( &blockingmutex );
	usleep(100);
	pthread_mutex_lock( &blockingmutex );
}

void rc::stoprc()
{
	rcstop = true;
	pthread_mutex_unlock( &blockingmutex );
	pthread_mutex_lock( &blockingmutex );
}

void rc::startrc()
{
	rcstop = false;
	pthread_mutex_unlock( &blockingmutex );
}

void rc::restart()
{
	close(fp);
	fp = open("/dev/dbox/rc0", O_RDONLY);
}

unsigned short rc::read_from_rc_raw()
{
	unsigned short read_code = 0;

	read(fp, &read_code, 2);

	last_read = read_code;

	return read_code;
}

unsigned short rc::get_last()
{
	return last_read;
}

unsigned short rc::convert_code(unsigned short rc)
{
	return rc % 0x40;
}

unsigned short rc::read_from_rc()
{
	usleep(100);
	if (key == -1)
	{
		pthread_mutex_lock( &blockingmutex );
		pthread_mutex_unlock( &blockingmutex );
	}
	//std::cout << "KEY: " << key << std::endl;
	int returnkey = key;
	key = -1;
	return returnkey;
}

unsigned short rc::read_from_rc2()
{
	unsigned short read_code = 0;

	pthread_mutex_lock( &mutex );

	if (!setting->getRCRepeat())
	{
		do
		{
			read(fp, &read_code, 2);
			//printf("RC: %x\n", read_code);
			if (setting->getSupportOldRc())
				read_code = old_to_new(read_code);
		} while (read_code == last_read || (read_code & 0xff00) == 0x5c00);
	}
	else
	{
		do
		{
			read(fp, &read_code, 2);
			//printf("RC: %x\n", read_code);
			if (setting->getSupportOldRc())
				read_code = old_to_new(read_code);
		} while ((read_code & 0xff00) == 0x5c00);

	}

	last_read = read_code;

	read_code %= 0x40;

	pthread_mutex_unlock( &mutex );

	return read_code;
}

int rc::old_to_new(int read_code)
{
	switch (read_code)
	{
	case RC2_1:
		return RC1_1;
	case RC2_2:
		return RC1_2;
	case RC2_3:
		return RC1_3;
	case RC2_4:
		return RC1_4;
	case RC2_5:
		return RC1_5;
	case RC2_6:
		return RC1_6;
	case RC2_7:
		return RC1_7;
	case RC2_8:
		return RC1_8;
	case RC2_9:
		return RC1_9;
	case RC2_0:
		return RC1_0;
	case RC2_STANDBY:
		return RC1_STANDBY;
	case RC2_HOME:
		return RC1_HOME;
	case RC2_MENU:
		return RC1_MENU;
	case RC2_RED:
		return RC1_RED;
	case RC2_GREEN:
		return RC1_GREEN;
	case RC2_YELLOW:
		return RC1_YELLOW;
	case RC2_BLUE:
		return RC1_BLUE;
	case RC2_OK:
		return RC1_OK;
	case RC2_VOLPLUS:
		return RC1_VOLPLUS;
	case RC2_VOLMINUS:
		return RC1_VOLMINUS;
	case RC2_MUTE:
		return RC1_MUTE;
	case RC2_HELP:
		return RC1_HELP;
	case RC2_UP:
		return RC1_UP;
	case RC2_DOWN:
		return RC1_DOWN;
	case RC2_RIGHT:
		return RC1_RIGHT;
	case RC2_LEFT:
		return RC1_LEFT;


	}
	return read_code;
}

int rc::get_number()
{
	int i, codes, number = -1;

	for (i = 0; i < NUMBER_RCS; i++)
	{
		for (codes = 0; codes < 10; codes++)
		{
			if (convert_code(last_read) == rc_codes[i][codes])
				number = codes;
		}
	}
	return number;
}


int rc::command_available()
{
	return (key != -1);
}


#endif
