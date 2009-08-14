#include <plugin.h>
#include "brdlg.h"

extern "C" int plugin_exec(PluginParam *par);

int plugin_exec(PluginParam *par)
{
   BitrateDialog dlg;
   dlg.show();
   dlg.exec();
   dlg.hide();
   return 0;
}
