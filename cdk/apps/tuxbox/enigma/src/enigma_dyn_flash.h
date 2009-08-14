/*
 * $Id: enigma_dyn_flash.h,v 1.5 2005/10/12 20:46:27 digi_casi Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
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
 
#ifndef __enigma_dyn_flash_h
#define __enigma_dyn_flash_h

#include <lib/base/thread.h>
#include <lib/base/message.h>

class eHTTPDynPathResolver;
void ezapFlashInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb);
eString getConfigFlashMgr(void);

class eFlashOperationsHandler: public eMainloop, private eThread, public Object
{
	eString progressMessage1, progressMessage2;
	int progressComplete;
	struct Message
	{
		int type;
		const char *mtd;
		const char *filename;
		enum
		{
			read,
			write,
			quit
		};
		Message(int type = 0, const char *mtd = 0, const char *filename = 0)
			:type(type), mtd(mtd), filename(filename)
		{}
	};
	eFixedMessagePump<Message> messages;
	static eFlashOperationsHandler *instance;
	void gotMessage(const Message &message);
	void thread();
	int writeFlash(eString mtd, eString fileName);
	int readFlash(eString mtd, eString fileName);
public:
	eFlashOperationsHandler();
	~eFlashOperationsHandler();
	void readPartition(const char * mtd, const char * filename);
	void writePartition(const char * mtd, const char * filename);
	void quitFlashOps();
	eString getProgressMessage1() { return progressMessage1; }
	eString getProgressMessage2() { return progressMessage2; }
	int getProgressComplete() { return progressComplete; }
	static eFlashOperationsHandler *getInstance() { return (instance) ? instance : new eFlashOperationsHandler(); }
};

class eFlashMgr
{
	eString h1, h2, h3, h4;
	typedef struct
	{
		eString dev, name, size, erasesize;
	} t_mtd;
	std::list<t_mtd> mtds;
	
public:
	eFlashMgr();
	~eFlashMgr();
	eString htmlList();
	eString getMTDName(eString mtd);
};

#endif /* __enigma_dyn_flash_h */

