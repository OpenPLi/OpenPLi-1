/***********************************************************************
 * ein weiteres wenig sinnvolles programm für die dbox2 (C) 2001 Ge0rG *
 *                                                                     *
 * Dieses Programm unterliegt der GPL, also nix klauen ;o)             *
 ***********************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <dbox/lcd-ks0713.h>

#include "ani.h"

typedef unsigned char screen_t[LCD_BUFFER_SIZE];


int lcd_fd;

void draw_screen(screen_t s)
{
	write(lcd_fd, s, LCD_BUFFER_SIZE);
}

void clr()
{
	screen_t blank;
	bzero(&blank, sizeof(blank));
	draw_screen(blank);
}

void init()
{
	int i;
	if ((lcd_fd = open("/dev/dbox/lcd0",O_RDWR)) < 0) {
		perror("open(/dev/dbox/lcd0)");
		exit(1);
	}
	clr();

	i = LCD_MODE_BIN;
	if (ioctl(lcd_fd,LCD_IOCTL_ASC_MODE,&i) < 0) {
		perror("init(LCD_MODE_BIN)");
		exit(1);
	}
}

void timeout_action()
{
	clr();
	exit(1);
}

void daemonize()
{
	int i;
	pid_t pid;

	if ((pid = fork()) != 0)
		exit(0);

	setsid(); /* become session leader */

	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	if ((pid = fork()) != 0)
		exit(0);

	for (i = 0; i < 10; i++)
		close(i);

	chdir("/");

	freopen("/dev/null", "a", stdin);
	freopen("/dev/null", "a", stdout);
	freopen("/dev/null", "a", stderr);
}

void usage(const char *name)
{
	printf("aniplay (C) 2001 Ge0rG\n");
	printf("usage: %s <file.ani> [--loop <loopcount>] [--timeout <seconds>]\n", name);
	printf("\n-d                  = run in the foreground (debug mode)\n--loop <loopcount>  = run <loopcount> times\n--timeout <seconds> = stop after <seconds>\n");
}

int main(int argc, char **argv)
{
	FILE *f;
	struct ani_header ah;
	screen_t *ani;
	int t, loop, loopcount, timeout;
	int i;
	int daemon = 1;
	const char *filename = NULL;
	loopcount = 0;
	timeout = 0;

	if (argc < 2)
	{
		usage(argv[0]);
		exit(1);
	}
	
	for (i = 1; i < argc; i++)
	{
		if (strstr(argv[i], "-d"))
		{
			daemon = 0;
		}
		else if (strstr(argv[i], "--loop"))
		{
			if (i + 1 < argc)
			{
				loopcount = atoi(argv[i + 1]);
				i++;
			}
		}
		else if (strstr(argv[i], "--timeout"))
		{
			if (i + 1 < argc)
			{
				timeout = atoi(argv[i + 1]);
				i++;
			}
		}
		else if (strstr(argv[i], "--help"))
		{
			usage(argv[0]);
			exit(1);
		}
		else
		{
			if (filename)
			{
				usage(argv[0]);
				exit(1);
			}
			else
			{
				filename = argv[i];
			}
		}
	}

	f = fopen(filename, "r");
	if (f == NULL)
	{
		printf("couldn't open %s\n", filename);
		exit(1);
	}
	fread(&ah, 1, sizeof(ah), f);
	ani = malloc(sizeof(screen_t)*ah.count);
	fread(ani, sizeof(screen_t), ah.count, f);
	fclose(f);

	if (daemon) daemonize();

	init();

	signal(SIGALRM, timeout_action);
	if (timeout) alarm(timeout);

	t = 0;
	loop = 0;
	while ((loop<loopcount) || (loopcount==0)) {
		for (t=0; t<ah.count; t++) {
			draw_screen(ani[t]);
			usleep(ah.delay);
		}
		loop++;
	}
	clr();
	return 0;
}
