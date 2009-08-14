#ifndef __CORE_GUI_NUMBERACTIONS__
#define __CORE_GUI_NUMBERACTIONS__

#include <lib/gui/actions.h>
#include <lib/base/i18n.h>
#include <lib/system/init.h>

struct numberActions
{
	eActionMap map;
	eAction key0, key1, key2, key3, key4, key5, key6, key7, key8, key9, keyExt1, keyExt2, keyBackspace;
	numberActions():
		map("numbers", _("number actions")),
		key0(map, "0", _("key 0"), eAction::prioDialog),
		key1(map, "1", _("key 1"), eAction::prioDialog),
		key2(map, "2", _("key 2"), eAction::prioDialog),
		key3(map, "3", _("key 3"), eAction::prioDialog),
		key4(map, "4", _("key 4"), eAction::prioDialog),
		key5(map, "5", _("key 5"), eAction::prioDialog),
		key6(map, "6", _("key 6"), eAction::prioDialog),
		key7(map, "7", _("key 7"), eAction::prioDialog),
		key8(map, "8", _("key 8"), eAction::prioDialog),
		key9(map, "9", _("key 9"), eAction::prioDialog),
		keyExt1(map, "ext1", _("extended key1"), eAction::prioDialog),
		keyExt2(map, "ext2", _("extended key2"), eAction::prioDialog),
		keyBackspace(map, "backspace", _("backspace"), eAction::prioDialog)
	{
	}
};
extern eAutoInitP0<numberActions> i_numberActions;
#endif

