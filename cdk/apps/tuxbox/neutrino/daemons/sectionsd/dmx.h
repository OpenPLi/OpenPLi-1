/*
 * $Header: /cvs/tuxbox/apps/tuxbox/neutrino/daemons/sectionsd/dmx.h,v 1.1 2003/02/05 22:13:10 thegoodguy Exp $
 *
 * DMX class (sectionsd) - d-box2 linux project
 *
 * (C) 2003 by thegoodguy <thegoodguy@berlios.de>
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

#ifndef __sectionsd__dmx_h__
#define __sectionsd__dmx_h__

#include <pthread.h>
#include <vector>

class DMX
{
 private:
	int             fd;
	pthread_mutex_t pauselock;
	unsigned char   pID;
	unsigned short  dmxBufferSizeInKB;
	bool            noCRC; // first filter always without CRC

 public:
	struct s_filters
	{
		unsigned char filter;
		unsigned char mask;
	};

	std::vector<s_filters> filters;
	int                    filter_index;
	time_t                 lastChanged;

	int                    pauseCounter;
	int                    real_pauseCounter;
	pthread_cond_t         change_cond;
	pthread_mutex_t        start_stop_mutex;


	DMX(const unsigned char p, const unsigned short bufferSizeInKB, const bool nCRC = false);
	~DMX();

	int start(void);
	int read(char *buf, const size_t buflength, const unsigned timeoutMInSeconds);
	void closefd(void);
	void addfilter(const unsigned char filter, const unsigned char mask);
	int stop(void);

	int pause(void); // increments pause_counter
	int unpause(void); // decrements pause_counter
	
	int real_pause(void);
	int real_unpause(void);

	int request_pause(void);
	int request_unpause(void);

	int change(const int new_filter_index); // locks while changing

	void lock(void);
	void unlock(void);

	bool isOpen(void);

	char * getSection(const unsigned timeoutInMSeconds, int &timeouts);
	// section with size < 3 + 5 are skipped !
};

#endif /* __sectionsd__dmx_h__ */

