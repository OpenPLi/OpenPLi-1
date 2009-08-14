#include <plugin.h>
#include <enigma_main.h>

// without this or a instance of eWidget enigma starts this plugin in own thread..
// this isn't so good :)
const char *fake_string = "_ZN7eWidgetD0Ev";

	// our plugin entry point, declared to use C calling convention
extern "C" int plugin_exec( PluginParam *par );

	// our entry point.
int plugin_exec( PluginParam *par )
{
	eZapMain::getInstance()->startNGrabRecord();
	return 0;
}
