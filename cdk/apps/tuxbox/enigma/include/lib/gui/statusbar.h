#ifndef __CORE_GUI_STATUSBAR__
#define __CORE_GUI_STATUSBAR__

#include <lib/gui/ewidget.h>
#include <lib/gui/elabel.h>

class eStatusBar : public eLabel
{
	void update( const eWidget *);
	const eWidget* current;

	int setProperty(const eString &, const eString &);
	void initialize();
	Connection conn;
	void init_eStatusBar();
public:
	enum
	{
		flagOwnerDraw = 128,
	};
	eStatusBar( eWidget*, const char* deco="eStatusBar" );
	void setFlags( int );
};

#endif // __CORE_GUI_STATUSBAR__

