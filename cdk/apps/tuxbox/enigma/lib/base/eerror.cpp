#include <lib/base/eerror.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include <lib/gui/emessage.h>

#ifdef MEMLEAK_CHECK
AllocList *allocList;
pthread_mutex_t memLock = 
	PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

void DumpUnfreed()
{
	AllocList::iterator i;
	unsigned int totalSize = 0;

	if(!allocList)
		return;

	size_t len = 1024;
	char *buffer = (char*)malloc(1024);
	for(i = allocList->begin(); i != allocList->end(); i++)
	{
		unsigned int tmp;
		printf("%s\tLINE %d\tADDRESS %p\t%d unfreed\ttype %d (btcount %d)\n",
			i->second.file, i->second.line, (void*)i->second.address, i->second.size, i->second.type, i->second.btcount);
		totalSize += i->second.size;
		char **bt_string = backtrace_symbols( i->second.backtrace, i->second.btcount );
		for ( tmp=0; tmp < i->second.btcount; tmp++ )
		{
			if ( bt_string[tmp] )
			{
				char *beg = strchr(bt_string[tmp], '(');
				if ( beg )
				{
					std::string tmp1(beg+1);
					int pos1=tmp1.find('+'), pos2=tmp1.find(')');
					if ( pos1 != std::string::npos && pos2 != std::string::npos )
					{
						std::string tmp2(tmp1.substr(pos1,(pos2-pos1)));
						tmp1.erase(pos1);
						if (tmp1.length())
						{
							int state;
							abi::__cxa_demangle(tmp1.c_str(), buffer, &len, &state);
							if (!state)
								printf("%d %s%s\n", tmp, buffer,tmp2.c_str());
							else
								printf("%d %s\n", tmp, bt_string[tmp]);
						}
					}
				}
				else
					printf("%d %s\n", tmp, bt_string[tmp]);
			}
		}
		free(bt_string);
		printf("\n");
	}
	free(buffer);

	printf("-----------------------------------------------------------\n");
	printf("Total Unfreed: %d bytes\n", totalSize);
	fflush(stdout);
};

#else
	#include <lib/system/elock.h>
#endif
pthread_mutex_t signalLock = 
	PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;

int infatal=0;

Signal2<void, int, const eString&> logOutput;
int logOutputConsole=1;
int logOutputSyslog = 0;

char *printtime(char buffer[], int size)
{
	struct  tm loctime ;
	struct timeval tim;

	gettimeofday(&tim, NULL);
	localtime_r(&tim.tv_sec, &loctime);
	snprintf(buffer, size, "%02d:%02d:%02d.%03ld",
					loctime.tm_hour, loctime.tm_min, loctime.tm_sec, tim.tv_usec/1000L);
	return buffer;
}

void eFatal(const char* fmt, ...)
{
	char buf[1024];
	char timestr[32];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	{
		singleLock s(signalLock);
		logOutput(lvlFatal, buf);
		if (logOutputSyslog)
		{
			syslog(LOG_DEBUG, "%s", buf);
		}
		fprintf(stderr, "%s: %s\n", printtime(timestr, sizeof(timestr)), buf);
	}
	if (!infatal)
	{
		infatal=1;
		eMessageBox msg(buf, "FATAL ERROR", eMessageBox::iconError|eMessageBox::btOK);
		msg.show();
		msg.exec();
	}
	_exit(-1);
}

#ifdef DEBUG
void eDebug(const char* fmt, ...)
{
	char buf[1024];
	char timestr[32];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	singleLock s(signalLock);
	if (logOutputSyslog)
	{
		syslog(LOG_DEBUG, "%s", buf);
	}
	if (logOutputConsole)
		fprintf(stderr, "%s: %s\n", printtime(timestr, sizeof(timestr)), buf);
	else
		logOutput(lvlDebug, eString(buf) + "\n");
}

void eDebugNoNewLine(const char* fmt, ...)
{
	char buf[1024];
	char timestr[32];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	singleLock s(signalLock);
	if (logOutputSyslog)
	{
		syslog(LOG_DEBUG, "%s", buf);
	}
	if (logOutputConsole)
		fprintf(stderr, "%s: %s", printtime(timestr, sizeof(timestr)), buf);
	else
		logOutput(lvlDebug, buf);
}

void eWarning(const char* fmt, ...)
{
	char buf[1024];
	char timestr[32];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	singleLock s(signalLock);
	if (logOutputSyslog)
	{
		syslog(LOG_DEBUG, "%s", buf);
	}
	if (logOutputConsole)
		fprintf(stderr, "%s: %s\n", printtime(timestr, sizeof(timestr)), buf);
	else
		logOutput(lvlWarning, eString(buf) + "\n");
}
#endif // DEBUG
