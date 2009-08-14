#include <plugin.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern "C" int plugin_exec( PluginParam *par );


int plugin_exec( PluginParam *par )
{
	system("/var/bin/tuxterm");

	return 0;
}

