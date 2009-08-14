#ifndef __emessage_h
#define __emessage_h

#include <lib/gui/ewindow.h>

class eLabel;
class eTimer;

/**
 * \brief A (modal) messagebox.
 */
class eMessageBox: public eWindow
{
	eTimer *timer, *sectimer;
	eLabel *text, *icon, *lTimeout;
	eWidget *def;
	int timeout;
	void pressedOK();
	void pressedCancel();
	void pressedYes();
	void pressedNo();
	void updateTimeoutLabel();
	void init_eMessageBox(eString message, eString caption, int flags, int def, int timeout );
protected:	
	int eventHandler( const eWidgetEvent & );
public:
	static int ShowBox(eString string, eString caption, int flags=btOK, int def=btOK, int timeout=0 )
	{
		eMessageBox message(string, caption, flags, def, timeout);
		message.show();
		int res=message.exec();
		message.hide();
		return res;
	}
	enum { btOK=1, btCancel=2, btYes=4, btNo=8, btMax};
	enum { iconInfo=16, iconWarning=32, iconQuestion=64, iconError=128 };
	/**
	 * \brief Creates a messagebox.
	 *
	 * example: 
	 * \code
{
  eMessageBox message("Your documentation sucks!\nPlease consider using Neutrino!", "Documentation");
  message.show();
  message.exec();
  message.hide();
} \endcode
	 * \param string The string displayed inside the messagebox.
	 * \param caption The title of the messagebox.
	 */
	eMessageBox(eString string, eString caption, int flags=btOK, int def=btOK, int timeout=0 );
	~eMessageBox();
};

#endif
