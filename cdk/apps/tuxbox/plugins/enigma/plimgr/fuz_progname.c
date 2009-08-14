#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int len, i;
	unsigned int pl;
	char *progname = argv[1];
        int fd = open("/dev/urandom", O_RDONLY);

        if (fd < 0)
        {
                perror("/dev/urandom");
                fprintf(stderr, "No random data, no point in going on.\n");
                exit(1);
        }

	if (! progname)
	{
		fprintf(stderr, "need program name as arg\n");
		exit(1);
	}

	len = strlen(progname);

	printf("\tchar * progname;\n\n");
	printf("\tprogname = argv[0];\n");
	printf("\tif (strchr(progname, '/'))\n");
	printf("\t{\n");
	printf("\t\tprogname = strrchr(progname, '/');\n");
	printf("\t\tprogname++;\n");
	printf("\t}\n");
	printf("\tif (strlen(progname) != %d)\n", len);
	printf("\t{\n");
	printf("\t\tputchar('\\n');\n");
	printf("\t\tprfuzz(sorry);\n");
	printf("\t\texit(1);\n");
	printf("\t}\n");
	printf("\tif (\n");
	
	for (i = 0; i< len; i++)
	{
        	read(fd, &pl, sizeof(pl));
        	pl %= 2;
		if (pl)
		{ // plus
        		read(fd, &pl, sizeof(pl));
        		pl %= (256 - progname[i]);
        		printf("\t\tprogname[%d] + 0x%02x != 0x%02x  || // %02x %c\n", i, pl, progname[i] + pl, progname[i], progname[i]);
		}
		else
		{ //min
        		read(fd, &pl, sizeof(pl));
        		pl %=progname[i];
        		printf("\t\tprogname[%d] - 0x%02x != 0x%02x  || // %02x %c\n", i, pl, progname[i] - pl, progname[i], progname[i]);
		}
	}
	printf("\t\tprogname[%d] != 0 )\n", len);
	printf("\t{\n");
	printf("\t\tputchar('\\n');\n");
	printf("\t\tprfuzz(sorry);\n");
	printf("\t\texit(2);\n");
	printf("\t}\n");
}

