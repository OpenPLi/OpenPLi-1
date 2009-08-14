/*
 * $Header: /cvs/tuxbox/apps/tuxbox/neutrino/daemons/sectionsd/dmx.cpp,v 1.2.2.3 2003/02/17 19:26:13 thegoodguy Exp $
 *
 * DMX class (sectionsd) - d-box2 linux project
 *
 * (C) 2001 by fnbrd,
 *     2003 by thegoodguy <thegoodguy@berlios.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include <dmx.h>
#include <dmxapi.h>
#include <debug.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <string>


extern int readNbytes(int fd, char *buf, const size_t n, unsigned timeoutInMSeconds);
extern void showProfiling(std::string text);
extern bool timeset;


DMX::DMX(const unsigned char p, const unsigned short bufferSizeInKB, const bool nCRC)
{
	fd = 0;
	lastChanged = 0;
	filter_index = 0;
	pID = p;
	dmxBufferSizeInKB = bufferSizeInKB;
	noCRC = nCRC;
	pthread_mutex_init(&pauselock, NULL);        // default = fast mutex
	pthread_mutex_init(&start_stop_mutex, NULL); // default = fast mutex
	pthread_cond_init (&change_cond, NULL);
	pauseCounter = 0;
	real_pauseCounter = 0;
}

DMX::~DMX()
{
	closefd();
	pthread_mutex_destroy(&pauselock);
	pthread_mutex_destroy(&start_stop_mutex);
	pthread_cond_destroy (&change_cond);
}

int DMX::read(char *buf, const size_t buflength, unsigned timeoutMInSeconds)
{
	return readNbytes(fd, buf, buflength, timeoutMInSeconds);
}

void DMX::closefd(void)
{
	if (fd)
	{
		close(fd);
		fd = 0;
	}
}

void DMX::addfilter(const unsigned char filter, const unsigned char mask)
{
	s_filters tmp;
	tmp.filter = filter;
	tmp.mask   = mask;
	filters.push_back(tmp);
}

int DMX::stop(void)
{
	if (!fd)
		return 1;
	
	pthread_mutex_lock(&start_stop_mutex);
	
	if (real_pauseCounter == 0)
		closefd();
	
	pthread_mutex_unlock(&start_stop_mutex);
	
	return 0;
}

void DMX::lock(void)
{
	pthread_mutex_lock(&start_stop_mutex);
}

void DMX::unlock(void)
{
	pthread_mutex_unlock(&start_stop_mutex);
}

bool DMX::isOpen(void)
{
	return fd ? true : false;
}

char * DMX::getSection(const unsigned timeoutInMSeconds, int &timeouts)
{
	struct minimal_section_header {
		unsigned char  table_id                 :  8;
		unsigned char  section_syntax_indicator :  1;
		unsigned char  reserved_future_use      :  1;
		unsigned char  reserved1                :  2;
		unsigned short section_length           : 12;
	} __attribute__ ((packed));  // 3 bytes total
	
	minimal_section_header initial_header;
	char * buf;
	int    rc;
	
	lock();
	
	rc = read((char *) &initial_header, 3, timeoutInMSeconds);
	
	if (rc <= 0)
	{
		unlock();
		if (rc == 0)
		{
			dprintf("dmx.read timeout - filter: %x - timeout# %d", filters[filter_index].filter, timeouts);
			timeouts++;
		}
		else
		{
			dprintf("dmx.read rc: %d - filter: %x", rc, filters[filter_index].filter);
			// restart DMX
			real_pause();
			real_unpause();
		}
		return NULL;
	}
	
	timeouts = 0;
	buf = new char[3 + initial_header.section_length];
	
	if (!buf)
	{
		unlock();
		closefd();
		fprintf(stderr, "[sectionsd] FATAL: Not enough memory: filter: %x\n", filters[filter_index].filter);
		throw std::bad_alloc();
		return NULL;
	}
	
	if (initial_header.section_length > 0)
		rc = read(buf + 3, initial_header.section_length, timeoutInMSeconds);
	
	unlock();
	
	if (rc <= 0)
	{
		delete[] buf;
		if (rc == 0)
		{
			dprintf("dmx.read timeout after header - filter: %x", filters[filter_index].filter);
		}
		else
		{
			dprintf("dmx.read rc: %d after header - filter: %x", rc, filters[filter_index].filter);
		}
		// DMX restart required, since part of the header has been read
		real_pause();
		real_unpause();
		return NULL;
	}
	
	// check if the filter worked correctly
	if (((initial_header.table_id ^ filters[filter_index].filter) & filters[filter_index].mask) != 0)
	{
		delete[] buf;
		printf("[sectionsd] filter 0x%x mask 0x%x -> skip sections for table 0x%x\n", filters[filter_index].filter, filters[filter_index].mask, initial_header.table_id);
		return NULL;
	}
	
	if (initial_header.section_length < 5)  // skip sections which are too short
	{
		delete[] buf;
		return NULL;
	}
	
	memcpy(buf, &initial_header, 3);
	
	return buf;
}

int DMX::start(void)
{
	if (fd)
	{
		return 1;
	}

	pthread_mutex_lock(&start_stop_mutex);

	if (real_pauseCounter != 0)
	{
		pthread_mutex_unlock(&start_stop_mutex);
		return 0;
	}

	if ((fd = open(DEMUX_DEVICE, O_RDWR)) == -1)
	{
		perror("[sectionsd] open dmx");
		pthread_mutex_unlock(&start_stop_mutex);
		return 2;
	}

	if (ioctl(fd, DMX_SET_BUFFER_SIZE, (unsigned long)(dmxBufferSizeInKB*1024UL)) == -1)
	{
		closefd();
		perror("[sectionsd] DMX: DMX_SET_BUFFER_SIZE");
		pthread_mutex_unlock(&start_stop_mutex);
		return 3;
	}

	if (!setfilter(fd, pID, filters[filter_index].filter, filters[filter_index].mask, DMX_IMMEDIATE_START | DMX_CHECK_CRC))
	{
		closefd();
		pthread_mutex_unlock(&start_stop_mutex);
		return 4;
	}

/*  	if (timeset)
	lastChanged=time(NULL);*/
	pthread_mutex_unlock(&start_stop_mutex);

	return 0;
}

