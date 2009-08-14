#include <plugin.h>
#include <stdio.h>
#include <src/enigma_main.h>
#include <lib/base/console.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/statusbar.h>

	// our plugin entry point, declared to use C calling convention
extern "C" int plugin_exec( PluginParam *par );

class statusWindow: public eWindow
{
	eConsoleAppContainer *conn;
	eButton *close;
	eLabel *status;
	eStatusBar *sbar;
	void appClosed(int)
	{
		close->show();
	}
	void dataAvail( eString str )
	{
		if ( str && str.length() > 10 )
		{
			while ( str[str.length()-1] == '\n' )
				str.erase( str.length()-1 );
			status->setText( status->getText()+str+'\n' );
		}
	}
	int eventHandler( const eWidgetEvent & evt )
	{
		if ( evt.type == eWidgetEvent::execBegin )
		{
			conn = new eConsoleAppContainer("/sbin/adsl-stop");
			if ( conn->running() )
			{
				CONNECT( conn->appClosed, statusWindow::appClosed );
				CONNECT( conn->dataAvail, statusWindow::dataAvail );
				status->setText( status->getText()+"disconnect...\n" );
			}
		}
		else if ( evt.type == eWidgetEvent::execDone )
		{
			if ( conn )
				delete conn;
		}
		else
			return eWindow::eventHandler(evt);
		return 1;
	}
public:
	statusWindow()
		:conn(0)
	{
		move(ePoint(150, 100));
		cresize(eSize(400, 250));
		setText(_("pppoe internet connection"));

		status = new eLabel(this);
		status->move(ePoint(10, 10));
		status->resize(eSize(380, 150));
		status->setText("Status:\n");
		status->setFlags(1 /*RS_WRAP*/ );

		close = new eButton(this);
		close->move(ePoint(10, 170));
		close->resize(eSize(185, 40));
		close->setText(_("close"));
		close->setShortcut("green");
		close->setShortcutPixmap("green");
		close->setHelpText(_("close window"));
		close->loadDeco();
		close->hide();
		CONNECT( close->selected, eWidget::accept );

		sbar = new eStatusBar(this);
		sbar->move( ePoint(0, clientrect.height()-30) );
		sbar->resize( eSize( clientrect.width(), 30) );
		sbar->loadDeco();
	}
};

	// our entry point.
int plugin_exec( PluginParam *par )
{
	statusWindow wnd;
	wnd.show();
	wnd.exec();
	wnd.hide();
	return 0;
}
