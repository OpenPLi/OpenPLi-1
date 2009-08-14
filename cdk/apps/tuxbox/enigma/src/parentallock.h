#ifndef __SRC_PARENTALLOCK_H_
#define __SRC_PARENTALLOCK_H_

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/enumber.h>
#include <lib/gui/echeckbox.h>
#include <callablemenu.h>

class eParentalSetup: public ePLiWindow, public eCallableMenu
{
	eCheckbox *parentallock, *setuplock, *hidelocked;
	eButton *changeParentalPin, *changeSetupPin;
	eLabel* pin_timeout_label;
	eLabel* maxpin_errors_label;
	eLabel* pinerror_block_time_label;
	eNumber* pin_timeout;
	eNumber* maxpin_errors;
	eNumber* pinerror_block_time;

	int shidelocked;

	int parentalpin, setuppin;

	int parentalpin_enabled, setuppin_enabled;

	int pintimeout;
	int maxpinerrors;
	int pinerrorblocktime;
		
private:
	void okPressed();
	void loadSettings();
	void saveSettings();
	void slockChecked(int);
	void plockChecked(int);
	void hidelockChecked(int);
	bool ChangePin(eButton*);
	void changePin( eButton* );
	void init_eParentalSetup();
public:
	eParentalSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

class ParentalLockWindow:public eWindow
{
	eLabel *lPin;
	eNumber *nPin;
	void init_ParentalLockWindow(const char* windowText, int curNum );
public:
	ParentalLockWindow( const char *, int );
	void numEntered(int *i);
};

class pinCheck
{
public:
	enum pinType { parental = 0, setup };

	pinCheck();
	
	static pinCheck* getInstance();

	bool checkPin(pinType pintype, bool allwaysValidatePin = false );
	
	void resetParentalPinOk();
	void resetSetupPinOk();

	bool isLocked() { return locked; }
	
	int pLockActive();

	void switchLock();
	void setLocked(bool lock) { locked = lock; }	

	bool getParentalEnabled();


private:
	void resetParentalPinSetInError();
	void resetSetupPinSetInError();

	static pinCheck* instance;

	struct timeval last_time_parental_pinok;
	struct timeval last_time_setup_pinok;

	struct timeval time_lock_disabled;

	bool locked;

	int parentalpinErrorCnt, setuppinErrorCnt;
	
	struct timeval time_parental_set_in_error;
	struct timeval time_setup_set_in_error;
};

#endif // __SRC_PARENTALLOCK_H_