int DMX::real_pause(void)
{
	if (!fd)
		return 1;

	pthread_mutex_lock(&start_stop_mutex);

	if (real_pauseCounter == 0)
	{
		if (ioctl(fd, DMX_STOP, 0) == -1)
		{
			closefd();
			perror("[sectionsd] DMX: DMX_STOP");
			pthread_mutex_unlock(&start_stop_mutex);
			return 2;
		}
	}

	//dprintf("real_pause: %d\n", real_pauseCounter);
	pthread_mutex_unlock(&start_stop_mutex);

	return 0;
}

int DMX::real_unpause(void)
{
	if (!fd)
		return 1;

	pthread_mutex_lock(&start_stop_mutex);

	if (real_pauseCounter == 0)
	{
		if (ioctl(fd, DMX_START, 0) == -1)
		{
			closefd();
			perror("[sectionsd] DMX: DMX_START");
			pthread_mutex_unlock(&start_stop_mutex);
			return 2;
		}

		//dprintf("real_unpause DONE: %d\n", real_pauseCounter);
	}

	//    else
	//dprintf("real_unpause NOT DONE: %d\n", real_pauseCounter);


	pthread_mutex_unlock(&start_stop_mutex);

	return 0;
}

int DMX::request_pause(void)
{
	real_pause(); // unlocked

	pthread_mutex_lock(&start_stop_mutex);
	//dprintf("request_pause: %d\n", real_pauseCounter);

	real_pauseCounter++;
	pthread_mutex_unlock(&start_stop_mutex);

	return 0;
}


int DMX::request_unpause(void)
{
	pthread_mutex_lock(&start_stop_mutex);
	//dprintf("request_unpause: %d\n", real_pauseCounter);
	--real_pauseCounter;
	pthread_mutex_unlock(&start_stop_mutex);

	real_unpause(); // unlocked

	return 0;
}


int DMX::pause(void)
{
	if (!fd)
		return 1;

	pthread_mutex_lock(&pauselock);

	//dprintf("lock from pc: %d\n", pauseCounter);
	pauseCounter++;

	pthread_mutex_unlock(&pauselock);

	return 0;
}

int DMX::unpause(void)
{
	if (!fd)
		return 1;

	pthread_mutex_lock(&pauselock);

	//dprintf("unlock from pc: %d\n", pauseCounter);
	--pauseCounter;

	pthread_mutex_unlock(&pauselock);

	return 0;
}

int DMX::change(const int new_filter_index)
{
	if (!fd)
		return 1;

	showProfiling("changeDMX: before pthread_mutex_lock(&start_stop_mutex)");
        pthread_mutex_lock(&start_stop_mutex);

	showProfiling("changeDMX: after pthread_mutex_lock(&start_stop_mutex)");

	if (real_pauseCounter > 0)
	{
		pthread_mutex_unlock(&start_stop_mutex);
		dprintf("changeDMX: for 0x%x ignored! because of real_pauseCounter> 0\n", filters[new_filter_index].filter);
		return 0;	// läuft nicht (zB streaming)
	}

	//	if(pID==0x12) // Nur bei EIT
	dprintf("changeDMX [%x]-> %s (0x%x)\n", pID, (new_filter_index == 0) ? "current/next" : "scheduled", filters[new_filter_index].filter);

/*	if (ioctl(fd, DMX_STOP, 0) == -1)
	{
	closefd();
	perror("[sectionsd] DMX: DMX_STOP");
	pthread_mutex_unlock(&start_stop_mutex);
	return 2;
	}
*/
	closefd();



//	if (new_filter_index != filter_index)
	{

		if ((fd = open(DEMUX_DEVICE, O_RDWR)) == -1)
		{
			perror("[sectionsd] open dmx: ");
			pthread_mutex_unlock(&start_stop_mutex);
			return 2;
		}

		if (ioctl(fd, DMX_SET_BUFFER_SIZE, (unsigned long)(dmxBufferSizeInKB*1024UL)) == -1)
		{
			closefd();
			perror("[sectionsd] DMX: DMX_SET_BUFFER_SIZE");
			pthread_mutex_unlock(&start_stop_mutex);
			return 3;
		}

		filter_index = new_filter_index;
		if (!setfilter(fd, pID, filters[filter_index].filter, filters[filter_index].mask, ((new_filter_index != 0) && (!noCRC)) ? DMX_IMMEDIATE_START | DMX_CHECK_CRC : DMX_IMMEDIATE_START))
		{
			closefd();
			pthread_mutex_unlock(&start_stop_mutex);
			return 3;
		}

		showProfiling("after DMX_SET_FILTER");
	}
/*	else
	{
	if (ioctl(fd, DMX_START, 0) == -1)
	{
	closefd();
	perror("[sectionsd] DMX: DMX_START");
	pthread_mutex_unlock(&start_stop_mutex);
	return 3;
	}
	showProfiling("after DMX_START");
	}
*/
        pthread_cond_signal(&change_cond);

	if (timeset)
		lastChanged = time(NULL);

	pthread_mutex_unlock(&start_stop_mutex);

	return 0;
}
