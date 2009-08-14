#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <stdint.h>
#include <dbox/fb.h>

void help (char *prog_name)
{
	printf("$Id: aviafbtool.c,v 1.3 2002/10/10 23:21:19 dirch Exp $\n\n");
	printf("Usage: %s <switches>\n\n",prog_name);
	printf("Switches:\n"
	"-h,     --help             help\n"
	"-p,     --pos <x> <y>      set position of fb\n"
	"-b,     --blev <val> <val> set the blev\n"
	"-s,     --show             show fb\n"
	"-u,     --hide             hide fb\n");
}

int main(int argc, char **argv)
{
	int x, y, fd, count;

	if (argc == 1)
	{
		help(argv[0]);
		return EXIT_FAILURE;
	}

	if ((fd = open("/dev/fb/0", O_RDWR)) < 0)
	{
		perror("open /dev/fb/0");
		return EXIT_FAILURE;
	}

	for(count=1;count<argc;count++)
	{

		/* -h or --help */
		if ((strcmp("-h",argv[count]) == 0) || (strcmp("--help",argv[count]) == 0))
		{
			help(argv[0]);
			return EXIT_FAILURE;
		}
		else if ((strcmp("-p",argv[count]) == 0) || (strcmp("--pos",argv[count]) == 0))
		{
			if (argc < count+3)
			{
				printf("No coordinates given\n");
				return EXIT_FAILURE;
			}
			else
			{
				count+=2;
				x = atoi(argv[count-1]);
				y = atoi(argv[count]);
				if (ioctl(fd, AVIA_GT_GV_SET_POS, (x << 16) | y) < 0)
				{
					perror("AVIA_GT_GV_SET_POS");
					return EXIT_FAILURE;
				}
			}
		}
		else if ((strcmp("-b",argv[count]) == 0) || (strcmp("--blev",argv[count]) == 0))
                {
                        if (argc < count+3)
                        {
                                printf("No blev given\n");
                                return EXIT_FAILURE;
                        }
                        else
                        {
                                count+=2;
				x = atoi(argv[count - 1]);
				y = atoi(argv[count]);
				
				if (ioctl(fd, AVIA_GT_GV_SET_BLEV, (x << 8) | y) < 0)
				{
					perror("AVIA_GT_GV_SET_BLEV");
					return EXIT_FAILURE;
				}
			}
		}
		else if ((strcmp("-s",argv[count]) == 0) || (strcmp("--show",argv[count]) == 0))
		{
			if (ioctl(fd, AVIA_GT_GV_SHOW, 0) < 0)
			{
				perror("AVIA_GT_GV_SHOW");
				return EXIT_FAILURE;
			}
		}
		else if ((strcmp("-u",argv[count]) == 0) || (strcmp("--hide",argv[count]) == 0))
		{
			if (ioctl(fd, AVIA_GT_GV_HIDE, 0) < 0)
			{
				perror("AVIA_GT_GV_HIDE");
				return EXIT_FAILURE;
			}
		}
		else
		{
			help(argv[0]);
			return EXIT_FAILURE;
		}
	}

	close (fd);

	return EXIT_SUCCESS;
}

