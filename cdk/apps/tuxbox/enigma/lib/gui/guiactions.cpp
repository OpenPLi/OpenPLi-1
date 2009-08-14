#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/gui/guiactions.h>

eAutoInitP0<cursorActions> i_cursorActions(eAutoInitNumbers::actions, "cursor actions");
eAutoInitP0<focusActions> i_focusActions(eAutoInitNumbers::actions, "focus actions");
eAutoInitP0<listActions> i_listActions(eAutoInitNumbers::actions, "list actions");
eAutoInitP0<shortcutActions> i_shortcutActions(eAutoInitNumbers::actions, "shortcut actions");
