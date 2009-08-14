#ifndef DISABLE_CI

#ifndef __ENIGMA_CI_H_
#define __ENIGMA_CI_H_

#include <src/enigma_mmi.h>
#include <callablemenu.h>

class eDVBCI;

class eButton;
class eCheckbox;
class eWindow;
class eStatusBar;

class enigmaCI: public eWindow, public eCallableMenu
{
	eButton *ok,*reset,*init,*app;
	eButton *reset2,*init2,*app2;

	eStatusBar *status;
	eDVBCI *DVBCI;
	eDVBCI *DVBCI2;
	eFixedMessagePump<const char*> ci_messages;
	eFixedMessagePump<const char*> ci2_messages;
private:
	void handleTwoServicesChecked(int);
	void okPressed();
	void resetPressed();
	void initPressed();
	void appPressed();
	void reset2Pressed();
	void init2Pressed();
	void app2Pressed();
	void gotCIinfoText(const char*);
	void gotCI2infoText(const char*);
	void updateCIinfo(const char* const&);
	void updateCI2info(const char* const&);
	void init_enigmaCI();
public:
	enigmaCI();
	~enigmaCI();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

class enigmaCIMMI : public enigmaMMI
{
	eDVBCI *ci;
	static std::map<eDVBCI*,enigmaCIMMI*> exist;
	void beginExec();
	void sendAnswer( AnswerType ans, int param, unsigned char *data );
public:
	static enigmaCIMMI* getInstance( eDVBCI* ci );
	enigmaCIMMI(eDVBCI *ci);
};

#endif // __ENIGMA_CI_H_

#endif // DISABLE_CI
