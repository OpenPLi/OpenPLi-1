#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <lib/base/estring.h>
#include <enigma_processutils.h>

long *eProcessUtils::getPID(const char *procname)
{
	struct dirent *entry;
	long *pidList = (long *)malloc(sizeof(long));

	char *name;
	int pid = 0;
	int i = 0;
	char buf[1024], cmdline[40];
	FILE *fp;

	// eDebug("getPID: %s", procname);
	DIR *dir = opendir("/proc");
	if (dir)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			name = entry->d_name;
			if (*name >= '0' && *name <= '9')
			{
				pid = atoi(name);
				sprintf(cmdline, "/proc/%d/cmdline", pid);
				
				if ((fp = fopen(cmdline, "r")) != NULL)
				{
					if ((fread(buf, 1, sizeof(buf) - 1, fp)) > 0)
					{
						if (strstr(buf, procname) != 0)
						{
							pidList = (long *)realloc( pidList, sizeof(long) * (i + 2));
							pidList[i++] = pid;
							// eDebug("getPID:    %d", pid);
						}
					}
					fclose(fp);
				}
			}
		}
		closedir(dir);
	}
	pidList[i] = (i == 0) ? -1 : 0;

	return pidList;
}

void killPID(long *pid)
{
	if (*pid != -1 && *pid != 0)
	{
		if (kill(*pid, SIGTERM) != 0)
			kill(*pid, SIGKILL);
		waitpid(*pid, 0, 0);
		*pid = -1;
        }
}

void eProcessUtils::killProcess(const char *procname)
{
	if(strlen(procname) != 0)
	{
		long *pidList;
		pidList = getPID(procname);

		if (*pidList > 0)
		{
			long *pid;
			for (pid = pidList; *pid != 0; pid++)
				killPID(pid);
		}
		free(pidList);
	}
}
