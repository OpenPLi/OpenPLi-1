/*
Revision 1.0  2008/07/22 23:50:45  fergy
Replaced mtd stuff for compatible building
*/

#ifndef UPDATE_H
#define UPDATE_H

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <libcramfs.h>

/* zlib required.. */
#include <zlib.h>
#include <string>
#include <sstream>
#include <mtd/mtd-user.h>
#include <curl/curl.h>
#include "rc.h"
#include "osd.h"
#include "settings.h"

#define UPDATE_UCODES 0
#define UPDATE_MANUALFILES 1
#define UPDATE_INET 2

class update
{
	void eraseall();

	int getUpdate();

	osd *osd_obj;
	rc *rc_obj;
	settings *settings_obj;

public:
	update(osd *o, rc *r, settings *s);

	unsigned char ip[4];
	std::string sourcedir;
	int cramfsmtd;

	void run(int type);

};

#endif
