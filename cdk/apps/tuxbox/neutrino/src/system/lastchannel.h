/*
DBoX2 -- Projekt
 
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


#ifndef SEEN_LastChannel
#define SEEN_LastChannel


class CLastChannel
{

	private:
		struct _LastCh
		{
			int             channel;
			unsigned long   timestamp;
		}
		lastchannels[8];
		int            pos;
		unsigned long  secs_diff_before_store;

#define  size_LASTCHANNELS 	(sizeof(lastchannels)/sizeof(struct _LastCh))


	public:
		CLastChannel  (void);
		void clear   (void);
		void store   (int channelnr);
		int  getlast (int n);
		void clear_storedelay (void);
		void set_store_difftime (int secs);
		int  get_store_difftime (void);

};


#endif
