/*
 * $Id: pluginx.c,v 1.2 2009/01/06 21:40:08 dbt Exp $
 *
 * commandline tool to execute gui plugins without gui
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
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

#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdint.h>
#include <dbox/fb.h>

#include <config.h>
#include <plugin.h>

#define FB_DEVICE	"/dev/fb/0"

#if HAVE_DVB_API_VERSION < 3
#define RC_DEVICE	"/dev/dbox/rc0"
#else
#define RC_DEVICE	"/dev/input/event0"
#endif

#define LCD_DEVICE	"/dev/dbox/lcd0"

struct handles_s
{
	void * handle;
	struct handles_s * first;
	struct handles_s * next;
	char * path;
};

enum
{
	TYPE_FB,
	TYPE_RC,
	TYPE_LCD
};

int fb = -1;
int rc = -1;
int lcd = -1;

struct handles_s * handles = NULL;

void usage (char * name)
{
	printf("usage: %s <option>\n", name);
	printf("options:\n");
	printf("\t-l <dir> (default: %s)\n", PLUGINDIR);
	printf("\t-x <basename>\n");
}

void load_so (char * filename)
{
	void * handle = NULL;
	char * path = NULL;

	if (filename[0] == '/')
	{
		path = (char *) malloc(strlen(filename) + 1);
		strcpy(path, filename);
	}
	else
	{
		path = (char *) malloc(strlen(PLUGINDIR) + strlen(filename) + 2);
		sprintf(path, "%s/%s", PLUGINDIR, filename);
	}

	handle = dlopen(path, RTLD_NOW|RTLD_GLOBAL);

	if (handle != NULL)
	{
		if (handles == NULL)
		{
			handles = (struct handles_s *) malloc(sizeof(struct handles_s));
			handles->first = handles;
			handles->first->first = handles->first;
			handles->first->handle = handle;
			handles->first->next = NULL;
			handles->first->path = path;
		}
		else
		{
			handles->next = (struct handles_s *) malloc(sizeof(struct handles_s));
			handles->next->first = handles->first;
			handles->next->handle = handle;
			handles->next->next = NULL;
			handles->next->path = path;
		}
	}
}


void fd_to_param (char * filename, int flags, const char * const id, int type, PluginParam * param)
{
	int fd = open(filename, flags);

	if (fd < 0)
	{
		perror(filename);
	}
	else
	{
		int tmp = fd;
		unsigned char i = 0;

		switch (type)
		{
		case TYPE_FB:
			fb = fd;
			break;
		case TYPE_RC:
			rc = fd;
			break;
		case TYPE_LCD:
			lcd = fd;
			break;
		}

		for (tmp = fd; tmp > 9; tmp /= 10)
			i++;

		param->id = id;
		param->val = (char *) malloc(i);
		sprintf(param->val, "%d", fd);
		param->next = NULL;
	}
}

PluginParam * parse_cfg (char * filename)
{
	FILE * cfg_file = fopen(filename, "r");

	if (cfg_file == NULL)
	{
		perror("fopen");
		return NULL;
	}
	else
	{
		char buf[80];

		PluginParam * first_param = NULL;
		PluginParam * current_param = (PluginParam *) malloc(sizeof(PluginParam));

		while (fgets(buf, sizeof(buf), cfg_file) != NULL)
		{
			*strchr(buf, '\n') = 0;

			if (!strncmp(buf, "depend=", 7))
			{
				char * start = buf + 7;
				char * end;

				while ((end = strchr(start, ',')) != NULL)
				{
					*end = 0;
					load_so(start);
					start = end + 1;
				}

				load_so(start);
			}
			else if (!strcmp(buf, "needfb=1"))
			{
				current_param->next = (PluginParam *) malloc(sizeof(PluginParam));
				fd_to_param(FB_DEVICE, O_RDWR, P_ID_FBUFFER, TYPE_FB, current_param->next);
			}
			else if (!strcmp(buf, "needrc=1"))
			{
				current_param->next = (PluginParam *) malloc(sizeof(PluginParam));
				fd_to_param(RC_DEVICE, O_RDONLY, P_ID_RCINPUT, TYPE_RC, current_param->next);
			}
			else if (!strcmp(buf, "needlcd=1"))
			{
				current_param->next = (PluginParam *) malloc(sizeof(PluginParam));
				fd_to_param(LCD_DEVICE, O_RDWR, P_ID_LCD, TYPE_LCD, current_param->next);
			}
			else
			{
				printf("unhandled line: %s\n", buf);
			}

			if (current_param->next != NULL)
			{
				if (first_param == NULL)
				{
					first_param = current_param->next;
					free(current_param);
					current_param = first_param;
				}
				else
				{
					current_param = current_param->next;
				}
			}
		}

		if (first_param == NULL)
			free(current_param);

		fclose(cfg_file);
		return first_param;
	}
}

int main (int argc, char ** argv)
{
	struct dirent ** plugin_list;
	unsigned int i;
	int plugin_count;
	char * plugin_dir = NULL;
	char * basename = NULL;

	if (argc == 1)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-l"))
		{
			if (argc > i + 1)
			{
				plugin_dir = (char *) malloc(strlen(argv[i + 1]) + 1);
				strcpy(plugin_dir, argv[i + 1]);
				i++;
			}
		}
		else if (!strcmp(argv[i], "-x"))
		{
			if (argc > i + 1)
			{
				basename = (char *) malloc(strlen(argv[i + 1]) + 1);
				strcpy(basename, argv[i + 1]);
				i++;
			}
			else
			{
				usage(argv[0]);
				free(plugin_dir);
				return EXIT_FAILURE;
			}
		}
		else
		{
			usage(argv[0]);
			free(basename);
			free(plugin_dir);
			return EXIT_FAILURE;
		}
	}

	if (plugin_dir == NULL)
	{
		plugin_dir = (char *) malloc(strlen(PLUGINDIR) + 1);
		strcpy(plugin_dir, PLUGINDIR);
	}

	if (!basename)
	{
		plugin_count = scandir(plugin_dir, &plugin_list, 0, alphasort);

		if (plugin_count < 0)
		{
			perror("scandir");
			free(plugin_dir);
			return EXIT_FAILURE;
		}

		for (i = 0; i < plugin_count; i++)
		{
			if (fnmatch("*.cfg", plugin_list[i]->d_name, 0) == 0)
			{
				plugin_list[i]->d_name[strlen(plugin_list[i]->d_name) - 4] = 0;
				printf("%s\n", plugin_list[i]->d_name);
			}

			free(plugin_list[i]);
		}

		free(plugin_list);
	}
	else
	{
		char plugin_path[strlen(plugin_dir) + strlen(basename) + 5];
		struct stat plugin_stat;

		sprintf(plugin_path, "%s/%s.cfg", plugin_dir, basename);

		if (stat(plugin_path, &plugin_stat) == -1)
		{
			perror("stat");
		}
		else
		{
			PluginParam * plugin_param = parse_cfg(plugin_path);

			if (plugin_param == NULL)
			{
				printf("error parsing %s\n", plugin_path);
			}
			else
			{
				sprintf(plugin_path, "%s/%s.so", plugin_dir, basename);

				if (stat(plugin_path, &plugin_stat) == -1)
				{
					perror("stat");
				}
				else
				{
					void * plugin_handle = dlopen(plugin_path, RTLD_NOW);

					if (plugin_handle == NULL)
					{
						printf("%s\n", dlerror());
					}
					else
					{
						PluginExec plugin_exec = (PluginExec) dlsym(plugin_handle, "plugin_exec");

						if (plugin_exec == NULL)
						{
							printf("%s\n", dlerror());
						}
						else
						{
							PluginParam * next = plugin_param->next;
							
							plugin_exec(plugin_param);

							while (plugin_param != NULL)
							{
								free(plugin_param->val);
								free(plugin_param);

								if (next)
								{
									plugin_param = next;
									next = plugin_param->next;
								}
								else
								{
									plugin_param = NULL;
								}
							}

							dlclose(plugin_handle);

							if (fb != -1)
							{
								ioctl(fb, AVIA_GT_GV_HIDE);
								close(fb);
							}

							if (rc != -1)
							{
								close(rc);
							}

							if (lcd != -1)
							{
								close(lcd);
							}
						}
					}
				}
			}
		}
		
		free(basename);
	}
	
	free(plugin_dir);

	if (handles != NULL)
	{
		struct handles_s * cur = handles->first;
		struct handles_s * next = handles->first->next;

		while (cur != NULL)
		{
			printf("unloading %s\n", cur->path);
			dlclose(cur->handle);
		
			free(cur->path);
			free(cur);

			if (next != NULL)
			{
				cur = next;
				next = cur->next;
			}
			else
			{
				cur = NULL;
			}
		}
	}

	return EXIT_SUCCESS;
}

