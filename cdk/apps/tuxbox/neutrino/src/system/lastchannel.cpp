/*
 DBOX2 -- Projekt
 
(c) 2001 rasc
 
Lizenz: GPL 
 
 
Lastchannel History buffer
 
Einfache Klasse fuer schnelles Zappen zum letzten Kanal.
Ggf. laesst sich damit ein kleines ChannelHistory-Menue aufbauen-
 
Das ganze ist als sich selbst ueberschreibender Ringpuffer realisiert,
welcher nach dem LIFO-prinzip wieder ausgelesen wird.
Es wird aber gecheckt, ob ein Neuer Wert ein Mindestzeitabstand zum alten
vorherigen Wert hat, damit bei schnellem Hochzappen, die "Skipped Channels"
nicht gespeichert werden.
 
*/


#include <sys/time.h>
#include <unistd.h>

#include "lastchannel.h"


//
//  -- Init Class vi Contructor
//

CLastChannel::CLastChannel (void)

{
	clear ();
	set_store_difftime (3);
}


//
// -- Clear the last channel buffer
//

void CLastChannel::clear (void)

{
	int i;


	for (i=0; i < (int)size_LASTCHANNELS; i++)
	{
		lastchannels[i].channel   = -1;
		lastchannels[i].timestamp = 0;
	}

	pos = 0;
}



//
// -- Store a channelnumber in Buffer
// -- Store only if channel != last channel...
// -- and time store delay is large enough
//

void CLastChannel::store (int channel)

{
	struct timeval  tv;


	gettimeofday (&tv, NULL);

	if (    ((tv.tv_sec - lastchannels[pos].timestamp) > secs_diff_before_store)
	        && (lastchannels[pos].channel != channel) )
	{

		// -- store channel on next pos (new channel)
		pos = (pos + 1) % size_LASTCHANNELS;

	}

	// -- remember time (secs)
	lastchannels[pos].channel    = channel;
	lastchannels[pos].timestamp  = tv.tv_sec;

}



//
// -- Clear store time delay
// -- means: set last time stamp to zero
// -- means: store next channel with "store" always
//

void CLastChannel::clear_storedelay (void)

{
	lastchannels[pos].timestamp = 0;
}




//
// -- Get last Channel-Entry
// -- IN:   n number of last channel in queue [0..]
// --       0 = current channel
// -- Return:  channelnumber or <0  (end of list)

int CLastChannel::getlast (int n)

{
	int lastpos;


	// too large anyway
	if (n > (int)size_LASTCHANNELS)
		return -1;

	// get correct buffer pos
	lastpos = (pos - n);
	if (lastpos < 0)
		lastpos += size_LASTCHANNELS;

	return lastchannels[lastpos].channel;
}


//
// -- set delaytime in secs, for accepting a new value
// -- get returns the value
//

void CLastChannel::set_store_difftime (int secs)

{
	secs_diff_before_store = secs;
}

int CLastChannel::get_store_difftime (void)

{
	return    secs_diff_before_store;
}
