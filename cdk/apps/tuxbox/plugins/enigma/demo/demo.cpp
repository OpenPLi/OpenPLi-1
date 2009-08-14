/* a small enigma plugin sample. just a dialog with a listbox and some dizzy logic behind it */
#include <plugin.h>
#include <stdio.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/listbox.h>

	// our plugin entry point, declared to use C calling convention
extern "C" int plugin_exec( PluginParam *par );

	// our small demo dialog. based on a eWindow and thus is toplevel and has decoration.
class eDemoDialog: public eWindow
{
		// our listbox we like to use.
	eListBox<eListBoxEntryText> *listbox;
		// two buttons, one for abort and another one for accept.
	eButton *bt_abort, *bt_ok;
		// and some label to display status information.
	eLabel *lb_selected;
		// our callback if the user selects something:
	void selectedItem(eListBoxEntryText *item);
	void selectionChanged(eListBoxEntryText *item);
	
	int counter;
public:
		// the constructor.
	eDemoDialog();
		// the destructor.
	~eDemoDialog();
};

	// our entry point.
int plugin_exec( PluginParam *par )
{
		// our demo dialog instance.
	eDemoDialog dlg;
		// show the dialog...
	dlg.show();
		// give control to dialog.. (the dialog is modal!)
	int result=dlg.exec();
		// and after it, hide it again.
	dlg.hide();
	
		// if the result of the dialog is non-zero, show this warning.
	if (result)
	{	
		eMessageBox msg("You aborted that nice demo dialog.", "User Abort", eMessageBox::iconWarning|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
	}
	return 0;
}


eDemoDialog::eDemoDialog(): eWindow(1)
{
		// move our dialog to 100.100...
	cmove(ePoint(100, 100));
		// ...and make it fill the screen.
	cresize(eSize(520, 376));
		// set a title.
	setText("enigma demo plugin demonstration");
	
		// create our listbox. it's a child of our window, thus we give "this" as parent.
	listbox=new eListBox<eListBoxEntryText>(this);
		// move it into our widget. all positions are relative to the parent widget.
	listbox->move(ePoint(10, 10));
		// leave some space for the buttons (thus height()-100)
	listbox->resize(eSize(clientrect.width()-20, clientrect.height()-60));
	listbox->loadDeco();

		// create the ok button
	bt_ok=new eButton(this);
	bt_ok->move(ePoint(clientrect.width()-110, clientrect.height()-40));
	bt_ok->resize(eSize(100, 30));
		// set the shortcut action. in this case, it's the green button.
	bt_ok->setShortcut("green");
	bt_ok->setShortcutPixmap("green");
		// load some decoration (border)
	bt_ok->loadDeco();
		// set Text
	bt_ok->setText("OK");
		// "connect" the button. if the user pressed this button (either by selecting it and pressing
		// OK or by pressing green) the button should to something:
		// in this case, this will connect bt_ok->selected to our "accept dialog".
		// accept is just a close(0); - we will return with zero as return code.
	CONNECT(bt_ok->selected, eWidget::accept);
	
	lb_selected=new eLabel(this);
	lb_selected->move(ePoint(120, clientrect.height()-40));
	lb_selected->resize(eSize(clientrect.width()-120-120, 30)); // leave space for both buttons
	lb_selected->setText("move selection!");
	
		// create the abort button	
	bt_abort=new eButton(this);
	bt_abort->move(ePoint(10, clientrect.height()-40)); // listbox goes to height()-40
	bt_abort->resize(eSize(100, 30));
	bt_abort->setShortcut("red");
	bt_abort->setShortcutPixmap("red");
	bt_abort->loadDeco();
	bt_abort->setText("Abort");
		// reject is close(1) - we will return with nonzero return code.
	CONNECT(bt_abort->selected, eWidget::reject);
	
		// create some dummy listbox entries:
	new eListBoxEntryText(listbox, "This is a listbox.", (void*)0);
	new eListBoxEntryText(listbox, "and has multiple entries", (void*)1);
	new eListBoxEntryText(listbox, "which have all", (void*)2);
	new eListBoxEntryText(listbox, "some specific meaning.", (void*)3);
	
		// if the user selects an item, we want to be notified:
	CONNECT(listbox->selected, eDemoDialog::selectedItem);

		// if the user selects an item, we want to be notified:
	CONNECT(listbox->selchanged, eDemoDialog::selectionChanged);
	
		// and we set the focus to the listbox:
	setFocus(listbox);
	
	counter=0;
}

eDemoDialog::~eDemoDialog()
{
	// we have to do almost nothing here. all widgets are automatically removed
	// since they are child of us. the eWidget-destructor will to this for us.
}

void eDemoDialog::selectedItem(eListBoxEntryText *item)
{
		// if the user selected something, show that:
	if (item)
	{
		eString message;
		if (counter++ > 10)
			message="feel lonely? just start coding on enigma instead of wasting your time! ;)";
		else
			switch ((int)item->getKey())
			{
			case 0:
				if (counter & 1)
					message="My name is feared in every dirty corner on this island!";
				else
					message="So you got that job as janitor, after all.";
				break;
			case 1:
				if (counter & 1)
					message="No one will ever catch ME fighting as badly as you do.";
				else
					message="You run THAT fast?";
				break;
			case 2:
				if (counter & 1)
					message="My sword is famous all over the Carribbean!";
				else
					message="Too bad no one's ever heard of YOU at all!";
				break;
			case 3:
				if (counter & 1)
					message="My wisest enemies run away at the first sight of me!";
				else
					message="Even BEFORE they smell your breath?";
				break;
			}
		eMessageBox msg(message, "\"random\" rant", ((counter&1)?eMessageBox::iconQuestion:eMessageBox::iconInfo)|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
	} else
			// otherwise, just close the dialog.
		reject();
}

void eDemoDialog::selectionChanged(eListBoxEntryText *item)
{
	counter=0;
	lb_selected->setText(eString().sprintf("you selected item %d", (int)(item->getKey())));
}
