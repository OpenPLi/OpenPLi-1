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
	eTimer stateTimer;
	int stateCount;
	eButton *close, *abort;
	eLabel *status;
	eStatusBar *sbar;
	void stateLoop()
	{
		status->setText(status->getText()+'.');
	}
	void appClosed(int)
	{
		stateTimer.stop();
		close->show();
		abort->hide();
	}
	void abortPressed()
	{
		status->hide();
		conn->kill();
		system("killall -9 pppd");
		system("killall -9 adsl-connect");
		system("killall -9 adsl-start");
	}
	void dataAvail( eString str )
	{
		if ( str && str.length() > 5 )
			status->setText( status->getText() + '\n' + str );
	}
	int eventHandler( const eWidgetEvent & evt )
	{
		if ( evt.type == eWidgetEvent::execBegin )
		{
			conn = new eConsoleAppContainer("/sbin/adsl-start");
			if ( conn->running() )
			{
				CONNECT( conn->appClosed, statusWindow::appClosed );
				CONNECT( conn->dataAvail, statusWindow::dataAvail );
				stateTimer.start(1000,false);
				status->setText( status->getText()+"connecting." );
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
		:conn(0), stateTimer(eApp), stateCount(0)
	{
		move(ePoint(150, 100));
		cresize(eSize(400, 250));
		setText(_("pppoe internet connection"));

		status = new eLabel(this);
		status->move(ePoint(10, 10));
		status->resize(eSize(380, 150));
		status->setText("Status:\n");

		abort = new eButton(this);
		abort->move(ePoint(10, 170));
		abort->resize(eSize(185, 40));
		abort->setText(_("abort"));
		abort->setShortcut("red");
		abort->setShortcutPixmap("red");
		abort->setHelpText(_("abort connection attempt"));
		abort->loadDeco();
		CONNECT( abort->selected, statusWindow::abortPressed );

		close = new eButton(this);
		close->move(ePoint(205, 170));
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

		CONNECT( stateTimer.timeout, statusWindow::stateLoop );
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
