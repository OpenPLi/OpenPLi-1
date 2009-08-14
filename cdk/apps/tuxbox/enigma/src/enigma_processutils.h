#ifndef __enigma_processutils__
#define __enigma_processutils__


class eProcessUtils
{
public:
	static long *getPID(const char *);
	static void killProcess(const char *);
};
#endif

