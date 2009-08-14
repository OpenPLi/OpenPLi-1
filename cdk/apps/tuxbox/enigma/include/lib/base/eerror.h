#ifndef __E_ERROR__
#define __E_ERROR__

// to use memleak check change the following in configure.ac
// * add -rdynamic to LD_FLAGS
// * add -DMEMLEAK_CHECK to CPP_FLAGS

#ifdef MEMLEAK_CHECK
#define BACKTRACE_DEPTH 10
#include <map>
#include <lib/system/elock.h>
#include <execinfo.h>
#include <string>
#include <new>
#include <cxxabi.h>
#endif // MEMLEAK_CHECK

#ifndef NULL
#define NULL 0
#endif

#include <libsig_comp.h>
#include <config.h>

class eString;

void eFatal(const char* fmt, ...);

enum { lvlDebug=1, lvlWarning=2, lvlFatal=4 };

extern Signal2<void, int, const eString&> logOutput;
extern int logOutputConsole;
extern int logOutputSyslog;

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef DEBUG
void eDebug(const char* fmt, ...);
void eDebugNoNewLine(const char* fmt, ...);
void eWarning(const char* fmt, ...);
#define ASSERT(x) { if (!(x)) eFatal("%s:%d ASSERTION %s FAILED!", __FILE__, __LINE__, #x); }

#ifdef MEMLEAK_CHECK
typedef struct
{
	unsigned int address;
	unsigned int size;
	char *file;
	void *backtrace[BACKTRACE_DEPTH];
	unsigned char btcount;
	unsigned short line;
	unsigned char type;
} ALLOC_INFO;

typedef std::map<unsigned int, ALLOC_INFO> AllocList;

extern AllocList *allocList;
extern pthread_mutex_t memLock;

static inline void AddTrack(unsigned int addr,  unsigned int asize,  const char *fname, unsigned int lnum, unsigned int type)
{
	ALLOC_INFO info;

	if(!allocList)
		allocList = new(AllocList);

	info.address = addr;
	info.file = strdup(fname);
	info.line = lnum;
	info.size = asize;
	info.type = type;
	info.btcount = backtrace( info.backtrace, BACKTRACE_DEPTH );
	singleLock s(memLock);
	(*allocList)[addr]=info;
};

static inline void RemoveTrack(unsigned int addr, unsigned int type)
{
	if(!allocList)
		return;
	AllocList::iterator i;
	singleLock s(memLock);
	i = allocList->find(addr);
	if ( i != allocList->end() )
	{
		if ( i->second.type != type )
			i->second.type=3;
		else
		{
			free(i->second.file);
			allocList->erase(i);
		}
	}
};

inline void * operator new(unsigned int size, const char *file, int line)
{
	void *ptr = (void *)malloc(size);
	AddTrack((unsigned int)ptr, size, file, line, 1);
	return(ptr);
};

inline void operator delete(void *p)
{
	RemoveTrack((unsigned int)p,1);
	free(p);
};

inline void * operator new[](unsigned int size, const char *file, int line)
{
	void *ptr = (void *)malloc(size);
	AddTrack((unsigned int)ptr, size, file, line, 2);
	return(ptr);
};

inline void operator delete[](void *p)
{
	RemoveTrack((unsigned int)p, 2);
	free(p);
};

void DumpUnfreed();

#define new new(__FILE__, __LINE__)

#endif // MEMLEAK_CHECK

#else
    inline void eDebug(const char* fmt, ...)
    {
    }

    inline void eDebugNoNewLine(const char* fmt, ...)
    {
    }

    inline void eWarning(const char* fmt, ...)
    {
    }
    #define ASSERT(x) do { } while (0)
#endif //DEBUG

#endif // __E_ERROR__
