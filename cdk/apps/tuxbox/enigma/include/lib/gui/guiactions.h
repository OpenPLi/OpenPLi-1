#ifndef __src__lib__gui__guiactions_h__
#define __src__lib__gui__guiactions_h__

#include <lib/system/init.h>
#include <lib/base/i18n.h>
#include <lib/gui/actions.h>

struct cursorActions
{
	eActionMap map;
	eAction up, down, left, right, insertchar, deletechar, capslock, ok, cancel, help;
	cursorActions():
		map("cursor", "Cursor"),
		up(map, "up", _("up"), eAction::prioWidget),
		down(map, "down", _("down"), eAction::prioWidget),
		left(map, "left", _("left"), eAction::prioWidget),
		right(map, "right", _("right"), eAction::prioWidget),
		insertchar(map, "insertchar", _("next char"), eAction::prioWidget),
		deletechar(map, "deletechar", _("prev char"), eAction::prioWidget),
		capslock(map, "capslock", _("CapsLock"), eAction::prioWidget),
		ok(map, "ok", "OK", eAction::prioWidget),
		cancel(map, "cancel", _("cancel"), eAction::prioDialog),
		help(map, "help", _("show the help window"), eAction::prioGlobal)
	{
	}
};

extern eAutoInitP0<cursorActions> i_cursorActions;

struct focusActions
{
	eActionMap map;
	eAction up, down, left, right;
	focusActions(): 
		map("focus", "Focus"),
		up(map, "up", _("up"), eAction::prioGlobal),
		down(map, "down", _("down"), eAction::prioGlobal),
		left(map, "left", _("left"), eAction::prioGlobal),
		right(map, "right", _("right"), eAction::prioGlobal)
	{
	}
};

extern eAutoInitP0<focusActions> i_focusActions;

struct listActions
{
	eActionMap map;
	eAction pageup, pagedown;
	listActions():
		map("list", "Listen"),
		pageup(map, "pageup", _("page up"), eAction::prioWidget+1),
		pagedown(map, "pagedown", _("page down"), eAction::prioWidget+1)
	{
	}
};

extern eAutoInitP0<listActions> i_listActions;

struct shortcutActions
{
	eActionMap map;
	eAction number0, number1, number2, number3, number4,
			number5, number6, number7, number8, number9, 
			red, green, yellow, blue, menu, escape;
	shortcutActions():
		map("shortcut", "Shortcuts"),
		number0(map, "0", "0", eAction::prioGlobal),
		number1(map, "1", "1", eAction::prioGlobal),
		number2(map, "2", "2", eAction::prioGlobal),
		number3(map, "3", "3", eAction::prioGlobal),
		number4(map, "4", "4", eAction::prioGlobal),
		number5(map, "5", "5", eAction::prioGlobal),
		number6(map, "6", "6", eAction::prioGlobal),
		number7(map, "7", "7", eAction::prioGlobal),
		number8(map, "8", "8", eAction::prioGlobal),
		number9(map, "9", "9", eAction::prioGlobal),
		red(map, "red", _("red"), eAction::prioGlobal),
		green(map, "green", _("green"), eAction::prioGlobal),
		yellow(map, "yellow", _("yellow"), eAction::prioGlobal),
		blue(map, "blue", _("blue"), eAction::prioGlobal),
		menu(map, "menu", _("menu"), eAction::prioGlobal),
		escape(map, "escape", _("escape"), eAction::prioGlobal)
	{
	}
};

extern eAutoInitP0<shortcutActions> i_shortcutActions;


#endif
