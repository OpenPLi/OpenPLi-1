#include <plugin.h>
#include <stdio.h>
#include <enigma_main.h>

	// our plugin entry point, declared to use C calling convention
extern "C" int plugin_exec( PluginParam *par );

// without this or a instance of eWidget enigma starts this plugin in own thread..
// this isn't so good :)
const char *fake_string = "_ZN7eWidgetD0Ev";

	// our entry point.
int plugin_exec( PluginParam *par )
{
	eZapMain::getInstance()->stopNGrabRecord();	
	return 0;
}
