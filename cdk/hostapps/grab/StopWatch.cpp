/*
 * $Id: StopWatch.cpp,v 1.1 2001/12/22 17:14:52 obi Exp $
 *
 * Copyright (C) 2001 Peter Niemayer et al.
 * See AUTHORS for details.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Log: StopWatch.cpp,v $
 * Revision 1.1  2001/12/22 17:14:52  obi
 * - added hostapps
 * - added grab to hostapps
 *
 *
 */
 
#include "StopWatch.h"

StopWatch::StopWatch()
{
	start();
}

void StopWatch::start()
{
	gettimeofday(&timev1, &tzdummy);
}
	
void StopWatch::set_to_past(long seconds)
{
	timev1.tv_sec -=  seconds;
}

long StopWatch::stop()
{
	gettimeofday(&timev2, &tzdummy);
	long sec = timev2.tv_sec - timev1.tv_sec;

	if (sec < 0)
		sec += 24*60*60;
	
	long us = (timev2.tv_usec - timev1.tv_usec + 500) / 1000;
	
	return 1000 * sec + us;
}

long StopWatch::stop_us(void)
{
	gettimeofday(&timev2, &tzdummy);
	long sec = timev2.tv_sec - timev1.tv_sec;

	if (sec < 0)
		sec += 24*60*60;
	
	long us = (timev2.tv_usec - timev1.tv_usec);
	
	return 1000000 * sec + us;
}

long StopWatch::restart()
{
	long res = stop();
	timev1 = timev2;
	
	return res;
}
